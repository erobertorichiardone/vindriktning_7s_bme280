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
 
#include <Arduino.h>
#include "CommandLine.h"
#include "Settings.h"

void showHelp()
{
    Serial.println("Commands:");
    Serial.println("help                   Shows this help");
    Serial.println("reboot                 Reboots the device");
    Serial.println("printwifisettings      Prints SSID and password of the current wifi connection");
    Serial.println("connectwifi SSID pass  Attempts to connect to a new wifi network");
    Serial.println("printsettings          Prints all available settings");
    Serial.println("get config_key         Prints the value of the given config key");
    Serial.println("set config_key value   Sets the value of the given config key to the given value");
    Serial.println("factoryreset           Forgets wifi and other settings, starts as new");
}

bool isNumber(const String &string) {
    for (int i = 0; i < string.length(); ++i) {
        if (!isDigit(string[i])) {
            return false;
        }
    }
    return true;
}

void parseCommand(Tokenizer &tokenizer, WifiMQTTManager &manager)
{
    if (tokenizer.numTokens() == 1) {
        if (tokenizer[0] == "help") {
            showHelp();
        } else if (tokenizer[0] == "factoryreset") {
            manager.factoryReset();
        } else if (tokenizer[0] == "reboot") {
            ESP.reset();
        } else if (tokenizer[0] == "printsettings") {
            Settings::self()->printSettings();
        } else if (tokenizer[0] == "printwifisettings") {
            Serial.print("SSID: ");
            Serial.println(manager.getWifiSSID());
            Serial.print("Pass: ");
            Serial.println(manager.getWifiPass());
            Serial.print("IP:   ");
            Serial.println(WiFi.localIP());
        }
    } else if (tokenizer.numTokens() == 2 && tokenizer[0] == "get") {
        const String key = tokenizer[1];
        if (tokenizer[1] == "use_wifi") {
            Serial.println(Settings::self()->useWifi() ? "true" : "false");
        } else if (key == "mqtt_topic") {
            Serial.println(Settings::self()->mqttTopic());
        } else if (key == "mqtt_server") {
            Serial.println(Settings::self()->mqttServer());
        } else if (key == "mqtt_port") {
            Serial.println(Settings::self()->mqttPort());
        } else if (key == "mqtt_user_name") {
            Serial.println(Settings::self()->mqttUserName());
        } else if (key == "mqtt_password") {
            Serial.println(Settings::self()->mqttPassword());
        } else {
            Serial.print("Invalid configuration key: ");
            Serial.println(key);
        }
    } else if (tokenizer.numTokens() == 3 && tokenizer[0] == "set") {
        const String key = tokenizer[1];
        const String val = tokenizer[2];
        if (tokenizer[1] == "use_wifi") {
            if (val == "true") {
                Settings::self()->setUseWifi(true);
            } else if (val == "false") {
                Settings::self()->setUseWifi(false);
            } else {
                Serial.println("Expected: set use_wifi [true|false]");
            }
        } else if (key == "mqtt_topic") {
            Settings::self()->setMqttTopic(val);
        } else if (key == "mqtt_server") {
            Settings::self()->setMqttServer(val);
        } else if (key == "mqtt_port") {
            if (!isNumber(val)) {
                Serial.println("Expected: set mqtt_port 1-65535");
            } else {
                int port = val.toInt();
                if (port < 0 || port > 65535) {
                    Serial.println("Expected: set mqtt_port 0-65535");
                } else {
                   Settings::self()->setMqttPort(port);
                }
            }
        } else if (key == "mqtt_user_name") {
            Settings::self()->setMqttUserName(val);
        } else if (key == "mqtt_password") {
            Settings::self()->setMqttPassword(val);
        } else {
            Serial.print("Invalid configuration key: ");
            Serial.println(key);
        }
    } else if (tokenizer.numTokens() == 3 && tokenizer[0] == "connectwifi") {
        //FIXME: find a way
        const String ssid = tokenizer[1];
        const String pass = tokenizer[2];
        WiFi.persistent(true);
        bool ret = WiFi.begin(ssid.c_str(), pass.c_str(), 0, NULL, true);
        if (!ret) {
            Serial.println("[Error] wifi connection failed");
        }
        Serial.println(WiFi.isConnected());
        WiFi.persistent(false);
    } else {
        Serial.println("Syntax error, available commands are:");
        showHelp();
    }
}
