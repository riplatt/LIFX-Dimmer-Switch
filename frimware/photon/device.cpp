/*
 *  device.cpp - Device half of library for talking to the LIFX blubs.
 */
#include "device.h"

// setup logger
Logger deviceLog("app.device");

device::device(){

};

void device::setUDP(lifxUDP *udpRef)
{
    deviceLog.trace("device::setUDP...");
    _deviceUdp = udpRef;
}

void device::setBroadcastIP(IPAddress broadcastIP)
{
    deviceLog.trace("device::setBroadcastIP...");
    _broadcastIP = broadcastIP;
}
void device::setRemotePort(uint16_t remotePort)
{
    deviceLog.trace("device::setRemotePort...");
    _remotePort = remotePort;
}

void device::getService()
{
    /* header */
    Header header = Header();
    int headerSize = sizeof(header);

    /* payload */
    // N/A

    /* UDP Packet */
    uint8_t udpPacket[headerSize];

    /* build header */
    header.size = headerSize;
    //header.origin = 0;
    header.tagged = 1;
    header.addressable = 1;
    header.protocol = 1024;
    header.source = _myID;
    //header.target[0] = 0;
    //header.target[1] = 0;
    //header.target[2] = 0;
    //header.target[3] = 0;
    //header.target[4] = 0;
    //header.target[5] = 0;
    //header.target[6] = 0;
    //header.target[7] = 0;
    //header.reservedA[0] = 0;
    //header.reservedA[1] = 0;
    //header.reservedA[2] = 0;
    //header.reservedA[3] = 0;
    //header.reservedA[4] = 0;
    //header.reservedA[5] = 0;
    //header.reservedB = 0;
    //header.ack_required = 0;
    //header.res_required = 0;
    //header.sequence = 0;
    //header.reservedC = 0;
    header.type = _deviceGetService;
    //header.reservedD = 0;

    /* build payload */
    // N/A

    /* build udp packet */
    memcpy(&udpPacket, &header, headerSize);

    /* Send UDP Packet */
    // TODO
    std::vector<byte> data(udpPacket, udpPacket + sizeof(udpPacket));
    _deviceUdp->add(data);
    deviceLog.info("deivce::getService - udpPacket size: %d", sizeof(udpPacket));

    _tmp = "deivce::getService - UDP: 0x";
    for (uint8_t i = 0; i < sizeof(udpPacket); i++)
    {
      _tmp.concat(String::format("%02X ", udpPacket[i]));
    }
    deviceLog.trace(_tmp);

};

void device::getPower()
{
    /* header */
    Header header = Header();
    int headerSize = sizeof(header);

    /* payload */
    // N/A

    /* UDP Packet */
    uint8_t udpPacket[headerSize];

    /* build header */
    header.size = headerSize;
    //header.origin = 0;
    header.tagged = 1;
    header.addressable = 1;
    header.protocol = 1024;
    header.source = _myID;
    //header.target[0] = 0;
    //header.target[1] = 0;
    //header.target[2] = 0;
    //header.target[3] = 0;
    //header.target[4] = 0;
    //header.target[5] = 0;
    //header.target[6] = 0;
    //header.target[7] = 0;
    //header.reservedA[0] = 0;
    //header.reservedA[1] = 0;
    //header.reservedA[2] = 0;
    //header.reservedA[3] = 0;
    //header.reservedA[4] = 0;
    //header.reservedA[5] = 0;
    //header.reservedB = 0;
    //header.ack_required = 0;
    //header.res_required = 0;
    //header.sequence = 0;
    //header.reservedC = 0;
    header.type = _deviceGetPower;
    //header.reservedD = 0;

    /* build payload */
    // N/A

    /* build udp packet */
    memcpy(&udpPacket, &header, headerSize);

    /* Send UDP Packet */
    std::vector<byte> data(udpPacket, udpPacket + sizeof(udpPacket));
    _deviceUdp->add(data);
    //_deviceUdp.add(udpPacket);
    deviceLog.info("deivce::getPower - udpPacket size: %d", sizeof(udpPacket));

    _tmp = "deivce::getPower - UDP: 0x";
    for (uint8_t i = 0; i < sizeof(udpPacket); i++)
    {
      _tmp.concat(String::format("%02X ", udpPacket[i]));
    }
    deviceLog.trace(_tmp);
}

