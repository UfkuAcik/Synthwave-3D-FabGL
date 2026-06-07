#include "Grid.h"

Grid::Grid() {
    scroll_offset = 0.0f;
    gridColor = fabgl::Color::Cyan; // Default neon cyan
}

void Grid::setGridColor(fabgl::Color color) {
    gridColor = color;
}

void Grid::update(float speed) {
    scroll_offset += speed;
    if (scroll_offset >= GRID_SPACING) {
        scroll_offset -= GRID_SPACING;
    }
}

void Grid::draw(fabgl::Canvas* canvas) {
    canvas->setPenColor(gridColor);

    // Draw horizontal grid lines (moving towards camera)
    for (int i = 1; i <= NUM_HORIZ_LINES; i++) {
        float z = (float)i * GRID_SPACING - scroll_offset;
        
        if (z < 1.0f) continue; // Behind or too close to camera
        
        float inv_z = FOCAL_LENGTH / z;
        int sy = (int)(CAMERA_H * inv_z) + HORIZON_Y;
        
        if (sy >= canvas->getHeight()) continue; // Below screen
        
        int sx_left  = (int)(-GRID_HALF_WIDTH * inv_z) + SCREEN_CX;
        int sx_right = (int)( GRID_HALF_WIDTH * inv_z) + SCREEN_CX;
        
        canvas->drawLine(sx_left, sy, sx_right, sy);
    }

    // Draw vertical grid lines (running toward horizon)
    for (int i = -NUM_VERT_LINES; i <= NUM_VERT_LINES; i++) {
        float x = (float)i * GRID_SPACING;
        
        // Near point
        float z_near = 1.0f;
        float inv_z_near = FOCAL_LENGTH / z_near;
        int x0 = (int)(x * inv_z_near) + SCREEN_CX;
        int y0 = (int)(CAMERA_H * inv_z_near) + HORIZON_Y;
        
        // Far point (Horizon)
        int x1 = SCREEN_CX; 
        int y1 = HORIZON_Y;
        
        // We project the far point properly
        float z_far = GRID_DEPTH;
        float inv_z_far = FOCAL_LENGTH / z_far;
        x1 = (int)(x * inv_z_far) + SCREEN_CX;
        y1 = (int)(CAMERA_H * inv_z_far) + HORIZON_Y;
        
        // Simple clipping (if line goes below screen)
        if (y0 >= canvas->getHeight()) {
           float t = (float)(canvas->getHeight() - 1 - y1) / (y0 - y1);
           x0 = x1 + (x0 - x1) * t;
           y0 = canvas->getHeight() - 1;
        }

        canvas->drawLine(x0, y0, x1, y1);
    }
}
