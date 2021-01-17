/**
 * @file WebApp.h
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
#ifndef WEB_APP_H_
#define WEB_APP_H_

#include <HTTPSServer.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>
#include <FS.h>
#include "App.h"
#include <map>
#include <vector>
#include "Config.h"
#include "ScoreboardClient.h"
#include "TaskGateway.h"

#ifndef MAX_SCOREBOARD_CLIENTS
#define MAX_SCOREBOARD_CLIENTS 8
#endif

#define APP_NAME "scoreboard"
using namespace httpsserver;

typedef struct wa_session {
    std::string user;
    bool        admin;
    uint32_t    accessed;
} WebAppSession;

class WebApp
{
public:
    static WebApp& getInstance();
    bool loadCert(const char* cert_file_name);
    bool begin(Config* config, FS* fs, const char* certname = nullptr);
    void addClient(ScoreboardClient* client);
    void removeClient(ScoreboardClient* client);
    void updateClients();

    Config* getConfig();
    FS*     getFS();

private:
    Config* _config;
    FS* _fs;
    SSLCert * _cert;
    HTTPServer * _server;
    std::vector<ScoreboardClient*> _clients;

    WebApp();
    bool start();
    bool loadCertFromFile(const char* cert_file_name);
    bool generateCert(const char* cert_file_name);
    bool fileExists(const char* cert_file_name, const char* ext);
    bool writeFile(const char* base_name, const char* ext, uint8_t* data, size_t len);
    bool readFile(const char* base_name, const char* ext, uint8_t** data, uint16_t* len);
    void task();
    friend void taskGateway<WebApp>(void* data);
};

#endif // WEB_APP_H_
