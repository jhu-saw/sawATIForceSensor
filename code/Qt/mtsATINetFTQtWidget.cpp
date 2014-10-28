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
    TimerPeriodInMilliseconds(periodInSeconds * 1000) // Qt timers are in milliseconds
{
    SimulateFT = false;
    Simulated.RawFTReadings.SetSize(6);
    Simulated.FilteredFTReadings.SetSize(6);
    // Setup CISST Interface
    mtsInterfaceRequired * interfaceRequired;
    interfaceRequired = AddInterfaceRequired("RequiresATINetFTSensor");
    if(interfaceRequired) {
        interfaceRequired->AddFunction("GetFTData", ForceSensor.GetFTData);
        interfaceRequired->AddFunction("GetRawFTData", ForceSensor.GetRawFTData);
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

    QSpacerItem * vSpacer = new QSpacerItem(40, 10, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QSpacerItem * hSpacer = new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Minimum);

    QVBoxLayout * spinBoxLayout = new QVBoxLayout;
    QHBoxLayout * rawValuesLayout = new QHBoxLayout;
    QHBoxLayout * filterValuesLayout = new QHBoxLayout;
    for (int i = 0; i < 6; ++i) {
        QFTRawSensorValues[i] = new QDoubleSpinBox;
        QFTRawSensorValues[i]->setReadOnly(true);
        rawValuesLayout->addWidget(QFTRawSensorValues[i]);

        QFTFilteredSensorValues[i] = new QDoubleSpinBox;
        QFTFilteredSensorValues[i]->setReadOnly(true);
        filterValuesLayout->addWidget(QFTFilteredSensorValues[i]);
    }    
    spinBoxLayout->addLayout(rawValuesLayout);
    spinBoxLayout->addLayout(filterValuesLayout);

    QHBoxLayout * utilityLaylout = new QHBoxLayout;
    SimCheckBox = new QCheckBox("Simulate");
    CloneFTButton = new QPushButton("Clone");
    utilityLaylout->addWidget(SimCheckBox);
    utilityLaylout->addWidget(CloneFTButton);
    utilityLaylout->addItem(hSpacer);

    QHBoxLayout * buttonLayout = new QHBoxLayout;
    RebiasButton = new QPushButton("Rebias");
    buttonLayout->addWidget(RebiasButton);
    buttonLayout->addStretch();

    // Tab1 layout order
    tab1Layout->addWidget(instructionsLabel);
    tab1Layout->addLayout(utilityLaylout);
    tab1Layout->addLayout(spinBoxLayout);
    tab1Layout->addLayout(buttonLayout);
    tab1Layout->addStretch();

    QWidget * tab1 = new QWidget;
    tab1->setLayout(tab1Layout);

    // Tab 2
    QVBoxLayout * tab2Layout = new QVBoxLayout;
    QMIntervalStatistics = new mtsQtWidgetIntervalStatistics();
    tab2Layout->addWidget(QMIntervalStatistics);

    QWidget * tab2 = new QWidget;
    tab2->setLayout(tab2Layout);

    // Setup tab widget
    tabWidget->addTab(tab1, "Sensor Stats");
    tabWidget->addTab(tab2, "Intervel Stats");
    tabWidget->show();

    setWindowTitle("ATI Force Sensor(N, N-mm)");
    resize(sizeHint());

    // setup Qt Connection
    connect(RebiasButton, SIGNAL(clicked()), this, SLOT(RebiasFTSensor()));
    connect(SimCheckBox, SIGNAL(clicked(bool)), this, SLOT(SimulateChecked(bool)));
    connect(CloneFTButton, SIGNAL(clicked()), this, SLOT(CloneFTSensor()));
}

void mtsATINetFTQtWidget::timerEvent(QTimerEvent * event)
{
    // make sure we should update the display
    if (this->isHidden()) {
        return;
    }

    if(SimulateFT) {
        for (int i = 0; i < 6; ++i) {
            Simulated.RawFTReadings[i] = QFTRawSensorValues[i]->value();
            Simulated.FilteredFTReadings[i] = QFTFilteredSensorValues[i]->value();
        }
    } else {
        mtsExecutionResult executionResult;
        executionResult = ForceSensor.GetFTData(ForceSensor.FilteredFTReadings);
        if (!executionResult) {
            CMN_LOG_CLASS_RUN_ERROR << "ForceSensor.GetFTData failed, \""
                                    << executionResult << "\"" << std::endl;
        }

        executionResult = ForceSensor.GetRawFTData(ForceSensor.RawFTReadings);
        if (!executionResult) {
            CMN_LOG_CLASS_RUN_ERROR << "ForceSensor.GetFTData failed, \""
                                    << executionResult << "\"" << std::endl;
        }

        for (int i = 0; i < 6; ++i) {
            QFTRawSensorValues[i]->setValue(ForceSensor.RawFTReadings[i]);
            QFTFilteredSensorValues[i]->setValue(ForceSensor.FilteredFTReadings[i]);
        }
    }

    ForceSensor.GetPeriodStatistics(IntervalStatistics);
    QMIntervalStatistics->SetValue(IntervalStatistics);
}

void mtsATINetFTQtWidget::RebiasFTSensor(void)
{
    ForceSensor.RebiasFTData();
}

// Copy current snapshot of the force sensor state to the simulated state
void mtsATINetFTQtWidget::CloneFTSensor(void)
{
    if(SimulateFT) {
        for (int i = 0; i < 6; ++i) {
            QFTRawSensorValues[i]->setValue(ForceSensor.RawFTReadings[i]);
            QFTFilteredSensorValues[i]->setValue(ForceSensor.FilteredFTReadings[i]);
        }
    } else {
        Simulated.RawFTReadings = ForceSensor.RawFTReadings;
        Simulated.FilteredFTReadings = ForceSensor.FilteredFTReadings;
    }
}

void mtsATINetFTQtWidget::SimulateChecked(bool isClicked)
{
    SimulateFT  = isClicked;
    for (int i = 0; i < 6; ++i) {
        QFTRawSensorValues[i]->setReadOnly(!isClicked);
        QFTFilteredSensorValues[i]->setReadOnly(!isClicked);
    }

    // Save previously set simulated values
    if(!SimulateFT) {
        for (int i = 0; i < 6; ++i) {
            Simulated.RawFTReadings[i] = QFTRawSensorValues[i]->value();
            Simulated.FilteredFTReadings[i] = QFTFilteredSensorValues[i]->value();
        }
    } else {
        for (int i = 0; i < 6; ++i) {
            QFTRawSensorValues[i]->setValue(Simulated.RawFTReadings[i]);
            QFTFilteredSensorValues[i]->setValue(Simulated.FilteredFTReadings[i]);
        }
    }
}
