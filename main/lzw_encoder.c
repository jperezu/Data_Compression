/***************************************************************************
*                    Lempel-Ziv-Welch Encoding Functions
*
*   File    : lzwencode.c
*   Purpose : Provides a function for Lempel-Ziv-Welch encoding of file
*             streams
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
#include <stdlib.h>
#include <errno.h>
#include "lzw.h"

/***************************************************************************
*                            TYPE DEFINITIONS
***************************************************************************/
/* node in dictionary tree */
typedef struct dict_node_t
{
	uint64_t codeWord;      /* code word for this entry */
    unsigned char suffixChar;   /* last char in encoded string */
    uint64_t prefixCode;    /* code for remaining chars in string */

    /* pointer to child nodes */
    struct dict_node_t *left;   /* child with < key */
    struct dict_node_t *right;  /* child with >= key */
} dict_node_t;

/***************************************************************************
*                                CONSTANTS
***************************************************************************/

/***************************************************************************
*                                  MACROS
***************************************************************************/

/***************************************************************************
*                            GLOBAL VARIABLES
***************************************************************************/

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/

/* dictionary tree node create/free */
static dict_node_t *MakeNode(const uint64_t codeWord,
    const uint64_t prefixCode, const unsigned char suffixChar);
static void FreeTree(dict_node_t *node);

/* searches tree for matching dictionary entry */
static dict_node_t *FindDictionaryEntry(dict_node_t *root,
    const uint64_t prefixCode, const unsigned char c);

/* makes key from prefix code and character */
static uint64_t MakeKey(const uint64_t prefixCode,
    const unsigned char suffixChar);

/* write encoded data */

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/***************************************************************************
*   Function   : LZWEncodeFile
*   Description: This routine reads an input file 1 character at a time and
*                writes out an LZW encoded version of that file.
*   Parameters : fpIn - pointer to the open binary file to encode
*                fpOut - pointer to the open binary file to write encoded
*                       output
*   Effects    : fpIn is encoded using the LZW algorithm with CODE_LEN codes
*                and written to fpOut.  Neither file is closed after exit.
*   Returned   : 0 for success, -1 for failure.  errno will be set in the
*                event of a failure.
***************************************************************************/
int LZWEncode(char* fpIn, int8_t* fpOut)
{
	uint64_t code;                  /* code for current string */
    unsigned char currentCodeLen;       /* length of the current code */
    uint64_t nextCode;              /* next available code index */
    uint64_t c;                              /* character to add to string */

    dict_node_t *dictRoot;              /* root of dictionary tree */
    dict_node_t *node;                  /* node of dictionary tree */


    /* validate arguments */
    if ((NULL == fpIn) || (NULL == fpOut))
    {
        errno = ENOENT;
        return -1;
    }


    /* initialize dictionary as empty */
    dictRoot = NULL;

    /* start MIN_CODE_LEN bit code words */
    currentCodeLen = MIN_CODE_LEN;

    nextCode = INIT_CODE;  /* code for next (first) string */

    /* now start the actual encoding process */

    c = *fpIn++;

    if ('\0' == c)
    {
        return -1;      /* empty file */
    }
    else
    {
        code = c - '0';       /* start with code string = first character */
    }

    /* create a tree root from 1st 2 character string */
    if ((c = *fpIn++) != '\0')
    {
    	if (c != '.') c = c - '0';
    	else c = 10;
        /* special case for NULL root */
        dictRoot = MakeNode(nextCode, code, c);

        if (NULL == dictRoot)
        {
            perror("Making Dictionary Root");
            //BitFileToFILE(bfpOut);
            return -1;
        }

        nextCode++;

        /* write code for 1st char */
        *fpOut++ = code;
        /* new code is just 2nd char */
        code = c;
    }

    /* now encode normally */
    while ((c = *fpIn++) != '\0')
    {
    	if (c != '.') c = c - '0';
    	else c = 10;
        /* look for code + c in the dictionary */
        node = FindDictionaryEntry(dictRoot, code, c);

        if ((node->prefixCode == code) &&
            (node->suffixChar == c))
        {
            /* code + c is in the dictionary, make it's code the new code */
            code = node->codeWord;
        }
        else
        {
            /* code + c is not in the dictionary, add it if there's room */
            if (nextCode <= MAX_CODE_LEN)
            {
                dict_node_t *tmp;


                tmp = MakeNode(nextCode, code, c);

                if (NULL == dictRoot)
                {
                    perror("Making Dictionary Node");
                    FreeTree(dictRoot);
                    //BitFileToFILE(bfpOut);
                    return -1;
                }

                if(nextCode != 9) nextCode++;
                else nextCode+=2;
                if (MakeKey(code, c) <
                    MakeKey(node->prefixCode, node->suffixChar))
                {
                    node->left = tmp;
                }
                else
                {
                    node->right = tmp;
                }
            }
            else
            {
            	printf("%lli/%i\n",nextCode, MAX_CODE_LEN);
                fprintf(stderr, "Error: Dictionary Full\n");
                free(dictRoot);
                return -1;
            }

            /* are we using enough bits to write out this code word? */
//            while ((code >= (CURRENT_MAX_CODES(currentCodeLen) - 1)) &&
//                (currentCodeLen < MAX_CODE_LEN))
//            {
//                /* mark need for bigger code word with all ones */
////                PutCodeWord(bfpOut, (CURRENT_MAX_CODES(currentCodeLen) - 1),
////                    currentCodeLen);
//
//                currentCodeLen++;
//            }

            /* write out code for the string before c was added */
            *fpOut++ = code;
            /* new code is just c */
            code = c;
        }
    }

    /* no more input.  write out last of the code. */
    *fpOut++ = code;
    *fpOut = -1;
    /* we've encoded everything, free bitfile structure */
    //BitFileToFILE(bfpOut);

    /* free the dictionary */
    FreeTree(dictRoot);

    return 0;
}

