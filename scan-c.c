#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#include "smpl1.h"

double WavelengthArray[4096];
bool waveRead;

static void doScan();
void readWavelength(double * WaveLengthArray);
void readSpec(int ExpN, int NScans, int Blank, signed short * rawSpec);

int main(int argc, char *argv[]) {
	printf("Start\n");
	fflush(stdout);
	doScan();
	return 0;
}


static void doScan() {
	printf("Do Scan\n");
	fflush(stdout);
	if(!smpl_DevDetect()) {
		if(!smpl_FindTheHID()) {
			printf("Spectroscope not Found\n");
			return;
		}
	}
	printf("Spectroscope Found\n");
	fflush(stdout);

	smpl_reset();
	printf("Wavelength\n");
	fflush(stdout);
	if(waveRead == false) {
		readWavelength(WavelengthArray);
		waveRead = true;
		//Todo check for success here.
	}
	printf("Run Scan\n");
	fflush(stdout);
	signed short rawSpec[4096];
	memset(rawSpec, 0,4096);
	readSpec(5,1,0,rawSpec);
	printf("Save Scan\n");
	fflush(stdout);


	char filename[1024];
	sprintf(filename, "./spectra/%u.csv", (unsigned)time(NULL));
	printf("Saving file %s\n", filename);
	fflush(stdout);
	// int q;
	// for(q = 0; q < 3646; q++) {
	// 		printf("{%f,%d},", WavelengthArray[q], rawSpec[q]);
	// }
	// fflush(stdout);
//	FILE * fp = fopen(filename, "w" );
//	if(fp != NULL) {
//
//		int q = 0;
//
//		for(q = 0; q < 3646; q++) {
//			fprintf(fp, "%lf,%hd\n", WavelengthArray[q], rawSpec[q]);
//		}
//		fclose(fp);
//
//		printf("Test Complete");
//	} else {
//        printf("Oh dear, something went wrong! %s\n", strerror(errno));
//		printf("Failed to write test results");
//	}
}

void readWavelength(double * WaveLengthArray){
	unsigned char cmd[10];
	unsigned char response[70];
	unsigned char	ReadArray[20480];

	memset(response, 0, 70);
	memset(ReadArray, 0, 20480);
	memset(cmd, 0, 10);

	// int BytesRd = 20481;
	int BytesRd = 80;
	int AddressFLASH = 0;
	int readCycle;
	/*reading itself*/

	//calibration coefficients
	if ((BytesRd - 64 * (int)(BytesRd / 64)) == 0)
		readCycle = (int)(BytesRd / 64) - 1;
	else
		readCycle = (int)(BytesRd / 64);
	int i;
	for(i = 0; i <= readCycle; i++) {
		int addr = AddressFLASH + i * 64;
		cmd[1] = 0xA1;
		cmd[2] = (char)((addr) >> 16);
		cmd[3] = (char)((addr) >> 8);
		cmd[4] = (char)(addr);
		smpl_ReadAndWriteToDevice(response, cmd, 0);
		// if (i == 0) {
		// 	smpl_ReadAndWriteToDevice(response, cmd, 0);
		// 	smpl_ReadAndWriteToDevice(response, cmd, 0);
		// 	smpl_ReadAndWriteToDevice(response, cmd, 0);
		// }
		int j;
		for(j = 0; j < 64; j++)
			ReadArray[i * 64 + j] = response[j];
	}


	char StringTmp[16];
	for(i = 0; i < 16; i++)
		StringTmp[i] = ReadArray[1 + i];
	double A1 = atof(StringTmp);
	for(i = 0; i < 16; i++)
		StringTmp[i] = ReadArray[1 + 16 + i];
	double B1 = atof(StringTmp);
	for(i = 0; i < 16; i++)
		StringTmp[i] = ReadArray[1 + 2 * 16 + i];
	double C1 = atof(StringTmp);

	//end of reading calibration coefficients
	printf("A1= %f\n", A1);
	printf("B1= %f\n", B1);
	printf("C1= %f\n", C1);

	//end of reading calibration coefficients
	for(i = 0; i < 3653; i++)
		WavelengthArray[i] = A1 * i * i + B1 * i + C1;
}

void readSpec(int ExpN, int NScans, int Blank, signed short * rawSpec) { // 5, 1, 0
	smpl_resetAddress();

	bool Trigger = 0;
	bool KeepTrigger = 0;
	bool Fast = 0;

	//send command to get spectra to memory
	unsigned char cmd[10];
	unsigned char receivedData[70];

	memset(cmd, 0, 10);
	cmd[1] = 1;
	cmd[2] = ExpN;		//low
	// cmd[7]=ExpN>>8;	//high
	cmd[3] = NScans;	//nmbScans
	cmd[4] = Blank;		//blank scans number
	cmd[5] = 1;
	if(Trigger == 0)
		cmd[6] = 0;
	else
		if(KeepTrigger == 0)
			cmd[6] = 1;
		else
			cmd[6] = 3;

	smpl_ReadAndWriteToDevice(NULL, cmd, 0);

	//check if data is already in memory
	sleep((int)(ExpN * 2.375 * (NScans * (Blank + 1))) / 1000);

	memset(cmd,0,10); //reuse the command array

	//This will wait until the spectroscope says the data has been collected
	cmd[1] = 2;//get status
	smpl_ReadAndWriteToDevice(receivedData, cmd, 0);
	// error is discovered here. It gets all 0 because of read error.
	// printf("receivedData:%d %d %d\n", receivedData[1], receivedData[2], receivedData[3]);
	while(receivedData[3] != 0)
		smpl_ReadAndWriteToDevice(receivedData, cmd, 1);

	//read data
	smpl_resetAddress();
	int i;
	for(i = 1; i <= NScans; i++)
	{
		smpl_GetSpectra(rawSpec, 1, 0, 3652, Fast, 0, 33, 3685);

		/*OutputReport1[1]=9;//move address
			OutputReport1[2]=0x01;
			OutputReport1[3]=0x80;
			ReadAndWriteToDevice(InputReport1,OutputReport1,1);*/

		//todo normalize?
	}

	smpl_resetAddress();
}
