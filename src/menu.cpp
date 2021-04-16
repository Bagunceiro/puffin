#include <ArduinoJson.h>
#include "menu.h"
#include "analogkeypad.h"
#include "framebuff.h"
#include "config.h"

LiquidCrystal_I2C lcd(0x27, 20, 4);

ScreenList screens;
MenuEntry menuRoot;

#include "screensjson.h"
#include "menujson.h"

extern BreadCrumb navigation;
extern FrameBuffer fb;

typedef const char *charset;
struct keyboard
{
  charset sets[4];
  int nsets;
};

void menuAdd(MenuEntry *m)
{
  m->reset();
  navigation.push(m);
}

void menuRemove()
{
  navigation.pop();
  if (!navigation.empty())
  {
    MenuEntry *m = navigation.top();
    m->reset();
  }
}

charset ALPHA_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
charset alpha_chars = "abcdefghijklmnopqrstuvwxyz";
charset num_chars = "0123456789";
charset punc_chars = " !\"#$%&'()*+,-./:;<=>?[\\]^_{|}~";

keyboard textkeyboard = {{ALPHA_chars, alpha_chars, num_chars, punc_chars}, 4};
keyboard numkeyboard = {{num_chars}, 1};

void dumpScreens()
{
  for (auto it : screens)
  {
    Serial.printf("%s\n", it.first);
    it.second.dump();
  }
}

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
  dumpScreens();
  Serial.println("---- ^^^^^^^^^^^^^^^^ ----");
}

MenuEntry::MenuEntry()
{
  name[0] = '\0';
  selected = 0;
  startDisplayAt = 0;
  pos = 0;
  content = "testing";
  setType(MISC_TYPE);
}

void MenuEntry::build(JsonObject &obj)
{
  static int level = 0;
  level++;
  setType(MENU_TYPE);
  if (obj["n"])
  {
    for (int i = 0; i < level; i++)
      Serial.print("  ");
    Serial.printf("%s\n", (const char *)obj["n"]);
    strncpy(name, (const char *)obj["n"], sizeof(name) - 1);
  }
  else
  {
    Serial.println("No name");
  }

  if (obj["menu"])
  {
    // Serial.println("It's a menu");
    JsonArray menuArray = obj["menu"].as<JsonArray>();
    for (JsonObject item : menuArray)
    {
      MenuEntry m;

      m.build(item);
      entries.push_back(m);
    }
  }
  else if (obj["type"])
  {
    const char *t = obj["type"];
    if (strcmp(t, "text") == 0)
      setType(TEXT_TYPE);
    else if (strcmp(t, "num") == 0)
      setType(NUM_TYPE);
    else if (strcmp(t, "check") == 0)
      setType(NUM_TYPE);
    // strncpy(type, (const char *)obj["type"], sizeof(type) - 1);
    for (int i = 0; i < level; i++)
      Serial.print("  ");
    Serial.printf("Leaf: type is %s\n", (const char *)obj["type"]);
  }
  else
  {
    for (int i = 0; i < level; i++)
      Serial.print("  ");
    Serial.println("Don't know what to do with it");
  }
  level--;
}
/*
{
  static int level = 0;
  level++;
  
  if (obj["menu"])
  {
    // Serial.println("It's a menu");
    JsonArray menuArray = obj["menu"].as<JsonArray>();
    for (JsonObject item : menuArray)
    {
      if (item["n"])
      {
        for (int i = 0; i < level; i++)
          Serial.print("  ");
        Serial.printf("%s\n", (const char *)item["n"]);
      }
      else
      {
        Serial.println("No name");
      }
      MenuEntry m;
      m.setType(MENU_TYPE);
      strncpy(m.name, (const char *)item["n"], sizeof(m.name) - 1);
      m.build(item);
      entries.push_back(m);
    }
  }
  else if (obj["type"])
  {
    setType(LEAF_TYPE);
    // strncpy(type, (const char *)obj["type"], sizeof(type) - 1);
    for (int i = 0; i < level; i++)
      Serial.print("  ");
    Serial.printf("Leaf: type is %s\n", (const char *)obj["type"]);
  }
  else
  {
    for (int i = 0; i < level; i++)
      Serial.print("  ");
    Serial.println("Don't know what to do with it");
  }
  level--;
}
*/

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
  menuRoot.dump();
}

