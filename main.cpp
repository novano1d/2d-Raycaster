#include <SDL2/SDL.h>
#include <math.h>
#include <iostream>

//helper functions

struct Point 
{
    double x,y;
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

bool isIntersecting(Point p1, Point p2, Point q1, Point q2) {
    return (((q1.x-p1.x)*(p2.y-p1.y) - (q1.y-p1.y)*(p2.x-p1.x))
            * ((q2.x-p1.x)*(p2.y-p1.y) - (q2.y-p1.y)*(p2.x-p1.x)) < 0)
            &&
           (((p1.x-q1.x)*(q2.y-q1.y) - (p1.y-q1.y)*(q2.x-q1.x))
            * ((p2.x-q1.x)*(q2.y-q1.y) - (p2.y-q1.y)*(q2.x-q1.x)) < 0);
}

//screen params
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//grid params
const int GRID_ROWS = 8;
const int GRID_COLS = 8;
bool map[GRID_ROWS][GRID_COLS] = {false};
int cellWidth = SCREEN_WIDTH / GRID_COLS;
int cellHeight = SCREEN_HEIGHT / GRID_ROWS;
const int PLAYER_SIZE = 20;
//const int SPEED = 10;

//dda raycast implementation
double raycast(Point start, double angle)
{
    Point testpos = start;
    double Sy = cellHeight * sin(angle*M_PI/180);
    double Sx = cellWidth * cos(angle*M_PI/180);
    double remainderx = start.x - (int)start.x; //find scale of first cell
    double remaindery = start.y - (int)start.y;
    double fSy = remaindery * cellHeight * sin(angle*M_PI/180);
    double fSx = remainderx * cellWidth * cos(angle*M_PI/180);
    double raysizex, raysizey;
    std::cout << (int)(testpos.x/cellWidth) << " " << (int)(testpos.y/cellHeight) << " " << map[(int)(testpos.x/cellWidth)][(int)(testpos.y/cellHeight)] << std::endl;
    std::cout << map[6][7] << std::endl;
    testpos.x += fSx;
    //something is very very wrong :)))
    if (map[(int)(testpos.y/cellWidth)][(int)(testpos.x/cellHeight)])
    {
        raysizex = sqrt( pow(testpos.x - start.x, 2) + pow( testpos.y - start.y, 2) );
        return raysizex;
    }
    testpos.y += fSy;
    if (map[(int)(testpos.y/cellWidth)][(int)(testpos.x/cellHeight)])
    {
        raysizey = sqrt( pow(testpos.x - start.x, 2) + pow( testpos.y - start.y, 2) );
        return raysizey;
    }
    int its = 0;
    while (its < 100)
    {
        //std::cout << testpos.x << std::endl;
        if (raysizey < raysizex)
        {
            testpos.y += Sy;
            if (map[(int)(testpos.y/cellWidth)][(int)(testpos.x/cellHeight)])
            {
                raysizey = sqrt( pow(testpos.x - start.x, 2) + pow( testpos.y - start.y, 2) );
                return raysizey;
            }
        }
        else
        {
            testpos.x += Sx;
            if (map[(int)(testpos.y/cellWidth)][(int)(testpos.x/cellHeight)])
            {
                raysizex = sqrt( pow(testpos.x - start.x, 2) + pow( testpos.y - start.y, 2) );
                return raysizex;
            }
        }
        its++;
    }
    
    return -1;
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
                        playerPos.y += sin(angle*M_PI/180);
                        playerPos.x += cos(angle*M_PI/180);
                        break;
                    case SDLK_d:
                    case SDLK_RIGHT:
                        //right
                        angle += 1;
                        break;
                    case SDLK_s:
                    case SDLK_DOWN:
                        //backward
                        playerPos.y -= sin(angle*M_PI/180);
                        playerPos.x -= cos(angle*M_PI/180);
                        break;
                    case SDLK_a:
                    case SDLK_LEFT:
                        //left
                        angle -= 1;
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
        drawLine(renderer, (int)playerPos.x + PLAYER_SIZE/2, (int)playerPos.y + PLAYER_SIZE/2, angle, raycast({playerPos.x + PLAYER_SIZE/2, playerPos.y + PLAYER_SIZE/2}, angle));

        //raycasting DDA


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