#include "Scenery.h"
#include <stdlib.h>
#include "Grid.h" // For FOCAL_LENGTH, CAMERA_H, etc.

SceneryManager::SceneryManager() {}

void SceneryManager::init() {
    for (int i = 0; i < MAX_PROPS; i++) {
        spawnProp(i, (float)i * 60.0f + 100.0f);
    }
}

void SceneryManager::spawnProp(int index, float zPos) {
    props[index].z = zPos;
    
    int side = (rand() % 2 == 0) ? -1 : 1;
    
    // Type selection
    int t = rand() % 100;
    if (t < 5) {
        props[index].type = PROP_OVERPASS;
        props[index].x = 0; // centered
    } else if (t < 10) {
        props[index].type = PROP_ARCH;
        props[index].x = 0; // centered
    } else {
        props[index].x = side * (160.0f + (rand() % 100)); // Off the highway
        int p = rand() % 8;
        if (p == 0) props[index].type = PROP_TREE;
        else if (p == 1) props[index].type = PROP_CACTUS;
        else if (p == 2) props[index].type = PROP_SIGN;
        else if (p == 3) props[index].type = PROP_LAMP;
        else if (p == 4) props[index].type = PROP_CUBE;
        else if (p == 5) props[index].type = PROP_CONE;
        else if (p == 6) props[index].type = PROP_RADAR;
        else props[index].type = PROP_CRYSTAL;
    }
    
    props[index].y = 0; // Ground level
    
    fabgl::Color colors[] = {fabgl::Color::BrightGreen, fabgl::Color::Cyan, fabgl::Color::BrightMagenta, fabgl::Color::Yellow};
    props[index].color = colors[rand() % 4];
}

void SceneryManager::update(float speed) {
    for (int i = 0; i < MAX_PROPS; i++) {
        props[i].z -= speed;
        if (props[i].z < 10.0f) {
            spawnProp(i, 800.0f + (rand() % 100)); // respawn far away
        }
    }
}

// Compare function for z-sorting
static int comparePropZ(const void* a, const void* b) {
    const SceneryProp* pa = (const SceneryProp*)a;
    const SceneryProp* pb = (const SceneryProp*)b;
    if (pa->z < pb->z) return 1;
    if (pa->z > pb->z) return -1;
    return 0;
}

void SceneryManager::draw(fabgl::Canvas* canvas) {
    // We should sort props by Z depth to draw back-to-front
    SceneryProp sortedProps[MAX_PROPS];
    for (int i = 0; i < MAX_PROPS; i++) sortedProps[i] = props[i];
    qsort(sortedProps, MAX_PROPS, sizeof(SceneryProp), comparePropZ);
    
    for (int i = 0; i < MAX_PROPS; i++) {
        const SceneryProp& p = sortedProps[i];
        if (p.z < 10.0f) continue;
        
        float inv_z = FOCAL_LENGTH / p.z;
        int sy = (int)(Grid::CAMERA_H * inv_z) + Grid::HORIZON_Y;
        int sx = (int)(p.x * inv_z) + SCREEN_CX;
        
        switch (p.type) {
            case PROP_TREE: drawTree(canvas, p, inv_z, sx, sy); break;
            case PROP_CACTUS: drawCactus(canvas, p, inv_z, sx, sy); break;
            case PROP_SIGN: drawSign(canvas, p, inv_z, sx, sy); break;
            case PROP_OVERPASS: drawOverpass(canvas, p, inv_z, sx, sy); break;
            case PROP_LAMP: drawLamp(canvas, p, inv_z, sx, sy); break;
            case PROP_ARCH: drawArch(canvas, p, inv_z, sx, sy); break;
            case PROP_CUBE: drawCube(canvas, p, inv_z, sx, sy); break;
            case PROP_CONE: drawCone(canvas, p, inv_z, sx, sy); break;
            case PROP_RADAR: drawRadar(canvas, p, inv_z, sx, sy); break;
            case PROP_CRYSTAL: drawCrystal(canvas, p, inv_z, sx, sy); break;
        }
    }
}

