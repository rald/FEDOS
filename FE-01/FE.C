#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <conio.h>
#include <dos.h>
#include <limits.h>
#include <sys/nearptr.h>


#include "font.h"
#include "bicycle.h"


#define VIDEO_INT           0x10  /* the BIOS video interrupt. */
#define WRITE_DOT           0x0C  /* BIOS func to plot a pixel. */
#define SET_MODE            0x00  /* BIOS func to set the video mode. */
#define VGA_256_COLOR_MODE  0x13  /* use to set 256-color mode. */
#define TEXT_MODE           0x03  /* use to set 80x25 text mode. */

#define PALETTE_INDEX       0x03C8
#define PALETTE_DATA        0x03C9
#define INPUT_STATUS        0x03DA
#define VRETRACE            0x08

#define SCREEN_WIDTH        320   /* width in pixels of mode 0x13 */
#define SCREEN_HEIGHT       200   /* height in pixels of mode 0x13 */
#define NUM_COLORS          256   /* number of colors in mode 0x13 */


#define DELAY_MAX 10
#define LINE_MAX 1024
#define FRAME_MAX 256


#define VKEY_BSP   8
#define VKEY_ENT  13
#define VKEY_ESC  27
#define VKEY_SPC  32
#define VKEY_LF  331
#define VKEY_RT  333
#define VKEY_DN  336
#define VKEY_UP  328
#define VKEY_DOT '.'
#define VKEY_CMA ','
#define VKEY_S   'S'
#define VKEY_s   's'




typedef unsigned char  byte;           
typedef unsigned short word;

byte *VGA = (byte *)0xA0000;      /* this points to video memory. */
word *my_clock = (word *)0x046C;  /* this points to the 18.2hz system clock. */

byte dbuf[SCREEN_WIDTH*SCREEN_HEIGHT];



void SetMode(byte mode);
void VWait(void);
void Flip(void);

void ClearScreen(byte color);

void DrawPoint(int x,int y,byte color);
void DrawRect(int x,int y,int w,int h,byte color);
void FillRect(int x,int y,int w,int h,byte color);
void FillCircle(int x,int y,int r,byte color);
void DrawCircle(int x0,int y0,int radius,byte color);

void DrawChar(int font[],int x,int y,byte fg,byte bg,byte ch);
void DrawText(int font[],int x,int y,byte fg,byte bg,const char *fmt,...);

void DrawGrid(int b[],int x,int y,int w,int h,int sz,byte fg,byte bg,byte c,int f);

void SaveFont(char *name,int b[],int w,int h,int f);

char *strupr(char *s);
char *strlwr(char *s);



