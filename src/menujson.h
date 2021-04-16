const char *menujson =
    R"--(
{
  "n": "Main",
  "menu": [
    {
      "n": "WiFi",
      "menu": [
        {
          "n": "SSID",
          "type": "text"
        },
        {
          "n": "PSK",
          "type": "text"
        },
        {
          "n": "WSP",
          "type": "check"
        }
      ]
    },
    {
      "n": "MQTT",
      "menu": [
        {
          "n": "MQTT Broker",
          "type": "text"
        },
        {
          "n": "MQTT Port",
          "type": "num"
        },
        {
          "n": "MQTT User",
          "type": "text"
        },
        {
          "n": "MQTT Password",
          "type": "text"
        }
      ]
    },
    {
      "n": "System",
      "menu": [
      ]
    }
  ]
}
)--";