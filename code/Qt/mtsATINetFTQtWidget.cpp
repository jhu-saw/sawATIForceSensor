/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  Author(s):  Anton Deguet
  Created on: 2013-08-24

  (C) Copyright 2013 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/


// system include
#include <iostream>

// Qt includes
#include <QString>
#include <QtGui>
#include <QMessageBox>
#include <QHeaderView>

// cisst
#include <cisstMultiTask/mtsInterfaceRequired.h>
#include <sawATIForceSensor/mtsATINetFTQtWidget.h>

CMN_IMPLEMENT_SERVICES_DERIVED_ONEARG(mtsATINetFTQtWidget, mtsComponent, std::string);

mtsATINetFTQtWidget::mtsATINetFTQtWidget(const std::string & componentName, double periodInSeconds):
    mtsComponent(componentName),
    TimerPeriodInMilliseconds(periodInSeconds), // Qt timers are in milliseconds
    PlotIndex(0)
{
    // Setup CISST Interface
    mtsInterfaceRequired * interfaceRequired;
    interfaceRequired = AddInterfaceRequired("RequiresATINetFTSensor");
    if(interfaceRequired) {
        interfaceRequired->AddFunction("GetFTData", ForceSensor.GetFTData);
        interfaceRequired->AddFunction("Rebias", ForceSensor.RebiasFTData);
        interfaceRequired->AddFunction("GetPeriodStatistics", ForceSensor.GetPeriodStatistics);
    }

    setupUi();
    startTimer(TimerPeriodInMilliseconds);
}

void mtsATINetFTQtWidget::Configure(const std::string &filename)
{
    CMN_LOG_CLASS_INIT_VERBOSE << "Configure: " << filename << std::endl;
}

void mtsATINetFTQtWidget::Startup(void)
{
    CMN_LOG_CLASS_INIT_VERBOSE << "Startup" << std::endl;
    if (!parent()) {
        show();
    }
}

void mtsATINetFTQtWidget::Cleanup(void)
{
    this->hide();
    CMN_LOG_CLASS_INIT_VERBOSE << "Cleanup" << std::endl;
}

void mtsATINetFTQtWidget::closeEvent(QCloseEvent * event)
{
    int answer = QMessageBox::warning(this, tr("mtsATINetFTQtWidget"),
                                      tr("Do you really want to quit this application?"),
                                      QMessageBox::No | QMessageBox::Yes);
    if (answer == QMessageBox::Yes) {
        event->accept();
        QCoreApplication::exit();
    } else {
        event->ignore();
    }
}

