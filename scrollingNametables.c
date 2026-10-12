//#resource "pic0.nam"
//#resource "pic1.nam"
//#resource "pic2.nam"
//#resource "pic3.nam"
//#resource "pic4.nam"
//#resource "pic5.nam"
//#resource "pic6.nam"
//#resource "pic7.nam"
//#resource "my.chr"

//#link "nametable_binary.s"
//#link "chr_generic.s"

#include "neslib.h"
#include "vrambuf.h" // VRAM update buffer module
//#link "vrambuf.c"

#define NES_MIRRORING 1

// Physics Constants
#define FLOOR_Y     180
#define PLAYER_X    32
#define GRAVITY     1
#define JUMP_FORCE -6
unsigned char gravity_timer = 0;

// Declare all 8 external nametables from your assembly file
extern const unsigned char nametable_0[];
extern const unsigned char nametable_1[];
extern const unsigned char nametable_2[];
extern const unsigned char nametable_3[];
extern const unsigned char nametable_4[];
extern const unsigned char nametable_5[];
extern const unsigned char nametable_6[];
extern const unsigned char nametable_7[];

const unsigned char * const world_nametables[8] = {
    nametable_0, nametable_1, nametable_2, nametable_3,
    nametable_4, nametable_5, nametable_6, nametable_7
};

// Sample palette (Background palette)
const unsigned char bg_palette[16] = {
    0x0f, 0x25, 0x2C, 0x30, // background colors
    0x0f, 0x00, 0x10, 0x30, 
    0x0f, 0x0A, 0x1A, 0x2A,
    0x0f, 0x00, 0x10, 0x30
};
// Sample palette (Sprite palette)
const unsigned char sprite_palette[16] = {
    0x0F, 0x15, 0x23, 0x37, // hero colors
    0x0F, 0x0F, 0x0F, 0x0F,
    0x0F, 0x0F, 0x0F, 0x0F,
    0x0F, 0x0F, 0x0F, 0x0F
};

const unsigned char metasprite_run1[] = {
    0,  0, 0x6C, 0, // Top-Left tile
    8,  0, 0x6D, 0, // Top-Right tile
    0,  8, 0x7C, 0, // Bottom-Left tile
    8,  8, 0x7D, 0, // Bottom-Right tile
    128
};

const unsigned char metasprite_run2[] = {
    0,  0, 0x6E, 0,
    8,  0, 0x6F, 0,
    0,  8, 0x7E, 0,
    8,  8, 0x7F, 0,
    128
};

const unsigned char metasprite_run3[] = {
    0,  0, 0x8C, 0,
    8,  0, 0x8D, 0,
    0,  8, 0x9C, 0,
    8,  8, 0x9D, 0,
    128
};

const unsigned char metasprite_jump[] = {
    0,  0, 0x8E, 0,
    8,  0, 0x8F, 0,
    0,  8, 0x9E, 0,
    8,  8, 0x9F, 0,
    128
};


const unsigned char metasprite_run1_L[] = {
    0,  0, 0x6D, 0x40, // Top-Left tile
    8,  0, 0x6C, 0x40, // Top-Right tile
    0,  8, 0x7D, 0x40, // Bottom-Left tile
    8,  8, 0x7C, 0x40, // Bottom-Right tile
    128
};

const unsigned char metasprite_run2_L[] = {
    0,  0, 0x6F, 0x40,
    8,  0, 0x6E, 0x40,
    0,  8, 0x7F, 0x40,
    8,  8, 0x7E, 0x40,
    128
};

const unsigned char metasprite_run3_L[] = {
    0,  0, 0x8D, 0x40,
    8,  0, 0x8C, 0x40,
    0,  8, 0x9D, 0x40,
    8,  8, 0x9C, 0x40,
    128
};

const unsigned char metasprite_jump_L[] = {
    0,  0, 0x8F, 0x40,
    8,  0, 0x8E, 0x40,
    0,  8, 0x9F, 0x40,
    8,  8, 0x9E, 0x40,
    128
};

// Running animation frame table
const unsigned char* const run_animation_R[] = {
    metasprite_run1,
    metasprite_run2,
    metasprite_run3
};

const unsigned char* const run_animation_L[] = {
    metasprite_run1_L,
    metasprite_run2_L,
    metasprite_run3_L
};

// -----------------------------------------------------------------------------
// PLAYER STATE VARIABLES
// -----------------------------------------------------------------------------
unsigned char player_y = FLOOR_Y;
signed char player_vy = 0;
unsigned char is_grounded = 1;

unsigned char anim_timer = 0;
unsigned char anim_frame = 0;

// Facing direction: 0 = Right (default), 1 = Left
unsigned char facing_left = 0;

// Track button press trigger specifically for jumping
unsigned char pad_trig = 0;

// -----------------------------------------------------------------------------
// FUNCTIONS
// -----------------------------------------------------------------------------

