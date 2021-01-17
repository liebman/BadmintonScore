/*
 * Config.cpp
 *
 *  Created on: Jun 10, 2018
 *      Author: chris.l
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
 */

#include "Config.h"
#include <SPIFFS.h>
#include <FS.h>
#include "Log.h"
#include "ArduinoJson.h"

static const char* TAG = "Config";
static const char* CONFIG_FILE = "/Config.json";


Config::Config() : _hostname(), _enable_password(), _start_wifi()
{
}

Config::~Config()
{
}

bool Config::begin()
{
    dlog.info(TAG, "begin: Mounting SPIFFS");

    if (!SPIFFS.begin())
    {
        dlog.warning(TAG, "begin: Formatting SPIFFS!!!!!");
        if (SPIFFS.format())
        {
            dlog.error(TAG, "begin: SPIFFS format failed!");
            return false;
        }
        dlog.info(TAG, "begin: mounting SPIFFS after format!");
        if (!SPIFFS.begin())
        {
            dlog.error(TAG, "begin: SPIFFS mount failed!!!!");
            return false;
        }
    }

    return true;
}

bool Config::load()
{
    dlog.info(TAG, "load: file: '%s'", CONFIG_FILE);

    if (!SPIFFS.exists(CONFIG_FILE))
    {
        dlog.warning(TAG, "load: config file does not exist!");
        return false;
    }

    File f = SPIFFS.open(CONFIG_FILE, "r");
    if (!f)
    {
        dlog.error(TAG, "load: failed to open config file!");
        return false;
    }

    StaticJsonDocument<512> doc;

    dlog.debug(TAG, "load: parsing file contents");
    DeserializationError err = deserializeJson(doc, f);
    if (err)
    {
        dlog.error(TAG, "deserializeJson() failed: '%s", err.c_str());
        return false;
    }
    f.close();

    _hostname        = doc["hostname"].as<String>();
    _enable_password = doc["enable_password"].as<String>();
    _start_wifi      = doc["start_wifi"];

    dlog.info(TAG, "load: hostname:'%s', enable:'%s', start_wifi:'%s'",
                   _hostname.c_str(), _enable_password.c_str(), _start_wifi?"true":"false");
    dlog.info(TAG, "load: config loaded!");
    return true;
}

void Config::save()
{
    dlog.info(TAG, "save: file: '%s'", CONFIG_FILE);

    File f = SPIFFS.open(CONFIG_FILE, "w");

    StaticJsonDocument<512> doc;

    doc["hostname"]        = _hostname;
    doc["enable_password"] = _enable_password;
    doc["start_wifi"]      = _start_wifi;

    serializeJson(doc, f);
    f.close();

    dlog.info(TAG, "save: config saved");
}

const String& Config::getHostname()
{
    return _hostname;
}

void Config::setHostname(const String& host)
{
    _hostname = host;
}
const String& Config::getEnablePassword()
{
    return _enable_password;
}

void Config::setEnablePassword(const String& password)
{
    _enable_password = password;
}

bool Config::getStartWiFi()
{
    return _start_wifi;
}

void Config::setStartWiFi(bool start_wifi)
{
    _start_wifi = start_wifi;
}
