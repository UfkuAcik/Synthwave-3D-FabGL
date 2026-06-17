#ifndef CAR_H
#define CAR_H

#include <stdint.h>
#include "Math3D.h"
#include "fabgl_lite/fabgl_lite.h"


// Define a Face (Triangle)
struct Face {
    uint16_t v1, v2, v3;
    uint8_t material; // 0: Body, 1: Window, 2: Wheel, 3: Headlight, 4: Taillight
    float nx, ny, nz;
};

struct Model3D {
    const float (*vertices)[3];
    const Face* faces;
    int num_vertices;
    int num_faces;
};

struct FaceDepth {
    int index;
    float z_depth;
};

class Car {
public:
    Car(const Model3D* initial_model) {
        setModel(initial_model);
        x = 0; y = 0; z = 0;
        color = fabgl::Color::White;
        rgbColor = fabgl::RGB888(255, 255, 255);
    }
    
    void init();
    
    ~Car() {
        // Nothing to free
    }

    void setModel(const Model3D* m) {
        model = m;
    }

    void setPosition(float px, float py, float pz) {
        x = px; y = py; z = pz;
    }

    void setColor(fabgl::Color c, fabgl::RGB888 rgb) {
        color = c;
        rgbColor = rgb;
    }

    void draw(fabgl::Canvas* canvas);

    float x, y, z;
    float yaw;
    const Model3D* model;
    fabgl::Color color;
    fabgl::RGB888 rgbColor;
};

#endif // CAR_H
