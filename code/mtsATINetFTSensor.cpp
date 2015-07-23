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
    ATI_PORT(49152),                /* Port the Net F/T always uses */
    ATI_COMMAND(2),                 /* Command code 2 starts streaming */
    ATI_NUM_SAMPLES(1),              /* Will send 1 sample `before stopping */
    Socket(osaSocket::UDP)
{
    Data = new mtsATINetFTSensorData;
    IsSaturated = false;
    Data->Port = ATI_PORT;

    FTRawData.SetSize(6);
    FTRawData.Zeros();
    FTBiasedData.SetSize(6);
    FTBiasVec.SetSize(6);
    FTBiasVec.Zeros();

    StateTable.AddData(FTBiasedData , "FTData");
    StateTable.AddData(ForceTorque  , "ForceTorque");
    StateTable.AddData(IsConnected  , "IsConnected");
    StateTable.AddData(IsSaturated  , "IsSaturated");
    StateTable.AddData(HasError     , "HasError");
    StateTable.AddData(PercentOfMax , "PercentOfMax");

    mtsInterfaceProvided * interfaceProvided = AddInterfaceProvided("ProvidesATINetFTSensor");
    if (interfaceProvided) {
        interfaceProvided->AddCommandReadState(StateTable, StateTable.PeriodStats, "GetPeriodStatistics");
        interfaceProvided->AddCommandReadState(StateTable, FTBiasedData, "GetFTData");
        interfaceProvided->AddCommandReadState(StateTable, ForceTorque, "GetForceTorque");
        interfaceProvided->AddCommandReadState(StateTable, IsConnected, "GetSocketStatus");
        interfaceProvided->AddCommandReadState(StateTable, IsSaturated, "GetIsSaturated");
        interfaceProvided->AddCommandReadState(StateTable, PercentOfMax, "GetPercentOfMax");
        interfaceProvided->AddCommandReadState(StateTable, HasError, "GetHasError");

        interfaceProvided->AddCommandVoid(&mtsATINetFTSensor::Rebias, this, "Rebias");
        interfaceProvided->AddCommandWrite(&mtsATINetFTSensor::SetFilter, this, "SetFilter", std::string(""));
        interfaceProvided->AddEventWrite(EventTriggers.ErrorMsg, "ErrorMsg", std::string(""));
    }
}

void mtsATINetFTSensor::Startup(void)
{
    if(UseCustomPort) {
        Socket.AssignPort(Data->Port);
    } else {
        *(uint16*)&(Data->Request)[0] = htons(0x1234); /* standard header. */
        *(uint16*)&(Data->Request)[2] = htons(ATI_COMMAND); /* per table 9.1 in Net F/T user manual. */
        *(uint32*)&(Data->Request)[4] = htonl(ATI_NUM_SAMPLES); /* see section 9.1 in Net F/T user manual. */

        Socket.SetDestination(IP, Data->Port);
    }
}

void mtsATINetFTSensor::Configure(const std::string & filename,
                                  bool useCustomPort,
                                  int customPortNumber)
{
    UseCustomPort = useCustomPort;
    if(UseCustomPort) {
        Data->Port = customPortNumber;
    }

    if (NetFTConfig.LoadCalibrationFile(filename)) {
        CMN_LOG_CLASS_RUN_WARNING << "Configure: file loaded - "
                                  << filename << std::endl;
    }
}

