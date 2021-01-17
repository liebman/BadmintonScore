/**
 * @file Display.cpp
 * @author Christopher B. Liebman
 * @brief 
 * @version 0.1
 * @date 2019-10-21
 * 
 * Copyright (c) 2020 Christoper B. Liebman
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */

#include "Arduino.h"
#include "Display.h"
#include <Adafruit_GFX.h>
#include <SmartMatrix_GFX.h>
#include <GFXFonts/br2_serif_score_23.h>
#include <Fonts/Lekton_Bold_18.h>
#include <MatrixHardware_ESP32_V0.h>
#include <SmartMatrix.h>
#include "Log.h"

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#define CMD_RENDER          ((const char*)0)   // refresh display
#define CMD_STOP_SCROLL     ((const char*)1)   // stop scrolling
#define CMD_BLINK           ((const char*)2)   // blink interval
#define CMD_SCROLL_GAMEOVER ((const char*)3)   // scroll _message once a minute

#define SCORE_FONT (&br2_serif_score_23)
#define FPS_FONT (font3x5)
#define STARTING_FONT (font5x7)
#define STARTING_FONT_WIDTH (5)
#define STARTING_FONT_HIGHT (7)
#define SCROLL_FONT (gohufont11b)
#define SCROLL_FONT_HIGHT (11)
#define GAME_OVER_FONT (gohufont11b)
#define GAME_OVER_FONT_HIGHT (11)

#define SCORE_COLOR  white

static bool initialized = false;
static const char* TAG = "Display";

#define COLOR_DEPTH 24                  // known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24
static const uint8_t kMatrixWidth = 64;        // known working: 32, 64, 96, 128
static const uint8_t kMatrixHeight = 32;       // known working: 16, 32, 48, 64
static const uint8_t kRefreshDepth = 24;       // known working: 24, 36, 48
static const uint8_t kDmaBufferRows = 2;       // known working: 2-4, use 2 to save memory, more to keep from dropping frames and automatically lowering refresh rate
static const uint8_t kPanelType = SMARTMATRIX_HUB75_32ROW_MOD16SCAN;
static const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);
static const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
static const uint8_t kScrollingLayerOptions = (SM_SCROLLING_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);
SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(scrollingLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);
static CRGB *gfx_buffer;
static void show_callback();
static SmartMatrix_GFX *gfx = new SmartMatrix_GFX(gfx_buffer, kMatrixWidth, kMatrixHeight, show_callback);
// Sadly this callback function must be copied around with this init code
static void show_callback() {
    backgroundLayer.swapBuffers(true);
    gfx_buffer = (CRGB *)backgroundLayer.backBuffer();
    gfx->newLedsPtr(gfx_buffer);
}

static const int defaultBrightness = (100*255)/100;    // full (100%) brightness
static const rgb24 red(255, 0, 0);
static const rgb24 green(0, 255, 0);
static const rgb24 blue(0, 0, 255);
static const rgb24 white(255, 255, 255);
static const rgb24 black(0, 0, 0);
static const rgb24 yellow(255, 255, 0);
static const rgb24 orange(255, 165, 0);

static const rgb24& getTeamColor(const Score::Team team)
{
    if (team == Score::RED)
    {
        return red;
    }
    return blue;
}


Display::Display(App& app) : _app(app)
{
    _queue = xQueueCreate( 4, sizeof( const char* ) );
}

Display::~Display()
{
    vQueueDelete(_queue);
    _queue = 0;
}

void Display::begin(const char* message)
{
    if (!initialized)
    {
        matrix.addLayer(&backgroundLayer);
        matrix.addLayer(&scrollingLayer);
        matrix.begin();
        matrix.setBrightness(defaultBrightness);
        backgroundLayer.setBrightness(defaultBrightness);
        // This sets the neomatrix and LEDMatrix pointers
        show_callback();
        backgroundLayer.fillScreen( black );
        backgroundLayer.swapBuffers();
        gfx->begin();
        gfx->setTextWrap(false);
        initialized = true;
    }
    splash(message);
    while (isScrolling())
    {
        delay(10);
        uint8_t value = random(255);
        doTwinkle(0, 0, kMatrixWidth, kMatrixHeight, value, value, value);
        gfx->show();
    }
    dlog.info(TAG, "Creating display task");
    xTaskCreatePinnedToCore(&taskGateway<Display>, "Display", 4096, this, 1, NULL, ARDUINO_RUNNING_CORE);
    _app.onModeChange(std::bind(&Display::modeChange, this, std::placeholders::_1));
}

