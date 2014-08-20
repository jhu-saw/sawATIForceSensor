#ifndef _devNetFTSensorATI_h
#define _devNetFTSensorATI_h

#include <iostream>
#include <cisstVector.h>
#include "ftconfig.h"

class devNetFTSensorATI : public cmnGenericObject
{
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, 5);

public:

    devNetFTSensorATI();
    ~devNetFTSensorATI();

    bool LoadCalibrationFile(const std::string &calFile);
    bool ParseCalibrationFile(const std::string &calFile);
    float GetMaxLoad(int axisIndex) const;
    int ToolTransform( float transformVector[], const std::string & distanceUnits, const std::string angleUnits);

protected:

    Calibration *calibration;
    Transform *trans;
    int *t;

    void makeNull(Calibration *cal);
    void intializeCalibrationValues();
    void ConvertStringToCString(const std::string & sourceString, char destString[], unsigned int destSize );
};

CMN_DECLARE_SERVICES_INSTANTIATION(devNetFTSensorATI)

#endif