void mtsATINetFTQtWidget::setupUi()
{
    QFont font;
    font.setBold(true);
    font.setPointSize(12);

    QTabWidget * tabWidget = new QTabWidget;

    // Tab 1
    QVBoxLayout * tab1Layout = new QVBoxLayout;
    QLabel * instructionsLabel = new QLabel("This widget displays the force and torques values sensed by the ATI NetFT Sensor.\nUnits - Force(N), Torque(N-mm) \nValue in the brackets of each header displays the max F/T(-value,+value)");

    QSpacerItem * vSpacer = new QSpacerItem(40, 10, QSizePolicy::Expanding, QSizePolicy::Preferred);
    QSpacerItem * hSpacer = new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Preferred);

    QVBoxLayout * spinBoxLayout = new QVBoxLayout;
    QHBoxLayout * ftValuesLayout = new QHBoxLayout;
    QLabel * ftLabel = new QLabel("Values");
    ftValuesLayout->addWidget(ftLabel);

    QFTSensorValues = new vctQtWidgetDynamicVectorDoubleRead();
    QFTSensorValues->SetPrecision(4);

    ftValuesLayout->addWidget(QFTSensorValues);
    spinBoxLayout->addLayout(ftValuesLayout);

    QHBoxLayout * buttonLayout = new QHBoxLayout;
    RebiasButton = new QPushButton("Rebias");
    buttonLayout->addWidget(RebiasButton);
    buttonLayout->addStretch();

    QHBoxLayout * sensorPlotLayout = new QHBoxLayout;
    QComboBox * QPlotItem = new QComboBox;
    QPlotItem->addItem("Fx");
    QPlotItem->addItem("Fy");
    QPlotItem->addItem("Fz");
    QPlotItem->addItem("FNorm");
    QPlotItem->addItem("Fxyz");

    QFTPlot = new vctPlot2DOpenGLQtWidget();
    QFTPlot->SetBackgroundColor(vct3(1.0, 1.0, 1.0));
    QFTPlot->resize(QFTPlot->sizeHint());
    QFTPlot->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    ForceScale = QFTPlot->AddScale("time");
    FTSignal[Fx] = ForceScale->AddSignal("fx");
    FTSignal[Fx]->SetColor(vctDouble3(1.0, 0.0, 0.0));
    FTSignal[Fy] = ForceScale->AddSignal("fy");
    FTSignal[Fy]->SetColor(vctDouble3(0.0, 1.0, 0.0));
    FTSignal[Fz] = ForceScale->AddSignal("fz");
    FTSignal[Fz]->SetColor(vctDouble3(0.0, 0.0, 1.0));
    FTSignal[FNorm] = ForceScale->AddSignal("fnorm");
    FTSignal[FNorm]->SetColor(vctDouble3(0.0, 0.0, 0.0));

    sensorPlotLayout->addWidget(QPlotItem);
    sensorPlotLayout->addWidget(QFTPlot);

    // Tab1 layout order
    tab1Layout->addWidget(instructionsLabel);
    tab1Layout->addLayout(spinBoxLayout);
    tab1Layout->addLayout(sensorPlotLayout);
    tab1Layout->addLayout(buttonLayout);
    tab1Layout->addSpacerItem(vSpacer);

    QWidget * tab1 = new QWidget;
    tab1->setLayout(tab1Layout);

    // Tab 2
    QVBoxLayout * tab2Layout = new QVBoxLayout;
    QMIntervalStatistics = new mtsQtWidgetIntervalStatistics();
    tab2Layout->addWidget(QMIntervalStatistics);
    tab2Layout->addStretch();

    QWidget * tab2 = new QWidget;
    tab2->setLayout(tab2Layout);

    // Setup tab widget
    tabWidget->addTab(tab1, "Sensor Stats");
    tabWidget->addTab(tab2, "Interval Stats");
    tabWidget->show();

    setWindowTitle("ATI Force Sensor(N, N-mm)");
    resize(sizeHint());

    // setup Qt Connection
    connect(RebiasButton, SIGNAL(clicked()), this, SLOT(SlotRebiasFTSensor()));
    connect(QPlotItem, SIGNAL(currentIndexChanged(int)), this, SLOT(SlotPlotIndex(int)));
}

void mtsATINetFTQtWidget::timerEvent(QTimerEvent * event)
{
    // make sure we should update the display
    if (this->isHidden()) {
        return;
    }
    mtsExecutionResult executionResult;
    executionResult = ForceSensor.GetFTData(ForceSensor.FTReadings);
    if (!executionResult) {
        CMN_LOG_CLASS_RUN_ERROR << "ForceSensor.GetFTData failed, \""
                                << executionResult << "\"" << std::endl;
    }

    QFTSensorValues->SetValue(ForceSensor.FTReadings);

    // Uppdate the plot
    vctDoubleVec forceOnly(3, 0.0), torqueOnly(3, 0.0);
    forceOnly.Assign(ForceSensor.FTReadings, 3);
    torqueOnly.Assign(ForceSensor.FTReadings, 3, 0, 3);
    ForceSensor.GetPeriodStatistics(IntervalStatistics);
    QMIntervalStatistics->SetValue(IntervalStatistics);


    if(PlotIndex < 3)
        FTSignal[PlotIndex]->AppendPoint(vctDouble2(ForceSensor.FTReadings.Timestamp(),
                                                    forceOnly.Element(PlotIndex)));
    else if(PlotIndex == Fxyz) {
        for (int i = 0; i < Fxyz; ++i) {
            FTSignal[i]->AppendPoint(vctDouble2(ForceSensor.FTReadings.Timestamp(),
                                                        forceOnly.Element(i)));
        }
    }
    else if(PlotIndex == FNorm)
        FTSignal[PlotIndex]->AppendPoint(vctDouble2(ForceSensor.FTReadings.Timestamp(),
                                                    forceOnly.Norm()));

    QFTPlot->updateGL();
}

void mtsATINetFTQtWidget::SlotRebiasFTSensor(void)
{
    ForceSensor.RebiasFTData();
}

void mtsATINetFTQtWidget::SlotPlotIndex(int newAxis)
{
    PlotIndex = newAxis;
    for (int i = 0; i < 4; ++i) {
        FTSignal[i]->SetVisible(false);
    }

    if(PlotIndex == Fxyz) {
        FTSignal[0]->SetVisible(true);
        FTSignal[1]->SetVisible(true);
        FTSignal[2]->SetVisible(true);
    } else {
        FTSignal[PlotIndex]->SetVisible(true);
    }

    QFTPlot->SetContinuousExpandYResetSlot();
}
