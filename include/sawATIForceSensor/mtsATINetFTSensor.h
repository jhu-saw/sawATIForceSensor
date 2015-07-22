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
#include <cisstParameterTypes/prmForceCartesianSet.h>

#include <sawATIForceSensor/mtsATINetFTConfig.h>

// forward declaration for internal data
class mtsATINetFTSensorData;

// Always include last
#include <sawATIForceSensor/sawATIForceSensorExport.h>

class CISST_EXPORT mtsATINetFTSensor: public mtsTaskContinuous
{
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_ALLOW_ALL);

public:

    enum FilterType{
        NO_FILTER = 0
    };

    mtsATINetFTSensor(const std::string & componentName);
    ~mtsATINetFTSensor() {
        Socket.Close();
    }

    void Startup(void);
    void Run(void);
    void Cleanup(void);
    void CloseSocket(void);
    void SetIPAddress(const std::string & ip);
    void Configure(const std::string & filename,
                   bool useCustomPort = false,
                   int customPortNumber = 0);
    void ApplyFilter(const mtsDoubleVec & rawFT, mtsDoubleVec & filteredFT, const FilterType & filter);

protected:
    void ConnectToSocket(void);
    void GetReadings(void);
    void GetReadingsFromCustomPort(void);
    void Rebias(void);
    void CheckSaturation(const unsigned int status);
    void SetFilter(const std::string & filterName);
    void CheckForErrors(const unsigned int status);

private:
    // Configuration
    mtsATINetFTConfig NetFTConfig;
    bool IsRebiasRequested;
    bool IsSaturated;
    bool HasError;
    double PercentOfMax;

    int ATI_PORT;
    int ATI_COMMAND;
    int ATI_NUM_SAMPLES;

    // Functions for events
    struct {
        mtsFunctionWrite ErrorMsg;
    } EventTriggers;

    // SOcket Information
    osaSocket Socket;
    bool IsConnected;
    bool UseCustomPort;
    mtsDoubleVec FTRawData;
    mtsDoubleVec FTBiasedData;
    mtsDoubleVec FTBiasVec;
    prmForceCartesianSet ForceTorque;

    std::string  IP;

    mtsATINetFTSensorData * Data;
    FilterType CurrentFilter;
};

CMN_DECLARE_SERVICES_INSTANTIATION(mtsATINetFTSensor);

#endif // _mtsATINetFTSensor_h
