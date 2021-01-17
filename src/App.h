/**
 * @file App.h
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

#ifndef APP_H_
#define APP_H_

#include <functional>
#include <vector>
#include "Score.h"

using AppMode = enum app_mode {STARTING, CHOOSING, RUNNING, GAME_OVER};
using ModeChangeCB = std::function<void(AppMode mode)>;
using ChangeCB = std::function<void(void)>;

class App
{
public:
    static App& getInstance();
    AppMode mode();
    AppMode mode(AppMode mode);
    void onChange(ChangeCB cb);
    void onModeChange(ModeChangeCB cb);
    Score::Side getSide(Score::Team team);
    Score::Team getTeam(Score::Side side);
    int getScore(Score::Side side);
    Score::Team getLeader();
    void setLimits(int value, int max_value);
    void swap();
    void reset();
    void incrementScore(Score::Side side, int delta);
    bool isGameOver();

private:
    App();
    void changeNotify();
    AppMode _mode;
    Score _score;
    std::vector<ModeChangeCB> _mode_cb;
    std::vector<ChangeCB> _change_cb;
};

#endif