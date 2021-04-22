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
          "type": "text"
        },
        {
          "n": "PSK",
          "type": "text"
        },
        {
          "n": "WPS",
          "type": "button"
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

/*
,
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
*/