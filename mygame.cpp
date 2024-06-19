//
// Created by gajko on 24.04.2024.
//
#include <chrono>
#include <SDL.h>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

// #include "SDL_image.h"
#include <set>

#include "vendored/sdl/include/SDL.h"
#include "vendored/sdl/include/SDL_events.h"
#include "vendored/sdl/include/SDL_video.h"
#include "vendored/sdl/include/SDL_error.h"
#include "vendored/sdl/include/SDL_rwops.h"
#include "vendored/sdl/src/render/SDL_sysrender.h"
#include "vendored/SDL/src/video/SDL_sysvideo.h"
// #include "vendored/sdl/src/dynapi/SDL_dynapi.h"
// #include "vendored/sdl/src/dynapi/SDL_dynapi_overrides.h"

// using vec_t = std::array<double, 2>;

#define TILE_SIZE 64                                                // floor tile size constant (in pixels)

struct player_t;                                                    // forward-declaring the player struct for like
                                                                    // exactly one method
struct game_map_t {
    int width, height;
    std::vector<int> map;
    int get(int x, int y) const {
        if (x < 0 || x >= width || y < 0 || y >= height) return 1;  // detecting going out of bounds
        return map[y*width + x];                                    // y - rows, x - columns
    }
};

game_map_t game_map = {30, 17,
    { // 0 - empty space, 1 - cube (never gonna need those, 2 - sqaure
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
        2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
        }
};

// textures
std::shared_ptr<SDL_Texture> load_image(SDL_Renderer *renderer, const std::string &filename) {
    SDL_Surface *surface = SDL_LoadBMP(filename.c_str());
    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create surface from image: %s", SDL_GetError());
        throw std::invalid_argument(SDL_GetError());
    }
    SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 0, 255, 255)); // dać cyan bg na transparentny chyba
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture from surface: %s", SDL_GetError());
        throw std::invalid_argument(SDL_GetError());
    }
    SDL_FreeSurface(surface);
    return {texture, [](SDL_Texture *t) { SDL_DestroyTexture(t); } };
}

// Move class, given to every action in the game
class Move {
public:
    std::shared_ptr<SDL_Texture> load_sheet(SDL_Renderer *renderer, const std::string &filename, int width, int height,
                                              int hurtboxW, int hurtboxH, int frames, double speed);
    SDL_Rect getSpecificFrame(int frame) const;
    int getFrameCount() const;
    double getAnimSpeed() const;
    SDL_Rect getHitbox() const;
    void setHitbox(int x, int y, int w, int h, int onHit, int onBlock, int hitstun, int firstActiveFrame, int lastActiveFrame);
    bool isActive() const;
    void setActive(bool isItActiveTho) const;
    void showHitbox(SDL_Renderer *renderer, double timer) const;

    bool confirmHit(SDL_Rect hurtbox) const;
    void doDamage(player_t player, SDL_Rect) const;

private:
    SDL_Texture* sprite_sheet = {};                                 // image texture we're taking sprites from
    int frame_count = 0;                                            // amount of total frames
    double anim_speed = 0;                                          // the bigger it is, the faster the sprites will cycle through
    mutable bool is_active = false;                                 // indicates when a hitbox is active for an attack
    int first_active_frame = 0;                                     // when the move starts being able to deal damage
int last_active_frame = 0;                                          // when the move stops being able to deal damage
    int damage_on_hit = 0;                                          // damage dealt on a clean hit
    int damage_on_block = 0;                                        // damage dealt when the opponent is blocking
    int hitstun_frames = 0;                                         // how long the "hurt" animation stays on the first frame
    SDL_Rect hitbox = {};                                           // hitbox coordinates and size
    std::vector<SDL_Rect> frame_list;                               // list of every frame of an action
};

std::shared_ptr<SDL_Texture> Move::load_sheet(SDL_Renderer *renderer, const std::string &filename, int width, int height,
                                            int hurtboxW, int hurtboxH, int frames, double speed) {
    // same thing as loading a regular bitmap for a single image (with multiple error messages bc I was paranoid)
    SDL_Surface *surface = SDL_LoadBMP(filename.c_str());
    if (!surface) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Couldn't create surface from image: %s", SDL_GetError(), NULL);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create surface from image: %s", SDL_GetError());
        throw std::invalid_argument(SDL_GetError());
    }
    SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 0, 255, 255)); // turns a pure cyan bg transparent
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Couldn't create texture from surface: %s", SDL_GetError(), NULL);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture from surface: %s", SDL_GetError());
        throw std::invalid_argument(SDL_GetError());
    }
    // setting some field variables
    sprite_sheet = texture;
    frame_count = frames;
    anim_speed = speed;

    // looping through the frames, i is height indices, j is width indices
    for (int i = 0; i < texture->h/height; i++) {
        for (int j = 0; j < texture->w/width; j++) {
            if (i*(texture->w/width) + j == frames) break;
            SDL_Rect frame = { j*width, i*height, hurtboxW, hurtboxH };  // make only a certain part of the rendering frame
                                                                              // to be made visible to the player
            frame_list.push_back(frame);                                      // put frame from sprite sheet onto the frame list
        }
    }

    SDL_FreeSurface(surface);                                                 // free surface from memory
    return {texture, [](SDL_Texture *t) { SDL_DestroyTexture(t); } };  // return the texture, then free it
}

SDL_Rect Move::getSpecificFrame(int frame) const {
    return frame_list.at(frame);                                            // picks a specific frame from the list/vector
}

int Move::getFrameCount() const {
    return frame_count;                                                     // returns total frame count
}

double Move::getAnimSpeed() const {
    return anim_speed;                                                      // returns animation "speed"
}

SDL_Rect Move::getHitbox() const {
    return hitbox;                                                          // returns the hitbox Rect
}

void Move::setHitbox(int x, int y, int w, int h, int onHit, int onBlock, int hitstun, int firstActiveFrame, int lastActiveFrame) {
    hitbox.x = x;                                                           // set the hitbox's starting coordinates
    hitbox.y = y;
    hitbox.w = w;                                                           // set the hitbox's width and height
    hitbox.h = h;
    damage_on_hit = onHit;                                                  // set other field variables
    damage_on_block = onBlock;
    hitstun_frames = hitstun;
    if (firstActiveFrame < 0) firstActiveFrame = 0;                         // preventative measures for active frames
    if (lastActiveFrame >= frame_count) lastActiveFrame = frame_count;
    first_active_frame = firstActiveFrame;                                  // set active frames
    last_active_frame = lastActiveFrame;
}

bool Move::isActive() const {
    return is_active;                                                       // check if the move is active
}

void Move::setActive(bool isItActiveTho) const {
    is_active = isItActiveTho;                                              // sets the move's activeness
}

void Move::showHitbox(SDL_Renderer *renderer, double timer) const {
    if (const int active_frame = (int)floor(timer / (1.0/anim_speed)) % frame_count;        // modulos out the current frame
                                                                                              // out of the total frame count
        active_frame >= first_active_frame - 1 && active_frame <= last_active_frame - 1) {    // second part of the if statement
                                                                                              // making it so that we only need to think about
                                                                                              // frames starting from frame 1, not 0 (when typing them into the function)
        setActive(true);                                                            // make hitbox active and render it when it is supposed to be
        SDL_SetRenderDrawColor(renderer, 255,0,0, 0xFF);
        SDL_RenderFillRect(renderer, &hitbox);
    } else setActive(false);
}

bool Move::confirmHit(const SDL_Rect hurtbox) const {
    return SDL_HasIntersection(&hitbox, &hurtbox);       // check if the move's hitbox intersects with player_rects
}

// initializing math vectors as either .x and .y fields or as [x,y]
union vect_t {
    struct { double x; double y;} v;
    double p[2];
};

vect_t operator+(const vect_t a, const vect_t b) {          // vector addition
    vect_t ret = a;
    ret.v.x += b.v.x;
    ret.v.y += b.v.y;
    return ret;
}

vect_t operator*(const vect_t a, const double b) {          // vector scalar multiplication
    vect_t ret = a;
    ret.v.x *= b;
    ret.v.y *= b;
    return ret;
}

struct player_t {
    vect_t p{}; // position
    vect_t v{}; // velocity
    vect_t a{}; // acceleration
    float health = 100.0;
    // int num;
    // enum character { PEPPINO, FOOTSIES };
    double time_since_last_input{};                         // used for ending anims properly ( not sure if it is used properly, but hey)
    bool is_attacking{};                                    // various player states
    bool is_blocking{};
    bool is_crouching{};
    bool is_jumping{};
    bool is_jumping_forward{};
    bool is_jumping_backward{};
    bool is_falling{};
    bool is_falling_forward{};
    bool is_falling_backward{};
    // bool is_walking;
    bool is_walking_backward{};
    bool is_walking_forward{};
    bool is_hurt{};
    bool is_dead{};

