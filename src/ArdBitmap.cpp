/*
   Copyright (C) 2016 Ignacio Vina (@igvina)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

// ArdBitmap: version 1.0.1

#include "ArdBitmap.h"

#define WIDTH 128
#define HEIGHT 64

ArdBitmap::ArdBitmap(unsigned char *screenBuffer) {
  sBuffer = screenBuffer;
}


////////////////////////
// COMPRESSED BITMAPS //
////////////////////////

void ArdBitmap::drawCompressed(int16_t sx, int16_t sy, const uint8_t *compBitmap, uint8_t color, uint8_t align, uint8_t mirror)
{
  //TODO: check why int16_t sizeCounter is a bit faster than uint16_t sizeCounter 
  int16_t sizeCounter;
  uint16_t len;
  int a, iCol;
  uint8_t decByte;
  uint8_t w, h;
  uint8_t col;
  boolean scanMode, scanZigZag;
  uint16_t encoderPos;
  uint8_t characterPos;


  // Read size from header (Max image size = 128 x 64)
  
  uint8_t byte0 = pgm_read_byte(&compBitmap[0]);
  uint8_t byte1 = pgm_read_byte(&compBitmap[1]);

  w = (byte0 & 0b01111111) + 1;
  h = (byte1 & 0b00111111) + 1;

  // Move positions to match alignment

  if (align & ALIGN_H_CENTER) {
    sx -= (w / 2);
  } else if (align & ALIGN_H_RIGHT) {
    sx -= w;
  }

  if (align & ALIGN_V_CENTER) {
    sy -= (h / 2);
  } else if (align & ALIGN_V_BOTTOM) {
    sy -= h;
  }

  // No need to draw at all if we're offscreen
  if (sx + w < 0 || sx > WIDTH - 1 || sy + h < 0 || sy > HEIGHT - 1)
    return;
   
  col = (byte0 >> 7) & 0x01;
  scanMode = ((byte1 >> 6) & 0x01) > 0;
  scanZigZag = ((byte1 >> 7) & 0x01) > 0;

  int yOffset = abs(sy) % 8;
  int sRow = sy / 8;
  if (sy < 0 && yOffset > 0) {
    sRow--;
    yOffset = 8 - yOffset;
  }

  uint8_t data;
  uint16_t bitmap_data;
  uint8_t mul_amt = 1 << yOffset;

  //uint16_t boffs;

  int8_t rows = h / 8;
  if (h % 8 != 0) rows++;

  // Init values
  iCol = 0;
  decByte = 0;
  encoderPos = 16;
  characterPos = 7;  
  a = 0;  

  if (mirror & MIRROR_VERTICAL) {
    a = rows - 1;
    scanMode = !scanMode;
  }

  int iColMod = (mirror & MIRROR_HORIZONTAL) ? w - 1  : 0;
  while (a < rows && a > -1) {
    
    sizeCounter = 1;
    while (((pgm_read_byte(&compBitmap[encoderPos / 8]) >> (encoderPos % 8)) & 0x01)  == 1) {
      sizeCounter ++;
      encoderPos++;
    }
    encoderPos ++;

    if (sizeCounter == 1) {
      len = 1 + ((pgm_read_byte(&compBitmap[encoderPos / 8]) >> (encoderPos % 8)) & 0x01);
      encoderPos++;
    } else {
      len = (1 << (sizeCounter - 1)) + 1 ;
   
      //TODO: check why int j is faster than uint16_t j 
      for (int j = 0; j < sizeCounter - 1; j++) {
        if (((pgm_read_byte(&compBitmap[encoderPos / 8]) >> (encoderPos % 8)) & 0x01) == 1) {
          len += (1 << j);
        }
        encoderPos++;
      }
    }

    for (uint16_t i = 0; i < len; i++)
    {

      #ifdef SPEED_HACK
      if (col == 0) {
        if (len - i > characterPos) {
          i += characterPos;
          characterPos = 0;
        } else {
          characterPos -= (len - i - 1);
          i = len;
        }
      } else if (len - i > characterPos) {
        if (characterPos == 7) {
          decByte = 0xFF;
        } else {
          decByte |= scanMode ? 0xFF >> (7 - characterPos) : (0xFF80 >> characterPos);
        }
        i += characterPos;
        characterPos = 0;
      } else {
        decByte |= scanMode ? BIT_SHIFT[characterPos] : BIT_SHIFT[7 - characterPos];
      }
      #else
      if (col) {
        decByte |= scanMode ? BIT_SHIFT[characterPos] : BIT_SHIFT[7 - characterPos];
      }
      #endif
      
      characterPos--;

      if (characterPos == 0xFF){

        //Paint decoded byte    
        int8_t bRow = sRow + a;
         
        if (decByte && bRow < (HEIGHT / 8) && iColMod + sx < WIDTH && iColMod + sx >= 0){
          
          bitmap_data = decByte * mul_amt;
          
          if (bRow >= 0) {
          
            data = sBuffer[(bRow * WIDTH) + sx + iColMod];
            if (color) {
              data |= bitmap_data & 0xFF;
            }else {
              data &= ~(bitmap_data & 0xFF);
            }
            sBuffer[(bRow * WIDTH) + sx + iColMod] = data;
          }
          
          if (yOffset && bRow < (HEIGHT / 8) - 1 && bRow > -2) {

            data = sBuffer[((bRow + 1) * WIDTH) + sx + iColMod];
            if (color) {
              data |= ((bitmap_data >> 8) & 0xFF);
            } else {
              data &= ~(((bitmap_data >> 8) & 0xFF));
            }
            sBuffer[((bRow + 1)*WIDTH) + sx + iColMod] = data;
          }
        }

        // Iterate next column-byte

        if (scanZigZag) {
          scanMode = !scanMode;
        }

        iCol++;

        if(mirror & MIRROR_HORIZONTAL){
          iColMod--;
        }else{
          iColMod++;
        }
        if (iCol >= w){
          
          iCol = 0;
          if (mirror & MIRROR_VERTICAL) {
            a--;
          } else {
            a++;
          }

          iColMod = (mirror & MIRROR_HORIZONTAL) ? w - 1  : 0;
        }

        // Reset decoded byte
        decByte = 0;
        characterPos = 7;
      }
    }

    // Toggle color for next span
    col = 1 - col; 
  }
}



void ArdBitmap::drawCompressedResized(int16_t sx, int16_t sy, const uint8_t *compBitmap, uint8_t color,uint8_t align, uint8_t mirror, float resize)
{

  //TODO: check if this can be done in a better way
  #ifdef RESIZE_HACK
  if (resize >= 1.0){
    return drawCompressed(sx, sy, compBitmap, color, align, mirror);
  }
  #else
  if (resize > 1.0){
    resize = 1.0;
  }
  #endif

  //TODO: check why int16_t sizeCounter is a bit faster than uint16_t sizeCounter 
  int16_t sizeCounter;
  uint16_t len;
  uint8_t a, iCol;
  uint8_t decByte;
  uint8_t w, wRes, h, hRes;
  uint8_t col;
  boolean scanMode, scanZigZag;
  uint16_t encoderPos;
  uint8_t characterPos;

  // Read size from header (Max image size = 128 x 64)
  
  uint8_t byte0 = pgm_read_byte(&compBitmap[0]);
  uint8_t byte1 = pgm_read_byte(&compBitmap[1]);

  w = (byte0 & 0b01111111) + 1;
  h = (byte1 & 0b00111111) + 1;

  wRes = (uint8_t)(w * resize);
  hRes = (uint8_t)(h * resize);

  if (align & ALIGN_H_CENTER) {
    sx -= (wRes / 2);
  } else if (align & ALIGN_H_RIGHT) {
    sx -= wRes;
  }

  if (align & ALIGN_V_CENTER) {
    sy -= (hRes / 2);
  } else if (align & ALIGN_V_BOTTOM) {
    sy -= hRes;
  }

  // No need to draw at all if we're offscreen
  if (sx + wRes < 0 || sx > WIDTH - 1 || sy + hRes < 0 || sy > HEIGHT - 1)
    return;

  col = (byte0 >> 7) & 0x01;
  scanMode = ((byte1 >> 6) & 0x01) > 0;
  scanZigZag = ((byte1 >> 7) & 0x01) > 0;

  int yOffset = abs(sy) % 8;
  int sRow = sy / 8;
  if (sy < 0) {
    sRow--;
    yOffset = 8 - yOffset;
  }

  uint8_t data;
  uint16_t bitmap_data;
  uint8_t mul_amt = 1 << yOffset;

  int rows = h / 8;
  if (h % 8 != 0) rows++;

  uint8_t rowsRes = hRes / 8; 
  if (hRes % 8 != 0) rowsRes++;    

  // Init values
  iCol = 0;
  decByte = 0;
  encoderPos = 16;
  characterPos = 7; 
  a = 0;  

  // Create Lookup tables to speed up drawing

  uint8_t x_LUT[w];

  for (uint8_t i=0 ; i < w; i++){
    x_LUT[i] = 0xFF;
  }
  // Precalculate column translation (0xFF if skipped)
  for (uint8_t i=0 ; i < wRes; i++){
    x_LUT[((uint16_t)i  *  w) / wRes] = (mirror & MIRROR_HORIZONTAL) ? wRes - 1 - i : i;
  }  

  uint8_t y_LUT[h];
 
  for (uint8_t i=0 ; i < h; i++){
    y_LUT[i] = 0xFF;
  }

  for (uint8_t i=0 ; i < hRes; i++){
    y_LUT[((uint16_t)i * h) / hRes] = (mirror & MIRROR_VERTICAL) ? hRes - 1 - i : i;
  }
  
  while (a < rows && /*a > -1*/ a != 0xFF) {
    
    sizeCounter = 1;
    while (((pgm_read_byte(&compBitmap[encoderPos / 8]) >> (encoderPos % 8)) & 0x01)  == 1) {
      sizeCounter ++;
      encoderPos++;
    }
    encoderPos ++;

    if (sizeCounter == 1) {
      len = 1 + ((pgm_read_byte(&compBitmap[encoderPos / 8]) >> (encoderPos % 8)) & 0x01);
      encoderPos++;
    } else {
      len = (1 << (sizeCounter - 1)) + 1 ;
   
      //TODO: check why int j is faster than uint16_t j 
      for (int j = 0; j < sizeCounter - 1; j++) {
        if (((pgm_read_byte(&compBitmap[encoderPos / 8]) >> (encoderPos % 8)) & 0x01) == 1) {
          len += (1 << j);
        }
        encoderPos++;
      }
    }

    for (uint16_t i = 0; i < len; i++)
    {

      #ifdef SPEED_HACK
      if (col == 0) {
        if (len - i > characterPos) {
          i += characterPos;
          characterPos = 0;
        } else {
          characterPos -= (len - i - 1);
          i = len;
        }
      } else if (len - i > characterPos) {
        if (characterPos == 7) {
          decByte = 0xFF;
        } else {
          decByte |= scanMode ? 0xFF >> (7 - characterPos) : (0xFF80 >> characterPos);
        }
        i += characterPos;
        characterPos = 0;
      } else {
        decByte |= scanMode ? BIT_SHIFT[characterPos] : BIT_SHIFT[7 - characterPos];
      }
      #else
      if (col) {
        decByte |= scanMode ? BIT_SHIFT[characterPos] : BIT_SHIFT[7 - characterPos];
      }
      #endif
      
      characterPos--;

      if (characterPos == 0xFF){

        //Paint decoded byte   
        int aRow8 = a * 8; 
        int16_t iColMod = x_LUT[iCol] + sx;
        
        // Skip if column not needed
        if (x_LUT[iCol] != 0xFF && iColMod < WIDTH && iColMod >= 0){
 
          for (uint8_t s = 0; s < 8 ;s++){

            if (y_LUT[aRow8+s] != 0xFF && decByte &  BIT_SHIFT[s]){ 

              //TODO: CHECK LIMITS ON LUT?
              uint8_t row = (uint8_t)(y_LUT[aRow8+s]+sy) / 8;
              
              if (row < (HEIGHT / 8)) {

                if (color) {
                  sBuffer[(row*WIDTH) + (uint8_t)iColMod] |=   BIT_SHIFT[((uint8_t)(y_LUT[aRow8+s]+sy) % 8)];
                } else {
                  sBuffer[(row*WIDTH) + (uint8_t)iColMod] &= ~ BIT_SHIFT[((uint8_t)(y_LUT[aRow8+s]+sy) % 8)];
                }
              }
              
            }
        }
      }

      // Iterate next column-byte

      if (scanZigZag) {
        scanMode = !scanMode;
      }

      iCol++;
      if (iCol >= w){
          
        iCol = 0;
        a++;
      }

      // Reset decoded byte
      decByte = 0;
      characterPos = 7;
      }
    }

    col = 1 - col; // toggle colour for next span
  }
}



