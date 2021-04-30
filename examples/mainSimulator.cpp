/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */
/* $Id: main.cpp,v 1.10 2015/01/29 04:50:54 mbalicki Exp $ */
/*
 Author(s):  Marcin Balicki
 Created on: 2015-04-30
 
 (C) Copyright 2004-2015 Johns Hopkins University (JHU), All Rights
 Reserved.
 
 --- begin cisst license - do not edit ---
 
 This software is provided "as is" under an open source license, with
 no warranty.  The complete license can be found in license.txt and
 http://www.cisst.org/cisst/license.txt.
 
 --- end cisst license ---
 */


#include <sawATIForceSensor/sawATINetFTSimulatorQtWidget.h>
	
#include <cisstOSAbstraction.h>
#include <cisstMultiTask/mtsManagerLocal.h>
#include <cisstMultiTask/mtsQtApplication.h>
#include <cisstMultiTask/mtsCollectorState.h>

#include <cisstCommon/cmnCommandLineOptions.h>

//#define LOG_VERBOSE

int main(int argc, char *argv[])
{
#ifdef LOG_VERBOSE
  cmnLogger::SetMask(CMN_LOG_ALLOW_ALL);
  cmnLogger::SetMaskDefaultLog(CMN_LOG_ALLOW_ALL);
  cmnLogger::AddChannel(std::cout, CMN_LOG_ALLOW_ALL);
  cmnLogger::SetMaskClass("sawATINetFTSimulatorQtWidget", CMN_LOG_LOD_RUN_VERBOSE);
#else
  cmnLogger::SetMask(CMN_LOG_ALLOW_ALL);
  cmnLogger::AddChannel(std::cout, CMN_LOG_ALLOW_ERRORS_AND_WARNINGS);
  cmnLogger::SetMaskClass("sawATINetFTSimulatorQtWidget", CMN_LOG_ALLOW_ERRORS_AND_WARNINGS);
 
#endif

  
  // parse options
  cmnCommandLineOptions options;
  std::string serverIP = "127.0.0.1";
  int serverPort = 5555;
  
  options.AddOptionOneValue("i", "ip",
                            "Force sensor Interface server IP address",
                            cmnCommandLineOptions::OPTIONAL_OPTION, &serverIP);
  
  options.AddOptionOneValue("p", "port",
                            "Force sensor Interface server Port Number",
                            cmnCommandLineOptions::OPTIONAL_OPTION, &serverPort);
  
  vct6 limits(50.0,50.0,70.0,500.0,500.0,500.0);
  
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
  
  
  std::string errorMessage;
  if (!options.Parse(argc, argv, errorMessage)) {
    std::cerr << "Error: " << errorMessage << std::endl;
    options.PrintUsage(std::cerr);
    return -1;
  }
  

  std::string processname = "ati-ft-sim";
  mtsManagerLocal * componentManager = 0;

  try {
      componentManager = mtsManagerLocal::GetInstance();
  } catch(...) {
      std::cerr << "Failed to get GCM instance." << std::endl;
      return -1;
  }

  // create a Qt application and tab to hold all widgets
  mtsQtApplication * qtAppTask = new mtsQtApplication("QtApplication", argc, argv);
  qtAppTask->Configure();
  componentManager->AddComponent(qtAppTask);

  // QApplication application(argc, argv);
  // application.setStyle("Plastique");
  
  //the name by which this task can be found, set the actuator number to 2
  sawATINetFTSimulatorQtWidget      *atiSimulator  = new sawATINetFTSimulatorQtWidget(1 * cmn_ms, serverIP, serverPort);
  atiSimulator->SetUpperLimits(limits[0],limits[1], limits[2], limits[3], limits[4], limits[5]);
  atiSimulator->SetLowerLimits(-limits[0],-limits[1], -limits[2], -limits[3], -limits[4], -limits[5]);
  
  
  componentManager->AddComponent(atiSimulator);

  componentManager->CreateAll();
  componentManager->WaitForStateAll(mtsComponentState::READY);
  componentManager->StartAll();
  componentManager->WaitForStateAll(mtsComponentState::ACTIVE);

  // kill all tasks and perform cleanup
  componentManager->KillAll();
  componentManager->WaitForStateAll(mtsComponentState::FINISHED, 5.0 * cmn_s);
  componentManager->Cleanup();
  delete qtAppTask; 

  cmnLogger::Kill();
  std::cout << "Quitting." << std::endl;

  return 0;
}
