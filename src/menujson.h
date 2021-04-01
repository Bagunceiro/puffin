const char *menujson =
    R"--(
{
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
          "type": "checkbox"
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