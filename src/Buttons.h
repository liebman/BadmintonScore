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

#ifndef BUTTONS_H_
#define BUTTONS_H_

#include <Arduino.h>
#include <functional>
#include <EasyButton.h>
#include "App.h"
#include "TaskGateway.h"

class Buttons
{
public:
    Buttons(App& app, int lhs_pin, int rhs_pin, int swap_pin);
    bool begin();
    bool isReset();
    void modeChange(AppMode mode);

private:
    App&       _app;
    EasyButton _lhs;
    EasyButton _rhs;
    EasyButton _swap;
    uint8_t    _lhs_pin;
    uint8_t    _rhs_pin;
    uint8_t    _swap_pin;
    void task();
    friend void taskGateway<Buttons>(void*data);};

#endif