void Display::modeChange(AppMode mode)
{
    dlog.info(TAG, "Display::modeChange: %d", mode);

    _no_clear = false;

    switch (mode)
    {
    case AppMode::STARTING:
        break;

    case AppMode::CHOOSING:
        break;

    case AppMode::RUNNING:
        _blinker.detach();
        _gameover.detach();
        break;

    case AppMode::GAME_OVER:
        _blinker.attach_ms(150, &Display::queueBlink, this);
        _gameover.attach_ms(60000, &Display::queueScrollGameOver, this);
        Display::queueScrollGameOver(this);
        break;
    }
    render();
}

void Display::getScoreSize(uint16_t* width, uint16_t* height)
{
    int16_t text_x1, text_y1;
    gfx->setFont(SCORE_FONT);
    gfx->getTextBounds("00", 0, 0, &text_x1, &text_y1, width, height);
    *width += 4;
    *height = kMatrixHeight;
}

void Display::drawTeamBackground(Score::Team team)
{
    uint16_t width, height;
    getScoreSize(&width, &height);
    // assume LHS
    int16_t x = 0;
    if (_app.getSide(team) == Score::RHS)
    {
        x = kMatrixWidth - width;
    }
    const rgb24& color = getTeamColor(team);
    gfx->fillRect(x, 0, width, kMatrixHeight, gfx->Color(color.red, color.green, color.blue));
}

void Display::drawSide(Score::Side side, int value, bool drawBackground)
{
    //Serial.printf("Display::drawSide: %d:%d\n", side, value);

    uint16_t width, height;
    getScoreSize(&width, &height);
    // assume LHS
    int16_t x = 2;
    int16_t y = kMatrixHeight - 3;
    if (side == Score::RHS)
    {
        x = kMatrixWidth - width + 2;
    }
    Score::Team team = _app.getTeam(side);
    gfx->setFont(SCORE_FONT);
    if (drawBackground)
    {
        drawTeamBackground(team);
    }
    gfx->setCursor(x, y);
    gfx->printf("%02d", value);
}

void Display::drawScoreboard()
{
    drawSide(Score::LHS, _app.getScore(Score::LHS));
    drawSide(Score::RHS, _app.getScore(Score::RHS));
}

void Display::drawChoices()
{
    drawSide(Score::LHS, 15, false);
    drawSide(Score::RHS, 21, false);
    if (!isScrolling())
    {
        scrollingLayer.setColor(green);
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(60);
        scrollingLayer.setFont(SCROLL_FONT);
        scrollingLayer.setOffsetFromTop(kMatrixHeight/2 - SCROLL_FONT_HIGHT/2);
        scrollingLayer.start("Choose Game Limit", -1);
    }
}

void Display::gameOver()
{
    uint16_t width, height;
    getScoreSize(&width, &height);
    Score::Team team = _app.getLeader();

    rgb24 box_color = black;

    if (_blink_state)
    {
        box_color = green;
    }

    // assume LHS
    int16_t x = 0;
    if (_app.getSide(team) == Score::RHS)
    {
        x = kMatrixWidth - width;
    }

    gfx->drawRect(x, 0, width, kMatrixHeight, gfx->Color(box_color.red, box_color.green, box_color.blue));
    gfx->drawRect(x+1, 1, width-2, kMatrixHeight-2, gfx->Color(box_color.red, box_color.green, box_color.blue));
}

void Display::drawStarting()
{
    const char* m = "Initializing";
    int16_t w = STARTING_FONT_WIDTH * strlen(m);
    int16_t x = kMatrixWidth/2 - w/2;
    int16_t y = 0;
    backgroundLayer.setFont(STARTING_FONT);
    backgroundLayer.drawString(x, y, white, m);
}

