#ArdBitmaplib: Compress and draw bitmaps on the Arduboy
By @igvina
##Usage:
- Compressor (v1.0):
Syntax: java -jar compressor.jar image [-options]

	-options:
         	-gs SKETCH_FOLDER       Generate sketch code
        	-fr VALUE               Change Frame rate (only animated gifs)
        	-v                      View compressed image
        	-anp PREFIX             Array name prefix
        	-ver                    Show encoder version


	-examples:
        	java -jar compressor.jar dog.gif -gs DOG -fr 15
        	java -jar compressor.jar dance.png -v

	+Notes: 
		- Supports JPG, PNG, GIF (also animated gifs)
		- Max image size = 128 x 64 pixels (resized if bigger)
		- Encoding ratio could be bigger than 1 (worse than original image) 
	

- Library (1.0.0):

	+Copy lib ( ArdBitmaplib.h and ArdBitmaplib.cpp) to your project folder
	+Add in .ino file:
		#include "ArdBitmaplib.h"
		ArdBitmaplib ardbitmap (arduboy);
	+ To draw call function: ardbitmap.drawCompressed (...) , ardbitmap.drawCompressedResized(...) , ardbitmap.drawBitmap (...) , drawBitmapResized(...)


	//Compressed images
    	void drawCompressed(int16_t sx, int16_t sy, const uint8_t *compBitmap, uint8_t color, uint8_t align, uint8_t mirror);   
    	void drawCompressedResized(int16_t sx, int16_t sy, const uint8_t *compBitmap, uint8_t color,uint8_t align, uint8_t mirror, float resize);
	
	//Uncompressed images
    	void drawBitmap(int16_t sx, int16_t sy, const uint8_t *bitmap,uint8_t w, uint8_t h, uint8_t color, uint8_t align, uint8_t mirror);
    	void drawBitmapResized(int16_t sx, int16_t sy, const uint8_t *bitmap, uint8_t w,uint8_t h, uint8_t color,uint8_t align, uint8_t mirror, float resize);

		
	\#define ALIGN_H_LEFT    0b00000000
	\#define ALIGN_H_RIGHT   0b00000001
	\#define ALIGN_H_CENTER  0b00000010
	\#define ALIGN_V_TOP     0b00000000
	\#define ALIGN_V_BOTTOM  0b00000100
	\#define ALIGN_V_CENTER  0b00001000
	\#define ALIGN_CENTER    0b00001010
	\#define ALIGN_NONE      0b00000000

	\#define MIRROR_NONE       0b00000000
	\#define MIRROR_HORIZONTAL 0b00000001
	\#define MIRROR_VERTICAL   0b00000010
	\#define MIRROR_HOR_VER    0b00000011
