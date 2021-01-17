/**
 * @file Score.h
 * @author Christoper B. Liebman
 * @brief Manages score values
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
 * At 20 all, the side which gains a 2 point lead first, wins that game.
 * At 29 all, the side scoring the 30th point, wins that game.
 */
#ifndef SCORE_H_
#define SCORE_H_

#define DEFAULT_SCORE_LIMIT 21
#define DEFAULT_SCORE_MAX_LIMIT 30

class Score
{
public:
    // when not swapped RED is on the LHS
    enum Team {RED = 0, BLUE = 1, NUM_TEAMS};
    enum Side {LHS = 0, RHS = 1, NUM_SIDES};
    Score();
    virtual ~Score();
    void swap();
    void reset();
    void setLimits(int limit, int max_limit);
    Side getSide(Team team);
    Team getTeam(Side side);
    Team getLeader();
    int getScore(Side side);
    void incrementScore(Side side, int increment);
    bool isGameOver();

private:
    volatile int  _limit;
    volatile int  _max_limit;
    volatile int  _score[NUM_TEAMS];
    volatile bool _swappped;
};

#endif /* SCORE_H_ */
