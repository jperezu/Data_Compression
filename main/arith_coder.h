/*
 * The code is used with permission of the Author and under the
 * MIT LICENSE stated below:
 *
 * Copyright Mark Nelson

 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software
 * is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Some changes have been implemented to adapt the code to the project specific
 * purpose and hardware platform.
 *
 * Listing 1 -- coder.h
 *
 * This header file contains the constants, declarations, and
 * prototypes needed to use the arithmetic coding routines.  These
 * declarations are for routines that need to interface with the
 * arithmetic coding stuff in coder.c
 *
 */

#include <stdint.h>

#define MAXIMUM_SCALE   16383  /* Maximum allowed frequency count */
#define ESCAPE          256    /* The escape symbol               */
#define DONE            -1     /* The output stream empty  symbol */
#define FLUSH           -2     /* The symbol to flush the model   */

/*
 * A symbol can either be represented as an int, or as a pair of
 * counts on a scale.  This structure gives a standard way of
 * defining it as a pair of counts.
 */
typedef struct {
                unsigned short int low_count;
                unsigned short int high_count;
                unsigned short int scale;
               } SYMBOL;

extern long underflow_bits;    /* The present underflow count in  */
                               /* the arithmetic coder.           */

/*
 * This is a the probability table for the symbol set used
 * in this example.  Each symbols has a low and high range,
 * and the total count is fixed at 11.
 */
typedef struct distribution{
          char c;
          unsigned int low;
          unsigned int high;
       } distribution;


/*
 * Function prototypes.
 */
void initialize_arithmetic_decoder( uint32_t *stream );
void remove_symbol_from_stream( uint32_t *stream, SYMBOL *s );
void initialize_arithmetic_encoder( void );
void encode_symbol( uint32_t *stream, SYMBOL *s );
void flush_arithmetic_encoder( uint32_t *stream );
unsigned short int get_current_count( SYMBOL *s );

void compress(char * input, uint32_t* compressed_file);
void expand(uint32_t* compressed_file, char* input);
void convert_int_to_symbol( char c, SYMBOL *s );
char convert_symbol_to_int( unsigned int count, SYMBOL *s );
void error_exit( char *message );

void print_distribution();
