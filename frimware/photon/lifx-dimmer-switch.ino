#include "common.h"
#include "button.h"
#include "lifx.h"

#define LOGGING_ON TRUE

#if (LOGGING_ON)
  SerialLogHandler logHandler(LOG_LEVEL_WARN, 
  {
    { "app", LOG_LEVEL_ALL},
    { "app.action", LOG_LEVEL_ALL},
    { "app.light", LOG_LEVEL_ALL},
    { "app.lifx", LOG_LEVEL_WARN},
    { "app.device", LOG_LEVEL_ALL},
    { "app.udp", LOG_LEVEL_INFO}
  });
#endif

Logger actionLog("app.action");
Logger _udpLog("app.udp");

void funcEncoder();

int _buttonA = D4;
int _buttonB = D1;
int _encoderPinA = D2;
int _encoderPinB = D3;

bool btnValueA = true;
bool btnValueB = true;
bool _trigger = false;
int _encoderValueA = 0;
int _encoderValueB = 0;
unsigned long _msEncoderDelta = 0;

volatile bool buttonLastA = false;
volatile bool buttonLastB = false;
volatile bool _encoderLastA = false;
volatile bool _encoderLastB = false;
volatile int _encoderPos = 0;
volatile unsigned long _msEncoder;

int _state = 0;
bool _msgSent = false;
bool _msgAction = false;
unsigned long _now = 0;
String _systemID;
uint32_t _myID = 7777;
bool _dimmMode = true;

//UDP Settings
IPAddress _broadcastIP;
int _remotePort = 56700;
uint16_t _udpPacketSize = 0;
uint32_t lastMsgTime = 0;
uint32_t _lastModeTime = 0;
uint32_t _lastEncoderTime = 0;
uint32_t _encoderSpeed = 0;
float _encoderPercent;
float _encoderSpeedMultiplier = 2.0;

button btnA = button();
button btnB = button();
lifx LIFX = lifx();
lifxUDP _lifxUDP = lifxUDP();

