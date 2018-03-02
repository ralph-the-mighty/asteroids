

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <stdio.h>

#include "jmath.h"


const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;



struct KeyState {
    bool isDown;
    bool wasDown;
};



SDL_Window* gWindow = NULL;
SDL_Surface* gScreenSurface = NULL;
KeyState Keys[1024] = {0};

bool running = true;


#define ANGLE M_PI * 0.02
#define MAX_VEL 3.0f
#define MIN_VEL 0.0f


#define internal static
#define local_persist static
#define global_variable static



struct Player {
    v2 pos;
    v2 vel;
    v2 rotation;
};


struct GameState {
    Player player;
    int score;
};



GameState GlobalGameState;



bool init(){
    bool success = true;
    
    //Initialize SDL
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL failed to initialize! SDL_Error: %s\n", SDL_GetError());
        success = false;
    }else{
        //Create Window
        gWindow = SDL_CreateWindow("asdf", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if(gWindow == NULL) {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
            success = false;
        } else {
            //Get window surface
            gScreenSurface = SDL_GetWindowSurface(gWindow);
        }
    }
    return success;
}


void close() {
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;
    SDL_Quit();
}



void DrawRect(SDL_Surface* Surface, int RectX, int  RectY, int RectW, int RectH, unsigned char R, unsigned char G, unsigned char B) {
    
    unsigned char* Row = (unsigned char*)Surface->pixels;
    for(int y = 0; y < Surface->h; y++) {
        unsigned int* Pixel = (unsigned int*)Row;
        for(int x = 0; x < Surface->w; x++) {
            if(x >= RectX && x <= RectX + RectW && y >= RectY && y <= RectY + RectH) {
                *Pixel = (R) | (G << 8) | (B << 16);
            }
            Pixel++;
            
        }
        Row += Surface->pitch;
    }
}



void PlotPoint(SDL_Surface* Surface, int X, int Y, int R, int G, int B){
    unsigned int* Pixel = (unsigned int *)((char*)Surface->pixels + Y * Surface->pitch + X*4);
    //*Pixel = SDL_MapRGB(Surface->format, R, G, B);
    *Pixel = (R) | (G << 8) | (B << 16);
}


void DrawLineBresenham(SDL_Surface* Surface, int X0, int Y0, int X1, int Y1, int R, int G, int B) {
    //IMPORTANT: swap endpoints if X0 is greater than Y0
    if(X0 > X1){
        int TempX = X0;
        int TempY = Y0;
        
        X0 = X1;
        Y0 = Y1;
        
        X1 = TempX;
        Y1 = TempY;
    }
    
    
    float DeltaX = X0 - X1;
    float DeltaY = Y0 - Y1;
    float Slope = DeltaY / DeltaX;
    
    
    
    // four basic cases: horizontal line, vertical line, ABS(slope) >= 1,
    // ABS(slope) < 1.  
    
    if(X0 == X1){
        //vertical line
        // TODO(JOSH): Too many conditionals here. break into separate branches
        for(int Y = MIN(Y0, Y1); Y < MAX(Y0, Y1) + 1; Y++) {
            //TODO: just set the bloody pixels
            PlotPoint(Surface,X0, Y, R, G, B);
        }
    } else if (Y0 == Y1) {
        //horizontal line
        for(int X = MIN(X0, X1); X < MAX(X0, X1) + 1; X++) {
            //TODO: just set the bloody pixels
            PlotPoint(Surface,X, Y0, R, G, B);
        }
    } else if (ABS(Slope) < 1) {
        //shallow line
        float Error = 0;
        int Y = Y0;
        
        for(int X = X0; X < X1; X++) {
            PlotPoint(Surface, X, Y, R, G, B);
            Error += ABS(Slope);
            while(Error >= 0.5) {
                Y += DeltaY < 0 ? 1 : -1;
                Error -= 1;
            }
        }
        
    } else {
        //deep line
        
        float DeltaError = 1 / ABS(Slope);
        float Error = 0;
        int X = X0;
        
        if(Y0 < Y1) {
            for(int Y = Y0; Y < Y1; Y++) {
                PlotPoint(Surface, X, Y, R, G, B);
                Error += DeltaError;
                while(Error >= 0.5) {
                    X += DeltaX < 0 ? 1 : -1;
                    Error -= 1;
                }
            }
        } else {
            for(int Y = Y0; Y > Y1; Y--) {
                PlotPoint(Surface, X, Y, R, G, B);
                Error += DeltaError;
                while(Error >= 0.5) {
                    X += DeltaX < 0 ? 1 : -1;
                    Error -= 1;
                }
            }
        }
    }
}



