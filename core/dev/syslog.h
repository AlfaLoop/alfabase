/*
This file contains an implementation of ee_printf that only requires a method to output a char to a UART without pulling in library code.
This code is based on a file that contains the following:
 Copyright (C) 2002 Michael Ringgaard. All rights reserved.
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
 3. Neither the name of the project nor the names of its contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 SUCH DAMAGE.
*/

#ifndef _SYSLOG_H__
#define _SYSLOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
//#include "contiki-conf.h"

//#ifdef SYSLOG_CONF_FLOAT
#define HAS_FLOAT
//#endif

/**
 * \brief      				Show the debug message on debugger APP over the Log services
 * \param *fmt    			This is the C string that contains the text to be written to the stream.
 *
 *							It can optionally contain embedded format tags that are replaced by the values
 *							specified in subsequent additional arguments and formatted as requested.
 *							Format tags prototype: %[flags][width][.precision][length]specifier, as explained below :
 *
 *							%x: got replaced by a hexadecimal value corresponding to integer variable.
 *							%c: got replaced by value of a character variable.
 *							%s: got replaced by value of a string variable.
 *							%d: got replaced by value of an integer variable.
 *             				%f: got replaced by value of a float variable.
 *							\n: got replaced by a newline.
 *
 * \return 					Total length
 */

int ee_vsprintf(char *buf, const char *fmt, va_list args);
int ee_sprintf(char *buf, char const *fmt, ...);
int syslog(const char *fmt, ...);


#ifdef __cplusplus
}
#endif
#endif /* _SYSLOG_H__ */
/** @} */
