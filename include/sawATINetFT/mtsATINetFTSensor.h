/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  Author(s):  Preetham Chalasani
  Created on: 2013

  (C) Copyright 2013-2014 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---

*/

#ifndef _mtsATINetFTSensor_h
#define _mtsATINetFTSensor_h

#include <cisstOSAbstraction/osaSocket.h>
#include <cisstMultiTask/mtsTaskContinuous.h>
#include <cisstMultiTask/mtsVector.h>

#include <sawATINetFT/mtsATINetFTConfig.h>

//#define PORT 49152 /* Port the Net F/T always uses */
#define COMMAND 2 /* Command code 2 starts streaming */
#define NUM_SAMPLES 1 /* Will send 1 sample `before stopping */

typedef unsigned int uint32;
typedef int int32;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned char byte;

class mtsATINetFTSensor: public mtsTaskContinuous
{
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_ALLOW_ERRORS_AND_WARNINGS );

public:
    mtsATINetFTSensor(const std::string & componentName);
    ~mtsATINetFTSensor() {
        Socket.Close();
    }

    void Startup(void);
    void Run(void);
    void Cleanup(void);
    void CloseSocket(void);
    void SetIPAddress(const std::string & ip);
    void Configure(const std::string & filename);

protected:
    void ConnectToSocket(void);
    void GetReadings(void);
    void RebiasFTData(void);
    bool IsSaturated(void);

private:
    // Configuration
    mtsATINetFTConfig NetFTConfig;
    bool Saturated;

    // Functions for events
    struct {
        mtsFunctionWrite RobotErrorMsg;
    } EventTriggers;

    // SOcket Information
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

#endif // _mtsATINetFTSensor_h
