/*
 * lifxUDP.cpp - library for multitasking sending of UDP packest.
 */

#include "lifxUDP.h"
// setup logger
Logger udpLog("app.udp");

lifxUDP::lifxUDP()
{

}

void lifxUDP::initialise(IPAddress broadcastIP, uint32_t remotePort)
{
  _remotePort = remotePort;
  _broadcastIP = broadcastIP;
  _myUdp.begin(_remotePort);
}

/* void lifxUDP::add(std::vector<byte> udpPacket)
{
	_udpPackets.push_back(udpPacket);
  udpLog.trace("lifxUDP::add - Message added, vector size: %d, udpPacket size: %d", _udpPackets.size(), sizeof(udpPacket));

} */

void lifxUDP::add(IPAddress IP,  std::vector<byte> Payload)
{
  udpMsg _udpMsg;
  _udpMsg.destinationIP = IP;
  _udpMsg.payload = Payload;
	_udpPayload.push_back(_udpMsg);
  udpLog.info("lifxUDP::add - Message added, vector size: %d, udpPacket size: %d, destinationIP: %d.%d.%d.%d", _udpPayload.size(), sizeof(Payload), IP[0], IP[1], IP[2], IP[3]);

}

/* void lifxUDP::send()
{
  udpLog.trace("lifxUDP::send - Sending...");
  if (WiFi.ready())
  {
    for(auto &_vPacket : _udpPackets)
    {
      int _sent = _myUdp.sendPacket(&_vPacket[0], _vPacket.size(), _broadcastIP, _remotePort);
      if( _sent < 0)
      {
        udpLog.error("lifxUDP::send - Error: Can't send UDP Code:%d", _sent);
        _reconnect();
      } else {
        udpLog.info("lifxUDP::send - %d sent...", _sent);
      }
    }
    _udpPackets.clear();
  } else {
    udpLog.error("lifxUDP::send - Error: WiFi is down...");
  }
} */

void lifxUDP::send()
{
  udpLog.trace("lifxUDP::send - Sending...");
  if (WiFi.ready())
  {
    for(auto &_Payload : _udpPayload)
    {
      int _sent = _myUdp.sendPacket(&_Payload.payload[0], _Payload.payload.size(), _Payload.destinationIP, _remotePort);
      if( _sent < 0)
      {
        udpLog.error("lifxUDP::send - Error: Can't send UDP Code:%d", _sent);
        _reconnect();
      } else {
        udpLog.info("lifxUDP::send - Sent %d bytes of data to %d.%d.%d.%d", _sent, _Payload.destinationIP[0], _Payload.destinationIP[1], _Payload.destinationIP[2], _Payload.destinationIP[3] );
      }
    }
    _udpPayload.clear();
  } else {
    udpLog.error("lifxUDP::send - Error: WiFi is down...");
  }
}

void lifxUDP::begin(int port)
{
  _myUdp.begin(port);
}

int lifxUDP::parsePacket()
{
  return _myUdp.parsePacket();
}

int lifxUDP::read(byte* packetBuffer, int maxSize)
{
  return _myUdp.read(packetBuffer, maxSize);
}

IPAddress lifxUDP::remoteIP()
{
  return _myUdp.remoteIP();
}

int lifxUDP::remotePort()
{
  return _myUdp.remotePort();
}

void lifxUDP::flush()
{
  _myUdp.flush();
}

void lifxUDP::_reconnect()
{
  int _status;
  _myUdp.stop();
  delay(5000);
  _status = _myUdp.begin(_remotePort);
  udpLog.trace("lifxUDP::_reconnect - Begin Status Code:%d", _status);
}

bool lifxUDP::available()
{
  // if(_udpPackets.size() > 0)
  // {
  //   return true;
  // }
  // return false;

  if(_udpPayload.size() > 0)
  {
    return true;
  }
  return false;
  
}
