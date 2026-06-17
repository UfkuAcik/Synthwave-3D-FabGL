#ifndef GRID_H
#define GRID_H

#include "fabgl_lite/fabgl_lite.h"

#include "Math3D.h"

class Grid {
public:
    Grid();
    void update(float speed);
    void draw(fabgl::Canvas* canvas);

    void setGridColor(fabgl::Color color);
    
    // Grid config
    static constexpr int NUM_HORIZ_LINES = 16;
    static constexpr int NUM_VERT_LINES = 7; // Half count (Total = 2*NUM_VERT_LINES + 1)
    static constexpr float GRID_SPACING = 40.0f;
    static constexpr float GRID_DEPTH = NUM_HORIZ_LINES * GRID_SPACING;
    
    static constexpr float CAMERA_H = 60.0f; // Height above grid
    static constexpr int HORIZON_Y = 60;     // Horizon line on screen
    static constexpr float GRID_HALF_WIDTH = 400.0f; // Width of horizontal lines
private:
    float scroll_offset;
    fabgl::Color gridColor;
};

#endif // GRID_H
