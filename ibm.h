unsigned char ibmDemo[132] = {
	0x00, 0xE0, // Clear Screen
	0xA2, 0x2A, // Set Index to 0x22A (554)
	0x60, 0x0C, // Set V[0] to 0x0C (b1100)
	0x61, 0x08, // Set V[1] to 0x08 (b1000)
	0xD0, 0x1F, // Draw Sprite at V[0],V[1], width (always) 8, height 16, from I (currently 0x22A (554))
	0x70, 0x09, // V[X] = 0x09
	0xA2, 0x39, // Set Index to 0x239 (569)
	0xD0, 0x1F, // Draw sprite at V[0],V[1], height 16
	0xA2, 0x48, // Set Index
	0x70, 0x08, // V[X] = 0x08
	0xD0, 0x1F, // Draw sprite at V[0],V[1], height 16
	0x70, 0x04, // V[X] = 0x04
	0xA2, 0x57, // Set Index
	0xD0, 0x1F, // Draw sprite at V[0],V[1], height 16
	0x70, 0x08, // V[X] = 0x08
	0xA2, 0x66, // Set Index
	0xD0, 0x1F, // Draw sprite at V[0],V[1], height 16
	0x70, 0x08, // V[X] = 0x08
	0xA2, 0x75, // Set Index
	0xD0, 0x1F, // Draw sprite at V[0],V[1], height 16
	0x12, 0x28, // Gosub 0x282
	0xFF, 0x00,
	0xFF, 0x00,
	0x3C, 0x00,
	0x3C, 0x00,
	0x3C, 0x00,
	0x3C, 0x00,
	0xFF, 0x00,
	0xFF, 0xFF,
	0x00, 0xFF,
	0x00, 0x38,
	0x00, 0x3F,
	0x00, 0x3F,
	0x00, 0x38,
	0x00, 0xFF,
	0x00, 0xFF,
	0x80, 0x00,
	0xE0, 0x00,
	0xE0, 0x00,
	0x80, 0x00,
	0x80, 0x00,
	0xE0, 0x00,
	0xE0, 0x00,
	0x80, 0xF8,
	0x00, 0xFC,
	0x00, 0x3E,
	0x00, 0x3F,
	0x00, 0x3B,
	0x00, 0x39,
	0x00, 0xF8,
	0x00, 0xF8,
	0x03, 0x00,
	0x07, 0x00,
	0x0F, 0x00,
	0xBF, 0x00,
	0xFB, 0x00,
	0xF3, 0x00,
	0xE3, 0x00,
	0x43, 0xE0,
	0x00, 0xE0,
	0x00, 0x80,
	0x00, 0x80,
	0x00, 0x80,
	0x00, 0x80,
	0x00, 0xE0,
	0x00, 0xE0
};
