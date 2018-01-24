/*
* mem_mng.c
*
* Created: 1/16/2018 11:13:21 AM
*  Author: alelop
*/
#include "mem_manage.h"

#if DEBUG_TRACE
#include "common.h"
#endif
#include <string.h>
#include "bios.h"

#define IN_USE_bm 0x8000
#define HEADER_SIZE sizeof(uint16_t)

volatile uint8_t memory[MEMORY_SIZE];
uint16_t totalAllocatedMemory;

void combine_unused_blocks(uint16_t index);

mem_loc* internal_malloc(uint16_t size) {
	uint16_t index = 0;
	uint16_t intSize, blkSize = 0;
	uint8_t inUse;

	if (size > MEMORY_SIZE - HEADER_SIZE || MEMORY_SIZE - totalAllocatedMemory < size + HEADER_SIZE)
		return NULL;

	CRITICAL_SYNC_START();
	intSize = size + HEADER_SIZE;

	/************************************************************************
	4 bytes
	hhmm mm
	xxxx xxxx xxxx xxxx
	0    4    8    12
	************************************************************************/
	combine_unused_blocks(0);
	while (index < MEMORY_SIZE) {
		memcpy(&blkSize, &memory[index], HEADER_SIZE);
		inUse = (blkSize & IN_USE_bm) > 0;
		blkSize &= ~IN_USE_bm;

		if (inUse == 0 && (blkSize == 0 || blkSize >= intSize))
			break;
		index += blkSize;
	}

	// found a block that was freed that we can re-purpose
	if (index < MEMORY_SIZE && blkSize > 0)
		return internal_realloc(&memory[index], size, false);

	if (index >= MEMORY_SIZE) {
		CRITICAL_SYNC_END();
		return NULL;
	}

	intSize |= IN_USE_bm;
	memcpy(&memory[index], &intSize, HEADER_SIZE);
	totalAllocatedMemory += size + HEADER_SIZE;

	CRITICAL_SYNC_END();
	return &memory[index + HEADER_SIZE];
}

mem_loc* internal_calloc(uint16_t numElements, uint8_t elementSize) {
	mem_loc *ptr;
	uint16_t size = numElements * elementSize;

	if (numElements != 0 && size / numElements != elementSize)
		return NULL;
	CRITICAL_SYNC_START();
	ptr = m_malloc(size);
	if (ptr == NULL)
		return ptr;

	memset(ptr, 0, size);
	CRITICAL_SYNC_END();
	return ptr;
}