void MenuEntry::reset()
{
  selected = 0;
  content = conf[name];
  pos = 0;
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

void MenuEntry::dump(int level)
{
  for (int i = 0; i < level; i++)
    Serial.print(" ");
  Serial.printf("%s:%d\n", name, type);
  Menu::iterator it;
  for (MenuEntry &m : entries)
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

void MenuEntry::output(FrameBuffer &fb)
{
  fb.clear();
  fb.setTitle(name);
  if (getType() == MENU_TYPE)
  {
    if (selected < startDisplayAt)
      startDisplayAt = selected;
    else
      while ((selected - startDisplayAt) >= 3)
        startDisplayAt++;
    for (int i = startDisplayAt; i < (int)entries.size(); i++)
    {
      fb.print((i == selected) ? '*' : ' ');
      fb.print(entries[i].name);
      fb.print('\n');
    }
  }
  else
  {
    fb.setCursor(0, 2);
    fb.print(content);
  }
  fb.display(lcd);
}

unsigned long keypressedAt = 0;
uint8_t keypress = 0;

void keydown(uint8_t k)
{
  keypressedAt = millis();
  keypress = k;
  Serial.printf("Key %d pressed\n", k);
}

bool keytick(uint8_t k, unsigned long durn)
{
  MenuEntry *current = NULL;
  MenuEntryType typ = MISC_TYPE; // ie don't know what it is

  bool result = false;

  if (durn >= 750)
  {

    if (!navigation.empty())
    {
      current = navigation.top();
      typ = current->getType();
    }

    switch (typ)
    {
    case NUM_TYPE:
    case TEXT_TYPE:
      switch (k)
      {
      case KEY_LEFT:
        Serial.println("Switch Character sets left");
        result = true;
        break;
      case KEY_RIGHT:
        Serial.println("Switch Character sets right");
        result = true;
        break;
      };
    default:
      break;
    };
  }
  return result;
}

void keyup(uint8_t k, unsigned long durn)
{
  unsigned long pressLength = millis() - keypressedAt;
  keypressedAt = 0;
  keypress = 0;
  Serial.printf("Key %d released (%lu)\n", k, pressLength);

  MenuEntry *current = NULL;
  lcd.blink_off();

  if (k == KEY_MENU)
  {
    if (navigation.empty())
    {
      menuAdd(&menuRoot);
    }
    else
      while (!navigation.empty())
      {
        navigation.pop();
      }
  }
  else // Key is not key_menu so is context specific
  {
    // Retrieve the current menu entry (if not on the main screen)
    if (!navigation.empty())
    {
      current = navigation.top();

      // If on a menu screen
      if (current->getType() == MENU_TYPE)
      {
        current->dealMenu(k);
      }
      // On a leaf screen
      else
      {
        current->dealLeaf(k);
      }
    }
  }

  if (!navigation.empty())
  {
    current = navigation.top(); // Refresh it. The above may have caused change of screen

    MenuEntryType typ = current->getType();
    Serial.println(current->getName());

    current->output(fb);
    switch (typ)
    {
    case MENU_TYPE:
      Serial.println("Menu");
      if (current->getSelected())
      {
        Serial.printf("Selected = %s\n", current->getSelected()->getName());
      }
      break;
    case TEXT_TYPE:
    case NUM_TYPE:
    case CHECK_TYPE:
      lcd.setCursor(current->pos, 2);
      lcd.blink_on();
      break;
    default:
      Serial.println("Unknown");
      break;
    }
  }
}

void MenuEntry::dealMenu(uint8_t k)
{
  switch (k)
  {
  case KEY_LEFT:
    menuRemove();
    break;
  case KEY_UP:
    selectminus();
    break;
  case KEY_RIGHT:
  case KEY_OK:
    menuAdd(getSelected());
    break;
  case KEY_DOWN:
    selectplus();
    break;
  }
}

void MenuEntry::dealLeaf(uint8_t k)
{
  unsigned char curchar = content[pos];

  switch (k)
  {
  case KEY_LEFT:
    if (pos > 0)
      pos--;
    break;
  case KEY_UP:
    curchar--;
    content[pos] = curchar;
    break;
  case KEY_DOWN:
    curchar++;
    content[pos] = curchar;
    break;
  case KEY_RIGHT:
    pos++;
    if (content.length() < pos)
    {
      content += ' ';
    }
    break;
  case KEY_OK:
    Serial.printf("Update config %s=%s\n", getName(), content.c_str());
    conf[name] = content;
    conf.writeFile();
    break;
  }
}