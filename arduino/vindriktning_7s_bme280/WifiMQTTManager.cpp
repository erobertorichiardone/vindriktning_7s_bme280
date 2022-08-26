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
 
#include "WifiMQTTManager.h"
#include "Settings.h"

WifiMQTTManager::WifiMQTTManager(char *captiveName)
    : m_captiveName(captiveName)
{
}

WifiMQTTManager::~WifiMQTTManager()
{}

void WifiMQTTManager::setup()
{
    Settings *s = Settings::self();
    WiFiManagerParameter mqttName("name", "Friendly Name", s->mqttTopic().c_str(), 40);
    WiFiManagerParameter mqttServer("server", "MQTT Server", s->mqttServer().c_str(), 40);
    WiFiManagerParameter mqttPort("port", "MQTT Port", String(s->mqttPort()).c_str(), 6);
    WiFiManagerParameter mqttUserName("username", "mqtt username", s->mqttUserName().c_str(), 40);
    WiFiManagerParameter mqttPassword("password", "mqtt password", s->mqttPassword().c_str(), 40);
    m_wifiManager.addParameter(&mqttName);
    m_wifiManager.addParameter(&mqttServer);
    m_wifiManager.addParameter(&mqttPort);
    m_wifiManager.addParameter(&mqttUserName);
    m_wifiManager.addParameter(&mqttPassword);

    m_wifiManager.setSaveConfigCallback([&]() {
        Settings *s = Settings::self();
        s->setMqttTopic(mqttName.getValue());
        s->setMqttServer(mqttServer.getValue());
        s->setMqttPort(max(long(0), min(long(65535), String(mqttPort.getValue()).toInt())));
        s->setMqttUserName(mqttUserName.getValue());
        s->setMqttPassword(mqttPassword.getValue());
        s->save();
    });

    // Workaround https://github.com/tzapu/WiFiManager/issues/1065

    m_wifiManager.setConnectTimeout(2);
    int attempt = 0;
    Serial.print("Connecting to wifi: ");
    Serial.print(m_wifiManager.getWiFiSSID());
    Serial.print(" ");
    Serial.println(m_wifiManager.getWiFiPass());

    bool ret = WiFi.begin(m_wifiManager.getWiFiSSID(), m_wifiManager.getWiFiPass(), 0, NULL, true);
    while(attempt < 30 && !WiFi.isConnected() && m_wifiManager.getWiFiSSID().length() > 0) {
        ++attempt;
        //bool ret = WiFi.begin(m_wifiManager.getWiFiSSID(), m_wifiManager.getWiFiPass(), 0, NULL, true);
        Serial.print(WiFi.status());
        Serial.println(".");
        delay(500);
    }

    if (!WiFi.isConnected()) {
        m_wifiManager.autoConnect(m_captiveName);
    }
    m_status = Status(m_status | Status::WifiConnected);
    Serial.println("Connected.");

   // readMQTTConfig();
}

void WifiMQTTManager::factoryReset()
{
    m_wifiManager.resetSettings();
    SPIFFS.format();
    ESP.reset();
}

String WifiMQTTManager::getWifiSSID()
{
    return m_wifiManager.getWiFiSSID();
}

String WifiMQTTManager::getWifiPass()
{
    return m_wifiManager.getWiFiPass();
}

void WifiMQTTManager::connectWifi(String ssid, String pass)
{
    m_wifiManager.setConnectTimeout(2);
    m_wifiManager.autoConnect(ssid.c_str(), pass.c_str());
}

bool WifiMQTTManager::tryPublish(const String &topic, const String &val)
{
    if (WiFi.status() != WL_CONNECTED) {
        setup();
    }

    Settings *s = Settings::self();

    if (!m_pubSubClient) {
        m_pubSubClient.reset(new PubSubClient(m_client));
    }
       

    m_pubSubClient->setServer(s->mqttServer().c_str(), s->mqttPort());
    int attempts = 0;

    while (attempts < 5 && !m_pubSubClient->connected()) {
        ++attempts;
        Serial.print("Attempting MQTT connection...");
        Serial.print(attempts);
        Serial.print(" ");
        // Attempt to connect
        m_client.connect(s->mqttServer().c_str(), s->mqttPort());
        Serial.print(s->mqttUserName().c_str());Serial.print(s->mqttPassword().c_str());
        if (m_pubSubClient->connect(topic.c_str(), s->mqttUserName().c_str(), s->mqttPassword().c_str())) {
            Serial.println("MQTT connected");
            break;
        } else {
            Serial.print("failed:");
            Serial.print(m_pubSubClient->state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }

    if (m_pubSubClient->connected()) {
        m_pubSubClient->loop();
        m_pubSubClient->publish(topic.c_str(), val.c_str(), true);
        // Seems to work better connecting and  disconnecting the client every time
        m_client.stop();
        return true;
    } else {
        Serial.println("Server not responding: giving up");
        return false;
    }
}