void setup()
{
  Time.zone(10.00);
  Time.setFormat(TIME_FORMAT_ISO8601_FULL);

  Serial.begin(115200);
  delay(1000);

  // Log some debug info
  Log.info("Starting up...");
  Log.info("System version: %s", System.version().c_str());
  Log.info("Device ID: %s", System.deviceID().c_str());
  Log.info("IP: %d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
  Log.info("Subnet: %d.%d.%d.%d", WiFi.subnetMask()[0], WiFi.subnetMask()[1], WiFi.subnetMask()[2], WiFi.subnetMask()[3]);
  Log.info("Gateway: %d.%d.%d.%d", WiFi.gatewayIP()[0], WiFi.gatewayIP()[1], WiFi.gatewayIP()[2], WiFi.gatewayIP()[3]);
  Log.info("SSID: %s", WiFi.SSID());

  IPAddress _myIP = WiFi.localIP();

  _myID = (_myIP[3] & 0x000000FF);
  Log.info("My ID: %lu", (unsigned long)_myID);

  _broadcastIP = IPAddress(_myIP[0], _myIP[1], _myIP[2], 255);

  _lifxUDP.initialise(_broadcastIP, _remotePort);

  // Setup the LIFX object
  LIFX.setUDP(&_lifxUDP);
  LIFX.setBroadcastIP(_broadcastIP);
  LIFX.setRemotePort(_remotePort);
  LIFX.getStatus();

  // Pin Modes
  pinMode(_buttonA, INPUT_PULLDOWN);
  pinMode(_buttonB, INPUT_PULLDOWN);
  pinMode(_encoderPinA, INPUT);
  pinMode(_encoderPinB, INPUT);
  // Interrupts
  attachInterrupt(_encoderPinA, funcEncoder, CHANGE);
}

void loop()
{
  _now = millis();

  // check to see if there are UDP commands to Send
  if (_lifxUDP.available() == true)
  {
    //Log.info("UDP message available for sending...");
    _lifxUDP.send();
    if (_msgAction == true)
    {
      _msgSent = true;
      _msgAction = false;
    }
    lastMsgTime = _now;
  }
  // Read buttons
  if (digitalRead(_buttonA) != buttonLastA)
  {
    buttonLastA = !buttonLastA;
    btnValueA = !digitalRead(_buttonA);
    actionLog.trace("Button A: %s", (btnValueA ? "True" : "False"));
  }

  if (digitalRead(_buttonB) != buttonLastB)
  {
    buttonLastB = !buttonLastB;
    btnValueB = !digitalRead(_buttonB);
    actionLog.trace("Button B: %s", (btnValueB ? "True" : "False"));
  }
  // Check button A (Push Button) for actions
  btnA.check(btnValueA, _now);
  // Switch On/Off
  if (btnA.getClick())
  {
    actionLog.trace("Button A: Clicked...");
    LIFX.togglePower();
    btnA.setClick(false);
    _msgAction = true;
  }

  if (btnA.getDoubleClick())
  {
    actionLog.trace("Button A: Double Clicked...");
    btnA.setDoubleClick(false);
  }

  if (btnA.getHold())
  {
    actionLog.trace("Button A: Held...");
    LIFX.discover();
    btnA.setHold(false);
    _msgAction = true;
  }

  if (btnA.getLongHold())
  {
    actionLog.trace("Button A: Long Hold...");
    btnA.setLongHold(false);
  }
  // Check button B (Encoder Switch) for actions
  btnB.check(btnValueB, _now);
  // change from dimmer mode
  if (btnB.getClick())
  {
    actionLog.trace("Button B: Clicked...");
    _dimmMode = !_dimmMode;
    actionLog.trace("Button B: Encoder Mode %s", (_dimmMode ? "Dimmer" : "Colour Cycle"));
    btnB.setClick(false);
    _lastModeTime = _now;
  }

  if (btnB.getDoubleClick())
  {
    actionLog.trace("Button B: Double Clicked...");
    btnB.setDoubleClick(false);
  }
  // Toggle between colour and white
  if (btnB.getHold())
  {
    actionLog.trace("Button B: Held...");
    LIFX.toggleColor();
    btnB.setHold(false);
  }

  if (btnB.getLongHold())
  {
    actionLog.trace("Button B: Long Hold...");
    btnB.setLongHold(false);
  }

  // Mode fall back
  if ((_now - _lastModeTime > 10000) && _dimmMode == false)
  {
    _dimmMode = true;
  }

  // Encodeer
  if (_trigger)
  {
    if (_dimmMode == true)
    {
      LIFX.dimLights(_encoderPercent * _encoderSpeedMultiplier);
    }
    else
    {
      LIFX.cycleColor(_encoderPercent * _encoderSpeedMultiplier);
    }
    _lastEncoderTime = _now;
    _lastModeTime = _now;
    _encoderPercent = 0;
    _trigger = false;
  }
  // get lamp status
  if ((((_now - lastMsgTime > 500) && (_msgSent == true)) || (_now - lastMsgTime > 30000)) && LIFX.Lights.size() > 0)
  {
    actionLog.trace("Getting status - Lights Vetor Size: %d", LIFX.Lights.size());
    actionLog.info("Getting status...");
    LIFX.getStatus();
    LIFX.getLocations();
    LIFX.getGroups();
    _msgSent = false;
  }
  //
  // Check if data has been received
  _udpPacketSize = _lifxUDP.parsePacket();
  if (_udpPacketSize > 0)
  {
    _udpLog.info("Incoming UDP packet size: %d", _udpPacketSize);
    byte _packetBuffer[_udpPacketSize]; //buffer to hold incoming packet

    // Read first 128 of data received
    _lifxUDP.read(_packetBuffer, _udpPacketSize);

    // Ignore other chars
    _lifxUDP.flush();

    // Store sender ip and port
    IPAddress senderIP = _lifxUDP.remoteIP();
    int port = _lifxUDP.remotePort();
    _udpLog.trace("Sender IP:%d.%d.%d.%d:%d", senderIP[0], senderIP[1], senderIP[2], senderIP[3], port);

    // translate data
    LIFX.msgIn(_packetBuffer, senderIP);
    // Set last time we saw a msg
    lastMsgTime = _now;
  }
}
//----------------------------------------------------------------
void funcEncoder()
{
  int _incrementor = 0;
  _encoderValueA = digitalRead(_encoderPinA);
  _encoderValueB = digitalRead(_encoderPinB);

  if (_encoderValueA != _encoderLastA)
  {
    int s = _state & 3;
    if (_encoderValueA)
      s |= 4;
    if (_encoderValueB)
      s |= 8;
    switch (s)
    {
    case 0:
    case 5:
    case 10:
    case 15:
      break;
    case 1:
    case 7:
    case 8:
    case 14:
      _incrementor = 2;
      break;
    case 2:
    case 4:
    case 11:
    case 13:
      _incrementor = -2;
      break;
    case 3:
    case 12:
      _incrementor = 1;
      break;
    default:
      _incrementor = -1;
      break;
    }
    _state = (s >> 2);

    if (abs(_incrementor) == 1)
    {
      _msEncoderDelta = millis() - _msEncoder;
      _msEncoderDelta = constrain(_msEncoderDelta, 0, 350);

      _encoderPercent = (1.01 - ((float)_msEncoderDelta / 350.00)) * _incrementor;

      _trigger = true;
      _msEncoder = millis();
    }

    _encoderLastA = _encoderValueA;
  }
}
