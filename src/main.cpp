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
#include <TaskScheduler.h>
#include <LittleFS.h>
#include <ESP8266httpUpdate.h>
#include <TZ.h>
#include <coredecls.h>

#include "config.h"
#include "framebuff.h"
#include "analogkeypad.h"
#include "menu.h"

#define serr Serial

bool testmode = false;
;

LiquidCrystal_I2C lcd(0x27, 20, 4);
BreadCrumb navigation;

PZEM004Tv30 pzem(14, 12);
extern LiquidCrystal_I2C lcd;
extern void wssetup();
extern AsyncEventSource events;
bool wifiConnected = false;
bool wasWiFiConnected = false;
ConfBlk conf("puffin.json");

WiFiClient mqttWifiClient;
PubSubClient mqttClient(mqttWifiClient);

int wifiattemptcount = 0;
const int WIFI_CONNECT_ATTEMPT_PAUSE = 15000;

Scheduler ts;
void doMeasurements();
void kpadloop();
void wifiloop();
void mqttloop();
void menuloop();
Task measurement_task(1000, TASK_FOREVER, &doMeasurements, &ts, true);
Task kpad_task(20, TASK_FOREVER, &kpadloop, &ts, true);
Task wifi_task(5000, TASK_FOREVER, &wifiloop, &ts, true);
Task mqtt_task(500, TASK_FOREVER, &mqttloop, &ts, true);

FrameBuffer fb(20, 10);

const char *version = __TIME__;

void reportProgress(size_t completed, size_t total)
{
    static int oldPhase = 1;
    int progress = (completed * 100) / total;

    int phase = (progress / 5) % 2; // report at 5% intervals

    if (phase != oldPhase)
    {
        char buffer[8];
        snprintf(buffer, 8, "%3d%%", progress);
        Serial.printf("%3d%% (%d/%d)\n", progress, completed, total);
        fb.writeField(5, 2, 15, buffer);
        fb.display(lcd);
        oldPhase = phase;
    }
}

