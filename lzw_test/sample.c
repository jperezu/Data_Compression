/***************************************************************************
*                 Sample Program Using LZW Encoding Library
*
*   File    : sample.c
*   Purpose : Demonstrate usage of LZW encoding library
*   Author  : Michael Dipperstein
*   Date    : January 30, 2005
*
****************************************************************************
*
* SAMPLE: Sample usage of Lempel-Ziv-Welch Encoding Library
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

/***************************************************************************
*                             INCLUDED FILES
***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "lzw.h"

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/****************************************************************************
*   Function   : main
*   Description: This is the main function for this program, it validates
*                the command line input and, if valid, it will call
*                functions to encode or decode a file using the lzw
*                coding algorithm.
*   Parameters : argc - number of parameters
*                argv - parameter list
*   Effects    : Encodes/Decodes input file
*   Returned   : 0 for success, -1 for failure.  errno will be set in the
*                event of a failure.
****************************************************************************/
int main(){

    char *fpIn = "123\0";   /* pointer to open input file */
    //49 50 51 256 258 257 259 262 257
    uint32_t *fpOut = (uint32_t*) malloc(sizeof(uint32_t));    /* pointer to open output file */


	LZWEncode(fpIn, fpOut);
	while(*fpOut != (int)'\0')printf("%i ", *fpOut++);
	//LZWDecode(fpOut, fpIn);


    //free(fpIn);
    free(fpOut);
    return 0;
}
