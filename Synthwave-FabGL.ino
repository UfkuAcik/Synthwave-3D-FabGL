#include "fabgl.h"
#include "Math3D.h"
#include "Grid.h"
#include "Models.h"

// ---------------------------------------------------------
// Global FabGL Controllers
// ---------------------------------------------------------
fabgl::VGAController DisplayController;
fabgl::Canvas        canvas(&DisplayController);
fabgl::PS2Controller PS2Controller;
fabgl::Keyboard      Keyboard;

// ---------------------------------------------------------
// Game State
// ---------------------------------------------------------
Grid grid;
Car playerCar(car_models[0]); // Default to first car

float carSpeed = 5.0f;
float manualYaw = 0.0f;
float targetX = 0.0f; // Target X position for smooth movement
int currentModelIndex = 0;

struct Theme {
    fabgl::Color gridColor;
    fabgl::Color carColor;
    fabgl::RGB888 carRGB;
};

Theme palettes[10] = {
    {fabgl::Color::Cyan, fabgl::Color::BrightMagenta, fabgl::RGB888(255, 0, 255)},        // Q
    {fabgl::Color::Green, fabgl::Color::Yellow, fabgl::RGB888(255, 255, 0)},              // W
    {fabgl::Color::Red, fabgl::Color::BrightCyan, fabgl::RGB888(0, 255, 255)},            // E
    {fabgl::Color::BrightBlue, fabgl::Color::White, fabgl::RGB888(255, 255, 255)},          // R
    {fabgl::Color::Magenta, fabgl::Color::BrightGreen, fabgl::RGB888(0, 255, 0)},       // T
    {fabgl::Color::BrightYellow, fabgl::Color::BrightRed, fabgl::RGB888(255, 0, 0)},    // Y
    {fabgl::Color::Blue, fabgl::Color::BrightMagenta, fabgl::RGB888(255, 0, 255)},        // U
    {fabgl::Color::BrightGreen, fabgl::Color::White, fabgl::RGB888(255, 255, 255)},         // I
    {fabgl::Color::BrightCyan, fabgl::Color::Yellow, fabgl::RGB888(255, 255, 0)},         // O
    {fabgl::Color::White, fabgl::Color::BrightBlue, fabgl::RGB888(0, 0, 255)}           // P
};

void changeTheme(int index) {
    if (index >= 0 && index < 10) {
        grid.setGridColor(palettes[index].gridColor);
        playerCar.setColor(palettes[index].carColor, palettes[index].carRGB);
    }
}

void changeModel(int index) {
    if (index >= 0 && index < 10) {
        playerCar.setModel(car_models[index]);
        currentModelIndex = index;
    }
}

void processInput() {
    // Process keyboard events if any
    while (Keyboard.virtualKeyAvailable()) {
        bool down;
        auto vk = Keyboard.getNextVirtualKey(&down);
        if (down) {
            // Colors: Q W E R T Y U I O P
            switch (vk) {
                case fabgl::VK_q: changeTheme(0); break;
                case fabgl::VK_w: changeTheme(1); break;
                case fabgl::VK_e: changeTheme(2); break;
                case fabgl::VK_r: changeTheme(3); break;
                case fabgl::VK_t: changeTheme(4); break;
                case fabgl::VK_y: changeTheme(5); break;
                case fabgl::VK_u: changeTheme(6); break;
                case fabgl::VK_i: changeTheme(7); break;
                case fabgl::VK_o: changeTheme(8); break;
                case fabgl::VK_p: changeTheme(9); break;
                case fabgl::VK_LEFT: 
                    targetX -= 15.0f; 
                    if (targetX < -200.0f) targetX = -200.0f;
                    break;
                case fabgl::VK_RIGHT: 
                    targetX += 15.0f; 
                    if (targetX > 200.0f) targetX = 200.0f;
                    break;
                case fabgl::VK_UP: 
                    carSpeed += 1.0f; 
                    if (carSpeed > 20.0f) carSpeed = 20.0f;
                    break;
                case fabgl::VK_DOWN: 
                    carSpeed -= 1.0f; 
                    if (carSpeed < 1.0f) carSpeed = 1.0f;
                    break;
                case fabgl::VK_n:
                    manualYaw -= 0.2f;
                    break;
                case fabgl::VK_m:
                    manualYaw += 0.2f;
                    break;
                default: break;
            }
            
            if (vk >= fabgl::VK_0 && vk <= fabgl::VK_9) {
                changeModel(vk - fabgl::VK_0);
            }
            else if (vk >= fabgl::VK_KP_0 && vk <= fabgl::VK_KP_9) {
                changeModel(vk - fabgl::VK_KP_0);
            }
        }
    }
}

void drawBackground() {
    canvas.setBrushColor(fabgl::Color::Black);
    canvas.fillRectangle(0, 0, canvas.getWidth() - 1, canvas.getHeight() - 1);
    
    int sun_cx = canvas.getWidth() / 2;
    int sun_cy = 70;
    int sun_r = 40;
    
    canvas.setPenColor(fabgl::Color::Red);
    canvas.setBrushColor(fabgl::Color::BrightRed);
    
    for (int y = -sun_r; y <= 0; y++) {
        int w = (int)sqrtf(sun_r * sun_r - y * y);
        canvas.drawLine(sun_cx - w, sun_cy + y, sun_cx + w, sun_cy + y);
    }
    
    for (int y = 2; y < sun_r; y += (y/5 + 2)) {
        int w = (int)sqrtf(sun_r * sun_r - y * y);
        canvas.drawLine(sun_cx - w, sun_cy + y, sun_cx + w, sun_cy + y);
    }
}

void setup() {
    Serial.begin(115200);

    PS2Controller.begin(PS2Preset::KeyboardPort0_MousePort1);
    Keyboard.begin(true, true, 0);

    DisplayController.begin();
    DisplayController.setResolution(VGA_320x200_75Hz, -1, -1, true);
    DisplayController.moveScreen(0, 8); // Shift VGA active area down to fill bottom gap

    playerCar.init();

    canvas.selectFont(&fabgl::FONT_8x8);
    canvas.setGlyphOptions(GlyphOptions().FillBackground(true));

    playerCar.setPosition(0, 90, 80); 
    changeTheme(0);
    changeModel(0);
}

void loop() {
    static unsigned long lastFrame = 0;
    static int frames = 0;
    static int currentFPS = 0;
    
    frames++;
    if (millis() - lastFrame >= 1000) {
        currentFPS = frames;
        frames = 0;
        lastFrame = millis();
    }

    processInput();

    float dx = targetX - playerCar.x;
    playerCar.x += dx * 0.1f;
    
    playerCar.yaw = -dx * 0.005f + manualYaw;

    grid.update(carSpeed);

    drawBackground();
    
    grid.draw(&canvas);
    playerCar.draw(&canvas);

    canvas.setPenColor(fabgl::Color::White);
    canvas.setBrushColor(fabgl::Color::Black);
    canvas.drawText(0, 0, "[Q-P] Colors  [0-2] Cars  [ARROWS] Move");
    
    canvas.setPenColor(fabgl::Color::BrightGreen);
    canvas.drawText(270, 0, (String(currentFPS) + " FPS").c_str());
    
    canvas.swapBuffers();
}
