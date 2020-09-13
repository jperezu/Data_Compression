/***************************************************************************
*                    Lempel-Ziv-Welch Decoding Functions
*
*   File    : lzwdecode.c
*   Purpose : Provides a function for decoding Lempel-Ziv-Welch encoded
*             file streams
*   Author  : Michael Dipperstein
*   Date    : January 30, 2005
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

/***************************************************************************
*                             INCLUDED FILES
***************************************************************************/
#include <stdio.h>
#include <errno.h>
#include "lzw.h"

/***************************************************************************
*                            TYPE DEFINITIONS
***************************************************************************/
typedef struct
{
    unsigned char suffixChar;   /* last char in encoded string */
    unsigned int prefixCode;    /* code for remaining chars in string */
} decode_dictionary_t;

/***************************************************************************
*                                CONSTANTS
***************************************************************************/

/***************************************************************************
*                                  MACROS
***************************************************************************/

/***************************************************************************
*                            GLOBAL VARIABLES
***************************************************************************/

/* dictionary of string the code word is the dictionary index */
static decode_dictionary_t dictionary[(MAX_CODES - FIRST_CODE)];

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/
static unsigned char DecodeRecursive(unsigned int code, char *fpOut);


/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/***************************************************************************
*   Function   : LZWDecodeFile
*   Description: This routine reads an input file 1 encoded string at a
*                time and decodes it using the LZW algorithm.
*   Parameters : fpIn - pointer to the open binary file to decode
*                fpOut - pointer to the open binary file to write decoded
*                       output
*   Effects    : fpIn is decoded using the LZW algorithm with CODE_LEN codes
*                and written to fpOut.  Neither file is closed after exit.
*   Returned   : 0 for success, -1 for failure.  errno will be set in the
*                event of a failure.
***************************************************************************/
int LZWDecode(uint32_t* fpIn, char *fpOut)
{

    unsigned int nextCode;              /* value of next code */
    unsigned int lastCode;              /* last decoded code word */
    unsigned int code;                  /* code word to decode */
    unsigned char currentCodeLen;       /* length of code words now */
    unsigned char c;                    /* last decoded character */

    /* validate arguments */
    if ((NULL == fpIn) || (NULL == fpOut))
    {
        errno = ENOENT;
        return -1;
    }

    /* start MIN_CODE_LEN bit code words */
    currentCodeLen = MIN_CODE_LEN;

    /* initialize for decoding */
    nextCode = FIRST_CODE;  /* code for next (first) string */

    /* first code from file must be a character.  use it for initial values */
    lastCode = *fpIn++;
    c = lastCode;
    //fputc(lastCode, fpOut);
    putc((char)lastCode, stdout);
    /* decode rest of file */
    while ((int)(code = *fpIn++) != (int)NULL)
    {

        /* look for code length increase marker */
        while (((CURRENT_MAX_CODES(currentCodeLen) - 1) == code) &&
            (currentCodeLen < MAX_CODE_LEN))
        {
            currentCodeLen++;
            code = *fpIn++;
        }

        if (code < nextCode)
        {
            /* we have a known code.  decode it */
            c = DecodeRecursive(code, fpOut);
        }
        else
        {
            /***************************************************************
            * We got a code that's not in our dictionary.  This must be due
            * to the string + char + string + char + string exception.
            * Build the decoded string using the last character + the
            * string from the last code.
            ***************************************************************/
            unsigned char tmp;

            tmp = c;
            c = DecodeRecursive(lastCode, fpOut);
            //fputc(tmp, fpOut);
            putc((char)tmp, stdout);
        }

        /* if room, add new code to the dictionary */
        if (nextCode < MAX_CODES)
        {
            dictionary[nextCode - FIRST_CODE].prefixCode = lastCode;
            dictionary[nextCode - FIRST_CODE].suffixChar = c;
            nextCode++;
        }

        /* save character and code for use in unknown code word case */
        lastCode = code;
    }

    /* we've decoded everything, free bitfile structure */
    //BitFileToFILE(bfpIn);

    return 0;
}

/***************************************************************************
*   Function   : DecodeRecursive
*   Description: This function uses the dictionary to decode a code word
*                into the string it represents and write it to the output
*                file.  The string is actually built in reverse order and
*                recursion is used to write it out in the correct order.
*   Parameters : code - the code word to decode
*                fpOut - the file that the decoded code word is written to
*   Effects    : Decoded code word is written to a file
*   Returned   : The first character in the decoded string
***************************************************************************/
static unsigned char DecodeRecursive(unsigned int code, char *fpOut)
{
    unsigned char c;
    unsigned char firstChar;

    if (code >= FIRST_CODE)
    {
        /* code word is string + c */
        c = dictionary[code - FIRST_CODE].suffixChar;
        code = dictionary[code - FIRST_CODE].prefixCode;

        /* evaluate new code word for remaining string */
        firstChar = DecodeRecursive(code, fpOut);
    }
    else
    {
        /* code word is just c */
        c = code;
        firstChar = code;
    }

    //fputc(c, fpOut);
    putc((char)c, stdout);
    return firstChar;
}

