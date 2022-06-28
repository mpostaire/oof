#include <gb/gb.h>
#include <gb/cgb.h>

#include <stdint.h>

#define GRAVITY 1
#define MAX_GRAVITY 64
#define PLAYER_ACCEL 4
#define PLAYER_MAX_ACCEL 16
#define PLAYER_JUMP_ACCEL 8
#define PLAYER_MAX_JUMP_TIME 3

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144
#define SCREEN_OFFSET_X 8
#define SCREEN_OFFSET_Y 16
#define PLAYER_SIZE 7 // always pick one less
#define TILEMAP_WIDTH 20
#define TILEMAP_HEIGHT 18

#define CORNER_NONE 0
#define CORNER_NW 1
#define CORNER_NE 2
#define CORNER_SW 3
#define CORNER_SE 4

#define SCREEN_POS_TO_TILEMAP_ID(x, y) (((x) >> 3) - 1 + (((y) >> 3) - 2) * TILEMAP_WIDTH)

// round up x to the nearest multiple of 8
#define ROUND_UP8(x) (((x) + 7) & (-8))
// round down x to the nearest multiple of 8
#define ROUND_DOWN8(x) ((x) & (-8))

const uint8_t sprite_data[] = {
    0x3C,0x3C,0x42,0x7E,0x99,0xFF,0xA9,0xFF,0x89,0xFF,0x89,0xFF,0x42,0x7E,0x3C,0x3C
};

// white tile and horizontal stripe tile
const uint8_t tile_data[] = {
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
    0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

struct {
    uint16_t x;
    uint16_t y;
    int16_t vx;
    int16_t vy;
    uint8_t jump_time;
    uint8_t can_jump;
    uint8_t tile_cooldown;
} player;

static void init(void) {
    // sound (these registers must be set in this order!)
    NR52_REG = 0x80; // turns on sound
    NR50_REG = 0x77; // sets the volume for both left and right channel to max
    NR51_REG = 0xFF; // all 4 sound channels enabled for both left and right channel

    // init dmg palettes
    BGP_REG = OBP0_REG = OBP1_REG = 0xE4;

    // load tile data into VRAM
    set_sprite_data(0, 4, sprite_data);

    // set sprite tile
    set_sprite_tile(0, 0);

    // Load Background tiles and then map
    set_bkg_data(0, 2, tile_data);
    set_bkg_tiles(0, 0, 20, 18, tile_map);

    // (8, 16) is the top left corner
    player.x = SCREEN_OFFSET_X << 4;
    player.y = SCREEN_OFFSET_Y << 4;
    player.jump_time = player.vx = player.vy = 0;
    player.can_jump = 0;
    player.tile_cooldown = 0;

    // show bkg and sprites
    SHOW_BKG;
    SHOW_SPRITES;

    DISPLAY_ON;
}

static inline void play_jump_sound(void) {
    NR10_REG = 0x16;
    NR11_REG = 0x40;
    NR12_REG = 0x73;
    NR13_REG = 0x00;
    NR14_REG = 0xC3;
}

static inline void place_tile(uint8_t tile) {
    // TODO tile_map array out of bounds check
    uint8_t tile_x = (player.x >> 4) + PLAYER_SIZE + 1;
    uint8_t tile_y = player.y >> 4;
    int16_t i = SCREEN_POS_TO_TILEMAP_ID(tile_x, tile_y);
    tile_map[i] = tile;
    set_bkg_tiles((tile_x >> 3) - 1, (tile_y >> 3) - 2, 1, 1, &tile);
}

static void handle_input(void) {
    // poll joypad
    uint8_t input = joypad();

    if (player.can_jump || player.jump_time) {
        if (input & J_A) {
            player.jump_time++;
            if (player.jump_time == 1)
                play_jump_sound();
            if (player.jump_time > PLAYER_MAX_JUMP_TIME) {
                player.jump_time = 0;
                player.can_jump = 0;
            }
            player.vy -= PLAYER_JUMP_ACCEL;
        } else {
            player.jump_time = 0;
            player.can_jump = 0;
        }
    }

    if (input & J_B && player.tile_cooldown == 0) {
        place_tile(1); // places a wall -- place_tile(0) removes a wall (places an "air" tile)
        player.tile_cooldown = 16;
    }

    if (input & J_LEFT) {
        if (player.vx > -PLAYER_MAX_ACCEL)
            player.vx -= PLAYER_ACCEL;
    } else if (input & J_RIGHT) {
        if (player.vx < PLAYER_MAX_ACCEL)
            player.vx += PLAYER_ACCEL;
    } else {
        player.vx = 0;
    }
}

static inline uint8_t player_corner_wall_collision(uint8_t px, uint8_t py) {
    // each corner of the player can collide with 1 tile, enumerate the corners.
    // up left, up right, down left, down right.

    int16_t i = SCREEN_POS_TO_TILEMAP_ID(px, py);
    if (i >= 0 && tile_map[i])
        return CORNER_NW;

    i = SCREEN_POS_TO_TILEMAP_ID(px + PLAYER_SIZE, py);
    if (i >= 0 && tile_map[i])
        return CORNER_NE;

    i = SCREEN_POS_TO_TILEMAP_ID(px, py + PLAYER_SIZE);
    if (i >= 0 && tile_map[i])
        return CORNER_SW;

    i = SCREEN_POS_TO_TILEMAP_ID(px + PLAYER_SIZE, py + PLAYER_SIZE);
    if (i >= 0 && tile_map[i])
        return CORNER_SE;

    return 0;
}

/**
 * @brief check the side of collision and stops the player's speed accordingly
 */
static void player_walls_collision(void) {
    uint8_t corner = CORNER_NONE;
    uint16_t new_screen_x = (player.x + player.vx) >> 4;
    uint16_t new_screen_y = (player.y + player.vy) >> 4;
    player.can_jump = 0;

    if ((corner = player_corner_wall_collision(new_screen_x, player.y >> 4))) {
        switch (corner) {
        case CORNER_NW:
        case CORNER_SW:
            // collision at the left of the player
            player.x = ROUND_UP8(new_screen_x) << 4;
            break;
        case CORNER_NE:
        case CORNER_SE:
            // collision at the right of the player
            player.x = ROUND_DOWN8(new_screen_x) << 4;
            break;
        }

        player.vx = 0;
    }

    if ((corner = player_corner_wall_collision(player.x >> 4, new_screen_y))) {
        switch (corner) {
        case CORNER_NE:
        case CORNER_NW:
            // collision at the top of the player
            player.y = ROUND_UP8(new_screen_y) << 4;
            // cancel jump when the player hits a ceiling
            player.can_jump = 0;
            player.jump_time = 0;
            break;
        case CORNER_SW:
        case CORNER_SE:
            // collision at the bottom of the player
            player.y = ROUND_DOWN8(new_screen_y) << 4;
            // can jump when the the player hits the ground
            player.can_jump = 1;
            player.jump_time = 0;
            break;
        }

        player.vy = 0;
    }
}

static void player_update(void) {
    // gravity
    player.vy += GRAVITY;
    if (player.vy > MAX_GRAVITY)
        player.vy = MAX_GRAVITY;

    player_walls_collision();

    player.x += player.vx;
    player.y += player.vy;

    if (player.tile_cooldown > 0)
        player.tile_cooldown--;
}

// main function
void main(void) {
    init();

    while (1) {
        handle_input();

        player_update();

        // Translate to pixels and move sprite
        // Downshift by 4 bits to use the whole number values
        move_sprite(0, player.x >> 4, player.y >> 4);

        // Done processing, yield CPU and wait for start of next frame (VBlank)
        wait_vbl_done();
    }
}
