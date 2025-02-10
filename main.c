#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

#define WIDTH 900
#define HEIGHT 500
#define PADDLE_WIDTH 20
#define PADDLE_HEIGHT 80
#define NUM_PADDLES 1

SDL_Window* window;
SDL_Renderer* renderer;

typedef struct paddle {
    SDL_FRect shape;
    int x;
    int y;
} Paddle;

typedef struct field {
    SDL_FRect borders[2];
    SDL_FRect middle_tile;
} Field;

Paddle paddles[NUM_PADDLES];
Field field;

void draw_background(Field f) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRects(renderer, f.borders, 2);
    f.middle_tile.y = 10;
    for (int i = 0; i < HEIGHT / 10; i++) {
        SDL_RenderFillRect(renderer, &f.middle_tile);
        f.middle_tile.y += 10;
    }
}

void update_paddles(Paddle* p, int paddle_count) {
    for (int i = 0; i < paddle_count; i++) {
        p[i].shape.y += p[i].y * 10;
    }
}

void draw_paddles(Paddle* p) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
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
        .x = 0,
        .y = 0
    };

    paddles[0] = player;

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
            .y = HEIGHT - 5
        },
        .middle_tile = (SDL_FRect) {
            .h = 5,
            .w = 5,
            .x = WIDTH / 2 - 2,
            .y = 10,    
        }
    };

    SDL_Event e;
    bool running = true;
   
    while (running) {
        SDL_SetRenderDrawColor(renderer, 10, 10, 10, 255);
        SDL_RenderClear(renderer);
         
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                 running = false;
                 break;
            }
            if (e.type == SDL_EVENT_KEY_DOWN)  {
                switch (e.key.key) {
                    case SDLK_S:
                        paddles[0].y = 1;
                        break;
                    case SDLK_W:
                        paddles[0].y = -1;
                        break;
                }
            }
            if (e.type == SDL_EVENT_KEY_UP)  {
                switch (e.key.key) {
                    case SDLK_S:
                        paddles[0].y = 0;
                        break;
                    case SDLK_W:
                        paddles[0].y = 0;
                        break;
                }
            } 
        }
         
        update_paddles(paddles, NUM_PADDLES);
        draw_background(field); 
        draw_paddles(paddles);

        SDL_RenderPresent(renderer);   
        
        SDL_Delay(17);
           
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
    
}

