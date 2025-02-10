#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

#define WIDTH 900
#define HEIGHT 500
#define PADDLE_WIDTH 20
#define PADDLE_HEIGHT 80
#define NUM_PADDLES 2
#define TARGET_FPS 120
#define TARGET_DT (1.0 / TARGET_FPS)

SDL_Color green = {68, 157, 68, 255};
SDL_Color white = {223, 240, 216};

SDL_Window* window;
SDL_Renderer* renderer;

typedef struct paddle {
    SDL_FRect shape;
    int velocity;
} Paddle;

typedef struct field {
    SDL_FRect borders[2];
    SDL_FRect middle_tile;
} Field;

Paddle paddles[NUM_PADDLES];
Field field;

void draw_background(Field f) {
    SDL_SetRenderDrawColor(renderer, white.r, white.g, white.b, white.a);
    SDL_RenderFillRects(renderer, f.borders, 2);
    f.middle_tile.y = 10;
    for (int i = 0; i < HEIGHT / 10; i++) {
        SDL_RenderFillRect(renderer, &f.middle_tile);
        f.middle_tile.y += 10;
    }
}

void update_paddles(Paddle* p, int paddle_count, double dt) {
    for (int i = 0; i < paddle_count; i++) {
        p[i].shape.y += p[i].velocity * 200 * dt;
        if (p[i].shape.y <= field.borders[0].y + field.borders[0].h) {
            p[i].shape.y = field.borders[0].y + field.borders[0].h;
        }
        if (p[i].shape.y >=field.borders[1].y - p[i].shape.h) {
            p[i].shape.y = field.borders[1].y - p[i].shape.h;
        }
    }
}

void draw_paddles(Paddle* p) {
    SDL_SetRenderDrawColor(renderer, white.r, white.g, white.b, white.a);
    for (int i = 0; i < NUM_PADDLES; i++) {
        SDL_RenderFillRect(renderer, &p[i].shape);
    }
}

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("Couldn't init SDL: %s\n", SDL_GetError());       
    }

    SDL_CreateWindowAndRenderer("20g_pong", WIDTH, HEIGHT, 0, &window, & renderer);

    if (window == NULL || renderer == NULL) {
        printf("Error: %s\n", SDL_GetError());
    }

    Paddle player = (Paddle) {
        .shape = (SDL_FRect) {
            .x = 10,
            .y = 100,
            .w = PADDLE_WIDTH,
            .h = PADDLE_HEIGHT
        },
        .velocity = 0
    };
    
    Paddle friend = (Paddle) {
            .shape = (SDL_FRect) {
                .x = WIDTH - 10 - PADDLE_WIDTH ,
                .y = 100,
                .w = PADDLE_WIDTH,
                .h = PADDLE_HEIGHT
            },
            .velocity = 0
        };

    paddles[0] = player;
    paddles[1] = friend;

    field = (Field) {
        .borders[0] = (SDL_FRect) {
            .h = 5,
            .w = 860,
            .x = 20,
            .y = 5            
        },
        .borders[1] = (SDL_FRect) {
            .h = 5,
            .w = 860,
            .x = 20,
            .y = HEIGHT - 10
        },
        .middle_tile = (SDL_FRect) {
            .h = 5,
            .w = 5,
            .x =(int)(WIDTH / 2 - 2),
            .y = 10,    
        }
    };

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
                    case SDLK_S:
                        paddles[0].velocity = 1;
                        break;
                    case SDLK_W:
                        paddles[0].velocity = -1;
                        break;
                    case SDLK_UP:
                        paddles[1].velocity = -1;
                        break;
                    case SDLK_DOWN:
                        paddles[1].velocity = 1;
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
            update_paddles(paddles, NUM_PADDLES, TARGET_DT);
            accumulator -= TARGET_DT;   
        }
        
        SDL_SetRenderDrawColor(renderer, green.r, green.g, green.b, green.a);
        SDL_RenderClear(renderer);
        
        draw_background(field);
        draw_paddles(paddles);       
        
        SDL_RenderPresent(renderer);   
        
        double frameTime = (SDL_GetPerformanceCounter() - current_time) * 1000.0 / SDL_GetPerformanceFrequency();
        if (frameTime < (1000.0 / TARGET_FPS)) {
            SDL_Delay((1000.0 / TARGET_FPS) - frameTime);
        }           
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
    
}

