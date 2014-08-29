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

#ifndef _mtsATINetFTConfig_h
#define _mtsATINetFTConfig_h

#include <iostream>
#include <cisstVector.h>

class mtsATINetFTConfig : public cmnGenericObject
{
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_ALLOW_ERRORS_AND_WARNINGS);

public:
    mtsATINetFTConfig();
    ~mtsATINetFTConfig();

    bool LoadCalibrationFile(const std::string & calFile);
    bool ParseCalibrationFile(const std::string & calFile);

protected:
    vct6 StrToVec(const std::string & strArray, char delim);

private:
    struct CalibrationInfo {
        std::string SerialNumber;
        std::string BodyStyle;
        std::string CalibrationPartNumber;
        std::string Family;
        std::string CalibrationDate;
        vctDoubleMat Matrix;
        vct6 GaugeOffsets;
        vct6 CalibrationIndex;
    };

    struct GeneralInfo {
        std::string ForceUnits;
        std::string TorqueUnits;
        double CountsPerForce;
        double CountsPerTorque;
        vct6 MaxRatings;
        vct6 Resolutions;
        vct6 Ranges;
        vct6 ScaleFactors16Bit;
    };

public:
    struct {
        CalibrationInfo CalibInfo;
        GeneralInfo GenInfo;
    } NetFT;

};

CMN_DECLARE_SERVICES_INSTANTIATION(mtsATINetFTConfig)

#endif // _mtsATINetFTConfig_h
