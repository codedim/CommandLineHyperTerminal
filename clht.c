/*
**  CLHT.EXE is a Command Line Hyper Terminal tool that allows user
**  to communicate with COM ports from the Windows CLI interface.
**
**  powered by CodeDim 07/2015
*/

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* to be (1) or not to be (0) verbose to stdout */
#define VERBOSE  1     
/* number of attemts to call ReadFile() function */
#define ATTEMPTS 4
/* serial port read and write timeouts */
#define TIMEOUTS 200
/* maximal length of read & write buffers */
#define MAXLEN   100

/* program CLI help information */
#define uses "\nCLHT is a Command Line Hyper Terminal\nUses:\n\
   clht COM1             - reads data from COM1 serial port;\n\
   clht COM1 test        - writes line \"test\" to COM1;\n\
   clht COM1 \"\"        - writes to COM1 the CR symbol;\n\
   clht COM1 /f dat.cfg  - writes to COM1 lines from the\n\
                           specified configuration file,\n\
                           a line must be started with '>'\n\
                           symbol.\n"

/* if the read or write operation is OK the returned value is 0 */
int ReadSerial(HANDLE h);
int WriteSerial(HANDLE h, char buf[]);
int WriteFromFile(HANDLE h, char * fn);


int main(int argc, char* argv[]) {	
	int PORT;
	DWORD CBR = 0;
	char * pCBR;

	/* parsing CLI arguments */
	if (argc < 2 || argc > 4) {
		printf("%s", uses);
		return 1;
	} else if (strlen(argv[1]) < 4) {
		printf("%s", uses);
		return 1;
	} else if (argv[1][0] != 'C' || argv[1][1] != 'O' || argv[1][2] != 'M') {
        printf("%s", uses);
		return 1;
	} else if ( !(PORT = atoi(&argv[1][3])) || PORT > 99) {
        printf("%s", uses);
		return 1;
	} else {
		/* if baudrate was passed as for example COM1:9600 */
		if (pCBR = strstr(argv[1], ":")) {
			if ( !(CBR = strtol(++pCBR, NULL, 10)) ) {
        		printf("%s", uses);
				return 1;
			}
		}

		if (VERBOSE) {
			if (CBR) 
				printf("Connecting to COM%d on %d bps..\n", PORT, CBR);
			else
				printf("Connecting to COM%d..\n", PORT);
		}
	}

	/* determining the serial port system name */
	char sPortNumber[4] = "";
	itoa(PORT, &sPortNumber[0], 10);
	char sPortName[11] = "\\\\.\\COM";
	strcat(&sPortName[0], &sPortNumber[0]);

	/* connecting to the serial port */
	HANDLE hSerial;
	for (int i = 0; i < ATTEMPTS*4; ++i) {
		hSerial = CreateFile(&sPortName[0], GENERIC_READ | GENERIC_WRITE, 0, 
			NULL, OPEN_EXISTING, 0, NULL);
		if (hSerial == INVALID_HANDLE_VALUE) {
			printf("Error: Cannot open the port COM%s (ErrCode: %d)\r\n", 
				&sPortNumber[0], GetLastError());
			Sleep(1000);
		} else {
			break;
		}
	}

	/* setting the port up */
	if (hSerial == INVALID_HANDLE_VALUE) { 
		return -1;        
	} else {
		if (VERBOSE) printf("Setting up the port parameters...\n");

		/* setting up the port timeouts */
		COMMTIMEOUTS CommTimeOuts;
 		CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;
 		CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
 		CommTimeOuts.ReadTotalTimeoutConstant = TIMEOUTS;
 		CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
 		CommTimeOuts.WriteTotalTimeoutConstant = TIMEOUTS;
 		SetCommTimeouts(hSerial, &CommTimeOuts);

        /* setting up the port parameters (baudrate) */
		if (CBR) {
			DCB ComDCM;
			memset(&ComDCM,0,sizeof(ComDCM));
	 		ComDCM.DCBlength = sizeof(DCB);
 			GetCommState(hSerial, &ComDCM);
    	    ComDCM.BaudRate = CBR;
 			SetCommState(hSerial, &ComDCM);
		}
	}
	
	/* if there are only two args - read data from the port */
	if (argc == 2) {
		if (ReadSerial(hSerial)) {
			CloseHandle(hSerial);
			return 1;
		}
	/* if argc equ 3 - write argv[2] to the port */
	} else if (argc == 3) {
		if (WriteSerial(hSerial, argv[2])) {
			CloseHandle(hSerial);
			return 1;
		}
		if (ReadSerial(hSerial)) {
			CloseHandle(hSerial);
			return 1;
		}
	/* well argc = 4, so we have to write data to the port */
	/* from specified in argv[3] file (if argv[2] = "/F"   */
	} else if (argv[2][0] == '/' && (argv[2][1] == 'F' || argv[2][1] == 'f')) {
		if (WriteFromFile(hSerial, argv[3])) {
			CloseHandle(hSerial);
			return 1;
		}
	/* else there is an error whithin CLI arguments */
	} else {
        printf("%s", uses);
		CloseHandle(hSerial);
		return 1;
	}
	
	CloseHandle(hSerial);
	return 0;
}


int WriteSerial(HANDLE h, char buf[]) {
	DWORD writedBytes = 0;
    char * data;
	char CR = 0x0D;

	printf(">");
	for (int i = 0; i <= strlen(&buf[0]); ++i) {
		data = (i ==  strlen(&buf[0])) ? &CR : &buf[i];

		if (!WriteFile(h, data, 1, &writedBytes, NULL)) {
			printf("Error: Cannot write to the port (ErrCode: %d)\r\n", 
				GetLastError());
			return 1;
		}
		printf("%c", *data);
	}
	printf("\n");

	return 0; 
}

int ReadSerial(HANDLE h) {
	DWORD readBytes = 0;
	char buf[MAXLEN] = "";

	for (int i = 0; i < ATTEMPTS; i++) {
		if(!ReadFile(h, buf, sizeof(buf), &readBytes, NULL)){
			printf("Error: Cannot read from the port (ErrCode: %d)\r\n", 
				GetLastError());
			return 1;
		}
		if (readBytes) printf("<%s\n", buf);
	}

	return 0;
}

int WriteFromFile(HANDLE h, char * fn) {
    FILE * file; 
	char buf[MAXLEN] = "";

	if ( !(file = fopen(fn,"r")) ) {
		printf("Error: Cannot open file \"%s\" (ErrCode: %d)\r\n", fn, 
			GetLastError());
		return 1;
	}

	while (fgets(buf, sizeof(buf), file)) {
		if (strlen(buf)) {
		    /* outputs comments on stdout */
			if (buf[0] == '#') {
				printf("%s", buf);
			}
			/* sends commands to the port */
			if (buf[0] == '>') {
				while (buf[strlen(buf)-1] == 0x0D || buf[strlen(buf)-1] == 0x0A) {
					buf[strlen(buf)-1] = 0x00;
				}
				if (WriteSerial(h, &buf[1])) {
					fclose(file);
					return 1;
				}
				if (ReadSerial(h)) {
					fclose(file);
					return 1;
				}
			}
			/* this is all :) */
		}
	}

	fclose(file);
	return 0;
}


