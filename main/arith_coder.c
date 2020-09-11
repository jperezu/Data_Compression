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
 * Listing 2 -- coder.c
 *
 * This file contains the code needed to accomplish arithmetic
 * coding of a symbol.  All the routines in this module need
 * to know in order to accomplish coding is what the probabilities
 * and scales of the symbol counts are.  This information is
 * generally passed in a SYMBOL structure.
 *
 * This code was first published by Ian H. Witten, Radford M. Neal,
 * and John G. Cleary in "Communications of the ACM" in June 1987,
 * and has been modified slightly for this article.  The code
 * is  published here with permission.
 */

#include "arith_coder.h"
#include <stdlib.h>
#include <stdio.h>
#include "bitio.h"

/*
 * These four variables define the current state of the arithmetic
 * coder/decoder.  They are assumed to be 16 bits long.  Note that
 * by declaring them as short ints, they will actually be 16 bits
 * on most 80X86 and 680X0 machines, as well as VAXen.
 */
static unsigned short int code;  /* The present input code value       */
static unsigned short int low;   /* Start of the current code range    */
static unsigned short int high;  /* End of the current code range      */
long underflow_bits;             /* Number of underflow bits pending   */

distribution probabilities_encoder[]= {{ '0',  0,  1  },
									   { '1',  1,  2  },
									   { '2',  2,  3  },
									   { '3',  3,  4  },
									   { '4',  4,  5  },
									   { '5',  5,  6  },
									   { '6',  6,  7  },
									   { '7',  7,  8  },
									   { '8',  8,  9 },
									   { '9',  9,  10 },
									   { '.',  10,  11 },
									   { '\0', 11, 12  }
									   };
distribution probabilities_decoder[]= {{ '0',  0,  1  },
									   { '1',  1,  2  },
									   { '2',  2,  3  },
									   { '3',  3,  4  },
									   { '4',  4,  5  },
									   { '5',  5,  6  },
									   { '6',  6,  7  },
									   { '7',  7,  8  },
									   { '8',  8,  9 },
									   { '9',  9,  10 },
									   { '.',  10,  11 },
									   { '\0', 11, 12  }
									   };

unsigned int encode_distribution_size = sizeof(probabilities_encoder)/sizeof(probabilities_encoder[0]);
unsigned int decode_distribution_size = sizeof(probabilities_decoder)/sizeof(probabilities_decoder[0]);
uint8_t stop = 0;
/*
 * This routine must be called to initialize the encoding process.
 * The high register is initialized to all 1s, and it is assumed that
 * it has an infinite string of 1s to be shifted into the lower bit
 * positions when needed.
 */
void initialize_arithmetic_encoder()
{
    low = 0;
    high = 0xffff;
    underflow_bits = 0;
}

/*
 * This routine is called to encode a symbol.  The symbol is passed
 * in the SYMBOL structure as a low count, a high count, and a range,
 * instead of the more conventional probability ranges.  The encoding
 * process takes two steps.  First, the values of high and low are
 * updated to take into account the range restriction created by the
 * new symbol.  Then, as many bits as possible are shifted out to
 * the output stream.  Finally, high and low are stable again and
 * the routine returns.
 */
void encode_symbol( uint32_t *stream, SYMBOL *s )
{
    long range;
/*
 * These three lines rescale high and low for the new symbol.
 */
    range = (long) ( high-low ) + 1;
    high = low + (unsigned short int )
                 (( range * s->high_count ) / s->scale - 1 );
    low = low + (unsigned short int )
                 (( range * s->low_count ) / s->scale );
/*
 * This loop turns out new bits until high and low are far enough
 * apart to have stabilized.
 */
    for ( ; ; )
    {
/*
 * If this test passes, it means that the MSDigits match, and can
 * be sent to the output stream.
 */
        if ( ( high & 0x8000 ) == ( low & 0x8000 ) )
        {
            output_bit( stream, high & 0x8000 );
            while ( underflow_bits > 0 )
            {
                output_bit( stream, ~high & 0x8000 );
                underflow_bits--;
            }
        }
/*
 * If this test passes, the numbers are in danger of underflow, because
 * the MSDigits don't match, and the 2nd digits are just one apart.
 */
        else if ( ( low & 0x4000 ) && !( high & 0x4000 ))
        {
            underflow_bits += 1;
            low &= 0x3fff;
            high |= 0x4000;
        }
        else
            return ;
        low <<= 1;
        high <<= 1;
        high |= 1;
    }
}

