//https://en.wikipedia.org/wiki/CHIP-8
#include "Pokitto.h"
#include <File>
#include <LibAudio>
bool shouldRender = false;
long int myDelay;
int gameMode=0;
int controlMode=0;

#include "fonts.h"
#include "filestuff.h"
#include "chip8.h"
#include "keypad.h"
#include "screen.h"
#include "buttonhandling.h"

#define DEBUG 0
using PC = Pokitto::Core;
using PD = Pokitto::Display;

int fpsCount=0;
long int fpsCounter;
long int lastMillis = PC::getTime();
char tempText[64];
int speedCount=0;
uint8_t guiBG[28*22];



// create an note using one of the oscillators
auto note = Audio::Note(32) .wave(1) .duration(1000) .volume(255);


long int tempTime;

void directChar(int x, int y, const uint8_t * gfx){
    uint16_t minPal[]={0,2016};

    int i=0;
    int width = gfx[0];
    int height = gfx[1];

    auto ts = &tempSprite[0];
    for(int y1=0; y1<height; y1++){
        auto lineP = &gfx[2+width*y1];
        for(int x1=width; x1; x1--){
            *ts++ = minPal[*lineP++];
        }    
    }    
    lcdTile(x, y, x+width, y+height, tempSprite,1);
}


void setFPS(int fps){
  myDelay = 1000 / fps;
}

int frameCount=0;

void myPrint(char x, char y, const char* text){
	for (std::uint8_t index = 0, x1 = 0; index < 255; ++index, ++x1){
		if(text[index] == '\0')
			break;
			
		std::size_t glyphIndex = text[index] - 32;
        //Pokiito::Display::drawBitmap(x + (x1 * 8), y, font8x8[glyphIndex]);
        guiBG[(x + x1) + 28*y] = glyphIndex;
	}
}
void myDirectPrint(char x, char y, const char* text){
	for (std::uint8_t index = 0, x1 = 0; index < 255; ++index, ++x1){
		if(text[index] == '\0')
			break;
			
		std::size_t glyphIndex = text[index] - 32;
        directChar((x+x1)*font2[glyphIndex][0],y*font2[glyphIndex][1],font2[glyphIndex]);
	}
}

void GUIFiller(std::uint8_t* line, std::uint32_t y, bool skip){

    uint32_t x = 0;
    uint32_t width = font2[0][0];
    uint32_t height = font2[0][1];
    uint32_t mapWidth = 28;
    uint32_t numTiles = (220/width);
    if(numTiles > mapWidth) numTiles=mapWidth;
    uint32_t tileIndex = (y/height) * mapWidth;
    uint32_t jStart = 2+(y%height) * width; // current line in current tile

    uint32_t lineOffset;
    uint32_t thisTile;

    #define bgTileLine()\
        thisTile = guiBG[tileIndex++];\
        lineOffset = jStart;\
        for(uint32_t t=width; t; --t){\
            line[x++] = font2[thisTile][lineOffset++];\
            if(x>220) break;\
        }

    for(uint32_t t=numTiles; t; --t){\
        bgTileLine();
    }
    
    while(x<220){
        line[x++]=0;
    }
        
}

void myBGFiller(std::uint8_t* line, std::uint32_t y, bool skip){

    int offset = 2+y*220;
    for(int x=0; x<220;){
        line[x++]=background[offset++];
    }        

    int y1 = (y-32)/3;
    if(y1<0 || y1>31) return;
    auto lineP = &line[14];
    auto chipLine = &gfx[y1*64];
    int pixel = *chipLine;
    for(int x=64; x;){
        *lineP++ = *chipLine;
        *lineP++ = *chipLine;
        *lineP++ = *chipLine++; x--;
    }
}


