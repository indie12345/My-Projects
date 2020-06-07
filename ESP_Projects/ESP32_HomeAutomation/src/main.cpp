/* Code tested on ESP32-CAM
        6th of June, 2020         */
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

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

/* In the ESP32-CAM board LED connected to GPIO 4 was active high and LED connected 
to GPIO 33 was active low. Please change these defiitions as per your hardware */
#define APPLIANCE_1     4
#define APPLIANCE_2     33
#define APPLIANCE_1_ON  digitalWrite(APPLIANCE_1, HIGH)
#define APPLIANCE_2_ON  digitalWrite(APPLIANCE_2, HIGH)
#define APPLIANCE_1_OFF digitalWrite(APPLIANCE_1, LOW)
#define APPLIANCE_2_OFF digitalWrite(APPLIANCE_2, LOW)

char state = 0;
const char *ssid = "YOUR_SSID";
const char *password = "YOUR_PASSWORD";

void update(char val);
void updateA1on(void);
void updateA2on(void);
void updateA1off(void);
void updateA2off(void);
void handleRoot00(void);
void handleRoot01(void);
void handleRoot10(void);
void handleRoot11(void);
void handleNotFound(void);

WebServer server(80);

void setup(void) 
{
  pinMode(APPLIANCE_1, OUTPUT);
  pinMode(APPLIANCE_2, OUTPUT);
  APPLIANCE_1_OFF;
  APPLIANCE_2_OFF;

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("myBoard")) 
  {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot00);  
  server.on("/a1on/", updateA1on);
  server.on("/a2on/", updateA2on);
  server.on("/a1off/", updateA1off);
  server.on("/a2off/", updateA2off);
  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) 
{
  server.handleClient();
}

void handleRoot00(void) 
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
	<h1 style="color:LightGray; font-size:60px; font-family:Monotype Corsiva;">My Home Automation</h1>

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
	function appliance1Button() 
	{
	  var checkBox = document.getElementById("switch1");
	  if(checkBox.checked == true)
	  {
		document.getElementById("demo").innerHTML = "Requesting to turn on appliance 1";
		window.location = 'http://'+location.hostname+'/a1on/';
	  }
	  else
	  {
		document.getElementById("demo").innerHTML = "Requesting to turn off appliance 1";
		window.location = 'http://'+location.hostname+'/a1off/';
	  }
	}

	function appliance2Button() 
	{
	  var checkBox = document.getElementById("switch2");
	  if(checkBox.checked == true)
	  {
		document.getElementById("demo").innerHTML = "Requesting to turn on appliance 2";
		window.location = 'http://'+location.hostname+'/a2on/';
	  }
	  else
	  {
		document.getElementById("demo").innerHTML = "Requesting to turn off appliance 2";
		window.location = 'http://'+location.hostname+'/a2off/';
	  }
	}
	</script>
	</body>
	</html>)rawliteral";
  server.send(200, "text/html", webpage);
}

void handleRoot01(void) 
{
  const char webpage1[] PROGMEM = R"rawliteral(
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

	/* Rounded sliders */
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
	<h1 style="color:LightGray; font-size:60px; font-family:Monotype Corsiva;">My Home Automation</h1>

	<h3 style="color:#4CAF50; font-size:30px; font-family:Monotype Corsiva;">Appliance 1</h3>

	<label class="switch">
	  <input type="checkbox" id="switch1" onclick="appliance1Button()" checked>
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
	function appliance1Button() 
	{
	  var checkBox = document.getElementById("switch1");
	  if(checkBox.checked == true)
	  {
		document.getElementById("demo").innerHTML = "Requesting to turn on appliance 1";
		window.location = 'http://'+location.hostname+'/a1on/';
	  }
	  else
	  {
		document.getElementById("demo").innerHTML = "Requesting to turn off appliance 1";
		window.location = 'http://'+location.hostname+'/a1off/';
	  }
	}

	function appliance2Button() 
	{
	  var checkBox = document.getElementById("switch2");
	  if(checkBox.checked == true)
	  {
		document.getElementById("demo").innerHTML = "Requesting to turn on appliance 2";
		window.location = 'http://'+location.hostname+'/a2on/';
	  }
	  else
	  {
		document.getElementById("demo").innerHTML = "Requesting to turn off appliance 2";
		window.location = 'http://'+location.hostname+'/a2off/';
	  }
	}
	</script>
	</body>
	</html>)rawliteral";
  server.send(200, "text/html", webpage1);
}

