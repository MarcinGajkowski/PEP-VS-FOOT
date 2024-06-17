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

#define TILE_SIZE 64

struct game_map_t {
    int width, height;
    std::vector<int> map;
    int get(int x, int y) const {
        if (x < 0 || x >= width || y < 0 || y >= height) return 1;  // ściany out of bounds
        return map[y*width + x];                                    // zapisanie jako bitmap, y to dany rząd tabeli, x to która kolumna
    }
};

game_map_t game_map = {30, 17,
    { // 30*17 zer w wektorze, 1-ki to cube, 2-ki to square
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

// class LAnimatedTexture
// {
// public:
//     LAnimatedTexture();                     //Initializes variables
//
//     ~LAnimatedTexture();                    //Deallocates memory
//
//     std::shared_ptr<SDL_Texture> load_image(SDL_Renderer *renderer, const std::string &filename);   //Loads image at specified path
//
//     void free();                            //Deallocates texture
//
//     void render(SDL_Renderer *renderer, const std::string &filename, int width, int height,  int frames, int frameDelay);   //Renders texture at given point
//
//     int getWidth();                         //Gets image dimensions
//     int getHeight();
//
//     SDL_Texture* getTexture();              // HOPEFULLY gets the hardware texture
//
// private:
//     SDL_Texture* mTexture;                  //The actual hardware texture
//
//     int mWidth;                             //Image dimensions
//     int mHeight;
// };
//
// LAnimatedTexture::LAnimatedTexture()
// {
//     mTexture = nullptr;
//     mWidth = 0;
//     mHeight = 0;
// }
//
// LAnimatedTexture::~LAnimatedTexture()
// {
//     free();
// }

// void LAnimatedTexture::free()
// {
//     if( mTexture != nullptr )              // Free texture if it exists
//     {
//         SDL_DestroyTexture( mTexture );
//         mTexture = nullptr;
//         mWidth = 0;
//         mHeight = 0;
//     }
// }

class Move {
public:
    std::shared_ptr<SDL_Texture> load_sheet(SDL_Renderer *renderer, const std::string &filename, int width, int height,
                                              int hurtboxW, int hurtboxH, int frames, double speed);
    SDL_Rect getSpecificFrame(int frame);
    int getFrameCount() const;
    double getAnimSpeed() const;
    SDL_Rect getHitbox();
    void setHitbox(SDL_Renderer *renderer, int x, int y, int w, int h, int onHit, int onBlock, int hitstun);
    // Move() = default;
    // Move(Move* m, const int hit, const int block, const int hitstun, SDL_Rect frames[]) {
    //     damage_on_hit = hit;
    //     damage_on_block = block;
    //     hitstun_frames = hitstun;
    //     m = malloc(sizeof(*Move) + sizeof(SDL_Rect) * );
    //
    // }
private:
    SDL_Texture* sprite_sheet = {};
    int frame_count = 0;
    double anim_speed = 0;
    int damage_on_hit = 0;
    int damage_on_block = 0;
    int hitstun_frames = 0;
    SDL_Rect hitbox = {};
    std::vector<SDL_Rect> frame_list;
};

std::shared_ptr<SDL_Texture> Move::load_sheet(SDL_Renderer *renderer, const std::string &filename, int width, int height,
                                            int hurtboxW, int hurtboxH, int frames, double speed) {
    SDL_Surface *surface = SDL_LoadBMP(filename.c_str());
    if (!surface) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Couldn't create surface from image: %s", SDL_GetError(), NULL);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create surface from image: %s", SDL_GetError());
        throw std::invalid_argument(SDL_GetError());
    }
    SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 0, 255, 255)); // turns cyan transparent
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Couldn't create texture from surface: %s", SDL_GetError(), NULL);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture from surface: %s", SDL_GetError());
        throw std::invalid_argument(SDL_GetError());
    }
    sprite_sheet = texture;
    frame_count = frames;
    anim_speed = speed;
    // frame_list.resize(frames);
    // SDL_Rect renderQuad = { 0, 0, sprite_sheet->w, sprite_sheet->h };

    for (int i = 0; i < texture->h/height; i++) {
        for (int j = 0; j < texture->w/width; j++) {
            if (i*(texture->w/width) + j == frames) break;
            SDL_Rect frame = { j*width, i*height, hurtboxW, hurtboxH };
            // frame_list.insert(frame_list.at(i*(texture->w/width) + j), frame);
            frame_list.push_back(frame);            // put frame from sprite sheet onto the frame list
        }
    }

    // renderQuad.x = frame_list[ idx ].x;
    // renderQuad.y = frame_list[ idx ].y;
    // renderQuad.w = frame_list[ idx ].w;
    // renderQuad.h = frame_list[ idx ].h;

    // SDL_RenderPresent(renderer);
    // ++currentFrame;
    // if (currentFrame/frameDelay >= frames) {
    //     currentFrame = 0;
    // }
    SDL_FreeSurface(surface);
    return {texture, [](SDL_Texture *t) { SDL_DestroyTexture(t); } };
}

SDL_Rect Move::getSpecificFrame(int frame) {
    return frame_list.at(frame);
}

int Move::getFrameCount() const {
    return frame_count;
}

double Move::getAnimSpeed() const {
    return anim_speed;
}

SDL_Rect Move::getHitbox() {
    return hitbox;
}

