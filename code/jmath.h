
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


//v3 vectors

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

#endif