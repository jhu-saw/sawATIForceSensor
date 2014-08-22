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

#ifndef _devNetFTSensorATITask_h
#define _devNetFTSensorATITask_h

#include <iostream>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <cisstMultiTask.h>
#include <cisstCommon.h>
#include <cisstOSAbstraction/osaSleep.h>
#include <cisstOSAbstraction/osaSocket.h>

#include "devNetFTSensorATI.h"

#define Fx 0
#define Fy 1
#define Fz 2
#define Tx 3
#define Ty 4
#define Tz 5

typedef unsigned int uint32;
typedef int int32;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned char byte;


class devNetFTSensorATITask: public mtsTaskPeriodic {
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_LOD_RUN_VERBOSE );

public:

    devNetFTSensorATITask(const std::string & taskName, double period);
    ~devNetFTSensorATITask(){};

    void Configure(const std::string &filename);
    void Rebias();


    void Startup(void);
    void Run(void);
    void Cleanup(void);

protected:

    mtsDoubleVec    FTData;
    mtsBool         IsSaturated;
    mtsBool         IsConnected;

    devNetFTSensorATI *FTSensorATI;

    void GetMaxLoads(mtsDoubleVec &values) const;
    void SetTransform(const mtsDoubleVec &values);
    void SaturationCheck();

private:

    // Functions for events
    struct {
        mtsFunctionWrite RobotErrorMsg;
    } EventTriggers;

    const osaTimeServer  &TimeServer;
    mtsDouble TimeStamp;

    mtsFunctionRead GetFTReadings;    
    mtsFunctionRead GetSocketStatus;

    mtsFunctionVoid RebiasFTValues;    

};

CMN_DECLARE_SERVICES_INSTANTIATION(devNetFTSensorATITask);

#endif
