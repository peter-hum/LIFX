/************************************************************************/
/* A library to control LIFX lights over the LAN                        */
/* Based on sample code from this fine person at                        */
/* https://community.lifx.com/t/sending-lan-packet-using-arduino/1460/3 */
/*                                                                      */
/* This library is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or    */
/* (at your option) any later version.                                  */
/*                                                                      */
/* This library is distributed in the hope that it will be useful, but  */
/* WITHOUT ANY WARRANTY; without even the implied warranty of           */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     */
/* General Public License for more details.                             */
/*                                                                      */
/* You should have received a copy of the GNU General Public License    */
/* along with this library. If not, see <http://www.gnu.org/licenses/>. */
/*                                                                      */
/* Written by Peter Humphrey July 2021.                                 */
/************************************************************************/

#include <stdint.h>
#include <arduino.h>
#include <vector>
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>


//#define DEBUG 1

#define LIFX_PORT 56700
#define LIFX_INCOMING_PACKET_BUFFER_LEN 300   // Packet buffer size
#define LIFX_MAC_LEN 6                        // Length in bytes of MAC address numbers
// Message types
#define LIFX_DEVICE_GETSERVICE 02
#define LIFX_DEVICE_STATESERVICE 03
#define LIFX_DEVICE_GETPOWER 20
#define LIFX_DEVICE_SETPOWER 21
#define LIFX_DEVICE_STATEPOWER 22
#define LIFX_DEVICE_GETLABEL 23
#define LIFX_DEVICE_STATELABEL 25
#define LIFX_DEVICE_GETLOCATION 48
#define LIFX_DEVICE_STATELOCATION 50
#define LIFX_DEVICE_GETGROUP 51
#define LIFX_DEVICE_STATEGROUP 53
#define LIFX_LIGHT_GET 101
#define LIFX_LIGHT_SETCOLOR 102
#define LIFX_LIGHT_STATE 107
#define LIFX_REDISCOVERY_INTERVAL 300000


// The LIFX Header structure
#pragma pack(push, 1)
typedef struct {
  /* frame */
  uint16_t size;
  uint16_t protocol:12;
  uint8_t  addressable:1;
  uint8_t  tagged:1;
  uint8_t  origin:2;
  uint32_t source;
  /* frame address */
  uint8_t  target[8];
  uint8_t  reserved[6];
  uint8_t  res_required:1;
  uint8_t  ack_required:1;
  uint8_t  :6;
  uint8_t  sequence;
  /* protocol header */
  uint64_t :64;
  uint16_t type;
  uint16_t :16;
  /* variable length payload follows */
} lifx_header;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
  uint16_t level;
} lifx_payload_device_power;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
  char label[32];
} lifx_payload_device_label;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
  byte location[16];
  char label[32];
  uint64_t updated_at;
} lifx_payload_device_location;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
  byte group[16];
  char label[32];
  uint64_t updated_at;
} lifx_payload_device_group;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
  uint16_t hue;
  uint16_t saturation;
  uint16_t brightness;
  uint16_t kelvin;
  uint16_t reserve1;
  uint16_t power;
  char label[32];
  uint64_t reserve2;
} lifx_payload_light_state;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
  uint8_t reserved;
  uint16_t hue;
  uint16_t saturation;
  uint16_t brightness;
  uint16_t kelvin;
  uint32_t duration;
} lifx_payload_light_setcolor;
#pragma pack(pop)

class Device
{
  public:
    Device(byte macAddress[], uint32_t ipAddress);
    byte *MacAddress();
    uint32_t IpAddress();
    char *MacAddressString();

    uint16_t Port = 0;
    uint16_t Power = 0;
    uint16_t Hue = 0;
    uint16_t Saturation = 0;
    uint16_t Brightness = 0;
    uint16_t Kelvin = 0;
    char Label[32];
    char Location[32];
    char Group[32];
    uint16_t LastMessageType = 0;
  private:
    uint32_t _ipAddress;
    byte _macAddress[LIFX_MAC_LEN];
    char _macString[19];
};

class Lifx
{
  typedef void (*CallbackFunction) (Lifx&);
  
  public:
    Lifx();
    void loop();
    void DealWithReceivedMessage(byte packet[], int packetLen, Device *device);  
    Device* DeviceAddToArray(byte macAddress[LIFX_MAC_LEN], IPAddress ipAddress);
    uint16_t DeviceCount();
    void DiscoveryCompleteCallback(CallbackFunction f);
    void DoDiscovery();
    void ReceivedMessage(byte packet[], int packetLen);
    void PrintDevices();
    void SendMessage(uint16_t messageType, byte *macAddress, IPAddress ipAddress, int payloadLen);
    void SetBrightnessByGroup(char *group, uint16_t brightness);
    void SetBrightnessByLabel(char *label, uint16_t brightness);
    void SetDeviceBrightness(Device *device, uint16_t brightness);
    void SetDevicePower(Device *device, uint16_t power);
    void SetPowerByGroup(char *group, uint16_t power);
    void SetPowerByLabel(char *label, uint16_t power);
    void StartDiscovery();
    uint16_t StateBrightnessByGroup(char *group);
    uint16_t StateBrightnessByLabel(char *label);
    uint16_t StatePowerByGroup(char *group);
    uint16_t StatePowerByLabel(char *label);
  private:
    std::vector<Device *> _devices;
    lifx_header _header;
    union
    {
      lifx_payload_device_power power;
      lifx_payload_device_label label;
      lifx_payload_device_location location;
      lifx_payload_device_group group;
      lifx_payload_light_state lightState;
      lifx_payload_light_setcolor setColor;
    } _payload;
    
    WiFiUDP _udp;
    CallbackFunction _discoveryCompleteFunction = NULL;
    bool _discoveryUnderway = 0;
    unsigned long _discoveryTimer;
    unsigned long _discoveryNextMsec;
    int _discoveryBroadcastCount;
    int _discoveryDeviceIndex;
};