mem_loc* internal_realloc(mem_loc *ptr, uint16_t size, bool verifyIsAllocated) {
	uint16_t blk, nextBlk = 0, prevBlk = 0;
	uint16_t blkSize, nextBlkSize = 0, prevBlkSize = 0, origBlkSize;
	uint16_t space = 0;
	mem_loc nextAddr = memory, prevAddr = 0, addr;
	mem_loc retPtr = 0;

	if (size > MEMORY_SIZE - HEADER_SIZE || ptr < memory || ptr >= memory + MEMORY_SIZE)
		return NULL;

	if (ptr == NULL)
		return m_malloc(size);

	retPtr = ptr;
	retPtr -= HEADER_SIZE;
	memcpy(&blk, retPtr, HEADER_SIZE);
	if (verifyIsAllocated && (blk & IN_USE_bm) == 0)
		return NULL;

	origBlkSize = blkSize = blk & ~IN_USE_bm;

	// remaining the same size
	if (blkSize == size + HEADER_SIZE)
		return ptr;

	CRITICAL_SYNC_START();

	combine_unused_blocks(0);
	// grab the previous block
	if (retPtr != memory) {
		while (nextAddr != retPtr && nextAddr < memory + MEMORY_SIZE) {
			memcpy(&prevBlk, nextAddr, HEADER_SIZE);
			prevBlkSize = prevBlk & ~IN_USE_bm;
			prevAddr = nextAddr;
			nextAddr += prevBlkSize;
		}
	}

	// grab the next block
	if (retPtr + blkSize < memory + MEMORY_SIZE) {
		memcpy(&nextBlk, retPtr + blkSize, HEADER_SIZE);
		nextBlkSize = nextBlk & ~IN_USE_bm;
		nextAddr = retPtr + blkSize;
	}

	// shrinking
	if (size < blkSize) {
		if (blkSize - size < HEADER_SIZE)
			goto realloc_end;

		space = blkSize - size - HEADER_SIZE;
		blkSize = size + HEADER_SIZE;
		blk = blkSize | IN_USE_bm;
		if ((nextBlk & IN_USE_bm) == 0)
			space += nextBlkSize;

		goto link_block;
	}
	// growing
	else {
		// if the previous block is unused, shift current block back to it if there's going to be enough room between
		// prev/current/next (if available) to resize the current block
		space = blkSize;
		addr = retPtr;
		if ((prevBlk & IN_USE_bm) == 0 && prevAddr != NULL) {
			space += prevBlkSize;
			addr = prevAddr;
		}
		if ((nextBlk & IN_USE_bm) == 0 && nextAddr != NULL)
			space += nextBlkSize;

		if (space >= size && addr + size + HEADER_SIZE <= memory + MEMORY_SIZE) {
			move_block:

			blk = (size | IN_USE_bm) + HEADER_SIZE;
			space = size + HEADER_SIZE > space ? 0 : space - size - HEADER_SIZE;

			if ((prevBlk & IN_USE_bm) == 0 && prevAddr != NULL) {
				memcpy(prevAddr + HEADER_SIZE, retPtr + HEADER_SIZE, blkSize - HEADER_SIZE);
				memset(retPtr + 1, 0, blkSize - 1);
				retPtr = prevAddr;
			}
		
			link_block:
			// if there's not enough room for a header after we expand this block, simply claim the extra space
			if (space < HEADER_SIZE)
				blk += space;
			// otherwise create a free'd header for the next block
			else {
				blkSize = blk & ~IN_USE_bm;
				nextAddr = retPtr + blkSize;
				nextBlkSize = space;
				nextBlk = nextBlkSize;
				memcpy(nextAddr, &nextBlk, HEADER_SIZE);
				space = 0;
			}

			memcpy(retPtr, &blk, HEADER_SIZE);
		}
		// no room to expand in-place, try looking elsewhere for an available block
		// the only possible option at this point is to find a free block with equal
		// or lesser size than the desired size since we already combined unused blocks
		else {
			addr = memory;
			while (addr < memory + MEMORY_SIZE) {
				memcpy(&blk, addr, HEADER_SIZE);
				blkSize = blk & ~IN_USE_bm;

				if ((blk & IN_USE_bm) == 0 && (blkSize == 0 || blkSize >= size + HEADER_SIZE))
					break;
				addr += blkSize;
			}
			// if no space left
			if (addr >= memory + MEMORY_SIZE || (blkSize == 0 && addr + size + HEADER_SIZE >= memory + MEMORY_SIZE)) {
				CRITICAL_SYNC_END();
				return NULL;
			}

			// found a spot we can use
			prevBlk = blk;
			prevBlkSize = blkSize;
			prevAddr = addr;
			space = prevBlkSize;
			blkSize = origBlkSize;

			nextAddr = prevAddr + prevBlkSize;
			memcpy(&nextBlk, nextAddr, HEADER_SIZE);
			nextBlkSize = nextBlk & ~IN_USE_bm;

			goto move_block;
		}
	}

	if (size + HEADER_SIZE > origBlkSize)
		totalAllocatedMemory += (size + HEADER_SIZE) - origBlkSize + space;
	else
		totalAllocatedMemory -= origBlkSize - size - HEADER_SIZE;

	realloc_end:
	CRITICAL_SYNC_END();
	return retPtr == 0 ? 0 : retPtr + HEADER_SIZE;
}

