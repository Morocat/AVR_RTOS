/*
 * common.c
 *
 * Created: 1/11/2018 10:51:21 AM
 *  Author: alelop
 */ 
 #include "common.h"

 #include "kernel.h"
 #if DEBUG_TRACE
 #include "serial.h"
 #include <stdio.h>
 #endif

 bool __m_assert(const char *__func, const char *__file, int __lineno, const char *__sexp) {
 #if DEBUG_TRACE
	 int prefix = 0;
	 while (*(__file + prefix) == '.' || *(__file + prefix) == '/')
		prefix++;
	 sprintf(gStrBuf, "Assert failure!\n\rFile: %s\n\rLine: %d\n\rExp: %s\n\r", __file + prefix, __lineno, __sexp);
	 serial_send_string(gStrBuf);
#endif
	kernel_disable_scheduler();
	 return false;
 }