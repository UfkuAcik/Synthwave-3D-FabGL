#include "Src/fabgl_lite/fabgl_lite.h"

#include "Src/Math3D.h"
#include "Src/Grid.h"
#include "Src/Models.h"
#include "Src/GamepadConfig.h"
#include "Src/Scenery.h"
#include <Bluepad32.h>

// ---------------------------------------------------------
// Global FabGL Controllers
// ---------------------------------------------------------
fabgl::VGAController DisplayController;
fabgl::Canvas        canvas(&DisplayController);

// ---------------------------------------------------------
// Game State
// ---------------------------------------------------------
Grid grid;
SceneryManager scenery;
Car playerCar(car_models[0]); // Default to first car

float carSpeed = 5.0f;
float manualYaw = 0.0f;
float targetX = 0.0f; // Target X position for smooth movement
int currentModelIndex = 0;
int currentThemeIndex = 0;
bool gameStarted = false;

struct Theme {
    fabgl::Color gridColor;
    fabgl::Color carColor;
    fabgl::RGB888 carRGB;
};

Theme palettes[10] = {
    {fabgl::Color::Cyan, fabgl::Color::BrightMagenta, fabgl::RGB888(255, 0, 255)},        // 0
    {fabgl::Color::Green, fabgl::Color::Yellow, fabgl::RGB888(255, 255, 0)},              // 1
    {fabgl::Color::Red, fabgl::Color::BrightCyan, fabgl::RGB888(0, 255, 255)},            // 2
    {fabgl::Color::BrightBlue, fabgl::Color::White, fabgl::RGB888(255, 255, 255)},        // 3
    {fabgl::Color::Magenta, fabgl::Color::BrightGreen, fabgl::RGB888(0, 255, 0)},         // 4
    {fabgl::Color::BrightYellow, fabgl::Color::BrightRed, fabgl::RGB888(255, 0, 0)},      // 5
    {fabgl::Color::Blue, fabgl::Color::BrightMagenta, fabgl::RGB888(255, 0, 255)},        // 6
    {fabgl::Color::BrightGreen, fabgl::Color::White, fabgl::RGB888(255, 255, 255)},       // 7
    {fabgl::Color::BrightCyan, fabgl::Color::Yellow, fabgl::RGB888(255, 255, 0)},         // 8
    {fabgl::Color::White, fabgl::Color::BrightBlue, fabgl::RGB888(0, 0, 255)}             // 9
};

void changeTheme(int index) {
    if (index >= 0 && index < 10) {
        currentThemeIndex = index;
        grid.setGridColor(palettes[index].gridColor);
        playerCar.setColor(palettes[index].carColor, palettes[index].carRGB);
    }
}

void changeModel(int index) {
    if (index >= 0 && index < 4) {
        playerCar.setModel(car_models[index]);
        currentModelIndex = index;
    }
}

// ---------------------------------------------------------
// Bluepad32
// ---------------------------------------------------------
ControllerPtr myGamepad = nullptr;

void onConnectedController(ControllerPtr ctl) {
    if (myGamepad == nullptr) {
        Serial.printf("CALLBACK: Controller is connected, index=%d\n", ctl->index());
        myGamepad = ctl;
        
        // Load config from SD using the same format as Gamepad Tester
        ControllerProperties props = ctl->getProperties();
        char macStr[32];
        sprintf(macStr, "%02X%02X", props.btaddr[4], props.btaddr[5]);
        Serial.println(String("MAC suffix: ") + macStr);
        g_gamepadConfig.loadConfig(String(macStr));
    } else {
        Serial.println("CALLBACK: Controller connected, but already have one.");
    }
}

void onDisconnectedController(ControllerPtr ctl) {
    if (myGamepad == ctl) {
        Serial.printf("CALLBACK: Controller disconnected\n");
        myGamepad = nullptr;
    }
}

