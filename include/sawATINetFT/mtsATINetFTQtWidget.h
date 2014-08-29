/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  Author(s):  Anton Deguet
  Created on: 2013-08-24

  (C) Copyright 2013-2014 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

#ifndef _mtsATINetFTQtWidget_h
#define _mtsATINetFTQtWidget_h

#include <cisstVector/vctQtWidgetDynamicVector.h>
#include <cisstMultiTask/mtsComponent.h>
#include <cisstMultiTask/mtsVector.h>
#include <cisstMultiTask/mtsQtWidgetIntervalStatistics.h>
#include <cisstParameterTypes/prmPositionCartesianGet.h>

#include <QtCore>
#include <QtGui>


class mtsATINetFTQtWidget: public QWidget, public mtsComponent
{
    Q_OBJECT;
    CMN_DECLARE_SERVICES(CMN_DYNAMIC_CREATION_ONEARG, CMN_LOG_ALLOW_ERRORS_AND_WARNINGS);

public:
    mtsATINetFTQtWidget(const std::string & componentName, double periodInSeconds = 50.0 * cmn_ms);
    ~mtsATINetFTQtWidget(){}

    void Configure(const std::string & filename = "");
    void Startup(void);
    void Cleanup(void);

protected:
    virtual void closeEvent(QCloseEvent * event);

private slots:
    void timerEvent(QTimerEvent * event);
    void RebiasFTSensor(void);
    // void LogClicked(void);

private:
    //! setup TeleOperation controller GUI
    void setupUi(void);
    int TimerPeriodInMilliseconds;

protected:
    struct NetFTStruct {
        mtsFunctionVoid     RebiasFTData;
        //         mtsFunctionWrite    SetLogEnabled;
        // mtsFunctionRead     GetLogEnabled;
        mtsFunctionRead     GetFTData;
        mtsFunctionRead     GetIsSaturated;
    } NetFT;


private:

    mtsDoubleVec FTReadings;
    mtsBool IsSaturated;
    vctQtWidgetDynamicVectorDoubleRead * QFTSensorValues;
    QPushButton * RebiasButton;

    // GUI: timing
    mtsIntervalStatistics IntervalStatistics;
    mtsQtWidgetIntervalStatistics * QMIntervalStatistics;
};

CMN_DECLARE_SERVICES_INSTANTIATION(mtsATINetFTQtWidget);

#endif // _mtsATINetFTQtWidget_h
