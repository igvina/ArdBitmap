#ArdBitmap: Compress and draw bitmaps on the Arduboy
By @igvina
##Features:
###Bitmap library:
* Works with compressed & uncompressed bitmaps.
* Real-time image resize (downscale).
* Horizontal/Vertical mirroring (fast).
* Bitmap alignment.

###Bitmap compressor:
* Compatible with PC/MAC/Linux (made with Java).
* Good compression (better than Cabi).
* Supports PNG, GIF (also animated gifs) & JPG.
* Autogenerate Sketchs from images or animated gifs (great for no-developers).

##Video:

<a href="http://www.youtube.com/watch?feature=player_embedded&v=vtDYwqJ68gU
" target="_blank"><img src="http://img.youtube.com/vi/vtDYwqJ68gU/0.jpg" 
alt="DEMO" width="240" height="180" border="10" /></a>

##Usage:
###Compressor (v1.0):
* Syntax: java -jar compressor.jar image [-options]
	* options:
		* -gs SKETCH_FOLDER       Generate sketch code
		* -fr VALUE               Change frame rate (only animated gifs)
		* -v                      View compressed image
		* -anp PREFIX             Array name prefix
		* -ver                    Show encoder version

	* examples:

        	"java -jar compressor.jar dog.gif -gs DOG -fr 15"
        	"java -jar compressor.jar dance.png -v"

	* Notes:
		* Supports PNG, GIF (also animated gifs) & JPG
		* Max image size = 128 x 64 pixels (resized if bigger)
		* Encoding ratio could be bigger than 1 (worse than original image)

###Bitmap library (1.0.1):
* Install the ArdBitmap library in the Arduino IDE
* Add in .ino file:
	* \#include <ArdBitmap.h>
	* ArdBitmap ardbitmap (arduboy.getBuffer());
* To draw call function: ardbitmap.drawCompressed(...) , ardbitmap.drawCompressedResized(...) , ardbitmap.drawBitmap(...) , ardbitmap.drawBitmapResized(...)

####Methods:

#####Compressed images:
* void drawCompressed(int16_t sx, int16_t sy, const uint8_t *compBitmap, uint8_t color, uint8_t align, uint8_t mirror);
* void drawCompressedResized(int16_t sx, int16_t sy, const uint8_t *compBitmap, uint8_t color,uint8_t align, uint8_t mirror, float resize);

#####Uncompressed images:
* void drawBitmap(int16_t sx, int16_t sy, const uint8_t *bitmap,uint8_t w, uint8_t h, uint8_t color, uint8_t align, uint8_t mirror);
* void drawBitmapResized(int16_t sx, int16_t sy, const uint8_t *bitmap, uint8_t w,uint8_t h, uint8_t color,uint8_t align, uint8_t mirror, float resize);

####Defines:
* #define ALIGN_H_LEFT
* #define ALIGN_H_RIGHT
* #define ALIGN_H_CENTER
* #define ALIGN_V_TOP
* #define ALIGN_V_BOTTOM
* #define ALIGN_V_CENTER
* #define ALIGN_CENTER
* #define ALIGN_NONE
* #define MIRROR_NONE
* #define MIRROR_HORIZONTAL
* #define MIRROR_VERTICAL
* #define MIRROR_HOR_VER
