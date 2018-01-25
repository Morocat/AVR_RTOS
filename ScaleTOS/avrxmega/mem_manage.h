/*
 * mem_manage.h
 *
 * Created: 1/16/2018 11:13:33 AM
 *  Author: alelop
 */ 


#ifndef MEM_MANAGE_H_
#define MEM_MANAGE_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "config.h"

//#define DEBUG_TRACE 1
//#define m_assert assert
//#define CRITICAL_SYNC_START()
//#define CRITICAL_SYNC_END()

#define MEMORY_SIZE 4096
//#define MEMORY_SIZE 32

#define m_malloc(_size) internal_malloc(_size)
#define m_free(_ptr) internal_free(_ptr)
#define m_realloc(_ptr, _size) internal_realloc(_ptr, _size, true)
#define m_calloc(_numEle, _eleSize) internal_calloc(_numEle, _eleSize)

//typedef uint32_t mem_loc;
typedef uint16_t mem_loc;

mem_loc* internal_malloc(uint16_t size);
mem_loc* internal_realloc(mem_loc *ptr, uint16_t size, bool verifyIsAllocated);
mem_loc* internal_calloc(uint16_t numElements, uint8_t elementSize);
void internal_free(mem_loc *ptr);

#if DEBUG_TRACE
void mem_manager_test(void);
void mem_manager_test2(void);
#endif

#endif /* MEM_MANAGE_H_ */