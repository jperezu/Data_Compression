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

	uint32_t  compressed_file = 0;
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
		input[i] = (j != INTEGER_DIG) ? digits[rand() % (sizeof(digits))] : '.';
		if(j == (INTEGER_DIG + DECIMAL_DIG)) j = 0;
		else j++;
	}
	input[stream_length] = '\0';
	printf("[%i samples in buffer]-----------------------------------------\n",
			stream_length/(INTEGER_DIG + DECIMAL_DIG + 1));
	printf("%s\n", input);


	compress(input, &compressed_file);
	printf("-Compressed: %0X\n",compressed_file);
	expand(&compressed_file, input);

	//print_distribution();
	if (stop) stream_length -= (INTEGER_DIG + DECIMAL_DIG + 1);
	vTaskDelay(10 / portTICK_PERIOD_MS);
	x++;
	}
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
}
