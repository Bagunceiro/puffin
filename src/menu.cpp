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

#define ADD_KEY_LLEFT 11
#define ADD_KEY_LRIGHT 12
#define ADD_KEY_LDOWN 13
#define ADD_KEY_LUP 13

charset ALPHA_chars = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";
charset alpha_chars = " abcdefghijklmnopqrstuvwxyz";
charset num_chars = "0123456789";
charset punc_chars = " !\"#$%&'()*+,-./:;<=>?[\\]^_{|}~";

keyset_t num_keyset = {{num_chars}, 1};
keyset_t text_keyset = {{alpha_chars, ALPHA_chars, num_chars, punc_chars}, 4};

void addCharAt(String &s, unsigned int pos, unsigned char c)
{
  while (s.length() <= pos)
  {
    s += ' ';
  }
  s[pos] = c;
}

bool keyset_t::findKey(unsigned char k, unsigned int &s, unsigned int &p)
{
  Serial.printf("findKey(%c, %d, %d)\n", k, s, p);
  if (k != '\00')
  {
    bool found = false;
    unsigned int s1 = s;
    bool sloop = false;
    for (; sloop ? (s < s1) : (s <= nsets); s++)
    {
      Serial.printf("test set %d\n", s);
      if (s == nsets)
      {
        s = -1;
        sloop = true;
        Serial.println("loop back");
        continue;
      }
      unsigned char c;
      for (p = 0; (c = set[s][p]) != '\00'; p++)
      {
        if (c == k)
        {
          Serial.printf("Found %c at %d %d\n", k, s, p);
          found = true;
          break;
        }
      }
      if (found)
        break;
    }
    if (!found)
    {
      s = 0;
      p = 0;
    }
    return found;
  }
  else
    p = 0;
}

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
  keyset = 0;
  keysetpos = 0;
  keyboard = NULL;
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
    {
      keyboard = &text_keyset;
      setType(TEXT_TYPE);
    }
    else if (strcmp(t, "num") == 0)
    {
      keyboard = &num_keyset;
      setType(NUM_TYPE);
    }
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
  if (content.length() > 0)
  {
    keyboard->findKey(content[0], keyset, keysetpos);
  }
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

bool keytick(uint8_t k, unsigned long durn)
{
  uint8_t transk = 0;
  bool result = false;

  if (durn >= 750)
  {
    switch (k)
    {
    case KEY_LEFT:
      transk = ADD_KEY_LLEFT;
      break;
    case KEY_RIGHT:
      transk = ADD_KEY_LRIGHT;
      break;
    case KEY_DOWN:
      transk = ADD_KEY_LDOWN;
      break;
    }
    if (transk != 0)
    {
      keyup(transk, durn);
      result = true;
    }
  }
  return result;
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
  unsigned char curchar = keyboard->set[keyset][keysetpos];

  switch (k)
  {
  case KEY_LEFT:
    if (pos > 0)
      pos--;
    break;
  case KEY_UP:
    keysetpos++;
    if (keysetpos >= strlen(keyboard->set[keyset]))
    {
      keysetpos = 0;
    }
    curchar = keyboard->set[keyset][keysetpos];
    Serial.printf("curchar (at %d) is now %d(%c)\n", pos, curchar, isprint(curchar) ? curchar : ' ');
    addCharAt(content, pos, curchar);
    break;
  case KEY_DOWN:
    keysetpos--;
    if (keysetpos < 0)
    {
      keysetpos = strlen(keyboard->set[keyset]) - 1;
    }
    curchar = keyboard->set[keyset][keysetpos];
    Serial.printf("curchar (at %d) is now %d(%c)\n", pos, curchar, isprint(curchar) ? curchar : ' ');
    addCharAt(content, pos, curchar);
    break;
  case KEY_RIGHT:
    pos++;
    if (content.length() < pos)
    {
      keysetpos = 0;
    }
    break;
  case KEY_OK:

    while (content.endsWith(" "))
    {
      content.remove(content.length() - 1);
    }
    Serial.printf("Update config %s=\"%s\"\n", getName(), content.c_str());
    conf[name] = content;
    conf.writeFile();
    navigation.pop();
    break;
  case ADD_KEY_LLEFT:
    if (keyset > 0)
      keyset--;
    else
      keyset = keyboard->nsets - 1;
    if (keysetpos >= strlen(keyboard->set[keyset]))
      keysetpos = 0;
    curchar = keyboard->set[keyset][keysetpos];
    addCharAt(content, pos, curchar);
    Serial.printf("Switch Character sets left to %d\n", keyset);
    break;
  case ADD_KEY_LRIGHT:
    if ((keyset + 1) < keyboard->nsets)
      keyset++;
    else
      keyset = 0;
    if (keysetpos >= strlen(keyboard->set[keyset]))
      keysetpos = 0;
    curchar = keyboard->set[keyset][keysetpos];
    addCharAt(content, pos, curchar);
    Serial.printf("Switch Character sets right to %d\n", keyset);
    break;
  case ADD_KEY_LDOWN:
    keyset = 0;
    keysetpos = 0;
    if (pos < content.length())
      content[pos] = ' ';
    break;
  }
  Serial.printf("dealLeaf(%d) pos = %d\n", k, pos);

  if (content.length() >= pos)
  {
    keyboard->findKey(content[pos], keyset, keysetpos);
  }
}