    enum AttackType {                                       // varying attack strength
        NONE = 0,
        groundL = 1, groundM, groundH, groundS,
        airL, airM, airH, airS,
    };
    Move idle{};                                            // non-attacking moves
    Move forward_walk{};
    Move back_walk{};
    Move jump{};
    Move fall{};
    Move forward_jump{};
    Move backward_jump{};
    Move forward_fall{};
    Move backward_fall{};
    Move crouch{};

    AttackType currentAttack = NONE;                        // attacking moves
    Move attackL{};
    Move attackM{};
    Move attackH{};
    Move attackS{};
    Move air_attackL{};
    Move air_attackM{};
    Move air_attackH{};
    Move air_attackS{};
    // Move block{};                                        // defensive actions
    // Move block_low{};
    Move hurt{};                                            // other actions
    Move dead{};
};

void Move::doDamage(player_t hitPlayer, const SDL_Rect hurtbox) const {
    if (isActive() && confirmHit(hurtbox)) {                            // is hitbox active and overlapping with the opponent's hurtbox
        // if (hitPlayer.is_blocking) {
        //     hitPlayer.health -= rand() % 6 * 0.1 + damage_on_block;
        // }
        hitPlayer.health -= rand() % 10 * 0.1 + damage_on_hit;          // apply set damage + a variable amount of extra credit
    } else {
        if (hitPlayer.health < 100.0) hitPlayer.health += 0.5;          // make the opponent heal a bit for whiffing your attack
                                                                        // (not sure if works)
    }
}

bool is_colliding(vect_t position, const game_map_t &map) {
    return map.get(position.v.x, position.v.y) > 0;                     // checks if the player is colliding with the map
}

bool is_grounded(const player_t &player, const game_map_t &map) {
    return map.get(player.p.v.x, player.p.v.y+0.01) > 0;              // checks if the player is on the ground or not
}

bool isPlayerDead(const player_t &player) {
    if (player.health > 0) return false;                                // checks if a player's health is below 0
    return true;
}

// updating the player's position during gameplay
player_t update_player(player_t player_old, const game_map_t &map, double dt) {
    player_t player = player_old;
    // player.num = 0;
    // player_t player2 = // ????
    // if (!is_grounded(player_old, map)) { // spadanie (w powietrzu)
    //     player_old.a.v.x = 0;
    //     player_old.a.v.y = 90;
    // } else { // opóźnienie (na ziemi)
    //     if (std::abs(player_old.v.v.x) > 10) {
    //         player_old.a.v.x = 0;
    //     }
    // }

    if (!is_grounded(player_old, map)) {
        if (player_old.currentAttack == player_t::airS) {
            player_old.a.v.y = 1000;                                // air S makes you SLAM into the ground
        } else {
            player_old.a.v.y = 50;                                  // gravitational pull
            player_old.a.v.x = player_old.a.v.x * 0.9;              // air resistance (drag)
        }
    }

    player.p = player_old.p + (player_old.v * dt) + (player_old.a*dt*dt)*0.5;       // calculating acceleration
    player.v = player_old.v + (player_old.a * dt);                                  // calculating velocity
    player.v = player.v * 0.93;                                                     // friction

    std::vector<vect_t> collision_points = {
        {{-0.4,0.0}},               // bottom left-ish
        {{0.4,0.0}},                // bottom right-ish, only those two are used (for floor collision)
    //     {{-0.4,2.0}},                        // top left-ish
    //     {{0.4,2.0}},                         // top right-ish
    //     {{1.0,0.5}},                         // lower right (side)
    //     {{-1.0,0.5}},                        // lower left (side)
    //     {{1.0,1.5}},                         // upper right (side)
    //     {{-1.0,1.5}}                         // upper left (side)
    };
    std::vector<vect_t> collision_mods = {
        {{0.0,-1.0}},               // moves up a bit
        {{0.0,-1.0}},
    //     {{0.0,1.0}},                         // moves down a bit
    //     {{0.0,1.0}},
    //     {{-1.0,0.0}},                        // pushes to the left
    //     {{1.0,0.0}},                         // pushes to the right
    //     {{-1.0,0.0}},                        // pushes to the left
    //     {{1.0,0.0}}                          // pushes to the right
    };

    for (int i = 0; i < collision_points.size(); i++) {
        auto test_point = player.p + collision_points[i];

        if (is_colliding(test_point, map)) {
            if (collision_mods[i].v.y < 0) {
                player.v.v.y = 0;                       // pushes up from the ground
                player.p.v.y = player_old.p.v.y;
            }
        }
    }
    if (player.p.v.y < 0) {                             // upper boundary
        player.p.v.y = 0;
        player.v.v.y = 0;
    }
    if (player.p.v.y > 17) {                            // lower boundary
        player.p.v.y = 17;
        player.v.v.y = 0;
    }
    if (player.p.v.x < 0) {                             // left boundary
        player.p.v.x = 0;
        player.v.v.x = 0;
    }
    if (player.p.v.x > 25) {                            // right boundary
        player.p.v.x = 25;
        player.v.v.x = 0;
    }
    return player;
}

void draw_map(SDL_Renderer *renderer, const game_map_t & map, const std::shared_ptr<SDL_Texture>& tex) {
    for (int y = map.height - 1; y >= 0; y--) {
        for (int x = 0; x < map.width; x++) {
            SDL_Rect dst = {x*TILE_SIZE, y*TILE_SIZE, TILE_SIZE*2, TILE_SIZE*2}; // render square is 128x128, only render the bottom right of it

            if (map.get(x,y) > 0) {
                SDL_Rect src = {128*(map.get(x,y) - 1), 0, TILE_SIZE*2, TILE_SIZE*2}; // make it so that both cubes and squares don't look weird when they're next to their own kind
                SDL_RenderCopy(renderer, tex.get(), &src, &dst);
            }
        }
    }
}

void healthBar(SDL_Renderer *renderer, const player_t& player, int x, int y, int w, int h, const SDL_Color bg_color, const SDL_Color hp_color) {
    // funky health check
    float percent = player.health / 100.0;
    percent = percent > 1.f ?                       // if greater than 1, send 1
        1.f : percent < 0.f ?                       // if less than 0, send 0
        0.f : percent;                              // if not less than 0, send the percent decimal between 0 and 1

    SDL_Color old_color;                            // checks last color rendered
    SDL_GetRenderDrawColor(renderer, &old_color.r, &old_color.g, &old_color.b, &old_color.a);

    SDL_Rect bg_rect = {x, y, w, h};
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
    SDL_RenderFillRect(renderer, &bg_rect);         // render horizontal NOT-health bar

    SDL_SetRenderDrawColor(renderer, hp_color.r, hp_color.g, hp_color.b, hp_color.a);
    int perc_w = (int)((float)w * percent);         // checks the percentage of health bar to see how much will have to be rendered
    int perc_x = x + (w - perc_w);
    SDL_Rect hp_rect = { perc_x, y, perc_w, h};
    SDL_RenderFillRect(renderer, &hp_rect);
    SDL_SetRenderDrawColor(renderer, old_color.r, old_color.g, old_color.b, old_color.a); // render health bar
}

