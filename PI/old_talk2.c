
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

char unsigned test[]  = "This";

// Sample data: Since sending 8 bits at a time, can store as chars
unsigned char receivedBuffer[DEFAULT_YUV420_SIZE];
//unsigned char sendBuffer[] = "P0Al8E3QBmCAyeEp2Uq90MUmONbL2GN51FTBuO0mB8O2Kskrp301H7M2bukm12R3nEoiMzTY5mBmimZtULMZC7W2hW4DtpFsVTVI7Hx6JR5RmmpaJ6vS57yZsJIsom5ghwRYiX6s40fyvMDe0zbHLZba9yhwZg70N4cslhi9jzZySp7to47mhRHEVksNy7ik5KTv16kSNoLS5xP4e7LWEvXrMoa4gODjBzPX1KEzc7wPbeEuBYtR16AWIjnV7HpGNktKCZjlkmUVGeyRlXj6vPsTo2xgYj24hc5nB9p9hVpBBuLi0f3YMsSLw5U5pL7REi2QuK0JE3Et34XRjlEgZrcvnkeAxKHnxXZoZ1v6CQSc72slE6i1qoRweUJoOv0wQ1lUXx8w8OJHIeLD7H5iCNmSQwF5a4SQCzVXrgWj6KctJAJJF3EPWyRA7kAMYmN0fZbwNJtkhCONACCmznTyp2u2GDVD4qSPaJaCEThH5KFQML9YuMxfKCMRVVYPj9IUJe0SrpqLStezc3YbQR6t2xZYhlOeYYMiHax71GVtwg7psApQkh6uI4qFwM3yXw75iLbONVg2hbmHy5R8Tsn2n9zkufGhS4TUEE784ZJ52ceNF1Ebgu8LCbaKPBusS20hllp2bDFfMBw0sxCsGByNTpLMls82kbBhho5f0sjkKFD6mfC0bj1PAziyFs9VMIWneBU3tqQMMWg3swDtoiurF77UOAkLMlCoRzxJLBCx4OJkSL5NhL4qDLYchfyrPo0rWYpeWnT6MgEtl4gGcKhN2xalmz1k8rsZpirjED5anKFAWmYGT45u8FHVe7ktHL2aIhG2QsYWEQckg4lZDkrZJEb7DW66kUgHe0MaPwYwuIjTPs4fU2E0bMfpiqzCLQ7ratPc2pgPlb91kTKAX9v4EZTuNwF0Q3OEui52nTNP7Kfzxg0q3wRKbMpeHAi02tS9xBy1Oypgbmzx9msFPempW0cEvYPMtgtK4prWGUxl";
unsigned char sendBuffer[DEFAULT_YUV420_SIZE];


void Receive(unsigned char* receive) {
	
	int i = 0,index = 0;
	uint32_t * addr = bcm2835_gpio + BCM2835_GPLEV0/4;
		   
	while(i <TOTAL_KB){                                                 
		while (index < DEFAULT_YUV420_SIZE) {
		//	if(DEBUG) printf("%d [waiting for data to be written...]\n", index);
			while(bcm2835_gpio_lev(READ) != HIGH);
			
			
			receive[index] = (unsigned char) (*addr >> 2);
		//	if(DEBUG) printf("[Receiving...]");
		
	//		if(DEBUG) printf("%c\n", receive[index]);
			bcm2835_gpio_write(WRITE, HIGH);
			while(bcm2835_gpio_lev(READ) != LOW);
			bcm2835_gpio_write(WRITE, LOW);
			index++;
		}
		index = 0;
		i++;
	}
}

void Send(unsigned char* send) {
	int i = 0,index = 0, len;
	uint32_t * addr = bcm2835_gpio + BCM2835_GPLEV0/4;
	//addr += 0x01;
	while(i <TOTAL_KB){  
		while (index < DEFAULT_YUV420_SIZE) {
			while(bcm2835_gpio_lev(READ) != LOW);
			len = 7;
			while(len >= 0){
				bcm2835_gpio_write(OUT[len], ((send[index] >> len) & 0x01));
			len--;
			}
	
	//*addr = send[index] << 10;	
	//	printf("2addr = %x\n",  *addr);
			bcm2835_gpio_write(WRITE, HIGH);
		//	if(DEBUG) printf("[waiting for %c to be read...]\n", send[index]);
			while(bcm2835_gpio_lev(READ) != HIGH);
			bcm2835_gpio_write(WRITE, LOW);
			
			index++;
		}
		index = 0;
		i++;
	}
}

void writeToYuv(unsigned char* received)
{
	FILE *fp;
	fp = fopen("/home/pi/Desktop/car.bin", "w+");
	fwrite(received, 1, sizeof(received), fp);
	fclose(fp);
}


int main(int argc, char **argv)
{
	
	int size = DEFAULT_YUV420_SIZE;
		char cmd[100];
	/*
	char cmd[100];
	int size = DEFAULT_YUV420_SIZE;
	
	if(argc !=2 ){
	  printf("To send a DE2, run executable with name of jpeg to send\n");
	  return 1;
 	}
	
	
	sprintf(cmd, "ls %s | jpeg2yuv -f 25 -I p | tail -n +3 > /home/pi/Desktop/result.yuv", argv[1]);
	printf("%s\n", cmd);
	//system("ls /home/pi/Desktop/sample.jpeg | jpeg2yuv -f 25 -I p | tail -n +3 > /home/pi/Desktop/result_sample.yuv");
*/
/*
	system("ls /home/pi/Desktop/sample.jpeg | /usr/local/lib/jpeg2yuv");

	return 0;
	*/
	FILE *fp;
	fp = fopen("/home/pi/Desktop/sample2.yuv", "rb");
	fread(sendBuffer, 1, size, fp);
	fclose(fp);
//	printf("%s\n", sendBuffer);
//	writeToYuv(sendBuffer);
//	return 0;
	
	if (!bcm2835_init())
	return 1;

	// Set the pin to be an input
    bcm2835_gpio_fsel(IN[0], BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(IN[1], BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(IN[2], BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(IN[3], BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(IN[4], BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(IN[5], BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(IN[6], BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(IN[7], BCM2835_GPIO_FSEL_INPT);
    
    bcm2835_gpio_fsel(READ, BCM2835_GPIO_FSEL_INPT);
    
    
    // Set the pin to be an output
    bcm2835_gpio_fsel(OUT[0], BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(OUT[1], BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(OUT[2], BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(OUT[3], BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(OUT[4], BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(OUT[5], BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(OUT[6], BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(OUT[7], BCM2835_GPIO_FSEL_OUTP);
    
    bcm2835_gpio_fsel(WRITE, BCM2835_GPIO_FSEL_OUTP);

    bcm2835_gpio_write(WRITE, LOW);


//	Send(sendBuffer);

	clock_t start , end;
	start = clock();
	Send(sendBuffer);
	printf("send done\n");
	Receive(receivedBuffer);
	//Send(sendBuffer);
	end = clock();
	//printf("receive done\n");
	printf("time taken = %.2f\n", (double)(end - start)/(double)CLOCKS_PER_SEC);
//Receive(receivedBuffer);
//	printf("send %s\n", sendBuffer);
//	printf("received %s\n", receivedBuffer);
	writeToYuv(receivedBuffer);
	bcm2835_close();
	
	return 0;
}

