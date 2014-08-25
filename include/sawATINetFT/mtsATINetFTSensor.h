/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
$Id: $

Author(s):  Preetham Chalasani
Created on: 2013

(C) Copyright 2006-2013 Johns Hopkins University (JHU), All Rights
Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---

*/

#ifndef _mtsATINetFTSensor_H
#define _mtsATINetFTSensor_H

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#endif

#include <cisstMultiTask.h>
#include <cisstCommon.h>
#include <cisstOSAbstraction/osaSleep.h>
#include <cisstOSAbstraction/osaSocket.h>

//#define PORT 49152 /* Port the Net F/T always uses */
#define COMMAND 2 /* Command code 2 starts streaming */
#define NUM_SAMPLES 1 /* Will send 1 sample `before stopping */

typedef unsigned int uint32;
typedef int int32;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned char byte;

class mtsATINetFTSensor: public mtsTaskContinuous {
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_ALLOW_ERRORS_AND_WARNINGS );

public:
    mtsATINetFTSensor(const std::string & taskName);
    ~mtsATINetFTSensor(){ Socket.Close(); };

    void Startup(void);
    void Run(void);
    void Cleanup(void);
    void CloseSocket(void);
    void SetIPAddress(std::string ip);

protected:       
    void ConnectToSocket(void);
    void GetReadings(void);
    void RebiasFTData(void);

private:

    // Functions for events
    struct {
        mtsFunctionWrite RobotErrorMsg;
    } EventTriggers;


    osaSocket Socket;
    bool            programStart;
    bool            IsConnected;
    mtsDoubleVec    FTData;
    mtsDoubleVec    RawFTData;
    mtsDoubleVec    InitialFTData;

    std::string     IP;
    uint16          Port;
    uint32          Rdt_sequence;
    uint32          Ft_sequence;
    uint32          Status;

    byte            Request[8];             /* The request data sent to the Net F/T. */
    byte            Response[36];			/* The raw response data received from the Net F/T. */
};

CMN_DECLARE_SERVICES_INSTANTIATION(mtsATINetFTSensor);

#endif