int main(int argc, char *argv[])
{
    using namespace std::chrono_literals;
    using namespace std::chrono;
    using namespace std;
    // make window fullscreen
    SDL_Window *window = SDL_CreateWindow("PEP VS. FOOT",  SDL_WINDOWPOS_CENTERED,  SDL_WINDOWPOS_CENTERED, 1280, 720,
                                            SDL_WINDOW_FULLSCREEN_DESKTOP);
    // make sure to enable vsync
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    double dt = 1./60.;

    // check if we're in video mode
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Couldn't initialize SDL: %s", SDL_GetError(), NULL);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        return 3;
    }

    // check if window was initialized
    if (!window) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Couldn't create window: %s", SDL_GetError(), NULL);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window: %s", SDL_GetError());
        return 3;
    }

    // check if renderer was initialized
    if (!renderer) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Couldn't create renderer: %s", SDL_GetError(), NULL);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create renderer: %s", SDL_GetError());
        return 3;
    }

    auto tiles_texture = load_image(renderer, "tiles.bmp");     // load tileset
    auto bg_texture = load_image(renderer, "background.bmp");   // load background

    SDL_Color bg_color = {0, 162, 232, 255};                    // background color for health bar

    SDL_Color hp_color = {255, 0, 0, 255};                      // health bar color

    bool still_playing = true;
    player_t player;                                                    // initialize player 1
    // player.PEPPINO;

    player.p.v.x = 0;                                                   // player 1's coords and starting v and a
    player.p.v.y = 4;
    player.a.v.x = 0;
    player.a.v.y = 0;
    player.v.v.x = 0;
    player.v.v.y = 0;

    player_t player2;                                                    // initialize player 2
    // player2.FOOTSIES;

    player2.p.v.x = 23;                                                   // player 2's coords and starting v and a
    player2.p.v.y = 4;
    player2.a.v.x = 0;
    player2.a.v.y = 0;
    player2.v.v.x = 0;
    player2.v.v.y = 0;

    srand((int)dt*60);                                      // RNG for random extra damage credit

    // load player 1's non-attacking actions
    auto player_idle = player.idle.load_sheet(renderer, "PEPPINO Sprites/idle/idle.bmp", 100, 100, 75, 96, 17, 20.0);
    auto player_fwalk = player.forward_walk.load_sheet(renderer, "PEPPINO Sprites/move/move.bmp", 100, 100, 85, 96, 12, 20.0);
    auto player_bwalk = player.back_walk.load_sheet(renderer, "PEPPINO Sprites/move/move.bmp", 100, 100, 85, 96, 12, 20.0);
    auto player_crouch = player.crouch.load_sheet(renderer, "PEPPINO Sprites/crouch/crouch.bmp", 100, 100, 85, 96, 2, 2.0);
    auto player_jump = player.jump.load_sheet(renderer, "PEPPINO Sprites/jump/jump.bmp", 100, 100, 85, 96, 12, 20.0);
    auto player_fall = player.fall.load_sheet(renderer, "PEPPINO Sprites/fall/fall.bmp", 100, 100, 85, 96, 3, 20.0);
    auto player_fjump = player.forward_jump.load_sheet(renderer, "PEPPINO Sprites/jump2/jump2.bmp", 100, 100, 90, 96, 12, 20.0);
    auto player_bjump = player.backward_jump.load_sheet(renderer, "PEPPINO Sprites/jump2/jump2.bmp", 100, 100, 90, 96, 12, 20.0);
    auto player_ffall = player.forward_fall.load_sheet(renderer, "PEPPINO Sprites/fall2/fall2.bmp", 100, 100, 90, 96, 3, 20.0);
    auto player_bfall = player.backward_fall.load_sheet(renderer, "PEPPINO Sprites/fall2/fall2.bmp", 100, 100, 90, 96, 3, 20.0);
    auto player_hurt = player.hurt.load_sheet(renderer, "PEPPINO Sprites/hurt/hurt.bmp", 100, 100, 75, 96, 3, 20.0);
    if (!is_grounded(player, game_map)) {
        auto player_dead = player.dead.load_sheet(renderer, "PEPPINO Sprites/outofpizza1/outofpizza1.bmp", 120, 100, 100, 96, 11, 16.0);
    }
    auto player_dead = player.dead.load_sheet(renderer, "PEPPINO Sprites/outofpizza4/outofpizza4.bmp", 120, 100, 100, 96, 6, 16.0);

    // load player 1's attacking actions and set hitboxes
    auto player_attackL = player.attackL.load_sheet(renderer, "PEPPINO Sprites/slap/slap.bmp", 200, 100, 130, 96, 12, 20.0);
    player.attackL.setHitbox((int)(player.p.v.x*TILE_SIZE+TILE_SIZE)+72, (int)(player.p.v.y*TILE_SIZE+TILE_SIZE+259)+41, 248, 180, 5, 2, 2, 2, 8); //!! dopisać poprawne wg player_rectów
    auto player_attackM = player.attackM.load_sheet(renderer, "PEPPINO Sprites/tackle/tackle.bmp", 100, 100, 98, 96, 8, 20.0);
    player.attackM.setHitbox((int)(player.p.v.x*TILE_SIZE+TILE_SIZE)+52, (int)(player.p.v.y*TILE_SIZE+TILE_SIZE+259)+50, 156, 152, 10, 3, 4, 3, 4); //!! dopisać poprawne wg player_rectów
    auto player_attackH = player.attackH.load_sheet(renderer, "PEPPINO Sprites/backkick/backkick.bmp", 100, 100, 100, 96, 8, 20.0);
    player.attackH.setHitbox((int)(player.p.v.x*TILE_SIZE+TILE_SIZE)+55, (int)(player.p.v.y*TILE_SIZE+TILE_SIZE+259)+40, 172, 188, 14, 6, 6, 5, 8); //!! dopisać poprawne wg player_rectów
    auto player_attackS = player.attackS.load_sheet(renderer, "PEPPINO Sprites/uppunch/uppunch.bmp", 100, 100, 100, 96, 7, 20.0);
    player.attackS.setHitbox((int)(player.p.v.x*TILE_SIZE+TILE_SIZE)+37, (int)(player.p.v.y*TILE_SIZE+TILE_SIZE+259)+0, 156, 200, 19, 8, 8, 4, 7); //!! dopisać poprawne wg player_rectów
    auto player_airL = player.air_attackL.load_sheet(renderer, "PEPPINO Sprites/machpunch/machpunch.bmp", 100, 100, 95, 96, 10, 20.0);
    player.air_attackL.setHitbox((int)(player.p.v.x*TILE_SIZE+TILE_SIZE)+57, (int)(player.p.v.y*TILE_SIZE+TILE_SIZE+259)+15, 164, 216, 4, 2, 2, 3, 7); //!! dopisać poprawne wg player_rectów
    auto player_airM = player.air_attackM.load_sheet(renderer, "PEPPINO Sprites/slapup/slapup.bmp", 200, 200, 150, 155, 7, 20.0);
    player.air_attackM.setHitbox((int)(player.p.v.x*TILE_SIZE+TILE_SIZE)+76, (int)(player.p.v.y*TILE_SIZE+TILE_SIZE+259)+19, 312, 280, 9, 3, 3, 3, 5); //!! dopisać poprawne wg player_rectów
    auto player_airH = player.air_attackH.load_sheet(renderer, "PEPPINO Sprites/knock/knock.bmp", 100, 100, 85, 96, 7, 20.0);
    player.air_attackH.setHitbox((int)(player.p.v.x*TILE_SIZE+TILE_SIZE)+58, (int)(player.p.v.y*TILE_SIZE+TILE_SIZE+259)+24, 144, 192, 13, 7, 7, 4, 5); //!! dopisać poprawne wg player_rectów
    auto player_airS = player.air_attackS.load_sheet(renderer, "PEPPINO Sprites/shoulder/shoulder.bmp", 100, 100, 80, 96, 7, 20.0);
    player.air_attackS.setHitbox((int)(player.p.v.x*TILE_SIZE+TILE_SIZE)+33, (int)(player.p.v.y*TILE_SIZE+TILE_SIZE+259)+53, 180, 180, 17, 10, 8, 3, 7); //!! dopisać poprawne wg player_rectów

    // load player 2's non-attacking actions
    auto player2_idle = player2.idle.load_sheet(renderer, "FOOTSIES Guy Sprites/Idle.bmp", 60, 50, 48, 50, 5, 12.0);
    auto player2_fwalk = player2.forward_walk.load_sheet(renderer, "FOOTSIES Guy Sprites/F00_Forward.bmp", 60, 50, 48, 50, 6, 12.0);
    auto player2_bwalk = player2.back_walk.load_sheet(renderer, "FOOTSIES Guy Sprites/F00_Backward.bmp", 60, 50, 48, 50, 6, 12.0);
    auto player2_crouch = player2.crouch.load_sheet(renderer, "FOOTSIES Guy Sprites/F00_CrouchGuard.bmp", 60, 50, 48, 50, 2, 1.0);
    auto player2_jump = player2.jump.load_sheet(renderer, "FOOTSIES Guy Sprites/Idle.bmp", 60, 50, 48, 50, 5, 12.0);
    auto player2_fall = player2.fall.load_sheet(renderer, "FOOTSIES Guy Sprites/Idle.bmp", 60, 50, 48, 50, 5, 12.0);
    auto player2_fjump = player2.forward_jump.load_sheet(renderer, "FOOTSIES Guy Sprites/Idle.bmp", 60, 50, 48, 50, 5, 12.0);
    auto player2_bjump = player2.backward_jump.load_sheet(renderer, "FOOTSIES Guy Sprites/Idle.bmp", 60, 50, 48, 50, 5, 12.0);
    auto player2_ffall = player2.forward_fall.load_sheet(renderer, "FOOTSIES Guy Sprites/Idle.bmp", 60, 50, 48, 50, 5, 12.0);
    auto player2_bfall = player2.backward_fall.load_sheet(renderer, "FOOTSIES Guy Sprites/Idle.bmp", 60, 50, 48, 50, 5, 12.0);
    auto player2_hurt = player2.hurt.load_sheet(renderer, "FOOTSIES Guy Sprites/F00_Damage.bmp", 60, 50, 45, 50, 4, 12.0);
    auto player2_dead = player2.dead.load_sheet(renderer, "FOOTSIES Guy Sprites/F00_Dead.bmp", 60, 50, 60, 50, 8, 8.0);

    // load player 2's attacking actions and set hitboxes
    auto player2_attackL = player2.attackL.load_sheet(renderer, "FOOTSIES Guy Sprites/F00_Attack_2.bmp", 60, 50, 55, 50, 5, 15.0);
    player2.attackL.setHitbox((int)(player2.p.v.x*TILE_SIZE+(TILE_SIZE/2))+41, (int)(player2.p.v.y*TILE_SIZE-TILE_SIZE+375)+23, 150, 200, 7, 2, 1, 3, 3); //!! dopisać poprawne wg player_rectów
    auto player2_attackM = player2.attackM.load_sheet(renderer, "FOOTSIES Guy Sprites/F00_Attack_0.bmp", 60, 50, 57, 50, 5, 15.0);
    player2.attackM.setHitbox((int)(player2.p.v.x*TILE_SIZE+(TILE_SIZE/2))+30, (int)(player2.p.v.y*TILE_SIZE-TILE_SIZE+375)+39, 260, 100, 11, 5, 4, 3, 3); //!! dopisać poprawne wg player_rectów
    auto player2_attackH = player2.attackH.load_sheet(renderer, "FOOTSIES Guy Sprites/F00_Attack_1.bmp", 60, 50, 60, 50, 8, 10.0);
    player2.attackH.setHitbox((int)(player2.p.v.x*TILE_SIZE+(TILE_SIZE/2))+33, (int)(player2.p.v.y*TILE_SIZE-TILE_SIZE+375)+16, 260, 190, 17, 8, 7, 6, 6); //!! dopisać poprawne wg player_rectów
    auto player2_attackS = player2.attackS.load_sheet(renderer, "FOOTSIES Guy Sprites/F00_Attack_3.bmp", 60, 50, 50, 50, 7, 8.0);
    player2.attackS.setHitbox((int)(player2.p.v.x*TILE_SIZE+(TILE_SIZE/2))+31, (int)(player2.p.v.y*TILE_SIZE-TILE_SIZE+375)+1, 180, 330, 23, 12, 10, 3, 4); //!! dopisać poprawne wg player_rectów
    auto player2_airL = player2.air_attackL.load_sheet(renderer, "FOOTSIES Guy Sprites/F00_Attack_2.bmp", 60, 50, 55, 50, 5, 15.0);
    player2.air_attackL.setHitbox((int)(player2.p.v.x*TILE_SIZE+(TILE_SIZE/2))+41, (int)(player2.p.v.y*TILE_SIZE-TILE_SIZE+375)+23, 150, 200, 4, 2, 1, 3, 3); //!! dopisać poprawne wg player_rectów
    auto player2_airM = player2.air_attackM.load_sheet(renderer, "FOOTSIES Guy Sprites/F00_Attack_3.bmp", 60, 50, 50, 50, 7, 8.0);
    player2.air_attackM.setHitbox((int)(player2.p.v.x*TILE_SIZE+(TILE_SIZE/2))+31, (int)(player2.p.v.y*TILE_SIZE-TILE_SIZE+375)+1, 180, 330, 13, 7, 10, 3, 4); //!! dopisać poprawne wg player_rectów
    auto player2_airH = player2.air_attackH.load_sheet(renderer, "FOOTSIES Guy Sprites/F00_Attack_1.bmp", 60, 50, 60, 50, 8, 10.0);
    player2.air_attackH.setHitbox((int)(player2.p.v.x*TILE_SIZE+(TILE_SIZE/2))+33, (int)(player2.p.v.y*TILE_SIZE-TILE_SIZE+375)+16, 260, 190, 17, 8, 8, 6, 6); //!! dopisać poprawne wg player_rectów
    auto player2_airS = player2.air_attackS.load_sheet(renderer, "FOOTSIES Guy Sprites/F00_Attack_0.bmp", 60, 50, 57, 50, 5, 15.0);
    player2.air_attackS.setHitbox((int)(player2.p.v.x*TILE_SIZE+(TILE_SIZE/2))+30, (int)(player2.p.v.y*TILE_SIZE-TILE_SIZE+375)+39, 260, 100, 19, 5, 5, 3, 3); //!! dopisać poprawne wg player_rectów

    double game_time = 0.;                                  // in-game timer (for animations and position updates)
    player.time_since_last_input = 0;                       // anim stopper for P1 (not sure if works)
    player2.time_since_last_input = 0;                      // anim stopper for P2 (not sure if works)
    steady_clock::time_point current_time = steady_clock::now();    // real time clock to base other clocks off of

    // main event loop
    while (still_playing) {
        // events
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            // stop falling after being grounded
            if (is_grounded(player, game_map)) {
                player.is_falling = false;
                player.is_falling_forward = false;
                player.is_falling_backward = false;
            }
            if (is_grounded(player2, game_map)) {
                player2.is_falling = false;
                player2.is_falling_forward = false;
                player2.is_falling_backward = false;
            }
            switch (event.type) {
                case SDL_QUIT: {
                    still_playing = false;
                    break;
                }
                case SDL_KEYDOWN: {
                    if (is_grounded(player, game_map)) {
                        // player.time_since_last_input = 0;

                        if (event.key.keysym.scancode == SDL_SCANCODE_W) {
                            // P1 jumping
                            player.a.v.y = -3200;
                            player.is_jumping = true;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_S) {
                            // P1 crouching
                            player.a.v.x = 0;
                            if (player.is_attacking) {
                                player.is_crouching = false;
                            };
                            player.is_walking_backward = false;
                            player.is_walking_forward = false;

                            player.is_crouching = true;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_A) {
                            // P1 walking backwards
                            if (player.is_attacking || player.is_crouching) {
                                player.is_walking_backward = false;
                                player.a.v.x = 0;
                            } else if (!is_grounded(player, game_map) && !player.is_attacking) {
                                player.is_walking_backward = false;
                                player.is_jumping_backward = true;
                            } else player.is_walking_backward = true;
                            player.a.v.x = -40;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_D) {
                            // P1 walking forwards
                            if (player.is_attacking || player.is_crouching) {
                                player.is_walking_forward = false;
                                player.a.v.x = 0;
                            } else if (!is_grounded(player, game_map) && !player.is_attacking) {
                                player.is_walking_forward = false;
                                player.is_jumping_forward = true;
                            } else player.is_walking_forward = true;
                            player.a.v.x = 50;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_N) {
                            // P1 ground Light
                            player.a.v.x = 0;
                            player.is_attacking = true;
                            player.currentAttack = player_t::groundL;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_M) {
                            // P1 ground Medium
                            player.v.v.x = 13;
                            player.is_attacking = true;
                            player.currentAttack = player_t::groundM;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_J) {
                            // P1 ground Heavy
                            player.v.v.x = 20;
                            player.is_attacking = true;
                            player.currentAttack = player_t::groundH;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_K) {
                            // P1 ground Special
                            player.a.v.x = 0;
                            player.is_attacking = true;
                            player.currentAttack = player_t::groundS;
                        };
                    }
                    else {
                        if (event.key.keysym.scancode == SDL_SCANCODE_N) {
                            // P1 air Light
                            player.a.v.x = 10;
                            player.is_attacking = true;
                            player.currentAttack = player_t::airL;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_M) {
                            // P1 air Medium
                            player.a.v.x = 0;
                            player.is_attacking = true;
                            player.currentAttack = player_t::airM;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_J) {
                            // P1 air Heavy
                            player.a.v.x = 0;
                            player.is_attacking = true;
                            player.currentAttack = player_t::airH;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_K) {
                            // P1 air Special
                            player.a.v.x = 30;
                            // player.a.v.y = 40000;
                            player.is_attacking = true;
                            player.currentAttack = player_t::airS;
                        };
                    }
                    if (is_grounded(player2, game_map)) {
                        // player2.time_since_last_input = 0;

                        if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
                            // P2 jumping
                            player2.a.v.y = -2500;
                            player2.is_jumping = true;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
                            // P2 crouching
                            player2.a.v.x = 0;
                            if (player2.is_attacking) {
                                player2.is_crouching = false;
                            };
                            player2.is_walking_backward = false;
                            player2.is_walking_forward = false;

                            player2.is_crouching = true;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
                            // P2 walking forwards
                            if (player2.is_attacking || player2.is_crouching) {
                                player2.is_walking_forward = false;
                                player2.a.v.x = 0;
                            } else if (!is_grounded(player2, game_map) && !player2.is_attacking) {
                                player2.is_walking_forward = false;
                                player2.is_jumping_forward = true;
                            } else player2.is_walking_forward = true;
                            player2.a.v.x = -35;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
                            // P2 walking backwards
                            if (player2.is_attacking || player2.is_crouching) {
                                player2.is_walking_backward = false;
                                player2.a.v.x = 0;
                            } else if (!is_grounded(player2, game_map) && !player2.is_attacking) {
                                player2.is_walking_backward = false;
                                player2.is_jumping_backward = true;
                            } else player2.is_walking_backward = true;
                            player2.a.v.x = 30;
                        };

                        if (event.key.keysym.scancode == SDL_SCANCODE_KP_1) {
                            // P2 ground Light
                            player2.a.v.x = 0;
                            player2.is_attacking = true;
                            player2.currentAttack = player_t::groundL;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_KP_2) {
                            // P2 ground Medium
                            player2.v.v.x = -4;
                            player2.is_attacking = true;
                            player2.currentAttack = player_t::groundM;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_KP_4) {
                            // P2 ground Heavy
                            player2.v.v.x = -9;
                            player2.is_attacking = true;
                            player2.currentAttack = player_t::groundH;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_KP_5) {
                            // P2 ground Special
                            player2.a.v.x = -20;
                            player2.a.v.y = -1050;
                            player2.is_attacking = true;
                            player2.currentAttack = player_t::groundS;
                        };
                    }
                    else {
                        if (event.key.keysym.scancode == SDL_SCANCODE_KP_1) {
                            // P2 air Light
                            player2.a.v.x = -1;
                            player2.is_attacking = true;
                            player2.currentAttack = player_t::airL;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_KP_2) {
                            // P2 air Medium
                            player2.a.v.x = -2;
                            player2.is_attacking = true;
                            player2.currentAttack = player_t::airM;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_KP_4) {
                            // P2 air Heavy
                            player2.v.v.x = -7;
                            player2.is_attacking = true;
                            player2.currentAttack = player_t::airH;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_KP_5) {
                            // P2 air Special
                            player2.v.v.x = -4;
                            player2.a.v.y = -900;
                            player2.is_attacking = true;
                            player2.currentAttack = player_t::airS;
                        };
                    }
                    // if (event.key.keysym.scancode == SDL_SCANCODE_UP) player.v.v.y = -50;
                    // // if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) player.v.v.y = 50;
                    // if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) player.v.v.x = -50;
                    // if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) player.v.v.x = 50;

                    break;
                }
                case SDL_KEYUP: {
                    // press O (capital o) to exit
                    if (event.key.keysym.scancode == SDL_SCANCODE_O) still_playing = false;

                    if (event.key.keysym.scancode == SDL_SCANCODE_W) {
                        // P1 start falling
                        player.a.v.y = 0;
                        player.is_jumping = false;
                        player.is_falling = true; //???
                    };
                    if (event.key.keysym.scancode == SDL_SCANCODE_S) {
                        // P1 stop crouching
                        player.a.v.y = 0;
                        player.is_crouching = false;
                    };
                    if (event.key.keysym.scancode == SDL_SCANCODE_A) {
                        // P1 stop walking backwards
                        player.a.v.x = 0;
                        if (!is_grounded(player, game_map)) {
                            player.is_falling_backward = true;
                        }
                        player.is_walking_backward = false;
                        player.is_jumping_backward = false;
                    };
                    if (event.key.keysym.scancode == SDL_SCANCODE_D) {
                        // P1 stop walking forwards
                        player.a.v.x = 0;
                        if (!is_grounded(player, game_map)) {
                            player.is_falling_forward = true;
                        }
                        player.is_walking_forward = false;
                        player.is_jumping_forward = false;
                    };

                    if (event.key.keysym.scancode == SDL_SCANCODE_N) {
                        // P1 stop Lighting
                        player.a.v.x = 0;
                        player.is_attacking = false;
                        player.currentAttack = player_t::NONE;
                    };
                    if (event.key.keysym.scancode == SDL_SCANCODE_M) {
                        // P1 stop Mediuming
                        player.a.v.x = 0;
                        player.is_attacking = false;
                        player.currentAttack = player_t::NONE;
                    };
                    if (event.key.keysym.scancode == SDL_SCANCODE_J) {
                        // P1 stop Heavying
                        player.a.v.x = 0;
                        player.is_attacking = false;
                        player.currentAttack = player_t::NONE;
                    };
                    if (event.key.keysym.scancode == SDL_SCANCODE_K) {
                        // P1 stop Specialing
                        player.a.v.x = 0;
                        player.is_attacking = false;
                        player.currentAttack = player_t::NONE;
                    };

                    if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
                        // P2 start falling
                        player2.a.v.y = 0;
                        player2.is_jumping = false;
                        if (is_grounded(player2, game_map)) {
                            player2.is_falling = false;
                            player2.is_falling_forward = false;
                            player2.is_falling_backward = false;
                        }
                    };
                    if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
                        // P2 stop crouching
                        player2.a.v.y = 0;
                        player2.is_crouching = false;
                    };
                    if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
                        // P2 stop walking forwards
                        player2.a.v.x = 0;
                        if (!is_grounded(player, game_map)) {
                            player2.is_falling_forward = true;
                        }
                        player2.is_walking_forward = false;
                        player2.is_jumping_forward = false;
                    };
                    if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
                        // P2 stop walking backwards
                        player2.a.v.x = 0;
                        if (!is_grounded(player, game_map)) {
                            player2.is_falling_backward = true;
                        }
                        player2.is_walking_backward = false;
                        player2.is_jumping_backward = false;
                    };
                    if (event.key.keysym.scancode == SDL_SCANCODE_KP_1) {
                        // P2 stop Lighting
                        player2.a.v.x = 0;
                        player2.is_attacking = false;
                        player2.currentAttack = player_t::NONE;
                    };
                    if (event.key.keysym.scancode == SDL_SCANCODE_KP_2) {
                        // P2 stop Mediuming
                        player2.a.v.x = 0;
                        player2.is_attacking = false;
                        player2.currentAttack = player_t::NONE;
                    };
                    if (event.key.keysym.scancode == SDL_SCANCODE_KP_4) {
                        // P2 stop Heavying
                        player2.a.v.x = 0;
                        player2.is_attacking = false;
                        player2.currentAttack = player_t::NONE;
                    };
                    if (event.key.keysym.scancode == SDL_SCANCODE_KP_5) {
                        // P2 stop Specialing
                        player2.a.v.x = 0;
                        player2.a.v.y = 0;
                        player2.is_attacking = false;
                        player2.currentAttack = player_t::NONE;
                    };

                    break;
                }
            }
        }
        // physics
        game_time += dt;                                    // add up time over time
        player.time_since_last_input += dt;
        player2.time_since_last_input += dt;

        player = update_player(player, game_map, dt);     // update player position
        player2 = update_player(player2, game_map, dt);

        // graphics
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, bg_texture.get(), NULL, NULL); // load up background
        healthBar(renderer, player, 100, 100, 700, 50, bg_color, hp_color); // load up P1 health bar
        healthBar(renderer, player2, 1800, 100, -700, 50, bg_color, hp_color); // load up P2 health bar

        draw_map(renderer, game_map, tiles_texture);            // draw map from tileset

        // P1 hurtbox
        SDL_Rect player_rect = {(int)(player.p.v.x*TILE_SIZE-(TILE_SIZE/2)), (int)(player.p.v.y*TILE_SIZE-TILE_SIZE-259), 400, 384};
        {
            int r = 0, g = 0, b = 0;
            if (is_grounded(player, game_map)) {
                r = 255;
            }
            if (is_colliding(player.p, game_map)) {
                g = 255;
            }
            SDL_SetRenderDrawColor(renderer, r,g,b, 0xFF);
        }
        // P2 hurtbox
        SDL_Rect player2_rect = {(int)(player2.p.v.x*TILE_SIZE-(TILE_SIZE/2)), (int)(player2.p.v.y*TILE_SIZE-TILE_SIZE-375), 600, 500};
        {
            int r2 = 0, g2 = 0, b2 = 0;
            if (is_grounded(player2, game_map)) {
                r2 = 255;
            }
            if (is_colliding(player2.p, game_map)) {
                g2 = 255;
            }
            SDL_SetRenderDrawColor(renderer, r2,g2,b2, 0xFF);
        }
        // very basic player collision, pushes them back into their corner little by little
        if (SDL_HasIntersection(&player_rect, &player2_rect)
            && is_grounded(player, game_map) && is_grounded(player2, game_map)) {
            player.p.v.x -= 0.08;
            player2.p.v.x += 0.08;
        } else if (SDL_HasIntersection(&player_rect, &player2_rect)
            // move them away a bit less in the air
            && !is_grounded(player, game_map) && !is_grounded(player2, game_map)) {
            player.p.v.x -= 0.02;
            player2.p.v.x += 0.02;
        }
        // render hurtboxes
        SDL_RenderDrawRect(renderer, &player_rect);
        SDL_RenderDrawRect(renderer, &player2_rect);

        // P1 idle anim
        if (!player.is_jumping && !player.is_crouching && !player.is_walking_backward && !player.is_walking_forward
         && !player.is_jumping_backward && !player.is_jumping_forward && !player.is_falling && !player.is_falling_backward
         && !player.is_falling_forward && !player.is_attacking && !player.is_hurt && !player.is_dead) {
            auto player_idle_sprite = player.idle.getSpecificFrame((int)floor(game_time / (1.0/player.idle.getAnimSpeed())) % player.idle.getFrameCount());
            SDL_RenderCopyEx(renderer, player_idle.get(), &player_idle_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);
        }
        // P1 jump anim
        if (player.is_jumping) {
            int jump_frame = (int)floor(player.time_since_last_input / (1.0/player.jump.getAnimSpeed())) % player.jump.getFrameCount();
            auto player_jump_sprite = player.jump.getSpecificFrame(jump_frame);

            if (jump_frame == player.jump.getFrameCount()) {
                // start falling after jumping
                player.is_jumping = false;
                player.is_falling = true;
            }
            SDL_RenderCopyEx(renderer, player_jump.get(), &player_jump_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);
        }
        // P1 jump forward anim
        if (player.is_jumping_forward) {
            int fjump_frame = (int)floor(player.time_since_last_input / (1.0/player.forward_jump.getAnimSpeed())) % player.forward_jump.getFrameCount();
            auto player_fjump_sprite = player.forward_jump.getSpecificFrame(fjump_frame);

            if (fjump_frame == player.forward_jump.getFrameCount()) {
                // start falling forward after jumping forward
                player.is_jumping_forward = false;
                player.is_falling_forward = true;
            }
            SDL_RenderCopyEx(renderer, player_fjump.get(), &player_fjump_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);
        }
        // P1 jumping backward
        if (player.is_jumping_backward) {
            int bjump_frame = (int)floor(player.time_since_last_input / (1.0/player.backward_jump.getAnimSpeed())) % player.backward_jump.getFrameCount();
            auto player_bjump_sprite = player.backward_jump.getSpecificFrame(bjump_frame);

            if (bjump_frame == player.backward_jump.getFrameCount()) {
                // start falling backward after jumping backward
                player.is_jumping_backward = false;
                player.is_falling_backward = true;
            }
            SDL_RenderCopyEx(renderer, player_bjump.get(), &player_bjump_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);
        }
        // P1 falling down
        if (player.is_falling) {
            auto player_fall_sprite = player.fall.getSpecificFrame((int)floor(game_time / (1.0/player.fall.getAnimSpeed())) % player.fall.getFrameCount());
            SDL_RenderCopyEx(renderer, player_fall.get(), &player_fall_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);
        }
        // P1 falling forward
        if (player.is_falling_forward) {
            auto player_ffall_sprite = player.forward_fall.getSpecificFrame((int)floor(game_time / (1.0/player.forward_fall.getAnimSpeed())) % player.forward_fall.getFrameCount());
            SDL_RenderCopyEx(renderer, player_ffall.get(), &player_ffall_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);
        }
        // P1 falling forward
        if (player.is_falling_backward) {
            auto player_bfall_sprite = player.backward_fall.getSpecificFrame((int)floor(game_time / (1.0/player.backward_fall.getAnimSpeed())) % player.backward_fall.getFrameCount());
            SDL_RenderCopyEx(renderer, player_bfall.get(), &player_bfall_sprite, &player_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
        }
        // P1 crouching
        if (player.is_crouching) {
            player.is_walking_forward = false;
            player.is_walking_backward = false;
            auto player_crouch_sprite = player.crouch.getSpecificFrame((int)floor(game_time / (1.0/player.crouch.getAnimSpeed())) % player.crouch.getFrameCount());
            SDL_RenderCopyEx(renderer, player_crouch.get(), &player_crouch_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);
        }
        // P1 walking forward
        if (player.is_walking_forward) {
            auto player_fwalk_sprite = player.forward_walk.getSpecificFrame((int)floor(game_time / (1.0/player.forward_walk.getAnimSpeed())) % player.forward_walk.getFrameCount());
            SDL_RenderCopyEx(renderer, player_fwalk.get(), &player_fwalk_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);
        }
        // P1 walking backward
        if (player.is_walking_backward) {
            auto player_bwalk_sprite = player.back_walk.getSpecificFrame((int)floor(game_time / (1.0/player.back_walk.getAnimSpeed())) % player.back_walk.getFrameCount());
            SDL_RenderCopyEx(renderer, player_bwalk.get(), &player_bwalk_sprite, &player_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
        }
        // check if P1 is attacking
        if (player.is_attacking) {
            player.is_walking_forward = false;
            player.is_walking_backward = false;
            player.is_crouching = false;
            // P1 Lighting
            if (player.currentAttack == player_t::groundL) {
                // render specific frame based on game time
                int groundL_frame = (int)floor(player.time_since_last_input / (1.0/player.attackL.getAnimSpeed())) % player.attackL.getFrameCount();
                auto player_attackL_sprite = player.attackL.getSpecificFrame(groundL_frame);

                player.attackL.showHitbox(renderer, player.time_since_last_input);
                SDL_RenderCopyEx(renderer, player_attackL.get(), &player_attackL_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);

                if (player.attackL.confirmHit(player2_rect)) {
                    player2.is_hurt = true;
                    // apply knockback
                    player2.a.v.x = 50;
                    player2.a.v.y = -50;
                    // deal damage
                    player.attackL.doDamage(player2, player2_rect);
                    if (is_grounded(player2, game_map)) {
                        // stop knockback when grounded
                        player2.a.v.x = 0;
                        player2.a.v.y = 0;
                    }
                    // I really don't want to do this for evey attack at this point, it's all the same down there
                }

                // (hopefully) stop attacking after the frames run out
                if (groundL_frame == player.attackL.getFrameCount()) {
                    player.is_attacking = false;
                }
            }
            // P1 Mediuming
            else if (player.currentAttack == player_t::groundM) {
                int groundM_frame = (int)floor(player.time_since_last_input / (1.0/player.attackM.getAnimSpeed())) % player.attackM.getFrameCount();
                auto player_attackM_sprite = player.attackM.getSpecificFrame(groundM_frame);

                player.attackM.showHitbox(renderer, player.time_since_last_input);
                SDL_RenderCopyEx(renderer, player_attackM.get(), &player_attackM_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);

                if (player.attackM.confirmHit(player2_rect)) {
                    player2.is_hurt = true;
                    player2.a.v.x = 200;
                    player2.a.v.y = -10;
                    player.attackM.doDamage(player2, player2_rect);
                    if (is_grounded(player2, game_map)) {
                        player2.a.v.x = 0;
                        player2.a.v.y = 0;
                    }
                }

                if (groundM_frame == player.attackM.getFrameCount()) {
                    player.is_attacking = false;
                }
            }
            // P1 Heavying
            else if (player.currentAttack == player_t::groundH) {
                int groundH_frame = (int)floor(player.time_since_last_input / (1.0/player.attackH.getAnimSpeed())) % player.attackH.getFrameCount();
                auto player_attackH_sprite = player.attackH.getSpecificFrame(groundH_frame);

                player.attackH.showHitbox(renderer, player.time_since_last_input);
                SDL_RenderCopyEx(renderer, player_attackH.get(), &player_attackH_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);

                if (player.attackH.confirmHit(player2_rect)) {
                    player2.is_hurt = true;
                    player2.a.v.x = 300;
                    player2.a.v.y = -30;
                    player.attackH.doDamage(player2, player2_rect);
                    if (is_grounded(player2, game_map)) {
                        player2.a.v.x = 0;
                        player2.a.v.y = 0;
                    }
                }

                if (groundH_frame == player.attackH.getFrameCount()) {
                    player.is_attacking = false;
                }
            }
            // P1 Specialing
            else if (player.currentAttack == player_t::groundS) {
                int groundS_frame = (int)floor(player.time_since_last_input / (1.0/player.attackS.getAnimSpeed())) % player.attackS.getFrameCount();
                auto player_attackS_sprite = player.attackS.getSpecificFrame(groundS_frame);

                player.attackS.showHitbox(renderer, player.time_since_last_input);
                SDL_RenderCopyEx(renderer, player_attackS.get(), &player_attackS_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);

                if (player.attackS.confirmHit(player2_rect)) {
                    player2.is_hurt = true;
                    player2.a.v.x = 100;
                    player2.a.v.y = -2000;
                    player.attackS.doDamage(player2, player2_rect);
                    if (is_grounded(player2, game_map)) {
                        player2.a.v.x = 0;
                        player2.a.v.y = 0;
                    }
                }

                if (groundS_frame == player.attackS.getFrameCount()) {
                    player.is_attacking = false;
                }
            }
            // P1 air Lighting
            else if (player.currentAttack == player_t::airL) {
                int airL_frame = (int)floor(player.time_since_last_input / (1.0/player.air_attackL.getAnimSpeed())) % player.air_attackL.getFrameCount();
                auto player_airL_sprite = player.air_attackL.getSpecificFrame(airL_frame);

                player.air_attackL.showHitbox(renderer, player.time_since_last_input);
                SDL_RenderCopyEx(renderer, player_airL.get(), &player_airL_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);

                if (player.air_attackL.confirmHit(player2_rect)) {
                    player2.is_hurt = true;
                    player2.a.v.x = 50;
                    player.air_attackL.doDamage(player2, player2_rect);
                    if (is_grounded(player2, game_map)) {
                        player2.a.v.x = 0;
                        player2.a.v.y = 0;
                    }
                }

                if (airL_frame == player.air_attackL.getFrameCount()) {
                    player.is_attacking = false;
                }
            }
            // P1 air Mediuming
            else if (player.currentAttack == player_t::airM) {
                int airM_frame = (int)floor(player.time_since_last_input / (1.0/player.air_attackM.getAnimSpeed())) % player.air_attackM.getFrameCount();
                auto player_airM_sprite = player.air_attackM.getSpecificFrame(airM_frame);

                player.air_attackM.showHitbox(renderer, player.time_since_last_input);
                SDL_RenderCopyEx(renderer, player_airM.get(), &player_airM_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);

                if (player.air_attackM.confirmHit(player2_rect)) {
                    player2.is_hurt = true;
                    player2.a.v.x = 70;
                    player2.a.v.y = 70;
                    player.air_attackM.doDamage(player2, player2_rect);
                    if (is_grounded(player2, game_map)) {
                        player2.a.v.x = 0;
                        player2.a.v.y = 0;
                    }
                }

                if (airM_frame == player.air_attackM.getFrameCount()) {
                    player.is_attacking = false;
                }
            }
            // P1 air Heavying
            else if (player.currentAttack == player_t::airH) {
                int airH_frame = (int)floor(player.time_since_last_input / (1.0/player.air_attackH.getAnimSpeed())) % player.air_attackH.getFrameCount();
                auto player_airH_sprite = player.air_attackH.getSpecificFrame(airH_frame);

                player.air_attackH.showHitbox(renderer, player.time_since_last_input);
                SDL_RenderCopyEx(renderer, player_airH.get(), &player_airH_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);

                if (player.air_attackM.confirmHit(player2_rect)) {
                    player2.is_hurt = true;
                    player2.a.v.x = 30;
                    player2.a.v.y = 500;
                    player.air_attackM.doDamage(player2, player2_rect);
                    if (is_grounded(player2, game_map)) {
                        player2.a.v.x = 0;
                        player2.a.v.y = 0;
                    }
                }

                if (airH_frame == player.air_attackH.getFrameCount()) {
                    player.is_attacking = false;
                }
            }
            // P1 air Specialing
            else if (player.currentAttack == player_t::airS) {
                int airS_frame = (int)floor(player.time_since_last_input / (1.0/player.air_attackS.getAnimSpeed())) % player.air_attackS.getFrameCount();
                auto player_airS_sprite = player.air_attackS.getSpecificFrame(airS_frame);

                player.air_attackS.showHitbox(renderer, player.time_since_last_input);
                SDL_RenderCopyEx(renderer, player_airS.get(), &player_airS_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);

                if (player.air_attackM.confirmHit(player2_rect)) {
                    player2.is_hurt = true;
                    player2.a.v.x = 10;
                    player2.a.v.y = 1200;
                    player.air_attackM.doDamage(player2, player2_rect);
                    if (is_grounded(player2, game_map)) {
                        player2.a.v.x = 0;
                        player2.a.v.y = 0;
                    }
                }

                if (airS_frame == player.air_attackS.getFrameCount()) {
                    player.is_attacking = false;
                }
            }
        }
        // P1 blocking
        // if (player.is_blocking) {
        //
        // }
        // P1 being hurt
        if (player.is_hurt) {
            auto player_hurt_sprite = player.hurt.getSpecificFrame((int)floor(game_time / (1.0/player.hurt.getAnimSpeed())) % player.hurt.getFrameCount());
            SDL_RenderCopyEx(renderer, player_hurt.get(), &player_hurt_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);
            if (is_grounded(player, game_map)) player.is_hurt = false;
        }
        // P1 dying
        if (player.is_dead) {
            auto player_dead_sprite = player.dead.getSpecificFrame((int)floor(game_time / (1.0/player.dead.getAnimSpeed())) % player.dead.getFrameCount());
            SDL_RenderCopyEx(renderer, player_dead.get(), &player_dead_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);
        }
        // P2 idle anim
        if (!player2.is_jumping && !player2.is_crouching && !player2.is_walking_backward && !player2.is_walking_forward
         && !player2.is_jumping_backward && !player2.is_jumping_forward && !player2.is_falling && !player2.is_falling_backward
         && !player2.is_falling_forward && !player2.is_attacking  && !player2.is_hurt && !player2.is_dead) {
            auto player2_idle_sprite = player2.idle.getSpecificFrame((int)floor(game_time / (1.0/player2.idle.getAnimSpeed())) % player2.idle.getFrameCount());
            SDL_RenderCopyEx(renderer, player2_idle.get(), &player2_idle_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
        }
        // P2 jumping anim
        if (player2.is_jumping) {
            int jump2_frame = (int)floor(player2.time_since_last_input / (1.0/player2.jump.getAnimSpeed())) % player2.jump.getFrameCount();
            auto player2_jump_sprite = player2.jump.getSpecificFrame(jump2_frame);

            if (jump2_frame == player2.jump.getFrameCount()) {
                player2.is_jumping = false;
                player2.is_falling = true;
            }
            SDL_RenderCopyEx(renderer, player2_jump.get(), &player2_jump_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
        }
        // P2 jumping forward anim
        if (player2.is_jumping_forward) {
            int fjump2_frame = (int)floor(player2.time_since_last_input / (1.0/player2.forward_jump.getAnimSpeed())) % player2.forward_jump.getFrameCount();
            auto player2_fjump_sprite = player2.forward_jump.getSpecificFrame(fjump2_frame);

            if (fjump2_frame == player2.forward_jump.getFrameCount()) {
                player2.is_jumping_forward = false;
                player2.is_falling_forward = true;
            }
            SDL_RenderCopyEx(renderer, player2_fjump.get(), &player2_fjump_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
        }
        // P2 jumping backward anim
        if (player2.is_jumping_backward) {
            int bjump2_frame = (int)floor(player2.time_since_last_input / (1.0/player2.backward_jump.getAnimSpeed())) % player2.backward_jump.getFrameCount();
            auto player2_bjump_sprite = player2.backward_jump.getSpecificFrame(bjump2_frame);

            if (bjump2_frame == player2.backward_jump.getFrameCount()) {
                player2.is_jumping_backward = false;
                player2.is_falling_backward = true;
            }
            SDL_RenderCopyEx(renderer, player2_bjump.get(), &player2_bjump_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
        }
        // P2 falling down
        if (player2.is_falling) {
            auto player2_fall_sprite = player2.fall.getSpecificFrame((int)floor(game_time / (1.0/player2.fall.getAnimSpeed())) % player2.fall.getFrameCount());
            SDL_RenderCopyEx(renderer, player2_fall.get(), &player2_fall_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
        }
        // P2 falling forward
        if (player2.is_falling_forward) {
            auto player2_ffall_sprite = player2.forward_fall.getSpecificFrame((int)floor(game_time / (1.0/player2.forward_fall.getAnimSpeed())) % player2.forward_fall.getFrameCount());
            SDL_RenderCopyEx(renderer, player2_ffall.get(), &player2_ffall_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
        }
        // P2 falling backward
        if (player2.is_falling_backward) {
            auto player2_bfall_sprite = player2.backward_fall.getSpecificFrame((int)floor(game_time / (1.0/player2.backward_fall.getAnimSpeed())) % player2.backward_fall.getFrameCount());
            SDL_RenderCopyEx(renderer, player2_ffall.get(), &player2_bfall_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
        }
        // P2 crouching
        if (player2.is_crouching) {
            auto player2_crouch_sprite = player2.crouch.getSpecificFrame((int)floor(game_time / (1.0/player2.crouch.getAnimSpeed())) % player2.crouch.getFrameCount());
            SDL_RenderCopyEx(renderer, player2_crouch.get(), &player2_crouch_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
        }
        // P2 walking forward
        if (player2.is_walking_forward) {
            auto player2_fwalk_sprite = player2.forward_walk.getSpecificFrame((int)floor(game_time / (1.0/player2.forward_walk.getAnimSpeed())) % player2.forward_walk.getFrameCount());
            SDL_RenderCopyEx(renderer, player2_fwalk.get(), &player2_fwalk_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
        }
        // P2 walking backward
        if (player2.is_walking_backward) {
            auto player2_bwalk_sprite = player2.back_walk.getSpecificFrame((int)floor(game_time / (1.0/player2.back_walk.getAnimSpeed())) % player2.back_walk.getFrameCount());
            SDL_RenderCopyEx(renderer, player2_bwalk.get(), &player2_bwalk_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
        }
        // check if P2 is attacking
        if (player2.is_attacking) {
            player2.is_walking_forward = false;
            player2.is_walking_backward = false;
            player2.is_crouching = false;
            // P2 Lighting
            if (player2.currentAttack == player_t::groundL) {
                int groundL2_frame = (int)floor(player2.time_since_last_input / (1.0/player2.attackL.getAnimSpeed())) % player2.attackL.getFrameCount();
                auto player2_attackL_sprite = player2.attackL.getSpecificFrame(groundL2_frame);

                player2.attackL.showHitbox(renderer, player2.time_since_last_input);
                SDL_RenderCopyEx(renderer, player2_attackL.get(), &player2_attackL_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);

                if (player2.attackL.confirmHit(player_rect)) {
                    player.is_hurt = true;
                    player.a.v.x = -40;
                    player.a.v.y = -10;
                    player2.attackL.doDamage(player, player_rect);
                    if (is_grounded(player, game_map)) {
                        player.a.v.x = 0;
                        player.a.v.y = 0;
                    }
                }

                if (groundL2_frame == player2.attackL.getFrameCount()) {
                    player2.is_attacking = false;
                }
            }
            // P2 Mediuming
            else if (player2.currentAttack == player_t::groundM) {
                int groundM2_frame = (int)floor(player2.time_since_last_input / (1.0/player2.attackM.getAnimSpeed())) % player2.attackM.getFrameCount();
                auto player2_attackM_sprite = player2.attackM.getSpecificFrame(groundM2_frame);

                player2.attackM.showHitbox(renderer, player2.time_since_last_input);
                SDL_RenderCopyEx(renderer, player2_attackM.get(), &player2_attackM_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);

                if (player2.attackM.confirmHit(player_rect)) {
                    player.is_hurt = true;
                    player.a.v.x = -40;
                    player.a.v.y = -200;
                    player2.attackM.doDamage(player, player_rect);
                    if (is_grounded(player, game_map)) {
                        player.a.v.x = 0;
                        player.a.v.y = 0;
                    }
                }

                if (groundM2_frame == player2.attackM.getFrameCount()) {
                    player2.is_attacking = false;
                }
            }
            // P2 Heavying
            else if (player2.currentAttack == player_t::groundH) {
                int groundH2_frame = (int)floor(player2.time_since_last_input / (1.0/player2.attackH.getAnimSpeed())) % player2.attackH.getFrameCount();
                auto player2_attackH_sprite = player2.attackH.getSpecificFrame(groundH2_frame);

                player2.attackH.showHitbox(renderer, player2.time_since_last_input);
                SDL_RenderCopyEx(renderer, player2_attackH.get(), &player2_attackH_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);

                if (player2.attackH.confirmHit(player_rect)) {
                    player.is_hurt = true;
                    player.a.v.x = -400;
                    player.a.v.y = -30;
                    player2.attackH.doDamage(player, player_rect);
                    if (is_grounded(player, game_map)) {
                        player.a.v.x = 0;
                        player.a.v.y = 0;
                    }
                }

                if (groundH2_frame == player2.attackH.getFrameCount()) {
                    player2.is_attacking = false;
                }
            }
            // P2 Specialing
            else if (player2.currentAttack == player_t::groundS) {
                int groundS2_frame = (int)floor(player2.time_since_last_input / (1.0/player2.attackS.getAnimSpeed())) % player2.attackS.getFrameCount();
                auto player2_attackS_sprite = player2.attackS.getSpecificFrame(groundS2_frame);

                player2.attackS.showHitbox(renderer, player2.time_since_last_input);
                SDL_RenderCopyEx(renderer, player2_attackS.get(), &player2_attackS_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);

                if (player2.attackS.confirmHit(player_rect)) {
                    player.is_hurt = true;
                    player.a.v.x = -80;
                    player.a.v.y = -1200;
                    player2.attackS.doDamage(player, player_rect);
                    if (is_grounded(player, game_map)) {
                        player.a.v.x = 0;
                        player.a.v.y = 0;
                    }
                }

                if (groundS2_frame == player2.attackS.getFrameCount()) {
                    player2.is_attacking = false;
                }
            }
            // P2 air Lighting
            else if (player2.currentAttack == player_t::airL) {
                int airL2_frame = (int)floor(player2.time_since_last_input / (1.0/player2.air_attackL.getAnimSpeed())) % player2.air_attackL.getFrameCount();
                auto player2_airL_sprite = player2.air_attackL.getSpecificFrame(airL2_frame);

                player2.air_attackL.showHitbox(renderer, player2.time_since_last_input);
                SDL_RenderCopyEx(renderer, player2_airL.get(), &player2_airL_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);

                if (player2.air_attackL.confirmHit(player_rect)) {
                    player.is_hurt = true;
                    player.a.v.x = -40;
                    player.a.v.y = -10;
                    player2.air_attackL.doDamage(player, player_rect);
                    if (is_grounded(player, game_map)) {
                        player.a.v.x = 0;
                        player.a.v.y = 0;
                    }
                }

                if (airL2_frame == player2.air_attackL.getFrameCount()) {
                    player2.is_attacking = false;
                }
            }
            // P2 air Mediuming
            else if (player2.currentAttack == player_t::airM) {
                int airM2_frame = (int)floor(player2.time_since_last_input / (1.0/player2.air_attackM.getAnimSpeed())) % player2.air_attackM.getFrameCount();
                auto player2_airM_sprite = player2.air_attackM.getSpecificFrame(airM2_frame);

                player2.air_attackM.showHitbox(renderer, player2.time_since_last_input);
                SDL_RenderCopyEx(renderer, player2_airM.get(), &player2_airM_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);

                if (player2.air_attackM.confirmHit(player_rect)) {
                    player.is_hurt = true;
                    player.a.v.x = -40;
                    player.a.v.y = -300;
                    player2.air_attackM.doDamage(player, player_rect);
                    if (is_grounded(player, game_map)) {
                        player.a.v.x = 0;
                        player.a.v.y = 0;
                    }
                }

                if (airM2_frame == player2.air_attackM.getFrameCount()) {
                    player2.is_attacking = false;
                }
            }
            // P2 air Heavying
            else if (player2.currentAttack == player_t::airH) {
                int airH2_frame = (int)floor(player2.time_since_last_input / (1.0/player2.air_attackH.getAnimSpeed())) % player2.air_attackH.getFrameCount();
                auto player2_airH_sprite = player2.air_attackH.getSpecificFrame(airH2_frame);

                player2.air_attackH.showHitbox(renderer, player2.time_since_last_input);
                SDL_RenderCopyEx(renderer, player2_airH.get(), &player2_airH_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);

                if (player2.air_attackH.confirmHit(player_rect)) {
                    player.is_hurt = true;
                    player.a.v.x = -400;
                    player.a.v.y = -10;
                    player2.air_attackH.doDamage(player, player_rect);
                    if (is_grounded(player, game_map)) {
                        player.a.v.x = 0;
                        player.a.v.y = 0;
                    }
                }

                if (airH2_frame == player2.air_attackH.getFrameCount()) {
                    player2.is_attacking = false;
                }
            }
            // P2 air Specialing
            else if (player2.currentAttack == player_t::airS) {
                int airS2_frame = (int)floor(player2.time_since_last_input / (1.0/player2.air_attackS.getAnimSpeed())) % player2.air_attackS.getFrameCount();
                auto player2_airS_sprite = player2.air_attackS.getSpecificFrame(airS2_frame);

                player2.air_attackS.showHitbox(renderer, player2.time_since_last_input);
                SDL_RenderCopyEx(renderer, player2_airS.get(), &player2_airS_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);

                if (player2.air_attackS.confirmHit(player_rect)) {
                    player.is_hurt = true;
                    player.a.v.x = -40;
                    player.a.v.y = 500;
                    player2.air_attackS.doDamage(player, player_rect);
                    if (is_grounded(player, game_map)) {
                        player.a.v.x = 0;
                        player.a.v.y = 0;
                    }
                }

                if (airS2_frame == player2.air_attackS.getFrameCount()) {
                    player2.is_attacking = false;
                }
            }
        }
        // P2 blocking
        // if (player2.is_blocking) {
        //
        // }
        // P2 being hurt
        if (player2.is_hurt) {
            auto player2_hurt_sprite = player2.hurt.getSpecificFrame((int)floor(game_time / (1.0/player2.hurt.getAnimSpeed())) % player2.hurt.getFrameCount());
            SDL_RenderCopyEx(renderer, player2_hurt.get(), &player2_hurt_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
            if (is_grounded(player2, game_map)) player2.is_hurt = false;
        }
        // P2 dying
        if (player2.is_dead) {
            auto player2_dead_sprite = player2.dead.getSpecificFrame((int)floor(game_time / (1.0/player2.dead.getAnimSpeed())) % player2.dead.getFrameCount());
            SDL_RenderCopyEx(renderer, player2_dead.get(), &player2_dead_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
        }


        // SDL_RenderDrawLine(renderer, 0, 0, x, y);                // linia podążająca za playerem
        // SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 0xff);
        SDL_RenderPresent(renderer);                                // actually present everything on screen

        // delays
        current_time = current_time + microseconds((long long int)(dt*1000000.0)); // stable framerate stuff (I think)
        std::this_thread::sleep_until(current_time);

    }

    SDL_DestroyRenderer(renderer);                                  // KILL the renderer when you're done
    SDL_DestroyWindow(window);                                      // KILL the display window as well

    SDL_Quit();                                                     // quit the program

    return 0;
}