void DrawLineWu(SDL_Surface* Surface, int X0, int Y0, int X1, int Y1) {
    // TODO(Josh): swap function
    if(X0 > X1){
        int TempX = X0;
        int TempY = Y0;
        
        X0 = X1;
        Y0 = Y1;
        
        X1 = TempX;
        Y1 = TempY;
    }
    
    
    float DeltaX = X0 - X1;
    float DeltaY = Y0 - Y1;
    float Slope = DeltaY / DeltaX;
    
    
    if(X0 == X1){
        //vertical line
        // TODO(JOSH): Too many conditionals here. break into separate branches
        for(int Y = MIN(Y0, Y1); Y <= MAX(Y0, Y1) + 1; Y++) {
            //TODO: just set the bloody pixels
            PlotPoint(Surface,X0, Y, 255, 255, 255);
        }
    } else if (Y0 == Y1) {
        //horizontal line
        for(int X = MIN(X0, X1); X <= MAX(X0, X1) + 1; X++) {
            //TODO: just set the bloody pixels
            PlotPoint(Surface,X, Y0, 255, 255, 255);
        }
    } else if (ABS(Slope) < 1) {
        //shallow line
        
        float Y = Y0;
        for(int X = X0; X < X1; X++) {
            float I1 = 1 - FractionalPart(Y);
            float I2 = FractionalPart(Y);
            PlotPoint(Surface, X, IntegerPart(Y),     I2 * 255, I2 * 255, I2 * 255);
            PlotPoint(Surface, X, IntegerPart(Y) - 1, I1 * 255, I1 * 255, I1 * 255);
            Y += Slope;
        }
        
    } else {
        //deep line
        if (Y0 < Y1) {
            float X = X0;
            for(int Y = Y0; Y < Y1; Y++) {
                float I1 = 1 - FractionalPart(X);
                float I2 = FractionalPart(X);
                PlotPoint(Surface, IntegerPart(X),     Y, I2 * 255, I2 * 255, I2 * 255);
                PlotPoint(Surface, IntegerPart(X) - 1 ,Y, I1 * 255, I1 * 255, I1 * 255);
                X += 1/Slope;
            }
        } else {
            float X = X0;
            for(int Y = Y0; Y > Y1; Y--) {
                float I1 = 1 - FractionalPart(X);
                float I2 = FractionalPart(X);
                PlotPoint(Surface, IntegerPart(X),     Y, I2 * 255, I2 * 255, I2 * 255);
                PlotPoint(Surface, IntegerPart(X) - 1 ,Y, I1 * 255, I1 * 255, I1 * 255);
                X -= 1/Slope;
            }
        }
    }
}



void DrawPlayer(SDL_Surface* Surface, Player* player, bool AntiAliased) {
    
    v2 Point1 = {0};
    v2 Point2 = {0};
    v2 Point3 = {0};
    
    
    v2 PerpRotation = {0};
    PerpRotation.x = player->rotation.y;
    PerpRotation.y = -player->rotation.x;
    
    Point1 = player->pos + PerpRotation * 8;
    Point2 = player->pos - PerpRotation * 8;
    Point3 = player->pos + player->rotation * 20;
    
    
    if(AntiAliased) {
        
        DrawLineWu(Surface, (int) Point1.x, (int) Point1.y, (int) Point2.x, (int) Point2.y);
        DrawLineWu(Surface, (int) Point2.x, (int) Point2.y, (int) Point3.x, (int) Point3.y);
        DrawLineWu(Surface, (int) Point3.x, (int) Point3.y, (int) Point1.x, (int) Point1.y);
    } else {
        
        DrawLineBresenham(Surface, (int) Point1.x, (int) Point1.y, (int) Point2.x, (int) Point2.y, 0xff, 0xff, 0xff);
        DrawLineBresenham(Surface, (int) Point2.x, (int) Point2.y, (int) Point3.x, (int) Point3.y, 0xff, 0xff, 0xff);
        DrawLineBresenham(Surface, (int) Point3.x, (int) Point3.y, (int) Point1.x, (int) Point1.y, 0xff, 0xff, 0xff);
    }
    
    /*
    DrawLineBresenham(Surface, (int)player->pos.x - 8, (int)player->pos.y, (int)player->pos.x + 8, (int)player->pos.y, 0xff, 0xff, 0xff);
    DrawLineBresenham(Surface, (int)player->pos.x + 8, (int)player->pos.y, (int)player->pos.x, (int)player->pos.y + 20, 0xff, 0xff, 0xff);
    DrawLineBresenham(Surface, player->pos.x, player->pos.y + 20, (int)player->pos.x - 8, (int) player->pos.y, 0xff, 0xff, 0xff);
    */
}







