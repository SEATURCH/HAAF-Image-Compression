
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

#define FILE_YUV "/home/pi/Desktop/sprint2.yuv"
#define FILE_TMP "/home/pi/Desktop/tmp.yuv"
#define FILE_W "/home/pi/Desktop/w.txt"
#define FILE_H "/home/pi/Desktop/h.txt"

//unsigned char receivedBuffer[DEFAULT_YUV420_SIZE];

//unsigned char sendBuffer[DEFAULT_YUV420_SIZE];


void Receive(unsigned char* receive, int image_size) {
	
	int index = 0;
	uint32_t * addr = bcm2835_gpio + BCM2835_GPLEV0/4;
                                             
		while (index < image_size) {

			while(bcm2835_gpio_lev(READ) != HIGH);
			
			
			receive[index] = (unsigned char) (*addr >> 2);

			bcm2835_gpio_write(WRITE, HIGH);
			while(bcm2835_gpio_lev(READ) != LOW);
			bcm2835_gpio_write(WRITE, LOW);
			index++;
		}
}

void Send(unsigned char* send, int image_size) {
	int index = 0, len;

		while (index < image_size) {
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

void writeToYuv(unsigned char* received, int image_size)
{
	FILE *fp;
	fp = fopen("/home/pi/Desktop/sprint2_output.yuv", "w+");
	fwrite(received, 1, image_size, fp);
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
	FILE *fp;
	char *fileOUT = FILE_YUV;
	char cmd[100];
	char fileIN[100];
	int W, H, image_size;

	if(!(fp = popen("zenity --title=\"Please select 'jpeg' image\" --file-selection --file-filter=\"\"*.jpg\" \"*.jpeg\"\" | tr \'\\n\' \'\\0\'", "r")))
	{
		return 0;
	}

	while(fgets(fileIN, sizeof(fileIN), fp)!=NULL){
	//	printf("%s", fileIN);
	}
	pclose(fp);

	sprintf(cmd, "ls %s | jpeg2yuv -f 25 -I p > %s", fileIN, FILE_TMP);
	// printf("%s\n", cmd);
	system(cmd);

	sprintf(cmd, "cat %s | tail -n +3 > %s", FILE_TMP, FILE_YUV);
	// printf("%s\n", cmd);
	system(cmd);
	
	sprintf(cmd, "head -1 %s | grep -o '[^C][0-9][0-9][0-9]' | grep -o '[0-9][0-9][0-9]' | head -1 > %s", FILE_TMP, FILE_W);
	// printf("%s\n", cmd);
	system(cmd);

	sprintf(cmd, "head -1 %s | grep -o '[^C][0-9][0-9][0-9]' | grep -o '[0-9][0-9][0-9]' | tail -n +2 > %s", FILE_TMP, FILE_H);
	// printf("%s\n", cmd);
	system(cmd);

	if(!(fp = fopen(FILE_W, "r"))){
		return 0;
	}
	
	fscanf(fp, "%i", &W);
	// printf("Width  = %i", W);
	fclose(fp);

	if(!(fp = fopen(FILE_H, "r"))){
		return 0;
	}
	
	fscanf(fp, "%i", &H);
	// printf("Height  = %i", H);
	fclose(fp);
	
	image_size = (W*H*3)/2;
	unsigned char receivedBuffer[image_size];
  unsigned char sendBuffer[image_size];

	FILE *fp;
	fp = fopen(name, "rb");
	fread(sendBuffer, 1, image_size, fp);
	fclose(fp);

	if (!bcm2835_init()){
	
		printf("ERROR! intializing BCM2835\n");
		return 1; 

	}

    bcm2835_gpio_write(WRITE, LOW); // initial state of handshake communication protocol

	clock_t start , end;
	start = clock();
	Send(sendBuffer, image_size);
	printf("send done\n");
	Receive(receivedBuffer, image_size);
	end = clock();
	printf("receive done\n");
	printf("time taken = %.2f\n", (double)(end - start)/(double)CLOCKS_PER_SEC);

	
	writeToYuv(receivedBuffer, sizeof(receivedBuffer));
	
	bcm2835_close();

  return 0;
}
