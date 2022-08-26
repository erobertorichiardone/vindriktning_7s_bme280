/*
 *   Copyright 2022 Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
 
#pragma once

#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <PubSubClient.h>

class WifiMQTTManager {
public:
    enum Status {
        None = 0,
        WifiHasCredentials = 1,
        WifiConnected = 1 << 1,
        MQTTHasCredentials = 1 << 2,
        MQTTConnected = 1 << 3
    };
    enum class Error {
        NoError = 0,
        WifiError = 1,
        MQTTError = 1 << 1,
        SPIFFSError = 1 << 2
    };
    WifiMQTTManager(char *captiveName);
    ~WifiMQTTManager();

    void setup();
    void factoryReset();

    String getWifiSSID();
    String getWifiPass();

    void connectWifi(String ssid, String pass);

    bool tryPublish(const String &topic, const String &val);

protected:
    void saveMQTTConfig();
    void readMQTTConfig();

private:
    char *m_captiveName;

    WiFiManager m_wifiManager;
    WiFiClient m_client;
    std::shared_ptr<PubSubClient> m_pubSubClient;
    Status m_status = Status::None;
    Error m_error = Error::NoError;
};
