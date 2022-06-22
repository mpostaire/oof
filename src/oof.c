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

struct {
    uint8_t x;
    uint8_t y;
    int8_t vx;
    int8_t vy;
    uint8_t jump;
} player;

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t w;
    uint8_t h;
} wall_t;

wall_t walls[] = {
    { .x = 16, .y = 96, .w = 128, .h = 8 },
    // { .x = 16, .y = 96, .w = 128, .h = 8 }
};

static void init(void) {
    // init dmg palettes
    BGP_REG = OBP0_REG = OBP1_REG = 0xE4;

    // load tile data into VRAM
    set_sprite_data(0, 4, sprite_data);

    // set sprite tile
    set_sprite_tile(0, 0);

    // Load Background tiles and then map
    set_bkg_data(0, 2, tile_data);
    set_bkg_tiles(0, 12, 20, 2, tile_map);

    player.x = 8;
    player.y = 16;
    player.jump = player.vx = player.vy = 0;

    // show bkg and sprites
    SHOW_BKG;
    SHOW_SPRITES;

    DISPLAY_ON;
}

static void handle_input(void) {
    // poll joypad
    uint8_t input = joypad();

    if (input & J_UP) {
        player.vy--;
        if (player.vy < -32) player.vy = -32;
    } else if (input & J_DOWN) {
        player.vy++;
        if (player.vy > 32) player.vy = 32;
    }

    if (input & J_LEFT) {
        player.vx--;
        if (player.vx < -32) player.vx = -32;
    } else if (input & J_RIGHT) {
        player.vx++;
        if (player.vx > 32) player.vx = 32;
    }

    if ((input & J_A) && (!player.jump)) {
        player.jump = 3;
    }
}

static void player_update(void) {
    // jump
    if (player.jump) {
        player.vy -= 2;
        if (player.vy < -8) player.vy = -8;
        player.jump--;
    }

    player.x += player.vx;
    player.y += player.vy;

    // gravity
    // player.vy += 4;

    // decelerate
    if (player.vy >= 0) {
        if (player.vy) player.vy--;
    } else player.vy++;
    if (player.vx >= 0) {
        if (player.vx) player.vx--;
    } else player.vx++;

}

static void player_collision(void) {
    for (uint8_t i = 0; i < sizeof(walls) / sizeof(wall_t); i++) {
        // TODO rectangle collision with different handling depending on the side of the wall
    }
}

// main function
void main(void) {
    init();

    while (1) {
        handle_input();

        player_update();

        player_collision();

        // Translate to pixels and move sprite
        // Downshift by 4 bits to use the whole number values
        move_sprite(0, player.x, player.y);

        // Done processing, yield CPU and wait for start of next frame (VBlank)
        wait_vbl_done();
    }
}
