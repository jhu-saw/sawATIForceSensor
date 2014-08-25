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

#include <sawATINetFT/devNetFTSensorATITask.h>

CMN_IMPLEMENT_SERVICES(devNetFTSensorATITask);

devNetFTSensorATITask::devNetFTSensorATITask(const std::string & taskName, double period):
mtsTaskPeriodic(taskName, period, false, 5000),
TimeServer(mtsTaskManager::GetInstance()->GetTimeServer())
{
    FTSensorATI = new devNetFTSensorATI();    

    FTData.AutomaticTimestamp() = false;

    FTData.SetSize(6);

    /*** Provided Interface ***/
    mtsInterfaceProvided *ftInterface	= AddInterfaceProvided("ProvidesATINetFTSensor");

    StateTable.AddData(FTData, "FTData");
    StateTable.AddData(IsSaturated, "IsSaturated");
    StateTable.AddData(IsConnected, "IsConnected");        

    if(ftInterface)
    {
        ftInterface->AddCommandReadState(StateTable, FTData, "GetFTData");
        ftInterface->AddCommandReadState(StateTable, IsSaturated, "GetIsSaturated");
        ftInterface->AddCommandReadState(StateTable, IsConnected, "GetFTConnection");

        ftInterface->AddCommandRead(&devNetFTSensorATITask::GetMaxLoads, this, "GetMaxLoads", mtsDoubleVec(6));
        ftInterface->AddCommandWrite(&devNetFTSensorATITask::SetTransform, this, "SetTransform", mtsDoubleVec(6));
        ftInterface->AddCommandVoid(&devNetFTSensorATITask::Rebias, this, "RebiasFTData");
        ftInterface->AddCommandRead(&mtsStateTable::GetIndexReader, &StateTable, "GetTableIndex");

        ftInterface->AddEventWrite(EventTriggers.RobotErrorMsg, "RobotErrorMsg", std::string(""));
    }

    StateTable.Advance();

    /*** Required Interface ***/
    mtsInterfaceRequired *socketInterface	= AddInterfaceRequired("RequiresSocket");

    if(socketInterface)
    {
        socketInterface->AddFunction("GetFTData", GetFTReadings);
        socketInterface->AddFunction("RebiasFTValues", RebiasFTValues);        
        socketInterface->AddFunction("GetSocketStatus", GetSocketStatus);
    }
}

void devNetFTSensorATITask::Rebias()
{
    RebiasFTValues();
    EventTriggers.RobotErrorMsg(std::string("Sensor ReBiased"));
    CMN_LOG_CLASS_INIT_WARNING << "FT Sensor Rebiased " << std::endl;
}

void devNetFTSensorATITask::Configure(const std::string & filename)
{
    if ( !FTSensorATI->LoadCalibrationFile(filename) )
        CMN_LOG_CLASS_INIT_ERROR << "Problems with loading NetFT Calibration file" << std::endl;

}

void devNetFTSensorATITask::Run(void)
{
    ProcessQueuedCommands();
    GetSocketStatus(IsConnected);
    TimeStamp = TimeServer.GetAbsoluteTimeInSeconds();
    if(IsConnected)
    {
        GetFTReadings(FTData);
        FTData.Timestamp() = TimeStamp;
        SaturationCheck();
    }
}

void devNetFTSensorATITask::Startup(void)
{
    StateTable.Advance();
}

void devNetFTSensorATITask::Cleanup(void)
{

}

void devNetFTSensorATITask::GetMaxLoads(mtsDoubleVec &values) const
{
    if( values.size() != 6 )
    {
        CMN_LOG_CLASS_INIT_VERBOSE << "GetNetFTSensorMaxLoads argument Wrong size : " << this->GetName() << std::endl;
        return;
    }

    CMN_LOG_CLASS_INIT_VERBOSE << "Getting NetFTSensorMaxLoads " << this->GetName() << std::endl;
    for( unsigned int i=0; i<6; i++)
        values[i] = FTSensorATI->GetMaxLoad(i);

}

void devNetFTSensorATITask::SetTransform(const mtsDoubleVec &values){


    if(values.size()>6){
        CMN_LOG_CLASS_INIT_ERROR<< "ATI NetFT Sensor  Transform : wrong size" << std::endl;
        return;
    }

    float transform[6];

    for(unsigned int i=0;i<6;i++){
        transform[i]=values[i];
    }

    FTSensorATI->ToolTransform(transform, "mm", "degrees");

}

void devNetFTSensorATITask::SaturationCheck()
{
    IsSaturated.Data = false;
    for( int i=0; i<6; i++)
    {
        if( abs(FTData[i]) >= FTSensorATI->GetMaxLoad(i))
        {
            IsSaturated.Data = true;
//            CMN_LOG_CLASS_INIT_ERROR<< "ATI NetFT Sensor :- Saturated " << std::endl;
        }       
    }    

    if(IsSaturated.Data)
    {
        EventTriggers.RobotErrorMsg(std::string("Saturated"));
    }
}