void mtsATINetFTSensor::Cleanup(void)
{
    if(!UseCustomPort) {
        *(uint16*)&(Data->Request)[0] = htons(0x1234);
        *(uint16*)&(Data->Request)[2] = htons(0); /* Stop streaming */
        *(uint32*)&(Data->Request)[4] = htonl(ATI_NUM_SAMPLES);

        // try to send, but timeout after 10 ms
        int result = Socket.Send((const char *)(Data->Request), 8, 10.0 * cmn_ms);
        if (result == -1) {
            CMN_LOG_CLASS_RUN_WARNING << "Cleanup: UDP send failed" << std::endl;
            return;
        }
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
    if(UseCustomPort) {
        GetReadingsFromCustomPort();
    } else {
        GetReadings();
    }

    if (IsSaturated) {
        CMN_LOG_CLASS_RUN_WARNING << "Run: sensor saturated" << std::endl;
        FTRawData.SetValid(false);
    }

    // Bias the FT data based on bias vec
    FTBiasedData = FTRawData - FTBiasVec;
    FTBiasedData.Valid() = FTRawData.Valid();
    ForceTorque.SetForce(FTBiasedData);
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
    if (result > 0) {
        this->Data->RdtSequence = ntohl(*(uint32*)&(Data->Response)[0]);
        this->Data->FtSequence = ntohl(*(uint32*)&(Data->Response)[4]);
        this->Data->Status = ntohl(*(uint32*)&(Data->Response)[8]);

        CheckSaturation(this->Data->Status);
        CheckForErrors(this->Data->Status);

        int temp;
        for (int i = 0; i < 6; i++ ) {
            temp = ntohl(*(int32*)&(Data->Response)[12 + i * 4]);
            FTRawData[i]= (double)((double)temp/1000000);
            FTRawData.SetValid(true);
        }
    }
    else {
        FTRawData.SetValid(false);

        // If there are packets missing then the state table will not be updated;
        // when queried previous FT will be returned;
        // If you want FT to be zero, when a packet is missed, uncomment the below line

        // FTRawData.Zeros();
    }
}

void mtsATINetFTSensor::GetReadingsFromCustomPort()
{
    // read force sensor data sending over a udp port
    // read UDP packets
    int bytesRead;
    char buffer[512];
    double *packetReceived;
    
    bytesRead = Socket.Receive(buffer, 56, 40.0* cmn_ms);
    if (bytesRead  > 0) {
        if (bytesRead == 7 * sizeof(double) ) {
            packetReceived = reinterpret_cast<double *>(buffer);
            // Force-Torque values
            for (int i = 0; i < 6; i++ ) {
                FTRawData[i] = packetReceived[i];
                FTRawData.SetValid(true);
            }

            // Error bit
            double error = packetReceived[6];
            if( error == 1)
                IsSaturated = true;
            else
                IsSaturated = false;

        } else {
            std::cerr << "!" << std::flush;
        }
    } else {
        CMN_LOG_CLASS_RUN_DEBUG << "GetReadings: UDP receive from xPC failed" << std::endl;
        FTRawData.SetValid(false);
        // FTRawData.Zeros();
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
    }
}

void mtsATINetFTSensor::SetFilter(const std::string &filterName)
{
    if(filterName == "NoFilter") {
        CurrentFilter = NO_FILTER;
    }
}

void mtsATINetFTSensor::Rebias(void)
{
    FTBiasVec = FTRawData;
    IsRebiasRequested = true;

    EventTriggers.ErrorMsg(std::string("Sensor ReBiased"));
    CMN_LOG_CLASS_RUN_VERBOSE << "FT Sensor Rebiased " << std::endl;
}

void mtsATINetFTSensor::CheckSaturation(const unsigned int status)
{
    if(status == ntohl(0x00020000))
        IsSaturated = true;
    else
        IsSaturated = false;
}

void mtsATINetFTSensor::CheckForErrors(const unsigned int status)
{
    // Check for errors
    if( (status == ntohl(0x00000000)) || (status == ntohl(0x80010000)) )
        HasError = false;
    else
        HasError = true;


    // Update PercentOfMax
    vctDoubleVec PercentOfMaxVec(6);
    PercentOfMaxVec.Assign(FTRawData.Abs());

    for (unsigned int i = 0; i < 6; i++)
        PercentOfMaxVec[i] /= NetFTConfig.GenInfo.Ranges[i];

    //save the largest value.
    PercentOfMax = PercentOfMaxVec.MaxElement();
}
