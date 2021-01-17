/**
 * @file Display.h
 * @author Christoper B. Liebman
 * @brief Manage matrix display
 * @version 0.1
 * @date 2019-10-25
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

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "Arduino.h"
#include "App.h"
#include "Ticker.h"
#include "TaskGateway.h"

class Display
{
public:
    Display(App& app);
    virtual ~Display();
    void begin(const char* message);
    void render();
    void message(const char* message);
    void splash(const char* message);
    bool isScrolling();
    void stopScrolling();
    void modeChange(AppMode mode);

private:
    App&                    _app;
    QueueHandle_t           _queue;
    const char* volatile    _message;       // Starting messages
    bool                    _blink_state;   // blink is on or off
    bool                    _no_clear;      // don't clear first on render
    Ticker                  _blinker;
    Ticker                  _gameover;

    static void queueBlink(Display* display);
    static void queueScrollGameOver(Display* display);
    void getScoreSize(uint16_t* width, uint16_t* height);
    void drawTeamBackground(Score::Team team);
    void drawSide(Score::Side side, int value, bool drawBackground = true);
    void drawScoreboard();
    void drawChoices();
    void gameOver();
    void drawStarting();
    void doRender();
    void doMessage(const char* message);
    void doStopScrolling();
    void doTwinkle(int16_t x, int16_t y, uint16_t width, uint16_t height, uint8_t r, uint8_t g, uint8_t b, size_t count = 0);
    void doPixels(int16_t x, int16_t y, uint16_t width, uint16_t height);
    void task();
    friend void taskGateway<Display>(void*data);
};
#endif
