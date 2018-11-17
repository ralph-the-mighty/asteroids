

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <stdio.h>
#include "enemies.h"
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

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))


struct Player {
    v2 pos;
    v2 vel;
    v2 rotation;
};


struct GameState {
    Player player;
    int score;
    
    asteroid* Asteroids;
    unsigned int AsteroidCount;
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



void PlotPoint(SDL_Surface* Surface, int X, int Y, unsigned int R, unsigned int G, unsigned int B){
    unsigned int* Pixel = (unsigned int *)((char*)Surface->pixels + Y * Surface->pitch + X*4);
    //*Pixel = SDL_MapRGB(Surface->format, R, G, B);
    *Pixel = (R) | (G << 8) | (B << 16);
}



void PlotPointBlend(SDL_Surface* Surface, int X, int Y, unsigned char Brightness) {
    unsigned int* PixelP = (unsigned int *)((char*)Surface->pixels + Y * Surface->pitch + X*4);
    if (Brightness > (unsigned char) *PixelP) {
        *PixelP = (Brightness) | (Brightness << 8) | (Brightness << 16);
    }
    
    
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
        for(int Y = MIN(Y0, Y1); Y <= MAX(Y0, Y1); Y++) {
            //TODO: just set the bloody pixels
            PlotPointBlend(Surface, X0, Y, 0xff);
        }
    } else if (Y0 == Y1) {
        //horizontal line
        for(int X = MIN(X0, X1); X <= MAX(X0, X1); X++) {
            //TODO: just set the bloody pixels
            PlotPointBlend(Surface, X, Y0, 0xff);
        }
    } else if (ABS(Slope) < 1) {
        //shallow line
        float Error = 0;
        int Y = Y0;
        
        for(int X = X0; X <= X1; X++) {
            PlotPointBlend(Surface, X, Y, 0xff);
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
            for(int Y = Y0; Y <= Y1; Y++) {
                PlotPointBlend(Surface, X, Y, 0xff);
                Error += DeltaError;
                while(Error >= 0.5) {
                    X += DeltaX < 0 ? 1 : -1;
                    Error -= 1;
                }
            }
        } else {
            for(int Y = Y0; Y >= Y1; Y--) {
                PlotPointBlend(Surface, X, Y, 0xff);
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
        for(int Y = MIN(Y0, Y1); Y <= MAX(Y0, Y1); Y++) {
            //TODO: just set the bloody pixels
            //PlotPoint(Surface,X0, Y, 255, 255, 255);
            PlotPointBlend(Surface, X0, Y, 255);
        }
    } else if (Y0 == Y1) {
        //horizontal line
        for(int X = MIN(X0, X1); X <= MAX(X0, X1); X++) {
            //TODO: just set the bloody pixels
            PlotPointBlend(Surface,X, Y0, 255);
        }
    } else if (ABS(Slope) == 1) {
        //diagonal line
        int X = X0;
        int Y = Y0;
        while(X <= X1) {
            PlotPointBlend(Surface, X, Y, 255);
            X++;
            Y += (int)Slope;
        }
    } else if (ABS(Slope) < 1) {
        //shallow line
        //draw begin
        PlotPoint(Surface, X0, Y0, 0xff, 0xff, 0xff);
        float Y = Y0 + Slope;
        for(int X = X0 + 1; X < X1; X++) {
            float I1 = 1 - FractionalPart(Y);
            float I2 = FractionalPart(Y);
            PlotPointBlend(Surface, X, IntegerPart(Y),     I1 * 255);
            PlotPointBlend(Surface, X, IntegerPart(Y) + 1, I2 * 255);
            Y += Slope;
        }
        //draw end
        PlotPointBlend(Surface, X1, Y1, 0xff);
        
    } else {
        //deep line
        if (Y0 < Y1) {
            PlotPointBlend(Surface, X0, Y0, 0xff);
            float X = X0 + 1/Slope;
            for(int Y = Y0 + 1; Y < Y1; Y++) {
                float I1 = 1 - FractionalPart(X);
                float I2 = FractionalPart(X);
                PlotPointBlend(Surface, IntegerPart(X),     Y, I1 * 255);
                PlotPointBlend(Surface, IntegerPart(X) + 1 ,Y, I2 * 255);
                X += 1/Slope;
            }
            PlotPointBlend(Surface, X1, Y1, 0xff);
        } else {
            PlotPoint(Surface, X0, Y0, 0xff, 0xff, 0xff);
            float X = X0 - 1/Slope;
            for(int Y = Y0 - 1; Y > Y1; Y--) {
                float I1 = 1 - FractionalPart(X);
                float I2 = FractionalPart(X);
                PlotPointBlend(Surface, IntegerPart(X),     Y, I1 * 255);
                PlotPointBlend(Surface, IntegerPart(X) + 1 ,Y, I2 * 255);
                X -= 1/Slope;
                
            }
            PlotPointBlend(Surface, X1, Y1, 0xff);
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
    
    Point1 = player->pos + (PerpRotation * 6) - (player->rotation * 5);
    Point2 = player->pos - (PerpRotation * 6) - (player->rotation * 5);
    Point3 = player->pos + player->rotation * 15;
    
    
    if(AntiAliased) {
        
        DrawLineWu(Surface, Round(Point1.x), Round(Point1.y), Round(Point2.x), Round(Point2.y));
        DrawLineWu(Surface, Round( Point2.x), Round(Point2.y), Round(Point3.x), Round(Point3.y));
        DrawLineWu(Surface, Round(Point3.x), Round(Point3.y), Round(Point1.x), Round(Point1.y));
    } else {
        
        DrawLineBresenham(Surface, (int) Point1.x, (int) Point1.y, (int) Point2.x, (int) Point2.y, 0xff, 0xff, 0xff);
        DrawLineBresenham(Surface, (int) Point2.x, (int) Point2.y, (int) Point3.x, (int) Point3.y, 0xff, 0xff, 0xff);
        DrawLineBresenham(Surface, (int) Point3.x, (int) Point3.y, (int) Point1.x, (int) Point1.y, 0xff, 0xff, 0xff);
    }
    
}



void DrawAsteroids(SDL_Surface* Surface, GameState* State) {
    
    for(int i = 0; i < State->AsteroidCount; i++) {
        asteroid A = State->Asteroids[i];
        int j;
        for(j = 0; j < ARRAY_SIZE(A.vertices) - 1; j++) {
            
            DrawLineWu(Surface, (int)A.vertices[j].x + A.pos.x, (int) A.vertices[j].y + A.pos.y,
                       (int) A.vertices[j + 1].x + A.pos.x,
                       (int) A.vertices[j + 1].y + A.pos.y);
        }
        
        DrawLineWu(Surface, (int)A.vertices[j].x + A.pos.x, (int) A.vertices[j].y + A.pos.y,
                   (int) A.vertices[0].x + A.pos.x,
                   (int) A.vertices[0].y + A.pos.y);
    }
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
        GlobalGameState.player.vel = GlobalGameState.player.vel + GlobalGameState.player.rotation * 0.1;
        if (length(GlobalGameState.player.vel) > MAX_VEL) {
            GlobalGameState.player.vel = normalize(GlobalGameState.player.vel) * MAX_VEL;
        }
    }
    
    if(Keys[SDL_SCANCODE_S].isDown && !Keys[SDL_SCANCODE_S].wasDown) {
        DrawAntialiased = !DrawAntialiased;
    }
    
    //update player
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
    
    //update asteroids
    for(int i = 0; i < game->AsteroidCount; i++) {
        game->Asteroids[i].pos.x += game->Asteroids[i].vel.x;
        game->Asteroids[i].pos.y += game->Asteroids[i].vel.y;
        
        
        if (game->Asteroids[i].pos.x < 20) {
            game->Asteroids[i].pos.x = SCREEN_WIDTH - 20;
        }
        
        if (game->Asteroids[i].pos.x > SCREEN_WIDTH - 20) {
            game->Asteroids[i].pos.x = 20;
        }
        
        
        if (game->Asteroids[i].pos.y < 20) {
            game->Asteroids[i].pos.y = SCREEN_HEIGHT - 20;
        }
        
        if (game->Asteroids[i].pos.y > SCREEN_HEIGHT - 20) {
            game->Asteroids[i].pos.y = 20;
        }
        
    }
    
    
    
    //Drawing
    SDL_FillRect(Surface, NULL, SDL_MapRGB(Surface->format, 0, 0, 0));
    DrawAsteroids(Surface, &GlobalGameState);
    DrawPlayer(Surface, &(game->player), DrawAntialiased);
    
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




void InitGame(GameState* State) {
    State->player.pos = V2(320, 240);
    State->player.vel = {0};
    State->player.rotation = {0};
    State->player.rotation.y = 1;
    
    
    State->AsteroidCount = 5;
    
    State->Asteroids = (asteroid*)malloc(sizeof(asteroid) * State->AsteroidCount);
    for(int i = 0; i < State->AsteroidCount; i++) {
        State->Asteroids[i].vertices[0] = {-5,-5};
        State->Asteroids[i].vertices[1] = {-5,5};
        State->Asteroids[i].vertices[2] = {0,7};
        State->Asteroids[i].vertices[3] = {5,5};
        State->Asteroids[i].vertices[4] = {5,-5};
        
        State->Asteroids[i].pos = {(float)20 * i + 100, (float)20 * i + 100};
        State->Asteroids[i].vel = {(float)1 * i, (float)1 * i};
    }
} 



int main( int argc, char* args[] ) {
    
    
    
    if(!init()){
        return 0;
    }
    
    InitGame(&GlobalGameState);
    
    
    unsigned int FPS = 60;
    float MilisecondsPerFrame = 1 / FPS * 1000;
    Uint32 FrameTimeEnd = 0;
    Uint32 FrameTimeStart = 0;
    Uint32 FrameTimeElapsed = 0;
    
    
    while(running) {
        FrameTimeStart = SDL_GetTicks();
        ProcessEvents();
        UpdateAndDraw(&GlobalGameState, gScreenSurface);
        SDL_UpdateWindowSurface(gWindow);
        
        // TODO(Josh): do actual fixed framerate control
        FrameTimeEnd = SDL_GetTicks();
        FrameTimeElapsed = FrameTimeEnd - FrameTimeStart;
        if(FrameTimeElapsed < 16) {
            SDL_Delay(16 - FrameTimeElapsed);
        }
        printf("miliseconds this frame: %i\n", SDL_GetTicks() - FrameTimeStart);
    }
    
    close();
    return 0;
}