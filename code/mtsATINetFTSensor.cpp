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

#include <cisstCommon/cmnPortability.h>
#include <cisstCommon/cmnConstants.h>
#include <cisstMultiTask/mtsInterfaceProvided.h>

#include <sawATIForceSensor/mtsATINetFTSensor.h>

#define ATI_PORT 49152 /* Port the Net F/T always uses */
#define ATI_COMMAND 2 /* Command code 2 starts streaming */
#define ATI_NUM_SAMPLES 1 /* Will send 1 sample `before stopping */

typedef unsigned int uint32;
typedef int int32;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned char byte;

class mtsATINetFTSensorData {
public:
    uint16 Port;
    uint32 RdtSequence;
    uint32 FtSequence;
    uint32 Status;

    byte Request[8];             /* The request data sent to the Net F/T. */
    byte Response[36];			/* The raw response data received from the Net F/T. */
};

#if (CISST_OS == CISST_LINUX)
#include <netinet/in.h>
#endif

CMN_IMPLEMENT_SERVICES(mtsATINetFTSensor)

mtsATINetFTSensor::mtsATINetFTSensor(const std::string & componentName):
    mtsTaskContinuous(componentName, 5000),
    Socket(osaSocket::UDP)
{
    Data = new mtsATINetFTSensorData;

    Data->Port = ATI_PORT;

    ForceTorque.SetSize(6);
    RawForceTorque.SetSize(6);

    StateTable.AddData(ForceTorque, "ForceTorque");
    StateTable.AddData(RawForceTorque, "RawForceTorque");
    StateTable.AddData(IsConnected, "IsConnected");

    mtsInterfaceProvided * interfaceProvided = AddInterfaceProvided("ProvidesATINetFTSensor");
    if (interfaceProvided) {
        interfaceProvided->AddCommandReadState(StateTable, ForceTorque, "GetFTData");
        interfaceProvided->AddCommandReadState(StateTable, RawForceTorque, "GetRawFTData");
        interfaceProvided->AddCommandReadState(StateTable, IsConnected, "GetSocketStatus");
        interfaceProvided->AddCommandReadState(StateTable, StateTable.PeriodStats,
                                               "GetPeriodStatistics");
        interfaceProvided->AddCommandVoid(&mtsATINetFTSensor::Rebias, this, "Rebias");
        interfaceProvided->AddCommandWrite(&mtsATINetFTSensor::SetFilter, this, "SetFilter", std::string(""));
        interfaceProvided->AddEventWrite(EventTriggers.ErrorMsg, "ErrorMsg", std::string(""));
    }
}

void mtsATINetFTSensor::Startup(void)
{
    *(uint16*)&(Data->Request)[0] = htons(0x1234); /* standard header. */
    *(uint16*)&(Data->Request)[2] = htons(ATI_COMMAND); /* per table 9.1 in Net F/T user manual. */
    *(uint32*)&(Data->Request)[4] = htonl(ATI_NUM_SAMPLES); /* see section 9.1 in Net F/T user manual. */

    Socket.SetDestination(IP, Data->Port);
}

void mtsATINetFTSensor::Configure(const std::string & filename)
{
    if (NetFTConfig.LoadCalibrationFile(filename)) {
        CMN_LOG_CLASS_RUN_WARNING << "Configure: file loaded - "
                                  << filename << std::endl;
    }
}

void mtsATINetFTSensor::Cleanup(void)
{
    *(uint16*)&(Data->Request)[0] = htons(0x1234);
    *(uint16*)&(Data->Request)[2] = htons(0); /* Stop streaming */
    *(uint32*)&(Data->Request)[4] = htonl(ATI_NUM_SAMPLES);

    // try to send, but timeout after 10 ms
    int result = Socket.Send((const char *)(Data->Request), 8, 10.0 * cmn_ms);
    if (result == -1) {
        CMN_LOG_CLASS_RUN_WARNING << "Cleanup: UDP send failed" << std::endl;
        return;
    }

    Socket.Close();
}

