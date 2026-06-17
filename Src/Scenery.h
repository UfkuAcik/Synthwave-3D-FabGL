#pragma once
#include "fabgl_lite/fabgl_lite.h"

#include "Math3D.h"

// Types of scenery objects
enum PropType {
    PROP_TREE,
    PROP_CACTUS,
    PROP_SIGN,
    PROP_OVERPASS,
    PROP_LAMP,
    PROP_ARCH,
    PROP_CUBE,
    PROP_CONE,
    PROP_RADAR,
    PROP_CRYSTAL
};

struct SceneryProp {
    float x, y, z;
    PropType type;
    fabgl::Color color;
};

#define MAX_PROPS 25

class SceneryManager {
public:
    SceneryManager();
    void init();
    void update(float speed);
    void draw(fabgl::Canvas* canvas);

private:
    SceneryProp props[MAX_PROPS];
    void spawnProp(int index, float zPos);
    void drawLine3D(fabgl::Canvas* canvas, float x1, float y1, float z1, float x2, float y2, float z2);
    void fillTriangle3D(fabgl::Canvas* canvas, float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3);
    void fillQuad3D(fabgl::Canvas* canvas, float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4);
    void drawTree(fabgl::Canvas* canvas, const SceneryProp& prop, float inv_z, int sx, int sy);
    void drawCactus(fabgl::Canvas* canvas, const SceneryProp& prop, float inv_z, int sx, int sy);
    void drawSign(fabgl::Canvas* canvas, const SceneryProp& prop, float inv_z, int sx, int sy);
    void drawOverpass(fabgl::Canvas* canvas, const SceneryProp& prop, float inv_z, int sx, int sy);
    void drawLamp(fabgl::Canvas* canvas, const SceneryProp& prop, float inv_z, int sx, int sy);
    void drawArch(fabgl::Canvas* canvas, const SceneryProp& prop, float inv_z, int sx, int sy);
    void drawCube(fabgl::Canvas* canvas, const SceneryProp& prop, float inv_z, int sx, int sy);
    void drawCone(fabgl::Canvas* canvas, const SceneryProp& prop, float inv_z, int sx, int sy);
    void drawRadar(fabgl::Canvas* canvas, const SceneryProp& prop, float inv_z, int sx, int sy);
    void drawCrystal(fabgl::Canvas* canvas, const SceneryProp& prop, float inv_z, int sx, int sy);
};