/*
 * At the end of the encoding process, there are still significant
 * bits left in the high and low registers.  We output two bits,
 * plus as many underflow bits as are necessary.
 */
void flush_arithmetic_encoder( uint32_t *stream )
{
    output_bit( stream, low & 0x4000 );
    underflow_bits++;
    while ( underflow_bits-- > 0 )
        output_bit( stream, ~low & 0x4000 );
}

/*
 * When decoding, this routine is called to figure out which symbol
 * is presently waiting to be decoded.  This routine expects to get
 * the current model scale in the s->scale parameter, and it returns
 * a count that corresponds to the present floating point code:
 *
 *  code = count / s->scale
 */
unsigned short int get_current_count( SYMBOL *s )
{
    long range;
    unsigned short int count;

    range = (long) ( high - low ) + 1;
    count = (short int)
            ((((long) ( code - low ) + 1 ) * s->scale-1 ) / range );
    //printf("\n( %u - %u ) + 1 ) * %u - 1 ) / %lu = %i\n",code, low, s->scale, range, count);
    //print_distribution();
    return( count );
}

/*
 * This routine is called to initialize the state of the arithmetic
 * decoder.  This involves initializing the high and low registers
 * to their conventional starting values, plus reading the first
 * 16 bits from the input stream into the code value.
 */
void initialize_arithmetic_decoder( uint32_t *stream )
{
    int i;

    code = 0;
    for ( i = 0 ; i < 16 ; i++ )
    {
        code <<= 1;
        code += input_bit( stream );
    }
    low = 0;
    high = 0xffff;
}

/*
 * Just figuring out what the present symbol is doesn't remove
 * it from the input bit stream.  After the character has been
 * decoded, this routine has to be called to remove it from the
 * input stream.
 */
void remove_symbol_from_stream( uint32_t *stream, SYMBOL *s )
{
    long range;

/*
 * First, the range is expanded to account for the symbol removal.
 */
    range = (long)( high - low ) + 1;
    high = low + (unsigned short int)
                 (( range * s->high_count ) / s->scale - 1 );
    low = low + (unsigned short int)
                 (( range * s->low_count ) / s->scale );
/*
 * Next, any possible bits are shipped out.
 */
    for ( ; ; )
    {
/*
 * If the MSDigits match, the bits will be shifted out.
 */
        if ( ( high & 0x8000 ) == ( low & 0x8000 ) )
        {
        }
/*
 * Else, if underflow is threatining, shift out the 2nd MSDigit.
 */
        else if ((low & 0x4000) == 0x4000  && (high & 0x4000) == 0 )
        {
            code ^= 0x4000;
            low   &= 0x3fff;
            high  |= 0x4000;
        }
/*
 * Otherwise, nothing can be shifted out, so I return.
 */
        else
            return;
        low <<= 1;
        high <<= 1;
        high |= 1;
        code <<= 1;
        code += input_bit( stream );
    }
}

/*
 * This is the compress routine.  It shows the basic algorithm for
 * the compression programs used in this article.  First, an input
 * characters is loaded.  The modeling routines are called to
 * convert the character to a symbol, which has a high, low and
 * range.  Finally, the arithmetic coder module is called to
 * output the symbols to the bit stream.
 */
void compress(char* input, uint32_t* compressed_file)
{
    int i;
    char c;
    SYMBOL s;

    puts( "Compressing..." );
    initialize_output_bitstream();
    initialize_arithmetic_encoder();
    for ( i=0 ; ; )
    {
        c = input[ i++ ];
        convert_int_to_symbol( c, &s );
        encode_symbol(compressed_file, &s );
        if ( c == '\0' )
            break;
    }
    flush_arithmetic_encoder(compressed_file );
    flush_output_bitstream(compressed_file );
}

