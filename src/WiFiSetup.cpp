/*
 * WiFiSetup.cpp
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
 *  Created on: May 27, 2018
 *      Author: chris.l
 */

#include "WiFiSetup.h"
#include <functional>

#include "Log.h"
static const char* TAG = "WiFiSetup";

WiFiSetup::WiFiSetup(Config& config, Display& display, boolean debug)
: _config(config),
  _display(display),
  _wm(),
  _hostname("hostname", "Hostname", "", 64),
  _enable_password("enable_password", "Enable Password", "", 64),
  _start_wifi("start_wifi", "Start WiFi", "false", 8)
{
    _wm.setDebugOutput(debug);

    using namespace std::placeholders;
    _wm.setAPCallback(std::bind(&WiFiSetup::startingPortal, this, _1));
    _wm.setSaveConfigCallback(std::bind(&WiFiSetup::saveConfig, this));
}

WiFiSetup::~WiFiSetup()
{
}

void WiFiSetup::connect(bool force_config)
{
    dlog.info(TAG, F("connect: disableing captive portal when auto-connecting"));
    WiFi.mode(WIFI_MODE_STA);
    _wm.setEnableConfigPortal(false); // don't automatically use the captive portal
    String devicename = "ScoreBoard:" + WiFi.macAddress();
    dlog.info(TAG, "Device name: %s", devicename.c_str());

    if (force_config)
    {
        dlog.info(TAG, F("connect: starting forced config portal!"));
        _wm.startConfigPortal(devicename.c_str(), nullptr);
    }
    else
    {
        dlog.info(TAG, F("connect: auto-connecting"));
        while (!_wm.autoConnect())
        {
            dlog.error(TAG, F("connect: not connected! retrying...."));
            delay(1000);
        }
    }
    WiFi.setSleep(false);
}

void WiFiSetup::startingPortal(WiFiManager* wmp)
{
    (void) wmp;
    dlog.info(TAG, F("startingPortal: updating param values"));
    _hostname.setValue(_config.getHostname().c_str(), 64);
    _enable_password.setValue(_config.getEnablePassword().c_str(), 64);
    _start_wifi.setValue(_config.getStartWiFi() ? "true" : "false", 10);

    dlog.info(TAG, F("startingPortal: adding params"));
    _wm.addParameter(&_hostname);
    _wm.addParameter(&_enable_password);
    _wm.addParameter(&_start_wifi);

    dlog.debug(TAG, F("startingPortal: params added!"));

    //
    // Update the display with the SSID to show portal is up
    //
    dlog.info(TAG, F("startingPortal: updating display"));
    _display.message(_wm.getConfigPortalSSID().c_str());
}

void WiFiSetup::saveConfig()
{
    dlog.info(TAG, "saveConfig: updating values");
    _config.setHostname(getHostname());
    _config.setEnablePassword(getEnablePassword());
    _config.setStartWiFi(getStartWiFi());
    dlog.debug(TAG, "saveConfig: saving!");
    _config.save();
}

const String WiFiSetup::getParam(WiFiManagerParameter& param)
{
    return param.getValue();
}

const String WiFiSetup::getHostname()
{
    return getParam(_hostname);
}

const String WiFiSetup::getEnablePassword()
{
    return getParam(_enable_password);
}

bool WiFiSetup::getStartWiFi()
{
    const String value = getParam(_start_wifi);
    return value.equalsIgnoreCase("True");
}
