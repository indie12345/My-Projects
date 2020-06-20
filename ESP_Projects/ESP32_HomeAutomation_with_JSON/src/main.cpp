#include <WiFi.h>
#include <ESPmDNS.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

#define ON_BOARD_LED        2
#define EXTERNAL_CONNECTION 27

#define APPLIANCE_1     ON_BOARD_LED
#define APPLIANCE_2     EXTERNAL_CONNECTION
#define APPLIANCE_1_ON  digitalWrite(APPLIANCE_1, HIGH)
#define APPLIANCE_2_ON  digitalWrite(APPLIANCE_2, HIGH)
#define APPLIANCE_1_OFF digitalWrite(APPLIANCE_1, LOW)
#define APPLIANCE_2_OFF digitalWrite(APPLIANCE_2, LOW)

/*
The bit 0 indicates status
  0 -> Off
  1 -> On  
The bit 1 indicates appliance
  0 -> Appliance 1
  1 -> Appliance 2
*/
#define A1_OFF          0x00
#define A1_ON           0x01
#define A2_OFF          0x02
#define A2_ON           0x03

char state = 0;
char point;
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

AsyncWebServer server(80);
WebSocketsServer websocket(81);

void handleRootJSON(AsyncWebServerRequest *request) 
{
  const char webpage[] PROGMEM = R"rawliteral(
	<!DOCTYPE html>
	<html>
	<head>
	<meta name="viewport" content="width=device-width, initial-scale=1">
	<style>
	body 
	{
	  background-color: rgba(10,10,10,0.95);
	}
	.switch 
	{
	  position: relative;
	  display: inline-block;
	  width: 60px;
	  height: 34px;
	}
	.switch input 
	{
	  opacity: 0;
	  width: 0;
	  height: 0;
	}
	.slider 
	{
	  position: absolute;
	  cursor: pointer;
	  top: 0;
	  left: 0;
	  right: 0;
	  bottom: 0;
	  background-color: #303050;
	  -webkit-transition: .4s;
	  transition: .4s;
	}
	.slider:before 
	{
	  position: absolute;
	  content: "";
	  height: 26px;
	  width: 26px;
	  left: 4px;
	  bottom: 4px;
	  background-color: white;
	  -webkit-transition: .4s;
	  transition: .4s;
	}
	input:checked + .slider 
	{
	  background-color: #2196F3;
	}
	input:focus + .slider 
	{
	  box-shadow: 0 0 1px #2196F3;
	}
	input:checked + .slider:before 
	{
	  -webkit-transform: translateX(26px);
	  -ms-transform: translateX(26px);
	  transform: translateX(26px);
	}
	.slider.round 
	{
	  border-radius: 34px;
	}
	.slider.round:before 
	{
	  border-radius: 50%;
	}
	</style>
	</head>
	<center>
	<h1 style="color:LightGray; font-size:60px; font-family:Monotype Corsiva;">My JSON Home Automation</h1>

	<h3 style="color:#4CAF50; font-size:30px; font-family:Monotype Corsiva;">Appliance 1</h3>

	<label class="switch">
	  <input type="checkbox" id="switch1" onclick="appliance1Button()">
	  <span class="slider round"></span>
	</label>

	<h3 style="color:#4CAF50; font-size:30px; font-family:Monotype Corsiva;">Appliance 2</h3>

	<label class="switch">
	  <input type="checkbox" id="switch2" onclick="appliance2Button()">
	  <span class="slider round"></span>
	</label>

	<p style="color:orange"; id="demo"></p>
	</center>

	<script>
  var appliance1Status = 0;
  var appliance2Status = 0;
	var connection = new WebSocket('ws://'+location.hostname+':81/');
  function appliance1Button() 
	{
	  var checkBox = document.getElementById("switch1");
	  if(checkBox.checked == true)
	  {
		document.getElementById("demo").innerHTML = "Requesting to turn on appliance 1";
		a1on();
	  }
	  else
	  {
		document.getElementById("demo").innerHTML = "Requesting to turn off appliance 1";
		a1off();
	  }
	}

	function appliance2Button() 
	{
	  var checkBox = document.getElementById("switch2");
	  if(checkBox.checked == true)
	  {
		document.getElementById("demo").innerHTML = "Requesting to turn on appliance 2";
		a2on();
	  }
	  else
	  {
		document.getElementById("demo").innerHTML = "Requesting to turn off appliance 2";
		a2off();
	  }
	}

  function a1on()
  {
    appliance1Status = 1;
    console.log("Appliance 1 is on");
    sendJSONData();
  }

  function a2on()
  {
    appliance2Status = 1;
    console.log("Appliance 2 is on");
    sendJSONData();
  }
  
  function a1off()
  {
    appliance1Status = 0;
    console.log("Appliance 1 is off");
    sendJSONData();
  }

  function a2off()
  {
    appliance2Status = 0;
    console.log("Appliance 2 is off");
    sendJSONData();
  }

  function sendJSONData()
  {
    var JSONData = '{"Appliance 1" : '+appliance1Status+'}';
    connection.send(JSONData);
  }

	</script>
	</body>
	</html>)rawliteral";
  request->send(200, "text/html", webpage);
}

void linkNotFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/html", "Not Found");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  String message;
  bool appliance1Status;
  DynamicJsonDocument JSONdoc(200);

  switch (type)
  {
    case WStype_DISCONNECTED:
    Serial.printf("[%u] Disconnected!\n", num);
    break;

    case WStype_CONNECTED:
    {
      IPAddress ip = websocket.remoteIP(num);
      Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

      // send message to client
      websocket.sendTXT(num, "Connected from server");
    }
    break;
  
    case WStype_TEXT:
    Serial.printf("[%u] get Text: %s\n", num, payload);
    message = String((char*)( payload));
    Serial.println(message);
    break;
  }
  
  DeserializationError error = deserializeJson(JSONdoc, message);

  if(error)
  {
    Serial.print("deserializeJson() failed");
    Serial.println(error.c_str());
    return;
  }

  appliance1Status = JSONdoc["Appliance 1"];
  digitalWrite(ON_BOARD_LED, appliance1Status);
}

void setup(void)
{
  Serial.begin(115200);

  pinMode(ON_BOARD_LED, OUTPUT);
  pinMode(EXTERNAL_CONNECTION, OUTPUT);

  Serial.println("Connecting to:");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  if (MDNS.begin("rahul")) 
  {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRootJSON);
  server.onNotFound(linkNotFound);
  server.begin();
  
  Serial.println("HTTP server started");
  websocket.begin();
  Serial.println("Websocket server started");
  websocket.onEvent(webSocketEvent);
}

void loop(void)
{
  websocket.loop();
}