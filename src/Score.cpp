/**
 * @file Score.cpp
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
#include "Score.h"
#include "Log.h"

static const char* TAG = "Score";

Score::Score()
: _limit(DEFAULT_SCORE_LIMIT),
  _max_limit(DEFAULT_SCORE_MAX_LIMIT),
  _score(), 
  _swappped(false)
{
}

Score::~Score()
{
}

void Score::swap()
{
    dlog.info(TAG, "Score::swap()");
    _swappped = !_swappped;
}

void Score::reset()
{
    dlog.info(TAG, "Score::reset()");
    for(int i = 0; i < NUM_TEAMS; ++i)
    {
        _score[i] = 0;
    }
    _swappped = false;
}

void Score::setLimits(int limit, int max_limit)
{
    dlog.info(TAG, "Score::setLimits: %d/%d", limit, max_limit);
    _limit     = limit;
    _max_limit = max_limit;
}

Score::Side Score::getSide(Team team)
{
    Side side = LHS;
    if (team == BLUE)
    {
        side = RHS;
    }
    if (_swappped)
    {
        if (side == LHS)
        {
            side = RHS;
        } else if (side == RHS)
        {
            side = LHS;
        }
    }
    return side;
}

Score::Team Score::getLeader()
{
    return _score[BLUE] > _score[RED] ? BLUE : RED;
}

Score::Team Score::getTeam(Side side)
{
    Team team = RED;
    if (side == RHS)
    {
        team = BLUE;
    }
    if (_swappped)
    {
        if (team == RED)
        {
            team = BLUE;
        } else if (team == BLUE)
        {
            team = RED;
        }
    }
    return team;
}

int Score::getScore(Side side)
{
    Team team = getTeam(side);
    return _score[team];
}

void Score::incrementScore(Side side, int increment)
{
    dlog.info(TAG, "Score::incrementScore: %s by %d", side == LHS ? "LHS" : "RHS", increment);
    if (isGameOver())
    {
        // no score can go up if game is over and only the winner can go down
        if (increment > 0 || getSide(getLeader()) != side)
        {
            return;
        }
    }
    Team team = getTeam(side);
    int score = _score[team] + increment;

    // can't go negative
    if (score < 0)
    {
        return;
    }
    _score[team] = score;
}

bool Score::isGameOver()
{
    // return if no one has reached the limit
    if (_score[RED] < _limit && _score[BLUE] < _limit)
    {
        return false;
    }

    // if more than 1 point lead wins
    if (abs(_score[RED] - _score[BLUE]) > 1)
    {
        return true;
    }

    // first one to max_limit wins
    if (_score[RED] >= _max_limit || _score[BLUE] >= _max_limit)
    {
        return true;
    }

    return false;
}
