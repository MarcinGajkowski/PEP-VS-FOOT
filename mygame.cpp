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
                                                  int frames);
    SDL_Rect getSpecificFrame(int frame);
    int getFrameCount() const;
    SDL_Rect getHitbox();
    void setHitbox(int x, int y, int w, int h);
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
    int damage_on_hit = 0;
    int damage_on_block = 0;
    int hitstun_frames = 0;
    SDL_Rect hitbox = {};
    std::vector<SDL_Rect> frame_list;
};

std::shared_ptr<SDL_Texture> Move::load_sheet(SDL_Renderer *renderer, const std::string &filename, int width, int height,
                                            int frames) {
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
    // frame_list.resize(frames);
    // SDL_Rect renderQuad = { 0, 0, sprite_sheet->w, sprite_sheet->h };

    for (int i = 0; i < texture->h/height; i++) {
        for (int j = 0; j < texture->w/width; j++) {
            if (i*(texture->w/width) + j == frames) break;
            SDL_Rect frame = { j*width, i*height, width, height };
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

SDL_Rect Move::getHitbox() {
    return hitbox;
}

void Move::setHitbox(int x, int y, int w, int h) {
    hitbox.x = x;
    hitbox.y = y;
    hitbox.w = w;
    hitbox.h = h;
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
    Move idle{};
    // Move walk{};
    // Move jump{};
    // Move forward_jump{};
    // Move attackL{};
    // Move attackM{};
    // Move attackH{};
    // Move attackS{};
    // Move air_attackL{};
    // Move air_attackM{};
    // Move air_attackH{};
    // Move air_attackS{};
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
        player_old.a.v.y = 50;  // przyspieszenie ziemskie
        player_old.a.v.x = player_old.a.v.x * 0.9;
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
    if (player.p.v.x > 30) {
        player.p.v.x = 30;
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

    // auto player_texture = load_image(renderer, "PEPPINO Sprites/crouch/crouch.bmp");
    // auto player2_texture = load_image(renderer, "FOOTSIES Guy Sprites/Idle_0.bmp");

    bool still_playing = true;
    player_t player;
    // player.PEPPINO;

    player.p.v.x = 2;
    player.p.v.y = 4;
    player.a.v.x = 0;
    player.a.v.y = 0;
    player.v.v.x = 0;
    player.v.v.y = 0;

    player_t player2;
    // player2.FOOTSIES;

    player2.p.v.x = 24;
    player2.p.v.y = 4;
    player2.a.v.x = 0;
    player2.a.v.y = 0;
    player2.v.v.x = 0;
    player2.v.v.y = 0;

    auto player_idle = player.idle.load_sheet(renderer, "PEPPINO Sprites/idle/idle.bmp", 100, 100,  17);
    auto player2_idle = player2.idle.load_sheet(renderer, "FOOTSIES Guy Sprites/Idle.bmp", 60, 50,  5);

    // int x = 100;
    // int y = 100;
    double game_time = 0.;
    steady_clock::time_point current_time = steady_clock::now();
    while (still_playing) {
        // events
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: {
                    still_playing = false;
                    break;
                }
                case SDL_KEYDOWN: {
                    if (is_grounded(player, game_map)) {
                        if (event.key.keysym.scancode == SDL_SCANCODE_W) player.a.v.y = -2000;
                        // if (event.key.keysym.scancode == SDL_SCANCODE_S) player.a.v.y = 50;
                        if (event.key.keysym.scancode == SDL_SCANCODE_A) player.a.v.x = -30;
                        if (event.key.keysym.scancode == SDL_SCANCODE_D) player.a.v.x = 30;
                    }
                    if (is_grounded(player2, game_map)) {
                        if (event.key.keysym.scancode == SDL_SCANCODE_UP) player2.a.v.y = -2000;
                        // if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) player2.a.v.y = 50;
                        if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) player2.a.v.x = -30;
                        if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) player2.a.v.x = 30;
                    }
                    // if (event.key.keysym.scancode == SDL_SCANCODE_UP) player.v.v.y = -50;
                    // // if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) player.v.v.y = 50;
                    // if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) player.v.v.x = -50;
                    // if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) player.v.v.x = 50;

                    break;
                }
                case SDL_KEYUP: {

                    if (event.key.keysym.scancode == SDL_SCANCODE_O) still_playing = false;

                    if (event.key.keysym.scancode == SDL_SCANCODE_W) player.a.v.y = 0;
                    // if (event.key.keysym.scancode == SDL_SCANCODE_S) player.a.v.y = 0;
                    if (event.key.keysym.scancode == SDL_SCANCODE_A) player.a.v.x = 0;
                    if (event.key.keysym.scancode == SDL_SCANCODE_D) player.a.v.x = 0;

                    if (event.key.keysym.scancode == SDL_SCANCODE_UP) player2.a.v.y = 0;
                    // if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) player2.a.v.y = 0;
                    if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) player2.a.v.x = 0;
                    if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) player2.a.v.x = 0;



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

        player = update_player(player, game_map, dt);
        player2 = update_player(player2, game_map, dt);

        // graphics
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, bg_texture.get(), NULL, NULL);
        // SDL_Rect clouds_rect = { x/5, y/5,1280,720};
        // SDL_RenderCopyEx(renderer, clouds_texture.get(), NULL, &clouds_rect, game_time*10, NULL, SDL_FLIP_NONE);

        draw_map(renderer, game_map, tiles_texture);

        SDL_Rect player_rect = {(int)(player.p.v.x*TILE_SIZE-(TILE_SIZE/2)), (int)(player.p.v.y*TILE_SIZE-TILE_SIZE-259), 400, 400};
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
            int r = 0, g = 0, b = 0;
            if (is_grounded(player2, game_map)) {
                r = 255;
            }
            if (is_colliding(player2.p, game_map)) {
                g = 255;
            }
            SDL_SetRenderDrawColor(renderer, r,g,b, 0xFF);
        }
        SDL_RenderDrawRect(renderer, &player_rect);
        SDL_RenderDrawRect(renderer, &player2_rect);
        auto player_sprite = player.idle.getSpecificFrame((int)game_time % player.idle.getFrameCount());
        auto player2_sprite = player2.idle.getSpecificFrame((int)game_time % player2.idle.getFrameCount());
        SDL_RenderCopyEx(renderer, player_idle.get(), &player_sprite, &player_rect, 0, NULL, SDL_FLIP_NONE);
        SDL_RenderCopyEx(renderer, player2_idle.get(), &player2_sprite, &player2_rect, 0, NULL, SDL_FLIP_HORIZONTAL);

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