void device::getLocation()
{
    /* header */
    Header header = Header();
    int headerSize = sizeof(header);

    /* payload */
    // N/A

    /* UDP Packet */
    uint8_t udpPacket[headerSize];

    /* build header */
    header.size = headerSize;
    //header.origin = 0;
    header.tagged = 1;
    header.addressable = 1;
    header.protocol = 1024;
    header.source = _myID;
    //header.target[0] = 0;
    //header.target[1] = 0;
    //header.target[2] = 0;
    //header.target[3] = 0;
    //header.target[4] = 0;
    //header.target[5] = 0;
    //header.target[6] = 0;
    //header.target[7] = 0;
    //header.reservedA[0] = 0;
    //header.reservedA[1] = 0;
    //header.reservedA[2] = 0;
    //header.reservedA[3] = 0;
    //header.reservedA[4] = 0;
    //header.reservedA[5] = 0;
    //header.reservedB = 0;
    //header.ack_required = 0;
    //header.res_required = 0;
    //header.sequence = 0;
    //header.reservedC = 0;
    header.type = _deviceGetLocation;
    //header.reservedD = 0;

    /* build payload */
    // N/A

    /* build udp packet */
    memcpy(&udpPacket, &header, headerSize);

    /* Send UDP Packet */
    std::vector<byte> data(udpPacket, udpPacket + sizeof(udpPacket));
    _deviceUdp->add(data);
    //_deviceUdp.add(udpPacket);
    deviceLog.info("device::getLocation - udpPacket size: %d", sizeof(udpPacket));

    _tmp = "device::getLocation - UDP: 0x";
    for (uint8_t i = 0; i < sizeof(udpPacket); i++)
    {
        _tmp.concat(udpPacket[i]);
        _tmp.concat(" ");
    }
    deviceLog.trace(_tmp);
}

void device::getGroup()
{
    /* header */
    Header header = Header();
    int headerSize = sizeof(header);

    /* payload */
    // N/A

    /* UDP Packet */
    uint8_t udpPacket[headerSize];

    /* build header */
    header.size = headerSize;
    //header.origin = 0;
    header.tagged = 1;
    header.addressable = 1;
    header.protocol = 1024;
    header.source = _myID;
    //header.target[0] = 0;
    //header.target[1] = 0;
    //header.target[2] = 0;
    //header.target[3] = 0;
    //header.target[4] = 0;
    //header.target[5] = 0;
    //header.target[6] = 0;
    //header.target[7] = 0;
    //header.reservedA[0] = 0;
    //header.reservedA[1] = 0;
    //header.reservedA[2] = 0;
    //header.reservedA[3] = 0;
    //header.reservedA[4] = 0;
    //header.reservedA[5] = 0;
    //header.reservedB = 0;
    //header.ack_required = 0;
    //header.res_required = 0;
    //header.sequence = 0;
    //header.reservedC = 0;
    header.type = _deviceGetGroup;
    //header.reservedD = 0;

    /* build payload */
    // N/A

    /* build udp packet */
    memcpy(&udpPacket, &header, headerSize);

    /* Send UDP Packet */
    std::vector<byte> data(udpPacket, udpPacket + sizeof(udpPacket));
    _deviceUdp->add(data);
    //_deviceUdp.add(udpPacket);
    deviceLog.info("device::getGroup - udpPacket size: %d", sizeof(udpPacket));

    _tmp = "device::getGroup - UDP: 0x";
    for (uint8_t i = 0; i < sizeof(udpPacket); i++)
    {
        _tmp.concat(udpPacket[i]);
        _tmp.concat(" ");
    }
    deviceLog.trace(_tmp);
}
