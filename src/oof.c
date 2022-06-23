#include <gb/gb.h>
#include <gb/cgb.h>

#include <stdint.h>

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144
#define SCREEN_OFFSET_X 8
#define SCREEN_OFFSET_Y 16
#define PLAYER_SIZE 7 // always pick one less
#define TILEMAP_WIDTH 20
#define TILEMAP_HEIGHT 18

#define PLAYER_POS_TO_TILE_ID(px, py) (((px) >> 3) - 1 + (((py) >> 3) - 2) * TILEMAP_WIDTH)

uint8_t sprite_data[] = {
    0x3C,0x3C,0x42,0x7E,0x99,0xFF,0xA9,0xFF,0x89,0xFF,0x89,0xFF,0x42,0x7E,0x3C,0x3C
};

// white tile and horizontal stripe tile
uint8_t tile_data[] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0xFF,0xFF,0xFF,0x00,0xFF,0xFF,0xFF,0x00,0xFF,0xFF,0xFF,0x00,0xFF,0xFF,0xFF,0x00
};

uint8_t tile_map[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

struct {
    uint8_t x;
    uint8_t y;
    int8_t vx;
    int8_t vy;
    uint8_t jump;
} player;

static void init(void) {
    // init dmg palettes
    BGP_REG = OBP0_REG = OBP1_REG = 0xE4;

    // load tile data into VRAM
    set_sprite_data(0, 4, sprite_data);

    // set sprite tile
    set_sprite_tile(0, 0);

    // Load Background tiles and then map
    set_bkg_data(0, 2, tile_data);
    set_bkg_tiles(0, 0, 20, 20, tile_map);

    // (8, 16) is the top left corner
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

static inline uint8_t player_over_wall(uint8_t px, uint8_t py) {
    // each corner of the player can collide with 1 tile, enumerate the corners.
    // up left, up right, down left, down right.
    return tile_map[PLAYER_POS_TO_TILE_ID(px, py)]
        || tile_map[PLAYER_POS_TO_TILE_ID(px + PLAYER_SIZE, py)]
        || tile_map[PLAYER_POS_TO_TILE_ID(px, py + PLAYER_SIZE)]
        || tile_map[PLAYER_POS_TO_TILE_ID(px + PLAYER_SIZE, py + PLAYER_SIZE)];
}

/**
 * @brief check the side of collision and stops the player's speed accordingly
 * @return number of directions colliding.
 */
static uint8_t player_walls_collision(void) {
    uint8_t ret = 0;

    if (player_over_wall(player.x + player.vx, player.y)) {
        player.vx = 0;
        ret++;
    }

    if (player_over_wall(player.x, player.y + player.vy)) {
        player.vy = 0;
        ret++;
    }

    return ret;
}

static void player_update(void) {
    // jump
    if (player.jump) {
        player.vy -= 2;
        if (player.vy < -8) player.vy = -8;
        player.jump--;
    }

    player_walls_collision();

    player.x += player.vx;
    player.y += player.vy;

    // decelerate
    if (player.vy >= 0) {
        if (player.vy) player.vy--;
    } else player.vy++;
    if (player.vx >= 0) {
        if (player.vx) player.vx--;
    } else player.vx++;
}

// main function
void main(void) {
    init();

    while (1) {
        handle_input();

        player_update();

        // Translate to pixels and move sprite
        // Downshift by 4 bits to use the whole number values
        move_sprite(0, player.x, player.y);

        // Done processing, yield CPU and wait for start of next frame (VBlank)
        wait_vbl_done();
    }
}
