#include "Car.h"
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

// Static buffers to prevent heap fragmentation on ESP32
#define MAX_VERTICES 3500
#define MAX_FACES 3600
static Vec2 projected[MAX_VERTICES];
static float vertex_z[MAX_VERTICES];
static FaceDepth face_depths[MAX_FACES];

void Car::init() {
    // Static buffers, no malloc needed
}

void Car::draw(fabgl::Canvas* canvas) {
    if (!model || model->num_vertices > MAX_VERTICES || model->num_faces > MAX_FACES) return;

    // Fast rotation lookup
    float sin_y = fast_sinf(yaw);
    float cos_y = fast_cosf(yaw);

    // Transform all vertices to 3D world space (for depth and normal calc)
    // We need 3D points for lighting and Z-sorting. 
    for (int i = 0; i < model->num_vertices; i++) {
        Vec3 p;
        p.x = model->vertices[i][0];
        p.y = model->vertices[i][1];
        p.z = model->vertices[i][2];

        // Rotation
        p = rotateY(p, sin_y, cos_y);

        // Scale by 3.0f (25% smaller than 4.0f)
        p.x *= 3.0f;
        p.y *= 3.0f;
        p.z *= 3.0f;

        // Translate
        p.x += x;
        p.y += y;
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

        // Painter's algorithm will handle it anyway!
        if (true) {
            float avg_z = cz;
            
            // Only add faces that are in front of the camera
            // Camera is at z=0, looking towards +z. Focal length handles projection.
            // Anything > -1000 is safely visible or in the scene
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

        // We already rotated the normal in the first loop, but to save memory we didn't store it.
        // Re-rotate it here for lighting. (Cheap)
        float nx = f.nx * cos_y + f.nz * sin_y;
        float ny = f.ny;
        float nz = -f.nx * sin_y + f.nz * cos_y;

        // Lighting: Directional light from top-front-left
        Vec3 lightDir = {-0.5f, -0.8f, 0.5f}; // Light source vector
        float dotProduct = nx * lightDir.x + ny * lightDir.y + nz * lightDir.z;
        float intensity = fabsf(dotProduct); // abs() to ignore winding order since we don't do backface culling
        
        if (intensity < 0.45f) intensity = 0.45f; // Ambient light (0.45 * 255 = 114)
        if (intensity > 1.0f) intensity = 1.0f;
        
        // Define materials
        if (f.material == 0) { // Body
            int r = rgbColor.R * intensity;
            int g = rgbColor.G * intensity;
            int b = rgbColor.B * intensity;
            canvas->setBrushColor(r, g, b);
            canvas->setPenColor(r, g, b);
        } else if (f.material == 1) { // Windows
            // Fixed bright blue for synthwave aesthetic, unaffected by lighting
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

        // Fill the polygon
        canvas->fillPath(pts, 3);
        
        // Draw Synthwave Wireframes (Panel Gaps) to highlight doors and structure
        canvas->setPenColor(fabgl::Color::Black);
        canvas->drawLine(x1, y1, x2, y2);
        canvas->drawLine(x2, y2, x3, y3);
        canvas->drawLine(x3, y3, x1, y1);
    }
}
