/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  Author(s):  Preetham Chalasani, Anton Deguet
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

#include <sawATIForceSensor/mtsATINetFTConfig.h>

// forward declaration for internal data
class mtsATINetFTSensorData;

// Always include last
#include <sawATIForceSensor/sawATIForceSensorExport.h>

class CISST_EXPORT mtsATINetFTSensor: public mtsTaskContinuous
{
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_ALLOW_ERRORS_AND_WARNINGS);

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
    void Rebias(void);
    bool IsSaturated(void);

private:
    // Configuration
    mtsATINetFTConfig NetFTConfig;
    bool Saturated;

    // Functions for events
    struct {
        mtsFunctionWrite ErrorMsg;
    } EventTriggers;

    // SOcket Information
    osaSocket Socket;
    bool         IsConnected;
    mtsDoubleVec ForceTorque;
    mtsDoubleVec RawForceTorque;
    mtsDoubleVec Bias;

    std::string  IP;

    mtsATINetFTSensorData * Data;
};

CMN_DECLARE_SERVICES_INSTANTIATION(mtsATINetFTSensor);

#endif // _mtsATINetFTSensor_h
