
#ifndef JMATH_H_INCLUDEGUARD
#define JMATH_H_INCLUDEGUARD

#include "math.h"
#include "stdlib.h"



//general utility

#define ABS(x) (x < 0 ? -x : x)
#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)


inline int IntegerPart(float f) {
    return (int) f;
}


float FractionalPart(float f) {
    return f - IntegerPart(f);
}


inline float Round(float f) {
    return IntegerPart(f + 0.5);
}



float randf() {
    return (float)rand() / (RAND_MAX + 1.0f);
}


// TODO(JOSH): test, debug these functions
float lerp(float a, float b, float e) {
    assert(e >= 0.0f && e <= 1.0f);
    assert(a <= b);
    return (b - a) * e + a;
}


float rand_in_range(float a, float b) {
    return lerp(a, b, randf());
}





//2d vectors

struct v2 {
    float x;
    float y;
    
    
    inline v2 operator*(float op) {
        v2 result;
        result.x = x * op;
        result.y = y * op;
        return result;
    }
    
    inline v2 operator+(v2 rhs) {
        v2 result;
        result.x = x + rhs.x;
        result.y = y + rhs.y;
        return result;
    }
    
    // TODO(JOSH): add other operators, check to see if this is actually correct and all that
    inline void operator+=(v2 rhs) {
        x += rhs.x;
        y += rhs.x;
        
    }
    
    
    inline v2 operator-(v2 rhs) {
        v2 result;
        result.x = x - rhs.x;
        result.y = y - rhs.y;
        return result;
    }
    
    inline v2 operator-() {
        v2 result;
        result.x = -x;
        result.y = -y;
        return result;
    }
};



inline v2 V2(float x, float y) {
    v2 v;
    v.x = x;
    v.y = y;
    return v;
};

inline float length(v2 v) {
    return sqrt(v.x*v.x + v.y*v.y);
}

inline v2 normalize(v2 v) {
    float l = length(v);
    if(l != 0) {
        v.x = v.x / l;
        v.y = v.y / l;
    }
    return v;
}

inline float dot(v2 u, v2 v) {
    return u.x * v.x + u.y * v.y;
}


inline v2 translate(v2 origin, v2 point) {
    return point + origin;
}

inline v2 rotate(float angle, v2 point) {
    v2 result;
    result.x = point.x * cos(angle) - point.y * sin(angle);
    result.y = point.x * sin(angle) + point.y * cos(angle);
    return result;
}


inline v2 transform(float angle, v2 origin, v2 point) {
    return translate(origin, rotate(angle, point));
}




//3D vectors

struct v3 {
    float x;
    float y;
    float z;
    
    inline v3 operator*(float op) {
        v3 result;
        result.x = x * op;
        result.y = y * op;
        result.z = z * op;
        return result;
    }
    
    inline v3 operator+(v3 rhs) {
        v3 result;
        result.x = x + rhs.x;
        result.y = y + rhs.y;
        result.z = z + rhs.z;
        return result;
    }
    
    inline v3 operator-(v3 rhs) {
        v3 result;
        result.x = x - rhs.x;
        result.y = y - rhs.y;
        result.z = z - rhs.z;
        return result;
    }
    
    inline v3 operator-() {
        v3 result;
        result.x = -x;
        result.y = -y;
        result.z = -z;
        return result;
    }
};



inline v3 V3(float x, float y, float z) {
    v3 v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
};

inline float length(v3 v) {
    return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

inline v3 normalize(v3 v) {
    float l = length(v);
    if(l != 0) {
        v.x = v.x / l;
        v.y = v.y / l;
        v.z = v.z / l;
    }
    return v;
}








bool PointInPolygon(v2 Point, v2* Vertices, int VertexCount) {
    int i, j = VertexCount - 1;
    bool Odd = false;
    
    for(i = 0; i < VertexCount; i++){
        // TODO(JOSH): check for and skip points  that are colinear with a side?
        //check for horizontal cast intersection
        if(Vertices[i].y < Point.y && Vertices[j].y >= Point.y
           || Vertices[j].y < Point.y && Vertices[i].y >= Point.y) {
            // calculate intersection
            if(Vertices[i].x + (Point.y - Vertices[i].y) / (Vertices[j].y - Vertices[i].y) * (Vertices[j].x - Vertices[i].x) < Point.x) {
                Odd = !Odd;
            }
        }
        j = i;
    }
    
    return Odd;
    
}









#endif