# HAAF Image Compression


HAAF is an image codec based on spatial prediction with 16x16 block sizes. LZ4 is used for compression of the transform coefficients, and the transforms used are from HM's h265 encoder.

Compression Range = CompressedSize : RawSize
BMP = Bitmap, 24-bit depth

#QP Ranges from 0-51, sizes shown are for the corresponding QP's.
QP0 : Lossless Quality
QP20: Moderate Quality Loss
QP40: Heavy Quality Loss

#*** Nios II Configuration ***

LZ4 Compression Factor: 0 (lowest computation time)
LZ4 Passes: 1

#320x240, (113kB raw YUV420 input, 227kB raw BMP output):
#  Low Entropy Image:
  	lake_320x240.yuv
  		QP0 : 42kB, 5.4 : 1
  		QP20: 25kB, 9.0 : 1
  		QP40: 18kB, 12.6 : 1
  
#  Medium Entropy Image:
  	house_320x240.yuv
  		QP0 : 90kB, 2.6 : 1
  		QP20: 56kB, 4.0 : 1
  		QP40: 38kB, 6.0 : 1
  
#  High Entropy Image:
  	feather_320x240.yuv
  		QP0 : 122kB, 1.8 : 1
  		QP20: 94kB,  2.4 : 1
  		QP40: 77kB,  3.0 : 1
  
#1280x720 (1350kB raw YUV420 input, 2700kB raw BMP output):
#  High Entropy:
  	catlarge_1280x720.yuv
  		QP0 : 781kB, 3.4 : 1
  		QP20: 392kB, 6.8 : 1
  		QP40: 245kB, 11.0 : 1

#*** i7 Configuration ***
 
LZ4 Compression Factor: 25
LZ4 Passes: 2

#320x240, (113kB raw YUV420 input, 227kB raw BMP output):
#  Low Entropy Image:
  	lake_320x240.yuv
  		QP0 : 29kB, 9.6 : 1
  		QP20: 16kB, 17.3 : 1
  		QP40: 12kB, 23.1 : 1
  		
#  	octopus_320x240.yuv
  		QP0 : 28kB, 8.1 : 1
  		QP20: 14kB, 16.2 : 1
  		QP40: 10kB, 22.2 : 1
  
#  Medium Entropy Image:
  	house_320x240.yuv
  		QP0 : 59kB, 4.7 : 1
  		QP20: 33kB, 8.4 : 1
  		QP40: 22kB, 12.6 : 1
  
#  High Entropy Image:
  	feather_320x240.yuv
  		QP0 : 87kB, 3.2 : 1
  		QP20: 59kB, 4.7 : 1
  		QP40: 46kB, 6.0 : 1

#1280x720 (1350kB raw YUV420 input, 2700kB raw BMP output):
#  High Entropy:
  	catlarge_1280x720.yuv
  		QP0 : 480kB, 5.6 : 1
  		QP20: 224kB, 12.1 : 1
  		QP40: 140kB, 19.3 : 1