void internal_free(mem_loc *ptr) {
	uint16_t blk;
	mem_loc addr = 0;

	if (ptr < memory || ptr >= memory + MEMORY_SIZE)
		return;
	
	addr = ptr;
	addr -= 2;
	memcpy(&blk, addr, HEADER_SIZE);
	if ((blk & IN_USE_bm) == 0)
		return;

	CRITICAL_SYNC_START();
	blk &= ~IN_USE_bm;
	memcpy(addr, &blk, HEADER_SIZE);
	totalAllocatedMemory -= blk;
	CRITICAL_SYNC_END();
}

void combine_unused_blocks(uint16_t index) {
	uint8_t inUse = 0;
	uint16_t nextBlk, blkSize = 0;

	while (inUse == 0 && index + blkSize < MEMORY_SIZE) {
		memcpy(&nextBlk, &memory[index] + blkSize, HEADER_SIZE);
		if (nextBlk == 0)
			break;
		inUse = (nextBlk & IN_USE_bm) >> 8;
		if (!inUse) {
			blkSize += nextBlk;
			memcpy(&memory[index], &blkSize, HEADER_SIZE);
		}
	}
}

#if DEBUG_TRACE

void mem_manager_test(void) {
	uint8_t i;
	mem_loc *data1, *data2, *data3, *data4;
	uint32_t actual;
	uint16_t header;

	data1 = m_malloc(4);
	memset(data1, 0xaa, 4);
	memcpy(&actual, &memory[2], 4);
	memcpy(&header, memory, HEADER_SIZE);
	m_assert(actual == 0xaaaaaaaa);
	m_assert(header == (6 | IN_USE_bm));
	m_assert(totalAllocatedMemory == 6);
	// memory: 06 80 aa aa   aa aa

	data2 = m_malloc(4);
	memset(data2, 0xbb, 4);
	actual = 0;
	memcpy(&actual, &memory[8], 4);
	m_assert(actual == 0xbbbbbbbb);
	m_assert(totalAllocatedMemory == 12);
	// memory: 06 80 aa aa   aa aa 06 80   bb bb bb bb

	m_free(data1);
	memcpy(&header, memory, HEADER_SIZE);
	m_assert(header == 6);
	m_assert(totalAllocatedMemory == 6);
	// memory: 06 00 aa aa   aa aa 06 80   bb bb bb bb

	data1 = m_malloc(6);
	memset(data1, 0xcc, 6);
	for (i = 0; i < 6; i++)
		m_assert(memory[i + 12 + HEADER_SIZE] == 0xcc);
	memcpy(&header, &memory[12], HEADER_SIZE);
	m_assert(header == ((6 + HEADER_SIZE) | IN_USE_bm));
	m_assert(totalAllocatedMemory == 14);
	// memory: 06 00 aa aa   aa aa 06 80   bb bb bb bb   08 80 cc cc   cc cc cc cc

	data3 = m_malloc(8);
	memset(data3, 0xdd, 8);
	for (i = 0; i < 8; i++)
		m_assert(memory[i + 20 + HEADER_SIZE] == 0xdd);
	m_assert(totalAllocatedMemory == 24);
	// memory: 06 00 aa aa   aa aa 06 80   bb bb bb bb   08 80 cc cc   cc cc cc cc    0a 80 dd dd   dd dd dd dd   dd dd 00 00

	// extend block at end of list with space remaining
	data3 = m_realloc(data3, 10);
	memset(data3, 0xdd, 10);
	memcpy(&header, &memory[0x14], HEADER_SIZE);
	m_assert(header == (0x000c | IN_USE_bm));
	for (i = 0; i < 10; i++)
		m_assert(memory[i + 20 + HEADER_SIZE] == 0xdd);
	m_assert(totalAllocatedMemory == 26);
	// memory: 06 00 aa aa   aa aa 06 80   bb bb bb bb   08 80 cc cc   cc cc cc cc    0c 80 dd dd   dd dd dd dd   dd dd dd dd

	// extend block at end of list with no space remaining
	data4 = m_realloc(data3, 12);
	m_assert(data4 == NULL);
	memcpy(&header, &memory[20], HEADER_SIZE);
	m_assert(header == (0x000c | IN_USE_bm));
	m_assert(totalAllocatedMemory == 26);
	// memory: 06 00 aa aa   aa aa 06 80   bb bb bb bb   08 80 cc cc   cc cc cc cc    0c 80 dd dd   dd dd dd dd   dd dd dd dd
}

