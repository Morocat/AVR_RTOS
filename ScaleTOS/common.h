/*
 * common.h
 *
 * Created: 1/11/2018 10:51:12 AM
 *  Author: alelop
 */ 


#ifndef COMMON_H_
#define COMMON_H_

#include "config.h"
#include <stdbool.h>

#if (!DEBUG_TRACE)
#include <assert.h>
#endif

#if DEBUG_TRACE
#define m_assert(e) if((e) == 0) {									  \
						__m_assert(__func__, __FILE__, __LINE__, #e); \
						asm volatile ("rjmp .-2 \n\t");				  \
}
#else
#define m_assert(e) assert(e)
#endif

bool __m_assert(const char *__func, const char *__file, int __lineno, const char *__sexp);

#if DEBUG_TRACE
char gStrBuf[128]; // generic string buffer
#endif

#endif /* COMMON_H_ */