#ifdef RENDER_FPS
static void renderFPS()
{
    static char fps_display[10];
    uint16_t real_fps = matrix.getRefreshRate();
    snprintf(fps_display, sizeof(fps_display), "%u", real_fps);
    int16_t min_x = 24; // 2 * SCORE_FONT->Width + 1;
    int16_t max_x = 40; //kMatrixWidth - (2 * SCORE_FONT->Width) + 3;
    int16_t width = FPS_FONT->Width * strlen(fps_display);
    int16_t x = min_x + (max_x-min_x)/2 - width/2;
    int16_t y = kMatrixHeight - FPS_FONT->Height -1;
    backgroundLayer.setFont(FPS_FONT);
    backgroundLayer.drawString(x, y, white, fps_display);
}
#endif

void Display::render()
{
    dlog.info(TAG, "render() before queue size: %u", uxQueueMessagesWaiting(_queue));
    const char* cmd = CMD_RENDER;
    // Send a pointer to a struct const char*.  Don't block if the
    // queue is already full.
   xQueueSend( _queue, &cmd, ( TickType_t ) 100 / portTICK_PERIOD_MS );
}

void Display::doRender()
{
    dlog.info(TAG, "doRender()");
    if (!_no_clear)
    {
        backgroundLayer.fillScreen(black);
    }

    switch (_app.mode())
    {
    case AppMode::STARTING:
        drawStarting();
        doMessage(_message);
        break;

    case AppMode::CHOOSING:
        drawChoices();
        break;

    case AppMode::RUNNING:
        if (!_no_clear)
        {
            _no_clear = true;
        }
        doStopScrolling();
        drawScoreboard();
        break;

    case AppMode::GAME_OVER:
        if (!_no_clear)
        {
            _no_clear = true;
        }
        drawScoreboard();
        gameOver();
    }
 
 #ifdef RENDER_FPS
    renderFPS();
 #endif
    gfx->show();
}

void Display::doTwinkle(int16_t x, int16_t y, uint16_t width, uint16_t height, uint8_t r, uint8_t g, uint8_t b, size_t count)
{
    int16_t x0, y0;

    if (count == 0)
    {
        count = (width + height) / 2;
    }

    // turn some on
    for (size_t i = 0; i < count; i++) {
        x0 = x + random(width);
        y0 = y + random(height);

        gfx->drawPixel(x0, y0, gfx->Color(r, g, b));
    }

    // and some off
    for (size_t i = 0; i < count; i++) {
        x0 = x + random(width);
        y0 = y + random(height);

        gfx->drawPixel(x0, y0, gfx->Color(0,0,0));
    }
}

void Display::doPixels(int16_t x, int16_t y, uint16_t width, uint16_t height)
{
    static unsigned long currentMillis = 0;
    const unsigned int transitionTime = 3000;

    if (millis() - currentMillis < transitionTime)
    {
        currentMillis = millis();
    }

    rgb24 color;
    float fraction = ((float)millis() - currentMillis) / ((float)transitionTime / 2);

    Score::Team team = _app.getLeader();
 
    if (team == Score::Team::RED) 
    {
        color.red = 255 - 255.0 * fraction;
        color.green = 0;
        color.blue = 0;
    }
    else 
    {
        color.red = 0;
        color.green = 0;
        color.blue = 255 - 255.0 * fraction;
    }

    doTwinkle(x, y, width, height, color.red, color.green, color.blue);
}

