/**
 * @file WebApp.cpp
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

#include "WebApp.h"
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <functional>
#include "ResourceParameters.hpp"
#include "Log.h"

static const char* TAG = "WebApp";

static const char* KEY_EXT     = ".key";
static const char* CRT_EXT     = ".crt";

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

static const char* getContentType(std::string reqFile)
{
    // We need to specify some content-type mapping, so the resources get delivered with the
    // right content type and are displayed correctly in the browser
    static char contentTypes[][2][32] = {
    {".html", "text/html"},
    {".css",  "text/css"},
    {".js",   "application/javascript"},
    {".json", "application/json"},
    {".png",  "image/png"},
    {".jpg",  "image/jpg"},
    {"", ""}
    };
    // Content-Type is guessed using the definition of the contentTypes-table defined above
    int cTypeIdx = 0;
    do {
        if(reqFile.rfind(contentTypes[cTypeIdx][0])!=std::string::npos) {
            return contentTypes[cTypeIdx][1];
        }
        cTypeIdx+=1;
    } while(strlen(contentTypes[cTypeIdx][0])>0);
    return "text/plain";
}

static void handle404(HTTPRequest * req, HTTPResponse * res)
{
    // Discard request body, if we received any
    // We do this, as this is the default node and may also server POST/PUT requests
    req->discardRequestBody();

    // Set the response status
    res->setStatusCode(404);
    res->setStatusText("Not Found");

    // Set content type of the response
    res->setHeader("Content-Type", "text/html");

    // Write a tiny HTTP page
    res->println("<!DOCTYPE html>");
    res->println("<html>");
    res->println("<head><title>Not Found</title></head>");
    res->println("<body><h1>404 Not Found</h1><p>The requested resource was not found on this server.</p></body>");
    res->println("</html>");
}

static void handleFS(HTTPRequest * req, HTTPResponse * res)
{
    FS* fs = WebApp::getInstance().getFS();
    if (fs == nullptr)
    {
        dlog.info(TAG, "WebApp::handleFS: app._fs is NULL!!!!");
        handle404(req, res);
        return;
    }
     // We only handle GET here
    if (req->getMethod() == "GET") {
        // Redirect / to /index.html
        std::string filename = req->getRequestString()=="/" ? "/index.html" : req->getRequestString();

        dlog.info(TAG, "WebApp::handleFS: filename: '%s'", filename.c_str());

        // Check if the file exists
        if (!fs->exists(filename.c_str())) {
            // Send "404 Not Found" as response, as the file doesn't seem to exist
            handle404(req, res);
            return;
        }

        File file = fs->open(filename.c_str());

        // Set length
        res->setHeader("Content-Length", httpsserver::intToString(file.size()));

        res->setHeader("Content-Type", getContentType(filename));

        // Read the file and write it to the response
        uint8_t buffer[256];
        size_t length = 0;
        do {
            length = file.read(buffer, 256);
            res->write(buffer, length);
        } while (length > 0);
        file.close();
    } else {
        // If there's any body, discard it
        req->discardRequestBody();
        // Send "405 Method not allowed" as response
        res->setStatusCode(405);
        res->setStatusText("Method not allowed");
        res->println("405 Method not allowed");
    } 
}

WebApp::WebApp() :
    _config(nullptr),
    _fs(nullptr),
    _cert(nullptr),
    _server(nullptr)
{
    dlog.info(TAG, "WebApp constructor");
    _clients.reserve(MAX_SCOREBOARD_CLIENTS);
}

WebApp& WebApp::getInstance()
{
    static WebApp *instance;
    if (instance == nullptr)
    {
        instance = new WebApp();
    }
    return *instance;
}

Config* WebApp::getConfig()
{
    return _config;
}

FS* WebApp::getFS()
{
    return _fs;
}

bool WebApp::begin(Config* config, FS* fs, const char* certname)
{
    dlog.info(TAG, "begin()");
    _config = config;
    _fs     = fs;

#ifdef USE_SECURE_SERVER
    if (certname != nullptr)
    {
        loadCert(certname);
    }
#endif
    dlog.info(TAG, "begin: Creating server task... ");
    xTaskCreatePinnedToCore(&taskGateway<WebApp>, "WebApp", 32768, this, 1, NULL, ARDUINO_RUNNING_CORE);
    return true;
}

void WebApp::task()
{
    dlog.info(TAG, "task: starting (this: 0x%08x)", this);
    start();
    dlog.info(TAG, "task: looping!");
    while(true)
    {
        if (_server != nullptr)
        {
            _server->loop();
        }
        delay(1);
    }
}

bool WebApp::start()
{
    // We can now use the new certificate to setup our server as usual.
#ifdef USE_SECURE_SERVER
    _server = new HTTPSServer(_cert, 443, MAX_SCOREBOARD_CLIENTS);
#else
    _server = new HTTPServer(80, MAX_SCOREBOARD_CLIENTS);
#endif
    ResourceNode* nodeRoot      = new ResourceNode("/", "GET", &handleFS);
    ResourceNode* nodeRootIndex = new ResourceNode("/index.html", "GET", &handleFS);
    ResourceNode* node404       = new ResourceNode("", "GET", &handle404);
    WebsocketNode* nodeWS       = new WebsocketNode("/ws", &ScoreboardClient::create);

    // Add the root node to the server
    _server->registerNode(nodeRoot);
    _server->registerNode(nodeRootIndex);
    _server->registerNode(nodeWS);

    // Add the 404 not found node to the server.
    _server->setDefaultNode(node404);
    App::getInstance().onChange(std::bind(&WebApp::updateClients, this));

    dlog.info(TAG, "Starting server...");
    _server->start();
    if (_server->isRunning()) {
        dlog.info(TAG, "Server ready.");
    }

    if (!MDNS.begin(_config->getHostname().c_str())) {
        Serial.println("Error setting up MDNS responder!");
        while(1) {
            delay(1000);
        }
    }
    // Add service to MDNS-SD
    MDNS.addService("https", "tcp", 443);
    return _server->isRunning();
}

bool WebApp::fileExists(const char* base_name, const char* ext)
{
    if (_fs == nullptr)
    {
        return false;
    }

    String name(base_name);
    name += ext;
    return _fs->exists(name);
}

bool WebApp::loadCert(const char* base_file_name)
{
    // First, we create an empty certificate:
    _cert = new SSLCert();
    if (loadCertFromFile(base_file_name))
    {
        return true;
    }
#ifndef HTTPS_DISABLE_SELFSIGNING
    return generateCert(base_file_name);
#else
    return false;
#endif
}

bool WebApp::loadCertFromFile(const char* base_file_name)
{
    if (!fileExists(base_file_name, KEY_EXT) || !fileExists(base_file_name, CRT_EXT))
    {
        return false;
    }
    uint16_t len = 0;
    uint8_t* buf = nullptr;
    if (!readFile(base_file_name, CRT_EXT, &buf, &len))
    {
        dlog.error(TAG, "******** WebApp::readFile '%s%s' failed!", base_file_name, CRT_EXT);
#ifndef HTTPS_DISABLE_SELFSIGNING
        return generateCert(base_file_name);
#else
        return false;
#endif
    }
    _cert->setCert(buf, len);
    buf = nullptr;
    len = 0;
    if (!readFile(base_file_name, KEY_EXT, &buf, &len))
    {
        dlog.error(TAG, "******** WebApp::readFile '%s%s' failed!", base_file_name, KEY_EXT);
#ifndef HTTPS_DISABLE_SELFSIGNING
        return generateCert(base_file_name);
#else
        return false;
#endif
    }
    _cert->setPK(buf, len);

    return true;
}

bool WebApp::writeFile(const char* base_name, const char* ext, uint8_t* data, size_t len)
{
    String name(base_name);
    name += ext;
    File f = _fs->open(name, "w");
    size_t written = f.write(data, len);
    f.close();
    dlog.info(TAG, "WebApp::writeFile: name:'%s' len:%u written:%u", name.c_str(), len, written);
    return written == len;
}

bool WebApp::readFile(const char* base_name, const char* ext, uint8_t** data, uint16_t* len)
{
    String name(base_name);
    name += ext;
    File f = _fs->open(name, "r");
    *len = f.size();
    *data = new uint8_t[*len];
    uint16_t count = f.read(*data, *len);
    dlog.info(TAG, "WebApp::readFile: name:'%s' len:%u read:%u", name.c_str(), *len, count);
    return count == *len;
}

#ifndef HTTPS_DISABLE_SELFSIGNING
bool WebApp::generateCert(const char* base_file_name)
{
    dlog.info(TAG, "Creating a new self-signed certificate.");
    dlog.info(TAG, "This may take up to a minute, so be patient ;-)");

    // Now, we use the function createSelfSignedCert to create private key and certificate.
    // The function takes the following paramters:
    // - Key size: 1024 or 2048 bit should be fine here, 4096 on the ESP might be "paranoid mode"
    //   (in generel: shorter key = faster but less secure)
    // - Distinguished name: The name of the host as used in certificates.
    //   If you want to run your own DNS, the part after CN (Common Name) should match the DNS
    //   entry pointing to your ESP32. You can try to insert an IP there, but that's not really good style.
    // - Dates for certificate validity (optional, default is 2019-2029, both included)
    //   Format is YYYYMMDDhhmmss
    int createCertResult = createSelfSignedCert(
        *_cert,
        KEYSIZE_2048,
        "CN=scoreboard.local,O=ZodCom,C=US",
        "20190112000000",
        "20300112000000"
    );

    // Now check if creating that worked
    if (createCertResult != 0) {
        dlog.error(TAG, "Cerating certificate failed. Error Code = 0x%02X, check SSLCert.hpp for details", createCertResult);
        while(true) delay(500); // TBD - no infinate loop!
    }
    dlog.info(TAG, "Creating the certificate was successful");
    if (_fs != nullptr)
    {
        writeFile(base_file_name, CRT_EXT, _cert->getCertData(), _cert->getCertLength());
        writeFile(base_file_name, KEY_EXT, _cert->getPKData(), _cert->getPKLength());
    }
    else
    {
        dlog.warn(TAG, "_fs is NULL, can't save generated cert!");
    }
    return true;
}
#endif

void WebApp::addClient(ScoreboardClient* client)
{
    dlog.info(TAG, "addClient() before size: %u", _clients.size());
    _clients.push_back(client);
    dlog.info(TAG, "addClient() after size: %u", _clients.size());
}

void WebApp::removeClient(ScoreboardClient* client)
{
    dlog.info(TAG, "removeClient() before size: %u", _clients.size());
    _clients.erase(std::remove(_clients.begin(), _clients.end(), client), _clients.end());
    dlog.info(TAG, "removeClient() after size: %u", _clients.size());
}

void WebApp::updateClients()
{
    dlog.info(TAG, "updateClients: clients: %d", _clients.size());
    App& app = App::getInstance();
    const char* mode = "UNKNOWN";
    switch (app.mode())
    {
        case AppMode::STARTING:
            mode = "STARTING";
            break;
        case AppMode::CHOOSING:
            mode = "CHOOSING";
            break;
        case AppMode::RUNNING:
            mode = "RUNNING";
            break;
        case AppMode::GAME_OVER:
            mode = "GAME_OVER";
            break;
    }
    const char* lhs_color = "red";
    const char* rhs_color = "blue";
    if (app.getTeam(Score::LHS) != Score::RED)
    {
        lhs_color = "blue";
        rhs_color = "red";
    }
    StaticJsonDocument<500> doc;
    doc["mode"] = mode;
    JsonObject lhs = doc.createNestedObject("lhs");
    lhs["color"] = lhs_color;
    lhs["score"] = app.getScore(Score::Side::LHS);
    JsonObject rhs = doc.createNestedObject("rhs");
    rhs["color"] = rhs_color;
    rhs["score"] = app.getScore(Score::Side::RHS);


    for (ScoreboardClient* client : _clients)
    {
        dlog.info(TAG, "updateClients: client: 0x%08x", client);
        if (client == nullptr)
        {
            continue;
        }
        if (client->isAdmin())
        {
            doc["group"] = "admin";
        }
        else
        {
            doc["group"] = "guest";
        }
        String data;
        data.reserve(measureJson(doc)+4);
        serializeJson(doc, data);
        data += "\n";
        dlog.info(TAG, "json: %s", data.c_str());
        client->send(data.c_str(), WebsocketHandler::SEND_TYPE_TEXT);
    }

}