void chip8Loop(){
    
    if(controlMode==0) updateKeys(); // external keypad
    if(controlMode==1) chip8_updateKeys(); // internal mapping
    
    chip8_emuCycle();
    if(delay_timer >0) delay_timer--;
    if(sound_timer >0){
        sound_timer--;
        note.play(); // Put the synth on on channel 0, play note on oscillator 0
    }else{
        Audio::stop<0>();
    }

/*
    int g = 0;
    sprintf(tempText,"%d:%d:%d:%d", key[g++],key[g++],key[g++],key[g++]); myDirectPrint(88,1,tempText);
    sprintf(tempText,"%d:%d:%d:%d", key[g++],key[g++],key[g++],key[g++]); myDirectPrint(88,2,tempText);
    sprintf(tempText,"%d:%d:%d:%d", key[g++],key[g++],key[g++],key[g++]); myDirectPrint(88,3,tempText);
    sprintf(tempText,"%d:%d:%d:%d", key[g++],key[g++],key[g++],key[g++]); myDirectPrint(88,4,tempText);
*/

    fpsCounter++;
    PC::update(1);

    while(PC::getTime()-tempTime < 1){;}
    tempTime = PC::getTime();
/*
    if(PC::getTime() >= lastMillis+1000){
        lastMillis = PC::getTime();
        fpsCount = fpsCounter;
        fpsCounter = 0;
    }
*/
}

int main(){
	PC::begin();
	PD::persistence=1;
    PD::adjustCharStep=0;
    PD::adjustLineStep=0;
    PD::fixedWidthFont = true;
    
    // This can only be done once, as were swapping layers around
    Pokitto::Display::load565Palette(background_pal);
    Pokitto::Display::lineFillers[0] = myBGFiller;
    // first update to draw the BG etc.
    PC::update();
    PC::update();

    Pokitto::Display::lineFillers[0] = GUIFiller;

    int frameCount=0;

    setFPS(60*8); // 60*8
    tempTime = PC::getTime();

    //loadProgram("/chip8/tetris.ch8");
    getDirectory("/chip8");
    //gameMode=10;
 

    gameMode = 0; // file loader
    int cursor = 0;
    int fileListStart=0;
    int fileListEnd=11;
    int fileListPos=0;

    updateButtons();
    while(_A_But[HELD]){
        updateButtons();
    }

    while (1) {
    
        updateButtons();
        int oldGameMode = gameMode;   
        switch(gameMode){
            case 0:
            
                if(_Down_But[NEW]){
                    cursor++;
                    if(cursor > fileListEnd-1) cursor = fileListEnd-1;
                }
                if(_Up_But[NEW]){
                    cursor--;
                    if(cursor < 0) cursor = 0;
                }
                
                fileListStart = cursor-5;
                if(fileListStart < 0) fileListStart = 0;
                fileListEnd = cursor+5;
                if(fileListEnd > numberOfFiles) fileListEnd = numberOfFiles;
                fileListPos = -fileListStart;
                
                if(_A_But[NEW]){
                    sprintf(tempText,"/chip8/%s",Files[cursor].name);
                    loadProgram(tempText);
                    Pokitto::Display::lineFillers[0] = myBGFiller;
                    // first update to draw the BG etc.
                    PC::update();
                    PC::update();
                    Pokitto::Display::lineFillers[0] = NULL;
                    gameMode=10;
                }
            
                for(int t=0; t<28*22; t++){
                    guiBG[t]=0;
                }
                for(int t=fileListStart; t<fileListEnd; t++){
                    myPrint(1,fileListPos+1+t,Files[t].name);
                }
                myPrint(0,fileListPos+1+cursor,">");

                PC::update();
            break;
            
            case 10:
                chip8Loop();
                if(_C_But[NEW]){gameMode=0;}
                if(_B_But[NEW]){controlMode=1-controlMode;}
            break;
        }

        if(oldGameMode != gameMode){
            if(gameMode==0){
                Pokitto::Display::lineFillers[0] = GUIFiller;
                cursor=0;
                clearScreen();
                directScreen(3);
                setWindow(0, 0, 220, 176);
                for(int t=0; t<28*22; t++){
                    guiBG[t]=0;
                }
            }
        }

    }
    return 1;
}