void Display::task()
{
    const char* message = nullptr;
    while(true) 
    {
        UBaseType_t cnt = uxQueueMessagesWaiting(_queue);
        dlog.trace(TAG, "loop: queue size: %u", cnt);

        if (xQueueReceive(_queue, &message, 100 / portTICK_PERIOD_MS ) == pdTRUE)
        {
            dlog.info(TAG, "loop: item from queue: 0x%08x", message);

            if (message == CMD_STOP_SCROLL)
            {
                doStopScrolling();
            }
            else if (message == CMD_BLINK)
            {
                _blink_state = !_blink_state;
            }
            else if (message == CMD_SCROLL_GAMEOVER)
            {
                scrollingLayer.setColor(green);
                scrollingLayer.setMode(wrapForward);
                scrollingLayer.setSpeed(40);
                scrollingLayer.setFont(GAME_OVER_FONT);
                scrollingLayer.setStartOffsetFromLeft(1);
                scrollingLayer.setOffsetFromTop(kMatrixHeight/2 - GAME_OVER_FONT_HIGHT/2);
                scrollingLayer.start("Game Over", 1);
            }
            else if (message != nullptr)
            {
                _message = message;
            }
            doRender();
        }
        else
        {
            uint16_t width, height;
            getScoreSize(&width, &height);
            int16_t x = width;
            int16_t y = 0;
            width = kMatrixWidth - 2 * width;
            const uint8_t blue_boost = 60; // the panel I am using red seems brighter than blue
            if (_app.mode() == AppMode::GAME_OVER)
            {
                gameOver();
                doPixels(x, y, width, height);
            }
            else if (_app.mode() == AppMode::CHOOSING)
            {
                uint8_t pixel = random(255-blue_boost);
                doTwinkle(0, 0, kMatrixWidth, kMatrixHeight, pixel, 0, 0, width+height);
                doTwinkle(0, 0, kMatrixWidth, kMatrixHeight, 0, 0, pixel+blue_boost, width+height);
                drawChoices();
            }
            else if (_app.mode() == AppMode::RUNNING)
            {
                int red_score = _app.getScore(_app.getSide(Score::Team::RED));
                int blue_score = _app.getScore(_app.getSide(Score::Team::BLUE));
                int count = blue_score+red_score;
                if (count < 1)
                {
                    count = 1;
                }
                uint8_t pixel = random(255);
                doTwinkle(x, y, width, height, pixel, pixel, pixel, count);
            }
            gfx->show();
        }
    }
}

void Display::splash(const char* message)
{
    scrollingLayer.setColor(green);
    scrollingLayer.setMode(wrapForward);
    scrollingLayer.setSpeed(60);
    scrollingLayer.setFont(SCROLL_FONT);
    scrollingLayer.setOffsetFromTop(kMatrixHeight/2 - SCROLL_FONT_HIGHT/2);
    scrollingLayer.start(message, 1);
}

void Display::message(const char* message)
{
    dlog.info(TAG, "message: '%s' before queue size: %u", message, uxQueueMessagesWaiting(_queue));
    // Send a pointer to a const char*.  Don't block if the
    // queue is already full.
    if (xQueueSend( _queue, &message, ( TickType_t ) 100 / portTICK_PERIOD_MS ) != pdTRUE)
    {
        dlog.error(TAG, "message: failed to queue message!");
    }
    delay(250); // short delay to insure message is displayed for a short time at least.
}

void Display::doMessage(const char* message)
{
    if (message)
    {
        dlog.info(TAG, "doMessage: %s", message);
        int16_t w = STARTING_FONT_WIDTH * strlen(message);
        int16_t h = STARTING_FONT_HIGHT;
        int16_t x = kMatrixWidth/2 - w/2;
        int16_t y = kMatrixHeight/2 - h/2;
        backgroundLayer.setFont(STARTING_FONT);
        backgroundLayer.drawString(x, y, orange, message);
    }
}

bool Display::isScrolling()
{
    return scrollingLayer.getStatus() != 0;
}

void Display::stopScrolling()
{
    dlog.info(TAG, "stopScrolling() before queue size: %u", uxQueueMessagesWaiting(_queue));
    // Send a pointer to a struct const char*.  Don't block if the
    // queue is already full.
    const char* ss = (const char*)1;
    xQueueSend( _queue, &ss, ( TickType_t ) 100 / portTICK_PERIOD_MS );
}

void Display::doStopScrolling()
{
    scrollingLayer.stop();
}

void Display::queueBlink(Display* display)
{
    const char* cmd = CMD_BLINK;
    xQueueSendFromISR(display->_queue, &cmd, nullptr);
}

void Display::queueScrollGameOver(Display* display)
{
    const char* cmd = CMD_SCROLL_GAMEOVER;
    xQueueSendFromISR(display->_queue, &cmd, nullptr);
}
