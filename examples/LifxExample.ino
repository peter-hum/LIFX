#include "Lifx.h"



#define POWER_BUTTON_PIN 14

//  LIFX
Lifx lifx;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {

  Serial.begin(115200);
  delay(10);

  // WIFI connection
  Serial.println("Connecting Wifi...");
  WiFi.begin("wfe1","dilbert2002");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // LIFX CALLBACK AND DISCOVERY
  lifx.DiscoveryCompleteCallback(DiscoveryComplete);
  lifx.StartDiscovery();
  
  //  PINS
  pinMode(POWER_BUTTON_PIN, INPUT);    
}

unsigned long powerButtonCheck = millis();
uint16_t power = 0;
char *lifxGroup = "Bedroom";

void loop() {

  lifx.loop();
  
  if (digitalRead(POWER_BUTTON_PIN) == HIGH && (millis() - powerButtonCheck) > 500)
  {
    power = power == 0 ? 65535 : 0;
    lifx.SetPowerByGroup(lifxGroup,power);
    powerButtonCheck = millis();
  }
}


void DiscoveryComplete(Lifx& l)
{
  power = l.StatePowerByGroup(lifxGroup);
  Serial.println("Discovery Complete");
  Serial.printf("Power: %d\n",power);
  Serial.printf("Device count: %i\n", l.DeviceCount());
  l.PrintDevices();
}
