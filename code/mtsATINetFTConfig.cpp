#include <sawATINetFT/mtsATINetFTConfig.h>

#include <cisstCommon/cmnXMLPath.h>
#include <cisstCommon/cmnPath.h>
#include <cisstCommon/cmnUnits.h>
#include <cisstOSAbstraction.h>

CMN_IMPLEMENT_SERVICES(mtsATINetFTConfig)

mtsATINetFTConfig::mtsATINetFTConfig()
{

}

mtsATINetFTConfig::~mtsATINetFTConfig()
{

}

bool mtsATINetFTConfig::LoadCalibrationFile(const std::string &calFile)
{   
    if(!ParseCalibrationFile(calFile))
    {
        CMN_LOG_CLASS_RUN_WARNING << "LoadCalibrationFile : Parsing failed " << std::endl;
        return false;
    }

    return true;
}

bool mtsATINetFTConfig::ParseCalibrationFile(const std::string &calFile)
{
    cmnXMLPath xmlPath;
    xmlPath.SetInputSource(calFile);

    std::string str;

    // Calibration info
    if (!xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblNetFTCalibrationInfo", "SerialNumber", NetFT.CalibInfo.SerialNumber, "") ||
            !xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblNetFTCalibrationInfo", "BodyStyle", NetFT.CalibInfo.BodyStyle, "") ||
            !xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblNetFTCalibrationInfo", "CalibrationPartNumber", NetFT.CalibInfo.CalibrationPartNumber, "") ||
            !xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblNetFTCalibrationInfo", "Family", NetFT.CalibInfo.Family, "") ||
            !xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblNetFTCalibrationInfo", "CalibrationDate", NetFT.CalibInfo.CalibrationDate, "")
            )
    {
        return false;
    }

    // Retrieving vct6
    if (!xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblNetFTCalibrationInfo", "GaugeOffsets", str, ""))
        return false;
    NetFT.CalibInfo.GaugeOffsets.Assign(StrToVec(str, ' '));

    if (!xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblNetFTCalibrationInfo", "CalibrationIndex", str, ""))
        return false;
    NetFT.CalibInfo.CalibrationIndex.Assign(StrToVec(str, ' '));


    // Calibration Matrix
    std::string Fx, Fy, Fz, Tx, Ty, Tz;
    if (!xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblNetFTCalibrationInfo", "MatrixFx", Fx, "") ||
            !xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblNetFTCalibrationInfo", "MatrixFy", Fy, "") ||
            !xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblNetFTCalibrationInfo", "MatrixFz", Fz, "") ||
            !xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblNetFTCalibrationInfo", "MatrixTx", Tx, "") ||
            !xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblNetFTCalibrationInfo", "MatrixTy", Ty, "") ||
            !xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblNetFTCalibrationInfo", "MatrixTz", Tz, "")
            )
    {
        return false;
    }

    NetFT.CalibInfo.Matrix.SetSize(6,6);
    NetFT.CalibInfo.Matrix.Row(0) = vctDoubleVec(StrToVec(Fx, ' '));
    NetFT.CalibInfo.Matrix.Row(1) = vctDoubleVec(StrToVec(Fy, ' '));
    NetFT.CalibInfo.Matrix.Row(2) = vctDoubleVec(StrToVec(Fz, ' '));
    NetFT.CalibInfo.Matrix.Row(3) = vctDoubleVec(StrToVec(Fx, ' '));
    NetFT.CalibInfo.Matrix.Row(4) = vctDoubleVec(StrToVec(Fy, ' '));
    NetFT.CalibInfo.Matrix.Row(5) = vctDoubleVec(StrToVec(Fz, ' '));


    // General info
    if (!xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblCalibrationInformation", "ForceUnits", NetFT.GenInfo.ForceUnits, "") ||
            !xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblCalibrationInformation", "TorqueUnits", NetFT.GenInfo.TorqueUnits, "")
            )
    {
        return false;
    }

    // Retrieving vct6
    if (!xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblCalibrationInformation", "MaxRatings", str, ""))
        return false;
    NetFT.GenInfo.MaxRatings.Assign(StrToVec(str, ' '));

    if (!xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblCalibrationInformation", "Resolutions", str, ""))
        return false;
    NetFT.GenInfo.Resolutions.Assign(StrToVec(str, ' '));

    if (!xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblCalibrationInformation", "Ranges", str, ""))
        return false;
    NetFT.GenInfo.Ranges.Assign(StrToVec(str, ' '));

    if (!xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblCalibrationInformation", "_x0031_6BitScaleFactors", str, ""))
        return false;
    NetFT.GenInfo.ScaleFactors16Bit.Assign(StrToVec(str, ' '));



    // Retrieving double
    if (!xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblCalibrationInformation", "CountsPerForce", str, ""))
        return false;
    NetFT.GenInfo.CountsPerForce = atof(str.c_str());

    if (!xmlPath.GetXMLValue("/dsNetFTCalibrationFile/tblCalibrationInformation", "CountsPerTorque", str, ""))
        return false;
    NetFT.GenInfo.CountsPerTorque = atof(str.c_str());

    return true;
}

vct6 mtsATINetFTConfig::StrToVec(const std::string strArray, char delim)
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
