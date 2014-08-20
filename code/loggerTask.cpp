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

#include <math.h>
#include <cisstOSAbstraction/osaGetTime.h>
#include <iomanip>
#include <time.h>
#include <sawATINetFT/loggerTask.h>

CMN_IMPLEMENT_SERVICES(loggerTask);

loggerTask::loggerTask(const std::string & taskName,double period):
    mtsTaskPeriodic(taskName, period, false, 1000)
{
    Delim= ',';
    FileNameBase = std::string("");

    SavePath = std::string("/home/rems/exp/Logs/NetFT/");
    LogEnabled = false;

    ftReadings.SetSize(6);    
    StateTable.AddData(LogEnabled, "LogEnabled");

    mtsInterfaceProvided *logInterface = AddInterfaceProvided("ProvidesFTLogger");
    if(logInterface)
    {        
        logInterface->AddCommandReadState(StateTable,LogEnabled , "GetLogEnabled");
        logInterface->AddCommandWrite(&loggerTask::SetLogEnabled, this, "SetLogEnabled", mtsBool());
        logInterface->AddCommandWrite(&loggerTask::SetFileName, this, "SetFileName", mtsStdString());
        logInterface->AddCommandWrite(&loggerTask::WriteData, this, "WriteData", mtsStdString());
    }

    mtsInterfaceRequired * ftInterface = AddInterfaceRequired("RequiresATINetFTSensor");
    if(ftInterface)
    {        
        ftInterface->AddFunction("GetFTData", GetFTData);
    }
}

loggerTask::~loggerTask()
{
    CloseFiles();
}

void loggerTask::SetLogEnabled(const mtsBool &enable)
{
    LogEnabled = enable;
    if(!LogEnabled.Data)
        CloseFiles();
}

void loggerTask::WriteData(const mtsStdString &note)
{
    osaAbsoluteTime abstime;

    if(!LogFile.is_open())
        OpenFiles(FileNameBase);

    LogFile << std::setprecision(6) << note.Timestamp() << ";" << note.Data << std::endl;
}

void loggerTask::Startup(void)
{

}

void loggerTask::Run(void)
{
    ProcessQueuedCommands();
    ProcessQueuedEvents();

    osaAbsoluteTime absTime;

    if(!LogEnabled.Data)
        return;

    if (!LogFile.is_open())
        OpenFiles(FileNameBase);


    GetFTData(ftReadings);
    LogFile << std::setprecision(14) << ftReadings.Timestamp() << Delim;

    unsigned int i=0;
    for (i=0; i < ftReadings.size(); i++)
        LogFile << ftReadings[i] << Delim;

    LogFile << std::endl;

}

bool loggerTask::OpenFiles(const std::string &fileNameBase)
{
    if(LogFile.is_open())
        CloseFiles();

    std::string dateTime;
    osaGetDateTimeString(dateTime);
    std::string fileName;

    fileName = SavePath + dateTime + std::string("-") + fileNameBase + std::string("-FTLog") + std::string(".csv");
    LogFile.open(fileName.c_str(), std::ios::out | std::ios::app);


    LogFile<<"TimeStamp"<<Delim;    
    LogFile << "FX" << Delim << "FY" << Delim << "FZ" << Delim
            << "TX" << Delim << "TY" << Delim << "TZ" << Delim;

    LogFile<<std::endl;

    CMN_LOG_CLASS_INIT_VERBOSE << "Log file Opened : " << fileName << std::endl;

    return true;
}

void loggerTask::SetFileName(const mtsStdString &fileNameBase)
{
    FileNameBase = fileNameBase.Data;
    LogEnabled = false;
    CloseFiles();
}

void loggerTask::CloseFiles(void)
{
    LogFile.close();
    CMN_LOG_CLASS_INIT_VERBOSE << "Log file Closed (If Opened)" << std::endl;
}

void loggerTask::Configure(const std::string &filename)
{
    CMN_LOG_CLASS_INIT_VERBOSE<< "Configuring Logger" << filename << std::endl;
}

void loggerTask::SetSavePath(const std::string &path)
{
    CMN_LOG_CLASS_INIT_VERBOSE<< "Setting path" << path << std::endl;
    SavePath = path;
}