void update_player(unsigned char pad) {
    // Jump trigger on A button
    if ((pad & PAD_A) && is_grounded) {
        player_vy = JUMP_FORCE;
        is_grounded = 0;
    }

    // Apply Velocity & Gravity
    player_y += player_vy;
    // Apply gravity every 2 frames (effectively 0.5 gravity)
    gravity_timer++;
    if (gravity_timer >= 3) {
        player_vy += GRAVITY;
        gravity_timer = 0;
    }

    // Floor Boundary
    if (player_y >= FLOOR_Y) {
        player_y = FLOOR_Y;
        player_vy = 0;
        is_grounded = 1;
    }
}

void update_animation(unsigned char pad) {
// Only animate if on the ground AND holding Left or Right
    if (is_grounded && (pad & (PAD_LEFT | PAD_RIGHT))) {
        anim_timer++;
        if (anim_timer >= 6) { // Advance frame every 6 ticks (~10 FPS)
            anim_timer = 0;
            anim_frame = (anim_frame + 1) % 2; // Toggle frame 0 and 1
        }
    } else {
        // Reset to standing frame and reset timer when stationary or idle
        anim_frame = 0;
        anim_timer = 0;
    }
}

void draw_player(void) {
    const unsigned char* current_metasprite;

    if (facing_left) {
        if (!is_grounded) {
            current_metasprite = metasprite_jump_L;
        } else {
            current_metasprite = run_animation_L[anim_frame];
        }
    } else {
        if (!is_grounded) {
            current_metasprite = metasprite_jump;
        } else {
            current_metasprite = run_animation_R[anim_frame];
        }
    }

    // Render 2x2 MetaSprite to OAM
    oam_meta_spr(PLAYER_X, player_y, 0, current_metasprite);
}

unsigned int cam_x = 0;
unsigned int old_cam_x = 0;
unsigned char moving = 0;

void update_nametable_column(unsigned int x_scroll, int direction) {
    // Declare all variables at the top (CC65 C89 requirement)
    unsigned int world_col;
    unsigned char screen_index;
    unsigned char local_col;
    const unsigned char *table;
    unsigned int base_addr;
    unsigned int ppu_addr;
    unsigned char col_slice[30];
    unsigned char row;

    // Determine which column to draw based on scroll direction
    if (direction > 0) {
        // Moving right: draw 32 tiles ahead on the right edge
        world_col = (x_scroll >> 3) + 32;
    } else {
        // Moving left: draw at the current left edge of the screen
        world_col = x_scroll >> 3;
    }

    screen_index = (world_col >> 5) & 7;
    local_col = world_col & 31;
    
    table = world_nametables[screen_index];
    base_addr = (screen_index & 1) ? 0x2400 : 0x2000;
    ppu_addr = base_addr + local_col;

    for (row = 0; row < 30; ++row) {
        col_slice[row] = table[(row * 32) + local_col];
    }

    // Queue the vertical column write and terminate stream
    vrambuf_put(ppu_addr | VRAMBUF_VERT, col_slice, 30);
    vrambuf_end();
}

void main(void) {
    // Declare all variables at the top of the function
    unsigned char pad;
    unsigned int cam_x = 0;
    unsigned int old_cam_x = 0;
    int scroll_dir = 0;
    unsigned char pad_prev = 0;
  
    bank_spr(0); // Sprites from $1000 (Player)
    bank_bg(1);  // Backgrounds from $0000 (Font/Tiles)
    
    ppu_off();
    pal_spr(sprite_palette);
    pal_bg(bg_palette);
  
    vram_adr(0x2000);
    vram_write(nametable_0, 1024);
    vram_adr(0x2400);
    vram_write(nametable_1, 1024);
  
    // Initialize vrambuf system
    vrambuf_clear();
    set_vram_update(updbuf);

    // Enable rendering
    ppu_on_all();

    // Infinite loop to keep rendering active
    while (1) {
        ppu_wait_nmi();
        vrambuf_clear();

        pad = pad_state(0);
      
      	// Compute button triggers (newly pressed buttons this frame)
        pad_trig = pad & ~pad_prev;
        pad_prev = pad;
      
      	pad = pad_poll(0);

        // Determine scroll direction from D-pad
        if (pad & PAD_RIGHT) {
            scroll_dir = 1;
            facing_left = 0;
        } else if (pad & PAD_LEFT) {
            scroll_dir = -1;
            facing_left = 1;
        } else {
            scroll_dir = 0;
        }
      
      	update_player(pad);
        update_animation(pad);

        // Clear sprite hardware memory and re-draw
        oam_clear();
        draw_player();

        // Apply movement and update columns when moving
        if (scroll_dir != 0) {
            if (scroll_dir > 0) {
                cam_x++;
                if (cam_x >= (256 * 8)) cam_x = 0;
            } else {
                if (cam_x > 0) cam_x--;
                else cam_x = (256 * 8) - 1;
            }

            scroll(cam_x, 0);

            if ((cam_x >> 3) != (old_cam_x >> 3)) {
                update_nametable_column(cam_x, scroll_dir);
            }
            old_cam_x = cam_x;
        } 	
  	vrambuf_end();
    }
}
