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

#include <sawATINetFT/mtsATINetFTSensor.h>
#if (CISST_OS == CISST_LINUX)
#include <netinet/in.h>
#endif

CMN_IMPLEMENT_SERVICES(mtsATINetFTSensor)

mtsATINetFTSensor::mtsATINetFTSensor(const std::string &taskName):
    mtsTaskContinuous(taskName, 5000),
    Socket(osaSocket::UDP),
    Port(49152)
{
    FTData.SetSize(6);
    RawFTData.SetSize(6);
    InitialFTData.SetSize(6);
    programStart = true;

    mtsInterfaceProvided * socketInterface = AddInterfaceProvided("ProvidesATINetFTSensor");

    StateTable.AddData(FTData, "FTData");
    StateTable.AddData(IsConnected, "IsConneted");

    socketInterface->AddCommandReadState(StateTable, FTData, "GetFTData");
    socketInterface->AddCommandReadState(StateTable, IsConnected, "GetSocketStatus");

    socketInterface->AddCommandVoid(&mtsATINetFTSensor::RebiasFTData, this, "RebiasFTData");

    socketInterface->AddEventWrite(EventTriggers.RobotErrorMsg, "RobotErrorMsg", std::string(""));

    StateTable.Advance();
}

void mtsATINetFTSensor::Startup(void)
{
    *(uint16*)&Request[0] = htons(0x1234); /* standard header. */
    *(uint16*)&Request[2] = htons(COMMAND); /* per table 9.1 in Net F/T user manual. */
    *(uint32*)&Request[4] = htonl(NUM_SAMPLES); /* see section 9.1 in Net F/T user manual. */

    Socket.SetDestination(IP, Port);
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
    result = Socket.Send((const char *)Request, 8, 10.0 *cmn_ms);
    if (result == -1){
        CMN_LOG_CLASS_RUN_WARNING << "GetReadings: UDP send failed" << std::endl;
        return;
    }

    // if we were able to send we should now receive
    result = Socket.Receive((char *)Response, 36, 10.0 * cmn_ms);
    if (result != -1) {
        this->Rdt_sequence = ntohl(*(uint32*)&Response[0]);
        this->Ft_sequence = ntohl(*(uint32*)&Response[4]);
        this->Status = ntohl(*(uint32*)&Response[8]);
        int temp;
        for (int i = 0; i < 6; i++ ) {
            temp = ntohl(*(int32*)&Response[12 + i * 4]);
            if (programStart) {
                InitialFTData[i] = (double)((double)temp/1000000);
                CMN_LOG_CLASS_INIT_DEBUG << "Initial data: " << InitialFTData[i] << " ";
            }
            RawFTData[i]= (double)((double)temp/1000000);
            FTData[i] = RawFTData[i] - InitialFTData[i];
            FTData.SetValid(true);
        }
    }
    else {
        CMN_LOG_CLASS_RUN_ERROR << "GetReadings: UDP receive failed" << std::endl;
        FTData.SetValid(false);
        FTData.Zeros();
    }
    programStart = false;
}

void mtsATINetFTSensor::RebiasFTData(void)
{
    InitialFTData.Assign(RawFTData);
    EventTriggers.RobotErrorMsg(std::string("Sensor ReBiased"));
    CMN_LOG_CLASS_RUN_VERBOSE << "FT Sensor Rebiased " << std::endl;
}

bool mtsATINetFTSensor::IsSaturated(void)
{
    vct6 MaxLoads = NetFTConfig.NetFT.GenInfo.Ranges;
    Saturated = false;
    for (size_t i = 0; i < MaxLoads.size(); ++i) {
        if (FTData[i] > 0 && FTData[i] > MaxLoads[i] ||
            (FTData[i] < 0 && FTData[i] < -MaxLoads[i]) )  {
            EventTriggers.RobotErrorMsg(std::string("Sensor Saturated"));
            Saturated = true;
        }
    }
    return Saturated;
}