int main(void) {

  bool quit=false;

  int key;

  int w=font_width,h=font_height;
  int f=65,sz=10;

  int b[w*h*FRAME_MAX];

  int cx=0,cy=0;

  int i,j,k;


  if (__djgpp_nearptr_enable() == 0) {
    printf("Could get access to first 640K of memory.\n");
    return 1;
  }

  VGA+=__djgpp_conventional_base;



  srand(time(NULL));                 /* seed the number generator. */



  SetMode(VGA_256_COLOR_MODE);       /* set the video mode. */



  outp(0x03C8,0);
  for(i=0;i<bicycle_num_colors;i++) {
    outp(0x03C9,(double)bicycle_palette[i*3+0]/255*63);
    outp(0x03C9,(double)bicycle_palette[i*3+1]/255*63);
    outp(0x03C9,(double)bicycle_palette[i*3+2]/255*63);
  }
  for(i=0;i<NUM_COLORS-bicycle_num_colors;i++) {
    outp(0x03C9,63);
    outp(0x03C9,63);
    outp(0x03C9,63);
  }  



  for(k=0;k<256;k++) {
    for(j=0;j<8;j++) {
      for(i=0;i<8;i++) {
        b[8*j+i+k*w*h]=font_pixels[8*j+i+k*w*h];
      }
    }
  }


  
  ClearScreen(0);

  DrawGrid(b,0,0,w,h,sz,3,0,1,f);
  DrawRect(cx*sz,cy*sz,sz,sz,2);
  DrawText(font_pixels,2,SCREEN_HEIGHT-8-2,3,0,"%03d",f);
  DrawText(font_pixels,2+8*4,SCREEN_HEIGHT-8-2,3,0,"%02d:%02d",cx,cy);


  for(i=0;i<256;i++) {
    DrawChar(b,i%16*8+10*9,i/16*8+1,3,0,i);
  }
  DrawRect(f%16*8+10*9-1,f/16*8,10,10,2);



  VWait();
  Flip();

  while(!quit) {

    if(kbhit()) {
      key=toupper(getch());
      if(key==0) key=getch()+256;
      switch(key) {
        case VKEY_ESC: quit=true; break;
        case VKEY_CMA:
          DrawRect(f%16*8+10*9-1,f/16*8,10,10,0);

          for(j=-1;j<=1;j++)
            for(i=-1;i<=1;i++)
              if( f%16+i>=0 && f%16+i<=15 &&
                  f/16+j>=0 && f/16+j<=15) {
                DrawChar(b,(f%16+i)*8+10*9,(f/16+j)*8+1,3,0,16*j+i+f);
              }

          if(f>0) f--; else f=255;
          DrawGrid(b,0,0,w,h,sz,3,0,1,f);
          DrawRect(cx*sz,cy*sz,sz,sz,2);
          DrawRect(f%16*8+10*9-1,f/16*8,10,10,2);
          DrawText(font_pixels,2,SCREEN_HEIGHT-8-2,3,0,"%03d",f);

          break;
        case VKEY_DOT:
          DrawRect(f%16*8+10*9-1,f/16*8,10,10,0);

          for(j=-1;j<=1;j++)
            for(i=-1;i<=1;i++)
              if( f%16+i>=0 && f%16+i<=15 &&
                  f/16+j>=0 && f/16+j<=15) {
                DrawChar(b,(f%16+i)*8+10*9,(f/16+j)*8+1,3,0,16*j+i+f);
              }

          if(f<255) f++; else f=0;
          DrawGrid(b,0,0,w,h,sz,3,0,1,f);
          DrawRect(cx*sz,cy*sz,sz,sz,2);
          DrawRect(f%16*8+10*9-1,f/16*8,10,10,2);
          DrawText(font_pixels,2,SCREEN_HEIGHT-8-2,3,0,"%03d",f);

          break;
        case VKEY_LF:
          DrawRect(cx*sz,cy*sz,sz,sz,1);
          if(cx>0) cx--; else cx=w-1;
          DrawRect(cx*sz,cy*sz,sz,sz,2);
          DrawText(font_pixels,2+8*4,SCREEN_HEIGHT-8-2,3,0,"%02d:%02d",cx,cy);
        break;
        case VKEY_UP:
          DrawRect(cx*sz,cy*sz,sz,sz,1);
          if(cy>0) cy--; else cy=h-1;
          DrawRect(cx*sz,cy*sz,sz,sz,2);
          DrawText(font_pixels,2+8*4,SCREEN_HEIGHT-8-2,3,0,"%02d:%02d",cx,cy);
        break;
        case VKEY_DN:
          DrawRect(cx*sz,cy*sz,sz,sz,1);
          if(cy<h-1) cy++; else cy=0;
          DrawRect(cx*sz,cy*sz,sz,sz,2);
          DrawText(font_pixels,2+8*4,SCREEN_HEIGHT-8-2,3,0,"%02d:%02d",cx,cy);
        break;
        case VKEY_RT:
          DrawRect(cx*sz,cy*sz,sz,sz,1);
          if(cx<w-1) cx++; else cx=0;
          DrawRect(cx*sz,cy*sz,sz,sz,2);
          DrawText(font_pixels,2+8*4,SCREEN_HEIGHT-8-2,3,0,"%02d:%02d",cx,cy);
        break;
        case VKEY_SPC:
          k=8*cy+cx+f*w*h;
          if(b[k]!=0) {
            b[k]=0;
            FillRect(cx*sz,cy*sz,sz,sz,0);
          } else {
            b[k]=1;
            FillRect(cx*sz,cy*sz,sz,sz,3);
          }
          DrawRect(cx*sz,cy*sz,sz,sz,2);
          DrawChar(b,f%16*8+10*9,f/16*8+1,3,0,f);
        break;
        case VKEY_S:
        case VKEY_s:
          SaveFont("font",b,w,h,256);
        break;
        default: break;
      }
    }

    VWait();
    Flip();
    
  }

  SetMode(TEXT_MODE);

  __djgpp_nearptr_disable();

  return 0;
}



double drand() {
  return rand()/(RAND_MAX+1.0);
}



void SetMode(byte mode) {
  union REGS regs;

  regs.h.ah = SET_MODE;
  regs.h.al = mode;
  int86(VIDEO_INT, &regs, &regs);
}



void VWait(void) {
  /* wait until done with vertical retrace */
  while  ((inp(INPUT_STATUS) & VRETRACE)) {};
  /* wait until done refreshing */
  while (!(inp(INPUT_STATUS) & VRETRACE)) {};
}



void ClearScreen(byte color) {
  memset(dbuf,color,SCREEN_WIDTH*SCREEN_HEIGHT);
}



void Flip(void) {
  memcpy(VGA,dbuf,SCREEN_WIDTH*SCREEN_HEIGHT);
}



void DrawPoint(int x,int y,byte color) {
  if(x>=0 && x<SCREEN_WIDTH && y>=0 && y<SCREEN_HEIGHT) {
    dbuf[y*SCREEN_WIDTH+x]=color;
  }
}



void DrawRect(int x,int y,int w,int h,byte color) {
  int i,j;
  for(i=0;i<w;i++) {
    DrawPoint(x+i,y,color);
    DrawPoint(x+i,y+h-1,color);
  }
  for(j=0;j<h;j++) {
    DrawPoint(x,y+j,color);
    DrawPoint(x+w-1,y+j,color);
  }
}



