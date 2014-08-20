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

#include <sawATINetFT/clientSocket.h>
#include <sawATINetFT/loggerTask.h>
#include <sawATINetFT/devNetFTSensorATITask.h>


// Qt include
#include <sawATINetFT/mtsATINetFTQtWidget.h>



const std::string GlobalComponentManagerAddress = "127.0.0.1";

int main (int argc, char ** argv)
{

    cmnLogger::SetMask(CMN_LOG_ALLOW_ALL);
    cmnLogger::AddChannel(std::cout, CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskClassMatching("devNetFTSensorATITask", CMN_LOG_ALLOW_VERBOSE | CMN_LOG_ALLOW_ERRORS_AND_WARNINGS);
    cmnLogger::SetMaskClassMatching("devNetFTSensorATI", CMN_LOG_ALLOW_VERBOSE | CMN_LOG_ALLOW_ERRORS_AND_WARNINGS);
    cmnLogger::SetMaskClassMatching("osaTimeServer", CMN_LOG_ALLOW_ERRORS_AND_WARNINGS);
    cmnLogger::SetMaskClass("cmnThrow", CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskClass("osaSocket", CMN_LOG_LEVEL_NONE);

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

    devNetFTSensorATITask *ATINetFtTask = new devNetFTSensorATITask("AtiNetFtTask", 0.002);
    clientSocket *socket = new clientSocket("SocketConnect", 0.001);       // Continuous    
    mtsATINetFTQtWidget *sampleGUI = new mtsATINetFTQtWidget("ATINetFTGUI");
//    loggerTask *logger = new loggerTask("ForceLogger", 0.02);

    socket->SetIPAddress("192.168.1.1");      // IP address of the FT sensor

    taskManager->AddComponent(sampleGUI);        
    taskManager->AddComponent(ATINetFtTask);
    taskManager->AddComponent(socket);
//    taskManager->AddComponent(logger);


    taskManager->Connect("ATINetFTGUI", "RequiresATINetFTSensor",
                         "AtiNetFtTask","ProvidesATINetFTSensor");

    taskManager->Connect("AtiNetFtTask","RequiresSocket",
                         "SocketConnect","ProvidesSocket");


//    taskManager->Connect("ForceLogger","RequiresATINetFTSensor",
//                         "AtiNetFtTask","ProvidesATINetFTSensor");


    taskManager->Connect("ATINetFTSensorProcess","AtiNetFtTask","RequiresLimitsAudio",
                          "AudioProc","LimitsAudio","ProvidesLimitsAudio",3);


    ATINetFtTask->Configure("/home/kraven/dev/cisst_nri/source/sawATINetFT/examples/NetFT15360.cal");

    taskManager->CreateAllAndWait(2.0 * cmn_s);
    taskManager->StartAllAndWait(2.0 * cmn_s);


    taskManager->KillAllAndWait(2.0 * cmn_s);
    taskManager->Cleanup();

    delete ATINetFtTask;

    delete socket;
    delete sampleGUI;

    osaSleep(0.1);

    return 0;
}
