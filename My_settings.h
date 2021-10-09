#define PROJ_SCREENMODE TASMODE // See also SECTION: TASMODE
//#define PROJ_SHOW_FPS_COUNTER
#define PROJ_FPS 255

// Choose how to use Pokitto's "extra" 4kb of RAM.
// Optional. Can be:
//   HIGH_RAM_OFF   - Needed for USB to work
//   HIGH_RAM_ON    - Simply enable the RAM hardware
//   HIGH_RAM_MUSIC - Enable and move the sound buffers to high RAM
// Default is HIGH_RAM_OFF.
#define PROJ_HIGH_RAM HIGH_RAM_OFF

// ---- SECTION: SOUND ----
#define NUM_CHANNELS 3 // Set to what you need; default is 2.
// Not strictly necessary.
#define PROJ_AUD_FREQ 8000 // Only 'cause it's used in main.cpp
// ---- SECTION: TASMODE ----
#define PROJ_MAX_SPRITES 100
#define PROJ_DEVELOPER_MODE 0
#define PROJ_LINE_FILLERS TAS::NOPFiller, TAS::NOPFiller, TAS::SpriteFiller
