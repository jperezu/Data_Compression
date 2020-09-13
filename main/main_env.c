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
#include "arith_coder.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"

#ifdef CONFIG_IDF_TARGET_ESP32
	#define CHIP_NAME "ESP32"
#endif

#define INTEGER_DIG 2
#define DECIMAL_DIG 2
#define N_SAMPLES 20
#define ITERATIONS 1000

char digits[] = { '0','1','2','3','4','5','6','7','8','9'};
extern uint8_t stop;
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
	uint32_t  compressed_file = 0;
	uint32_t mem_1;
	uint32_t mem_2;
	uint32_t compressed;
	int stream_length = 0;
	int x = 0;
	int i;
	int j;


	while(!stop && x < ITERATIONS){
		stream_length += (INTEGER_DIG + DECIMAL_DIG + 1);
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
			stream_length/(INTEGER_DIG + DECIMAL_DIG + 1));
	printf("%s\n", input);


//random input has been generated
	mem_1 = heap_caps_get_free_size(MALLOC_CAP_8BIT);		//returns the number of free bytes in data memory


	time_1 = esp_timer_get_time();
	compress(input, &compressed_file);		//running compression algorithm
	time_2 = esp_timer_get_time();
	mem_2 = heap_caps_get_free_size(MALLOC_CAP_8BIT);
	printf("-Compressed: %0X\n",compressed_file);
	expand(&compressed_file, input);		//running decompression algorithm
	time_3 = esp_timer_get_time();

	//print_distribution();
	if (stop) stream_length -= (INTEGER_DIG + DECIMAL_DIG + 1);
	vTaskDelay(10 / portTICK_PERIOD_MS);
	x++;
	}

	comp_time = time_2 - time_1;		//get compression time
	decomp_time = time_3 - time_1;		//get decompression time
	//compressed = mem_1
	float ratio_string = (float) stream_length/
			      (float) sizeof(compressed_file);
	float ratio_float = (float) sizeof(float)*stream_length/
				      (float) (sizeof(compressed_file)*(INTEGER_DIG + DECIMAL_DIG + 1));
	printf("--String Scenario (%i samples [%i bytes] in %i bytes )--\n",
			stream_length/(INTEGER_DIG + DECIMAL_DIG + 1),
			stream_length/sizeof(char),
			sizeof(compressed_file));
	printf("\tMax compression ratio: %.2f\n",ratio_string);
	printf("\tMax data rate savings: %.2f%%\n", 100 - 100/ratio_string);
	printf("--Float Scenario (%i samples [%i bytes] in %i bytes)--\n",
			stream_length/(INTEGER_DIG + DECIMAL_DIG + 1),
			sizeof(float)*stream_length/(INTEGER_DIG + DECIMAL_DIG + 1),
			sizeof(compressed_file));
	printf("\tMax compression ratio: %.2f\n",ratio_float);
	printf("\tMax data rate savings: %.2f%%\n", 100 - 100/ratio_float);
	printf("\tCompressing time: %lld\n", comp_time);
	printf("\tCompressing time: %lld\n", decomp_time);
}
