#include <sawATINetFT/devNetFTSensorATI.h>

#include <cisstCommon/cmnXMLPath.h>
#include <cisstCommon/cmnPath.h>
#include <cisstCommon/cmnUnits.h>
#include <cisstOSAbstraction.h>

CMN_IMPLEMENT_SERVICES(devNetFTSensorATI)

#define WHOLE_LOTTA_CHARACTERS 512
#define FIRST_TORQUE_INDEX 3
#define NUM_FT_AXES 6
#define NUM_MATRIX_ELEMENTS 36
#define NUM_STRAIN_GAUGES 6
#define GAUGE_SATURATION_LEVEL 0.995

devNetFTSensorATI::devNetFTSensorATI(): calibration(NULL)
{

}

devNetFTSensorATI::~devNetFTSensorATI()
{
    if ( NULL != calibration )
        DAQFTCLIBRARY::destroyCalibration(calibration);
}

bool devNetFTSensorATI::LoadCalibrationFile(const std::string &calFile)
{
    if( NULL != calibration )
        DAQFTCLIBRARY::destroyCalibration(calibration);

    if( !ParseCalibrationFile(calFile) || calibration == NULL)
    {
        CMN_LOG_CLASS_INIT_ERROR << "Failed to Load Calibration " << calFile << std::endl;
        return false;
    }
    else
        CMN_LOG_CLASS_INIT_VERBOSE << "Calibration file loaded " << calFile << std::endl;

    DAQFTCLIBRARY::ResetDefaults(calibration);

    // TODO
    return true;
}

void devNetFTSensorATI::intializeCalibrationValues()
{
    CMN_LOG_INIT_VERBOSE << "Initializing Calibration values" << std::endl;

    calibration->TempCompAvailable = false;
    for (int i = 0; i < MAX_GAUGES; ++i)
    {
        calibration->rt.bias_slopes[i] = 0;
        calibration->rt.gain_slopes[i] = 0;
    }

    calibration->rt.thermistor = 0;
    calibration->BasicTransform.AngleUnits = DAQFTCLIBRARY::ATI_strdup("deg\0");
    calibration->cfg.UserTransform.AngleUnits = DAQFTCLIBRARY::ATI_strdup("deg\0");
    calibration->BiPolar = true;
    calibration->VoltageRange = 20;
    calibration->HWTempComp = false;

}

