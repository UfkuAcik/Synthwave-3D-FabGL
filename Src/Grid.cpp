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
    int screenH = canvas->getHeight();
    
    // Draw solid floor (Asphalt / Desert)
    canvas->setBrushColor(20, 20, 20); // Dark Gray
    // Only clear up to Y=71 because background (buildings/sun) extends down to 71
    canvas->fillRectangle(0, 71, canvas->getWidth() - 1, screenH - 1);
    
    canvas->setPenColor(gridColor);
    
    // First pass: draw the regular grid lines that are on screen
    int lowestGridLineY = HORIZON_Y; // Track the lowest (highest Y) visible grid line
    for (int i = 1; i <= NUM_HORIZ_LINES; i++) {
        float z = (float)i * GRID_SPACING - scroll_offset;
        
        if (z < 1.0f) continue;
        
        float inv_z = FOCAL_LENGTH / z;
        int sy = (int)(CAMERA_H * inv_z) + HORIZON_Y;
        
        if (sy >= screenH) continue; // Below screen - skip
        if (sy <= HORIZON_Y) continue; // Above horizon - skip
        
        int sx_left  = (int)(-GRID_HALF_WIDTH * inv_z) + SCREEN_CX;
        int sx_right = (int)( GRID_HALF_WIDTH * inv_z) + SCREEN_CX;
        
        canvas->drawLine(sx_left, sy, sx_right, sy);
        
        if (sy > lowestGridLineY) lowestGridLineY = sy;
    }
    
    // Second pass: fill the bottom gap
    if (lowestGridLineY < screenH - 1) {
        float z_bottom = CAMERA_H * FOCAL_LENGTH / (float)(screenH - 1 - HORIZON_Y);
        float inv_z_bottom = FOCAL_LENGTH / z_bottom;
        int sx_left  = (int)(-GRID_HALF_WIDTH * inv_z_bottom) + SCREEN_CX;
        int sx_right = (int)( GRID_HALF_WIDTH * inv_z_bottom) + SCREEN_CX;
        canvas->setPenColor(gridColor);
        canvas->drawLine(sx_left, screenH - 1, sx_right, screenH - 1);
    }

    // Draw vertical grid lines (running toward horizon)
    for (int i = -NUM_VERT_LINES; i <= NUM_VERT_LINES; i++) {
        float x = (float)i * GRID_SPACING;
        
        // Near point - use screen bottom instead of z_near=1
        float z_bottom = CAMERA_H * FOCAL_LENGTH / (float)(screenH - 1 - HORIZON_Y);
        float inv_z_bottom = FOCAL_LENGTH / z_bottom;
        int x0 = (int)(x * inv_z_bottom) + SCREEN_CX;
        int y0 = screenH - 1;
        
        // Far point
        float z_far = GRID_DEPTH;
        float inv_z_far = FOCAL_LENGTH / z_far;
        int x1 = (int)(x * inv_z_far) + SCREEN_CX;
        int y1 = (int)(CAMERA_H * inv_z_far) + HORIZON_Y;

        // Visual Enhancement: Central lanes = Highway, Outer lanes = Neon Terrain
        if (i >= -3 && i <= 3) {
            // Central highway area
            if (i == -3 || i == 3) {
                // Highway border lines (solid white)
                canvas->setPenColor(fabgl::Color::White);
                canvas->drawLine(x0, y0, x1, y1);
            } else {
                // Highway inner dashed lines
                canvas->setPenColor(fabgl::Color::BrightBlack); // Greyish
                // Fake dashed line by skipping some parts. 
                // A true dashed line in perspective requires stepping z, but for now we draw solid dark gray
                canvas->drawLine(x0, y0, x1, y1);
            }
        } else {
            // Neon side terrain
            canvas->setPenColor(gridColor);
            canvas->drawLine(x0, y0, x1, y1);
        }
    }
}