void processInput() {
    bool dpad_left = false, dpad_right = false;
    bool btn_a = false, btn_y = false, btn_l1 = false, btn_r1 = false, btn_l2 = false, btn_r2 = false;
    bool start_pressed = false;
    int axisRX = 0;

    if (myGamepad && myGamepad->isConnected()) {
        uint64_t currentInput = 0;
        
        // Build 64-bit mask matching Gamepad-Tester logic
        uint16_t btns = myGamepad->buttons();
        currentInput |= btns; // A, B, X, Y, L1, R1, L2, R2 are in bottom 16 bits
        
        uint8_t dpad = myGamepad->dpad();
        if (dpad & 0x01) currentInput |= (1ULL << 16); // DPAD_UP
        if (dpad & 0x02) currentInput |= (1ULL << 17); // DPAD_DOWN
        if (dpad & 0x08) currentInput |= (1ULL << 18); // DPAD_LEFT
        if (dpad & 0x04) currentInput |= (1ULL << 19); // DPAD_RIGHT
        
        uint16_t miscBtns = myGamepad->miscButtons();
        if (miscBtns & 0x04) currentInput |= (1ULL << 13); // START button is often 0x04 in misc or mapped differently.
        // Wait, the tester assigns START to a specific virtual button.
        // We just pass the entire mask to isPressed.

        btn_a = g_gamepadConfig.isPressed("A", currentInput);
        btn_y = g_gamepadConfig.isPressed("Y", currentInput);
        dpad_left = g_gamepadConfig.isPressed("DPAD_LEFT", currentInput);
        dpad_right = g_gamepadConfig.isPressed("DPAD_RIGHT", currentInput);
        btn_l1 = g_gamepadConfig.isPressed("L1", currentInput);
        btn_r1 = g_gamepadConfig.isPressed("R1", currentInput);
        btn_l2 = g_gamepadConfig.isPressed("L2", currentInput);
        btn_r2 = g_gamepadConfig.isPressed("R2", currentInput);
        start_pressed = g_gamepadConfig.isPressed("START", currentInput);
        
        axisRX = myGamepad->axisRX();
    }

    if (!gameStarted) {
        if (start_pressed) {
            gameStarted = true;
        }
        return;
    }

    // Process logic
    static bool prev_l1 = false, prev_r1 = false, prev_l2 = false, prev_r2 = false;

    if (dpad_left) {
        targetX -= 15.0f;
        if (targetX < -150.0f) targetX = -150.0f; // Constrain to central lanes
    }
    if (dpad_right) {
        targetX += 15.0f;
        if (targetX > 150.0f) targetX = 150.0f;
    }
    
    if (btn_a) { // Accelerate
        carSpeed += 0.5f;
        if (carSpeed > 25.0f) carSpeed = 25.0f;
    } else if (btn_y) { // Decelerate
        carSpeed -= 0.5f;
        if (carSpeed < 1.0f) carSpeed = 1.0f;
    } else { // Coast down slowly
        if (carSpeed > 5.0f) carSpeed -= 0.1f;
    }

    // Right Analog for Yaw
    if (axisRX < -100) manualYaw -= 0.05f;
    else if (axisRX > 100) manualYaw += 0.05f;
    else manualYaw *= 0.9f; // auto-center slowly

    // Button down events
    if (btn_l1 && !prev_l1) {
        changeModel((currentModelIndex - 1 + 4) % 4);
    }
    if (btn_r1 && !prev_r1) {
        changeModel((currentModelIndex + 1) % 4);
    }
    if (btn_l2 && !prev_l2) {
        changeTheme((currentThemeIndex - 1 + 10) % 10);
    }
    if (btn_r2 && !prev_r2) {
        changeTheme((currentThemeIndex + 1) % 10);
    }

    prev_l1 = btn_l1; prev_r1 = btn_r1;
    prev_l2 = btn_l2; prev_r2 = false;
}

// Fireworks (Celebration Effect)
struct FireworkParticle {
    float x, y;
    float vx, vy;
    int life;
    fabgl::RGB888 color;
};
#define NUM_FW_PARTICLES 60
FireworkParticle fw[NUM_FW_PARTICLES];

void updateAndDrawFireworks() {
    bool allDead = true;
    for(int i=0; i<NUM_FW_PARTICLES; i++) {
        if (fw[i].life > 0) allDead = false;
    }
    
    if (allDead && random(100) < 10) { // 10% chance to burst when empty
        float ex = random(40, 280);
        float ey = random(10, 40); // high in the sky
        fabgl::RGB888 c = fabgl::RGB888(random(100,255), random(100,255), random(100,255));
        for(int i=0; i<NUM_FW_PARTICLES; i++) {
            fw[i].x = ex;
            fw[i].y = ey;
            float angle = random(0, 360) * 3.14159f / 180.0f;
            float speed = random(5, 30) / 10.0f;
            fw[i].vx = cos(angle) * speed;
            fw[i].vy = sin(angle) * speed;
            fw[i].life = random(20, 40);
            fw[i].color = c;
        }
    }
    
    for(int i=0; i<NUM_FW_PARTICLES; i++) {
        if (fw[i].life > 0) {
            canvas.setPenColor(fw[i].color);
            canvas.setPixel((int)fw[i].x, (int)fw[i].y);
            // Draw a trail dot
            canvas.setPixel((int)(fw[i].x - fw[i].vx), (int)(fw[i].y - fw[i].vy));
            
            fw[i].x += fw[i].vx;
            fw[i].y += fw[i].vy;
            fw[i].vy += 0.05f; // Gravity
            fw[i].life--;
        }
    }
}