bool devNetFTSensorATI::ParseCalibrationFile(const std::string &calFile)
{
    if( calibration == NULL )
        DAQFTCLIBRARY::destroyCalibration(calibration);

    calibration = (Calibration *) calloc(1, sizeof(Calibration));

    intializeCalibrationValues();

    CMN_LOG_INIT_VERBOSE << "Parsing " << calFile << std::endl;

    cmnXMLPath config;
    config.SetInputSource(calFile);

    std::string serial, bodystyle, family;
    int ng = 0;

    if( config.GetXMLValue("/NetFTSensor", "@SerialNumber",serial)    &&
        config.GetXMLValue("/NetFTSensor", "@BodyStyle",bodystyle) &&
        config.GetXMLValue("/NetFTSensor", "@Family",family)    &&
        config.GetXMLValue("/NetFTSensor", "@NumGages",ng) )
    {
        calibration->rt.NumChannels = (unsigned short) ng;
        calibration->Serial = DAQFTCLIBRARY::ATI_strdup(serial.c_str());
        calibration->BodyStyle = DAQFTCLIBRARY::ATI_strdup(bodystyle.c_str());
        calibration->Family = DAQFTCLIBRARY::ATI_strdup(family.c_str());
        calibration->rt.NumChannels = calibration->rt.NumChannels + 1;
    }
    else
    {
        CMN_LOG_CLASS_INIT_ERROR << "Failed to load config " << std::endl;
        return false;
    }


    std::string partNumber, calDate, forceUnits, torqueUnits, distUnits, angleUnits, biPolar, hwTempComp;

    if( config.GetXMLValue("/NetFTSensor/Calibration", "@PartNumber", partNumber)   &&
        config.GetXMLValue("/NetFTSensor/Calibration", "@CalDate", calDate)         &&
        config.GetXMLValue("/NetFTSensor/Calibration", "@ForceUnits", forceUnits)   &&
        config.GetXMLValue("/NetFTSensor/Calibration", "@TorqueUnits", torqueUnits) &&
        config.GetXMLValue("/NetFTSensor/Calibration", "@DistUnits", distUnits)      &&
        config.GetXMLValue("/NetFTSensor/Calibration", "@OutputBipolar", biPolar)   &&
        config.GetXMLValue("/NetFTSensor/Calibration", "@OutputRange", calibration->VoltageRange)   &&
        config.GetXMLValue("/NetFTSensor/Calibration", "@HWTempComp", hwTempComp) )
    {
        calibration->PartNumber = DAQFTCLIBRARY::ATI_strdup(partNumber.c_str());
        calibration->CalDate = DAQFTCLIBRARY::ATI_strdup(calDate.c_str());
        calibration->ForceUnits = DAQFTCLIBRARY::ATI_strdup(forceUnits.c_str());
        calibration->TorqueUnits = DAQFTCLIBRARY::ATI_strdup(torqueUnits.c_str());
        calibration->BasicTransform.DistUnits = DAQFTCLIBRARY::ATI_strdup(distUnits.c_str());

        calibration->cfg.ForceUnits = DAQFTCLIBRARY::ATI_strdup(calibration->ForceUnits);
        calibration->cfg.TorqueUnits = DAQFTCLIBRARY::ATI_strdup(calibration->TorqueUnits);
        calibration->cfg.UserTransform.DistUnits = DAQFTCLIBRARY::ATI_strdup(calibration->BasicTransform.DistUnits);
        calibration->cfg.UserTransform.AngleUnits = DAQFTCLIBRARY::ATI_strdup(calibration->BasicTransform.AngleUnits);

        if( biPolar.compare("True") == 0 )
            calibration->BiPolar = 1;
        else
            calibration->BiPolar = 0;

        if( hwTempComp.compare("True") == 0)
            calibration->HWTempComp = 1;
        else
            calibration->HWTempComp = 0;
    }
    else
    {
        CMN_LOG_CLASS_INIT_ERROR << "Failed to load config " << std::endl;
        return false;
    }


    std::vector<double> TT;
    TT.resize(6);

    for (int i = 0; i < 6; ++i)
        TT[i] = 0;

    if( config.GetXMLValue("/NetFTSensor/Calibration/BasicTransform", "@Dx", TT[0]) &&
        config.GetXMLValue("/NetFTSensor/Calibration/BasicTransform", "@Dy", TT[1]) &&
        config.GetXMLValue("/NetFTSensor/Calibration/BasicTransform", "@Dz", TT[2]) &&
        config.GetXMLValue("/NetFTSensor/Calibration/BasicTransform", "@Rx", TT[3]) &&
        config.GetXMLValue("/NetFTSensor/Calibration/BasicTransform", "@Ry", TT[4]) &&
        config.GetXMLValue("/NetFTSensor/Calibration/BasicTransform", "@Rz", TT[5]) )
    {
        for(int i=0; i<6; i++)
            calibration->BasicTransform.TT[i] = TT[i];
    }
    else
    {
        CMN_LOG_CLASS_INIT_ERROR << "Failed to load config " << std::endl;
        return false;
    }



    char xmlContext[100];
    calibration->rt.NumAxes = 0;
    std::string aname;
    bool loop = false;

    while(!loop)
    {
        sprintf(xmlContext, "/NetFTSensor/Calibration/Axis[%d]", calibration->rt.NumAxes+1);
        if ( config.GetXMLValue(xmlContext, "@Name", aname) )
        {
            calibration->AxisNames[calibration->rt.NumAxes] = DAQFTCLIBRARY::ATI_strdup(aname.c_str());
            calibration->rt.NumAxes++;
        }
        else
            loop = true;

    }

    std::cout << "number of axis " << calibration->rt.NumAxes<< std::endl;

    for (int axis = 0; axis < calibration->rt.NumAxes; axis++)
    {
        calibration->MaxLoads[axis] = 0;
        double scale = 1, max = 0;
        std::string values;        

        sprintf(xmlContext, "/NetFTSensor/Calibration/Axis[%d]", axis+1);
        if( config.GetXMLValue(xmlContext, "@scale", scale) &&
            config.GetXMLValue(xmlContext, "@values", values) &&
            config.GetXMLValue(xmlContext, "@max", max) )
        {
            calibration->MaxLoads[axis] = max;
        }
        else
        {
            CMN_LOG_CLASS_INIT_ERROR << "Failed to load config " << std::endl;
            return false;
        }

        float tempArray[8];
        DAQFTCLIBRARY::Separate((char *)(values.c_str()), tempArray, (unsigned short)(calibration->rt.NumChannels+1));
        for(int j=0; j<calibration->rt.NumChannels-1; j++)
            calibration->BasicMatrix[axis][j] = tempArray[j]/scale;
    }

    CMN_LOG_CLASS_INIT_VERBOSE << "Configured Successfully " << std::endl;

    return true;
}


float devNetFTSensorATI::GetMaxLoad(int axisIndex) const
{
    if( NULL == calibration)
        return 0;

    float retval;

    retval= calibration->MaxLoads[axisIndex];

    if( axisIndex < FIRST_TORQUE_INDEX )
        retval *= DAQFTCLIBRARY::ForceConv(calibration->cfg.ForceUnits)/DAQFTCLIBRARY::ForceConv(calibration->ForceUnits);
    else
        retval *= DAQFTCLIBRARY::TorqueConv( calibration->cfg.TorqueUnits) / DAQFTCLIBRARY::TorqueConv(calibration->TorqueUnits);

    return retval;

}

int devNetFTSensorATI::ToolTransform(float transformVector[], const std::string &distanceUnits, const std::string angleUnits)
{
    float tempTransforms[6];
    char cstrDistUnits[WHOLE_LOTTA_CHARACTERS];
    char cstrAngleUnits[WHOLE_LOTTA_CHARACTERS];
    int i;
    ConvertStringToCString( distanceUnits, cstrDistUnits, WHOLE_LOTTA_CHARACTERS );
    ConvertStringToCString( angleUnits, cstrAngleUnits, WHOLE_LOTTA_CHARACTERS );
    /*
        precondition: transformVector has the transformation values
        postcondition: tempTransforms has a copy of transformVector, i = NUM_FT_AXES
    */
    for ( i = 0; i < NUM_FT_AXES; i++ )
    {
        tempTransforms[i] = (float)transformVector[i];
    }
    return DAQFTCLIBRARY::SetToolTransform( calibration, tempTransforms, cstrDistUnits, cstrAngleUnits );

}

void devNetFTSensorATI::ConvertStringToCString(const std::string & sourceString, char destString[], unsigned int destSize )
{
    unsigned int i;
    for ( i = 0; ( i < destSize ) && ( i < sourceString.size() ); i++ )
    {
        destString[i] = sourceString.c_str()[i];
    }
    destString[i] = '\0';
}
