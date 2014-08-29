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

#include <cisstCommon.h>
#include <cisstOSAbstraction.h>
#include <cisstMultiTask/mtsQtApplication.h>
#include <sawTextToSpeech/mtsTextToSpeech.h>

#include <sawATINetFT/mtsATINetFTSensor.h>

// Qt include
#include <sawATINetFT/mtsATINetFTQtWidget.h>


const std::string GlobalComponentManagerAddress = "127.0.0.1";

int main(int argc, char ** argv)
{
    cmnLogger::SetMask(CMN_LOG_ALLOW_ALL);
    cmnLogger::AddChannel(std::cout, CMN_LOG_ALLOW_ALL);

    mtsComponentManager * componentManager;
    try {
        componentManager = mtsManagerLocal::GetInstance(GlobalComponentManagerAddress, "ATINetFTSensorProcess");
    } catch (...) {
        CMN_LOG_INIT_WARNING << "Failed to initialize Global component manager" << std::endl;
        CMN_LOG_INIT_WARNING << "Running in local mode" << std::endl;
        componentManager = mtsComponentManager::GetInstance();
    }

    // create a Qt application and tab to hold all widgets
    mtsQtApplication *qtAppTask = new mtsQtApplication("QtApplication", argc, argv);
    qtAppTask->Configure();
    componentManager->AddComponent(qtAppTask);

    mtsATINetFTSensor * forceSensor = new mtsATINetFTSensor("ATINetFTTask");       // Continuous
    mtsATINetFTQtWidget * netFTGui = new mtsATINetFTQtWidget("ATINetFTGUI");
    mtsTextToSpeech * textToSpeech = new mtsTextToSpeech;

    textToSpeech->AddInterfaceRequiredForEventString("ErrorMsg", "RobotErrorMsg");
    //    textToSpeech->SetPreemptive(true);

    forceSensor->SetIPAddress("192.168.1.1");      // IP address of the FT sensor
    //     netFTLogger->SetSavePath("/home/kraven/dev/cisst_nri/build/cisst/bin/");

    componentManager->AddComponent(netFTGui);
    componentManager->AddComponent(forceSensor);
    // componentManager->AddComponent(netFTLogger);
    componentManager->AddComponent(textToSpeech);


    componentManager->Connect(textToSpeech->GetName(), "ErrorMsg", "ATINetFTTask", "ProvidesATINetFTSensor");

    componentManager->Connect("ATINetFTGUI"      , "RequiresATINetFTSensor",
                              "ATINetFTTask"     , "ProvidesATINetFTSensor");

    componentManager->CreateAllAndWait(2.0 * cmn_s);
    componentManager->StartAllAndWait(2.0 * cmn_s);

    componentManager->KillAllAndWait(2.0 * cmn_s);
    componentManager->Cleanup();

    delete forceSensor;

    cmnLogger::Kill();

    return 0;
}
