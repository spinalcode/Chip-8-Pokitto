
/*
        +-+-+-+-+
        |0|1|2|3| 
        +-+-+-+-+
        |4|5|6|7| 
        +-+-+-+-+
        |8|9|A|B| 
        +-+-+-+-+
        |C|D|E|F| 
        +-+-+-+-+
*/

void updateKeys(){
    // redefine the pins
    // pins for keypad
    DigitalOut c1 = DigitalOut(EXT3); // P0_11
    DigitalOut c2 = DigitalOut(EXT5); // P0_13
    DigitalOut c3 = DigitalOut(EXT7); // P0_17
    DigitalOut c4 = DigitalOut(EXT9); // P0_19
    
    DigitalIn r5 = DigitalIn(EXT11); // P1_21
    DigitalIn r6 = DigitalIn(EXT13); // P1_23
    DigitalIn r7 = DigitalIn(EXT15); // P1_6
    DigitalIn r8 = DigitalIn(EXT17); // P1_26

    c1.write(0); c2.write(0); c3.write(0); c4.write(0);
    
    // check row 1
    c1.write(1);
    key[0] = r5.read();
    key[4] = r6.read();
    key[8] = r7.read();
    key[12] = r8.read();
    
    // check row 2
    c1.write(0);
    c2.write(1);
    key[1] = r5.read();
    key[5] = r6.read();
    key[9] = r7.read();
    key[13] = r8.read();

    // check row 3
    c2.write(0);
    c3.write(1);
    key[2] = r5.read();
    key[6] = r6.read();
    key[10] = r7.read();
    key[14] = r8.read();

    // check row 3
    c3.write(0);
    c4.write(1);
    key[3] = r5.read();
    key[7] = r6.read();
    key[11] = r7.read();
    key[15] = r8.read();
    c4.write(0);
}
