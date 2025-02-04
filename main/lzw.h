/***************************************************************************
*          Header for Lempel-Ziv-Welch Encoding and Decoding Library
*
*   File    : lzw.h
*   Purpose : Provides prototypes for functions that use Lempel-Ziv-Welch
*             coding to encode/decode files.
*   Author  : Michael Dipperstein
*   Date    : January 30, 2004
*
****************************************************************************
*
* LZW: An ANSI C Lempel-Ziv-Welch Encoding/Decoding Routines
* Copyright (C) 2005, 2007, 2014 by
* Michael Dipperstein (mdipper@alumni.engr.ucsb.edu)
*
* This file is part of the lzw library.
*
* The lzw library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation; either version 3 of the
* License, or (at your option) any later version.
*
* The lzw library is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
* General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
***************************************************************************/

#ifndef _LZW_H_
#define _LZW_H_

/***************************************************************************
*                             INCLUDED FILES
***************************************************************************/
#include <stdio.h>
#include <limits.h>

/***************************************************************************
*                                CONSTANTS
***************************************************************************/
#define MIN_CODE_LEN   	8                   /* min # bits in a coded word */
#define MAX_CODE_LEN    256                /* max # bits in a code word */
#define MIN_DECODE_LEN  9                   /* min # bits in a decode word */
#define MAX_DECODE_LEN  14                  /* max # bits in a code word */

#define INIT_CODE		11
#define FIRST_CODE      256     /* value of 1st string code */
#define MAX_CODES       (1 << MAX_CODE_LEN)
#define MAX_DECODES       (1 << MAX_DECODE_LEN)

#define DEBUG		0

#if (MIN_DECODE_LEN <= CHAR_BIT)
#error Code words must be larger than 1 character
#endif

#if ((MAX_DECODES - 1) > INT_MAX)
#error There cannot be more codes than can fit in an integer
#endif

/***************************************************************************
*                                  MACROS
***************************************************************************/
#define CURRENT_MAX_CODES(bits)     ((unsigned int)(1 << (bits)))
#define CURRENT_MAX_DECODES(bits)     ((unsigned int)(1 << (bits)))
/***************************************************************************
*                               PROTOTYPES
***************************************************************************/
 /* encode inFile */
int LZWEncode(char* fpIn, int8_t* fpOut);

/* decode inFile*/
int LZWDecode(int8_t* fpIn, char *fpOut);


#endif  /* ndef _LZW_H_ */
