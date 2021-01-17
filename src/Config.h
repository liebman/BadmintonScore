/*
 * Config.h
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

#ifndef CONFIG_H_
#define CONFIG_H_

#include "Arduino.h"

class Config
{
public:
    Config();
    virtual ~Config();

    bool        begin();

    bool        load();
    void        save();

    const String& getHostname();
    void          setHostname(const String& host);
    const String& getEnablePassword();
    void        setEnablePassword(const String& password);
    bool        getStartWiFi();
    void        setStartWiFi(bool start_wifi);

private:
    String _hostname;
    String _enable_password;
    bool _start_wifi;
};

#endif /* CONFIG_H_ */