void drawBackground() {
    canvas.setBrushColor(fabgl::Color::Black);
    canvas.fillRectangle(0, 0, canvas.getWidth() - 1, canvas.getHeight() - 1);
    
    // Parallax based on car X
    float bg_offset = -playerCar.x * 0.3f;

    // MEGA SUN
    int sun_r = 55;
    int sun_cx = 160 + bg_offset;
    int sun_cy = 56;
    
    // Draw the sun slices (Synthwave style)
    for (int y = -sun_r; y < sun_r; y++) {
        if (y > 0) {
            int slice_height = 2 + (y / 10); 
            int gap_height = 2 + (y / 15);
            int cycle = slice_height + gap_height;
            if (y % cycle > slice_height) continue;
        }
        
        int w = (int)sqrtf(sun_r * sun_r - y * y);
        if (y < -20) {
            canvas.setPenColor(fabgl::Color::BrightYellow);
        } else if (y < 10) {
            canvas.setPenColor(255, 128, 0); // Orange
        } else {
            canvas.setPenColor(fabgl::Color::BrightRed);
        }
        canvas.drawLine(sun_cx - w, sun_cy + y, sun_cx + w, sun_cy + y);
    }
    // Fireworks (Dot particle effect)
    updateAndDrawFireworks();

    // City Apartments
    for (int i=0; i<30; i++) {
        float bx = fmod((i * 65.0f) + bg_offset + 2000.0f, 2000.0f) - 400.0f;
        if (bx > -100 && bx < 400) {
            // City Apartment Building (Detailed)
            float bh = 30.0f + fmod(i * 47.0f, 75.0f);
            float bw = 35.0f + fmod(i * 23.0f, 35.0f);
            
            canvas.setBrushColor(10, 10, 30);
            canvas.fillRectangle(bx, 71 - bh, bx + bw, 71);
            
            canvas.setPenColor(fabgl::Color::Cyan);
            canvas.drawRectangle(bx, 71 - bh, bx + bw, 71);
            
            // Windows
            if (bh > 40.0f) {
                canvas.setPenColor(fabgl::Color::BrightYellow);
                for(float wy = 71 - bh + 5.0f; wy < 66; wy += 10.0f) {
                    canvas.drawLine(bx + 5, wy, bx + bw - 5, wy);
                }
            }
        }
    }
}
void drawHUD(int fps) {
    // Top Bar Background
    canvas.setBrushColor(fabgl::RGB888(0, 0, 0));
    canvas.setPenColor(fabgl::RGB888(0, 255, 255));
    canvas.fillRectangle(0, 0, 319, 12);
    canvas.drawLine(0, 13, 319, 13);
    
    canvas.setPenColor(fabgl::Color::White);
    canvas.setBrushColor(fabgl::Color::Black);
    
    char buf[64];
    sprintf(buf, "SPEED:%02d  THEME:%d  MODEL:%d", (int)carSpeed, currentThemeIndex, currentModelIndex);
    canvas.drawText(2, 2, buf);
    
    sprintf(buf, "%d FPS", fps);
    canvas.setPenColor(fabgl::Color::BrightGreen);
    canvas.drawText(270, 2, buf);
}

void setup() {
    Serial.begin(115200);

    // ALLOCATE VGA BUFFERS FIRST!
    // This prevents Bluepad32 from fragmenting the internal SRAM and causing double buffering allocation to fail.
    // Initialize VGA Display Controller
    // You can change the pins below to match your ESP32 wiring
    DisplayController.begin(
        GPIO_NUM_22, // Red   (Red_1)
        GPIO_NUM_19, // Green (Green_1)
        GPIO_NUM_5,  // Blue  (Blue_1)
        GPIO_NUM_23, // HSync
        GPIO_NUM_15  // VSync
    );
    DisplayController.setResolution(VGA_320x200_75Hz, 320, 120, true);

    // Initialize SD Card for reading Gamepad configurations
    // You can change the SPI pins below to match your ESP32 wiring
    // Arguments: SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN
    g_gamepadConfig.initSD(14, 35, 12, 13);

    // Initialize Bluepad32 AFTER VGA memory is allocated
    BP32.setup(&onConnectedController, &onDisconnectedController);
    BP32.enableVirtualDevice(false);

    playerCar.init();
    scenery.init();

    canvas.selectFont(&fabgl::FONT_8x8);
    canvas.setGlyphOptions(GlyphOptions().FillBackground(true));

    playerCar.setPosition(0, 92, 435); 
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

    // Update Bluepad32
    bool dataUpdated = BP32.update();
    processInput();

    if (!gameStarted) {
        canvas.setBrushColor(fabgl::Color::Black);
        canvas.fillRectangle(0, 0, 319, 199);
        canvas.setPenColor(fabgl::Color::Cyan);
        canvas.setBrushColor(fabgl::Color::Black);
        
        if (myGamepad && myGamepad->isConnected()) {
            canvas.drawText(60, 90, "GAMEPAD CONNECTED");
            if ((millis() / 500) % 2 == 0) {
                canvas.drawText(75, 110, "PRESS START TO PLAY");
            }
        } else {
            canvas.drawText(60, 90, "WAITING FOR GAMEPAD...");
        }
        canvas.swapBuffers();
        return;
    }

    float dx = targetX - playerCar.x;
    playerCar.x += dx * 0.1f;
    
    playerCar.yaw = -dx * 0.005f + manualYaw;

    grid.update(carSpeed);
    scenery.update(carSpeed);

    drawBackground();
    
    grid.draw(&canvas);
    scenery.draw(&canvas);
    playerCar.draw(&canvas);

    drawHUD(currentFPS);
    
    canvas.swapBuffers();
}
