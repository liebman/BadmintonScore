/**
 * @file App.cpp
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

#include "App.h"
#include "Arduino.h"
#include "Log.h"

static const char* TAG = "App";

AppMode App::mode()
{
    return _mode;
}

AppMode App::mode(AppMode mode)
{
    AppMode old_mode = _mode;
    for(ModeChangeCB cb : _mode_cb)
    {
        cb(mode);
    }
    _mode = mode;
    changeNotify();
    return old_mode;
}

void App::changeNotify()
{
    dlog.info(TAG, "changeNotify() size: %d", _change_cb.size());
    for(ChangeCB cb : _change_cb)
    {
        dlog.info(TAG, "changeNotify() on client!");
        cb();
    }
}

void App::onChange(ChangeCB cb)
{
    _change_cb.push_back(cb);
}

void App::onModeChange(ModeChangeCB cb)
{
    _mode_cb.push_back(cb);
}

Score::Side App::getSide(Score::Team team)
{
    return _score.getSide(team);
}

Score::Team App::getTeam(Score::Side side)
{
    return _score.getTeam(side);
}

int App::getScore(Score::Side side)
{
    return _score.getScore(side);
}

Score::Team App::getLeader()
{
    return _score.getLeader();
}

void App::setLimits(int value, int max_value)
{
    _score.setLimits(value, max_value);
    mode(AppMode::RUNNING); // will call changeNotify()
}

void App::swap()
{
    _score.swap();
    changeNotify();
}

void App::reset()
{
    _score.reset();
    changeNotify();
}

void App::incrementScore(Score::Side side, int delta)
{
    _score.incrementScore(side, delta);
    changeNotify();
}

bool App::isGameOver()
{
    return _score.isGameOver();
}

App::App()
: _mode(),
  _score(),
  _mode_cb()
{
}

App& App::getInstance()
{
    static App instance;
    return instance;
}

