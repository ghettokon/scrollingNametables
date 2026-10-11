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

// Buffer format for NESLib: [PPU_HI, PPU_LO, 30 data bytes..., 0xFF (terminator)]
// We use a size of 35 to safely fit headers, 30 bytes, and the end marker.
unsigned char vram_update_buf[40];

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

void update_nametable_column(unsigned int world_col) {
    unsigned char screen_index;
    unsigned char local_col;
    const unsigned char *table;
    unsigned int base_addr;
    unsigned int ppu_addr;
    unsigned char row;

    screen_index = world_col >> 5;  
    local_col = world_col & 31;    
    table = world_nametables[screen_index];
    
    // Under horizontal mirroring, even screens map to $2000, odd to $2400
    base_addr = (screen_index & 1) ? 0x2400 : 0x2000;
    ppu_addr = base_addr + local_col;
    
    // Native NESLib vertical buffer format:
    // [High byte of PPU address | NT_UPD_VERT], [Low byte], [Length], [Data bytes...], [NT_UPD_EOF]
    vram_update_buf[0] = (ppu_addr >> 8) | NT_UPD_VERT;
    vram_update_buf[1] = ppu_addr & 0xff;
    vram_update_buf[2] = 30; // Length of the vertical strip
    
    for (row = 0; row < 30; ++row) {
        vram_update_buf[3 + row] = table[(row * 32) + local_col];
    }
    
    vram_update_buf[33] = NT_UPD_EOF; // End of buffer marker (0xff)
}

void main(void) {
    bank_bg(0);  // Backgrounds from $0000 (Font/Tiles)
    bank_spr(1); // Sprites from $1000 (Player)
    ppu_off();
    pal_bg(palette);
  
    set_vram_update(vram_update_buf);
  
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

        // Check if the camera crossed into a new tile column (every 8 pixels)
        if ((cam_x >> 3) != (old_cam_x >> 3)) {
            unsigned int new_col = (cam_x >> 3) + 32; // Look at the column just entering the right edge
            update_nametable_column(new_col);
            // Determine which nametable and local column this falls into
            // (A full column update function writes 30 bytes vertically to VRAM during VBLANK)
        }
      	
      	old_cam_x = cam_x;
    }
}