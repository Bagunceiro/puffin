#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
#include <ESP8266httpUpdate.h>
#include <Wire.h>
#include <PZEM004Tv30.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

#include "framebuff.h"

#include "menu.h"

#define serr Serial

PZEM004Tv30 pzem(14, 12);
extern LiquidCrystal_I2C lcd;

WiFiClient mqttWifiClient;
PubSubClient mqttClient(mqttWifiClient);

int wifiattemptcount = 0;
const int WIFI_CONNECT_ATTEMPT_PAUSE = 15000;

FrameBuffer fb(20,10);

void wpsInit()
{
    /*
    esp_wps_config_t wpsconfig;

    wpsconfig.crypto_funcs = &g_wifi_default_wps_crypto_funcs;
    wpsconfig.wps_type = WPS_TYPE_PBC;
    strcpy(wpsconfig.factory_info.manufacturer, "PA");
    strcpy(wpsconfig.factory_info.model_number, "1");
    strcpy(wpsconfig.factory_info.model_name, "Conchita");
    strcpy(wpsconfig.factory_info.device_name, "ESP32");
    esp_wifi_wps_enable(&wpsconfig);
    esp_wifi_wps_start(0);
    */
}

bool mqttinit()
{
    static unsigned long lastAttempt = 0;
    unsigned long now = millis();
    bool result = false;

    if ((lastAttempt == 0) || ((now - lastAttempt) > 10000))
    {
        serr.print("Connecting to MQTT ");
        lastAttempt = now;

        String clientID = String("PowerMeter_") + String(millis() % 1000);

        mqttClient.setServer("192.168.0.101", 1883);

        if (mqttClient.connect(clientID.c_str(),
                               "ctlr",
                               "fatty"))
        {
            serr.println("MQTT connected");
            result = true;
        }
        else
        {
            serr.print("Failed: ");
            serr.println(mqttClient.state());
        }
    }
    return result;
}

static bool mqttConnected = false;

bool mqttpoll()
{
    bool result = true;

    if (!mqttClient.loop())
    {
        if (mqttConnected)
        {
            serr.println("Lost MQTT Connection");
            result = false;
        }
        if (mqttinit())
        {
            serr.println("MQTT Connected");
        }
    }
    mqttConnected = result;
    return result;
}

const char antenna[8] =
{
    0b10101,
    0b10101,
    0b01110,
    0b00100,
    0b00100,
    0b00100,
    0b00000,
    0b00000
};

extern void checkMenu();
extern MenuEntry menuRoot;

void setup()
{
    Serial.begin(9600);
    Wire.begin(4, 5);
    lcd.init();
    lcd.backlight();

    WiFi.mode(WIFI_STA);

    lcd.createChar(0, antenna);

    Screen::build();
    menuRoot.buildmenu();
    menuRoot.dump();
    lcd.display();
}

long sigFigs(const long val, unsigned int numFigs)
{
    char buffer[16];
    sprintf(buffer, "%ld", val);

    for (uint8_t i = numFigs + 1; i < strlen(buffer); i++)
    {
        buffer[i] = '0';
    }
    long result;
    sscanf(buffer, "%ld", &result);
    return result;
}

void blankField(const int col, const int row, const int length)
{
    for (int i = 0; i < length; i++) lcd.print(' ');
}

void displayField(const int col, const int row, const int length, float value, uint8_t dp, const char *suffix)
{
    fb.setCursor(col, row);

    char buffer[length + 1];
    snprintf(buffer, length, "%%.%df%%s", dp);
    // Serial.println(buffer);

    char buffer2[length + 1];
    snprintf(buffer2, length, buffer, value, suffix);
    // Serial.println(buffer2);
    fb.print(buffer2);
    for (int i = strlen(buffer2); i < length; i++) fb.print(" ");
}

void loop()
{
    bool wifiConnected = false;

    if (WiFi.status() == WL_CONNECTED)
    {
        wifiConnected = true;
        mqttpoll();
    }
    else
    {
        static unsigned long then = millis();
        unsigned long sinceThen = millis() - then;
        if (sinceThen > 5000)
        {
            WiFi.begin("asgard_2g", "enaLkraP");
            then = millis();
        }
    }

    int vScale = 1;
    int aScale = 1000;
    int wScale = 1000;
    int kwhScale = 1000;
    int hzScale = 10;
    int pfScale = 100;

    float vf = pzem.voltage();
    float af = pzem.current();
    float wf = pzem.power();
    float kwhf = pzem.energy();
    float hzf = pzem.frequency();
    float pff = pzem.pf();

    int voltage = (isnan(vf) ? 0 : vf * vScale);
    float v = ((float)sigFigs(voltage, 3)) / vScale;
    
    int current = (isnan(af) ? 0 : af * aScale);
    float a = ((float)sigFigs(current, 3)) / aScale;
    
    int power = (isnan(af) ? 0 : wf * wScale);
    float w = ((float)sigFigs(power, 3)) / wScale;

    int energy = (isnan(af) ? 0 : kwhf * kwhScale);
    float kwh = ((float)sigFigs(energy, 3)) / kwhScale;

    int frequency = (isnan(hzf) ? 0 : hzf * hzScale);
    float hz = ((float)sigFigs(frequency, 2)) / hzScale;
    
    int powerFactor = (isnan(pff) ? 0 : pff * pfScale);
    float pf = ((float)sigFigs(powerFactor, 2)) / pfScale;

    StaticJsonDocument<512> doc;
    doc["V"] = v;
    doc["A"] = a;
    doc["W"] = w;
    doc["kWh"] = kwh;
    doc["Hz"] = hz;
    doc["PF"] = pf;
    String s;
    serializeJson(doc, s);
    // Serial.println(s);
    mqttClient.publish("powermeter/data", s.c_str());

    const char *asuff = " A";
    uint8_t adp = 3;
    if (a < 1000)
    {
        a *= 1000;
        asuff = " mA";
        adp = 0;
    }
    const char *kwhsuff = " kWh";
    uint8_t kwhdp = 3;
    if (kwh < 1000)
    {
        kwh *= 1000;
        kwhsuff = " Wh";
        kwhdp = 0;
    }
    const char *wsuff = " W";
    uint8_t wdp = 3;
    if (w < 1000)
    {
        w *= 1000;
        wsuff = " mW";
        wdp = 0;
    }

    displayField(0, 1, 10, v, 1, "V");
    displayField(0, 2, 10, a, adp, asuff);
    displayField(0, 3, 10, w, wdp, wsuff);
    displayField(10, 1, 10, hz, 1, " Hz");
    displayField(10, 2, 10, pf, 2, " PF");
    displayField(10, 3, 10, kwh, kwhdp, kwhsuff);


    fb.setCursor(18,0);
    fb.write(mqttConnected ? 'M' : ' ');
    fb.write(wifiConnected ? byte(0) : ' ');
    fb.display(lcd);
    delay(1000);
}
