all: Bitstream.o CodingUnitStructure.o Encode.o ImageCompression.o lz4io.o OpenPicture.o Open_Picture.o Prediction.o Quantize.o Transform.o Utility.o bcm2835.o lz4.o lz4frame.o lz4hc.o xxhash.o
	gcc Bitstream.o CodingUnitStructure.o Encode.o ImageCompression.o lz4io.o OpenPicture.o Open_Picture.o Prediction.o Quantize.o Transform.o Utility.o bcm2835.o lz4.o lz4frame.o lz4hc.o xxhash.o -o ImageCompression
ImageCompression.o: ImageCompression.c
	gcc -c ImageCompression.c
Bitstream.o: Bitstream.c
	gcc -c Bitstream.c
CodingUnitStructure.o: CodingUnitStructure.c
	gcc -c CodingUnitStructure.c
Encode.o: Encode.c
	gcc -c Encode.c
lz4io.o: lz4io.c
	gcc -c lz4io.c
OpenPicture.o: OpenPicture.c
	gcc -c OpenPicture.c
Prediction.o: Prediction.c
	gcc -c Prediction.c
Quantize.o: Quantize.c
	gcc -c Quantize.c
Transform.o: Transform.c
	gcc -c Transform.c
Utility.o: Utility.c
	gcc -c Utility.c
lz4.o : ../include/lz4/lz4.c
	gcc -c ../include/lz4/lz4.c
lz4frame.o : ../include/lz4/lz4frame.c
	gcc -c ../include/lz4/lz4frame.c
lz4hc.o : ../include/lz4/lz4hc.c
	gcc -c ../include/lz4/lz4hc.c
xxhash.o : ../include/lz4/xxhash.c
	gcc -c ../include/lz4/xxhash.c
Open_Picture.o: Open_Picture.c
	gcc -c Open_Picture.c
bcm2835.o: /home/pi/bcm2835-1.39/src/bcm2835.c
	gcc -c /home/pi/bcm2835-1.39/src/bcm2835.c
clean:
	rm -rf *o launch
