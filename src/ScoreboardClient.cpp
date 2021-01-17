/**
 * @file ScoreboardClient.cpp
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

#include "ScoreboardClient.h"
#include "WebApp.h"
#include "Log.h"
#include <ArduinoJson.h>

static const char* TAG = "ScoreboardClient";

WebsocketHandler* ScoreboardClient::create()
{
    ScoreboardClient* client = new ScoreboardClient();
    WebApp::getInstance().addClient(client);
    return client;
}

void ScoreboardClient::onMessage(WebsocketInputStreambuf * input)
{
    // Get the input message
    std::ostringstream ss;
    std::string msg;
    ss << input;
    msg = ss.str();
    dlog.info(TAG, "websocket message: '%s'", msg.c_str());
    StaticJsonDocument<500> doc;
    DeserializationError err = deserializeJson(doc, msg);
    if (err)
    {
        dlog.error(TAG, "deserializeJson() failed: '%s", err.c_str());
        return;
    }
    App& app = App::getInstance();
    String action = doc["action"];
    if (action == "refresh")
    {
        // {"action":"refresh"}
        WebApp::getInstance().updateClients();
    }
    else if (action == "enable")
    {
        // {"action":"enable","password":"admin"}
        Config* config = WebApp::getInstance().getConfig();
        if (config != nullptr && doc["password"] == config->getEnablePassword())
        {
            _admin = true;
        }
        else
        {
            _admin = false;
        }
        WebApp::getInstance().updateClients();
    }
    else if (isAdmin())
    {
        if (action == "update")
        {
            // {"action":"update","side":"lhs","delta":1}
            Score::Side side = doc["side"].as<String>() == "lhs" ? Score::Side::LHS : Score::Side::RHS;
            int delta = doc["delta"];
            app.incrementScore(side, delta);
        }
        else if (action == "limits")
        {
            // {"action":"limits","limit":15,"max_limit":21}
            int limit     = doc["limit"];
            int max_limit = doc["max_limit"];
            if (limit > 0 && max_limit > 0)
            {
                app.setLimits(limit, max_limit);
                if (app.mode() == AppMode::CHOOSING)
                {
                    app.mode(AppMode::RUNNING);
                }
            }
        }
        else if (action == "swap")
        {
            // {"action":"swap"}
            app.swap();
        }
        else if (action == "reset")
        {
            // {"action":"reset"}
            app.reset();
        }
    }
    else
    {
        dlog.error(TAG, "unknown action: %s", action.c_str());
    }
}

void ScoreboardClient::onClose()
{
    WebApp::getInstance().removeClient(this);
    // delete handled by caller.
    //delete this;
}
