#pragma once

#include <map>
#include <Stream.h>

class ConfBlk: public std::map<String, String>
{
public:
    ConfBlk(const String& fileName = "/config.json");
    void dump(Stream& s) const;
    bool writeStream(Stream& s) const;
    bool writeFile() const;
    bool readStream(Stream& s);
    bool readFile();
    void setFileName(const String& n) { _fileName = n; }
    const String& getFileName() const { return _fileName; }
    private:
    String _fileName;
};