void mtsATINetFTSensor::SetIPAddress(const std::string & ip)
{
    IP = ip;
}

void mtsATINetFTSensor::Run(void)
{
    ProcessQueuedCommands();
    GetReadings();

    if (IsSaturated()) {
        CMN_LOG_CLASS_RUN_WARNING << "Run: sensor saturated" << std::endl;
    }
}

void mtsATINetFTSensor::GetReadings(void)
{
    int result;
    // try to send, but timeout after 10 ms
    result = Socket.Send((const char *)(Data->Request), 8, 10.0 * cmn_ms);
    if (result == -1) {
        CMN_LOG_CLASS_RUN_WARNING << "GetReadings: UDP send failed" << std::endl;
        return;
    }

    // if we were able to send we should now receive
    result = Socket.Receive((char *)(Data->Response), 36, 10.0 * cmn_ms);
    if (result != -1) {
        this->Data->RdtSequence = ntohl(*(uint32*)&(Data->Response)[0]);
        this->Data->FtSequence = ntohl(*(uint32*)&(Data->Response)[4]);
        this->Data->Status = ntohl(*(uint32*)&(Data->Response)[8]);
//        std::cerr << "Status " << this->Data->Status;
        int temp;
        for (int i = 0; i < 6; i++ ) {
            temp = ntohl(*(int32*)&(Data->Response)[12 + i * 4]);
            RawForceTorque[i]= (double)((double)temp/1000000);
            ApplyFilter(RawForceTorque, ForceTorque, CurrentFilter);
            RawForceTorque.SetValid(true);
            ForceTorque.SetValid(true);
        }
    }
    else {
        CMN_LOG_CLASS_RUN_ERROR << "GetReadings: UDP receive failed" << std::endl;
        RawForceTorque.SetValid(false);
        RawForceTorque.Zeros();
        ForceTorque.SetValid(false);
        ForceTorque.Zeros();
    }
}

void mtsATINetFTSensor::ApplyFilter(const mtsDoubleVec & rawFT, mtsDoubleVec & filteredFT, const FilterType &filter)
{
    if(rawFT.size() != 6) {
        CMN_LOG_CLASS_RUN_ERROR << "ApplyFilter: Size mismatch" << std::endl;
        return;
    }

    if(filter == NO_FILTER) {
        filteredFT = rawFT;
    } else if(filter == BW_FILTER) {
        filteredFT = rawFT;
    }
}

void mtsATINetFTSensor::SetFilter(const std::string &filterName)
{
    if(filterName == "NoFilter") {
        CurrentFilter == NO_FILTER;
    } else if(filterName == "Butterworth") {
        CurrentFilter = BW_FILTER;
    }
}

void mtsATINetFTSensor::Rebias(void)
{
    *(uint16*)&(Data->Request)[2] = htons(0x0042); /* per table 9.1 in Net F/T user manual. */

    // try to send, but timeout after 10 ms
    int result = Socket.Send((const char *)(Data->Request), 8, 10.0 * cmn_ms);
    if (result == -1) {
        CMN_LOG_CLASS_RUN_WARNING << "Rebias: UDP send failed" << std::endl;
        return;
    }

    EventTriggers.ErrorMsg(std::string("Sensor ReBiased"));
    CMN_LOG_CLASS_RUN_VERBOSE << "FT Sensor Rebiased " << std::endl;

    *(uint16*)&(Data->Request)[2] = htons(ATI_COMMAND);
}

bool mtsATINetFTSensor::IsSaturated(void)
{
    vct6 MaxLoads = NetFTConfig.NetFT.GenInfo.Ranges;
    Saturated = false;
    for (size_t i = 0; i < MaxLoads.size(); ++i) {
        if (((ForceTorque[i] > 0) && (ForceTorque[i] > MaxLoads[i])) ||
            ((ForceTorque[i] < 0) && (ForceTorque[i] < -MaxLoads[i])) )  {
            EventTriggers.ErrorMsg(std::string("Sensor Saturated"));
            Saturated = true;
        }
    }
    return Saturated;
}
