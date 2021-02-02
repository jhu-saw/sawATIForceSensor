/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
 Author(s):  Marcin Balicki
 Created on: 2015-04-30

 (C) Copyright 2004-2021 Johns Hopkins University (JHU), All Rights Reserved.

 --- begin cisst license - do not edit ---

 This software is provided "as is" under an open source license, with
 no warranty.  The complete license can be found in license.txt and
 http://www.cisst.org/cisst/license.txt.

 --- end cisst license ---
 */

#include <sawATIForceSensor/sawATINetFTSimulatorQtWidget.h>
#include <cisstCommon/cmnCommandLineOptions.h>
#include <cisstMultiTask/mtsQtApplication.h>
#include <cisstMultiTask/mtsCollectorState.h>
#include <cisstMultiTask/mtsManagerLocal.h>

#include <sawATIForceSensor/mtsATINetFTSensor.h>
#include <sawATIForceSensor/mtsATINetFTQtWidget.h>

#include <QApplication>


#if (CISST_OS == CISST_DARWIN || CISST_OS == CISST_LINUX)
#include  <sys/types.h>
#include  <signal.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>

void SIGQUIT_Handler(int sig)
{
    signal(sig, SIG_IGN);
    printf("From SIGQUIT: just got a %d (SIGQUIT ^\\) signal and is about to quit\n", sig);
    QApplication::quit();
    printf("Just called Quit\n");
}

void Register_SIGQUIT_Handler(void) {

    if (signal(SIGQUIT, SIGQUIT_Handler) == SIG_ERR) {
        printf("SIGQUIT install error\n");
        exit(2);
    }
}
#endif

//#define LOG_VERBOSE

int main(int argc, char *argv[])
{
#ifdef LOG_VERBOSE
    cmnLogger::SetMask(CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskDefaultLog(CMN_LOG_ALLOW_ALL);
    cmnLogger::AddChannel(std::cout, CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskClass("sawATINetFTSimulatorQtWidget", CMN_LOG_LOD_RUN_VERBOSE);
#else
    cmnLogger::SetMask(CMN_LOG_ALLOW_ERRORS_AND_WARNINGS);
    cmnLogger::SetMaskDefaultLog(CMN_LOG_ALLOW_ERRORS_AND_WARNINGS);
    cmnLogger::AddChannel(std::cout, CMN_LOG_ALLOW_ERRORS_AND_WARNINGS);
    cmnLogger::SetMaskClass("mtsATINetFTSensor", CMN_LOG_ALLOW_VERBOSE);
    cmnLogger::SetMaskClass("mtsATINetFTQtWidget", CMN_LOG_ALLOW_VERBOSE);
#endif


    cmnLogger::SetMask(CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskDefaultLog(CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskClass("sawATINetFTSimulatorQtWidget", CMN_LOG_LOD_RUN_VERBOSE);
    cmnLogger::SetMaskFunction(CMN_LOG_ALLOW_ALL);
    cmnLogger::AddChannel(std::cerr, CMN_LOG_LOD_RUN_VERBOSE);
    cmnLogger::AddChannel(std::cout, CMN_LOG_LOD_RUN_VERBOSE);


    // parse options
    cmnCommandLineOptions options;
    std::string serverIP = "127.0.0.1";
    int serverPort = 5555;
    std::string configFile = "";
    double socketTimeout = 10 * cmn_ms;
    vct6 limits(50.0, 50.0, 70.0, 500.0, 500.0, 500.0);

    options.AddOptionOneValue("p", "port",
                              "Force sensor Interface server Port Number",
                              cmnCommandLineOptions::OPTIONAL_OPTION, &serverPort);
    options.AddOptionOneValue("x", "fx",
                              "Max abs force in x",
                              cmnCommandLineOptions::OPTIONAL_OPTION, &limits[0]);
    options.AddOptionOneValue("y", "fy",
                              "Max abs force in y",
                              cmnCommandLineOptions::OPTIONAL_OPTION, &limits[1]);
    options.AddOptionOneValue("z", "fz",
                              "Max abs force in z",
                              cmnCommandLineOptions::OPTIONAL_OPTION, &limits[2]);
    options.AddOptionOneValue("h", "tx",
                              "Max abs torque in x",
                              cmnCommandLineOptions::OPTIONAL_OPTION, &limits[3]);
    options.AddOptionOneValue("j", "ty",
                              "Max abs torque in y",
                              cmnCommandLineOptions::OPTIONAL_OPTION, &limits[4]);
    options.AddOptionOneValue("k", "tz",
                              "Max abs torque in z",
                              cmnCommandLineOptions::OPTIONAL_OPTION, &limits[5]);
    options.AddOptionOneValue("c", "configuration",
                              "XML configuration file",
                              cmnCommandLineOptions::REQUIRED_OPTION, &configFile);
    options.AddOptionOneValue("t", "timeout",
                              "Socket send/receive timeout",
                              cmnCommandLineOptions::OPTIONAL_OPTION, &socketTimeout);

    std::string errorMessage;
    if (!options.Parse(argc, argv, errorMessage)) {
        std::cerr << "Error: " << errorMessage << std::endl;
        options.PrintUsage(std::cerr);
        return -1;
    }

    mtsManagerLocal * componentManager = mtsManagerLocal::GetInstance();

    // create a Qt application and tab to hold all widgets
    mtsQtApplication * qtAppTask = new mtsQtApplication("QtApplication", argc, argv);
    qtAppTask->Configure();
    componentManager->AddComponent(qtAppTask);

    mtsATINetFTSensor * forceSensor = new mtsATINetFTSensor("ForceSensor");
    forceSensor->SetIPAddress(serverIP);      // IP address of the FT sensor
    if(serverPort) {
        forceSensor->Configure(configFile, socketTimeout, serverPort);
    } else {
        forceSensor->Configure(configFile, socketTimeout);
    }
    componentManager->AddComponent(forceSensor);

    mtsATINetFTQtWidget * forceSensorGUI = new mtsATINetFTQtWidget("ATINetFTGUI");
    componentManager->AddComponent(forceSensorGUI);
    componentManager->Connect("ATINetFTGUI", "RequiresATINetFTSensor",
                              "ForceSensor", "ProvidesATINetFTSensor");

    //the name by which this task can be found, set the actuator number to 2
    sawATINetFTSimulatorQtWidget * atiSimulator = new sawATINetFTSimulatorQtWidget(1 * cmn_ms, serverIP, serverPort);
    atiSimulator->SetUpperLimits( limits[0],  limits[1],  limits[2],  limits[3],  limits[4],  limits[5]);
    atiSimulator->SetLowerLimits(-limits[0], -limits[1], -limits[2], -limits[3], -limits[4], -limits[5]);
    componentManager->AddComponent(atiSimulator);

#if (CISST_OS == CISST_DARWIN || CISST_OS == CISST_LINUX)
    Register_SIGQUIT_Handler();
#endif

    componentManager->CreateAll();
    componentManager->WaitForStateAll(mtsComponentState::READY);
    componentManager->StartAll();
    componentManager->WaitForStateAll(mtsComponentState::ACTIVE);

    // kill all tasks and perform cleanup
    componentManager->KillAll();
    componentManager->WaitForStateAll(mtsComponentState::FINISHED, 5.0 * cmn_s);
    componentManager->Cleanup();

    delete forceSensor;
    delete qtAppTask;

    cmnLogger::Kill();

    return 0;
}
