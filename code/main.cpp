#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#include "entities.h"
#include "jmath.h"
#include "array.cpp"




#define TURN_RATE M_PI * 1.5f        // radians / sec
#define THRUST_VEL 500.0f            // pixels / sec^2
#define PLAYER_MAX_VEL 400.0f        // pixels / sec
#define PLAYER_MIN_VEL 0.0f          // pixels / sec
#define BULLET_VEL 500.0f            // pixels / sec

#define internal static
#define local_persist static
#define global_variable static

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))


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
bool paused = false;
bool debug_mode = false;
unsigned int frame = 0;


#define ISDOWN(key) (Keys[SDL_SCANCODE_##key].isDown)
#define WASDOWN(key) (Keys[SDL_SCANCODE_##key].wasDown)
#define CAMEDOWN(key) (ISDOWN(key) && !WASDOWN(key))



struct Player {
    v2 pos;
    v2 vel;
    v2 rotation;
};


struct GameState {
    Player player;
    int score;
    array<Asteroid> asteroids;
    array<Bullet> bullets;
};



GameState GlobalGameState = {0};




bool SDL_Init(){
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


bool IsConvex(v2* poly, int n) {
    
    v2 p0 = poly[0];
    v2 p1 = poly[1];
    v2 p2 = poly[2];
    v2 d1 = p1 - p0;
    v2 d2 = p2 - p1;
    bool firstsign = (d1.x * d2.y - d1.y * d2.x) >= 0 ? true : false;
    
    for(int i = 1; i < n; i++){
        p0 = poly[i];
        p1 = poly[(i + 1) % n];
        p2 = poly[(i + 2) % n];
        
        d1 = p1 - p0;
        d2 = p2 - p1;
        
        bool nextsign = (d1.x * d2.y - d1.y * d2.x) >= 0 ? true : false;
        
        if (nextsign != firstsign) return false;
    }
    return true;
}


void GenAsteroid(GameState* game, v2 pos, float size, int gen) {
    Asteroid a = {0};
    a.pos = pos;
    a.size = size;
    a.gen = gen;
    
    do {
        for (int j = 0; j < 5; j++){
            float angle = j * (2 * M_PI) / 5;
            float distance = rand_in_range(a.size / 4, a.size);
            a.vertices[j] = {(float)cos(angle) * (distance), (float)sin(angle) * distance};
        }
        
    } while(!IsConvex(a.vertices, 5));
    
    
    //adjust points so that origin is center of gravity;
    
    
    v2 sum = {0};
    float f;
    float twicearea = 0;
    
    for(int k = 0; k < 5; k++) {
        v2 p1 = a.vertices[k];
        v2 p2 = a.vertices[(k + 1) % 5];
        f = (p1.x * p2.y - p2.x * p1.y);
        sum.x += (p1.x + p2.x) * f;
        sum.y += (p1.y + p2.y) * f;
        twicearea += f;
    }
    
    for(int k = 0; k < 5; k++) {
        a.vertices[k].x -= (sum.x / (twicearea * 3));
        a.vertices[k].y -= (sum.y / (twicearea * 3));
    }
    
    a.vel = {randf() * 100.0f - 50.0f, randf() * 100.0f - 50.0f };
    a.rot_vel = randf() * 5 - 5;
    
    game->asteroids.insert(a);
    
}



void GenAsteroids(GameState* game, int count) {
    game->asteroids.destroy();
    game->asteroids.init();
    
    for(int i = 0; i < count; i++) {
        v2 pos = { randf() * SCREEN_WIDTH, randf() * SCREEN_HEIGHT };
        GenAsteroid(game, pos, 50, 2);
    }
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
    if(X < 0) X += SCREEN_WIDTH;
    if(Y < 0) Y +=SCREEN_HEIGHT;
    
    if(X >= SCREEN_WIDTH)  X -= SCREEN_WIDTH;
    if(Y >= SCREEN_HEIGHT) Y -= SCREEN_HEIGHT;
    unsigned int* Pixel = (unsigned int *)((char*)Surface->pixels + Y * Surface->pitch + X*4);
    //*Pixel = SDL_MapRGB(Surface->format, R, G, B);
    *Pixel = (B) | (G << 8) | (R << 16);
}



void PlotPointBlend(SDL_Surface* Surface, int X, int Y, unsigned char Brightness) {
    if(X < 0) X += SCREEN_WIDTH;
    if(Y < 0) Y += SCREEN_HEIGHT;
    
    if(X >= SCREEN_WIDTH)  X -= SCREEN_WIDTH;
    if(Y >= SCREEN_HEIGHT) Y -= SCREEN_HEIGHT;
    
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
            PlotPoint(Surface, X0, Y, R, G, B);
        }
    } else if (Y0 == Y1) {
        //horizontal line
        for(int X = MIN(X0, X1); X <= MAX(X0, X1); X++) {
            //TODO: just set the bloody pixels
            PlotPoint(Surface, X, Y0, R, G, B);
        }
    } else if (ABS(Slope) < 1) {
        //shallow line
        float Error = 0;
        int Y = Y0;
        
        for(int X = X0; X <= X1; X++) {
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
            for(int Y = Y0; Y <= Y1; Y++) {
                PlotPoint(Surface, X, Y, R, G, B);
                Error += DeltaError;
                while(Error >= 0.5) {
                    X += DeltaX < 0 ? 1 : -1;
                    Error -= 1;
                }
            }
        } else {
            for(int Y = Y0; Y >= Y1; Y--) {
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


void DrawLineBresenham(SDL_Surface *surface, v2 p1, v2 p2, int r, int g, int b) {
    DrawLineBresenham(surface, (int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y, r, g, b);
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

void DrawLineWu(SDL_Surface *surface, v2 p1, v2 p2) {
    DrawLineWu(surface, (int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y);
}



void DrawTriangle(SDL_Surface* Surface, v2 p1, v2 p2, v2 p3) {
    DrawLineWu(Surface, Round(p1.x), Round(p1.y), Round(p2.x), Round(p2.y));
    DrawLineWu(Surface, Round( p2.x), Round(p2.y), Round(p3.x), Round(p3.y));
    DrawLineWu(Surface, Round(p3.x), Round(p3.y), Round(p1.x), Round(p1.y));
}



void DrawPlayer(SDL_Surface* Surface, Player* player) {
    
    v2 Point1 = {0};
    v2 Point2 = {0};
    v2 Point3 = {0};
    
    
    v2 PerpRotation = {0};
    PerpRotation.x = player->rotation.y;
    PerpRotation.y = -player->rotation.x;
    
    Point1 = player->pos + (PerpRotation * 6) - (player->rotation * 5);
    Point2 = player->pos - (PerpRotation * 6) - (player->rotation * 5);
    Point3 = player->pos + player->rotation * 15;
    
    
    DrawTriangle(Surface, Point1, Point2, Point3);
    
    
    
    //draw flame
    
    if(ISDOWN(UP) && (frame % 2) == 0) {
        v2 p1 = Point1 - (player->rotation * 2) - PerpRotation * 3;
        v2 p2 = Point2 - (player->rotation * 2) + PerpRotation * 3;
        v2 p3 = player->pos - (player->rotation * 10);
        
        DrawTriangle(Surface, p1, p2, p3);
    }
    
    
    
}


void DrawMarker(SDL_Surface *Surface, int x, int y, int R, int G, int B) {
    PlotPoint(Surface, x, y, R, G, B); 
    PlotPoint(Surface, x, y + 1, R, G, B); 
    PlotPoint(Surface, x, y - 1, R, G, B); 
    PlotPoint(Surface, x + 1, y, R, G, B); 
    PlotPoint(Surface, x - 1, y, R, G, B);
}



void DrawAsteroids(SDL_Surface* Surface, GameState* game) {
    
    for(int i = 0; i < game->asteroids.length; i++) {
        Asteroid a = game->asteroids[i];
        int j;
        for(j = 0; j < ARRAY_SIZE(a.vertices) - 1; j++) {
            DrawLineWu(Surface, transform(a.rot, a.pos, a.vertices[j]), transform(a.rot, a.pos, a.vertices[j + 1]));
            
        }
        
        DrawLineWu(Surface, transform(a.rot, a.pos, a.vertices[j]), transform(a.rot, a.pos, a.vertices[0]));
        
        
        if(debug_mode) { 
            DrawMarker(Surface, (int)a.pos.x, (int)a.pos.y, 255, 0, 0);
        }
    }
}



inline void WrapPosition(v2* pos) {
    
    if(pos->x < 0) pos->x += SCREEN_WIDTH;
    if(pos->y < 0) pos->y += SCREEN_HEIGHT;
    
    if(pos->x >= SCREEN_WIDTH)  pos->x -= SCREEN_WIDTH;
    if(pos->y >= SCREEN_HEIGHT) pos->y -= SCREEN_HEIGHT;
}




void Update(GameState* game, double dt) {
    frame++;
    
    
    if(CAMEDOWN(P)) {
        paused = !paused;
    }
    
    
    if(CAMEDOWN(D)) {
        debug_mode = !debug_mode;
    }
    
    if(CAMEDOWN(SPACE) && paused) {
        goto updateplayer;
    }
    
    
    if(CAMEDOWN(G)) {
        GenAsteroids(&GlobalGameState, 10);
    }
    
    if(CAMEDOWN(K)) {
        game->asteroids.remove(0);
    }
    
    if (paused) return;
    
    
    
    updateplayer:
    //update player
    if(ISDOWN(LEFT)) {
        v2 NewRotation = {0};
        NewRotation.x = game->player.rotation.x * cos(-TURN_RATE * dt) - game->player.rotation.y * sin(-TURN_RATE * dt);
        NewRotation.y = game->player.rotation.x * sin(-TURN_RATE * dt) + game->player.rotation.y * cos(-TURN_RATE * dt);
        
        game->player.rotation = NewRotation;
        
    }
    
    if(ISDOWN(RIGHT)) {
        v2 NewRotation = {0};
        NewRotation.x = game->player.rotation.x * cos(TURN_RATE * dt) - game->player.rotation.y * sin(TURN_RATE * dt);
        NewRotation.y = game->player.rotation.x * sin(TURN_RATE * dt) + game->player.rotation.y * cos(TURN_RATE * dt);
        
        game->player.rotation = NewRotation;
    }
    
    if(ISDOWN(UP)) {
        game->player.vel = game->player.vel + game->player.rotation * THRUST_VEL * dt;
        if (length(game->player.vel) > PLAYER_MAX_VEL) {
            game->player.vel = normalize(game->player.vel) * PLAYER_MAX_VEL;
        }
    }
    
    if(CAMEDOWN(SPACE)) {
        Bullet bullet = {0};
        bullet.lifetime = 2.0f;
        bullet.pos = game->player.pos + game->player.rotation * 15;
        bullet.vel = (game->player.rotation * BULLET_VEL);
        game->bullets.insert(bullet);
    }
    
    
    
    game->player.pos.x += game->player.vel.x * dt;
    game->player.pos.y += game->player.vel.y * dt;
    WrapPosition(&(game->player.pos));
    
    
    //update asteroids
    // TODO(JOSH): Find the bug where the positions dont update correctly at the edges
    for(int i = 0; i < game->asteroids.length; i++) {
        Asteroid* a = &(game->asteroids[i]);
        a->pos.x += a->vel.x * dt;
        a->pos.y += a->vel.y * dt;
        a->rot += a->rot_vel * dt;
        if (a->rot >= 2 * M_PI) {
            a->rot -= 2 * M_PI;
        }
        
        WrapPosition(&(a->pos));
    }
    
    
    
    //update bullets
    
    for(int i = 0; i < game->bullets.length; i++) {
        game->bullets[i].lifetime -= dt;
        
        if (game->bullets[i].lifetime <= 0) {
            game->bullets.remove(i);
            continue;
        }
        
        game->bullets[i].pos = game->bullets[i].pos + game->bullets[i].vel * dt;
        WrapPosition(&(game->bullets[i].pos));
    }
    
    
    //collision detection
    for(int i = 0; i < game->asteroids.length; i++) {
        Asteroid a = game->asteroids[i];
        v2 transformed_points[5];
        for(int j = 0; j < 5; j++) {
            transformed_points[j] = transform(a.rot, a.pos, a.vertices[j]);
        }
        for(int k = 0; k < game->bullets.length; k++) {
            if(PointInPolygon(game->bullets[k].pos , transformed_points, 5)) {
                if(a.gen > 0) {
                    GenAsteroid(game, a.pos, a.size * 0.75, a.gen - 1);
                    GenAsteroid(game, a.pos, a.size * 0.75, a.gen - 1);
                }
                game->bullets.remove(k);
                game->asteroids.remove(i);
            }
        }
    }
}


void DrawBullets(SDL_Surface* Surface, GameState* game) {
    
    for(int i = 0; i < game->bullets.length; i++) {
        Bullet b = game->bullets[i];
        int trail_length = 5;
        v2 trail_pos = b.pos;
        v2 trail_dir = -normalize(b.vel);
        PlotPoint(Surface, (int)b.pos.x, (int)b.pos.y, 255, 255, 255);
        for(int i = 1; i <= trail_length; i++) {
            PlotPoint(Surface, (int)trail_pos.x, (int) trail_pos.y, 255 / i, 255 / i, 255 / i);
            trail_pos = trail_pos + trail_dir;
        }
    }
}

void Draw(GameState* game, SDL_Surface* Surface) { 
    
    //Drawing
    SDL_FillRect(Surface, NULL, SDL_MapRGB(Surface->format, 0, 0, 0));
    DrawAsteroids(Surface, game);
    DrawBullets(Surface, game);
    DrawPlayer(Surface, &(game->player));
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



void InitGame(GameState* game) {
    game->player.pos = V2(320, 240);
    game->player.vel = {0};
    game->player.rotation = {0};
    game->player.rotation.y = 1;
    
    game->bullets.init();
    
    GenAsteroids(game, 10);
} 




void WriteDebugStats(GameState* State) {
    FILE* file = fopen("debug.txt", "w");
    fprintf(file, "DEBUG INFO FRAME %d\n\n", frame);
    fprintf(file, "Player info:\n");
    fprintf(file, "  %f, %f", State->player.pos.x, State->player.pos.y);
    fprintf(file, "\nAsteroid info:\n");
    for(int i = 0; i < State->asteroids.length; i++) {
        Asteroid a = State->asteroids[i];
        fprintf(file, "  %i: %f, %f\n", i, a.pos.x, a.pos.y); 
    }
    fclose(file);
}





int main( int argc, char* args[] ) {
    if(!SDL_Init()){
        return 0;
    }
    
    srand(time(NULL));
    
    InitGame(&GlobalGameState);
    
    double SecondsPerTick = 1.0 / SDL_GetPerformanceFrequency();
    const double dt = 1.0f / 60.0f;
    double currentTime = SDL_GetPerformanceCounter() * SecondsPerTick;
    double accumulator = 0.0;
    
    
    while(running) {
        double newTime = SDL_GetPerformanceCounter() * SecondsPerTick;
        double frameTime = newTime - currentTime;
        currentTime = newTime;
        accumulator += frameTime;
        
        printf("Time this frame: %f\r", frameTime);
        
        while(accumulator >= dt) {
            ProcessEvents();
            Update(&GlobalGameState, dt);
            //WriteDebugStats(&GlobalGameState);
            accumulator -= dt;
        }
        
        Draw(&GlobalGameState, gScreenSurface);
        SDL_UpdateWindowSurface(gWindow);
    }
    close();
    return 0;
}