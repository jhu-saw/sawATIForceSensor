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

#include <sawATIForceSensor/mtsATINetFTConfig.h>

#include <cisstCommon/cmnXMLPath.h>
#include <cisstCommon/cmnPath.h>
#include <cisstCommon/cmnUnits.h>
#include <cisstVector/vctDynamicVectorTypes.h>

CMN_IMPLEMENT_SERVICES(mtsATINetFTConfig);

mtsATINetFTConfig::mtsATINetFTConfig()
{
}

mtsATINetFTConfig::~mtsATINetFTConfig()
{
}

bool mtsATINetFTConfig::LoadCalibrationFile(const std::string & calFile)
{
    if (!ParseCalibrationFile(calFile)) {
        CMN_LOG_CLASS_RUN_WARNING << "LoadCalibrationFile: Parsing failed " << std::endl;
        return false;
    }

    return true;
}

bool mtsATINetFTConfig::ParseCalibrationFile(const std::string & calFile)
{
    cmnXMLPath xmlPath;
    xmlPath.SetInputSource(calFile);

    std::string str;

    // Calibration info
    if (!xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblNetFTCalibrationInfo", "SerialNumber", CalibInfo.SerialNumber, "") ||
        !xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblNetFTCalibrationInfo", "BodyStyle", CalibInfo.BodyStyle, "") ||
        !xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblNetFTCalibrationInfo", "CalibrationPartNumber", CalibInfo.CalibrationPartNumber, "") ||
        !xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblNetFTCalibrationInfo", "Family", CalibInfo.Family, "") ||
        !xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblNetFTCalibrationInfo", "CalibrationDate", CalibInfo.CalibrationDate, "")
        ) {
            return false;
        }

    // Retrieving vct6
    if (!xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblNetFTCalibrationInfo", "GaugeOffsets", str, "")) {
        return false;
    }
    CalibInfo.GaugeOffsets.Assign(StrToVec(str, ' '));

    if (!xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblNetFTCalibrationInfo", "CalibrationIndex", str, "")) {
        return false;
    }
    CalibInfo.CalibrationIndex.Assign(StrToVec(str, ' '));

    // Calibration Matrix
    std::string Fx, Fy, Fz, Tx, Ty, Tz;
    if (!xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblNetFTCalibrationInfo", "MatrixFx", Fx, "") ||
        !xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblNetFTCalibrationInfo", "MatrixFy", Fy, "") ||
        !xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblNetFTCalibrationInfo", "MatrixFz", Fz, "") ||
        !xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblNetFTCalibrationInfo", "MatrixTx", Tx, "") ||
        !xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblNetFTCalibrationInfo", "MatrixTy", Ty, "") ||
        !xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblNetFTCalibrationInfo", "MatrixTz", Tz, "")
        ) {
            return false;
        }

    CalibInfo.Matrix.SetSize(6,6);
    CalibInfo.Matrix.Row(0) = vctDoubleVec(StrToVec(Fx, ' '));
    CalibInfo.Matrix.Row(1) = vctDoubleVec(StrToVec(Fy, ' '));
    CalibInfo.Matrix.Row(2) = vctDoubleVec(StrToVec(Fz, ' '));
    CalibInfo.Matrix.Row(3) = vctDoubleVec(StrToVec(Fx, ' '));
    CalibInfo.Matrix.Row(4) = vctDoubleVec(StrToVec(Fy, ' '));
    CalibInfo.Matrix.Row(5) = vctDoubleVec(StrToVec(Fz, ' '));


    // General info
    if (!xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblCalibrationInformation", "ForceUnits", GenInfo.ForceUnits, "") ||
        !xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblCalibrationInformation", "TorqueUnits", GenInfo.TorqueUnits, "")
        ) {
            return false;
        }

    // Retrieving vct6
    if (!xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblCalibrationInformation", "MaxRatings", str, "")) {
        return false;
    }
    GenInfo.MaxRatings.Assign(StrToVec(str, ' '));

    if (!xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblCalibrationInformation", "Resolutions", str, "")) {
        return false;
    }
    GenInfo.Resolutions.Assign(StrToVec(str, ' '));

    if (!xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblCalibrationInformation", "Ranges", str, "")) {
        return false;
    }
    GenInfo.Ranges.Assign(StrToVec(str, ' '));

    if (!xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblCalibrationInformation", "_x0031_6BitScaleFactors", str, "")) {
        return false;
    }
    GenInfo.ScaleFactors16Bit.Assign(StrToVec(str, ' '));



    // Retrieving double
    if (!xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblCalibrationInformation", "CountsPerForce", str, "")) {
        return false;
    }
    GenInfo.CountsPerForce = atof(str.c_str());

    if (!xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblCalibrationInformation", "CountsPerTorque", str, "")) {
        return false;
    }
    GenInfo.CountsPerTorque = atof(str.c_str());

    return true;
}

vct6 mtsATINetFTConfig::StrToVec(const std::string & strArray, char delim)
{
    vct6 doubleArray;
    std::stringstream ss(strArray);
    std::string item;
    int i = 0;
    while (std::getline(ss, item, delim)) {
        doubleArray[i++] = atof(item.c_str());
    }
    return doubleArray;
}
