#include <ArduinoJson.h>
#include "menu.h"
#include "analogkeypad.h"
#include "framebuff.h"
#include "config.h"

extern LiquidCrystal_I2C lcd;

MenuEntry menuRoot;

#include "menujson.h"

extern BreadCrumb navigation;
extern FrameBuffer fb;

#define ADD_KEY_LLEFT 11
#define ADD_KEY_LRIGHT 12
#define ADD_KEY_LDOWN 13
#define ADD_KEY_LUP 14
#define ADD_KEY_LMENU 15

charset ALPHA_chars = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";
charset alpha_chars = " abcdefghijklmnopqrstuvwxyz";
charset num_chars = "0123456789.";
charset punc_chars = "!\"#$%&'()*+,-./:;<=>?[\\]^_{|}~";

keyset_t num_keyset = {{num_chars}, 1};
keyset_t text_keyset = {{alpha_chars, ALPHA_chars, num_chars, punc_chars}, 4};

void (*MenuEntry::leafCallback)(const char *) = NULL;

void MenuEntry::addChar()
{
  while (content.length() <= pos)
  {
    content += ' ';
  }
  content[pos] = keyboard->set[keyset][keysetpos];
}

bool keyset_t::findKey(unsigned char k, unsigned int &s, unsigned int &p)
{
  bool found = false;
  if (k != '\00')
  {
    unsigned int s1 = s;
    bool sloop = false;
    for (; sloop ? (s < s1) : (s <= nsets); s++)
    {
      if (s == nsets)
      {
        s = -1;
        sloop = true;
        continue;
      }
      unsigned char c;
      for (p = 0; (c = set[s][p]) != '\00'; p++)
      {
        if (c == k)
        {
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
  }
  else
    p = 0;
  return found;
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

MenuEntry::MenuEntry()
{
  name[0] = '\0';
  leafkey[0] = '\0';
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
    /*
    for (int i = 0; i < level; i++)
      Serial.print("  ");
    Serial.printf("%s\n", (const char *)obj["n"]);
    */
    strncpy(name, (const char *)obj["n"], sizeof(name) - 1);
  }
  else
  {
    // Serial.println("No name");
  }

  if (obj["menu"])
  {
    JsonArray menuArray = obj["menu"].as<JsonArray>();
    for (JsonObject item : menuArray)
    {
      MenuEntry m;

      m.build(item);
      entries.push_back(m);
    }
  }
  if (obj["type"])
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
    else if (strcmp(t, "leaf") == 0)
    {
      setType(LEAF_TYPE);
    }
  }
  if (obj["leaf"])
  {
    const char *l = obj["leaf"];
    setType(LEAF_TYPE);
    strncpy(leafkey, l, sizeof(name) - 1);
  }
  level--;
}

void MenuEntry::buildmenu()
{
  StaticJsonDocument<1536> doc;
  DeserializationError error = deserializeJson(doc, menujson);
  if (error)
  {
    Serial.println("menu desc deserialisation error");
    Serial.println(error.c_str());
  }
  JsonObject root = doc.as<JsonObject>();
  menuRoot.build(root);
  // menuRoot.dump();
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

/*
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
*/

void MenuEntry::output(FrameBuffer &fb)
{
  fb.clear();
  fb.setTitle(name);
  switch (getType())
  {
  case MENU_TYPE:
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
  break;
  case LEAF_TYPE:
  {
    const char* key = leafkey;
    if (key == NULL)
    {
      key = name;
    }
    if (leafCallback)
      leafCallback(key);
  }
  break;
  default:
  {
    fb.writeField(0, 1, 20, "");
    fb.writeField(0, 3, 20, "");
    int kp1 = keysetpos + 1;
    int kp2 = keysetpos - 1;
    if (kp1 >= (int)strlen(keyboard->set[keyset]))
      kp1 = 0;
    if (kp2 < 0)
      kp2 = strlen(keyboard->set[keyset]) - 1;
    fb.setCursor(pos, 1);
    fb.print(keyboard->set[keyset][kp1]);
    fb.setCursor(pos, 3);
    fb.print(keyboard->set[keyset][kp2]);
    fb.setCursor(0, 2);
    fb.print(content);
  }
  break;
  }
  fb.display(lcd);
}

void keydown(uint8_t k)
{
}

void keyup(uint8_t k, unsigned long durn)
{
  MenuEntry *current = NULL;
  lcd.blink_off();

  if (k == KEY_MENU)
  {
    if (navigation.empty())
      menuAdd(&menuRoot);
    else
      navigation.pop();
  }
  else if (k == ADD_KEY_LMENU)
  {
    while (!navigation.empty())
      navigation.pop();
  }
  else // Key is context specific
  {
    // Retrieve the current menu entry (if not on the main screen)
    if (!navigation.empty())
    {
      current = navigation.top();
      current->deal(k);
    }
  }

  if (!navigation.empty())
  {
    current = navigation.top(); // Refresh it. The above may have caused change of screen

    MenuEntryType typ = current->getType();

    switch (typ)
    {
    case TEXT_TYPE:
    case NUM_TYPE:
      current->output(fb);
      lcd.setCursor(current->pos, 2);
      lcd.blink_on();
      break;
    case MENU_TYPE:
    default:
      current->output(fb);
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
    case KEY_MENU:
      transk = ADD_KEY_LMENU;
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

void MenuEntry::deal(uint8_t k)
{
  switch (type)
  {
  case MENU_TYPE:
    dealMenu(k);
    break;
  default:
    dealInput(k);
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

void MenuEntry::dealInput(uint8_t k)
{
  switch (k)
  {
  case KEY_LEFT:
    if (pos > 0)
      pos--;
    break;
  case KEY_RIGHT:
    pos++;
    if (content.length() < pos)
      keysetpos = 0;
    break;
  case KEY_UP:
    keysetpos++;
    if (keysetpos >= strlen(keyboard->set[keyset]))
      keysetpos = 0;
    addChar();
    break;
  case KEY_DOWN:
    if (keysetpos > 0)
      keysetpos--;
    else
      keysetpos = strlen(keyboard->set[keyset]) - 1;
    addChar();
    break;
  case KEY_OK:
    while (content.endsWith(" "))
      content.remove(content.length() - 1);
    Serial.printf("Update config %s=\"%s\"\n", getName(), content.c_str());
    conf[name] = content;
    conf.writeFile();
    navigation.pop();
    break;
  case ADD_KEY_LLEFT: // Shift keyset
    if (keyset > 0)
      keyset--;
    else
      keyset = keyboard->nsets - 1;
    if (keysetpos >= strlen(keyboard->set[keyset]))
      keysetpos = 0;
    addChar();
    break;
  case ADD_KEY_LRIGHT: // Shift keyset right
    if ((keyset + 1) < keyboard->nsets)
      keyset++;
    else
      keyset = 0;
    if (keysetpos >= strlen(keyboard->set[keyset]))
      keysetpos = 0;
    addChar();
    break;
  case ADD_KEY_LDOWN: // Delete
    keyset = 0;
    keysetpos = 0;
    if (pos < content.length())
      content[pos] = ' ';
    break;
  }

  if (content.length() >= pos)
  {
    keyboard->findKey(content[pos], keyset, keysetpos);
  }
}