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

#include <cisstCommon.h>
#include <cisstOSAbstraction.h>
#include <cisstMultiTask/mtsQtApplication.h>
#include <sawTextToSpeech/mtsTextToSpeech.h>

#include <sawATINetFT/mtsATINetFTSensor.h>
#include <sawATINetFT/loggerTask.h>

// Qt include
#include <sawATINetFT/mtsATINetFTQtWidget.h>


const std::string GlobalComponentManagerAddress = "127.0.0.1";

int main (int argc, char ** argv)
{

    cmnLogger::SetMask(CMN_LOG_ALLOW_ALL);
    cmnLogger::AddChannel(std::cout, CMN_LOG_ALLOW_ALL);    
    cmnLogger::SetMaskClassMatching("osaTimeServer", CMN_LOG_ALLOW_ERRORS_AND_WARNINGS);
    cmnLogger::SetMaskClass("cmnThrow", CMN_LOG_ALLOW_ALL);    

    mtsTaskManager * taskManager;
    try {
        taskManager = mtsManagerLocal::GetInstance(GlobalComponentManagerAddress,"ATINetFTSensorProcess");
    } catch (...) {
        CMN_LOG_INIT_WARNING << "Failed to initialize Global component manager" << std::endl;
        CMN_LOG_INIT_WARNING << "Running in local mode" << std::endl;
        taskManager = mtsTaskManager::GetInstance();
    }

    // create a Qt application and tab to hold all widgets
    mtsQtApplication *qtAppTask = new mtsQtApplication("QtApplication", argc, argv);
    qtAppTask->Configure();
    taskManager->AddComponent(qtAppTask);

    mtsATINetFTSensor *netFTTask = new mtsATINetFTSensor("ATINetFTTask");       // Continuous
    mtsATINetFTQtWidget *netFTGui = new mtsATINetFTQtWidget("ATINetFTGUI");
    loggerTask *netFTLogger = new loggerTask("ATINetFTLogger", 0.02);
    mtsTextToSpeech* textToSpeech = new mtsTextToSpeech;

    textToSpeech->AddInterfaceRequiredForEventString("ErrorMsg", "RobotErrorMsg");
    textToSpeech->SetPreemptive(true);

    netFTTask->SetIPAddress("192.168.1.1");      // IP address of the FT sensor
    netFTLogger->SetSavePath("/home/kraven/dev/cisst_nri/build/cisst/bin/");

    taskManager->AddComponent(netFTGui);
    taskManager->AddComponent(netFTTask);
    taskManager->AddComponent(netFTLogger);
    taskManager->AddComponent(textToSpeech);


    taskManager->Connect(textToSpeech->GetName(), "ErrorMsg", "ATINetFTTask", "ProvidesATINetFTSensor");

    taskManager->Connect("ATINetFTGUI"      , "RequiresATINetFTSensor",
                         "ATINetFTTask"     , "ProvidesATINetFTSensor");

    taskManager->Connect("ATINetFTGUI"      , "RequiresFTLogger",
                         "ATINetFTLogger"   , "ProvidesFTLogger");

    taskManager->Connect("ATINetFTLogger"   , "RequiresATINetFTSensor",
                         "ATINetFTTask"     , "ProvidesATINetFTSensor");

    taskManager->CreateAllAndWait(2.0 * cmn_s);
    taskManager->StartAllAndWait(2.0 * cmn_s);


    taskManager->KillAllAndWait(2.0 * cmn_s);
    taskManager->Cleanup();

    delete netFTLogger;
    delete netFTGui;
    delete netFTTask;
    delete netFTGui;

    osaSleep(0.1);

    return 0;
}
