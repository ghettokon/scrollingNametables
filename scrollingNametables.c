//#resource "pic0.nam"
//#resource "pic1.nam"
//#resource "pic2.nam"
//#resource "pic3.nam"
//#resource "pic4.nam"
//#resource "pic5.nam"
//#resource "pic6.nam"
//#resource "pic7.nam"

//#link "nametable_binary.s"
//#link "chr_generic.s"

#include <nes.h>
#include "neslib.h"

#define NES_MIRRORING 1

// Declare all 8 external nametables from your assembly file
extern const unsigned char nametable_0[];
extern const unsigned char nametable_1[];
extern const unsigned char nametable_2[];
extern const unsigned char nametable_3[];
extern const unsigned char nametable_4[];
extern const unsigned char nametable_5[];
extern const unsigned char nametable_6[];
extern const unsigned char nametable_7[];

const unsigned char *const world_nametables[] = {
    nametable_0, nametable_1, nametable_2, nametable_3,
    nametable_4, nametable_5, nametable_6, nametable_7
};

// Sample palette (Background palette 0)
const unsigned char palette[16] = {
    0x0f, 0x00, 0x10, 0x30, // Black background, Dark Gray, Light Gray, White
    0x0f, 0x00, 0x10, 0x30,
    0x0f, 0x00, 0x10, 0x30,
    0x0f, 0x00, 0x10, 0x30
};

unsigned int cam_x = 0;
unsigned int old_cam_x = 0;
unsigned char current_pair = 255; // Force initial load

void load_screens_for_position(unsigned int x) {
    // Determine which screen pair (0 to 3) we are currently viewing
    // Each screen is 256 pixels wide. Pair 0 = screens 0-1, Pair 1 = screens 2-3, etc.
    unsigned char pair_index = (x >> 8) >> 1; 
    
    if (pair_index != current_pair) {
        current_pair = pair_index;
        
        ppu_off(); // Safe because we only do this when changing screen pairs
        
        // Load left nametable ($2000)
        vram_adr(0x2000);
        vram_write(world_nametables[current_pair * 2], 960);
        
        // Load right nametable ($2400)
        vram_adr(0x2400);
        vram_write(world_nametables[(current_pair * 2) + 1], 960);
        
        ppu_on_all();
    }
}

void main(void) {
    bank_bg(0);  // Backgrounds from $0000 (Font/Tiles)
    bank_spr(1); // Sprites from $1000 (Player)
    
    ppu_off();
    pal_bg(palette);
  
    //set_vram_update(vram_update_buf);
  
    vram_adr(0x2000);
    vram_write(nametable_0, 960);
    vram_adr(0x2400);
    vram_write(nametable_1, 960);

    // Enable rendering
    ppu_on_all();

    // Infinite loop to keep rendering active
    while (1) {
        // Advance camera position (adjust speed as needed)
        cam_x++;
        if (cam_x >= (256 * 8)) cam_x = 0; // Loop across all 8 screens (2048 pixels total)

        ppu_wait_frame();
      
        // Set hardware scroll position
        scroll(cam_x, 0);

        load_screens_for_position(cam_x);
      	
      	old_cam_x = cam_x;
    }
}