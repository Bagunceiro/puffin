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
        {
          "n": "System Update",
          "menu" : [
            {
              "n" : "Update Confirm?",
              "type" : "leaf"
            }
          ]
        },
        {
          "n": "Charset",
          "menu": [
            {
              "n": "Charset1",
              "type": "leaf"
            },
            {
              "n": "Charset2",
              "type": "leaf"
            },
            {
              "n": "Charset3",
              "type": "leaf"
            },
            {
              "n": "Charset4",
              "type": "leaf"
            }
          ]
        }
      ]
    }
  ]
}
)--";