#include <gb/gb.h>
#include <gb/cgb.h>

#include <stdint.h>
#define PLAYER_SIZE 8 
#define PIX_H 160
#define PIX_V 144
#define TMAP_X 20
#define TMAP_Y 18 
#define tile_row(py) (((py) >> 3))
#define tile_index(px, tile_row) (((px) >> 3) + (tile_row) * TMAP_X)
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
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
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
    { .x = 96 + 8, .y = 96 + 16, .w = 8, .h = 8 },
    { .x = 16 + 8, .y = 104 + 16, .w = 128, .h = 8 },
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

static uint8_t player_walls_collision(void) {
    // TODO don't use a wall_t array but deduce if there is a collision by reading the tilemap

    uint8_t px = player.x + player.vx;
    uint8_t py = player.y + player.vy;

    for (uint8_t i = 0; i < sizeof(walls) / sizeof(wall_t); i++) {
        if (px + PLAYER_SIZE > walls[i].x && px < walls[i].x + walls[i].w && py + 8 > walls[i].y && py < walls[i].y + walls[i].h) {
            // TODO if speed > 1, the player can be stopped before the wall
            player.vx = 0;
            player.vy = 0;
            return 1;
        }
    }
    return 0;
}
static uint8_t is_position_colliding(uint8_t px, uint8_t py) {
    uint8_t player_tr = tile_row(py);
    uint8_t player_ti = tile_index(px, player_tr);
    
    //each corner of the player can collide with 1 tile, enumerate the corners.
    //up left is player_ti
    //up right is player_ti + 1
    uint8_t c1_ti = player_ti + 1;
    //down left is player_ti + TMAP_X
    uint8_t c2_ti = player_ti + TMAP_X;
    //down right is c2_ti + 1
    uint8_t c3_ti = c2_ti + 1;

    return  tile_map[player_ti] +
            tile_map[c1_ti] +
            tile_map[c2_ti] +
            tile_map[c3_ti] > 0;


}
/**
 * @brief check the side of collision and stops the player's speed accordingly
 * @return number of directions colliding.
 */
static uint8_t alt_player_walls_collision(void) {
    // TODO don't use a wall_t array but deduce if there is a collision by reading the tilemap
    uint8_t ret = 0;
    uint8_t px = player.x + player.vx;
    uint8_t py = player.y + player.vy;

    if (is_position_colliding(px, player.y)) {
        player.vx = 0;
        px = player.x;
        ret += 1;
    }
    if (is_position_colliding(player.x, py)) {
        player.vy = 0;
        py = player.y;
        ret += 1;
    }
    player.x = px;
    player.y = py;
    return ret;
}
static void player_update(void) {
    // jump
    if (player.jump) {
        player.vy -= 2;
        if (player.vy < -8) player.vy = -8;
        player.jump--;
    }

    // if (player_walls_collision())
        // player.vx = player.vy = 0;

    alt_player_walls_collision();

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
