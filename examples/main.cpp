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
#include <sawATINetFT/mtsATINetFTLogger.h>

// Qt include
#include <sawATINetFT/mtsATINetFTQtWidget.h>


const std::string GlobalComponentManagerAddress = "127.0.0.1";

int main (int argc, char ** argv)
{

    cmnLogger::SetMask(CMN_LOG_ALLOW_ALL);
    cmnLogger::AddChannel(std::cout, CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskClassMatching("osaTimeServer", CMN_LOG_ALLOW_ERRORS_AND_WARNINGS);
    cmnLogger::SetMaskClass("cmnThrow", CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskClass("mtsCollectorState", CMN_LOG_ALLOW_ALL);

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
    netFTTask->SetIPAddress("192.168.1.1");      // IP address of the FT sensor
    netFTTask->Configure("/home/kraven/dev/cisst_nri/source/sawATINetFT/examples/FT15360Net.xml");
    taskManager->AddComponent(netFTTask);

    mtsATINetFTQtWidget *netFTGui = new mtsATINetFTQtWidget("ATINetFTGUI");
    taskManager->AddComponent(netFTGui);

    mtsATINetFTLogger *netFTLogger = new mtsATINetFTLogger("ATINetFTLogger", 0.02);
    netFTLogger->SetSavePath("/home/kraven/dev/cisst_nri/build/cisst/bin/");
    taskManager->AddComponent(netFTLogger);

    mtsTextToSpeech* textToSpeech = new mtsTextToSpeech;
    textToSpeech->AddInterfaceRequiredForEventString("ErrorMsg", "RobotErrorMsg");
    textToSpeech->SetPreemptive(true);
    taskManager->AddComponent(textToSpeech);

    mtsCollectorState *stateCollector = new mtsCollectorState(netFTTask->GetName(),
                                                         netFTTask->GetDefaultStateTableName(),
                                                         mtsCollectorBase::COLLECTOR_FILE_FORMAT_CSV);

    stateCollector->AddSignal("FTData");
    taskManager->AddComponent(stateCollector);
    stateCollector->UseSeparateLogFileDefault();
    stateCollector->Connect();

    taskManager->Connect(textToSpeech->GetName(), "ErrorMsg", "ATINetFTTask", "ProvidesATINetFTSensor");

    taskManager->Connect("ATINetFTGUI"      , "RequiresATINetFTSensor",
                         "ATINetFTTask"     , "ProvidesATINetFTSensor");

    taskManager->Connect("ATINetFTGUI"      , "RequiresFTLogger",
                         "ATINetFTLogger"   , "ProvidesFTLogger");

    taskManager->Connect("ATINetFTLogger"   , "RequiresATINetFTSensor",
                         "ATINetFTTask"     , "ProvidesATINetFTSensor");


    // create and start all tasks
    stateCollector->StartCollection(0.0);

    taskManager->CreateAll();
    taskManager->WaitForStateAll(mtsComponentState::READY);
    taskManager->StartAll();
    taskManager->WaitForStateAll(mtsComponentState::ACTIVE);

    osaSleep(5*cmn_s);
    stateCollector->StopCollection(0.0);

    // kill all tasks and perform cleanup
    taskManager->KillAll();
    taskManager->WaitForStateAll(mtsComponentState::FINISHED, 2.0 * cmn_s);
    taskManager->Cleanup();

    delete netFTLogger;
    delete netFTGui;
    delete netFTTask;
    delete netFTGui;

    return 0;
}