/*
 * This expansion routine demonstrates the basic algorithm used for
 * decompression in this article.  It first goes to the modeling
 * module and gets the scale for the current context.  (Note that
 * the scale is fixed here, since this is not an adaptive model).
 * It then asks the arithmetic decoder to give a high and low
 * value for the current input number scaled to match the current
 * range.  Finally, it asks the modeling unit to convert the
 * high and low values to a symbol.
 */
void expand(uint32_t* compressed_file, char* input)
{
    SYMBOL s;
    char c;
    unsigned int count;
    unsigned int i = 0;
    puts( "Decoding..." );

    initialize_input_bitstream();
    initialize_arithmetic_decoder( compressed_file );
    for ( ; ; )
    {
        s.scale = decode_distribution_size;
        count = get_current_count( &s );
        c = convert_symbol_to_int( count, &s );
        if ( c == '\0' )
            break;
        remove_symbol_from_stream( compressed_file, &s );
        if(c == input[i])
			putc( c, stdout );
		else{
			error_exit(" [DATA CORRUPTED]");
        	printf("Expected [%c] but [%c] decoded\n", input[i], c);
        	return;
		}
        i++;
    }
 	putc( '\n', stdout );
}

/*
 * This routine is called to convert a character read in from
 * the text input stream to a low, high, range SYMBOL.  This is
 * part of the modeling function.  In this case, all that needs
 * to be done is to find the character in the probabilities table
 * and then retrieve the low and high values for that symbol.
 */
void convert_int_to_symbol( char c, SYMBOL *s )
{
    int i;
    int j;
    i=0;
    for ( ; ; )
    {
        if ( c == probabilities_decoder[ i ].c )
        {
            s->low_count = probabilities_encoder[ i ].low;
            s->high_count = probabilities_encoder[ i ].high;
            s->scale = encode_distribution_size;

            probabilities_encoder[ i ].high++;
            for (j=i+1;j< sizeof(probabilities_encoder)/sizeof(probabilities_encoder[0]); j++){
            	probabilities_encoder[j].low++;
            	probabilities_encoder[j].high++;
            }
            encode_distribution_size++;
            return;
        }
        if ( probabilities_decoder[i].c == '\0' )
            error_exit( "Trying to encode a char not in the table" );
        i++;
    }
}

/*
 * This modeling function is called to convert a SYMBOL value
 * consisting of a low, high, and range value into a text character
 * that can be sent to a file.  It does this by finding the symbol
 * in the probability table that straddles the current range.
 */
char convert_symbol_to_int( unsigned int count, SYMBOL *s )
{
    int i;
    int j;
    i = 0;
    for ( ; ; )
    {
        if ( count >= probabilities_decoder[ i ].low &&
             count < probabilities_decoder[ i ].high )
        {
            s->low_count = probabilities_decoder[ i ].low;
            s->high_count = probabilities_decoder[ i ].high;
            s->scale = decode_distribution_size;

            probabilities_decoder[ i ].high++;
			for (j=i+1;j< sizeof(probabilities_decoder)/sizeof(probabilities_decoder[0]); j++){
				probabilities_decoder[j].low++;
				probabilities_decoder[j].high++;
			}
			decode_distribution_size++;
            return( probabilities_decoder[ i ].c );
        }
        if ( probabilities_decoder[ i ].c == '\0' ){
            error_exit( "Failure to decode character" );
        }
        i++;
    }
}

/*
 * A generic error routine.
 */
void error_exit( char *message )
{
    puts( message );
    stop = 1;
}

void print_distribution(){
	int j;
	printf("   DISTRIBUTION\n");
		printf(" ENCODER || DECODER\n");
		for (j=0;j< sizeof(probabilities_encoder)/sizeof(probabilities_encoder[0]); j++)
					printf("{%c,%i,%i} || {%c,%i,%i}\n",
							probabilities_encoder[j].c,
							probabilities_encoder[j].low,
							probabilities_encoder[j].high,
							probabilities_decoder[j].c,
							probabilities_decoder[j].low,
							probabilities_decoder[j].high);
}