//////////////////////////
// UNCOMPRESSED BITMAPS //
//////////////////////////


void ArdBitmap::drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t w, uint8_t h, uint8_t color, uint8_t align, uint8_t mirror)
{

  // Move positions to match alignment

  if (align & ALIGN_H_CENTER) {
    x -= (w / 2);
  } else if (align & ALIGN_H_RIGHT) {
    x -= w;
  }

  if (align & ALIGN_V_CENTER) {
    y -= (h / 2);
  } else if (align & ALIGN_V_BOTTOM) {
    y -= h;
  }

  // no need to draw at all of we're offscreen
  if (x + w <= 0 || x > WIDTH - 1 || y + h <= 0 || y > HEIGHT - 1)
    return;

  if (bitmap == NULL)
    return;
 
  // xOffset technically doesn't need to be 16 bit but the math operations
  // are measurably faster if it is
  uint16_t xOffset, ofs;
  int8_t yOffset = abs(y) % 8;
  int8_t sRow = y / 8;
  uint8_t loop_h, start_h, rendered_width;

  if (y < 0 && yOffset > 0) {
    sRow--;
    yOffset = 8 - yOffset;
  }

  // if the left side of the render is offscreen skip those loops
  if (x < 0) {
    xOffset = abs(x);
  } else {
    xOffset = 0;
  }

  // if the right side of the render is offscreen skip those loops
  if (x + w > WIDTH - 1) {
    rendered_width = ((WIDTH - x) - xOffset);
  } else {
    rendered_width = (w - xOffset);
  }

  // if the top side of the render is offscreen skip those loops
  if (sRow < -1) {
    start_h = abs(sRow) - 1;
  } else {
    start_h = 0;
  }

  loop_h = h / 8 + (h % 8 > 0 ? 1 : 0); // divide, then round up

  // if (sRow + loop_h - 1 > (HEIGHT/8)-1)
  if (sRow + loop_h > (HEIGHT / 8)) {
    loop_h = (HEIGHT / 8) - sRow;
  }

  // prepare variables for loops later so we can compare with 0
  // instead of comparing two variables
  loop_h -= start_h;

  sRow += start_h;
  ofs = (sRow * WIDTH) + x + xOffset;
  
  uint8_t *bofs = (uint8_t *)bitmap + (start_h * w) + xOffset;

  if (mirror & MIRROR_HORIZONTAL)  {
    bofs += rendered_width - 1;
    if (x < 0){
      bofs -= w - rendered_width;
    } else{
      bofs += w - rendered_width;
    }
  }
  
  if (mirror & MIRROR_VERTICAL) {
    bofs += (loop_h - 1) * w;
    if (y < 0){
      bofs -=  (start_h * w);
    } else {
      bofs +=  (sRow  * w);
    }
  }
  
  uint8_t data;
  uint8_t mul_amt = 1 << yOffset;
  uint16_t bitmap_data;

      // really if yOffset = 0 you have a faster case here that could be
      // optimized
      for (uint8_t a = 0; a < loop_h; a++) {
        for (uint8_t iCol = 0; iCol < rendered_width; iCol++) {
          data = pgm_read_byte(bofs);
          if(data) {
            if (mirror & MIRROR_VERTICAL){
              //reverse bits
              data = (data & 0xF0) >> 4 | (data & 0x0F) << 4;
              data = (data & 0xCC) >> 2 | (data & 0x33) << 2;
              data = (data & 0xAA) >> 1 | (data & 0x55) << 1;

              //LUT - No speed improvement and more mem
              //data = (((REVERSE_16[(data & 0x0F)]) << 4) + REVERSE_16[((data & 0xF0) >> 4)]);

              //Fast but too much mem
              //data = REVERSE_256[data];
            }
            
            bitmap_data = data * mul_amt;
            if (sRow >= 0) {
              data = sBuffer[ofs];
              if (color){
                data |= bitmap_data & 0xFF;
              } else {
                data &= ~(bitmap_data & 0xFF);
              }
              sBuffer[ofs] = data;
            }
            
            if (yOffset != 0 && sRow < 7) {
              data = sBuffer[ofs + WIDTH];
              if (color){
                data |= (bitmap_data >> 8) & 0xFF;
              } else{
                data &= ~((bitmap_data >> 8) & 0xFF); 
              }
              sBuffer[ofs + WIDTH] = data;
            }
          }
          ofs++;
          
          if (mirror & MIRROR_HORIZONTAL){
            bofs--;
          } else{
            bofs++;
          }
        }
        sRow++;
        
        if (mirror & MIRROR_HORIZONTAL){
          bofs += w + rendered_width;
        } else{
          bofs += w - rendered_width;
        }
          
        if (mirror & MIRROR_VERTICAL){
          bofs -= 2 * w;
        }
        ofs += WIDTH - rendered_width; 
      }    
}