t_httpUpdate_return systemUpdate(const String &url)
{
    WiFiClient httpclient;
    char buffer[22];

    measurement_task.disable();

    ESPhttpUpdate.rebootOnUpdate(false);

    Update.onProgress(reportProgress);

    fb.clear();
    fb.setTitle("System Updating");

    t_httpUpdate_return ret = ESPhttpUpdate.update(httpclient, url);

    switch (ret)
    {
    case HTTP_UPDATE_FAILED:
        Serial.printf("Update fail error (%d): %s\n",
                      ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        snprintf(buffer, 20, "Failed(%d)", ESPhttpUpdate.getLastError());
        fb.writeField(0, 2, 20, buffer);
        break;

    case HTTP_UPDATE_NO_UPDATES:
        Serial.println("No update file available");
        fb.writeField(0, 2, 20, "No update available");
        break;

    case HTTP_UPDATE_OK:
        Serial.println("System updated");
        fb.setTitle("Reseting");
        fb.writeField(4, 2, 20, "Please Wait");
        fb.display(lcd);
        ESP.restart();
        break;
    }
    fb.display(lcd);
    return ret;
}

bool mqttinit()
{
    static unsigned long lastAttempt = 0;
    unsigned long now = millis();
    bool result = false;

    if ((lastAttempt == 0) || ((now - lastAttempt) > 10000))
    {
        serr.println("Connecting to MQTT");
        lastAttempt = now;

        String clientID = String("Puffin_") + String(millis() % 1000);

        mqttClient.setServer(conf[mqttbroker_n].c_str(), 1883);

        if (mqttClient.connect(clientID.c_str(),
                               "ctlr",
                               "fatty"))
        {
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

void mqttloop()
{
    if (wifiConnected)
    {
        if (!mqttClient.loop())
        {
            if (mqttConnected)
            {
                serr.println("Lost MQTT Connection");
                mqttConnected = false;
            }
            if (mqttinit())
            {
                serr.println("MQTT Connected");
                mqttConnected = true;
            }
        }
    }
    else mqttConnected = false;
}

const char block[8] =
    {
        0b11111,
        0b11111,
        0b11111,
        0b11111,
        0b11111,
        0b11111,
        0b11111,
        0b11111};
const char antenna[8] =
    {
        0b10101,
        0b10101,
        0b01110,
        0b00100,
        0b00100,
        0b00100,
        0b00100,
        0b00000};
const char working1[8] =
    {
        0b11111,
        0b11111,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000};
const char working2[8] =
    {
        0b00000,
        0b00000,
        0b11111,
        0b11111,
        0b00000,
        0b00000,
        0b00000,
        0b00000};
const char working3[8] =
    {
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b11111,
        0b11111,
        0b00000,
        0b00000};
const char working4[8] =
    {
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b11111,
        0b11111};

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

void displayStatus()
{
    static int indic = 0;
    switch (indic)
    {
    case 0:
        lcd.createChar(1, working1);
        break;
    case 1:
        lcd.createChar(1, working2);
        break;
    case 2:
        lcd.createChar(1, working3);
        break;
    case 3:
        lcd.createChar(1, working4);
        break;
    }
    if (++indic >= 4)
        indic = 0;
    fb.setCursor(17, 0);
    fb.write(mqttConnected ? 'M' : ' ');
    fb.write(wifiConnected ? byte(0) : ' ');
    fb.write(1);
}

void doMeasurements()
{
    float v;
    float a;
    float hz;
    float pf;

    if (!testmode)
    {
        v = pzem.voltage();
        a = pzem.current();
        hz = pzem.frequency();
        pf = pzem.pf();
    }
    else
    {
        v = (1295 + random(10)) / 10.0;
        a = (800.0 + random(300)) / 1000;
        pf = (85 + random(10)) / 100.0;
        hz = (590 + random(10)) / 10.0;
    }

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

    bool onMainScreen = navigation.empty();

    char ds[12];
    const char *fmt = "%6s %s";
    snprintf(ds, 10, fmt, v_s, "V");
    if (onMainScreen)
        fb.writeField(0, 1, 10, ds);
    events.send(ds, "voltage", millis());
    snprintf(ds, 10, fmt, a_s, asuf);
    if (onMainScreen)
        fb.writeField(0, 2, 10, ds);
    events.send(ds, "current", millis());
    snprintf(ds, 10, fmt, w_s, wsuf);
    if (onMainScreen)
        fb.writeField(0, 3, 10, ds);
    events.send(ds, "power", millis());
    snprintf(ds, 10, fmt, hz_s, "Hz");
    if (onMainScreen)
        fb.writeField(10, 1, 10, ds);
    events.send(ds, "frequency", millis());
    snprintf(ds, 10, fmt, pf_s, "PF");
    if (onMainScreen)
        fb.writeField(10, 2, 10, ds);
    events.send(ds, "power factor", millis());
    snprintf(ds, 10, fmt, va_s, "VA");
    if (onMainScreen)
        fb.writeField(10, 3, 10, ds);
    events.send(ds, "ap power", millis());
    displayStatus();
    fb.display(lcd);
}

void keyup(uint8_t k, unsigned long);
void keydown(uint8_t k);
bool keytick(uint8_t k, unsigned long);

AnalogKeyPad kpad(keydown, keyup, keytick);

void kpadloop()
{
    kpad.poll();
}

void wifiloop()
{
    if (conf[ssid_n].length() > 0)
    {
        wifiConnected = false;

        if (WiFi.status() == WL_CONNECTED)
        {
            wifiConnected = true;
            if (!wasWiFiConnected)
            {
                wasWiFiConnected = true;
                Serial.printf("WiFi Connected - IP %s\n", WiFi.localIP().toString().c_str());
                Serial.printf("Connected to %s/%s\n", WiFi.SSID().c_str(), WiFi.psk().c_str());
            }
            // mqttpoll();
        }
        else
        {
            static unsigned long then = 0;
            unsigned long sinceThen = millis() - then;
            if ((sinceThen > 5000) || (then == 0))
            {
                WiFi.begin(conf[ssid_n], conf[psk_n]);
                then = millis();
            }
            if (wasWiFiConnected)
            {
                wasWiFiConnected = false;
                Serial.printf("WiFi Connection lost\n");
            }
        }
    }
}

void displayCharset(int set)
{
    fb.clear();
    int start = set * 64;
    for (int r = 0; r < 4; r++)
    {
        fb.setCursor(0, r);
        for (int i = 0; i < 16; i++)
        {
            fb.write(byte(start + (r * 16) + i));
        }
    }
    fb.display(lcd);
}

void leafHandler(const char *key)
{
    Serial.printf("Leaf %s\n", key);
    if (strcmp(key, "update") == 0)
    {
        systemUpdate("http://192.168.0.101/bin/puffin.bin");
    }
    if (strcmp(key, "conwifi") == 0)
    {
        fb.clear();
        fb.setCursor(0,2);
        fb.setTitle("Connecting to WiFi");
        wifi_task.enable();
    }
    if (strcmp(key, "diswifi") == 0)
    {
        WiFi.disconnect();
        fb.clear();
        fb.setCursor(0,2);
        fb.print("WiFi disconnected");
        wifiConnected = false;
        wifi_task.disable();      
    }
    if (strcmp(key, "reset") == 0)
    {
        ESP.restart();
    }
    if (strcmp(key, "char1") == 0)
    {
        displayCharset(0);
    }
    else if (strcmp(key, "char2") == 0)
    {
        displayCharset(1);
    }
    if (strcmp(key, "char3") == 0)
    {
        displayCharset(2);
    }
    else if (strcmp(key, "char4") == 0)
    {
        displayCharset(3);
    }
}

void setup()
{
    Serial.begin(9600);
    configTime(TZ_America_Sao_Paulo, "pool.ntp.org");
    LittleFS.begin();
    if (conf.readFile())
    {
        conf.dump(Serial);
    }
    Wire.begin(5, 4);
    lcd.init();
    lcd.backlight();
    WiFi.mode(WIFI_STA);

    lcd.createChar(129, block);
    lcd.createChar(0, antenna);

    menuRoot.setLeafCallback(leafHandler);
    menuRoot.buildmenu();
    lcd.display();
    wssetup();
    fb.onClear(displayStatus);
}

void loop()
{
    ts.execute();
}