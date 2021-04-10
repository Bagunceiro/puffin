#include <ArduinoJson.h>
#include "menu.h"

LiquidCrystal_I2C lcd(0x27, 20, 4);

ScreenList screens;
MenuEntry menuRoot;

#include "screensjson.h"
#include "menujson.h"

void Screen::build()
{
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, screenjson);
  if (error)
  {
    Serial.println("screens desc deserialisation error");
  }
  else
  {
    JsonObject root = doc.as<JsonObject>();
    JsonArray arrayOfScreens = root["screen"].as<JsonArray>();
    for (JsonObject screenDesc : arrayOfScreens)
    {
      std::pair<ScreenList::iterator, bool> ret;
      Screen tempsc;
      ret = screens.insert({(const char *)screenDesc["name"], tempsc});

      if (ret.second == false)
      {
        Serial.printf("Duplicate Screen name %s\n", (const char *)screenDesc["name"]);
      }
      else
      {
        Screen &sc = screens[screenDesc["name"]];

        JsonArray arrayOfFields = screenDesc["field"].as<JsonArray>();
        for (JsonObject fieldDesc : arrayOfFields)
        {
          ScreenField sf((const uint8_t)fieldDesc["x"], (const uint8_t)fieldDesc["y"], (const uint8_t)fieldDesc["l"]);
          sc.addField((const char *)fieldDesc["name"], sf);
        }
      }
    }
  }
  Serial.println("---- Sceen List Built ----");
}

MenuEntry::MenuEntry()
{
    name[0] = '\0';
}

void MenuEntry::build(JsonObject& obj)
{
  static int level = 0;
  level++;
  if (obj["menu"])
  {
    // Serial.println("It's a menu");
    JsonArray menuArray = obj["menu"].as<JsonArray>();
    for (JsonObject item : menuArray)
    {
      // serializeJson(item, Serial);
      // Serial.println();

      if (item["n"])
      {
        for (int i = 0; i < level; i++) Serial.print("  ");
        Serial.printf("%s\n", (const char*)item["n"]);
      }
      else
      {
        Serial.println("No name");
      }
      MenuEntry m;
      strncpy(m.name, (const char*)item["n"], sizeof(m.name)-1);
      m.build(item);
      entries.push_back(m);
    }
  }
  else if (obj["type"])
  {
    strncpy(type, (const char*)obj["type"], sizeof(type) -1);
    for (int i = 0; i < level; i++) Serial.print("  ");
    Serial.printf("Leaf: type is %s\n", (const char*)obj["type"]);
  }
  else
  {
    for (int i = 0; i < level; i++) Serial.print("  ");
    Serial.println("Don't know what to do with it");
  }
  level--;
}

void MenuEntry::buildmenu()
{
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, menujson);
  if (error)
  {
    Serial.println("menu desc deserialisation error");
    Serial.println(error.c_str());
  }
  JsonObject root = doc.as<JsonObject>();
  menuRoot.build(root);
}

void Screen::dump()
{
  ScreenFieldList::iterator it;
  for (it = fields.begin(); it != fields.end(); ++it)
  {
    Serial.printf(" Field %s\n", (const char *)it->first);
    ((ScreenField &)it->second).dump();
  }
}

void checkScreens()
{
  ScreenList::iterator it;
  for (it = screens.begin(); it != screens.end(); ++it)
  {
    Serial.printf("Screen %s\n", (const char *)it->first);
    ((Screen &)it->second).dump();
  }
}

void MenuEntry::dump(int level)
{
  for (int i = 0; i < level; i++) Serial.print(" ");
  Serial.printf("%s:%s\n", name, type);
  Menu::iterator it;
  for (MenuEntry&m : entries)
  {
    m.dump(level + 1);
  }
}


ScreenField::ScreenField(const uint8_t _x, const uint8_t _y, const uint8_t _l)
{
  x = _x;
  y = _y;
  length = _l;
}

ScreenField::ScreenField()
{
  x = 0;
  y = 0;
  length = 0;
}

ScreenField::~ScreenField()
{
}

int ScreenField::display(const char *val)
{
  char buff[length + 1];
  int l = strlen(val);

  strncpy(buff, val, length);
  lcd.setCursor(x, y);
  lcd.print(buff);
  for (int i = l; i < length; i++)
  {
    lcd.print(' ');
  }
  return l;
}

Screen::Screen(const uint8_t _cols, const uint8_t _rows)
{
    cols = _cols;
    rows = _rows;
}

Screen::~Screen()
{
}

Screen::Screen(const Screen &rhs)
{
  // fields = rhs.fields;
  *this = rhs;
}

Screen &Screen::operator=(const Screen &rhs)
{
  rows = rhs.rows;
  cols = rhs.cols;
  fields = rhs.fields;
  return *this;
}

bool ScreenField::overlaps(const ScreenField &field) const
{
  if (y == field.y)
  {
    if ((start() >= field.start() && start() <= field.end()) || (end() >= field.start() && end() <= field.end()))
    {
      return true;
    }
  }
  return false;
}

bool Screen::addField(const char *fieldname, const ScreenField &newfield)
{
  for (auto const &f : fields)
  {
    if (newfield.overlaps(f.second))
    {
      Serial.printf("%s overlaps with %s\n", fieldname, f.first);
    }
  }
  fields[fieldname] = newfield;
  return true;
}

int Screen::display(const char *fieldname, const char *val)
{
  auto search = fields.find(fieldname);
  if (search != fields.end())
  {
    ScreenField &f = search->second;
    return f.display(val);
  }
  return -1;
}

void MenuEntry::output(FrameBuffer& fb)
{
  fb.clear();
  fb.setTitle(name);
  for (unsigned int i = 0; i < entries.size(); i++)
  {
    fb.print(' ');
    fb.print(entries[i].name);
    fb.print('\n');
  }
  fb.display(lcd);
}
