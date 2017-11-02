/*
 * lifx.cpp - library for talking to the LIFX blubs.
 */

#include "lifx.h"

// setup logger
Logger lifxLog("app.lifx");

lifx::lifx()
{
    _device = device();
    _powerState = 0;
}

void lifx::setUDP(lifxUDP *udpRef)
{
    _lifxUdp = udpRef;
    _device.setUDP(_lifxUdp);
}

void lifx::setBroadcastIP(IPAddress broadcastIP)
{
    _broadcastIP = broadcastIP;
    _device.setBroadcastIP(broadcastIP);
}

void lifx::setRemotePort(uint16_t remotePort)
{
    _remotePort = remotePort;
    _device.setRemotePort(remotePort);
}

void lifx::discover()
{
    lifxLog.info("lifx discover...");
    _device.getPower();
}

void lifx::addLight(uint8_t mac[6], IPAddress ip, uint32_t port)
{
    // Serial.printlnf("lifx addLight...");
    int i;
    bool found = false;

    for (auto &Light : Lights)
    {
        lifxLog.info("lifx::addLight - Looking for MAC: 0x %02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        if (Light.matchMac(mac))
        {
            found = true;
            break;
        }
    }
    lifxLog.info("lifx::addLight - Already in Vector: %s", (found ? "true" : "false"));
    if (found == false)
    {
        lifxLog.info("lifx::addLight - Before: Size of Vector: %d, Bytes: %d", Lights.size(), (sizeof(Lights[0]) * Lights.size()));
        light _light = light();
        Lights.push_back(_light);

        if (Lights.empty() == false)
        {
            Lights.back().setMAC(mac);
            Lights.back().setPort(port);
            Lights.back().setIP(ip);
            Lights.back().setUDP(_lifxUdp);
            Lights.back().setBroadcastIP(_broadcastIP);
            Lights.back().setRemotePort(_remotePort);
        }
    }
    lifxLog.info("lifx::addLight - After: Size of Vector: %d, Bytes: %d", Lights.size(), (sizeof(Lights[0]) * Lights.size()));
}
/*
* remove and resort array
*/
void lifx::removeLight(uint8_t mac[6])
{
    // Serial.printlnf("lifx removeLight...");
    lifxLog.info("lifx::removeLight - Removing MAC: 0x %02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    uint8_t _index;
    bool found = false;

    for (auto &Light : Lights)
    {
        if (Light.matchMac(mac))
        {
            found = true;
            break;
        }
        _index++;
    }

    if (found == true)
    {
        if (Lights.size() == 1)
        {
            Lights.clear();
        }
        else
        {
            Lights.erase(Lights.begin() + _index);
        }
    }

    lifxLog.info("lifx::removeLight - Size of Vector: %d", Lights.size());
}

void lifx::togglePower()
{
    for (auto &Light : Lights)
    {
        uint16_t level = Light.getPowerLevel();
        // Serial.printlnf("lifx::togglePower - Light: %d, Current Power Level %d", i, level);
        level = level > 0 ? 0 : 65535;
        Light.setPower(level, 200);
    }
}

void lifx::toggleColor()
{
    HSBK _hsbk;
    bool _colorMode;
    for (auto &Light : Lights)
    {
        _hsbk = Light.getHSBK();
        if (_hsbk.saturation > 0)
        {
            // is colour mode, save colour state, get last white status and set to white mode
            Light.setLastColorHSBK(_hsbk);
            _hsbk.saturation = 0;
            _hsbk = Light.getLastWhiteHSBK();
            _colorMode = false;
        }
        else
        {
            // is white mode, save white state, get last white status and set to colour mode
            Light.setLastWhiteHSBK(_hsbk);
            _hsbk.saturation = 65535;
            _hsbk = Light.getLastColorHSBK();
            _colorMode = true;
        }
        Light.setColorMode(_colorMode);
        Light.setColor(_hsbk);
    }
}

void lifx::cycleColor(float step)
{
    lifxLog.info("lifx::cycleColour - Step: %0.4f", step);
    HSBK _hsbk;
    float _hue;
    float _kelvin;
    for (auto &Light : Lights)
    {
        _hsbk = Light.getHSBK();
        lifxLog.info("lifx::cycleColour - Colour Mode: %s", Light.getColorMode() ? "Colour" : "Temperature");
        if (Light.getColorMode() == true)
        {
            //0 - 65535 := 0 - 100%
            _hue = _hsbk.hue + (65535 * (step / 100.00));
            // range limit
            if (_hue > 65535)
            {
                _hue = 0 + (_hue - 65535);
            }
            else if (_hue < 0)
            {
                _hue = 65535 + _hue;
            }
            _hsbk.hue = (uint16_t)_hue;
        }
        else
        {
            // 2500 - 9000
            _kelvin = _hsbk.kelvin + (6500 * (step / 100.00));
            // range limit
            if (_kelvin > 9000)
            {
                _kelvin = 9000;
            }
            else if (_kelvin < 2500)
            {
                _kelvin = 2500;
            }
            _hsbk.kelvin = (uint16_t)_kelvin;
        }
        Light.setColor(_hsbk, 200);
    }
}

void lifx::dimLights(float step)
{
    lifxLog.info("lifx::dimLights - Step: %0.4f", step);
    HSBK _hsbk;
    float _brightness;
    float _step2;
    for (auto &Light : Lights)
    {
        _hsbk = Light.getHSBK();
        //0 - 65535 := 0 - 100%
        _step2 = (65535 * (step / 100.00));
        _step2 = (float)((int)((_step2 > 0.0) ? (_step2 + 0.5) : (_step2 - 0.5)));
        lifxLog.info("lifx::dimLights - Step2: %0.4f", _step2);
        _brightness = _hsbk.brightness + _step2;
        // range limit
        if (_brightness > 65535)
        {
            _brightness = 65535;
        }
        else if (_brightness < 0)
        {
            _brightness = 0;
        }
        // send new brightness
        _hsbk.brightness = (uint16_t)_brightness;
        Light.setColor(_hsbk, 200);
    }
}

void lifx::getStatus()
{
    for (auto &Light : Lights)
    {
        Light.get();
    }
}

void lifx::getLocations()
{
    _device.getLocation();
}

void lifx::getGroups()
{
    _device.getGroup();
}

void lifx::msgIn(byte packetBuffer[128], IPAddress ip)
{
    /*
        is it a lifx msg
        is it for me (senders mac)
        Check for return type Status
        send to light status function
    */
    // LIFX Header Decode
    // Frame
    uint16_t lifxSize = packetBuffer[0] + (packetBuffer[1] << 8); //little endian
    uint8_t lifxOrigin = ((packetBuffer[2] + (packetBuffer[3] << 8)) & 0xC000) >> 14;
    bool lifxTagged = ((packetBuffer[2] + (packetBuffer[3] << 8)) & 0x2000) >> 13;
    bool lifxAddressable = ((packetBuffer[2] + (packetBuffer[3] << 8)) & 0x1000) >> 12;
    uint16_t lifxProtocol = ((packetBuffer[2] + (packetBuffer[3] << 8)) & 0x0FFF);
    uint32_t lifxSource = packetBuffer[4] + (packetBuffer[5] << 8) + (packetBuffer[6] << 16) + (packetBuffer[7] << 24);
    // Frame Address
    uint8_t lifxTarget[8] = {packetBuffer[8], packetBuffer[9], packetBuffer[10], packetBuffer[11], packetBuffer[12], packetBuffer[13], packetBuffer[14], packetBuffer[15]};
    uint8_t lifxReservedA[6] = {packetBuffer[16], packetBuffer[17], packetBuffer[18], packetBuffer[19], packetBuffer[20], packetBuffer[21]};
    uint8_t lifxReservedB = (packetBuffer[22] & 0xFC) >> 2;
    bool lifxAckRequired = (packetBuffer[22] & 0x02) >> 1;
    bool lifxResRequired = packetBuffer[22] & 0x01;
    uint8_t lifxSequence = packetBuffer[23];
    // Protocol Header
    uint64_t lifxReservedC = (packetBuffer[24] + (packetBuffer[25] << 4) + (packetBuffer[26] << 8) + (packetBuffer[27] << 12)) + ((packetBuffer[28] + (packetBuffer[29] << 4) + (packetBuffer[30] << 8) + (packetBuffer[31] << 12)) << 16);
    uint16_t lifxPacketType = packetBuffer[32] + (packetBuffer[33] << 8);
    uint16_t lifxReservedD = packetBuffer[34] + (packetBuffer[35] << 8);

    //lifxLog.trace("lifx::msgIn -- PacketType: %d", lifxPacketType);

    lifxLog.trace("lifx::msgIn - Header:");
    lifxLog.trace("lifx::msgIn -- Frame:");
    lifxLog.trace("lifx::msgIn --- Size: %d", lifxSize);
    lifxLog.trace("lifx::msgIn --- Origin: %d", lifxOrigin);
    lifxLog.trace("lifx::msgIn --- Tagged: %s", lifxTagged ? "true" : "false");
    lifxLog.trace("lifx::msgIn --- Addressable: %s", lifxAddressable ? "true" : "false");
    lifxLog.trace("lifx::msgIn --- Protocol: %d", lifxProtocol);
    lifxLog.trace("lifx::msgIn --- Source: %d", lifxSource);
    lifxLog.trace("lifx::msgIn -- Frame Address:");
    lifxLog.trace("lifx::msgIn --- Target: 0x %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", lifxTarget[0], lifxTarget[1], lifxTarget[2], lifxTarget[3], lifxTarget[4], lifxTarget[5], lifxTarget[6], lifxTarget[7]);
    lifxLog.trace("lifx::msgIn --- ReservedA: 0x %02X %02X %02X %02X %02X %02X", lifxReservedA[0], lifxReservedA[1], lifxReservedA[2], lifxReservedA[3], lifxReservedA[4], lifxReservedA[5]);
    lifxLog.trace("lifx::msgIn --- ReservedB: %d", lifxReservedB);
    lifxLog.trace("lifx::msgIn --- AckRequired: %s", lifxAckRequired ? "true" : "false");
    lifxLog.trace("lifx::msgIn --- ResRequired: %s", lifxResRequired ? "true" : "false");
    lifxLog.trace("lifx::msgIn --- Sequence: %d", lifxSequence);
    lifxLog.trace("lifx::msgIn -- Protocol Header:");
    lifxLog.trace("lifx::msgIn --- ReservedA: %d", lifxReservedC);
    lifxLog.trace("lifx::msgIn --- PacketType: %d", lifxPacketType);
    lifxLog.trace("lifx::msgIn --- ReservedD: %d", lifxReservedD);

    lifxLog.trace("lifx::msgIn - Payload: %d", lifxPacketType);

    switch (lifxPacketType)
    {
    case _deviceStateService:
    {
        uint8_t _service;
        uint32_t _port;

        _service = packetBuffer[36];
        _port = packetBuffer[37] + (packetBuffer[38] << 8) + (packetBuffer[39] << 16) + (packetBuffer[40] << 24);

        lifxLog.trace("lifx::msgIn -- Service: %d", _service);
        lifxLog.trace("lifx::msgIn -- Port: %d", _port);

        if (lifxSource = _myID)
        {
            // we sent the msg that this is in response to
            // TODO:
        }
        break;
    }
    case _deviceStatePower:
    {
        uint16_t _level;

        _level = packetBuffer[36] + (packetBuffer[37] << 8);

        lifxLog.trace("lifx::msgIn -- Level: %d", _level);

        if (lifxSource = _myID)
        {
            uint8_t mac[6];
            mac[0] = lifxTarget[0];
            mac[1] = lifxTarget[1];
            mac[2] = lifxTarget[2];
            mac[3] = lifxTarget[3];
            mac[4] = lifxTarget[4];
            mac[5] = lifxTarget[5];

            if (_level > 0)
            {
                addLight(mac, ip, 56700);
            }
            else
            {
                removeLight(mac);
            }
        }
        break;
    }
    case _deviceStateWifiInfo:
    {
        uint32_t _signal;
        uint32_t _tx;
        uint32_t _rx;
        uint16_t _reserved;

        _signal = packetBuffer[36] + (packetBuffer[37] << 8) + (packetBuffer[38] << 16) + (packetBuffer[39] << 24);
        _tx = packetBuffer[40] + (packetBuffer[41] << 8) + (packetBuffer[42] << 16) + (packetBuffer[43] << 24);
        _rx = packetBuffer[44] + (packetBuffer[45] << 8) + (packetBuffer[46] << 16) + (packetBuffer[47] << 24);
        _reserved = packetBuffer[48] + (packetBuffer[49] << 8);

        lifxLog.trace("lifx::msgIn -- Signal: %d", _signal);
        lifxLog.trace("lifx::msgIn -- Tx: %d", _tx);
        lifxLog.trace("lifx::msgIn -- Rx: %d", _rx);
        lifxLog.trace("lifx::msgIn -- Reserved: %d", _reserved);

        uint8_t mac[6] = {lifxTarget[0], lifxTarget[1], lifxTarget[2], lifxTarget[3], lifxTarget[4], lifxTarget[5]};

        for (auto &Light : Lights)
        {
            if (Light.matchMac(mac))
            {
                Light.setSignal(_signal);
                Light.setTx(_tx);
                Light.setRx(_rx);
                //Lights[i].setReserved(_reserved);
            }
        }
        break;
    }
    case _deviceEchoResponse:
    {
        uint64_t _payload;
        _payload = (packetBuffer[36] + (packetBuffer[37] << 4) + (packetBuffer[38] << 8) + (packetBuffer[39] << 12)) + ((packetBuffer[40] + (packetBuffer[41] << 4) + (packetBuffer[42] << 8) + (packetBuffer[43] << 12)) << 16);

        lifxLog.trace("lifx::msgIn -- Payload: %ll", _payload);

        break;
    }
    case _deviceAcknowledgement:
    {
        lifxLog.trace("lifx::msgIn -- Payload: Acknowledgement");
        break;
    }

    case _lightStatePower: // Sent by a device to provide the current power level.
    {
        uint16_t _level;
        uint8_t _mac[6] = {lifxTarget[0], lifxTarget[1], lifxTarget[2], lifxTarget[3], lifxTarget[4], lifxTarget[5]};
        _level = packetBuffer[36] + (packetBuffer[37] << 8);

        lifxLog.trace("lifx::msgIn -- Level: %d", _level);

        for (auto &Light : Lights)
        {
            if (Light.matchMac(_mac))
            {
                Light.setPowerLevel(_level);
            }
        }
        break;
    }
    case _lightState: // Sent by a device to provide the current light state.
    {
        HSBK _hsbk;
        int16_t _reservedA;
        uint16_t _power;
        std::vector<byte> _lableData;
        String _lable;
        uint64_t _reservedB;
        uint8_t _mac[6] = {lifxTarget[0], lifxTarget[1], lifxTarget[2], lifxTarget[3], lifxTarget[4], lifxTarget[5]};

        _hsbk.hue = packetBuffer[36] + (packetBuffer[37] << 8);
        _hsbk.saturation = packetBuffer[38] + (packetBuffer[39] << 8);
        _hsbk.brightness = packetBuffer[40] + (packetBuffer[41] << 8);
        _hsbk.kelvin = packetBuffer[42] + (packetBuffer[43] << 8);
        _reservedA = packetBuffer[44] + (packetBuffer[45] << 8);
        _power = packetBuffer[46] + (packetBuffer[47] << 8);
        _lableData = {packetBuffer[48], packetBuffer[49], packetBuffer[50], packetBuffer[51], packetBuffer[52], packetBuffer[53], packetBuffer[54], packetBuffer[55], packetBuffer[56], packetBuffer[57], packetBuffer[58], packetBuffer[59], packetBuffer[60], packetBuffer[61], packetBuffer[62], packetBuffer[63], packetBuffer[64], packetBuffer[65], packetBuffer[66], packetBuffer[67], packetBuffer[68], packetBuffer[69], packetBuffer[70], packetBuffer[71], packetBuffer[72], packetBuffer[73], packetBuffer[74], packetBuffer[75], packetBuffer[76], packetBuffer[77], packetBuffer[78], packetBuffer[79]};
        std::string _lable2(_lableData.begin(), _lableData.end());
        //_lable              = _lable2.data();
        _reservedB = packetBuffer[80] + (packetBuffer[81] << 8) + (packetBuffer[82] << 16) + (packetBuffer[83] << 24) + (packetBuffer[84] << 32) + (packetBuffer[85] << 40) + (packetBuffer[86] << 48) + (packetBuffer[87] << 56);

        //lifxLog.trace("lifx::msgIn - Raw:- Hue:%d, Saturation:%d, Brightness:%d, Kelvin:%d", _hsbk.hue, _hsbk.saturation, _hsbk.brightness, _hsbk.kelvin);
        lifxLog.trace("lifx::msgIn -- Hue: %0.2f", (float)(_hsbk.hue / (65535 / 359)));
        lifxLog.trace("lifx::msgIn -- Saturation: %0.2f", ((float)_hsbk.saturation / 65535.00) * 100.00);
        lifxLog.trace("lifx::msgIn -- Brightness: %0.2f", ((float)_hsbk.brightness / 65535.00) * 100.00);
        lifxLog.trace("lifx::msgIn -- Kelvin: %d", _hsbk.kelvin);
        lifxLog.trace("lifx::msgIn -- ReservedA: %d", _reservedA);
        lifxLog.trace("lifx::msgIn -- Power: %d", _power);
        lifxLog.trace("lifx::msgIn -- Lable: %s", _lable2.data());
        lifxLog.trace("lifx::msgIn -- ReservedB: %d", _reservedB);

        for (auto &Light : Lights)
        {
            if (Light.matchMac(_mac))
            {
                Light.setHSBK(_hsbk);
                Light.setPowerLevel(_power);
                if (_hsbk.saturation > 0)
                {
                    // White Mode
                    Light.setLastWhiteHSBK(_hsbk);
                }else{
                    // Colour Mode
                    Light.setLastColorHSBK(_hsbk);
                }
            }
        }
        break;
    }
    case _deviceStateLocation:
    {
        /*location	    byte array, size: 16 bytes
          label	        string, size: 32 bytes
          updated_at	unsigned 64-bit integer */

        //std::vector<byte> _location;
        std::vector<byte> _lableData;
        uint64_t _updatedAt;

        /*_location = {packetBuffer[36], packetBuffer[37], packetBuffer[38], packetBuffer[39], packetBuffer[40], packetBuffer[41], packetBuffer[42], packetBuffer[43], packetBuffer[44], packetBuffer[45], packetBuffer[46], packetBuffer[47], packetBuffer[48], packetBuffer[49], packetBuffer[50], packetBuffer[51]};
        std::string _lable1(_location.begin(), _location.end());*/
        _lableData = {packetBuffer[52], packetBuffer[53], packetBuffer[54], packetBuffer[55], packetBuffer[56], packetBuffer[57], packetBuffer[58], packetBuffer[59], packetBuffer[60], packetBuffer[61], packetBuffer[62], packetBuffer[63], packetBuffer[64], packetBuffer[65], packetBuffer[66], packetBuffer[67], packetBuffer[68], packetBuffer[69], packetBuffer[70], packetBuffer[71], packetBuffer[72], packetBuffer[73], packetBuffer[74], packetBuffer[75], packetBuffer[76], packetBuffer[77], packetBuffer[78], packetBuffer[79], packetBuffer[80], packetBuffer[81], packetBuffer[82], packetBuffer[83]};
        std::string _lable2(_lableData.begin(), _lableData.end());
        _updatedAt = packetBuffer[84] + (packetBuffer[85] << 8) + (packetBuffer[86] << 16) + (packetBuffer[87] << 24) + (packetBuffer[88] << 32) + (packetBuffer[89] << 40) + (packetBuffer[90] << 48) + (packetBuffer[91] << 56);

        lifxLog.trace("lifx::msgIn -- Location: 0x{%02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X}", packetBuffer[36], packetBuffer[37], packetBuffer[38], packetBuffer[39], packetBuffer[40], packetBuffer[41], packetBuffer[42], packetBuffer[43], packetBuffer[44], packetBuffer[45], packetBuffer[46], packetBuffer[47], packetBuffer[48], packetBuffer[49], packetBuffer[50], packetBuffer[51]);
        lifxLog.trace("lifx::msgIn -- Lable: %s", _lable2.data());
        lifxLog.trace("lifx::msgIn -- Updated At: %d", _updatedAt);

        break;
    }
    case _deviceStateGroup:
    {
        /*location	    byte array, size: 16 bytes
          label	        string, size: 32 bytes
          updated_at	unsigned 64-bit integer */

        //std::vector<byte> _location;
        std::vector<byte> _lableData;
        uint64_t _updatedAt;

        /*_location = {packetBuffer[36], packetBuffer[37], packetBuffer[38], packetBuffer[39], packetBuffer[40], packetBuffer[41], packetBuffer[42], packetBuffer[43], packetBuffer[44], packetBuffer[45], packetBuffer[46], packetBuffer[47], packetBuffer[48], packetBuffer[49], packetBuffer[50], packetBuffer[51]};
        std::string _lable1(_location.begin(), _location.end());*/
        _lableData = {packetBuffer[52], packetBuffer[53], packetBuffer[54], packetBuffer[55], packetBuffer[56], packetBuffer[57], packetBuffer[58], packetBuffer[59], packetBuffer[60], packetBuffer[61], packetBuffer[62], packetBuffer[63], packetBuffer[64], packetBuffer[65], packetBuffer[66], packetBuffer[67], packetBuffer[68], packetBuffer[69], packetBuffer[70], packetBuffer[71], packetBuffer[72], packetBuffer[73], packetBuffer[74], packetBuffer[75], packetBuffer[76], packetBuffer[77], packetBuffer[78], packetBuffer[79], packetBuffer[80], packetBuffer[81], packetBuffer[82], packetBuffer[83]};
        std::string _lable2(_lableData.begin(), _lableData.end());
        _updatedAt = packetBuffer[84] + (packetBuffer[85] << 8) + (packetBuffer[86] << 16) + (packetBuffer[87] << 24) + (packetBuffer[88] << 32) + (packetBuffer[89] << 40) + (packetBuffer[90] << 48) + (packetBuffer[91] << 56);

        lifxLog.trace("lifx::msgIn -- Location: 0x{%02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X}", packetBuffer[36], packetBuffer[37], packetBuffer[38], packetBuffer[39], packetBuffer[40], packetBuffer[41], packetBuffer[42], packetBuffer[43], packetBuffer[44], packetBuffer[45], packetBuffer[46], packetBuffer[47], packetBuffer[48], packetBuffer[49], packetBuffer[50], packetBuffer[51]);
        lifxLog.trace("lifx::msgIn -- Lable: %s", _lable2.data());
        lifxLog.trace("lifx::msgIn -- Updated At: %d", _updatedAt);

        break;
    }
    default:
    {

        _tmp = "lifx::msgIn - Unknowen Payload:  0x ";
        for (int i = 36; i < lifxSize; i++)
        {
            _tmp.concat(String::format("%02X ", packetBuffer[i]));
            //Serial.printf("%02X ", packetBuffer[i]);
        }
        //Serial.println("");
        lifxLog.trace(_tmp);
        break;
    }

    } //end switch
}
