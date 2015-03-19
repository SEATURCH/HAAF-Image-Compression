
/*
 * untitled.c
 * 
 * Copyright 2015  <pi@raspberrypi>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


#include <stdio.h>
#include <bcm2835.h>
#include <time.h>

RPiGPIOPin IN[] = {
	RPI_BPLUS_GPIO_J8_03,
	RPI_BPLUS_GPIO_J8_05,
RPI_BPLUS_GPIO_J8_07,
RPI_BPLUS_GPIO_J8_29,
RPI_BPLUS_GPIO_J8_31,
RPI_BPLUS_GPIO_J8_26,
RPI_BPLUS_GPIO_J8_24,
RPI_BPLUS_GPIO_J8_21
};

RPiGPIOPin OUT[] = {
RPI_BPLUS_GPIO_J8_19,
RPI_BPLUS_GPIO_J8_23,
RPI_BPLUS_GPIO_J8_32,
RPI_BPLUS_GPIO_J8_33,
RPI_BPLUS_GPIO_J8_08,
RPI_BPLUS_GPIO_J8_10,
RPI_BPLUS_GPIO_J8_36,
RPI_BPLUS_GPIO_J8_11
};

#define READ RPI_BPLUS_GPIO_J8_40

#define WRITE RPI_BPLUS_GPIO_J8_38
#define DEBUG 0
#define TOTAL_KB 1
#define BUFFERSIZE 1000

#define DEFAULT_WIDTH 320
#define DEFAULT_HEIGHT 240
#define DEFAULT_YUV420_SIZE (DEFAULT_WIDTH*DEFAULT_HEIGHT*3)/2
#define FILENAME "/home/pi/Desktop/sprint1.yuv"

unsigned char receivedBuffer[DEFAULT_YUV420_SIZE];

unsigned char sendBuffer[DEFAULT_YUV420_SIZE];


void Receive(unsigned char* receive) {
	
	int index = 0;
	uint32_t * addr = bcm2835_gpio + BCM2835_GPLEV0/4;
                                             
		while (index < DEFAULT_YUV420_SIZE) {

			while(bcm2835_gpio_lev(READ) != HIGH);
			
			
			receive[index] = (unsigned char) (*addr >> 2);

			bcm2835_gpio_write(WRITE, HIGH);
			while(bcm2835_gpio_lev(READ) != LOW);
			bcm2835_gpio_write(WRITE, LOW);
			index++;
		}
}

void Send(unsigned char* send) {
	int index = 0, len;

		while (index < DEFAULT_YUV420_SIZE) {
			while(bcm2835_gpio_lev(READ) != LOW);
			len = 7;
			while(len >= 0){
				bcm2835_gpio_write(OUT[len], ((send[index] >> len) & 0x01));
			len--;
			}

			bcm2835_gpio_write(WRITE, HIGH);

			while(bcm2835_gpio_lev(READ) != HIGH);
			bcm2835_gpio_write(WRITE, LOW);
			
			index++;
		}
}

void writeToYuv(unsigned char* received, int size)
{
	FILE *fp;
	fp = fopen("/home/pi/Desktop/car.yuv", "w+");
	fwrite(received, 1, size, fp);
	fclose(fp);
}

void initialize_pins_IO(){
	int i;
	for(i = 0; i < 8; i++){
    bcm2835_gpio_fsel(IN[i], BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(OUT[i], BCM2835_GPIO_FSEL_OUTP);
	}
    
    bcm2835_gpio_fsel(READ, BCM2835_GPIO_FSEL_INPT);    
    bcm2835_gpio_fsel(WRITE, BCM2835_GPIO_FSEL_OUTP);
}
int main(int argc, char **argv)
{

	char *name = FILENAME;
	char cmd[100];
	int size = DEFAULT_YUV420_SIZE;
	
	if(argc !=2 ){
	  printf("To send a DE2, run executable with name of jpeg to send\n");
	  return 1;
 	}
	
	
	sprintf(cmd, "ls %s | jpeg2yuv -f 25 -I p | tail -n +3 > %s", argv[1], name);
	printf("%s\n", cmd);
	system(cmd);

	return 0;

	FILE *fp;
	fp = fopen(name, "rb");
	fread(sendBuffer, 1, size, fp);
	fclose(fp);

	if (!bcm2835_init()){
	
		printf("ERROR! intializing BCM2835\n");
		return 1; 

	}

    bcm2835_gpio_write(WRITE, LOW); // initial state of handshake communication protocol

	clock_t start , end;
	start = clock();
	Send(sendBuffer);
	printf("send done\n");
	Receive(receivedBuffer);
	end = clock();
	printf("receive done\n");
	printf("time taken = %.2f\n", (double)(end - start)/(double)CLOCKS_PER_SEC);

	
	writeToYuv(receivedBuffer, sizeof(receivedBuffer));
	
	bcm2835_close();
	
	return 0;
}

