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
#include <cisstVector/vctPlot2DOpenGLQtWidget.h>

#include <QWidget>
#include <QtGui>
#include <QPushButton>
#include <QDoubleSpinBox>

// Always include last
#include <sawATIForceSensor/sawATIForceSensorQtExport.h>

class CISST_EXPORT mtsATINetFTQtWidget: public QWidget, public mtsComponent
{
    Q_OBJECT;
    CMN_DECLARE_SERVICES(CMN_DYNAMIC_CREATION_ONEARG, CMN_LOG_ALLOW_ALL);

public:
    enum {
        Fx = 0,
        Fy = 1,
        Fz = 2,
        FNorm = 3,
        Fxyz = 4,
        Txyz = 5
    };
    mtsATINetFTQtWidget(const std::string & componentName, double periodInSeconds = 50.0 * cmn_ms);
    ~mtsATINetFTQtWidget(){}

    void Configure(const std::string & filename = "");
    void Startup(void);
    void Cleanup(void);

protected:
    virtual void closeEvent(QCloseEvent * event);

private:
    void setupUi(void);
    void SetupSensorPlot(void);

private:
    struct NetFTStruct {
        mtsFunctionVoid RebiasForceTorque;
        mtsFunctionRead GetFTData;        
        mtsFunctionRead GetPeriodStatistics;
        mtsFunctionRead GetIsConnected;
        mtsFunctionRead GetIsSaturated;
        mtsFunctionRead GetHasError;

        mtsDoubleVec FTReadings;
        bool IsConnected;
        bool IsSaturated;
        bool HasError;
    } ForceSensor;

    mtsBool IsSaturated;
    vctQtWidgetDynamicVectorDoubleRead * QFTSensorValues;

    QPushButton * RebiasButton;
    QLabel * ConnectionStatus;

    QLabel * UpperLimit;
    QLabel * LowerLimit;

    QLineEdit * ErrorMsg;

    vctPlot2DOpenGLQtWidget * QFTPlot;
    vctPlot2DBase::Signal * ForceSignal[3];
    vctPlot2DBase::Signal * FNormSignal;
    vctPlot2DBase::Signal * TorqueSignal[3];

    vctPlot2DBase::Scale * ForceScale;
    vctPlot2DBase::Scale * TorqueScale;

    double Time;
    int PlotIndex;
    int TimerPeriodInMilliseconds;

    // Timing
    mtsIntervalStatistics IntervalStatistics;
    mtsQtWidgetIntervalStatistics * QMIntervalStatistics;

private slots:
    void timerEvent(QTimerEvent * event);
    void SlotRebiasFTSensor(void);
    void SlotPlotIndex(int newAxis);
};

CMN_DECLARE_SERVICES_INSTANTIATION(mtsATINetFTQtWidget);

#endif // _mtsATINetFTQtWidget_h