void handleRoot10(void) 
{
  Serial.println("handleRoot10 called");
  const char webpage2[] PROGMEM = R"rawliteral(
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

	/* Rounded sliders */
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
	<h1 style="color:LightGray; font-size:60px; font-family:Monotype Corsiva;">My Home Automation</h1>

	<h3 style="color:#4CAF50; font-size:30px; font-family:Monotype Corsiva;">Appliance 1</h3>

	<label class="switch">
	  <input type="checkbox" id="switch1" onclick="appliance1Button()">
	  <span class="slider round"></span>
	</label>

	<h3 style="color:#4CAF50; font-size:30px; font-family:Monotype Corsiva;">Appliance 2</h3>

	<label class="switch">
	  <input type="checkbox" id="switch2" onclick="appliance2Button()" checked>
	  <span class="slider round"></span>
	</label>

	<p style="color:orange"; id="demo"></p>
	</center>

	<script>
	function appliance1Button() 
	{
	  var checkBox = document.getElementById("switch1");
	  if(checkBox.checked == true)
	  {
		document.getElementById("demo").innerHTML = "Requesting to turn on appliance 1";
		window.location = 'http://'+location.hostname+'/a1on/';
	  }
	  else
	  {
		document.getElementById("demo").innerHTML = "Requesting to turn off appliance 1";
		window.location = 'http://'+location.hostname+'/a1off/';
	  }
	}

	function appliance2Button() 
	{
	  var checkBox = document.getElementById("switch2");
	  if(checkBox.checked == true)
	  {
		document.getElementById("demo").innerHTML = "Requesting to turn on appliance 2";
		window.location = 'http://'+location.hostname+'/a2on/';
	  }
	  else
	  {
		document.getElementById("demo").innerHTML = "Requesting to turn off appliance 2";
		window.location = 'http://'+location.hostname+'/a2off/';
	  }
	}
	</script>
	</body>
	</html>)rawliteral";
  server.send(200, "text/html", webpage2);
}

void handleRoot11(void) 
{
  Serial.println("handleRoot11 called");
  const char webpage3[] PROGMEM = R"rawliteral(
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

	/* Rounded sliders */
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
	<h1 style="color:LightGray; font-size:60px; font-family:Monotype Corsiva;">My Home Automation</h1>

	<h3 style="color:#4CAF50; font-size:30px; font-family:Monotype Corsiva;">Appliance 1</h3>

	<label class="switch">
	  <input type="checkbox" id="switch1" onclick="appliance1Button()" checked>
	  <span class="slider round"></span>
	</label>

	<h3 style="color:#4CAF50; font-size:30px; font-family:Monotype Corsiva;">Appliance 2</h3>

	<label class="switch">
	  <input type="checkbox" id="switch2" onclick="appliance2Button()" checked>
	  <span class="slider round"></span>
	</label>

	<p style="color:orange"; id="demo"></p>
	</center>

	<script>
	function appliance1Button() 
	{
	  var checkBox = document.getElementById("switch1");
	  if(checkBox.checked == true)
	  {
		document.getElementById("demo").innerHTML = "Requesting to turn on appliance 1";
		window.location = 'http://'+location.hostname+'/a1on/';
	  }
	  else
	  {
		document.getElementById("demo").innerHTML = "Requesting to turn off appliance 1";
		window.location = 'http://'+location.hostname+'/a1off/';
	  }
	}

	function appliance2Button() 
	{
	  var checkBox = document.getElementById("switch2");
	  if(checkBox.checked == true)
	  {
		document.getElementById("demo").innerHTML = "Requesting to turn on appliance 2";
		window.location = 'http://'+location.hostname+'/a2on/';
	  }
	  else
	  {
		document.getElementById("demo").innerHTML = "Requesting to turn off appliance 2";
		window.location = 'http://'+location.hostname+'/a2off/';
	  }
	}
	</script>
	</body>
	</html>)rawliteral";
  server.send(200, "text/html", webpage3);
  Serial.println("handleRoot11 exited");
}

void handleNotFound() 
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) 
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
}

void update(char val)
{ 
  switch(val)
  {
    case 0:
      state &= 2;
      break;

    case 1:
      state |= 1;
      break;

    case 2:
      state &= 1;
      break;

    case 3:
      state |= 2;
      break;

    default:
      break;    
  }

  switch(state)
  {
    case 0:
      handleRoot00();
      break;

    case 1:
      handleRoot01();
      break;

    case 2:
      handleRoot10();
      break;

    case 3:
      handleRoot11();
      break;

    default:
      break;     
  }
}

void updateA1on(void)
{
  update(A1_ON);
  APPLIANCE_1_ON;
}

void updateA2on(void)
{
  update(A2_ON);
  APPLIANCE_2_ON;
}

void updateA1off(void)
{
  update(A1_OFF);
  APPLIANCE_1_OFF;
}

void updateA2off(void)
{
  update(A2_OFF);
  APPLIANCE_2_OFF;
}