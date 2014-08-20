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

#ifndef _loggerTask_h
#define _loggerTask_h

#include <string>
#include <iostream>
#include <cisstMultiTask.h>
#include <cisstOSAbstraction.h>

class loggerTask: public mtsTaskPeriodic {
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_LOD_RUN_ERROR);

public:
    loggerTask(const std::string & taskName,  double period);
    ~loggerTask() ;

    void Startup(void);
    void Run(void);
    void Configure(const std::string & filename);
    void Cleanup(void) {};

    void WriteData(const mtsStdString &note);
    void SetFileName(const mtsStdString & fileNameBase);
    void SetLogEnabled(const mtsBool &enable);

    void SetSavePath(const std::string & path);

protected:
    void EnableLog();
    void DisableLog();
    bool OpenFiles(const std::string &fileNameBase);
    void CloseFiles(void);

private:    
    mtsFunctionRead GetFTData;

    mtsDoubleVec ftReadings;
    mtsBool LogEnabled;

    char Delim;
    std::string FileNameBase;
    std::string SavePath;

    std::ofstream   LogFile;
    std::ofstream   LogFileNotes;

};

CMN_DECLARE_SERVICES_INSTANTIATION(loggerTask);

#endif // _loggerTask_h
