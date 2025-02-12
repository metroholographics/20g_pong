#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_SCORE 3

#define WIDTH 900
#define HEIGHT 500

#define PADDLE_WIDTH 20
#define PADDLE_HEIGHT 80
#define PADDLE_SPEED 250
#define NUM_PADDLES 2

#define BALL_HEIGHT 15
#define BALL_WIDTH 15
#define BALL_MOVE_SPEED 600

#define TARGET_FPS 240
#define TARGET_DT (1.0 / TARGET_FPS)

typedef enum game_state {
    MENU,
    PLAY
} Game_State;

typedef enum ball_state {
    GAME_BALL_SPAWN,
    GAME_BALL_MOVING,
    GAME_BALL_HIT_WALL,
    GAME_BALL_HIT_PADDLE,
    GAME_BALL_OUT_LEFT,
    GAME_BALL_OUT_RIGHT
} Ball_State;

typedef struct game {
    Game_State state;
} Game;

typedef struct Field {
    SDL_FRect borders[2];
    SDL_FRect middle_tile;
} Field;

typedef struct Paddle {
    SDL_FRect shape;
    int velocity;
    int score;
} Paddle;

typedef struct Ball {
    SDL_FRect shape;
    Ball_State state;
    int vel_x;
    int vel_y;
    float dir_x;
    float dir_y;
} Ball;

typedef struct text_elements {
    SDL_Texture* score_text[NUM_PADDLES];
    SDL_FRect score_box[NUM_PADDLES];
    SDL_Texture* menu_text;
    SDL_FRect menu_box; 
} Text_Elements;

Game game;
Field field;
Paddle paddles[NUM_PADDLES];
Ball ball;
Text_Elements text_ui;

SDL_Color colour_green = {68, 157, 68, 255};
SDL_Color colour_white = {223, 240, 216, 255};
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
TTF_Font* font = NULL;