void Move::setHitbox(SDL_Renderer *renderer, int x, int y, int w, int h, int onHit, int onBlock, int hitstun) {
    hitbox.x = x;
    hitbox.y = y;
    hitbox.w = w;
    hitbox.h = h;
    damage_on_hit = onHit;
    damage_on_block = onBlock;
    hitstun_frames = hitstun;
    SDL_SetRenderDrawColor(renderer, 255,0,0, 0xFF);
    SDL_RenderFillRect(renderer, &hitbox);
}

// int LAnimatedTexture::getWidth() {                                // ??? we'll see
//     return mWidth;
// }
//
// int LAnimatedTexture::getHeight() {
//     return mHeight;
// }
//
// SDL_Texture* LAnimatedTexture::getTexture() {
//     return mTexture;
// }

union vect_t {                                     // można użyć jako albo [x,y] albo tablicę 2-wymiarową
    struct { double x; double y;} v;
    double p[2];
};

vect_t operator+(const vect_t a, const vect_t b) { // dodawanie wektorów
    vect_t ret = a;
    ret.v.x += b.v.x;
    ret.v.y += b.v.y;
    return ret;
}

vect_t operator*(const vect_t a, const double b) { // mnożenie wektorów przez skalar
    vect_t ret = a;
    ret.v.x *= b;
    ret.v.y *= b;
    return ret;
}

struct player_t {
    vect_t p; // position
    vect_t v; // velocity
    vect_t a; // acceleration
    int health = 100;
    // int num;
    // enum character { PEPPINO, FOOTSIES };
    double time_since_last_input;
    bool is_attacking;
    bool is_blocking;
    bool is_crouching;
    bool is_jumping;
    bool is_jumping_forward;
    bool is_jumping_backward;
    bool is_falling;
    bool is_falling_forward;
    bool is_falling_backward;
    // bool is_walking;
    bool is_walking_backward;
    bool is_walking_forward;

    enum AttackType {
        NONE = 0,
        groundL = 1, groundM, groundH, groundS,
        airL, airM, airH, airS,
    };
    Move idle{};
    Move forward_walk{};
    Move back_walk{};
    Move jump{};
    Move fall{};
    Move forward_jump{};
    Move backward_jump{};
    Move forward_fall{};
    Move backward_fall{};
    Move crouch{};

    AttackType currentAttack = NONE;
    Move attackL{};
    Move attackM{};
    Move attackH{};
    Move attackS{};
    Move air_attackL{};
    Move air_attackM{};
    Move air_attackH{};
    Move air_attackS{};
    // Move block{};
    // Move block_low{};
    // Move hurt{};
    // Move dead{};
};

// struct player_t* createPlayer(struct player_t *pl, vect_t p, vect_t v, vect_t a) {
//     pl = static_cast<player_t *>(malloc(sizeof(*pl) + sizeof(Move) * 16) + sizeof(double) * 6);
//
//     pl->p = p;
//     pl->v = v;
//     pl->a = a;
//     return pl;
// }


bool is_colliding(vect_t position, const game_map_t &map) {
    return map.get(position.v.x, position.v.y) > 0;
}

// bool is_colliding_with_opp(player_t player, const game_map_t &map) {
//     return map.get(player.p.v.x, player.p.v.y) > 0;
// }

bool is_grounded(player_t player, const game_map_t &map) { // opp collision??? maybe??
    return map.get(player.p.v.x, player.p.v.y+0.01) > 0;
}

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
            player_old.a.v.y = 1000;
        } else {
            player_old.a.v.y = 50;  // przyspieszenie ziemskie
            player_old.a.v.x = player_old.a.v.x * 0.9;
        }
    }

    player.p = player_old.p + (player_old.v * dt) + (player_old.a*dt*dt)*0.5;
    player.v = player_old.v + (player_old.a * dt);
    player.v = player.v * 0.93; // zwalnianie (prymitywne)

    std::vector<vect_t> collision_points = {
        {{-0.4,0.0}}, // lewy dolny
        {{0.4,0.0}},   // prawy dolny
    //     {{-0.4,2.0}},   // lewy górny
    //     {{0.4,2.0}},   // prawy górny
    //     {{1.0,0.5}},   // po prawej na dole
    //     {{-1.0,0.5}}, // po lewej na dole
    //     {{1.0,1.5}}, // po prawej na górze
    //     {{-1.0,1.5}}  // po lewej na dole
    };
    std::vector<vect_t> collision_mods = {
        {{0.0,-1.0}}, // podwyższa?
        {{0.0,-1.0}},
    //     {{0.0,1.0}},   // obniża?
    //     {{0.0,1.0}},
    //     {{-1.0,0.0}},  // pcha do lewej
    //     {{1.0,0.0}},  // pcha do prawej
    //     {{-1.0,0.0}},  // pcha do lewej
    //     {{1.0,0.0}}  // pcha do prawej
    };

    for (int i = 0; i < collision_points.size(); i++) {
        auto test_point = player.p + collision_points[i];

        if (is_colliding(test_point, map)) {
            if (collision_mods[i].v.y < 0) {
                player.v.v.y = 0;
                player.p.v.y = player_old.p.v.y;
            }
        }
        // if (is_colliding_with_opp(player, map)) { // ????
        //     if (collision_mods[i].v.x < 0) {
        //         player.v.v.x = 0;
        //         player.p.v.x = player_old.p.v.x;
        //     }
        // }
    }
    // if (is_colliding(player, map)) {
    //     auto p_x = player;
    //     auto p_y = player;
    //     p_x.p.v.y = 0;
    //     p_y.p.v.x = 0;
    //     player.p = player_old.p;
    //
    //     if (!is_colliding(p_x, map)) {
    //         if (player_old.v.v.y > 0) player.p.v.y = (double)(((int)player.p.v.y) + 1) - 0.01;
    //         player.v.v.y = 0;
    //         player.a.v.y = 0;
    //     }
    //     if (!is_colliding(p_y, map)) {
    //         player.v.v.x = 0;
    //         player.a.v.x = 0;
    //     }
    // }
    if (player.p.v.y < 0) {
        player.p.v.y = 0;
        player.v.v.y = 0;
    }
    if (player.p.v.y > 17) {
        player.p.v.y = 17;
        player.v.v.y = 0;
    }
    if (player.p.v.x < 0) {
        player.p.v.x = 0;
        player.v.v.x = 0;
    }
    if (player.p.v.x > 25) {
        player.p.v.x = 25;
        player.v.v.x = 0;
    }
    return player;
}

