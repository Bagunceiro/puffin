const char *menujson =
    R"--(
{
  "n": "Main",
  "menu": [
    {
      "n": "WiFi",
      "menu": [
        {
          "n" : "Connect to WiFi",
          "leaf" : "conwifi"
        },
        {
          "n" : "Disconnect WiFi",
          "leaf" : "diswifi"
        },
        {
          "n": "SSID",
          "text" : "ssid"
        },
        {
          "n": "PSK",
          "text" : "psk"
        },
        {
          "n": "MDNS name",
          "text" : "mdns"
        }
      ]
    },
    {
      "n": "MQTT",
      "menu": [
        {
          "n": "MQTT Broker",
          "text": "mqttbroker"
        },
        {
          "n": "MQTT Port",
          "num": "mqttport"
        },
        {
          "n": "MQTT User",
          "text": "mqttuser"
        },
        {
          "n": "MQTT Password",
          "text": "mqttpwd"
        }
      ]
    },
    {
      "n": "System",
      "menu": [
        {
          "n": "System Reset",
          "menu" : [
            {
              "n" : "Reset Confirm?",
              "leaf" : "reset"
            }
          ]
        },
        {
          "n": "System Update",
          "menu" : [
            {
              "n" : "Update Confirm?",
              "leaf" : "update"
            }
          ]
        },
        {
          "n": "Charset",
          "menu": [
            {
              "n": "Charset1",
              "leaf" : "char1"
            },
            {
              "n": "Charset2",
              "leaf" : "char2"
            },
            {
              "n": "Charset3",
              "leaf" : "char3"
            },
            {
              "n": "Charset4",
              "leaf" : "char4"
            }
          ]
        }
      ]
    }
  ]
}
)--";