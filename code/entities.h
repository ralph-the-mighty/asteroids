#include "jmath.h"

struct asteroid {
    //in object space
    v2 vertices[5];
    float size;
    
    float rot;
    float rot_vel;
    
    v2 pos;
    v2 vel;
};


