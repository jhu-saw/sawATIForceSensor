/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  Author(s):  Preetham Chalasani
  Created on: 2013

  (C) Copyright 2013-2021 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---

*/

#include <cisstCommon/cmnCommandLineOptions.h>
#include <cisstMultiTask/mtsQtApplication.h>
#include <cisstMultiTask/mtsCollectorState.h>

#include <sawATIForceSensor/mtsATINetFTSensor.h>
#include <sawATIForceSensor/mtsATINetFTQtWidget.h>

#include <ros/ros.h>
#include <cisst_ros_bridge/mtsROSBridge.h>

int main(int argc, char ** argv)
{
    cmnLogger::SetMask(CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskDefaultLog(CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskClass("mtsATINetFTSensor", CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskFunction(CMN_LOG_ALLOW_ALL);
    cmnLogger::AddChannel(std::cerr, CMN_LOG_ALLOW_ERRORS_AND_WARNINGS);

    // parse options
    cmnCommandLineOptions options;
    std::string configFile = "";
    std::string ftip = "192.168.1.8";
    int customPort = 0;
    double socketTimeout = 10 * cmn_ms;
    double rosPeriod = 10.0 * cmn_ms;

    options.AddOptionOneValue("c", "configuration",
                              "XML configuration file",
                              cmnCommandLineOptions::REQUIRED_OPTION, &configFile);
    options.AddOptionOneValue("i", "ftip",
                              "Force sensor IP address",
                              cmnCommandLineOptions::OPTIONAL_OPTION, &ftip);
    options.AddOptionOneValue("r", "ros-period",
                              "period in seconds to read all tool positions (default 0.01, 10 ms, 100Hz).  There is no point to have a period higher than the tracker component",
                              cmnCommandLineOptions::OPTIONAL_OPTION, &rosPeriod);
    options.AddOptionOneValue("p", "customPort",
                              "Custom Port Number",
                              cmnCommandLineOptions::OPTIONAL_OPTION, &customPort);
    options.AddOptionOneValue("t", "timeout",
                              "Socket send/receive timeout",
                              cmnCommandLineOptions::OPTIONAL_OPTION, &socketTimeout);

    std::string errorMessage;
    if (!options.Parse(argc, argv, errorMessage)) {
        std::cerr << "Error: " << errorMessage << std::endl;
        options.PrintUsage(std::cerr);
        return -1;
    }

    std::string processname = "ati-ft";
    mtsManagerLocal * componentManager = mtsManagerLocal::GetInstance();

    // create a Qt application and tab to hold all widgets
    mtsQtApplication * qtAppTask = new mtsQtApplication("QtApplication", argc, argv);
    qtAppTask->Configure();
    componentManager->AddComponent(qtAppTask);

    mtsATINetFTSensor * forceSensor = new mtsATINetFTSensor("ForceSensor");       // Continuous
    forceSensor->SetIPAddress(ftip);      // IP address of the FT sensor
    if(customPort) {
        forceSensor->Configure(configFile, socketTimeout, customPort);
    } else {
        forceSensor->Configure(configFile, socketTimeout);
    }
    componentManager->AddComponent(forceSensor);

    mtsATINetFTQtWidget * forceSensorGUI = new mtsATINetFTQtWidget("ATINetFTGUI");
    componentManager->AddComponent(forceSensorGUI);
    componentManager->Connect("ATINetFTGUI", "RequiresATINetFTSensor",
                              "ForceSensor", "ProvidesATINetFTSensor");

    // Ros Bridge
    std::string bridgeName = "sawATIForceSensor";
    mtsROSBridge * rosBridge = new mtsROSBridge(bridgeName,
                                                rosPeriod, true);
    // configure the bridge
    rosBridge->AddPublisherFromCommandRead<prmForceCartesianGet, geometry_msgs::WrenchStamped>
        (forceSensor->GetName(), "GetForceTorque", "wrench");

    rosBridge->AddPublisherFromCommandRead<mtsDoubleVec, geometry_msgs::WrenchStamped>
        (forceSensor->GetName(), "GetFTData", "raw_wrench");

    componentManager->AddComponent(rosBridge);
    componentManager->Connect(bridgeName, forceSensor->GetName(),
                              forceSensor->GetName(), "ProvidesATINetFTSensor");

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