void ArdBitmap::drawBitmapResized(int16_t sx, int16_t sy, const uint8_t *bitmap, uint8_t w,uint8_t h, uint8_t color,uint8_t align, uint8_t mirror, float resize)
{

  //TODO: check if this can be done in a better way
  #ifdef RESIZE_HACK
  if (resize >= 1.0){
    return drawBitmap(sx, sy, bitmap,w, h, color, align, mirror);
  }
  #else
  if (resize > 1.0){ 
    resize = 1.0;
  }
  #endif
  
  //TODO: check why int16_t sizeCounter is a bit faster than uint16_t sizeCounter 
  int16_t sizeCounter;
  uint16_t len;
  uint8_t a, iCol;
  uint8_t data;
  uint8_t  wRes,  hRes;
  uint8_t col;

  wRes = (uint8_t)(w * resize);
  hRes = (uint8_t)(h * resize);


  // Move positions to match alignment  
  if (align & ALIGN_H_CENTER) {
    sx -= (wRes / 2);
  } else if (align & ALIGN_H_RIGHT) {
    sx -= wRes;
  }

  if (align & ALIGN_V_CENTER) {
    sy -= (hRes / 2);
  } else if (align & ALIGN_V_BOTTOM) {
    sy -= hRes;
  }

  // No need to draw at all if we're offscreen
  if (sx + wRes < 0 || sx > WIDTH - 1 || sy + hRes < 0 || sy > HEIGHT - 1)
    return;

  int yOffset = abs(sy) % 8;
  int sRow = sy / 8;
  if (sy < 0) {
    sRow--;
    yOffset = 8 - yOffset;
  }

  int rows = h / 8;
  if (h % 8 != 0) rows++;

  uint8_t rowsRes = hRes / 8; 
  if (hRes % 8 != 0) rowsRes++;    

  // Init values
  iCol = 0;
  a = 0;  

  // Create Lookup tables to speed up drawing

  uint8_t x_LUT[w];

  for (uint8_t i=0 ; i < w; i++){
    x_LUT[i] = 0xFF;
  }
  // Precalculate column translation (0xFF if skipped)
  for (uint8_t i=0 ; i < wRes; i++){
    x_LUT[((uint16_t)i  *  w) / wRes] = (mirror & MIRROR_HORIZONTAL) ? wRes - 1 - i : i;
  }  

  uint8_t y_LUT[h];
 
  for (uint8_t i=0 ; i < h; i++){
    y_LUT[i] = 0xFF;
  }

  for (uint8_t i=0 ; i < hRes; i++){
    y_LUT[((uint16_t)i * h) / hRes] = (mirror & MIRROR_VERTICAL) ?  hRes - 1 - i : i;
  }

  len = w * rows;
  
  for (uint16_t i = 0; i < len ; i++){

    data = pgm_read_byte(&bitmap[i]);
    int aRow8 = a * 8; 
    int16_t iColMod =  x_LUT[iCol] + sx;
        
    // Skip if column not needed
    if (x_LUT[iCol] != 0xFF && iColMod < WIDTH && iColMod >= 0){
      for (uint8_t s = 0; s < 8 ;s++){
        if (y_LUT[aRow8+s] != 0xFF && data &  BIT_SHIFT[s]){ 
          //TODO: CHECK LIMITS ON LUT?
          uint8_t row = (uint8_t)(y_LUT[aRow8+s]+sy) / 8;
                   
          if (row < (HEIGHT / 8)) {           
            if (color) {
              sBuffer[(row*WIDTH) + (uint8_t)iColMod] |=   BIT_SHIFT[((uint8_t)(y_LUT[aRow8+s]+sy) % 8)];
            } else {
              sBuffer[(row*WIDTH) + (uint8_t)iColMod] &= ~ BIT_SHIFT[((uint8_t)(y_LUT[aRow8+s]+sy) % 8)];
            }    
          }
         }
      }
    }
    
    iCol++;
    if (iCol >= w){      
        iCol = 0;
        a++;
    }
  }
}

