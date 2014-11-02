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

#include <cisstMultiTask/mtsComponent.h>
#include <cisstMultiTask/mtsVector.h>
#include <cisstMultiTask/mtsQtWidgetIntervalStatistics.h>
#include <cisstParameterTypes/prmPositionCartesianGet.h>
#include <cisstVector/vctQtWidgetDynamicVector.h>

#include <QWidget>
#include <QtGui>
#include <QPushButton>
#include <QDoubleSpinBox>

#include <sawATIForceSensor/qcustomplot.h>

// Always include last
#include <sawATIForceSensor/sawATIForceSensorQtExport.h>

class CISST_EXPORT mtsATINetFTQtWidget: public QWidget, public mtsComponent
{
    Q_OBJECT;
    CMN_DECLARE_SERVICES(CMN_DYNAMIC_CREATION_ONEARG, CMN_LOG_ALLOW_ALL);

public:
    mtsATINetFTQtWidget(const std::string & componentName, double periodInSeconds = 50.0 * cmn_ms);
    ~mtsATINetFTQtWidget(){}

    void Configure(const std::string & filename = "");
    void Startup(void);
    void Cleanup(void);

protected:
    virtual void closeEvent(QCloseEvent * event);
    QCustomPlot* SetupRealTimePlot(const std::string XAxis, const std::string Yaxis);
    void UpdateRealTimePlot(QCustomPlot* plot,double key, double value);
private:
    //! setup TeleOperation controller GUI
    void setupUi(void);
    int TimerPeriodInMilliseconds;

protected:
    struct NetFTStruct {
        mtsFunctionVoid RebiasFTData;
        mtsFunctionRead GetFTData;
        mtsFunctionRead GetIsSaturated;
        mtsFunctionRead GetPeriodStatistics;

        mtsDoubleVec FTReadings;
    } ForceSensor;


private:

    mtsBool IsSaturated;    
//    QDoubleSpinBox * QFTSensorValues[6];
    vctQtWidgetDynamicVectorDoubleRead * QFTSensorValues;

    QPushButton * RebiasButton;
    QLabel * ConnectionStatus;
    QCustomPlot * SensorRTPlot;
    QLabel * SensorRTPlotFPS;
    double Time;

    // Timing
    mtsIntervalStatistics IntervalStatistics;
    mtsQtWidgetIntervalStatistics * QMIntervalStatistics;

private slots:
    void timerEvent(QTimerEvent * event);
    void RebiasFTSensor(void);    
};

CMN_DECLARE_SERVICES_INSTANTIATION(mtsATINetFTQtWidget);

#endif // _mtsATINetFTQtWidget_h