void draw_map(SDL_Renderer *renderer, game_map_t & map, const std::shared_ptr<SDL_Texture>& tex) {
    for (int y = map.height - 1; y >= 0; y--) {
        for (int x = 0; x < map.width; x++) {
            SDL_Rect dst = {x*TILE_SIZE, y*TILE_SIZE, TILE_SIZE*2, TILE_SIZE*2}; // 64x64 px, rozmiar całego tile'a to 128x128 px
            // SDL_SetRenderDrawColor(renderer, 255, 64, 64, 255);
            if (map.get(x,y) > 0) {
                // SDL_RenderDrawRect(renderer, &dst);
                SDL_Rect src = {128*(map.get(x,y) - 1), 0, TILE_SIZE*2, TILE_SIZE*2}; // szerokość *
                SDL_RenderCopy(renderer, tex.get(), &src, &dst);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    using namespace std::chrono_literals;
    using namespace std::chrono;
    using namespace std;
    SDL_Window *window = SDL_CreateWindow("PEP VS. FOOT",  SDL_WINDOWPOS_CENTERED,  SDL_WINDOWPOS_CENTERED, 1280, 720,
                                            SDL_WINDOW_FULLSCREEN_DESKTOP);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    double dt = 1./60.;
    // Uint32 ticks = SDL_GetTicks();
    // const Uint32 anim_speed = ticks / 100;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Couldn't initialize SDL: %s", SDL_GetError(), NULL);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        return 3;
    }

    /*if (SDL_CreateWindowAndRenderer(1280, 720, SDL_WINDOW_RESIZABLE, &window, &renderer) < 0) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Couldn't create window and renderer: %s", SDL_GetError(), NULL);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
        return 3;
    }*/

    if (!window) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Couldn't create window: %s", SDL_GetError(), NULL);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window: %s", SDL_GetError());
        return 3;
    }

    if (!renderer) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Couldn't create renderer: %s", SDL_GetError(), NULL);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create renderer: %s", SDL_GetError());
        return 3;
    }

    // auto player_texture = load_image(renderer, "player.bmp");
    // auto clouds_texture = load_image(renderer, "clouds.bmp");
    auto tiles_texture = load_image(renderer, "tiles.bmp");
    auto bg_texture = load_image(renderer, "background.bmp");

    bool still_playing = true;
    player_t player;
    // player.PEPPINO;

    player.p.v.x = 0;
    player.p.v.y = 4;
    player.a.v.x = 0;
    player.a.v.y = 0;
    player.v.v.x = 0;
    player.v.v.y = 0;

    player_t player2;
    // player2.FOOTSIES;

    player2.p.v.x = 23;
    player2.p.v.y = 4;
    player2.a.v.x = 0;
    player2.a.v.y = 0;
    player2.v.v.x = 0;
    player2.v.v.y = 0;

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

    auto player_attackL = player.attackL.load_sheet(renderer, "PEPPINO Sprites/slap/slap.bmp", 200, 100, 130, 96, 12, 20.0);
    auto player_attackM = player.attackM.load_sheet(renderer, "PEPPINO Sprites/tackle/tackle.bmp", 100, 100, 98, 96, 8, 20.0);
    auto player_attackH = player.attackH.load_sheet(renderer, "PEPPINO Sprites/backkick/backkick.bmp", 100, 100, 100, 96, 8, 20.0);
    auto player_attackS = player.attackS.load_sheet(renderer, "PEPPINO Sprites/uppunch/uppunch.bmp", 100, 100, 100, 96, 7, 20.0);
    auto player_airL = player.air_attackL.load_sheet(renderer, "PEPPINO Sprites/machpunch/machpunch.bmp", 100, 100, 95, 96, 10, 20.0);
    auto player_airM = player.air_attackM.load_sheet(renderer, "PEPPINO Sprites/slapup/slapup.bmp", 200, 200, 150, 155, 7, 20.0);
    auto player_airH = player.air_attackH.load_sheet(renderer, "PEPPINO Sprites/knock/knock.bmp", 100, 100, 85, 96, 7, 20.0);
    auto player_airS = player.air_attackS.load_sheet(renderer, "PEPPINO Sprites/shoulder/shoulder.bmp", 100, 100, 80, 96, 7, 20.0);

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

    auto player2_attackL = player2.attackL.load_sheet(renderer, "FOOTSIES Guy Sprites/F00_Attack_2.bmp", 60, 50, 55, 50, 5, 15.0);
    auto player2_attackM = player2.attackM.load_sheet(renderer, "FOOTSIES Guy Sprites/F00_Attack_0.bmp", 60, 50, 57, 50, 5, 15.0);
    auto player2_attackH = player2.attackH.load_sheet(renderer, "FOOTSIES Guy Sprites/F00_Attack_1.bmp", 60, 50, 60, 50, 8, 10.0);
    auto player2_attackS = player2.attackS.load_sheet(renderer, "FOOTSIES Guy Sprites/F00_Attack_3.bmp", 60, 50, 50, 50, 7, 8.0);
    auto player2_airL = player2.air_attackL.load_sheet(renderer, "FOOTSIES Guy Sprites/F00_Attack_2.bmp", 60, 50, 55, 50, 5, 15.0);
    auto player2_airM = player2.air_attackM.load_sheet(renderer, "FOOTSIES Guy Sprites/F00_Attack_0.bmp", 60, 50, 57, 50, 5, 15.0);
    auto player2_airH = player2.air_attackH.load_sheet(renderer, "FOOTSIES Guy Sprites/F00_Attack_1.bmp", 60, 50, 60, 50, 8, 10.0);
    auto player2_airS = player2.air_attackS.load_sheet(renderer, "FOOTSIES Guy Sprites/F00_Attack_3.bmp", 60, 50, 50, 50, 7, 8.0);

    double game_time = 0.;
    player.time_since_last_input = 0;
    player2.time_since_last_input = 0;
    steady_clock::time_point current_time = steady_clock::now();
    while (still_playing) {
        // events
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
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
                            player.a.v.y = -3200;
                            player.is_jumping = true;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_S) {
                            player.a.v.x = 0;
                            if (player.is_attacking) {
                                player.is_crouching = false;
                            };
                            player.is_walking_backward = false;
                            player.is_walking_forward = false;

                            player.is_crouching = true;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_A) {
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
                            player.a.v.x = 0;
                            player.is_attacking = true;
                            player.currentAttack = player_t::groundL;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_M) {
                            player.v.v.x = 13;
                            player.is_attacking = true;
                            player.currentAttack = player_t::groundM;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_J) {
                            player.v.v.x = 20;
                            player.is_attacking = true;
                            player.currentAttack = player_t::groundH;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_K) {
                            player.a.v.x = 0;
                            player.is_attacking = true;
                            player.currentAttack = player_t::groundS;
                        };
                    }
                    else {
                        if (event.key.keysym.scancode == SDL_SCANCODE_N) {
                            player.a.v.x = 10;
                            player.is_attacking = true;
                            player.currentAttack = player_t::airL;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_M) {
                            player.a.v.x = 0;
                            player.is_attacking = true;
                            player.currentAttack = player_t::airM;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_J) {
                            player.a.v.x = 0;
                            player.is_attacking = true;
                            player.currentAttack = player_t::airH;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_K) {
                            player.a.v.x = 30;
                            // player.a.v.y = 40000;
                            player.is_attacking = true;
                            player.currentAttack = player_t::airS;
                        };
                    }
                    if (is_grounded(player2, game_map)) {
                        // player2.time_since_last_input = 0;

                        if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
                            player2.a.v.y = -2500;
                            player2.is_jumping = true;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
                            player2.a.v.x = 0;
                            if (player2.is_attacking) {
                                player2.is_crouching = false;
                            };
                            player2.is_walking_backward = false;
                            player2.is_walking_forward = false;

                            player2.is_crouching = true;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
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
                            player2.a.v.x = 0;
                            player2.is_attacking = true;
                            player2.currentAttack = player_t::groundL;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_KP_2) {
                            player2.a.v.x = -4;
                            player2.is_attacking = true;
                            player2.currentAttack = player_t::groundM;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_KP_4) {
                            player2.a.v.x = -16;
                            player2.is_attacking = true;
                            player2.currentAttack = player_t::groundH;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_KP_5) {
                            player2.a.v.x = -20;
                            player2.a.v.y = -2050;
                            player2.is_attacking = true;
                            player2.currentAttack = player_t::groundS;
                        };
                    }
                    else {
                        if (event.key.keysym.scancode == SDL_SCANCODE_KP_1) {
                            player2.a.v.x = -1;
                            player2.is_attacking = true;
                            player2.currentAttack = player_t::airL;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_KP_2) {
                            player2.a.v.x = -2;
                            player2.is_attacking = true;
                            player2.currentAttack = player_t::airM;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_KP_4) {
                            player2.a.v.x = -5;
                            player2.is_attacking = true;
                            player2.currentAttack = player_t::airH;
                        };
                        if (event.key.keysym.scancode == SDL_SCANCODE_KP_5) {
                            player2.a.v.x = -12;
                            player2.a.v.y = -1800;
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
                    if (event.key.keysym.scancode == SDL_SCANCODE_O) still_playing = false;

                    if (event.key.keysym.scancode == SDL_SCANCODE_W) {
                        // if (player.a.v.y == 50) {
                        //     player.is_falling = true;
                        // }
                        player.a.v.y = 0;
                        // if (player.is_jumping_forward) {
                        //     player.is_jumping_forward = false;
                        // } else if (player.is_jumping_backward) {
                        //     player.is_jumping_backward = false;
                        // }
                        player.is_jumping = false;
                        player.is_falling = true; //???
                    };
                    if (event.key.keysym.scancode == SDL_SCANCODE_S) {
                        player.a.v.y = 0;
                        player.is_crouching = false;
                    };
                    if (event.key.keysym.scancode == SDL_SCANCODE_A) {
                        player.a.v.x = 0;
                        if (!is_grounded(player, game_map)) {
                            player.is_falling_backward = true;
                        }
                        player.is_walking_backward = false;
                        player.is_jumping_backward = false;
                    };
                    if (event.key.keysym.scancode == SDL_SCANCODE_D) {
                        player.a.v.x = 0;
                        if (!is_grounded(player, game_map)) {
                            player.is_falling_forward = true;
                        }
                        player.is_walking_forward = false;
                        player.is_jumping_forward = false;
                    };

                    if (event.key.keysym.scancode == SDL_SCANCODE_N) {
                        player.a.v.x = 0;
                        player.is_attacking = false;
                        player.currentAttack = player_t::NONE;
                    };
                    if (event.key.keysym.scancode == SDL_SCANCODE_M) {
                        player.a.v.x = 0;
                        player.is_attacking = false;
                        player.currentAttack = player_t::NONE;
                    };
                    if (event.key.keysym.scancode == SDL_SCANCODE_J) {
                        player.a.v.x = 0;
                        player.is_attacking = false;
                        player.currentAttack = player_t::NONE;
                    };
                    if (event.key.keysym.scancode == SDL_SCANCODE_K) {
                        player.a.v.x = 0;
                        player.is_attacking = false;
                        player.currentAttack = player_t::NONE;
                    };

                    if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
                        // if (!is_grounded(player2, game_map) && !player2.is_jumping) {
                        //     // player2.is_falling = true;
                        // }
                        player2.a.v.y = 0;
                        // if (player2.is_jumping_forward) {
                        //     player2.is_jumping_forward = false;
                        //     player2.is_falling_forward = true;
                        // } else if (player2.is_jumping_backward) {
                        //     player2.is_jumping_backward = false;
                        //     player2.is_falling_backward = true;
                        // }
                        player2.is_jumping = false;
                        if (is_grounded(player2, game_map)) {
                            player2.is_falling = false;
                            player2.is_falling_forward = false;
                            player2.is_falling_backward = false;
                        }
                    };
                    if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
                        player2.a.v.y = 0;
                        player2.is_crouching = false;
                    };
                    if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
                        player2.a.v.x = 0;
                        if (!is_grounded(player, game_map)) {
                            player2.is_falling_forward = true;
                        }
                        player2.is_walking_forward = false;
                        player2.is_jumping_forward = false;
                    };
                    if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
                        player2.a.v.x = 0;
                        if (!is_grounded(player, game_map)) {
                            player2.is_falling_backward = true;
                        }
                        player2.is_walking_backward = false;
                        player2.is_jumping_backward = false;
                    };
                    if (event.key.keysym.scancode == SDL_SCANCODE_KP_1) {
                        player2.a.v.x = 0;
                        player2.is_attacking = false;
                        player2.currentAttack = player_t::NONE;
                    };
                    if (event.key.keysym.scancode == SDL_SCANCODE_KP_2) {
                        player2.a.v.x = 0;
                        player2.is_attacking = false;
                        player2.currentAttack = player_t::NONE;
                    };
                    if (event.key.keysym.scancode == SDL_SCANCODE_KP_4) {
                        player2.a.v.x = 0;
                        player2.is_attacking = false;
                        player2.currentAttack = player_t::NONE;
                    };
                    if (event.key.keysym.scancode == SDL_SCANCODE_KP_5) {
                        player2.a.v.x = 0;
                        player2.a.v.y = 0;
                        player2.is_attacking = false;
                        player2.currentAttack = player_t::NONE;
                    };
                    // if (event.key.keysym.scancode == SDL_SCANCODE_UP) player.v.v.y = -0;
                    // // if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) player.v.v.y = 0;
                    // if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) player.v.v.x = -0;
                    // if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) player.v.v.x = 0;

                    break;
                }
                // case SDL_MOUSEMOTION: {
                //     x = event.motion.x;
                //     y = event.motion.y;
                //     break;
                // }
                // default: ;
            }
        }
        // physics
        game_time += dt;
        player.time_since_last_input += dt;
        player2.time_since_last_input += dt;

        player = update_player(player, game_map, dt);
        player2 = update_player(player2, game_map, dt);

        // graphics
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, bg_texture.get(), NULL, NULL);
        // SDL_Rect clouds_rect = { x/5, y/5,1280,720};
        // SDL_RenderCopyEx(renderer, clouds_texture.get(), NULL, &clouds_rect, game_time*10, NULL, SDL_FLIP_NONE);

        draw_map(renderer, game_map, tiles_texture);

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
        SDL_RenderDrawRect(renderer, &player_rect);
        SDL_RenderDrawRect(renderer, &player2_rect);

        if (!player.is_jumping && !player.is_crouching && !player.is_walking_backward
        && !player.is_walking_forward && !player.is_jumping_backward && !player.is_jumping_forward
        && !player.is_falling && !player.is_falling_backward && !player.is_falling_forward && !player.is_attacking) {
            auto player_idle_sprite = player.idle.getSpecificFrame((int)floor(game_time / (1.0/player.idle.getAnimSpeed())) % player.idle.getFrameCount());
            SDL_RenderCopyEx(renderer, player_idle.get(), &player_idle_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);
        }
        if (player.is_jumping) {
            int jump_frame = (int)floor(player.time_since_last_input / (1.0/player.jump.getAnimSpeed())) % player.jump.getFrameCount();
            auto player_jump_sprite = player.jump.getSpecificFrame(jump_frame);

            if (jump_frame == player.jump.getFrameCount()) {
                player.is_jumping = false;
                player.is_falling = true;
            }
            SDL_RenderCopyEx(renderer, player_jump.get(), &player_jump_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);
        }
        if (player.is_jumping_forward) {
            int fjump_frame = (int)floor(player.time_since_last_input / (1.0/player.forward_jump.getAnimSpeed())) % player.forward_jump.getFrameCount();
            auto player_fjump_sprite = player.forward_jump.getSpecificFrame(fjump_frame);

            if (fjump_frame == player.forward_jump.getFrameCount()) {
                player.is_jumping_forward = false;
                player.is_falling_forward = true;
            }
            SDL_RenderCopyEx(renderer, player_fjump.get(), &player_fjump_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);
        }
        if (player.is_jumping_backward) {
            int bjump_frame = (int)floor(player.time_since_last_input / (1.0/player.backward_jump.getAnimSpeed())) % player.backward_jump.getFrameCount();
            auto player_bjump_sprite = player.backward_jump.getSpecificFrame(bjump_frame);

            if (bjump_frame == player.backward_jump.getFrameCount()) {
                player.is_jumping_backward = false;
                player.is_falling_backward = true;
            }
            SDL_RenderCopyEx(renderer, player_bjump.get(), &player_bjump_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);
        }
        if (player.is_falling) {
            auto player_fall_sprite = player.fall.getSpecificFrame((int)floor(game_time / (1.0/player.fall.getAnimSpeed())) % player.fall.getFrameCount());
            SDL_RenderCopyEx(renderer, player_fall.get(), &player_fall_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);
        }
        if (player.is_falling_forward) {
            auto player_ffall_sprite = player.forward_fall.getSpecificFrame((int)floor(game_time / (1.0/player.forward_fall.getAnimSpeed())) % player.forward_fall.getFrameCount());
            SDL_RenderCopyEx(renderer, player_ffall.get(), &player_ffall_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);
        }
        if (player.is_falling_backward) {
            auto player_bfall_sprite = player.backward_fall.getSpecificFrame((int)floor(game_time / (1.0/player.backward_fall.getAnimSpeed())) % player.backward_fall.getFrameCount());
            SDL_RenderCopyEx(renderer, player_bfall.get(), &player_bfall_sprite, &player_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
        }
        if (player.is_crouching) {
            player.is_walking_forward = false;
            player.is_walking_backward = false;
            auto player_crouch_sprite = player.crouch.getSpecificFrame((int)floor(game_time / (1.0/player.crouch.getAnimSpeed())) % player.crouch.getFrameCount());
            SDL_RenderCopyEx(renderer, player_crouch.get(), &player_crouch_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);
        }
        if (player.is_walking_forward) {
            auto player_fwalk_sprite = player.forward_walk.getSpecificFrame((int)floor(game_time / (1.0/player.forward_walk.getAnimSpeed())) % player.forward_walk.getFrameCount());
            SDL_RenderCopyEx(renderer, player_fwalk.get(), &player_fwalk_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);
        }
        if (player.is_walking_backward) {
            auto player_bwalk_sprite = player.back_walk.getSpecificFrame((int)floor(game_time / (1.0/player.back_walk.getAnimSpeed())) % player.back_walk.getFrameCount());
            SDL_RenderCopyEx(renderer, player_bwalk.get(), &player_bwalk_sprite, &player_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
        }
        if (player.is_attacking) {
            player.is_walking_forward = false;
            player.is_walking_backward = false;
            player.is_crouching = false;
            if (player.currentAttack == player_t::groundL) {
                int groundL_frame = (int)floor(player.time_since_last_input / (1.0/player.attackL.getAnimSpeed())) % player.attackL.getFrameCount();
                auto player_attackL_sprite = player.attackL.getSpecificFrame(groundL_frame);
                SDL_RenderCopyEx(renderer, player_attackL.get(), &player_attackL_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);
                if (groundL_frame == player.attackL.getFrameCount()) {
                    player.is_attacking = false;
                }
            }
            else if (player.currentAttack == player_t::groundM) {
                int groundM_frame = (int)floor(player.time_since_last_input / (1.0/player.attackM.getAnimSpeed())) % player.attackM.getFrameCount();
                auto player_attackM_sprite = player.attackM.getSpecificFrame(groundM_frame);
                SDL_RenderCopyEx(renderer, player_attackM.get(), &player_attackM_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);
                if (groundM_frame == player.attackM.getFrameCount()) {
                    player.is_attacking = false;
                }
            }
            else if (player.currentAttack == player_t::groundH) {
                int groundH_frame = (int)floor(player.time_since_last_input / (1.0/player.attackH.getAnimSpeed())) % player.attackH.getFrameCount();
                auto player_attackH_sprite = player.attackH.getSpecificFrame(groundH_frame);
                SDL_RenderCopyEx(renderer, player_attackH.get(), &player_attackH_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);
                if (groundH_frame == player.attackH.getFrameCount()) {
                    player.is_attacking = false;
                }
            }
            else if (player.currentAttack == player_t::groundS) {
                int groundS_frame = (int)floor(player.time_since_last_input / (1.0/player.attackS.getAnimSpeed())) % player.attackS.getFrameCount();
                auto player_attackS_sprite = player.attackS.getSpecificFrame(groundS_frame);
                SDL_RenderCopyEx(renderer, player_attackS.get(), &player_attackS_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);
                if (groundS_frame == player.attackS.getFrameCount()) {
                    player.is_attacking = false;
                }
            }
            else if (player.currentAttack == player_t::airL) {
                int airL_frame = (int)floor(player.time_since_last_input / (1.0/player.air_attackL.getAnimSpeed())) % player.air_attackL.getFrameCount();
                auto player_airL_sprite = player.air_attackL.getSpecificFrame(airL_frame);
                SDL_RenderCopyEx(renderer, player_airL.get(), &player_airL_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);
                if (airL_frame == player.air_attackL.getFrameCount()) {
                    player.is_attacking = false;
                }
            }
            else if (player.currentAttack == player_t::airM) {
                int airM_frame = (int)floor(player.time_since_last_input / (1.0/player.air_attackM.getAnimSpeed())) % player.air_attackM.getFrameCount();
                auto player_airM_sprite = player.air_attackM.getSpecificFrame(airM_frame);
                SDL_RenderCopyEx(renderer, player_airM.get(), &player_airM_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);
                if (airM_frame == player.air_attackM.getFrameCount()) {
                    player.is_attacking = false;
                }
            }
            else if (player.currentAttack == player_t::airH) {
                int airH_frame = (int)floor(player.time_since_last_input / (1.0/player.air_attackH.getAnimSpeed())) % player.air_attackH.getFrameCount();
                auto player_airH_sprite = player.air_attackH.getSpecificFrame(airH_frame);
                SDL_RenderCopyEx(renderer, player_airH.get(), &player_airH_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);
                if (airH_frame == player.air_attackH.getFrameCount()) {
                    player.is_attacking = false;
                }
            }
            else if (player.currentAttack == player_t::airS) {
                int airS_frame = (int)floor(player.time_since_last_input / (1.0/player.air_attackS.getAnimSpeed())) % player.air_attackS.getFrameCount();
                auto player_airS_sprite = player.air_attackS.getSpecificFrame(airS_frame);
                SDL_RenderCopyEx(renderer, player_airS.get(), &player_airS_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);
                if (airS_frame == player.air_attackS.getFrameCount()) {
                    player.is_attacking = false;
                }
            }
        }
        // if (player.is_blocking) {
        //
        // }
        if (!player2.is_jumping && !player2.is_crouching && !player2.is_walking_backward
        && !player2.is_walking_forward && !player2.is_jumping_backward && !player2.is_jumping_forward
        && !player2.is_falling && !player2.is_falling_backward && !player2.is_falling_forward && !player2.is_attacking) {
            auto player2_idle_sprite = player2.idle.getSpecificFrame((int)floor(game_time / (1.0/player2.idle.getAnimSpeed())) % player2.idle.getFrameCount());
            SDL_RenderCopyEx(renderer, player2_idle.get(), &player2_idle_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
        }
        if (player2.is_jumping) {
            int jump2_frame = (int)floor(player2.time_since_last_input / (1.0/player2.jump.getAnimSpeed())) % player2.jump.getFrameCount();
            auto player2_jump_sprite = player2.jump.getSpecificFrame(jump2_frame);

            if (jump2_frame == player2.jump.getFrameCount()) {
                player2.is_jumping = false;
                player2.is_falling = true;
            }
            SDL_RenderCopyEx(renderer, player2_jump.get(), &player2_jump_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
        }
        if (player2.is_jumping_forward) {
            int fjump2_frame = (int)floor(player2.time_since_last_input / (1.0/player2.forward_jump.getAnimSpeed())) % player2.forward_jump.getFrameCount();
            auto player2_fjump_sprite = player2.forward_jump.getSpecificFrame(fjump2_frame);

            if (fjump2_frame == player2.forward_jump.getFrameCount()) {
                player2.is_jumping_forward = false;
                player2.is_falling_forward = true;
            }
            SDL_RenderCopyEx(renderer, player2_fjump.get(), &player2_fjump_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
        }
        if (player2.is_jumping_backward) {
            int bjump2_frame = (int)floor(player2.time_since_last_input / (1.0/player2.backward_jump.getAnimSpeed())) % player2.backward_jump.getFrameCount();
            auto player2_bjump_sprite = player2.backward_jump.getSpecificFrame(bjump2_frame);

            if (bjump2_frame == player2.backward_jump.getFrameCount()) {
                player2.is_jumping_backward = false;
                player2.is_falling_backward = true;
            }
            SDL_RenderCopyEx(renderer, player2_bjump.get(), &player2_bjump_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
        }
        if (player2.is_falling) {
            auto player2_fall_sprite = player2.fall.getSpecificFrame((int)floor(game_time / (1.0/player2.fall.getAnimSpeed())) % player2.fall.getFrameCount());
            SDL_RenderCopyEx(renderer, player2_fall.get(), &player2_fall_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
        }
        if (player2.is_falling_forward) {
            auto player2_ffall_sprite = player2.forward_fall.getSpecificFrame((int)floor(game_time / (1.0/player2.forward_fall.getAnimSpeed())) % player2.forward_fall.getFrameCount());
            SDL_RenderCopyEx(renderer, player2_ffall.get(), &player2_ffall_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
        }
        if (player2.is_falling_backward) {
            auto player2_bfall_sprite = player2.backward_fall.getSpecificFrame((int)floor(game_time / (1.0/player2.backward_fall.getAnimSpeed())) % player2.backward_fall.getFrameCount());
            SDL_RenderCopyEx(renderer, player2_ffall.get(), &player2_bfall_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
        }
        if (player2.is_crouching) {
            auto player2_crouch_sprite = player2.crouch.getSpecificFrame((int)floor(game_time / (1.0/player2.crouch.getAnimSpeed())) % player2.crouch.getFrameCount());
            SDL_RenderCopyEx(renderer, player2_crouch.get(), &player2_crouch_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
        }
        if (player2.is_walking_forward) {
            auto player2_fwalk_sprite = player2.forward_walk.getSpecificFrame((int)floor(game_time / (1.0/player2.forward_walk.getAnimSpeed())) % player2.forward_walk.getFrameCount());
            SDL_RenderCopyEx(renderer, player2_fwalk.get(), &player2_fwalk_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
        }
        if (player2.is_walking_backward) {
            auto player2_bwalk_sprite = player2.back_walk.getSpecificFrame((int)floor(game_time / (1.0/player2.back_walk.getAnimSpeed())) % player2.back_walk.getFrameCount());
            SDL_RenderCopyEx(renderer, player2_bwalk.get(), &player2_bwalk_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
        }
        if (player2.is_attacking) {
            player2.is_walking_forward = false;
            player2.is_walking_backward = false;
            player2.is_crouching = false;
            if (player2.currentAttack == player_t::groundL) {
                int groundL2_frame = (int)floor(player2.time_since_last_input / (1.0/player2.attackL.getAnimSpeed())) % player2.attackL.getFrameCount();
                auto player2_attackL_sprite = player2.attackL.getSpecificFrame(groundL2_frame);
                SDL_RenderCopyEx(renderer, player2_attackL.get(), &player2_attackL_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
                if (groundL2_frame == player2.attackL.getFrameCount()) {
                    player2.is_attacking = false;
                }
            }
            else if (player2.currentAttack == player_t::groundM) {
                int groundM2_frame = (int)floor(player2.time_since_last_input / (1.0/player2.attackM.getAnimSpeed())) % player2.attackM.getFrameCount();
                auto player2_attackM_sprite = player2.attackM.getSpecificFrame(groundM2_frame);
                SDL_RenderCopyEx(renderer, player2_attackM.get(), &player2_attackM_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
                if (groundM2_frame == player2.attackM.getFrameCount()) {
                    player2.is_attacking = false;
                }
            }
            else if (player2.currentAttack == player_t::groundH) {
                int groundH2_frame = (int)floor(player2.time_since_last_input / (1.0/player2.attackH.getAnimSpeed())) % player2.attackH.getFrameCount();
                auto player2_attackH_sprite = player2.attackH.getSpecificFrame(groundH2_frame);
                SDL_RenderCopyEx(renderer, player2_attackH.get(), &player2_attackH_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
                if (groundH2_frame == player2.attackH.getFrameCount()) {
                    player2.is_attacking = false;
                }
            }
            else if (player2.currentAttack == player_t::groundS) {
                int groundS2_frame = (int)floor(player2.time_since_last_input / (1.0/player2.attackS.getAnimSpeed())) % player2.attackS.getFrameCount();
                auto player2_attackS_sprite = player2.attackS.getSpecificFrame(groundS2_frame);
                SDL_RenderCopyEx(renderer, player2_attackS.get(), &player2_attackS_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
                if (groundS2_frame == player2.attackS.getFrameCount()) {
                    player2.is_attacking = false;
                }
            }
            else if (player2.currentAttack == player_t::airL) {
                int airL2_frame = (int)floor(player2.time_since_last_input / (1.0/player2.air_attackL.getAnimSpeed())) % player2.air_attackL.getFrameCount();
                auto player2_airL_sprite = player2.air_attackL.getSpecificFrame(airL2_frame);
                SDL_RenderCopyEx(renderer, player2_airL.get(), &player2_airL_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
                if (airL2_frame == player2.air_attackL.getFrameCount()) {
                    player2.is_attacking = false;
                }
            }
            else if (player2.currentAttack == player_t::airM) {
                int airM2_frame = (int)floor(player2.time_since_last_input / (1.0/player2.air_attackM.getAnimSpeed())) % player2.air_attackM.getFrameCount();
                auto player2_airM_sprite = player2.air_attackM.getSpecificFrame(airM2_frame);
                SDL_RenderCopyEx(renderer, player2_airM.get(), &player2_airM_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
                if (airM2_frame == player2.air_attackM.getFrameCount()) {
                    player2.is_attacking = false;
                }
            }
            else if (player2.currentAttack == player_t::airH) {
                int airH2_frame = (int)floor(player2.time_since_last_input / (1.0/player2.air_attackH.getAnimSpeed())) % player2.air_attackH.getFrameCount();
                auto player2_airH_sprite = player2.air_attackH.getSpecificFrame(airH2_frame);
                SDL_RenderCopyEx(renderer, player2_airH.get(), &player2_airH_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
                if (airH2_frame == player2.air_attackH.getFrameCount()) {
                    player2.is_attacking = false;
                }
            }
            else if (player2.currentAttack == player_t::airS) {
                int airS2_frame = (int)floor(player2.time_since_last_input / (1.0/player2.air_attackS.getAnimSpeed())) % player2.air_attackS.getFrameCount();
                auto player2_airS_sprite = player2.air_attackS.getSpecificFrame(airS2_frame);
                SDL_RenderCopyEx(renderer, player2_airS.get(), &player2_airS_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
                if (airS2_frame == player2.air_attackS.getFrameCount()) {
                    player2.is_attacking = false;
                }
            }
        }

        // if (player2.is_blocking) {
        //
        // }

        // SDL_RenderDrawLine(renderer, 0, 0, x, y);                // linia podążająca za playerem
        // SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 0xff);
        // SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        SDL_RenderPresent(renderer);

        // delays
        current_time = current_time + microseconds((long long int)(dt*1000000.0));
        std::this_thread::sleep_until(current_time);

    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
