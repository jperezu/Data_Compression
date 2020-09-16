
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#define N_SAMPLES 0 //Max: 118(AC) 35(LWZ)
#define CODING_TYPE 0// 0 -> Arithmetic, 1 -> LZW, 2 -> Both

//Global variables
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
	int64_t time_4 = 0;
	int64_t time_5 = 0;
	int64_t time_6 = 0;
	int64_t time_7 = 0;
	int64_t time_8 = 0;
	int64_t comp_time_arith = 0;
	int64_t decomp_time_arith = 0;
	int64_t comp_time_lzw = 0;
	int64_t decomp_time_lzw = 0;

	uint32_t mem_1_arith = 0;
	uint32_t mem_2_arith = 0;
	uint32_t mem_1_lzw = 0;
	uint32_t mem_2_lzw = 0;
	uint16_t  arith_compressed;
	int8_t* lzw_compressed;
	int stream_length;
	int i;
	int j;
	int z = 0;

	stream_length = (N_SAMPLES != 0) ? N_SAMPLES * sample_length : 0;
	lzw_compressed = malloc((stream_length)*sizeof(int8_t));
	if (DEBUG) printf("-FREE HEAP: %i\n",esp_get_free_heap_size());

	while(!stop){
		if  (N_SAMPLES == 0)  stream_length += sample_length;

		//Generate an input of STREAM_LENGTH bits to feed the algorithm
		j = 0;

		char input[stream_length];
		for (i=0;i<stream_length;i++){
			input[i] = (j != INTEGER_DIG) ? digits[esp_random() % (sizeof(digits))] : '.';	//condition ? (true):(false);
			if(j == (INTEGER_DIG + DECIMAL_DIG)) j = 0;
			else j++;
		}
		input[stream_length] = '\0';		//end of file
		if (DEBUG){
			printf("[%i samples in buffer]-----------------------------------------\n",
					stream_length/sample_length);
			printf("%s\n", input);
		}
		/********************** ARITHMETIC CODING **********************/
		if(CODING_TYPE == 0 || CODING_TYPE == 2){
			mem_1_arith = esp_get_free_heap_size();
			//arith_compressed = malloc(sizeof(uint16_t));
			time_1 = esp_timer_get_time();
			compress(input, &arith_compressed);		//running compression algorithm
			time_2 = esp_timer_get_time();
			mem_2_arith = esp_get_free_heap_size();
			if (DEBUG){
				printf("-ARITH COMPRESS:\n%0X\nDecode:\n", arith_compressed);
			}
			time_3 = esp_timer_get_time();
			expand(&arith_compressed, input);		//running decompression algorithm
			time_4 = esp_timer_get_time();

			if (DEBUG){
				//print_distribution();
			}

			if (stop) stream_length -= sample_length;
			if(esp_get_free_heap_size() < stream_length) stop = 1;

			//printf("%lld\n",time_4 - time_3);

		}
		/**************************************************************/

		/***************************** LZW ****************************/
		if(CODING_TYPE == 1 || CODING_TYPE == 2){
			lzw_compressed = realloc(NULL, stream_length*sizeof(int8_t));
			//lzw_compressed = malloc((stream_length)*sizeof(int8_t));
			mem_1_lzw = esp_get_free_heap_size();
			time_5 = esp_timer_get_time();

			LZWEncode(input, lzw_compressed);			//running compression algorithm

			time_6 = esp_timer_get_time();
			mem_2_lzw = esp_get_free_heap_size();

			z = 0;
			if(DEBUG) printf("LWZ COMPRESS:\n");
			while(lzw_compressed[z] != -1){
				if(DEBUG) printf("%0X", lzw_compressed[z]);
				z++;
			}
			if(DEBUG) printf("\nDecode:\n");
			time_7 = esp_timer_get_time();

			LZWDecode(lzw_compressed, input);		//running decompression algorithm

			free(lzw_compressed);
			time_8 = esp_timer_get_time();

			if (stop) stream_length -= sample_length;
			if(esp_get_free_heap_size() < stream_length) stop = 1;

			//printf("%lld\n",time_6 - time_5);

			printf("%u\n", z*sizeof(int8_t));

		}
		if (DEBUG) printf("FREE HEAP: %i\n",esp_get_free_heap_size());
		/**************************************************************/
		vTaskDelay(10 / portTICK_PERIOD_MS);

		//x++;
	}

	//printf("%i\n",stream_length);

	if(CODING_TYPE == 2){
		comp_time_arith = time_2 - time_1;		//get compression time
		decomp_time_arith = time_4 - time_3;		//get decompression time

		comp_time_lzw = time_6 - time_5;		//get compression time
		decomp_time_lzw = time_8 - time_7;		//get decompression time

		{
			//REWORK!! PART OF THE CODE REMOVED BY ENRIC'S COMMIT

		}

		printf("\n\t~ARITHMETIC CODING~\n"
				"-> Stream size: %u, compressed size: %u, used heap: %u [bytes]\n",
				stream_length*sizeof(char), sizeof(uint16_t), mem_1_arith - mem_2_arith);
		printf("\n   EXECUTION TIME (us)\n");
		printf("+------------------------+\n");
		printf("|Compressing time: %lld   |\n", comp_time_arith);
		printf("+------------------------+\n");
		printf("|Decoding time: %lld     |\n", decomp_time_arith);
		printf("+------------------------+\n");

		printf("\n\t~LEMPEL-ZIV-WELCH~\n"
				"-> Stream size: %u, compressed size: %u, used heap: %u [bytes]\n",
				stream_length*sizeof(char), z*sizeof(int8_t), mem_1_lzw - mem_2_lzw);
		printf("\n   EXECUTION TIME (us)\n");
		printf("+------------------------+\n");
		printf("|Compressing time: %lld  |\n", comp_time_lzw);
		printf("+------------------------+\n");
		printf("|Decoding time: %lld      |\n", decomp_time_lzw);
		printf("+------------------------+\n");
	}
}