/***************************************************************************
*   Function   : MakeKey
*   Description: This routine creates a simple key from a prefix code and
*                an appended character.  The key may be used to establish
*                an order when building/searching a dictionary tree.
*   Parameters : prefixCode - code for all but the last character of a
*                             string.
*                suffixChar - the last character of a string
*   Effects    : None
*   Returned   : Key built from string represented as a prefix + char.  Key
*                format is {ms nibble of c} + prefix + {ls nibble of c}
***************************************************************************/
static uint64_t MakeKey(const uint64_t prefixCode,
    const unsigned char suffixChar)
{
	uint64_t key;

    /* position ms nibble */
    key = suffixChar & 0xF0;
    key <<= MAX_CODE_LEN;

    /* include prefix code */
    key |= (prefixCode << 4);

    /* inclulde ls nibble */
    key |= (suffixChar & 0x0F);

    return key;
}

/***************************************************************************
*   Function   : MakeNode
*   Description: This routine creates and initializes a dictionary entry
*                for a string and the code word that encodes it.
*   Parameters : codeWord - code word used to encode the string prefixCode +
*                           suffixChar
*                prefixCode - code for all but the last character of a
*                             string.
*                suffixChar - the last character of a string
*   Effects    : Node is allocated for new dictionary entry
*   Returned   : Pointer to newly allocated node or NULL on error.
*                errno will be set on an error.
***************************************************************************/
static dict_node_t *MakeNode(const uint64_t codeWord,
    const uint64_t prefixCode, const unsigned char suffixChar)
{
    dict_node_t *node;

    node = malloc(sizeof(dict_node_t));

    if (NULL != node)
    {
        node->codeWord = codeWord;
        node->prefixCode = prefixCode;
        node->suffixChar = suffixChar;

        node->left = NULL;
        node->right = NULL;
    }

    return node;
}

/***************************************************************************
*   Function   : FreeTree
*   Description: This routine will free all nodes of a tree rooted at the
*                node past as a parameter.
*   Parameters : node - root of tree to free
*   Effects    : frees allocated tree node from initial parameter down.
*   Returned   : none
***************************************************************************/
static void FreeTree(dict_node_t *node)
{
    if (NULL == node)
    {
        /* nothing to free */
        return;
    }

    /* free left branch */
    if (node->left != NULL)
    {
        FreeTree(node->left);
    }

    /* free right branch */
    if (node->right != NULL)
    {
        FreeTree(node->right);
    }

    /* free root */
    free(node);
}

/***************************************************************************
*   Function   : FindDictionaryEntry
*   Description: This routine searches the dictionary tree for an entry
*                with a matching string (prefix code + suffix character).
*                If one isn't found, the parent node for that string is
*                returned.
*   Parameters : prefixCode - code for the prefix of string
*                c - last character in string
*   Effects    : None
*   Returned   : If string is in dictionary, pointer to node containing
*                string, otherwise pointer to suitable parent node.  NULL
*                is returned for an empty tree.
***************************************************************************/
static dict_node_t *FindDictionaryEntry(dict_node_t *root,
    const uint64_t prefixCode, const unsigned char c)
{
	uint64_t searchKey, key;

    if (NULL == root)
    {
        return NULL;
    }

    searchKey = MakeKey(prefixCode, c);     /* key of string to find */

    while (1)
    {
        /* key of current node */
        key = MakeKey(root->prefixCode, root->suffixChar);

        if (key == searchKey)
        {
            /* current node contains string */
            return root;
        }
        else if (searchKey < key)
        {
            if (NULL != root->left)
            {
                /* check left branch for string */
                root = root->left;
            }
            else
            {
                /* string isn't in tree, it can be added as a left child */
                return root;
            }
        }
        else
        {
            if (NULL != root->right)
            {
                /* check right branch for string */
                root = root->right;
            }
            else
            {
                /* string isn't in tree, it can be added as a right child */
                return root;
            }
        }
    }
}
