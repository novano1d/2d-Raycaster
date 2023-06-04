#include <SDL2/SDL.h>
#include <math.h>
#include <iostream>

//helper functions

struct Point 
{
    double x,y;
    Point operator*(const double& a) const
    {
        return {a*x, a*y};
    }
    Point operator+(const Point& a) const
    {
        return {a.x+x, a.y+y};
    }
};

//returns end
Point drawLine(SDL_Renderer* renderer, int startX, int startY, double angle, int length) 
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    double angleRadians = angle * M_PI / 180.0;
    int endX = startX + static_cast<int>(length * std::cos(angleRadians));
    int endY = startY + static_cast<int>(length * std::sin(angleRadians));
    SDL_RenderDrawLine(renderer, startX, startY, endX, endY);
    return {endX, endY};
}

//screen params
const int SCREEN_WIDTH = 600;
const int SCREEN_HEIGHT = 600;

const int MAX_RAY_DISTANCE = (SCREEN_WIDTH > SCREEN_HEIGHT) ? SCREEN_WIDTH : SCREEN_HEIGHT;

//grid params
const int GRID_ROWS = 20;
const int GRID_COLS = 20;
bool map[GRID_ROWS][GRID_COLS] = {false};
int cellWidth = SCREEN_WIDTH / GRID_COLS;
int cellHeight = SCREEN_HEIGHT / GRID_ROWS;
const int PLAYER_SIZE = 20;
const int SPEED = 10;

//DDA raycast implementation
double raycast(Point start, double angle) {
    double angleRadians = angle * M_PI / 180.0;
    //using point as 2d vector to keep clean
    Point rayDir = { cos(angleRadians), sin(angleRadians) };
    Point rayUnitStepSize = { sqrt( 1 + (rayDir.y / rayDir.x) * (rayDir.y / rayDir.x)), sqrt( 1 + (rayDir.x / rayDir.y) * (rayDir.x / rayDir.y)) };

    Point mapCheck = {(int)start.x, (int)start.y};
    Point rayLength;

    Point step;
    
    if (rayDir.x < 0) 
    {
        step.x = -1;
        rayLength.x = (start.x - mapCheck.x) * rayUnitStepSize.x;
    }
    else
    {
        step.x = 1;
        rayLength.x = (mapCheck.x + 1 - start.x) * rayUnitStepSize.x;
    } 
    if (rayDir.y < 0) 
    {
        step.y = -1;
        rayLength.y = (start.y - mapCheck.y) * rayUnitStepSize.y;
    }
    else 
    {
        step.y = 1;
        rayLength.y = (mapCheck.y + 1 - start.y) * rayUnitStepSize.y;
    }
    bool tileFound = false;
    double maxDistance = (SCREEN_WIDTH > SCREEN_HEIGHT) ? SCREEN_WIDTH : SCREEN_HEIGHT;
    double distance = 0;
    while (!tileFound && distance < maxDistance)
    {
        if (rayLength.x < rayLength.y)
        {
            mapCheck.x += step.x;
            distance = rayLength.x;
            rayLength.x += rayUnitStepSize.x;
        }
        else
        {
            mapCheck.y += step.y;
            distance = rayLength.y;
            rayLength.y += rayUnitStepSize.y;
        }
        if (mapCheck.x >= 0 && mapCheck.x < SCREEN_WIDTH && mapCheck.y >= 0 && mapCheck.y < SCREEN_HEIGHT)
        {
            if (map[(int)(mapCheck.y/cellHeight)][(int)(mapCheck.x/cellWidth)])
            {
                Point end = start + rayDir * distance;
                return hypot(rayDir.x * distance, rayDir.y * distance);
            } 
        }
    }
    return 1000000; //return large number
}



int main(int argc, char **argv)
{
    //window
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);

    //stuff for logic
    Point playerPos = { SCREEN_WIDTH/2, SCREEN_HEIGHT/2 };
    bool quit = false;
    
    
    int angle = 0; //player angle

    //set up player texture
    SDL_Surface* surface = SDL_CreateRGBSurface(0, PLAYER_SIZE, PLAYER_SIZE, 32, 0, 0, 0, 0);
    SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, 255, 0, 0));  // Fill with red color
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    while (!quit) 
    {
        SDL_Event event;
        while (SDL_PollEvent(&event)) 
        {
            switch (event.type)
            {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) 
                {
                    case SDLK_w:
                    case SDLK_UP:
                        //forward
                        playerPos.y += SPEED*sin(angle*M_PI/180);
                        playerPos.x += SPEED*cos(angle*M_PI/180);
                        break;
                    case SDLK_d:
                    case SDLK_RIGHT:
                        //right
                        angle += 1;
                        angle = fmod(angle, 360); //keep value between 0 and 360
                        break;
                    case SDLK_s:
                    case SDLK_DOWN:
                        //backward
                        playerPos.y -= SPEED*sin(angle*M_PI/180);
                        playerPos.x -= SPEED*cos(angle*M_PI/180);
                        break;
                    case SDLK_a:
                    case SDLK_LEFT:
                        //left
                        angle -= 1;
                        angle = fmod(angle, 360); 
                        break;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                map[event.motion.y/cellHeight][event.motion.x/cellWidth] = !map[event.motion.y/cellHeight][event.motion.x/cellWidth];
                break;
            case SDL_QUIT: //handle user quit
                quit = true;
                break;
            }
        }
        //deal with render updates here down
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        //draw squares
        for (int i = 0; i < GRID_ROWS; i++){
            for (int j = 0; j < GRID_COLS; j++){
                if (map[i][j])
                {
                    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                    SDL_Rect square = { j * cellWidth, i*cellHeight, cellWidth, cellHeight};
                    SDL_RenderFillRect(renderer, &square);
                }
            }
        }
        //draw player
        SDL_Rect player = {(int)playerPos.x, (int)playerPos.y, PLAYER_SIZE, PLAYER_SIZE};
        SDL_RenderCopyEx(renderer, texture, nullptr, &player, angle, nullptr, SDL_FLIP_NONE);
        const int FOV = 50; //actual fov is 2x this value
        for (int i = angle - FOV; i < angle + FOV; i++) drawLine(renderer, (int)playerPos.x + PLAYER_SIZE/2, (int)playerPos.y + PLAYER_SIZE/2, i, raycast({playerPos.x + PLAYER_SIZE/2, playerPos.y + PLAYER_SIZE/2}, i));


        //draw lines
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (int i = 1; i <= GRID_ROWS; i++) SDL_RenderDrawLine(renderer, 0, i*cellHeight, SCREEN_WIDTH, i*cellHeight);
        for (int i = 1; i <= GRID_COLS; i++) SDL_RenderDrawLine(renderer, i*cellWidth, 0, i*cellWidth, SCREEN_HEIGHT);
        SDL_RenderPresent(renderer);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}