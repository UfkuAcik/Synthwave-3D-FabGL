#include "Car.h"
#include "Models.h"
#include <math.h>
#include <stdlib.h>

// Helper for qsort
static int compareFaceDepth(const void* a, const void* b) {
    const FaceDepth* fa = (const FaceDepth*)a;
    const FaceDepth* fb = (const FaceDepth*)b;
    // We want to sort from furthest (largest Z) to closest (smallest Z)
    if (fa->z_depth < fb->z_depth) return 1;
    if (fa->z_depth > fb->z_depth) return -1;
    return 0;
}

// Buffer sizes for rendering
#define MAX_VERTICES 6500
#define MAX_FACES 3600

// Pointers to buffers (allocated in PSRAM to save internal SRAM for VGA)
static Vec2* projected = nullptr;
static float* vertex_z = nullptr;
static FaceDepth* face_depths = nullptr;

void Car::init() {
    // Allocate rendering buffers in PSRAM to free internal SRAM for VGA double buffers
    if (!projected) {
        if (psramFound()) {
            projected = (Vec2*)ps_malloc(MAX_VERTICES * sizeof(Vec2));
            vertex_z = (float*)ps_malloc(MAX_VERTICES * sizeof(float));
            face_depths = (FaceDepth*)ps_malloc(MAX_FACES * sizeof(FaceDepth));
            Serial.println("Car buffers allocated in PSRAM");
        } else {
            projected = (Vec2*)malloc(MAX_VERTICES * sizeof(Vec2));
            vertex_z = (float*)malloc(MAX_VERTICES * sizeof(float));
            face_depths = (FaceDepth*)malloc(MAX_FACES * sizeof(FaceDepth));
            Serial.println("Car buffers allocated in SRAM (no PSRAM)");
        }
    }
}

void Car::draw(fabgl::Canvas* canvas) {
    if (!model || model->num_vertices > MAX_VERTICES || model->num_faces > MAX_FACES) return;

    // Fast rotation lookup
    float sin_y = fast_sinf(yaw);
    float cos_y = fast_cosf(yaw);

    // Transform all vertices to 3D world space
    for (int i = 0; i < model->num_vertices; i++) {
        Vec3 p;
        float cy = fast_cosf(yaw);
        float sy = fast_sinf(yaw);
        
        // Scale and position car so wheels sit on the road surface.
        // project() maps p.y=0 to horizon, p.y=CAMERA_H(60) to ground.
        // Model wheels are at Y≈-14. Adding 14 puts wheels at 0, then +60 puts them on ground.
        p.x = model->vertices[i][0] * 3.0f;
        p.y = (model->vertices[i][1] + 14.0f) * 3.0f + 60.0f;
        p.z = model->vertices[i][2] * 3.0f;
        
        // Fix Model 3 (Gallardo) facing backwards by rotating 180 degrees (negate X and Z)
        if (model == &model_gallardo) {
            p.x = -p.x;
            p.z = -p.z;
        }
        
        // Rotate (Yaw)
        float rx = p.x * cy + p.z * sy;
        float rz = -p.x * sy + p.z * cy;
        p.x = rx;
        p.z = rz;

        // Translate to world position
        p.x += x;
        // p.y is already absolute relative to the camera
        p.z += z;

        vertex_z[i] = p.z;

        // Project
        projected[i] = project(p);
    }

    int visible_faces = 0;

    for (int i = 0; i < model->num_faces; i++) {
        const Face& f = model->faces[i];
        
        // Calculate Face Depth
        float cz = (vertex_z[f.v1] + vertex_z[f.v2] + vertex_z[f.v3]) * 0.333333f;

        // Painter's algorithm will handle it
        if (true) {
            float avg_z = cz;
            
            if (avg_z > -1000.0f) {
                face_depths[visible_faces].index = i;
                face_depths[visible_faces].z_depth = avg_z;
                visible_faces++;
            }
        }
    }

    // Sort visible faces by Z-depth (Back to Front)
    qsort(face_depths, visible_faces, sizeof(FaceDepth), compareFaceDepth);

    // Draw faces
    for (int i = 0; i < visible_faces; i++) {
        int idx = face_depths[i].index;
        const Face& f = model->faces[idx];
        
        int x1 = projected[f.v1].x; int y1 = projected[f.v1].y;
        int x2 = projected[f.v2].x; int y2 = projected[f.v2].y;
        int x3 = projected[f.v3].x; int y3 = projected[f.v3].y;

        float nx = f.nx * cos_y + f.nz * sin_y;
        float ny = f.ny;
        float nz = -f.nx * sin_y + f.nz * cos_y;

        // Lighting: Directional light from top-front-left
        Vec3 lightDir = {-0.5f, -0.8f, 0.5f}; // Light source vector
        float dotProduct = nx * lightDir.x + ny * lightDir.y + nz * lightDir.z;
        float intensity = fabsf(dotProduct); 
        
        if (intensity < 0.45f) intensity = 0.45f; 
        if (intensity > 1.0f) intensity = 1.0f;
        
        // Define materials
        if (f.material == 0) { // Body
            int r = rgbColor.R * intensity;
            int g = rgbColor.G * intensity;
            int b = rgbColor.B * intensity;
            canvas->setBrushColor(r, g, b);
            canvas->setPenColor(r, g, b);
        } else if (f.material == 1) { // Windows
            canvas->setBrushColor(0, 50, 200);
            canvas->setPenColor(0, 50, 200);
        } else if (f.material == 2) { // Wheels
            int c = 40 * intensity;
            canvas->setBrushColor(c, c, c);
            canvas->setPenColor(c, c, c);
        } else if (f.material == 3) { // Headlights
            canvas->setBrushColor(255, 255, 255);
            canvas->setPenColor(255, 255, 255);
        } else if (f.material == 4) { // Taillights
            canvas->setBrushColor(255, 0, 0);
            canvas->setPenColor(255, 0, 0);
        }

        // Draw solid triangle using fillPath
        fabgl::Point pts[3];
        pts[0].X = x1; pts[0].Y = y1;
        pts[1].X = x2; pts[1].Y = y2;
        pts[2].X = x3; pts[2].Y = y3;

        // Fill the polygon (no wireframes)
        canvas->fillPath(pts, 3);
    }
}
