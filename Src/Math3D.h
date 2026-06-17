#ifndef MATH3D_H
#define MATH3D_H

#include <Arduino.h>
#include <math.h>

// ---------------------------------------------------------
// 3D Math Types
// ---------------------------------------------------------
struct Vec3 {
    float x, y, z;
};

struct Vec2 {
    int16_t x, y;
};

// ---------------------------------------------------------
// Fast Math Utilities
// ---------------------------------------------------------

// 256-entry Sine Lookup Table (Quarter wave: 0 to 90 degrees)
// Values scaled by 32767 (Q15 format)
static const int16_t sin_lut[257] PROGMEM = {
    0, 201, 402, 603, 804, 1005, 1206, 1406, 1607, 1808, 2009, 2209, 2410, 2610, 2811, 3011, 
    3211, 3411, 3611, 3811, 4011, 4210, 4409, 4608, 4807, 5006, 5205, 5403, 5601, 5799, 5997, 6195, 
    6392, 6589, 6786, 6982, 7179, 7375, 7571, 7766, 7961, 8156, 8351, 8545, 8739, 8932, 9126, 9319, 
    9511, 9703, 9895, 10086, 10278, 10469, 10659, 10849, 11038, 11227, 11416, 11604, 11792, 11980, 12166, 12353, 
    12539, 12724, 12909, 13094, 13278, 13462, 13645, 13827, 14009, 14191, 14372, 14552, 14732, 14911, 15090, 15268, 
    15446, 15623, 15799, 15975, 16150, 16325, 16499, 16672, 16845, 17017, 17189, 17360, 17530, 17699, 17868, 18036, 
    18204, 18371, 18537, 18702, 18867, 19031, 19194, 19357, 19519, 19680, 19840, 20000, 20159, 20317, 20474, 20631, 
    20787, 20942, 21096, 21249, 21402, 21554, 21705, 21855, 22004, 22153, 22301, 22448, 22594, 22739, 22883, 23027, 
    23169, 23311, 23452, 23592, 23731, 23869, 24006, 24143, 24278, 24413, 24546, 24679, 24811, 24942, 25072, 25201, 
    25329, 25456, 25582, 25707, 25831, 25954, 26077, 26198, 26318, 26437, 26556, 26673, 26789, 26905, 27019, 27132, 
    27244, 27355, 27465, 27574, 27682, 27789, 27895, 28000, 28104, 28207, 28309, 28410, 28510, 28608, 28706, 28802, 
    28897, 28992, 29085, 29177, 29268, 29358, 29446, 29534, 29620, 29705, 29789, 29872, 29953, 30033, 30112, 30190, 
    30267, 30342, 30416, 30489, 30561, 30632, 30701, 30769, 30836, 30902, 30966, 31029, 31091, 31152, 31211, 31269, 
    31326, 31382, 31436, 31489, 31541, 31591, 31640, 31688, 31735, 31780, 31824, 31866, 31908, 31948, 31986, 32023, 
    32059, 32093, 32126, 32158, 32188, 32217, 32244, 32270, 32295, 32318, 32340, 32360, 32379, 32396, 32412, 32427, 
    32440, 32452, 32462, 32471, 32478, 32484, 32489, 32492, 32494, 32494, 32493, 32490, 32486, 32480, 32472, 32463, 
    32453
};

// Fast sine using LUT with quadrant mirroring
inline float fast_sinf(float angle) {
    // Normalize angle to 0-1024 range (1024 = 360 degrees)
    int idx = (int)(angle * (1024.0f / (2.0f * M_PI))) & 0x3FF;
    
    int quadrant = idx >> 8;  // 0-3
    int index = idx & 0xFF;   // 0-255
    
    int16_t val;
    switch (quadrant) {
        case 0: val = pgm_read_word(&sin_lut[index]); break;
        case 1: val = pgm_read_word(&sin_lut[256 - index]); break;
        case 2: val = -pgm_read_word(&sin_lut[index]); break;
        case 3: val = -pgm_read_word(&sin_lut[256 - index]); break;
        default: val = 0; break;
    }
    return val * (1.0f / 32767.0f);
}

// Fast cosine is phase-shifted sine
inline float fast_cosf(float angle) {
    return fast_sinf(angle + (M_PI * 0.5f));
}

// ---------------------------------------------------------
// 3D Transformations
// ---------------------------------------------------------

// Rotate a point around the Y axis (Yaw)
inline Vec3 rotateY(Vec3 p, float sin_a, float cos_a) {
    return {
        p.x * cos_a + p.z * sin_a,
        p.y,
        -p.x * sin_a + p.z * cos_a
    };
}

// Rotate a point around the X axis (Pitch)
inline Vec3 rotateX(Vec3 p, float sin_a, float cos_a) {
    return {
        p.x,
        p.y * cos_a - p.z * sin_a,
        p.y * sin_a + p.z * cos_a
    };
}

// Simple perspective projection
#define FOCAL_LENGTH 256.0f
#define SCREEN_CX    160  // center X (320/2)
#define SCREEN_CY    100  // center Y (200/2)

inline Vec2 project(Vec3 p) {
    Vec2 result;
    // Avoid division by zero
    float z = p.z;
    if (z < 1.0f) z = 1.0f; 
    
    float inv_z = FOCAL_LENGTH / z;  
    result.x = (int16_t)(p.x * inv_z) + SCREEN_CX;
    result.y = (int16_t)(p.y * inv_z) + 60; // Horizon Y
    return result;
}

#endif // MATH3D_H