void SceneryManager::drawLine3D(fabgl::Canvas* canvas, float x1, float y1, float z1, float x2, float y2, float z2) {
    if (z1 < 5.0f && z2 < 5.0f) return;
    
    // Near clipping approximation
    if (z1 < 5.0f) {
        float t = (5.0f - z1) / (z2 - z1);
        x1 = x1 + t * (x2 - x1);
        y1 = y1 + t * (y2 - y1);
        z1 = 5.0f;
    }
    if (z2 < 5.0f) {
        float t = (5.0f - z2) / (z1 - z2);
        x2 = x2 + t * (x1 - x2);
        y2 = y2 + t * (y1 - y2);
        z2 = 5.0f;
    }

    float iz1 = FOCAL_LENGTH / z1;
    float iz2 = FOCAL_LENGTH / z2;
    int sx1 = (int)(x1 * iz1) + SCREEN_CX;
    int sy1 = (int)((Grid::CAMERA_H - y1) * iz1) + Grid::HORIZON_Y;
    int sx2 = (int)(x2 * iz2) + SCREEN_CX;
    int sy2 = (int)((Grid::CAMERA_H - y2) * iz2) + Grid::HORIZON_Y;
    canvas->drawLine(sx1, sy1, sx2, sy2);
}

void SceneryManager::fillTriangle3D(fabgl::Canvas* canvas, float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3) {
    if (z1 < 5.0f || z2 < 5.0f || z3 < 5.0f) return;
    fabgl::Point pts[3];
    float iz1 = FOCAL_LENGTH / z1;
    float iz2 = FOCAL_LENGTH / z2;
    float iz3 = FOCAL_LENGTH / z3;
    pts[0] = fabgl::Point((int)(x1 * iz1) + SCREEN_CX, (int)((Grid::CAMERA_H - y1) * iz1) + Grid::HORIZON_Y);
    pts[1] = fabgl::Point((int)(x2 * iz2) + SCREEN_CX, (int)((Grid::CAMERA_H - y2) * iz2) + Grid::HORIZON_Y);
    pts[2] = fabgl::Point((int)(x3 * iz3) + SCREEN_CX, (int)((Grid::CAMERA_H - y3) * iz3) + Grid::HORIZON_Y);
    canvas->fillPath(pts, 3);
}

void SceneryManager::fillQuad3D(fabgl::Canvas* canvas, float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4) {
    if (z1 < 5.0f || z2 < 5.0f || z3 < 5.0f || z4 < 5.0f) return;
    fabgl::Point pts[4];
    float iz1 = FOCAL_LENGTH / z1;
    float iz2 = FOCAL_LENGTH / z2;
    float iz3 = FOCAL_LENGTH / z3;
    float iz4 = FOCAL_LENGTH / z4;
    pts[0] = fabgl::Point((int)(x1 * iz1) + SCREEN_CX, (int)((Grid::CAMERA_H - y1) * iz1) + Grid::HORIZON_Y);
    pts[1] = fabgl::Point((int)(x2 * iz2) + SCREEN_CX, (int)((Grid::CAMERA_H - y2) * iz2) + Grid::HORIZON_Y);
    pts[2] = fabgl::Point((int)(x3 * iz3) + SCREEN_CX, (int)((Grid::CAMERA_H - y3) * iz3) + Grid::HORIZON_Y);
    pts[3] = fabgl::Point((int)(x4 * iz4) + SCREEN_CX, (int)((Grid::CAMERA_H - y4) * iz4) + Grid::HORIZON_Y);
    canvas->fillPath(pts, 4);
}

void SceneryManager::drawTree(fabgl::Canvas* canvas, const SceneryProp& prop, float inv_z, int sx, int sy) {
    float px = prop.x;
    float pz = prop.z;
    float h = 180.0f;
    
    // Trunk
    canvas->setBrushColor(fabgl::RGB888(139, 69, 19)); // SaddleBrown
    canvas->setPenColor(fabgl::RGB888(139, 69, 19)); 
    fillQuad3D(canvas, px-5, 0, pz, px+5, 0, pz, px+2, h, pz, px-2, h, pz);
    
    // Fronds
    canvas->setBrushColor(fabgl::RGB888(34, 139, 34)); // ForestGreen
    canvas->setPenColor(fabgl::RGB888(34, 139, 34)); 
    for (int i=0; i<8; i++) {
        float angle = i * 6.283f / 8.0f;
        float fx = cos(angle) * 50.0f;
        float fz = sin(angle) * 50.0f;
        fillTriangle3D(canvas, px, h, pz, px + fx*0.5f, h+20.0f, pz + fz*0.5f, px + fx, h-20.0f, pz + fz);
    }
}

