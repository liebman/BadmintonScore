/**
 * @file ScoreboardClient.h
 * @author Christoper B. Liebman
 * @brief Manager web app
 * @version 0.1
 * @date 2019-12-20
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

#ifndef SCOREBOARD_CLIENT_H_
#define SCOREBOARD_CLIENT_H_
#include <WebsocketHandler.hpp>
#include "Config.h"

using namespace httpsserver;

class ScoreboardClient : public WebsocketHandler {
public:
    // This method is called by the webserver to instantiate a new handler for each
    // client that connects to the websocket endpoint
    static WebsocketHandler* create();

    // This method is called when a message arrives
    void onMessage(WebsocketInputStreambuf * input);

    // Handler function on connection close
    void onClose();

    bool isAdmin() {return _admin;}

private:
    bool _admin;
};
#endif // SCOREBOARD_CLIENT_H_
