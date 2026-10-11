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

void load_nametable(unsigned char index) {
    ppu_off();
    vram_adr(0x2000);
    vram_write(world_nametables[index], 960);
    ppu_on_all();
}

void load_attributes_separately(const unsigned char *tiles, const unsigned char *attributes) {
    // 1. Write tile bytes
    vram_adr(0x2000);
    vram_write(tiles, 960);

    // 2. Write attribute bytes to $23C0 (Nametable 0 attribute offset)
    vram_adr(0x23C0);
    vram_write(attributes, 64);
}

// Given a world column coordinate (0 to 255), get the correct tile from the 8 screens
unsigned char get_world_tile(unsigned int world_col, unsigned char row) {
    unsigned char screen_index = world_col >> 5; // Divide by 32 columns per screen
    unsigned char local_col = world_col & 31;   // Modulo 32
    
    // Each nametable is 960 bytes (32 rows x 30 columns)
    const unsigned char *table = world_nametables[screen_index];
    return table[(row * 32) + local_col];
}

void update_nametable_column(unsigned int world_col) {
    unsigned char screen_index = world_col >> 5;
    unsigned char local_col = world_col & 31;
    const unsigned char *table = world_nametables[screen_index];
    
    // Calculate PPU destination address based on column
    // (Alternates between $2000 and $2400 depending on horizontal mirroring and nametable boundary)
    unsigned int ppu_addr = 0x2000 + (local_col & 31); // Simplified base calculation
    
    vram_adr(ppu_addr);
    // Write the column vertically (30 rows)
    // Note: Writing vertical columns row-by-row requires a VRAM step setting or individual writes if step is 32.
}

void main(void) {
    bank_bg(0);  // Backgrounds from $0000 (Font/Tiles)
    bank_spr(1); // Sprites from $1000 (Player)
    ppu_off();
    pal_bg(palette);
  
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

        // Set hardware scroll position
        scroll(cam_x, 0);

        // Check if the camera crossed into a new tile column (every 8 pixels)
        if ((cam_x >> 3) != (old_cam_x >> 3)) {
            unsigned int new_col = (cam_x >> 3) + 32; // Look at the column just entering the right edge
            
            // Determine which nametable and local column this falls into
            // (A full column update function writes 30 bytes vertically to VRAM during VBLANK)
        }

        old_cam_x = cam_x;
        ppu_wait_frame();
    }
}