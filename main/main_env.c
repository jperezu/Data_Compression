/*
 * Listing 5 -- bill.c
 *
 * This short demonstration program will use arithmetic data
 * compression to encode and then decode a string that only uses
 * the letters out of the phrase "BILL GATES".  It uses a fixed
 * table of probabilities that is hardcoded in.  Note that it
 * differs from the example in the article in that it adds a single
 * new symbol, '\0', which is used to indicate the end of the string.
 *
 * To build this program:
 *
 * Turbo C:     tcc -w bill.c bitio.c coder.c
 * QuickC:      qcl /W3 bill.c bitio.c coder.c
 * Zortech:     ztc bill.c bitio.c coder.c
 * *NIX:        gcc -o bill bill.c bitio.c coder.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"


#include "arith_coder.h"
#include "lzw.h"

#ifdef CONFIG_IDF_TARGET_ESP32
	#define CHIP_NAME "ESP32"
#endif

//Test evaluation variables
#define INTEGER_DIG 2
#define DECIMAL_DIG 2
#define N_SAMPLES 20
#define ITERATIONS 1000
#define STATIC_SIZE 0
#define CODING_TYPE 1 // 0 -> Arithmetic, 1 -> LZW

char digits[] = { '0','1','2','3','4','5','6','7','8','9'};
extern uint8_t stop;
int sample_length = (INTEGER_DIG + DECIMAL_DIG + 1);
/*
 * This example program compresses an input string, sending
 * the output to a file.  It then expands the output file,
 * sending the decoded characters to the screen.
 */



void app_main(){

	esp_err_t esp_timer_init(); //initialization of the timer -- call this function from stratup

	int64_t time_1 = 0;
	int64_t time_2 = 0;
	int64_t time_3 = 0;
	int64_t comp_time = 0;
	int64_t decomp_time = 0;
	uint32_t*  arith_compressed;
	uint32_t* lzw_compressed;
	int stream_length;
	int x = 0;
	int i;
	int j;

	stream_length = (STATIC_SIZE) ? N_SAMPLES * sample_length : 0;

	while(!stop && x < ITERATIONS){
		if  (!STATIC_SIZE)  stream_length += sample_length;
		char input[stream_length];

		//Generate an input of STREAM_LENGTH bits to feed the algorithm
	j = 0;
	for (i=0;i<stream_length;i++){
		input[i] = (j != INTEGER_DIG) ? digits[rand() % (sizeof(digits))] : '.';	//condition ? (true):(false);
		if(j == (INTEGER_DIG + DECIMAL_DIG)) j = 0;
		else j++;
	}
	input[stream_length] = '\0';		//end of file
	printf("[%i samples in buffer]-----------------------------------------\n",
			stream_length/sample_length);
	printf("%s\n", input);

/********************** ARITHMETIC CODING **********************/
	if(!CODING_TYPE){
	arith_compressed = malloc(sizeof(uint32_t));
	time_1 = esp_timer_get_time();
		compress(input, arith_compressed);		//running compression algorithm
	time_2 = esp_timer_get_time();
	printf("%0X\n", *arith_compressed);
		expand(arith_compressed, input);		//running decompression algorithm
	time_3 = esp_timer_get_time();
	//print_distribution();
	if (stop) stream_length -= sample_length;
	if(esp_get_free_heap_size() < stream_length) stop = 1;
	}
/**************************************************************/

/***************************** LZW ****************************/
	if(CODING_TYPE){
		lzw_compressed = malloc((stream_length+1)*sizeof(uint32_t));
		time_1 = esp_timer_get_time();
		printf("Encode:\n");
			LZWEncode(input, lzw_compressed);		//running compression algorithm
		time_2 = esp_timer_get_time();
		i = 0;
//BUGGED?? //////////////////////////////////////////
		while(lzw_compressed[i] != NULL){
				printf("%i", lzw_compressed[i]);
				i++;
			}
//////////////////////////////////////////////////
		printf("\n");
		printf("\nDecode:\n");
			LZWDecode(lzw_compressed, input);		//running decompression algorithm
		printf("\n");
		time_3 = esp_timer_get_time();

		printf("FREE HEAP: %i\n",esp_get_free_heap_size());
		if ( esp_get_free_heap_size() < stream_length) stop = 1;
		//if (stop) stream_length -= sample_length;
		}

/**************************************************************/
	vTaskDelay(10 / portTICK_PERIOD_MS);
	x++;
	}

	comp_time = time_2 - time_1;		//get compression time
	decomp_time = time_3 - time_1;		//get decompression time
	//compressed = mem_1

	if(!CODING_TYPE){
		//REWORK!! PART OF THE CODE REMOVED BY ENRIC'S COMMIT
		float ratio_string = (float) stream_length/
					  (float) sizeof(arith_compressed);
		float ratio_float = (float) sizeof(float)*stream_length/
						  (float) (sizeof(arith_compressed)*sample_length);
		printf("--String Scenario (%i samples [%i bytes] in %i bytes )--\n",
				stream_length/sample_length,
				stream_length/sizeof(char),
				sizeof(arith_compressed));
		printf("\tMax compression ratio: %.2f\n",ratio_string);
		printf("\tMax data rate savings: %.2f%%\n", 100 - 100/ratio_string);
		printf("--Float Scenario (%i samples [%i bytes] in %i bytes)--\n",
				stream_length/sample_length,
				sizeof(float)*stream_length/sample_length,
				sizeof(arith_compressed));
		printf("\tMax compression ratio: %.2f\n",ratio_float);
		printf("\tMax data rate savings: %.2f%%\n", 100 - 100/ratio_float);
	}
	printf("Compressing time: %lld\n", comp_time);
	printf("Decoding time: %lld\n", decomp_time);
}
