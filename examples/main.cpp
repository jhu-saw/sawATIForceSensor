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

//#include <stdlib.h>
#include <cisstCommon/cmnCommandLineOptions.h>
#include <cisstMultiTask/mtsQtApplication.h>
#include <cisstMultiTask/mtsCollectorState.h>

#include <sawTextToSpeech/mtsTextToSpeech.h>
#include <sawATIForceSensor/mtsATINetFTSensor.h>
#include <sawATIForceSensor/mtsATINetFTQtWidget.h>

int main(int argc, char ** argv)
{
    cmnLogger::SetMask(CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskDefaultLog(CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskClass("mtsATINetFT", CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskFunction(CMN_LOG_ALLOW_ALL);
    cmnLogger::AddChannel(std::cerr, CMN_LOG_ALLOW_ERRORS_AND_WARNINGS);

    // parse options
    cmnCommandLineOptions options;
    std::string gcmip = "-1";
    std::string configFile = "";
    std::string ftip = "192.168.1.8";
    int customPort = 0;
    double socketTimeout = 10 * cmn_ms;

    options.AddOptionOneValue("c", "configuration",
                              "XML configuration file",
                              cmnCommandLineOptions::REQUIRED_OPTION, &configFile);

    options.AddOptionOneValue("i", "ftip",
                              "Force sensor IP address",
                              cmnCommandLineOptions::OPTIONAL_OPTION, &ftip);

    options.AddOptionOneValue("p", "customPort",
                              "Custom Port Number",
                              cmnCommandLineOptions::OPTIONAL_OPTION, &customPort);

    options.AddOptionOneValue("t", "timeout",
                              "Socket send/receive timeout",
                              cmnCommandLineOptions::OPTIONAL_OPTION, &socketTimeout);

    options.AddOptionOneValue("g", "gcmip",
                              "global component manager IP address",
                              cmnCommandLineOptions::OPTIONAL_OPTION, &gcmip);

    std::string errorMessage;
    if (!options.Parse(argc, argv, errorMessage)) {
        std::cerr << "Error: " << errorMessage << std::endl;
        options.PrintUsage(std::cerr);
        return -1;
    }

    std::string processname = "ati-ft";
    mtsManagerLocal * componentManager = 0;
    if (gcmip != "-1") {
        try {
            componentManager = mtsManagerLocal::GetInstance(gcmip, processname);
        } catch(...) {
            std::cerr << "Failed to get GCM instance." << std::endl;
            return -1;
        }
    } else {
        componentManager = mtsManagerLocal::GetInstance();
    }

    // create a Qt application and tab to hold all widgets
    mtsQtApplication * qtAppTask = new mtsQtApplication("QtApplication", argc, argv);
    qtAppTask->Configure();
    componentManager->AddComponent(qtAppTask);

    mtsATINetFTSensor * forceSensor = new mtsATINetFTSensor("ForceSensor");       // Continuous
    forceSensor->SetIPAddress(ftip);      // IP address of the FT sensor
    if(customPort)
        forceSensor->Configure(configFile, socketTimeout, customPort);
    else
        forceSensor->Configure(configFile, socketTimeout);

    componentManager->AddComponent(forceSensor);

    mtsATINetFTQtWidget * forceSensorGUI = new mtsATINetFTQtWidget("ATINetFTGUI");
    componentManager->AddComponent(forceSensorGUI);

    mtsTextToSpeech * textToSpeech = new mtsTextToSpeech;
    textToSpeech->AddInterfaceRequiredForEventString("ErrorMsg", "ErrorMsg");
    textToSpeech->SetPreemptive(true);
    componentManager->AddComponent(textToSpeech);

    componentManager->Connect(textToSpeech->GetName(), "ErrorMsg", "ForceSensor", "ProvidesATINetFTSensor");

    componentManager->Connect("ATINetFTGUI", "RequiresATINetFTSensor",
                              "ForceSensor", "ProvidesATINetFTSensor");

    componentManager->CreateAll();
    componentManager->WaitForStateAll(mtsComponentState::READY);
    componentManager->StartAll();
    componentManager->WaitForStateAll(mtsComponentState::ACTIVE);

    // kill all tasks and perform cleanup
    componentManager->KillAll();
    componentManager->WaitForStateAll(mtsComponentState::FINISHED, 2.0 * cmn_s);
    componentManager->Cleanup();

    delete forceSensor;

    cmnLogger::Kill();

    return 0;
}
