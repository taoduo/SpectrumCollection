#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "hidapi.h"

#define OUTPUT_REPORT_BYTE_LENGTH 9
#define INPUT_REPORT_BYTE_LENGTH 64
#define MAX_STR 256
#define PRODUCT_ID 0x0001
#define VENDOR_ID 0x20E2	// ASEQ Instruments vendor ID

int main(int argc, char *argv[]) {
	int res; // response state holder

	// INIT THE DEVICE (WRITE): prepare the command
	unsigned char cmd[10];
	memset(cmd, 0, 10);
	cmd[1]=0x03; // set the command
	cmd[0]=0; // some boilerplate stuff
	cmd[9]=15;
	// open the device, assuming there is only one spectrometer
	hid_device *spectrometer = hid_open(VENDOR_ID, PRODUCT_ID, NULL);
	if (spectrometer == NULL) {
		printf("Device open failed\n");
		return 1;
	}
	// actually deliver the command
	res = hid_write(spectrometer, cmd, OUTPUT_REPORT_BYTE_LENGTH);
	if(res < 0) {
		printf("Unable to write report\n");
	} else {
		printf("Write Successful\n");
	}
	// close the device
	hid_close(spectrometer);

	// INIT THE DEVICE (READ): prepare the input buffer
	unsigned char input[100];
	memset(input, 0, INPUT_REPORT_BYTE_LENGTH);
	// open the device, assuming there is only one spectrometer
	spectrometer = hid_open(VENDOR_ID, PRODUCT_ID, NULL);
	if (spectrometer == NULL) {
		printf("Device open failed\n");
		return 1;
	}
	// read the report
	res = hid_read_timeout(spectrometer, input, INPUT_REPORT_BYTE_LENGTH, 1 * 1000);
	if(res <= 0) {
  	printf("Unable to read report:%d\n", res);
		return 2;
  } else {
		printf("Read Successful\n");
	}
	// some boilerplate stuff: making the first character 0
  memmove(&input[1], input, INPUT_REPORT_BYTE_LENGTH);
  input[0] = 0;
	// close the device
	hid_close(spectrometer);
	return 0;
}
