//https://en.wikipedia.org/wiki/CHIP-8
#include "Pokitto.h"
#include <File>
#include <LibAudio>
bool shouldRender = false;
long int myDelay;
#include "chip8.h"
#include "keypad.h"
#include "screen.h"

#define DEBUG 0
int fpsCount=0;
long int fpsCounter;

using PC = Pokitto::Core;
using PD = Pokitto::Display;


long int tempTime;

void setFPS(int fps){
  myDelay = 1000 / fps;
}

char tempText[64];
int frameCount=0;

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




int main(){
	PC::begin();
	PD::persistence=1;
    PD::adjustCharStep=0;
    PD::adjustLineStep=0;
    PD::fixedWidthFont = true;
    
    chip8_init();
    loadProgram();
    

  // create an note using one of the oscillators
  auto note = Audio::Note(32)
                    .wave(1)
                    .duration(1000)
                    .volume(255);

    // This can only be done once, as were swapping layers around
    Pokitto::Display::lineFillers[0] = myBGFiller;

    Pokitto::Display::load565Palette(background_pal);
    int frameCount=0;

    setFPS(60); // 60*8
    tempTime = PC::getTime();

    long int lastMillis = PC::getTime();
    char tempText[64];
    int speedCount=0;
    
    // first update to draw the BG etc.
    PC::update();
    PC::update();
    PD::setTASRowMask(0b1100'00000000'00000011);
    
    myDelay /=8;
    
    while (1) {

        fpsCounter++;
        
        updateKeys(); // external keypad
        //chip8_updateKeys(); // internal mapping
        chip8_emuCycle();
        if(delay_timer >0) delay_timer--;
        if(sound_timer >0){
            sound_timer--;
            note.play(); // Put the synth on on channel 0, play note on oscillator 0
        }else{
            Audio::stop<0>();
            
        }
        PC::update(1);

        while(PC::getTime()-tempTime < myDelay){;}
        tempTime = PC::getTime();

        if(PC::getTime() >= lastMillis+1000){
            lastMillis = PC::getTime();
            fpsCount = fpsCounter;
            fpsCounter = 0;
            //sprintf(tempText,"FPS:%d\n",fpsCount);
            //PD::print(0,0,tempText);
            //PC::update();
        }

    }
    return 1;
}