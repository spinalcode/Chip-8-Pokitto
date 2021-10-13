#define offsetX 14
#define offsetY 32
#define timer_multiplier 8
/*
     _   _            _       _     _           
    | | | |          (_)     | |   | |          
    | | | | __ _ _ __ _  __ _| |__ | | ___  ___ 
    | | | |/ _` | '__| |/ _` | '_ \| |/ _ \/ __|
    \ \_/ / (_| | |  | | (_| | |_) | |  __/\__ \
     \___/ \__,_|_|  |_|\__,_|_.__/|_|\___||___/
*/

const uint8_t chip8_fontset[80] = { 
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

// 16bit current opcode storage
uint16_t opcode;
// 4kb of RAM
uint8_t memory[4096];
// There are 16 registers
uint8_t V[16];
// 16bit Address register
uint16_t I;
// 16bit Program Counter
uint16_t programCounter;

/* Chip-8 memory map
0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
0x200-0xFFF - Program ROM and work RAM
*/

// Chip-8 'screen' is 64x32 1bpp, all gfx are xor'd to this array
unsigned char gfx[64 * 32];

// Two 'hardware' timers, count down when >0
uint32_t delay_timer;
// the sound timer causes a beep when not zero
uint32_t sound_timer;

// the stack on the chip-8 only has 16 levels
unsigned short stack[16];
// the stack pointer is the current position in the stack
unsigned short stackPointer;

// there are 16 buttons on a chip-8
unsigned char key[16];

/*
     _____                _   _                 
    |  ___|              | | (_)                
    | |_ _   _ _ __   ___| |_ _  ___  _ __  ___ 
    |  _| | | | '_ \ / __| __| |/ _ \| '_ \/ __|
    | | | |_| | | | | (__| |_| | (_) | | | \__ \
    \_| \___,_|_| |_|\___|\__|_|\___/|_| |_|___/

*/

void clearScreen(){
    // clear the screen
    for(int t=0; t<2048; t++){
        gfx[t]=0;
    }
}

void chip8_init(){
    // Initialize registers and memory once
    clearScreen();
    for(int t=0; t<16; t++){
        stack[t]=0;
    }
    stackPointer=0;
    for(int t=0; t<4096; t++){
        memory[t]=0;
    }
    for(int t=0; t<16; t++){
        V[t]=0;
    }
    programCounter=0x200; // programs should be loaded to 0x200 (512)
    I=0;
    opcode=0;
    // font is loaded to 0x50 onwards ??? - can't be correct
    for(int t=0; t<80; t++){
        memory[t] = chip8_fontset[t];
    }
}

void loadProgram(char * fileName){
    // programs should be loaded at 0x200 (512)
    // simply copy the 'ROM' to that location
    chip8_init();
    
    File ch8File;
    if(ch8File.openRO(fileName)){
        int fileSize = ch8File.size();
        // check if fileSize + 512 > 4095
        
        ch8File.read(&memory[512], fileSize);
    }

}

void setup_dat(uint16_t data)
{
    SET_MASK_P2;
    LPC_GPIO_PORT->MPIN[2] = (data<<3); // write bits to port
    CLR_MASK_P2;
}

void cmd(uint16_t data)
{
    CLR_CS; // select lcd
    CLR_CD; // clear CD = command
    SET_RD; // RD high, do not read
    setup_dat(data); // function that inputs the data into the relevant bus lines
    CLR_WR_SLOW;  // WR low
    SET_WR;  // WR low, then high = write strobe
    SET_CS; // de-select lcd
}
void dat(uint16_t data)
{
    CLR_CS;
    SET_CD;
    SET_RD;
    setup_dat(data);
    CLR_WR;
    SET_WR;
    SET_CS;
}
void setWindow(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    /*
    x1+=offsetX;
    y1+=offsetY;
    x2+=offsetX;
    y2+=offsetY;
    */
    cmd(0x37); dat(y1);
    cmd(0x36); dat(y2-1);
    cmd(0x39); dat(x1);
    cmd(0x38); dat(x2-1);
    cmd(0x20); dat(y1);
    cmd(0x21); dat(x1);
}

void lcdTile(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t* gfx, int scale = 1){
    int height = y1-y0;
    int width = x1-x0;
    x0*=scale; x1*=scale; y0*=scale; y1*=scale;
    setWindow(offsetX+x0, offsetY+y0, offsetX+x1, offsetY+y1);
    cmd(0x22);
    int i=0;
    for (int y=0; y<height; y++) {
        for(int w=0; w<scale; w++){
            for(int x=0; x<width; x++){
                for(int z = 0; z<scale; z++){
                    dat(gfx[i]);
                }
                i++;
            }
            i-=width;
        }
        i+=width;
    }
}


uint16_t tempSprite[64*32]; // max sprite size
// direct sprite test
void directSprite(int x, int y, int h){
    uint16_t minPal[]={0,2016};
    // get bg from x,y,8,h
    int i=0;
    int width = 8;
    while(x+width>64)width--;

    if(y+h > 32) h=y-32;

    auto ts = &tempSprite[0];
    for(int y1=0; y1<h; y1++){
        auto lineP = &gfx[1+x+64*(y+y1)];
        for(int x1=width; x1; x1--){
            *ts++ = minPal[*lineP++];
        }    
    }    
    lcdTile(x, y, x+width, y+h, tempSprite,3);
}

void directScreen(int scale){
    int numPix = (64*scale)*(32*scale);
    setWindow(offsetX, offsetY, offsetX+64*scale, offsetY+32*scale);
    cmd(0x22);
    for (int x=0; x<numPix; x++) {
        dat(0);
    }
    setWindow(0, 0, 220, 176);
}


void chip8_emuCycle(){

    int pixPos;
    // Fetch opcode
    // read 16bits from memory at location pc
    opcode = memory[programCounter++] << 8 | memory[programCounter++];

    // Decode Opcode
    // 0xF000
    //   BXYN
    //    NNN
    //     NN
    int B = (opcode >> 12) & 0xF;
    int X = (opcode >> 8) & 0xF;
    int Y = (opcode >> 4) & 0xF;
    int N = opcode & 0xF;
    int NN = opcode & 0xFF;
    int NNN = opcode & 0xFFF;

    bool foundInstruction = false;
    //myDelay = 0; // estimated time of instruction
    switch(B){
        case 0x0: // either clear screen or return from sub...
            if(X==0x0){
                if(Y==0xE){
                    if(N==0x0){
                        clearScreen();
                        directScreen(3);
                        shouldRender = true;
                    }else if(N==0xE){
                        //Return from subroutine...
                        //stackPointer--;
                        programCounter = stack[--stackPointer];
                    }
                }
            }
        break;

        case 0x1: // Gogo NNN
            programCounter = NNN;
        break;

        case 0x2: // Call routine at NNN
            stack[stackPointer++] = programCounter;
            //stackPointer++;
            programCounter = NNN;
        break;

        case 0x3: // skip next instruction if V[X] == NN
            if(V[X]==NN){
                programCounter+=2;
            }
        break;

        case 0x4: // skip next instruction if V[X] != NN
            if(V[X]!=NN){
                programCounter+=2;
            }
        break;

        case 0x5: // skip next instruction if V[X] == V[Y]
            if(V[X]==V[Y]){
                programCounter+=2;
            }
        break;

        case 0x6: // V[X] = NN
            V[X] = NN;
        break;

        case 0x7: // V[X] += NN
            //myDelay = 45;
            V[X] += NN;
        break;

        case 0x8: // 
            if(N==0x0){ // V[X] = V[Y]
                V[X] = V[Y];
            }else
            if(N==0x1){ // V[X] |= V[Y] (or)
                V[X] |= V[Y];
            }else
            if(N==0x2){ // V[X] &= V[Y] (and)
                V[X] &= V[Y];
            }else
            if(N==0x3){ // V[X] ^= V[Y] (xor)
                V[X] ^= V[Y];
            }else
            if(N==0x4){ // 8XY4: Adds V[Y] to V[X]. V[F] is set to 1 when there's a carry, and to 0 when there isn't
                if(V[X] + V[Y] >= 256){
                    V[0xF] = 1;
                }else{
                    V[0xF] = 0;
                }
                V[X] += V[Y];
            }else
            if(N==0x5){ // V[X] -= V[Y] (subtract) V[F] is set to 0 when there's a borrow, and 1 when there isn't
                if(V[X]-V[Y] >= 0){
                    V[0xF]=1;
                }else{
                    V[0xF]=0;
                }
                V[X] -= V[Y];
            }else
            if(N==0x6){ // V[X] >> 1 (right shift) V[F] is set to the value of the least significant bit of VX before the shift
                V[0xf] = V[X] & 1;
                V[X] = V[X] >> 1;
            }else
            if(N==0x7){ // V[Y] - V[X] VF is set to 0 when there's a borrow, and 1 when there is not.
                if(V[X]-V[Y]>=0){
                    V[0xF]=0;
                }else{
                    V[0xF]=1;
                }
                V[X] = V[Y] - V[X];
            }else
            if(N==0xE){ // V[X] << 1 Stores the most significant bit of VX in VF and then shifts VX to the left by 1.
                V[0xF] = V[X] >> 7; // msb into V[15]
                V[X] = V[X] << 1; // << 1
            }
        break;

        case 0x9: // Skips the next instruction if VX does not equal VY. (Usually the next instruction is a jump to skip a code block);
            if(V[X]!=V[Y]){
                programCounter+=2;
            }
        break;

        case 0xA: // ANNN: Sets I to the address NNN
            I = NNN;
        break;

        case 0xB: // BNNN: Goto NNN (PC = V0 + NNN)
            programCounter = V[0] + NNN;
        break;

        case 0xC: // CXNN: X = random() & NN
            V[X] = random(255) & NN;
        break;

        // 0xD is Draw stuff
        case 0xD: // Draw Sprite X,Y,N
            // sprite is 8xN in size
            // the sprite data starts a I
            V[0xF]=0; // reset collision detection
            pixPos = V[X]+64*V[Y];
            for(int y1=0; y1<N; y1++){
                uint8_t row = memory[I+(y1%64)];
                for(int x1=0; x1<8; x1++){
                    uint8_t pixel = (row >> (7-(x1%32)))&1;
                    pixPos++;
                    if(pixel==1){
                        gfx[pixPos] ^= 1;
                        if(gfx[pixPos]==0) V[0xF] = 1;
                    }
                }
                pixPos+=56;
            }
            directSprite(V[X], V[Y], N);
            shouldRender = true;
        break;

        // 0xE is some keyboard stuff
        case 0xE:
            if(NN==0x9E){ // EX9E = Skip next instruction if Key in V[X] is pressed
                if(key[V[X]]){
                    programCounter +=2;
                }
            }
            if(NN==0xA1){ // EXA1 = Skip next instruction if Key in V[X] is NOT pressed
                if(!key[V[X]]){
                    programCounter +=2;
                }
            }
        break;

        case 0xF:
            if(NN==0x07){ // FX07 = getDelay()
                V[X] = delay_timer / timer_multiplier;
            }else
            if(NN==0x0A){ // FX0A = getKey() - BLOCKING?
                for(int t=0; t<16; t++){
                    if(key[t]==1) V[X] = t;
                }
            }else
            if(NN==0x15){ // FX15 = setDelay(X)
                delay_timer = V[X] * timer_multiplier;
            }else
            if(NN==0x18){ // FX18 = setSoundTime(X)
                sound_timer = V[X] * timer_multiplier;
            }else
            if(NN==0x1E){ // FX1E = I += V[X]
                I += V[X];
            }else
            if(NN==0x29){ // FX29 = I = sprite_addr[Vx]
                I = V[X] * 5; // 5 bytes per character
            }else
            if(NN==0x33){ // FX33 = I = BCD(V[X]) Binary Coded Decimal
                uint8_t digit = V[X];
                memory[I+2] = digit % 10;
                digit /=10;
                memory[I+1] = digit % 10;
                digit /=10;
                memory[I] = digit % 10;
            }else
            if(NN==0x55){ // FX55 = mem copy V[0]-V[X] to I to I+X
                for(int t=0; t<X+1; t++){
                    memory[I++] = V[t];
                }
            }else
            if(NN==0x65){ // FX65 = reverse of 55
                for (int t=0; t<X+1; t++) {   // X is inclusive
                    V[t] = memory[I++];
                }
            }
        break;

        default:
          break;
    } // switch

}

void chip8_updateKeys(){
    // clear the keypad
    for(int t=0; t<16; t++){
        key[t]=0;
    }
    
    // Read the button states outside of the update() function
    //from minilib by FManga
    bool ABtn = *((volatile char*)(0xA0000000 + 1*0x20 + 9));
    bool BBtn = *((volatile char*)(0xA0000000 + 1*0x20 + 4));
    bool CBtn = *((volatile char*)(0xA0000000 + 1*0x20 + 10));
    bool UBtn = *((volatile char*)(0xA0000000 + 1*0x20 + 13));
    bool DBtn = *((volatile char*)(0xA0000000 + 1*0x20 + 3));
    bool LBtn = *((volatile char*)(0xA0000000 + 1*0x20 + 25));
    bool RBtn = *((volatile char*)(0xA0000000 + 1*0x20 + 7));

    if(UBtn){ key[2] = 1; }
    if(LBtn){ key[4] = 1; }
    if(RBtn){ key[6] = 1; }
    if(DBtn){ key[8] = 1; }
    if(ABtn){ key[5] = 1; }
    if(BBtn){ key[5] = 1; }

}

