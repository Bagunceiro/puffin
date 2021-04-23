#include "wifiserial.h"

class WiFiSerialServer : public WiFiServer
{
public:
    WiFiSerialServer();
    void init(const String &a, const uint16_t port = 1685);
    void handle();

private:
    String announcement;
};

WiFiSerialServer WSerialServer;

void WiFiSerialClient::begin(const String &a)
{
    WSerialServer.init(a);
}
void WiFiSerialClient::loop()
{
    while (available()) read(); // Need to access the client to enable connectiion status update
    WSerialServer.handle();
}

WiFiSerialServer::WiFiSerialServer() : WiFiServer(1685)
{
}

void WiFiSerialServer::init(const String &a, const uint16_t port)
{
    begin(port);
    setNoDelay(true);
    announcement = a;
}

void WiFiSerialServer::handle()
{
    if (hasClient())
    {
        Serial.println("New telnet client");
        if (!serr.connected())
        {
            serr.stop();
            serr = available();
            serr.println(announcement.c_str());
        }
        else
        {
            Serial.println("Telnet client already connected");
            available().stop();
        }
    }
}

size_t WiFiSerialClient::write(uint8_t c)
{
    if (connected()) WiFiClient::write(c);
    return Serial.write(c);
}
size_t WiFiSerialClient::write(const uint8_t *buf, size_t size)
{
    if (connected()) WiFiClient::write(buf, size);
    return Serial.write(buf, size);
}