void SceneryManager::drawCactus(fabgl::Canvas* canvas, const SceneryProp& prop, float inv_z, int sx, int sy) {
    float px = prop.x;
    float pz = prop.z;
    canvas->setBrushColor(fabgl::RGB888(0, 100, 0)); // Dark Green
    canvas->setPenColor(fabgl::RGB888(0, 100, 0));
    
    // Main Trunk
    fillQuad3D(canvas, px-8, 0, pz, px+8, 0, pz, px+8, 110, pz, px-8, 110, pz);
    
    // Left arm
    fillQuad3D(canvas, px-25, 40, pz, px-8, 40, pz, px-8, 50, pz, px-25, 50, pz);
    fillQuad3D(canvas, px-25, 40, pz, px-15, 40, pz, px-15, 80, pz, px-25, 80, pz);
    
    // Right arm
    fillQuad3D(canvas, px+8, 50, pz, px+25, 50, pz, px+25, 60, pz, px+8, 60, pz);
    fillQuad3D(canvas, px+15, 50, pz, px+25, 50, pz, px+25, 90, pz, px+15, 90, pz);
}

void SceneryManager::drawSign(fabgl::Canvas* canvas, const SceneryProp& prop, float inv_z, int sx, int sy) {
    float px = prop.x;
    float pz = prop.z;
    
    // Pole
    canvas->setBrushColor(fabgl::Color::White);
    canvas->setPenColor(fabgl::Color::White);
    fillQuad3D(canvas, px-4, 0, pz, px+4, 0, pz, px+4, 90, pz, px-4, 90, pz);
    
    float sw = 45.0f;
    float sh = 35.0f;
    float d = 10.0f;
    
    // Billboard Background
    canvas->setBrushColor(fabgl::Color::Black);
    canvas->setPenColor(fabgl::Color::Black);
    fillQuad3D(canvas, px-sw, 90, pz-d, px+sw, 90, pz-d, px+sw, 90+sh, pz-d, px-sw, 90+sh, pz-d);
    
    // Neon details (drawn as lines on top)
    canvas->setPenColor(prop.color);
    for(int i=(int)(-sw+10); i<(int)sw; i+=10) {
        drawLine3D(canvas, px + i, 90, pz - d, px + i, 90 + sh, pz - d);
    }
    for(int i=90+5; i<90+(int)sh; i+=5) {
        drawLine3D(canvas, px - sw, i, pz - d, px + sw, i, pz - d);
    }
}

void SceneryManager::drawOverpass(fabgl::Canvas* canvas, const SceneryProp& prop, float inv_z, int sx, int sy) {
    float pz = prop.z;
    float w = 220.0f;
    float h = 140.0f;
    float d = 60.0f;
    
    canvas->setBrushColor(fabgl::Color::White);
    canvas->setPenColor(fabgl::Color::White);
    // Left Arch
    fillQuad3D(canvas, -w-30, 0, pz, -w, 0, pz, -w, h, pz, -w-30, h, pz);
    // Right Arch
    fillQuad3D(canvas, w, 0, pz, w+30, 0, pz, w+30, h, pz, w, h, pz);
    
    // Top Bridge Span
    fillQuad3D(canvas, -w-30, h, pz, w+30, h, pz, w+30, h+30, pz, -w-30, h+30, pz);
    
    // Neon Signs on Bridge
    canvas->setBrushColor(fabgl::Color::Black);
    canvas->setPenColor(fabgl::Color::Black);
    fillQuad3D(canvas, -w, h+5, pz-1, w, h+5, pz-1, w, h+25, pz-1, -w, h+25, pz-1);
    
    canvas->setBrushColor(255, 255, 0); // YELLOW Chevrons
    for (float cvx = -150; cvx < -30; cvx += 40) {
        fillTriangle3D(canvas, cvx, h+10, pz-2, cvx+20, h+15, pz-2, cvx, h+20, pz-2);
    }
    for (float cvx = 150; cvx > 30; cvx -= 40) {
        fillTriangle3D(canvas, cvx, h+10, pz-2, cvx-20, h+15, pz-2, cvx, h+20, pz-2);
    }
}

void SceneryManager::drawLamp(fabgl::Canvas* canvas, const SceneryProp& prop, float inv_z, int sx, int sy) {
    float px = prop.x; float pz = prop.z;
    float h = 150.0f;
    // Pole
    canvas->setBrushColor(fabgl::RGB888(50, 50, 50));
    canvas->setPenColor(fabgl::RGB888(50, 50, 50));
    fillQuad3D(canvas, px-2, 0, pz, px+2, 0, pz, px+2, h, pz, px-2, h, pz);
    // Arm
    int side = (px > 0) ? -1 : 1;
    fillQuad3D(canvas, px, h-2, pz, px, h+2, pz, px + side*50.0f, h+2, pz, px + side*50.0f, h-2, pz);
    // Light
    canvas->setBrushColor(fabgl::RGB888(0, 255, 255)); // Cyan light
    fillQuad3D(canvas, px + side*40.0f, h-5, pz, px + side*50.0f, h-5, pz, px + side*50.0f, h, pz, px + side*40.0f, h, pz);
}