uint8_t mem2[] = { 0x17, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0x03,
0x80, 0xcc, 0x04, 0x80, 0xaa, 0xaa, 0x02, 0x80 };

uint8_t mem3[] = { 0x0d, 0x80, 0xaa, 0xaa, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0x0a, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0x03,
					0x80, 0xcc, 0x04, 0, 0, 0, 0x02, 0x80 };

uint8_t mem4[] = { 0x08, 0x80, 0xaa, 0xaa, 0, 0, 0x00, 0,
					0x0f, 0, 0, 0, 0, 0x0a, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0x03,
					0x80, 0xcc, 0x04, 0, 0, 0, 0x02, 0x80 };

uint8_t mem5[] = { 0x02, 0x80, 0x15, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0x0a, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0x03,
					0x80, 0xcc, 0x04, 0, 0, 0, 0x02, 0x80 };

void mem_manager_test2(void) {
	mem_loc data1;
	// extend block and cause it to move locations
	totalAllocatedMemory = 9;
	memcpy(memory, mem2, MEMORY_SIZE);
	data1 = m_realloc(memory + 0x1a + HEADER_SIZE, 11);
	m_assert(data1 != NULL);
	m_assert(memcmp(mem3, memory, MEMORY_SIZE) == 0);
	m_assert(totalAllocatedMemory == 18);

	totalAllocatedMemory = 7;
	memcpy(memory, mem2, MEMORY_SIZE);
	memory[31] = 0;
	mem3[31] = 0;
	data1 = m_realloc(memory + 0x1a + HEADER_SIZE, 11);
	m_assert(data1 != NULL);
	m_assert(memcmp(mem3, memory, MEMORY_SIZE) == 0);
	m_assert(totalAllocatedMemory == 16);

	// extend block and claim extra space since there's no room for a header
	mem3[31] = 0x80;
	memcpy(memory, mem3, MEMORY_SIZE);
	totalAllocatedMemory = 0x12;
	data1 = m_realloc(memory + HEADER_SIZE, 20);
	m_assert(totalAllocatedMemory == 0x1c);

	// standard realloc to make it smaller and require a dummy header to be created
	memcpy(memory, mem3, MEMORY_SIZE);
	totalAllocatedMemory = 0x12;
	data1 = m_realloc(memory + HEADER_SIZE, 0x06);
	m_assert(memcmp(mem4, memory, MEMORY_SIZE) == 0);
	m_assert(totalAllocatedMemory == 0x0d);

	// realloc to size zero
	memcpy(memory, mem3, MEMORY_SIZE);
	totalAllocatedMemory = 0x12;
	data1 = m_realloc(memory + HEADER_SIZE, 0);
	m_assert(memcmp(mem5, memory, MEMORY_SIZE) == 0);
	m_assert(totalAllocatedMemory == 7);

	// realloc by only 1 so that its forced to claim the extra space and not change size
	memcpy(memory, mem3, MEMORY_SIZE);
	totalAllocatedMemory = 0x12;
	data1 = m_realloc(memory + HEADER_SIZE, 0x0c);
	m_assert(memcmp(mem3, memory, MEMORY_SIZE) == 0);
	m_assert(totalAllocatedMemory == 0x12);
}

#endif