void FillRect(int x,int y,int w,int h,byte color) {
  int i,j;
  for(j=0;j<h;j++) {
    for(i=0;i<w;i++) {
      DrawPoint(x+i,y+j,color);
    }
  }
}



void DrawCircle(int x0, int y0, int radius, byte color) {
  int f = 1 - radius;
  int ddF_x = 0;
  int ddF_y = -2 * radius;
  int x = 0;
  int y = radius;

  DrawPoint(x0, y0 + radius, color);
  DrawPoint(x0, y0 - radius, color);
  DrawPoint(x0 + radius, y0, color);
  DrawPoint(x0 - radius, y0, color);

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x + 1;
    DrawPoint(x0 + x, y0 + y, color);
    DrawPoint(x0 - x, y0 + y, color);
    DrawPoint(x0 + x, y0 - y, color);
    DrawPoint(x0 - x, y0 - y, color);
    DrawPoint(x0 + y, y0 + x, color);
    DrawPoint(x0 - y, y0 + x, color);
    DrawPoint(x0 + y, y0 - x, color);
    DrawPoint(x0 - y, y0 - x, color);
  }
}



void FillCircle(int x0,int y0,int r,byte color) {
  double x,y;
  double dx,dy;
  double d;
  
  for(y=0;y<r*2;y++) {
    for(x=0;x<r*2;x++) {
      dx=r-x;
      dy=r-y;
      d=sqrt(dx*dx+dy*dy);
  
      // Point lies outside of the circle
      if(d-r>1) continue;
  
      // Edge threshold
      if(r/d<1) continue;
      
      DrawPoint(x+x0-r,y+y0-r,color);
    }
  }

}
 


void DrawChar(int font[],int x,int y,byte fg,byte bg,byte ch) {
  int i,j,k;
  for(j=0;j<8;j++) {
    for(i=0;i<8;i++) {
      k=font[64*ch+8*j+i];
      if(k!=0) {
        DrawPoint(x+i,y+j,fg);
      } else {
        DrawPoint(x+i,y+j,bg);
      }
    }
  }
}



void DrawText(int font[],int x,int y,byte fg,byte bg,const char *fmt,...) {
  int i=x,j=y,k=0;

  va_list args;
  
  char *buf=NULL;
  int blen=0;
  
  va_start(args,fmt);
    blen=vsnprintf(NULL,0,fmt,args);
  va_end(args);
  
  buf=calloc(blen+1,sizeof(*buf));
  
  va_start(args,fmt);
    vsnprintf(buf,blen+1,fmt,args);
  va_end(args);
  
  while(buf[k]) {
    DrawChar(font,i,j,fg,bg,buf[k]);
    i+=8;
    if(i+8>SCREEN_WIDTH) {
      i=0;
      j+=8;
      if(j+8>SCREEN_HEIGHT) break;
    }
    k++;
  }
}



void DrawGrid(int b[],int x,int y,int w,int h,int sz,byte fg,byte bg,byte c,int f) {
  int i,j;
  for(j=0;j<h;j++) {
    for(i=0;i<w;i++) {
      FillRect(sz*i+x,sz*j+y,sz,sz,b[8*j+i+f*w*h]?fg:bg);
      DrawRect(sz*i+x,sz*j+y,sz,sz,c);
    }
  }
}



void SaveFont(char *name,int b[],int w,int h,int f) {

  FILE *fout=NULL;

  char path[PATH_MAX];

  int i,j,k;

  snprintf(path,PATH_MAX,"%s.H",strupr(name));
  
  fout=fopen(path,"wt");

  fprintf(fout,"#ifndef %s_H\n",strupr(name));
  fprintf(fout,"#define %s_H\n\n",strupr(name));
  fprintf(fout,"int %s_frames=%d;\n\n",strlwr(name),f);
  fprintf(fout,"int %s_width=%d;\n",strlwr(name),w);
  fprintf(fout,"int %s_height=%d;\n\n",strlwr(name),h);
  fprintf(fout,"int %s_pixels[] = {\n\n",strlwr(name));

  for(k=0;k<f;k++) {
    fprintf(fout,"/* %d */\n\n",k);
    for(j=0;j<8;j++) {
      fprintf(fout,"  ");
      for(i=0;i<8;i++) {
        fprintf(fout,"%d,",b[8*j+i+k*w*h]?1:0);
      }
      fprintf(fout,"\n");
    }
    fprintf(fout,"\n");
  }
  fprintf(fout,"};\n\n");

  fprintf(fout,"#endif\n\n\n");


  fclose(fout);

}


char *strupr(char *s) {
  char *p=s;
  while(*p) {
    *p=toupper(*p);
    p++;
  }
  return s;
}



char *strlwr(char *s) {
  char *p=s;
  while(*p) {
    *p=tolower(*p);
    p++;
  }
  return s;
}