void UpdateAndDraw(GameState* game, SDL_Surface* Surface) {
    local_persist bool DrawAntialiased = false;
    
    
    //ProcessInput(game, Surface);
    if(Keys[SDL_SCANCODE_LEFT].isDown) {
        v2 NewRotation = {0};
        NewRotation.x = GlobalGameState.player.rotation.x * cos(-ANGLE) - GlobalGameState.player.rotation.y * sin(-ANGLE);
        NewRotation.y = GlobalGameState.player.rotation.x * sin(-ANGLE) + GlobalGameState.player.rotation.y * cos(-ANGLE);
        
        GlobalGameState.player.rotation = NewRotation;
        
    }
    
    if(Keys[SDL_SCANCODE_RIGHT].isDown) {
        v2 NewRotation = {0};
        NewRotation.x = GlobalGameState.player.rotation.x * cos(ANGLE) - GlobalGameState.player.rotation.y * sin(ANGLE);
        NewRotation.y = GlobalGameState.player.rotation.x * sin(ANGLE) + GlobalGameState.player.rotation.y * cos(ANGLE);
        
        GlobalGameState.player.rotation = NewRotation;
    }
    if(Keys[SDL_SCANCODE_UP].isDown) {
        GlobalGameState.player.vel = GlobalGameState.player.vel + GlobalGameState.player.rotation * 0.05;
        if (length(GlobalGameState.player.vel) > MAX_VEL) {
            GlobalGameState.player.vel = normalize(GlobalGameState.player.vel) * MAX_VEL;
        }
    }
    
    if(Keys[SDL_SCANCODE_S].isDown && !Keys[SDL_SCANCODE_S].wasDown) {
        DrawAntialiased = !DrawAntialiased;
    }
    
    //update
    game->player.pos.x += game->player.vel.x;
    game->player.pos.y += game->player.vel.y;
    
    if (game->player.pos.x < 20) {
        game->player.pos.x = SCREEN_WIDTH - 20;
    }
    
    if (game->player.pos.x > SCREEN_WIDTH - 20) {
        game->player.pos.x = 20;
    }
    
    
    if (game->player.pos.y < 20) {
        game->player.pos.y = SCREEN_HEIGHT - 20;
    }
    
    if (game->player.pos.y > SCREEN_HEIGHT - 20) {
        game->player.pos.y = 20;
    }
    
    
    
    //Drawing
    SDL_FillRect(Surface, NULL, SDL_MapRGB(Surface->format, 0, 0, 0));
    DrawPlayer(Surface, &(game->player), DrawAntialiased);
    
    
    //TEST
    
#if 0    
    local_persist int X0 = 320;
    local_persist int Y0 = 240;
    
    local_persist int X1 = 420;
    local_persist int Y1 = 290;
    
    
    DrawLineWu(gScreenSurface, X0, Y0, X1, Y1);
    
    if(Keys[SDL_SCANCODE_RIGHT]) {
        X1++;
    }
    
    if(Keys[SDL_SCANCODE_LEFT]) {
        X1--;
    }
    
    if(Keys[SDL_SCANCODE_UP]) {
        Y1--;
    }
    
    if(Keys[SDL_SCANCODE_DOWN]) {
        Y1++;
    }
#endif
    
    
}



void ProcessEvents() {
    
    //Update keymap
    for(int i = 0; i < 1024; i++) {
        Keys[i].wasDown = Keys[i].isDown;
    }
    
    //Event Loop
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if(e.type == SDL_QUIT) {
            running = false;
        }else if(e.type == SDL_KEYDOWN) {
            int scancode = e.key.keysym.scancode;
            Keys[scancode].wasDown = Keys[scancode].isDown;
            Keys[scancode].isDown = true;
        }else if(e.type == SDL_KEYUP) {
            int scancode = e.key.keysym.scancode;
            Keys[scancode].wasDown = Keys[scancode].isDown;
            Keys[scancode].isDown = false;
        }
    }
    
}



int main( int argc, char* args[] ) {
    
    GlobalGameState.player.pos = V2(320, 240);
    GlobalGameState.player.vel = {0};
    GlobalGameState.player.rotation = {0};
    GlobalGameState.player.rotation.y = 1;
    
    
    if(!init()){
        return 0;
    }
    
    while(running) {
        ProcessEvents();
        UpdateAndDraw(&GlobalGameState, gScreenSurface);
        SDL_UpdateWindowSurface(gWindow);
        
        // TODO(Josh): do actual fixed framerate control
        SDL_Delay(10);
    }
    
    
    
    close();
    return 0;
    
}