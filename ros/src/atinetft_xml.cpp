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
#include <cisstCommon/cmnQt.h>
#include <cisstMultiTask/mtsTaskManager.h>
#include <sawATIForceSensor/mtsATINetFTSensor.h>
#include <sawATIForceSensor/mtsATINetFTQtWidget.h>

#include <cisst_ros_crtk/mts_ros_crtk_bridge.h>

#include <QApplication>


int main(int argc, char ** argv)
{
    // log configuration
    cmnLogger::SetMask(CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskDefaultLog(CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskClass("mtsATINetFTSensor", CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskFunction(CMN_LOG_ALLOW_ALL);
    cmnLogger::AddChannel(std::cerr, CMN_LOG_ALLOW_ERRORS_AND_WARNINGS);

    // create ROS node handle
    ros::init(argc, argv, "atift", ros::init_options::AnonymousName);
    ros::NodeHandle rosNodeHandle;

    // parse options
    cmnCommandLineOptions options;
    std::string configFile = "";
    std::string ftip = "192.168.1.8";
    int customPort = 0;
    double socketTimeout = 10 * cmn_ms;
    double rosPeriod = 10.0 * cmn_ms;
    std::list<std::string> managerConfig;

    options.AddOptionOneValue("c", "configuration",
                              "XML configuration file",
                              cmnCommandLineOptions::OPTIONAL_OPTION, &configFile);
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
    options.AddOptionMultipleValues("m", "component-manager",
                                    "JSON files to configure component manager",
                                    cmnCommandLineOptions::OPTIONAL_OPTION, &managerConfig);
    options.AddOptionNoValue("D", "dark-mode",
                             "replaces the default Qt palette with darker colors");

    // check that all required options have been provided
    std::string errorMessage;
    if (!options.Parse(argc, argv, errorMessage)) {
        std::cerr << "Error: " << errorMessage << std::endl;
        options.PrintUsage(std::cerr);
        return -1;
    }
    std::string arguments;
    options.PrintParsedArguments(arguments);
    std::cout << "Options provided:" << std::endl << arguments << std::endl;

    mtsManagerLocal * componentManager = mtsManagerLocal::GetInstance();

    mtsATINetFTSensor * forceSensor = new mtsATINetFTSensor("ForceSensor");
    forceSensor->SetIPAddress(ftip);
    if(customPort) {
        forceSensor->Configure(configFile, socketTimeout, customPort);
    } else {
        forceSensor->Configure(configFile, socketTimeout);
    }
    componentManager->AddComponent(forceSensor);

    // create a Qt user interface
    QApplication application(argc, argv);
    cmnQt::QApplicationExitsOnCtrlC();
    if (options.IsSet("dark-mode")) {
        cmnQt::SetDarkMode();
    }

    mtsATINetFTQtWidget * forceSensorGUI = new mtsATINetFTQtWidget("ATINetFTGUI");
    componentManager->AddComponent(forceSensorGUI);
    componentManager->Connect("ATINetFTGUI", "RequiresATINetFTSensor",
                              "ForceSensor", "ProvidesATINetFTSensor");

    // ROS CRTK bridge
    mts_ros_crtk_bridge * crtk_bridge
        = new mts_ros_crtk_bridge("atift_crtk_bridge", &rosNodeHandle);
    crtk_bridge->bridge_interface_provided(forceSensor->GetName(),
                                           "ProvidesATINetFTSensor",
                                           "", // ros sub namespace
                                           rosPeriod);
    componentManager->AddComponent(crtk_bridge);
    crtk_bridge->Connect();

    // create and start all components
    componentManager->CreateAllAndWait(5.0 * cmn_s);
    componentManager->StartAllAndWait(5.0 * cmn_s);

    // run Qt user interface
    forceSensorGUI->show();
    application.exec();

    // kill all components and perform cleanup
    componentManager->KillAllAndWait(5.0 * cmn_s);
    componentManager->Cleanup();

    delete forceSensor;

    cmnLogger::Kill();

    return 0;
}
