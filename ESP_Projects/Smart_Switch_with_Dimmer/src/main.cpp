/*
 * Simple example for how to use multiple SinricPro Switch device:
 * - setup 4 switch devices
 * - handle request using multiple callbacks
 * 
 * If you encounter any issues:
 * - check the readme.md at https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md
 * - ensure all dependent libraries are installed
 *   - see https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md#arduinoide
 *   - see https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md#dependencies
 * - open serial monitor and check whats happening
 * - check full user documentation at https://sinricpro.github.io/esp8266-esp32-sdk
 * - visit https://github.com/sinricpro/esp8266-esp32-sdk/issues and check for existing issues or open a new one
 */

// Uncomment the following line to enable serial debug output gfjydfgihouhh
//#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
       #define DEBUG_ESP_PORT Serial
       #define NODEBUG_WEBSOCKETS
       #define NDEBUG
#endif

#include <Arduino.h>
#include <WiFi.h>
#include "SinricPro.h"
#include "SinricProSwitch.h"
#include "SinricProDimSwitch.h"

#define WIFI_SSID         "YOUR-SSID"    
#define WIFI_PASS         "YOUR-PASSWORD"
#define APP_KEY           "YOUR-APP-KEY"
#define APP_SECRET        "YOUR-APP-SECRET"

#define SWITCH_ID         "YOUR-SWITCH-ID"
#define DIM_SWITCH_ID     "YOUR-DIMMER-ID"   

#define BAUD_RATE         115200

#define LED               33

#define FLASH             4
#define FLASH_CHANNEL     0
#define PWM_FREQ          5000
#define PWM_RESOLUTION    8

int dutyCycle = 50;
bool ledState = false;

// we use a struct to store all states and values for our dimmable switch
struct 
{
  bool powerState = false;
  int powerLevel = 0;
} device_state;

bool onDimmerPowerState(const String &deviceId, bool &state);
bool onDimmerPowerLevel(const String &deviceId, int &powerLevel);
bool onDimmerAdjustPowerLevel(const String &deviceId, int &levelDelta);
bool onSwitchPowerState(const String &deviceId, bool &state);
void setupWiFi(void);
void resumeState(void);
void noConnectionState(void);
void setupSinricPro(void);

void setup() 
{
  pinMode(LED, OUTPUT);
  ledcSetup(FLASH_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(FLASH, FLASH_CHANNEL);
  digitalWrite(LED, HIGH);
  ledcWrite(FLASH_CHANNEL, dutyCycle);

  Serial.begin(BAUD_RATE); 
  Serial.printf("\r\n\r\n");
  setupWiFi();
  setupSinricPro();
}

void loop() 
{
  SinricPro.handle();
}

bool onDimmerPowerState(const String &deviceId, bool &state) 
{
  //Serial.printf("Device %s power turned %s \r\n", deviceId.c_str(), state?"on":"off");
  Serial.printf("Flash light turned %s\r\n", state?"on":"off");
  device_state.powerState = state;
  ledcWrite(FLASH_CHANNEL, state? dutyCycle : 0);
  return true;
}

bool onDimmerPowerLevel(const String &deviceId, int &powerLevel) 
{
  device_state.powerLevel = powerLevel;
  Serial.printf("Device %s power level changed to %d\r\n", deviceId.c_str(), device_state.powerLevel);
  ledcWrite(FLASH_CHANNEL, (powerLevel << 2));
  return true;
}

bool onDimmerAdjustPowerLevel(const String &deviceId, int &levelDelta) 
{
  device_state.powerLevel += levelDelta;
  Serial.printf("Device %s power level changed about %i to %d\r\n", deviceId.c_str(), levelDelta, device_state.powerLevel);
  levelDelta = device_state.powerLevel;
  return true;
}

bool onSwitchPowerState(const String &deviceId, bool &state) 
{
  digitalWrite(LED, !state);
  Serial.printf("Device 1 turned %s\r\n", state?"on":"off");
  return true;
}

void setupWiFi(void) 
{
  Serial.printf("\r\n[Wifi]: Connecting");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.printf(".");
    delay(250);
  }

  Serial.printf("connected!\r\n[WiFi]: IP-Address is %s\r\n", WiFi.localIP().toString().c_str());
}

void resumeState(void)
{
  ledcSetup(FLASH_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcWrite(FLASH_CHANNEL, ledState? dutyCycle : 0);
}

void noConnectionState(void)
{
  ledcSetup(FLASH_CHANNEL, 100, 8);
  ledcWrite(FLASH_CHANNEL, 5);
}

void setupSinricPro(void) 
{
  SinricProDimSwitch &myDimSwitch = SinricPro[DIM_SWITCH_ID];

  // set callback function to device
  myDimSwitch.onPowerState(onDimmerPowerState);
  myDimSwitch.onPowerLevel(onDimmerPowerLevel);
  myDimSwitch.onAdjustPowerLevel(onDimmerAdjustPowerLevel);
  
  // add devices and callbacks to SinricPro
  SinricProSwitch& mySwitch1 = SinricPro[SWITCH_ID];
  mySwitch1.onPowerState(onSwitchPowerState);

  // setup SinricPro
  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); resumeState();}); 
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); noConnectionState();});
  SinricPro.begin(APP_KEY, APP_SECRET);
}