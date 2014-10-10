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

// Qt include
#include <QString>
#include <QtGui>

// cisst
#include <cisstMultiTask/mtsInterfaceRequired.h>
#include <sawATIForceSensor/mtsATINetFTQtWidget.h>

CMN_IMPLEMENT_SERVICES_DERIVED_ONEARG(mtsATINetFTQtWidget, mtsComponent, std::string);

mtsATINetFTQtWidget::mtsATINetFTQtWidget(const std::string & componentName, double periodInSeconds):
    mtsComponent(componentName),
    TimerPeriodInMilliseconds(periodInSeconds * 1000) // Qt timers are in milliseconds
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

void mtsATINetFTQtWidget::timerEvent(QTimerEvent * event)
{
    // make sure we should update the display
    if (this->isHidden()) {
        return;
    }

    mtsExecutionResult executionResult;
    executionResult = ForceSensor.GetFTData(FTReadings);
    if (!executionResult) {
        CMN_LOG_CLASS_RUN_ERROR << "ForceSensor.GetFTData failed, \""
                                << executionResult << "\"" << std::endl;
    }

    QStringList heading;
    heading << "Fx(25)" << "Fy(25)" << "Fz(30)" << "Tx(250)" << "Ty(250)" << "Tz(250)";

    QFTSensorValues->SetValue(FTReadings);
    QFTSensorValues->setHorizontalHeaderLabels(heading);

    ForceSensor.GetPeriodStatistics(IntervalStatistics);
    QMIntervalStatistics->SetValue(IntervalStatistics);
}

void mtsATINetFTQtWidget::RebiasFTSensor(void)
{
    ForceSensor.RebiasFTData();
}

void mtsATINetFTQtWidget::setupUi()
{
    QFont font;
    font.setBold(true);
    font.setPointSize(12);

    QVBoxLayout * controlLayout = new QVBoxLayout;

    QLabel * instructionsLabel = new QLabel("This widget displays the force and torques values sensed by the ATI NetFT Sensor.\nUnits - Force(N), Torque(N-mm) \nValue in the brackets of each header displays the max F/T(-value,+value)");
    controlLayout->addWidget(instructionsLabel);

    QSpacerItem * vSpacer = new QSpacerItem(40, 10, QSizePolicy::Expanding, QSizePolicy::Minimum);
    controlLayout->addItem(vSpacer);

    QFTSensorValues = new vctQtWidgetDynamicVectorDoubleRead();
    QFTSensorValues->setFixedSize(560, 50);
    QFTSensorValues->horizontalHeader()->show();
    QFTSensorValues->SetPrecision(5);

    controlLayout->addWidget(QFTSensorValues);

    QMIntervalStatistics = new mtsQtWidgetIntervalStatistics();
    controlLayout->addWidget(QMIntervalStatistics);

    QHBoxLayout * buttonLayout = new QHBoxLayout;

    RebiasButton = new QPushButton("Rebias");
    buttonLayout->addWidget(RebiasButton);

    buttonLayout->addStretch();

    controlLayout->addItem(vSpacer);
    controlLayout->addLayout(buttonLayout);
    controlLayout->addStretch();

    QHBoxLayout * mainLayout = new QHBoxLayout;
    mainLayout->addLayout(controlLayout);

    setLayout(mainLayout);

    setWindowTitle("ATI Force Sensor(N, N-mm)");
    resize(sizeHint());

    // setup Qt Connection
    connect(RebiasButton, SIGNAL(clicked()), this, SLOT(RebiasFTSensor()));
}
