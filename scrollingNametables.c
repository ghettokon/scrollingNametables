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

int last_screen = -1; // Tracks the last processed 256-pixel screen index

void load_screens_for_position(unsigned int x) {
    int current_left_screen;
    int upcoming_screen;
    unsigned int target_vram;
// Determine which 256-pixel screen is currently at the left edge of the viewport
    current_left_screen = x >> 8;
    
    // Check if the camera has crossed into a new screen boundary
    if (current_left_screen != last_screen) {
        last_screen = current_left_screen;
        
        // The screen entering on the right side (ahead of the camera)
        upcoming_screen = (current_left_screen + 1) & 7; // Wrap around at 8 (pic0 to pic7)
        
        // Under horizontal mirroring, determine which VRAM address ($2000 or $2400) 
        // corresponds to the off-screen nametable that just finished exiting on the left.
        // Even screens map to $2000, odd screens map to $2400.
        target_vram = (upcoming_screen & 1) ? 0x2400 : 0x2000;
        
        ppu_off(); // Safe because we only do this once per 256 pixels when a screen changes
        
        // Load the upcoming screen binary into the newly freed VRAM slot
        vram_adr(target_vram);
        vram_write(world_nametables[upcoming_screen], 960);
        
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