void SceneryManager::drawArch(fabgl::Canvas* canvas, const SceneryProp& prop, float inv_z, int sx, int sy) {
    float pz = prop.z;
    float hw = 220.0f; // Half width
    float h = 180.0f;
    canvas->setPenColor(prop.color);
    drawLine3D(canvas, -hw, 0, pz, -hw, h, pz);
    drawLine3D(canvas, hw, 0, pz, hw, h, pz);
    drawLine3D(canvas, -hw, h, pz, 0, h + 60.0f, pz);
    drawLine3D(canvas, 0, h + 60.0f, pz, hw, h, pz);
}

void SceneryManager::drawCube(fabgl::Canvas* canvas, const SceneryProp& prop, float inv_z, int sx, int sy) {
    float px = prop.x; float pz = prop.z;
    float h = 80.0f; float s = 20.0f;
    canvas->setPenColor(prop.color);
    drawLine3D(canvas, px-s, h-s, pz-s, px+s, h-s, pz-s);
    drawLine3D(canvas, px+s, h-s, pz-s, px+s, h+s, pz-s);
    drawLine3D(canvas, px+s, h+s, pz-s, px-s, h+s, pz-s);
    drawLine3D(canvas, px-s, h+s, pz-s, px-s, h-s, pz-s);
    drawLine3D(canvas, px-s, h-s, pz+s, px+s, h-s, pz+s);
    drawLine3D(canvas, px+s, h-s, pz+s, px+s, h+s, pz+s);
    drawLine3D(canvas, px+s, h+s, pz+s, px-s, h+s, pz+s);
    drawLine3D(canvas, px-s, h+s, pz+s, px-s, h-s, pz+s);
    drawLine3D(canvas, px-s, h-s, pz-s, px-s, h-s, pz+s);
    drawLine3D(canvas, px+s, h-s, pz-s, px+s, h-s, pz+s);
    drawLine3D(canvas, px+s, h+s, pz-s, px+s, h+s, pz+s);
    drawLine3D(canvas, px-s, h+s, pz-s, px-s, h+s, pz+s);
}

void SceneryManager::drawCone(fabgl::Canvas* canvas, const SceneryProp& prop, float inv_z, int sx, int sy) {
    float px = prop.x; float pz = prop.z;
    float h = 30.0f; float w = 10.0f;
    canvas->setBrushColor(fabgl::RGB888(255, 69, 0));
    canvas->setPenColor(fabgl::RGB888(255, 69, 0));
    fillTriangle3D(canvas, px-w, 0, pz, px+w, 0, pz, px, h, pz);
    canvas->setPenColor(fabgl::RGB888(255, 255, 255));
    drawLine3D(canvas, px-w*0.6f, h*0.4f, pz-1, px+w*0.6f, h*0.4f, pz-1);
}

void SceneryManager::drawRadar(fabgl::Canvas* canvas, const SceneryProp& prop, float inv_z, int sx, int sy) {
    float px = prop.x; float pz = prop.z;
    float h = 60.0f;
    canvas->setBrushColor(fabgl::RGB888(80, 80, 80));
    canvas->setPenColor(fabgl::RGB888(80, 80, 80));
    fillQuad3D(canvas, px-5, 0, pz, px+5, 0, pz, px+3, h, pz, px-3, h, pz);
    canvas->setPenColor(prop.color);
    int size = (int)(40.0f * inv_z);
    int cx = (int)(px * inv_z) + SCREEN_CX;
    int cy = (int)((Grid::CAMERA_H - h) * inv_z) + Grid::HORIZON_Y;
    if (size > 0 && cy > 0 && cy < canvas->getHeight()) {
        canvas->drawEllipse(cx, cy, size/2, size);
        canvas->drawLine(cx, cy, cx + (px>0?-size:size), cy - size/2);
    }
}

void SceneryManager::drawCrystal(fabgl::Canvas* canvas, const SceneryProp& prop, float inv_z, int sx, int sy) {
    float px = prop.x; float pz = prop.z;
    float h = 100.0f; float w = 20.0f;
    canvas->setPenColor(prop.color);
    drawLine3D(canvas, px-w, 0, pz, px, h, pz);
    drawLine3D(canvas, px+w, 0, pz, px, h, pz);
    drawLine3D(canvas, px, 0, pz-w, px, h, pz);
    canvas->setBrushColor(prop.color);
    fillTriangle3D(canvas, px-w, 0, pz, px, 0, pz-w, px, h, pz);
}
