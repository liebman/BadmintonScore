/**
 * @file Buttons.h
 * @author Christoper B. Liebman
 * @brief Manager web app
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

#include "Buttons.h"
#include "Log.h"

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

static const char* TAG = "Buttons";

Buttons::Buttons(App& app, int lhs_pin, int rhs_pin, int swap_pin)
: _app(app),
  _lhs(lhs_pin), _rhs(rhs_pin), _swap(swap_pin),
  _lhs_pin(lhs_pin), _rhs_pin(rhs_pin), _swap_pin(swap_pin)
{
}

bool Buttons::begin()
{
    dlog.info(TAG, "Creating Buttons task... ");
    xTaskCreatePinnedToCore(&taskGateway<Buttons>, "Buttons", 8192, this, 1, NULL, ARDUINO_RUNNING_CORE);
    return true;    
}

void Buttons::task()
{
    dlog.info(TAG, "task()");
    pinMode(_swap_pin, INPUT_PULLUP);
    pinMode(_lhs_pin,  INPUT_PULLUP);
    pinMode(_rhs_pin,  INPUT_PULLUP);
    _app.onModeChange(std::bind(&Buttons::modeChange, this, std::placeholders::_1));
    while(true)
    {
        _swap.read();
        _lhs.read();
        _rhs.read();
    }
    delay(1);
}

bool Buttons::isReset()
{
    return _swap.pressedFor(10000);
}

void Buttons::modeChange(AppMode mode)
{
    dlog.info(TAG, "modeChange()");
    switch (mode)
    {
    case AppMode::STARTING:
        dlog.info(TAG, "modeChange: STARTING");
        _swap.onPressed(nullptr);
        _swap.onPressedFor(1000, nullptr);
        _lhs.onPressed(nullptr);
        _lhs.onPressedFor(1000, nullptr);
        _rhs.onPressed(nullptr);
        _rhs.onPressedFor(1000, nullptr);
        break;

    case AppMode::CHOOSING:
        dlog.info(TAG, "modeChange: CHOOSING");
        _lhs.onPressed(std::bind(&App::setLimits, &_app, 15, 20));
        _rhs.onPressed(std::bind(&App::setLimits, &_app, 21, 30));
        break;

    case AppMode::RUNNING:
        dlog.info(TAG, "modeChange: RUNNING");
        _swap.onPressed(std::bind(&App::swap, &_app));
        _swap.onPressedFor(1000, std::bind(&App::reset, &_app));
        _lhs.onPressed(std::bind(&App::incrementScore, &_app, Score::LHS, 1));
        _lhs.onPressedFor(1000, std::bind(&App::incrementScore, &_app, Score::LHS, -1));
        _rhs.onPressed(std::bind(&App::incrementScore, &_app, Score::RHS, 1));
        _rhs.onPressedFor(1000, std::bind(&App::incrementScore, &_app, Score::RHS, -1));
        break;

    case AppMode::GAME_OVER:
        dlog.info(TAG, "modeChange: GAME_OVER");
        break;
    }
}
