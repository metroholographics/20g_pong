#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_SCORE 11
#define WIDTH 900
#define HEIGHT 500
#define PADDLE_WIDTH 20
#define PADDLE_HEIGHT 80
#define PADDLE_SPEED 200
#define NUM_PADDLES 2
#define BALL_HEIGHT 20
#define BALL_WIDTH 20
#define TARGET_FPS 120
#define TARGET_DT (1.0 / TARGET_FPS)

typedef enum game_state {
    GAME_BALL_SPAWN,
    GAME_BALL_MOVING,
    GAME_BALL_HIT_WALL,
    GAME_BALL_HIT_PADDLE
} Ball_State;

typedef struct field {
    SDL_FRect borders[2];
    SDL_FRect middle_tile;
} Field;

typedef struct paddle {
    SDL_FRect shape;
    int velocity;
    int score;
} Paddle;

typedef struct ball {
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
} Text_Elements;

Field field;
Paddle paddles[NUM_PADDLES];
Ball ball;
Text_Elements text_ui;

SDL_Color green = {68, 157, 68, 255};
SDL_Color white = {223, 240, 216, 255};
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* texture = NULL;
TTF_Font* font = NULL;


void update_paddles(Paddle* p, int paddle_count, double dt) {
    for (int i = 0; i < paddle_count; i++) {
        p[i].shape.y += p[i].velocity * PADDLE_SPEED * dt;
        if (p[i].shape.y <= field.borders[0].y + field.borders[0].h) {
            p[i].shape.y = field.borders[0].y + field.borders[0].h;
        }
        if (p[i].shape.y >=field.borders[1].y - p[i].shape.h) {
            p[i].shape.y = field.borders[1].y - p[i].shape.h;
        }
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

    return false;
}

void update_text() {
    
}

void update_game(double dt) {
    if (ball.state == GAME_BALL_SPAWN) {
        ball.shape.x = (WIDTH * 0.5f) - (BALL_WIDTH * 0.5f);
        ball.shape.y = (HEIGHT * 0.5f) - (BALL_HEIGHT * 0.5f);
        ball.vel_x = -1;
        ball.vel_y = -1;
        ball.dir_x = 0.5;
        ball.dir_y = 0.375;
        ball.state = GAME_BALL_MOVING;
    } else if (ball.state == GAME_BALL_MOVING) {
        float new_x = ball.shape.x;
        float new_y = ball.shape.y;
        new_x += (ball.vel_x * ball.dir_x) * 500 * dt;
        new_y += (ball.vel_y * ball.dir_y) * 500 * dt;
        Ball collider_ball = ball;
        collider_ball.shape.x = new_x;
        collider_ball.shape.y = new_y;
        
        if (get_collision_and_state(&collider_ball)){
            switch (collider_ball.state) {
                case GAME_BALL_HIT_WALL:
                    collider_ball.vel_y *= -1;
                    break;
                case GAME_BALL_HIT_PADDLE:
                    collider_ball.vel_x *= -1;
                    break;
                default:
                    break;
            }
        }
        ball = collider_ball;
        ball.state = GAME_BALL_MOVING;                          
    }
    
    update_paddles(paddles, NUM_PADDLES, TARGET_DT);    
}

void draw_background(Field f) {
    SDL_SetRenderDrawColor(renderer, white.r, white.g, white.b, white.a);
    SDL_RenderFillRects(renderer, f.borders, 2);
    f.middle_tile.y = 10;
    // :todo - make a 'centre line' rect array once beforehand, draw here in one go
    for (int i = 0; i < HEIGHT / 10; i++) {
        SDL_RenderFillRect(renderer, &f.middle_tile);
        f.middle_tile.y += 10;
    }
}

void draw_ball(Ball* b) {
    SDL_SetRenderDrawColor(renderer, white.r, white.g , white.b, white.a);
    SDL_RenderFillRect(renderer, &b->shape);
}

void draw_paddles(Paddle* p) {
    SDL_SetRenderDrawColor(renderer, white.r, white.g, white.b, white.a);
    for (int i = 0; i < NUM_PADDLES; i++) {
        SDL_RenderFillRect(renderer, &p[i].shape);
    }
}

bool set_score_text(int score) {
    char buff[10];
    char *num = itoa(score, buff, 10);
    if (score > MAX_SCORE) {
        memcpy(buff,"Error", sizeof("Error"));
    }
    for (int i = 0; i < NUM_PADDLES; i++) {
        SDL_Surface* t = TTF_RenderText_Blended(font, num, 0, white);
        if (t == NULL) {
            printf("Error_surface: %s\n", SDL_GetError());
            return false;
        }
        text_ui.score_text[i] = SDL_CreateTextureFromSurface(renderer, t);
        SDL_DestroySurface(t);
    }
    return true;
}

void draw_game_text() {
   for (int i = 0; i < NUM_PADDLES; i++) {
        SDL_RenderTexture(renderer, text_ui.score_text[i], NULL, &text_ui.score_box[i]);    
    }
    
}

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
      printf("Couldn't init SDL: %s\n", SDL_GetError());
      return -1;
    }

    SDL_CreateWindowAndRenderer("20g_pong", WIDTH, HEIGHT, 0, &window, & renderer);
    if (window == NULL || renderer == NULL) {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }
    
    if (!TTF_Init()) {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }
    
    SDL_Surface* text;

    font = TTF_OpenFont(".\\fonts\\FiraCode-Bold.ttf", 60);
    if (!font) {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }
        
    field = (Field) {
            .borders[0]  = (SDL_FRect) {.h = 5, .w = 860, .x = 20, .y = 5},
            .borders[1]  = (SDL_FRect) {.h = 5, .w = 860, .x = 20, .y = HEIGHT - 10},
            .middle_tile = (SDL_FRect) {.h = 5,.w = 5, .x = WIDTH * 0.5f - 2, .y = 10,}
        };

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

    paddles[0] = player;
    paddles[1] = friend;

    ball = (Ball) {
        .shape = (SDL_FRect) {.h = BALL_HEIGHT,.w = BALL_WIDTH,.x = WIDTH * 0.5f - (BALL_WIDTH * 0.5f), .y = HEIGHT * 0.5f - (BALL_HEIGHT * 0.5f)},
        .vel_x = 0,
        .vel_y = 0,
        .state = GAME_BALL_SPAWN   
    };

    text_ui = (Text_Elements) {
        .score_text[0] = NULL,
        .score_text[1] = NULL,
        .score_box[0]  = (SDL_FRect) {.h = 30, .w = 50, .x = 50, .y = 20},  
        .score_box[1]  = (SDL_FRect) {.h = 30, .w = 50, .x = WIDTH - 50 - 15, .y = 20}   
     };

     if (!set_score_text(0)) {
         printf("Error setting score - abort\n");
         return -1;
     }    
    
    SDL_Event e;
    bool running = true;

    uint64_t current_time = SDL_GetPerformanceCounter();
    uint64_t last_time = 0;
    double delta_time = 0.0f;
    
           
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
            update_game(TARGET_DT);
            accumulator -= TARGET_DT;   
        }
        
        SDL_SetRenderDrawColor(renderer, green.r, green.g, green.b, green.a);
        SDL_RenderClear(renderer);
        
        draw_background(field);
        draw_ball(&ball);
        draw_paddles(paddles);       
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

