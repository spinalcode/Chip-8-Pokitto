// pins for keypad
DigitalOut c1 = DigitalOut(EXT3); // P0_11
DigitalOut c2 = DigitalOut(EXT5); // P0_13
DigitalOut c3 = DigitalOut(EXT7); // P0_17
DigitalOut c4 = DigitalOut(EXT9); // P0_19

DigitalIn r5 = DigitalIn(EXT11); // P1_21
DigitalIn r6 = DigitalIn(EXT13); // P1_23
DigitalIn r7 = DigitalIn(EXT15); // P1_6
DigitalIn r8 = DigitalIn(EXT17); // P1_26

void clearPins(){
    c1.write(0);
    c2.write(0);
    c3.write(0);
    c4.write(0);
}

/*
        +-+-+-+-+
        |1|2|3|C| 
        +-+-+-+-+
        |4|5|6|D| 
        +-+-+-+-+
        |7|8|9|E| 
        +-+-+-+-+
        |A|0|B|F| 
        +-+-+-+-+
*/

void updateKeys(){
    // check row 1
    clearPins();
    c1.write(1);
    key[0x1] = r5.read();
    key[0x4] = r6.read();
    key[0x7] = r7.read();
    key[0xA] = r8.read();
    
    // check row 2
    clearPins();
    c2.write(1);
    key[0x2] = r5.read();
    key[0x5] = r6.read();
    key[0x8] = r7.read();
    key[0x0] = r8.read();

    // check row 3
    clearPins();
    c3.write(1);
    key[0x3] = r5.read();
    key[0x6] = r6.read();
    key[0x9] = r7.read();
    key[0xB] = r8.read();

    // check row 3
    clearPins();
    c4.write(1);
    key[0xC] = r5.read();
    key[0xD] = r6.read();
    key[0xE] = r7.read();
    key[0xF] = r8.read();
}
