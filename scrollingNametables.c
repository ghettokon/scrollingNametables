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

// VRAM update buffer module
#include "vrambuf.h"
//#link "vrambuf.c"

//#define NES_MIRRORING 1

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
// Define VRAM buffer array (required by vrambuf module)
// Place this near your other global variables

// Define VRAM buffer array (required by vrambuf module)
unsigned char updbuf[128];


void update_nametable_column(unsigned int x_scroll) {
    unsigned int world_col;
    unsigned char screen_index;
    unsigned char local_col;
    const unsigned char *table;
    unsigned int base_addr;
    unsigned int ppu_addr;
    unsigned char col_slice[30];
    unsigned char row;

    world_col = (x_scroll >> 3) + 32;
    screen_index = (world_col >> 5) & 7;
    local_col = world_col & 31;
    
    table = world_nametables[screen_index];
    base_addr = (screen_index & 1) ? 0x2400 : 0x2000;
    ppu_addr = base_addr + local_col;

    for (row = 0; row < 30; ++row) {
        col_slice[row] = table[(row * 32) + local_col];
    }

    vrambuf_put(ppu_addr | VRAMBUF_VERT, col_slice, 30);
}

unsigned char need_column_update = 0;
unsigned int target_world_col = 0;

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
  
    // Initialize vrambuf system
    vrambuf_clear();
    set_vram_update(updbuf);

    // Enable rendering
    ppu_on_all();

    // Infinite loop to keep rendering active
    while (1) {
        // Advance camera position (adjust speed as needed)
        cam_x++;
        if (cam_x >= (256 * 8)) cam_x = 0;
      
        // Ensure VRAM buffer is cleared and synced at the start of the frame[cite: 4]
        ppu_wait_nmi();[cite: 4]
        vrambuf_clear();[cite: 4]
      
        // Update hardware scroll position
        scroll(cam_x, 0);

        // Queue a column update the exact moment the tile column shifts
        if ((cam_x >> 3) != (old_cam_x >> 3)) {
            update_nametable_column(cam_x);
        }
        old_cam_x = cam_x;

        // End the vrambuf queue so the NMI handler can flush it during VBLANK[cite: 4]
        vrambuf_end();
    }
}