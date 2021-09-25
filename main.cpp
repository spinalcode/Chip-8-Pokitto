//https://en.wikipedia.org/wiki/CHIP-8
#include "Pokitto.h"
#include "test.h"

using PC = Pokitto::Core;
using PD = Pokitto::Display;


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
unsigned char delay_timer;
// the sound timer causes a beep when not zero
unsigned char sound_timer;

// the stack on the chip-8 only has 16 levels
unsigned short stack[16];
// the stack pointer is the current position in the stack
unsigned short stackPointer;

// there are 16 buttons on a chip-8
unsigned char key[16];

void clearScreen(){
    // clear the screen
    for(int t=0; t<2048; t++){
        gfx[t]=0;
    }
}

void renderScreen(){
    int x=0;
    int y=0;
    for(int t=0; t<2048; t++){
        PD::drawPixel(x,y,gfx[t]);
        x++;
        if(x==64){
            x=0;
            y++;
        }
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

void loadProgram(){
    
    // programs should be loaded at 0x200 (512)
    // simply copy the 'ROM' to that location
    for(int i = 0; i < sizeof(testROM); ++i){
        memory[i + 512] = testROM[i];
    }
}
 
void chip8_emuCycle(){

    // Fetch opcode
    // read 16bits from memory at location pc
    opcode = memory[programCounter] << 8 | memory[programCounter + 1];

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
    switch(B){
        case 0x0: // either clear screen or return from sub...
            if(X==0x0){
                if(Y==0xE){
                    if(N==0x0){
                        foundInstruction = true;
                        // clear the screen
                        clearScreen();
                        // increase the program counter by 2 bytes
                        programCounter +=2;
                    }else if(N==0xE){
                        //Return from subroutine...
                        programCounter = stack[stackPointer];
                        programCounter +=2;
                        stackPointer--;
                    }
                }
            }
        break;

        case 0x1: // Gogo NNN
            foundInstruction = true;
            programCounter = NNN;
        break;

        case 0x2: // Call routine at NNN
            foundInstruction = true;
            stack[stackPointer++] = programCounter;
            programCounter = NNN;
        break;

        case 0x3: // skip next instruction if V[X] == NN
            foundInstruction = true;
            if(V[X]==NN){
                programCounter+=4;
            }else{
                programCounter+=2;
            }
        break;

        case 0x4: // skip next instruction if V[X] != NN
            foundInstruction = true;
            if(V[X]!=NN){
                programCounter+=4;
            }else{
                programCounter+=2;
            }
        break;

        case 0x5: // skip next instruction if V[X] == V[Y]
            foundInstruction = true;
            if(V[X]==V[Y]){
                programCounter+=4;
            }else{
                programCounter+=2;
            }
        break;

        case 0x6: // V[X] = NN
            foundInstruction = true;
            V[X] = NN;
            programCounter+=2;
        break;

        case 0x7: // V[X] += NN
            foundInstruction = true;
            V[X] += NN;
            programCounter+=2;
        break;

        case 0x8: // 
            if(N==0x0){ // V[X] = V[Y]
                V[X] = V[Y];
                programCounter+=2;
            }
            if(N==0x1){ // V[X] |= V[Y] (or)
                V[X] |= V[Y];
                programCounter+=2;
            }
            if(N==0x2){ // V[X] &= V[Y] (and)
                V[X] &= V[Y];
                programCounter+=2;
            }
            if(N==0x3){ // V[X] ^= V[Y] (xor)
                V[X] ^= V[Y];
                programCounter+=2;
            }
            if(N==0x4){ // 8XY4: Adds V[Y] to V[X]. V[F] is set to 1 when there's a carry, and to 0 when there isn't
                if(V[X] + V[Y] >= 256){
                    V[0xF] = 1;
                }else{
                    V[0xF] = 0;
                }
                V[X] += V[Y];
                programCounter+=2;
            }
            if(N==0x5){ // V[X] -= V[Y] (subtract) V[F] is set to 0 when there's a borrow, and 1 when there isn't
                if(V[X]-V[Y] >= 0){
                    V[0xF]=1;
                }else{
                    V[0xF]=0;
                }
                V[X] -= V[Y];
                programCounter+=2;
            }
            if(N==0x6){ // V[X] >> 1 (right shift) V[F] is set to the value of the least significant bit of VX before the shift
                V[0xf] = V[X] & 1;
                V[X] = V[X] >> 1;
                programCounter+=2;
            }
            if(N==0x7){ // V[Y] - V[X] VF is set to 0 when there's a borrow, and 1 when there is not.
                if(V[Y]-V[X]<0){
                    V[0xF]=1;
                }else{
                    V[0xF]=0;
                }
                V[X] = V[Y] - V[X];
                programCounter+=2;
            }
            if(N==0xE){ // V[X] << 1 Stores the most significant bit of VX in VF and then shifts VX to the left by 1.
                V[0xF] = V[X] >> 7;
                V[X] = V[X] << 1;
                programCounter+=2;
            }
        break;

        case 0x9: // Skips the next instruction if VX does not equal VY. (Usually the next instruction is a jump to skip a code block);
            foundInstruction = true;
            if(V[X]!=V[Y]){
                programCounter+=4;
            }else{
                programCounter+=2;
            }
        break;

        case 0xA: // ANNN: Sets I to the address NNN
            I = NNN;
            programCounter += 2;
        break;

        case 0xB: // BNNN: Goto NNN (PC = V0 + NNN)
            programCounter = V[0] + NNN;
        break;

        case 0xC: // CXNN: X = random() & NN
            V[X] = random(255) & NN;
            programCounter += 2;
        break;

        // 0xD is Draw stuff
        case 0xD: // Draw Sprite X,Y,N
            // sprite is 8xN in size
            // the sprite data starts a I
            V[0xF]=0; // reset collision detection
            for(int y1=0; y1<N; y1++){
                uint8_t row = memory[I+y1];
                for(int x1=0; x1<8; x1++){
                    uint8_t pixel = (row >> (7-x1))&1;
                    int pixPos = (V[X]+x1)+64*(V[Y]+y1);
                    if(pixel==1){
                        gfx[pixPos] ^= 1;
                        if(gfx[pixPos]==0) V[0xF] = 1;
                    }
                    
                }
            }
            programCounter += 2;
        break;

        // 0xE is some keyboard stuff
        case 0xE:
            if(NN==0x9E){ // EX9E = Skip next instruction if Key in V[X] is pressed
                if(key[V[X]]){
                    programCounter +=4;
                }else {
                    programCounter +=2;
                }
            }
            if(NN==0xA1){ // EXA1 = Skip next instruction if Key in V[X] is NOT pressed
                if(!key[V[X]]){
                    programCounter +=4;
                }else {
                    programCounter +=2;
                }
            }
        break;

        case 0xF:
            if(NN==0x07){ // FX07 = getDelay()
                V[X] = delay_timer;
                programCounter +=2;
            }
            if(NN==0x0A){ // FX07 = getKey() - BLOCKING
                // ???
                programCounter +=2;
            }
            if(NN==0x15){ // FX15 = setDelay(X)
                delay_timer = V[X];
                programCounter +=2;
            }
            if(NN==0x18){ // FX18 = setSoundTime(X)
                sound_timer = V[X];
                programCounter +=2;
            }
            if(NN==0x1E){ // FX1E = I += V[X]
                I += V[X];
                programCounter +=2;
            }
            if(NN==0x29){ // FX29 = I = sprite_addr[Vx]
                I += V[X] * 5; // 5 bytes per character
                programCounter +=2;
            }
            if(NN==0x33){ // FX33 = I = BCD(V[X]) Binary Coded Decimal
                // Stores the binary-coded decimal representation of VX,
                // with the most significant of three digits at the address
                // in I, the middle digit at I plus 1, and the least significant
                // digit at I plus 2. (In other words, take the decimal representation
                // of VX, place the hundreds digit in memory at location in I, the tens
                // digit at location I+1, and the ones digit at location I+2.);
                memory[I] = V[X]/100;
                memory[I+1] = V[X]%10;
                memory[I+2] = V[X]/10;
                programCounter +=2;
            }
            if(NN==0x55){ // FX55 = mem copy V[0]-V[X] to I to I+X
                for(int t=0; t<X; t++){
                    memory[I+t] = V[t];
                }
                programCounter +=2;
            }
            if(NN==0x65){ // FX55 = reverse of 55
                for(int t=0; t<X; t++){
                    V[t] = memory[I+t];
                }
                programCounter +=2;
            }
        break;



        default:
          //PD::print("Unknown opcode: 0x%X\n", opcode);
          break;
    } // switch

    if(delay_timer >0)delay_timer--;
    if(sound_timer >0)sound_timer--;
    
}

int main(){
	PC::begin();
	PD::persistence=1;

    chip8_init();
    loadProgram();

    while (1) {
        
        if (PC::update()) {
            chip8_emuCycle();
            renderScreen();
        }
        
/*    
        if(Pokitto::Buttons::aBtn()){
        }

        PD::drawBitmap(rand() % (PD::width - 32), rand() % (PD::height - 32), Smile);
*/       
    }
    return 1;
}