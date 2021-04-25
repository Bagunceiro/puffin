#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "wifiserial.h"

// Replace with your network credentials
// const char *ssid = "asgard_2g";
// const char *password = "enaLkraP";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");

String processor(const String &var)
{
  /*
  getSensorReadings();
  //serr.println(var);
  if(var == "TEMPERATURE"){
    return String(temperature);
  }
  else if(var == "HUMIDITY"){
    return String(humidity);
  }
  else if(var == "PRESSURE"){
    return String(pressure);
  }
*/
  return "";
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
 <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    p { font-size: 1.2rem;}
    body {  margin: 0;}
    .topnav { overflow: hidden; background-color: #50B8B4; color: white; font-size: 1rem; }
    .content { padding: 10px; }
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); }
    .cards { max-width: 800px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); }
    .reading { font-size: 1.4rem; }
  </style>
</head>
<body>
  <div class="topnav">
    <h1>Power Monitor</h1>
  </div>
  <div class="content">
    <div class="cards">
      <div class="card">
        <p>VOLTAGE</p><p><span class="reading"><span id="volt">%VOLTAGE%</span></span></p>
      </div>
      <div class="card">
        <p>CURRENT</p><p><span class="reading"><span id="current">%CURRENT%</span></span></p>
      </div>
      <div class="card">
        <p>POWER FACTOR</p><p><span class="reading"><span id="pf">%POWERFACTOR%</span></span></p>
      </div>
      <div class="card">
        <p>FREQUENCY</p><p><span class="reading"><span id="freq">%FREQUENCY%</span></span></p>
      </div>
      <div class="card">
        <p>POWER</p><p><span class="reading"><span id="power">%POWER%</span></span></p>
      </div>
      <div class="card">
        <p>APPARENT POWER</p><p><span class="reading"><span id="ap power">%AP POWER%</span></span></p>
      </div>
      <div class="card">
        <p>Energy</p><p><span class="reading"><span id="energy">%ENERGY%</span></span></p>
      </div>
    </div>
  </div>
<script>
if (!!window.EventSource) {
 var source = new EventSource('/events');
 
 source.addEventListener('open', function(e) {
  console.log("Events Connected");
 }, false);
 source.addEventListener('error', function(e) {
  if (e.target.readyState != EventSource.OPEN) {
    console.log("Events Disconnected");
  }
 }, false);
 
 source.addEventListener('message', function(e) {
  console.log("message", e.data);
 }, false);
 
 source.addEventListener('voltage', function(e) {
  console.log("voltage", e.data);
  document.getElementById("volt").innerHTML = e.data;
 }, false);
 
 source.addEventListener('current', function(e) {
  console.log("current", e.data);
  document.getElementById("current").innerHTML = e.data;
 }, false);
 
 source.addEventListener('power factor', function(e) {
  console.log("power factor", e.data);
  document.getElementById("pf").innerHTML = e.data;
 }, false);

 source.addEventListener('frequency', function(e) {
  console.log("frequency", e.data);
  document.getElementById("freq").innerHTML = e.data;
 }, false);

 source.addEventListener('power', function(e) {
  console.log("power", e.data);
  document.getElementById("power").innerHTML = e.data;
 }, false);

 source.addEventListener('ap power', function(e) {
  console.log("ap power", e.data);
  document.getElementById("ap power").innerHTML = e.data;
 }, false);

  source.addEventListener('energy', function(e) {
  console.log("energy", e.data);
  document.getElementById("energy").innerHTML = e.data;
 }, false);
}
</script>
</body>
</html>)rawliteral";

void wssetup()
{
  // Handle Web Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  // Handle Web Server Events
  events.onConnect([](AsyncEventSourceClient *client) {
    if (client->lastId())
    {
      serr.println("Client reconnected");
    }
  });
  server.addHandler(&events);
  server.begin();
}