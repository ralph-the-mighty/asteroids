
#include "math.h"


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