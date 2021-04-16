#include "confblk.h"
#include <ArduinoJson.h>
#include "LittleFS.h"

ConfBlk::ConfBlk(const String &fileName)
{
    _fileName = fileName;
}

void ConfBlk::dump(Stream &s) const
{
    for (auto iterator : *this)
    {
        s.printf("%s  = %s\n", iterator.first.c_str(), iterator.second.c_str());
    }
}

bool ConfBlk::writeStream(Stream &s) const
{
    StaticJsonDocument<512> doc;
    for (auto iterator : *this)
    {
        doc[iterator.first] = iterator.second;
    }
    serializeJson(doc, s);

    return true;
}

bool ConfBlk::writeFile() const
{
    bool result = false;

    File configFile = LittleFS.open(_fileName, "w");
    if (!configFile)
    {
        perror("");
        Serial.println("Config file open for write failed");
        result = false;
    }
    else
    {
        writeStream(configFile);
        configFile.close();
        result = true;
    }

    return result;
}

bool ConfBlk::readStream(Stream &s)
{
    bool result = false;
    StaticJsonDocument<512> doc;

    DeserializationError error = deserializeJson(doc, s);
    if (error)
    {
        Serial.printf("Config deserialization error (%d)\n", error.code());
        result = false;
    }
    else
    {
        JsonObject root = doc.as<JsonObject>();
        for (JsonPair kv : root)
        {
            (*this)[kv.key().c_str()] = (const char *)kv.value();
        }
        result = true;
    }
    return result;
}

bool ConfBlk::readFile()
{
    bool result = false;

    File configFile = LittleFS.open(_fileName, "r");
    if (!configFile)
    {
        Serial.println("Config file open for read failed");
    }
    else
    {
        result = readStream(configFile);
        configFile.close();
        result = true;
    }

    return result;
}