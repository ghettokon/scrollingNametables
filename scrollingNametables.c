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

// Declare external assembly symbol exported in nametable.s
extern const unsigned char nametable_0[];
extern const unsigned char nametable_1[];
extern const unsigned char nametable_2[];
extern const unsigned char nametable_3[];
extern const unsigned char nametable_4[];
extern const unsigned char nametable_5[];
extern const unsigned char nametable_6[];
extern const unsigned char nametable_7[];

// Create an array of pointers for easy indexing
const unsigned char *const nametables[] = {
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

void load_attributes_separately(const unsigned char *tiles, const unsigned char *attributes) {
    // 1. Write tile bytes
    vram_adr(0x2000);
    vram_write(tiles, 960);

    // 2. Write attribute bytes to $23C0 (Nametable 0 attribute offset)
    vram_adr(0x23C0);
    vram_write(attributes, 64);
}

void main(void) {
    bank_bg(0);  // Backgrounds from $0000 (Font/Tiles)
    bank_spr(1); // Sprites from $1000 (Player)
    // Disable rendering while writing to PPU memory
    ppu_off();

    // Load background color palette
    pal_bg(palette);

    // Set PPU address to Nametable 0 start ($2000)
    vram_adr(0x2000);

    // Write the full 960-byte nametable array from nametable.s
    // (32 rows x 30 columns = 960 bytes)
    vram_write(nametable_0, 960);

    // Reset scroll position
    scroll(0, 0);

    // Enable rendering
    ppu_on_all();

    // Infinite loop to keep rendering active
    while (1) {
        ppu_wait_frame();
    }
}