// ::init
bool initialise_sdl(SDL_Window** w, SDL_Renderer** r, TTF_Font** f) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
          printf("Couldn't init SDL: %s\n", SDL_GetError());
          return false;
    }

    SDL_CreateWindowAndRenderer("20g_pong", WIDTH, HEIGHT, 0, w, r);
    if (w == NULL || r  == NULL) {
        printf("Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
   
    if (!TTF_Init()) {
        printf("Error: %s\n", SDL_GetError());
        return false;
    }
    
    *f = TTF_OpenFont(".\\fonts\\FiraCode-Bold.ttf", 36);
    if (font == NULL) {
        printf("Error_font: %s\n", SDL_GetError());
        return false;
    }
        
   return true; 
}

void set_field(Field* f) {
    *f = (Field) {
        .borders[0]  = (SDL_FRect) {.h = 5, .w = 860, .x = 20, .y = 5},
        .borders[1]  = (SDL_FRect) {.h = 5, .w = 860, .x = 20, .y = HEIGHT - 10},
        .middle_tile = (SDL_FRect) {.h = 5,.w = 5, .x = WIDTH * 0.5f - 2, .y = 10,}
    };
}

void reset_paddles(Paddle* p) {
    Paddle player = (Paddle) {
        .shape    = (SDL_FRect) {.x = 10, .y = 100, .w = PADDLE_WIDTH, .h = PADDLE_HEIGHT},
        .velocity = 0,
        .score    = 0
    };
    
    Paddle friend = (Paddle) {
        .shape    = (SDL_FRect) {.x = WIDTH - 10 - PADDLE_WIDTH, .y = 100, .w = PADDLE_WIDTH, .h = PADDLE_HEIGHT},
        .velocity = 0,
        .score    = 0
    };

    p[0] = player;
    p[1] = friend;
}

void reset_ball(Ball* b) {
    *b = (Ball) {
        .shape = (SDL_FRect) {.h = BALL_HEIGHT, .w = BALL_WIDTH,
            .x = WIDTH * 0.5f - (BALL_WIDTH * 0.5f), .y = HEIGHT * 0.5f - (BALL_HEIGHT * 0.5f)},
        //::todo make the vel and dir values random
        .vel_x = -1,
        .vel_y = -1,
        .dir_x = 0.5,
        .dir_y = 0.5,
        .state = GAME_BALL_SPAWN   
    }; 
}

void reset_text_ui(Text_Elements* ui) {
    *ui = (Text_Elements) {
        .score_text[0] = NULL,
        .score_text[1] = NULL,
        .score_box[0]  = (SDL_FRect) {.h = 30, .w = 50, .x = 50, .y = 20},  
        .score_box[1]  = (SDL_FRect) {.h = 30, .w = 50, .x = WIDTH - 50 - 15, .y = 20},   
    };    
}

bool set_score_text(Text_Elements* ui, int paddle_id, int score) {
    char buff[10];
    char *num = itoa(score, buff, 10);
    if (score > MAX_SCORE) {
        memcpy(buff,"Error", sizeof("Error"));
    }
    
    SDL_Surface* t = TTF_RenderText_Blended(font, num, 0, colour_white);
    if (t == NULL) {
        printf("Error_surface: %s\n", SDL_GetError());
        return false;
    }
    
    ui->score_text[paddle_id] = SDL_CreateTextureFromSurface(renderer, t);
    SDL_DestroySurface(t);
    return true;
}

// ::update
void reset_score_text(Text_Elements* ui, Paddle* p, int paddle_count) {
    for (int i = 0; i < paddle_count; i++) {
        // ::todo: find way to error handle this
        set_score_text(ui, i, p[i].score);
    }
}

bool aabb_collision_rects(SDL_FRect a, SDL_FRect b) {
    if (a.x < b.x + b.w && a.x + b.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y) {
            return true;
    }              
    return false;
}

bool get_collision_and_state(Ball* b) {
    // check border collision 
    for (int i = 0; i < 2; i++) {
        if (aabb_collision_rects(b->shape, field.borders[i])) {
            b->state = GAME_BALL_HIT_WALL;
            return true;
        }      
    }
    // check paddle collision 
    for (int i = 0; i < NUM_PADDLES; i++) {
        if (aabb_collision_rects(b->shape, paddles[i].shape)) {
            b->state = GAME_BALL_HIT_PADDLE;
            return true;
        }      
    }

    if (b->shape.x + b->shape.w < 0) {
        b->state = GAME_BALL_OUT_LEFT;
        return true;
    }

    if (b->shape.x > WIDTH) {
        b->state = GAME_BALL_OUT_RIGHT;
        return true;
    }

    return false;
}


void update_score(Paddle* p, Text_Elements* ui,  int id) {
    set_score_text(ui, id, p[id].score);
}

void update_paddles(Paddle* p, Text_Elements* ui, int paddle_count, double dt) {
    for (int i = 0; i < paddle_count; i++) {
        if (p[i].score > MAX_SCORE) {
                    reset_paddles(p);
                    reset_score_text(ui, p, NUM_PADDLES);
                    game.state = MENU;
                    return;
        }
        
        p[i].shape.y += p[i].velocity * PADDLE_SPEED * dt;
        if (p[i].shape.y <= field.borders[0].y + field.borders[0].h) {
            p[i].shape.y = field.borders[0].y + field.borders[0].h;
        }
        if (p[i].shape.y >=field.borders[1].y - p[i].shape.h) {
            p[i].shape.y = field.borders[1].y - p[i].shape.h;
        }               
    }   
}

void update_game(Text_Elements* ui, Paddle* p, Ball* b,  double dt) {
    if (b->state == GAME_BALL_SPAWN) {
        reset_ball(b);
        b->state = GAME_BALL_MOVING;
    } else if (b->state == GAME_BALL_MOVING) {
        float new_x = b->shape.x;
        float new_y = b->shape.y;
        new_x += (b->vel_x * b->dir_x) * BALL_MOVE_SPEED * dt;
        new_y += (b->vel_y * b->dir_y) * BALL_MOVE_SPEED * dt;
        Ball collider_ball = *b;
        collider_ball.shape.x = new_x;
        collider_ball.shape.y = new_y;
        if (get_collision_and_state(&collider_ball)) {
            switch (collider_ball.state) {
                case GAME_BALL_HIT_WALL:
                    collider_ball.vel_y *= -1;
                    break;
                case GAME_BALL_HIT_PADDLE:
                    collider_ball.vel_x *= -1;
                    break;
                case GAME_BALL_OUT_RIGHT:
                    p[0].score += 1;
                    update_score(paddles, ui,  0);
                    break;
                case GAME_BALL_OUT_LEFT:
                    p[1].score += 1;
                    update_score(paddles, ui, 1);
                    break;
                default:
                    break;
            }
        }
        *b = collider_ball;
        b->state = (b->state == GAME_BALL_OUT_RIGHT || b->state == GAME_BALL_OUT_LEFT) ? GAME_BALL_SPAWN : GAME_BALL_MOVING;
        update_paddles(paddles, ui, NUM_PADDLES, TARGET_DT);                           
    }     
}

//::draw
void draw_background(Field f, SDL_Color col) {
    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
    SDL_RenderFillRects(renderer, f.borders, 2);
    f.middle_tile.y = 10;
    //::todo - make a 'centre line' rect array once beforehand, draw here in one go
    for (int i = 0; i < HEIGHT / 10; i++) {
        SDL_RenderFillRect(renderer, &f.middle_tile);
        f.middle_tile.y += 10;
    }
}

void draw_ball(Ball b, SDL_Color col) {
    SDL_SetRenderDrawColor(renderer, col.r, col.g , col.b, col.a);
    SDL_RenderFillRect(renderer, &b.shape);
}

void draw_paddles(Paddle* p, SDL_Color col) {
    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
    for (int i = 0; i < NUM_PADDLES; i++) {
        SDL_RenderFillRect(renderer, &p[i].shape);
    }
}

void draw_game_text() {
    for (int i = 0; i < NUM_PADDLES; i++) {
        SDL_FRect dest;
        SDL_GetTextureSize(text_ui.score_text[i], &dest.w, &dest.h);
        text_ui.score_box[i].w = dest.w;
        text_ui.score_box[i].h = dest.h;
        SDL_RenderTexture(renderer, text_ui.score_text[i], NULL, &text_ui.score_box[i]);    
    }
    if (game.state == MENU) {
        SDL_FRect dest;
        SDL_GetTextureSize(text_ui.menu_text, &dest.w, &dest.h);
        
        dest.x = text_ui.menu_box.x + (text_ui.menu_box.w - dest.w) / 2;
        dest.y = text_ui.menu_box.y + ((text_ui.menu_box.h - dest.h) / 2);
        SDL_SetRenderDrawColor(renderer,10, 10, 10, 100);
        SDL_RenderFillRect(renderer, &text_ui.menu_box);
        SDL_SetRenderDrawColor(renderer, 10,10,10,255);
        SDL_RenderTexture(renderer, text_ui.menu_text, NULL, &dest);
        // SDL_RenderFillRect(renderer, &dest);
        
    }
}

void make_menu(Text_Elements* ui) {
    SDL_Surface* s = TTF_RenderText_Blended_Wrapped(font, "20g_pong\n\nplay: y/n", 0, colour_white, 0);
    if (s) {
        ui->menu_text = SDL_CreateTextureFromSurface(renderer, s);
        SDL_DestroySurface(s);
    }

    
}

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;
   
    if (!initialise_sdl(&window, &renderer, &font)) {
        printf("Aborting!\n");
        return -1;   
     }

    set_field(&field);
    reset_paddles(paddles);
    reset_ball(&ball);
    reset_text_ui(&text_ui);
    reset_score_text(&text_ui, paddles, NUM_PADDLES);
    text_ui.menu_text = NULL;
    text_ui.menu_box = (SDL_FRect) {.w = WIDTH - 200, .h = HEIGHT - 200, .x = 100, .y = 100};
    make_menu(&text_ui);
       
    SDL_Event e;

    uint64_t current_time = SDL_GetPerformanceCounter();
    uint64_t last_time = 0;
    double delta_time = 0.0f;

    bool running = true;
    game.state = MENU;        
       
    while (running) {
        last_time = current_time;
        current_time = SDL_GetPerformanceCounter();
        delta_time = (double)((current_time - last_time) * 1000) / SDL_GetPerformanceFrequency();
         
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                 running = false;
                 break;
            }
            if (e.type == SDL_EVENT_KEY_DOWN)  {
                switch (e.key.key) {
                    case SDLK_W:
                        paddles[0].velocity = -1;
                        break;
                    case SDLK_S:
                        paddles[0].velocity = 1;
                        break;
                    case SDLK_UP:
                        paddles[1].velocity = -1;
                        break;
                    case SDLK_DOWN:
                        paddles[1].velocity = 1;
                        break;
                    case SDLK_R:
                        ball.state = GAME_BALL_SPAWN;
                    case SDLK_Y:
                        if (game.state == MENU) {
                            game.state = PLAY;
                        }
                        break;
                    case SDLK_N:
                        if (game.state == MENU) {
                            running = false;
                        }
                        break;
                }
            }
            if (e.type == SDL_EVENT_KEY_UP)  {
                switch (e.key.key) {
                    case SDLK_S:
                        paddles[0].velocity = 0;
                        break;
                    case SDLK_W:
                        paddles[0].velocity = 0;
                        break;
                    case SDLK_UP:
                        paddles[1].velocity = 0;
                        break;
                    case SDLK_DOWN:
                        paddles[1].velocity = 0;
                        break;
                }
            } 
        }

        static double accumulator = 0.0;
        accumulator += delta_time / 1000.0;
        while (accumulator >= TARGET_DT) {
            switch (game.state) {
                case MENU:
                    break;
                case PLAY:
                    update_game(&text_ui, paddles, &ball, TARGET_DT);
                    break;
                default:
                    break;        
            }
            accumulator -= TARGET_DT;   
        }
            
        
        
        SDL_SetRenderDrawColor(renderer, colour_green.r, colour_green.g, colour_green.b, colour_green.a);
        SDL_RenderClear(renderer);
        
        draw_background(field, colour_white);
        draw_ball(ball, colour_white);
        draw_paddles(paddles, colour_white);       
        draw_game_text();

        SDL_RenderPresent(renderer);

        double frameTime = (SDL_GetPerformanceCounter() - current_time) * 1000.0 / SDL_GetPerformanceFrequency();
        if (frameTime < (1000.0 / TARGET_FPS)) {
            SDL_Delay((1000.0 / TARGET_FPS) - frameTime);
        }           
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    if (font) {
        TTF_CloseFont(font);
    }
    TTF_Quit();
    SDL_Quit();
    return 0;
}

