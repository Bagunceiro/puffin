#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
#include <ESP8266httpUpdate.h>
#include <Wire.h>
#include <PZEM004Tv30.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "framebuff.h"
#include "analogkeypad.h"
#include "timedevent.h"

#include "menu.h"

#define serr Serial

PZEM004Tv30 pzem(14, 12);
extern LiquidCrystal_I2C lcd;
extern void wssetup();
extern AsyncEventSource events;
bool wifiConnected = false;

WiFiClient mqttWifiClient;
PubSubClient mqttClient(mqttWifiClient);

int wifiattemptcount = 0;
const int WIFI_CONNECT_ATTEMPT_PAUSE = 15000;

FrameBuffer fb(20, 10);

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
        0b00000};

extern void checkMenu();
extern MenuEntry menuRoot;

void displayField(const int col, const int row, const int length, const char *value)
{
    fb.setCursor(col, row);

    int l = strlen(value);
    for (int i = 0; i < length; i++)
    {
        if (i < l)
        {
            fb.print(value[i]);
        }
        else
            fb.print(' ');
    }
}

char *sigfigs(const float val, const int figs, char *buffer)
{
    int oom;
    if (val == 0)
        oom = 0;
    else
        oom = floorf(log10f(val));
    int ndp = figs - oom - 1;
    float scale = powf(10, ndp);
    float result = roundf(val * scale) / scale;
    if (ndp < 0)
        ndp = 0;
    char formatstr[6];
    sprintf(formatstr, "%%.%df", ndp);
    sprintf(buffer, formatstr, result);
    return buffer;
}

void doMeasurements(void *)
{
    // pzem.updateValues();

    float v = pzem.voltage();
    float a = pzem.current();
    float hz = pzem.frequency();
    float pf = pzem.pf();

    // *************
    // Test Values
    //v  = (1295 + random(10)) / 10.0;
    a  = (800.0 + random(300)) / 1000;
    pf = (85 + random(10)) / 100.0;
    //hz = (590 + random(10)) / 10.0;

    // *************

    float w = v * a * pf;
    float va = v * a;

    if (isnan(v))
        v = 0;
    if (isnan(a))
        a = 0;
    if (isnan(w))
        w = 0;
    if (isnan(hz))
        hz = 0;
    if (isnan(pf))
        pf = 1;
    if (isnan(va))
        va = 0;

    char v_s[8];
    char a_s[8];
    char w_s[8];
    char hz_s[8];
    char pf_s[8];
    char va_s[8];

    sigfigs(v, 4, v_s);
    sigfigs(a, 3, a_s);
    sigfigs(w, 3, w_s);
    sigfigs(pf, 2, pf_s);
    sigfigs(hz, 3, hz_s);
    sigfigs(va, 3, va_s);


    StaticJsonDocument<512> doc;
    doc["V"] = v_s;
    doc["A"] = a_s;
    doc["W"] = w_s;
    doc["VA"] = va_s;
    doc["Hz"] = hz_s;
    doc["PF"] = pf_s;
    String s;
    serializeJson(doc, s);
    mqttClient.publish("powermeter/data", s.c_str());
    events.send("ping", NULL, millis());

    char const *asuf = "A";
    if (a < 1)
    {
        sigfigs(a * 1000, 3, a_s);
        asuf = "mA";
    }
    char const *wsuf = "W";
    if (w < 1)
    {
        sigfigs(w * 1000, 3, w_s);
        wsuf = "mW";
    }

    char ds[12];
    const char *fmt = "%6s %s";
    snprintf(ds, 10, fmt, v_s, "V");
    displayField(0, 1, 10, ds);
    events.send(ds, "voltage", millis());
    snprintf(ds, 10, fmt, a_s, asuf);
    displayField(0, 2, 10, ds);
    events.send(ds, "current", millis());
    snprintf(ds, 10, fmt, w_s, wsuf);
    displayField(0, 3, 10, ds);
    events.send(ds, "power", millis());
    snprintf(ds, 10, fmt, hz_s, "Hz");
    displayField(10, 1, 10, ds);
    events.send(ds, "frequency", millis());
    snprintf(ds, 10, fmt, pf_s, "PF");
    displayField(10, 2, 10, ds);
    events.send(ds, "power factor", millis());
    snprintf(ds, 10, fmt, va_s, "VA");
    displayField(10, 3, 10, ds);
    events.send(ds, "ap power", millis());

    fb.setCursor(18, 0);
    fb.write(mqttConnected ? 'M' : ' ');
    fb.write(wifiConnected ? byte(0) : ' ');
    fb.display(lcd);
}

void keyup(uint8_t k)
{
    Serial.printf("Key %d released\n", k);
}

void keydown(uint8_t k)
{
    Serial.printf("Key %d pressed\n", k);
}

AnalogKeyPad kpad(keydown, keyup);

void kpadloop(void *)
{
    kpad.poll();
}

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
    wssetup();
    TimedEvent t("measurements", doMeasurements, 1000);
    t.start();
    TimedEvent t2("kpad", kpadloop, 20);
    t2.start();
}

/*
void blankField(const int col, const int row, const int length)
{
    for (int i = 0; i < length; i++)
        lcd.print(' ');
}
*/

void loop()
{
    wifiConnected = false;
    static bool wasWiFiConnected = false;

    if (WiFi.status() == WL_CONNECTED)
    {
        wifiConnected = true;
        if (!wasWiFiConnected)
        {
            wasWiFiConnected = true;
            Serial.printf("WiFi Connected - IP %s\n", WiFi.localIP().toString().c_str());
        }
        mqttpoll();
    }
    else
    {
        static unsigned long then = 0;
        unsigned long sinceThen = millis() - then;
        if ((sinceThen > 5000) || (then == 0))
        {
            WiFi.begin("asgard_2g", "enaLkraP");
            then = millis();
        }
        if (wasWiFiConnected)
        {
            wasWiFiConnected = false;
            Serial.printf("WiFi Connection lost\n");
        }
    }
    TimedEvent::poll();
}