/**
 * @file main.cpp
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
#include "App.h"
#include "WebApp.h"
#include "Buttons.h"
#include "Display.h"
#include <functional>
#include <SPIFFS.h>
#include "Config.h"
#include "WiFiSetup.h"
#include "Log.h"
#include "DLogPrintWriter.h"

const uint8_t SCORE_RHS_PIN  = 17;
const uint8_t SCORE_LHS_PIN  = 18;
const uint8_t SCORE_SWAP_PIN = 19;

static App& app = App::getInstance();
static Display display(app);
static Buttons buttons(app, SCORE_LHS_PIN, SCORE_RHS_PIN, SCORE_SWAP_PIN);

DLog& dlog = DLog::getLog();
Config config;

#ifdef USE_NETWORK_BY_DEFAULT
const bool DEFAULT_USE_NETWORK = true;
#else
const bool DEFAULT_USE_NETWORK = false;
#endif

#ifdef SHOW_MEMORY_USAGE
#define MEMORY_USAGE(s)  printMemInfo(s)
void printMemInfo(const char* label)
{
    static const char* TAG = "printMemInfo";
    uint32_t heap_size = ESP.getHeapSize();
    uint32_t heap_free = ESP.getFreeHeap();
    uint32_t min_free = ESP.getMinFreeHeap();
    uint32_t largest_free = ESP.getMaxAllocHeap();
    dlog.info(TAG, "%s - size:%u free:%u largest:%u minfree:%u", label, heap_size, heap_free, largest_free, min_free);
}
#else
#define MEMORY_USAGE(s) 
#endif

static void logTimeFirst(DLogBuffer& buffer, DLogLevel level)
{
    (void)level; // not used
    const char* name = pcTaskGetTaskName(NULL);
    uint32_t core =  xPortGetCoreID();
    //buffer.printf("%1.3f ", millis()/1000.0);
    buffer.printf("%010lu %u:%s ", millis(), core, name);
}

void initWiFi(bool force_config)
{
        MEMORY_USAGE("before WiFiSetup");
        WiFiSetup wfs(config, display, true);
        wfs.connect(force_config);
    
        MEMORY_USAGE("before WebApp");
        display.message("WebApp");
        WebApp& wapp = WebApp::getInstance();
        wapp.begin(&config, &SPIFFS, "/scoreboard");
}

void setup() {
    static const char* TAG = "setup";
    Serial.begin(115200);
    dlog.begin(new DLogPrintWriter(Serial));
    dlog.setPreFunc(&logTimeFirst);
    delay(200);
    dlog.info(TAG, "Starting!");
    MEMORY_USAGE("setup");

    buttons.begin();

    display.begin("Scoreboard 1.0");
    app.mode(AppMode::STARTING);
    display.render();

    MEMORY_USAGE("befor Config");
    display.message("Config");
    config.begin();
    bool force_config = false;
    if (!config.load())
    {
        dlog.info(TAG, "config failed to load, forcing config if wifi enabled");
        force_config = true;
    }
    MEMORY_USAGE("after Config");

    bool start_network = DEFAULT_USE_NETWORK;
    if (config.getStartWiFi())
    {
        start_network = true;
    }

    if (digitalRead(SCORE_LHS_PIN) == 0 && digitalRead(SCORE_RHS_PIN) == 0)
    {
        force_config = true;
    }

    if (digitalRead(SCORE_SWAP_PIN) == 0)
    {
        start_network = !start_network;
    }

    if (start_network)
    {
        initWiFi(force_config);
    }

    //
    // any app change notifies the display
    //
    MEMORY_USAGE("before app.onChange");
    app.onChange(std::bind(&Display::render, &display));

    MEMORY_USAGE("setup done");
}

void loop()
{
#ifdef SHOW_MEMORY_USAGE
    static uint32_t last_memory_display = 0;
    uint32_t now = millis();
    if ((now - last_memory_display) > SHOW_MEMORY_USAGE)
    {
        MEMORY_USAGE("loop");
        last_memory_display = now;
    }
#endif

    // soft reset: hold swap for 10ish seconds
    if (buttons.isReset())
    {
        ESP.restart();
    }

    switch (app.mode())
    {
    case AppMode::STARTING:
        if (!display.isScrolling())
        {
            app.mode(AppMode::CHOOSING);
        }
        break;

    case AppMode::CHOOSING:
        // state change here happens in choice()
        break;

    case AppMode::RUNNING:
        if (app.isGameOver())
        {
            app.mode(AppMode::GAME_OVER);
        }
        break;

    case AppMode::GAME_OVER:
        if (!app.isGameOver())
        {
            app.mode(AppMode::RUNNING);
        }
        break;
    }
    delay(1);
}
