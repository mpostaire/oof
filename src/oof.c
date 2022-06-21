#include <gb/gb.h>
#include <gb/cgb.h>

#include <stdint.h>

uint8_t sprite_data[] = {
    0x3C,0x3C,0x42,0x7E,0x99,0xFF,0xA9,0xFF,0x89,0xFF,0x89,0xFF,0x42,0x7E,0x3C,0x3C
};

// black tile
uint8_t tile_data[] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0xFF,0xFF,0xFF,0x00,0xFF,0xFF,0xFF,0x00,0xFF,0xFF,0xFF,0x00,0xFF,0xFF,0xFF,0x00
};

uint8_t tile_map[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0
};

// sprite coords
uint16_t pos_x, pos_y;
int16_t vel_x, vel_y;
uint8_t jump;

// main function
void main(void) {
    // init palettes
    BGP_REG = 0xE4;
    OBP0_REG = 0xE4;
    OBP1_REG = 0xE4;

    // load tile data into VRAM
    set_sprite_data(0, 4, sprite_data);

    // set sprite tile
    set_sprite_tile(0, 0);

    // Load Background tiles and then map
    set_bkg_data(0, 2, tile_data);
    set_bkg_tiles(0, 12, 20, 2, tile_map);

    // show bkg and sprites
    SHOW_BKG;
    SHOW_SPRITES;

    pos_x = pos_y = 64 << 4;
    jump = vel_x = vel_y = 0;

    while (1) {
        // poll joypad
        uint8_t input = joypad();

        // game object
        if (input & J_UP) {
            vel_y -= 2;
            if (vel_y < -64) vel_y = -64;
        } else if (input & J_DOWN) {
            vel_y += 2;
            if (vel_y > 64) vel_y = 64;
        }
        if (input & J_LEFT) {
            vel_x -= 2;
            if (vel_x < -64) vel_x = -64;
        } else if (input & J_RIGHT) {
            vel_x += 2;
            if (vel_x > 64) vel_x = 64;
        }
        if ((input & J_A) && (!jump)) {
            jump = 3;
        }

        // jump
        if (jump) {
            vel_y -= 8;
            if (vel_y < -32) vel_y = -32;
            jump--;
        }

        pos_x += vel_x;
        pos_y += vel_y;

        // Translate to pixels and move sprite
        // Downshift by 4 bits to use the whole number values
        move_sprite(0, pos_x >> 4, pos_y >> 4);

        // decelerate
        if (vel_y >= 0) {
            if (vel_y) vel_y--;
        } else vel_y++;
        if (vel_x >= 0) {
            if (vel_x) vel_x--;
        } else vel_x++;

        // Done processing, yield CPU and wait for start of next frame (VBlank)
        wait_vbl_done();
    }
}
