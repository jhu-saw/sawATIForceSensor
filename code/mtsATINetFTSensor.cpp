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

#include <cisstCommon/cmnConstants.h>

#include <sawATINetFT/mtsATINetFTSensor.h>


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

    mtsInterfaceProvided *socketInterface = AddInterfaceProvided("ProvidesATINetFTSensor");

    StateTable.AddData(FTData, "FTData");
    StateTable.AddData(IsConnected, "IsConneted");

    socketInterface->AddCommandReadState(StateTable, FTData, "GetFTData");
    socketInterface->AddCommandReadState(StateTable, IsConnected, "GetSocketStatus");

    socketInterface->AddCommandVoid(&mtsATINetFTSensor::RebiasFTData, this, "RebiasFTData");

    socketInterface->AddEventWrite(EventTriggers.RobotErrorMsg, "RobotErrorMsg", std::string(""));

    StateTable.Advance();
}

void mtsATINetFTSensor::Startup()
{    
    *(uint16*)&Request[0] = htons(0x1234); /* standard header. */
    *(uint16*)&Request[2] = htons(COMMAND); /* per table 9.1 in Net F/T user manual. */
    *(uint32*)&Request[4] = htonl(NUM_SAMPLES); /* see section 9.1 in Net F/T user manual. */

    Socket.SetDestination(IP, Port);
}

void mtsATINetFTSensor::Cleanup()
{
    Socket.Close();
}

void mtsATINetFTSensor::SetIPAddress(std::string ip)
{
    IP = ip;
}

void mtsATINetFTSensor::Run()
{   
    ProcessQueuedCommands();
    GetReadings();
}

void mtsATINetFTSensor::GetReadings(void)
{    

    int result;
    result = Socket.Send((const char *)Request, 8, 10*cmn_ms);
    if (result == -1){
        CMN_LOG_CLASS_RUN_WARNING << "GetReadings : UDP Send Failed " << std::endl;
        return;
    }

    result = Socket.Receive((char *)Response, 36, 10*cmn_ms);
    if (result != -1){

        this->Rdt_sequence = ntohl(*(uint32*)&Response[0]);
        this->Ft_sequence = ntohl(*(uint32*)&Response[4]);
        this->Status = ntohl(*(uint32*)&Response[8]);
        int temp;
        for( int i = 0; i < 6; i++ ) {
            temp = ntohl(*(int32*)&Response[12 + i * 4]);
            if(programStart)
            {
                InitialFTData[i] = (double)((double)temp/1000000);
                std::cout << InitialFTData[i] << " ";
            }

            RawFTData[i]= (double)((double)temp/1000000);
            FTData[i] = RawFTData[i] - InitialFTData[i];
            FTData.SetValid(true);
        }
    }
    else {
        CMN_LOG_CLASS_RUN_ERROR << "GetReadings : UDP Receive Failed" << std::endl;
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
