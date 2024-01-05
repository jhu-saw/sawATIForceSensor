/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  Author(s):  Anton Deguet
  Created on: 2013-08-24

  (C) Copyright 2013-2021 Johns Hopkins University (JHU), All Rights Reserved.

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
#include <QComboBox>
#include <QLineEdit>

// cisst
#include <cisstMultiTask/mtsInterfaceRequired.h>
#include <sawATIForceSensor/mtsATINetFTQtWidget.h>

CMN_IMPLEMENT_SERVICES_DERIVED_ONEARG(mtsATINetFTQtWidget, mtsComponent, std::string);

mtsATINetFTQtWidget::mtsATINetFTQtWidget(const std::string & componentName, double periodInSeconds):
    mtsComponent(componentName),
    TimerPeriodInMilliseconds(periodInSeconds * 1000.0) // Qt timers are in milliseconds
{
    // Setup CISST Interface
    mtsInterfaceRequired * interfaceRequired;
    interfaceRequired = AddInterfaceRequired("RequiresATINetFTSensor");
    if(interfaceRequired) {
        interfaceRequired->AddFunction("measured_cf", ForceSensor.measured_cf);
        interfaceRequired->AddFunction("Rebias", ForceSensor.RebiasForceTorque);
        interfaceRequired->AddFunction("GetPeriodStatistics", ForceSensor.GetPeriodStatistics);
        interfaceRequired->AddFunction("GetIsConnected", ForceSensor.GetIsConnected);
        interfaceRequired->AddFunction("GetIsSaturated", ForceSensor.GetIsSaturated);
        interfaceRequired->AddFunction("GetHasError", ForceSensor.GetHasError);
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

    //--- Tab 1
    QVBoxLayout * tab1Layout = new QVBoxLayout;
    QLabel * instructionsLabel = new QLabel("This widget displays the force and torques values sensed by the ATI NetFT Sensor.\nUnits - Force(N), Torque(N-mm) \nValue in the brackets of each header displays the max F/T(-value,+value)");

    // Spacers
    QSpacerItem * vSpacer = new QSpacerItem(40, 10, QSizePolicy::Expanding, QSizePolicy::Preferred);
//    QSpacerItem * hSpacer = new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Preferred);

    QFTWidget = new vctForceTorqueQtWidget();

    // Layout for Error Messages
    QHBoxLayout * errorLayout = new QHBoxLayout;
    QLabel *errorLabel = new QLabel("Info");

    ErrorMsg = new QLineEdit("No Error");
    ErrorMsg->setFont(QFont("lucida", 12, QFont::Bold, true));
    ErrorMsg->setStyleSheet("QLineEdit {background-color: green }");

    errorLayout->addWidget(errorLabel);
    errorLayout->addWidget(ErrorMsg);
    errorLayout->addStretch();

    // Layout containing rebias button
    QHBoxLayout * buttonLayout = new QHBoxLayout;
    RebiasButton = new QPushButton("Rebias");
    buttonLayout->addWidget(RebiasButton);
    buttonLayout->addStretch();

    // Tab1 layout order
    tab1Layout->addWidget(instructionsLabel);
    tab1Layout->addWidget(QFTWidget);
    tab1Layout->addSpacerItem(vSpacer);
    tab1Layout->addLayout(errorLayout);
    tab1Layout->addLayout(buttonLayout);
    tab1Layout->addSpacerItem(vSpacer);

    QWidget * tab1 = new QWidget;
    tab1->setLayout(tab1Layout);

    //--- Tab 2
    QVBoxLayout * tab2Layout = new QVBoxLayout;
    QMIntervalStatistics = new mtsIntervalStatisticsQtWidget();
    tab2Layout->addWidget(QMIntervalStatistics);
    tab2Layout->addStretch();

    QWidget * tab2 = new QWidget;
    tab2->setLayout(tab2Layout);

    // Setup tab widget
    tabWidget->addTab(tab1, "Sensor Stats");
    tabWidget->addTab(tab2, "Interval Stats");

    QHBoxLayout * mainLayout = new QHBoxLayout;
    mainLayout->addWidget(tabWidget);
    setLayout(mainLayout);

    setWindowTitle("ATI Force Sensor(N, N-mm)");
    resize(sizeHint());

    // setup Qt Connection
    connect(RebiasButton, SIGNAL(clicked()), this, SLOT(SlotRebiasFTSensor()));
}

void mtsATINetFTQtWidget::timerEvent(QTimerEvent * event)
{
    event->accept();

    // make sure we should update the display
    if (this->isHidden()) {
        return;
    }

    mtsExecutionResult executionResult;
    executionResult = ForceSensor.measured_cf(m_measured_cf);
    if (!executionResult) {
        CMN_LOG_CLASS_RUN_ERROR << "ForceSensor.measured_cf failed, \""
                                << executionResult << "\"" << std::endl;
    }
    QFTWidget->SetValue(m_measured_cf.F(), m_measured_cf.T(), m_measured_cf.Timestamp());

    // Update error state
    ForceSensor.GetIsConnected(ForceSensor.IsConnected);
    ForceSensor.GetIsSaturated(ForceSensor.IsSaturated);
    ForceSensor.GetHasError(ForceSensor.HasError);

    if(!ForceSensor.IsConnected) {
        ErrorMsg->setText(QString("Not Connected"));
        ErrorMsg->setStyleSheet("QLineEdit {background-color: red }");
    } else if (ForceSensor.HasError) {
        ErrorMsg->setText(QString("Hardware Error"));
        ErrorMsg->setStyleSheet("QLineEdit {background-color: red }");
    } else if(ForceSensor.IsSaturated) {
        ErrorMsg->setText(QString("Saturated"));
        ErrorMsg->setStyleSheet("QLineEdit {background-color: red }");
    } else {
        ErrorMsg->setText(QString("Connected : No Error"));
        ErrorMsg->setStyleSheet("QLineEdit {background-color:green }");
    }

    // update interval statistics
    ForceSensor.GetPeriodStatistics(IntervalStatistics);
    QMIntervalStatistics->SetValue(IntervalStatistics);
}

void mtsATINetFTQtWidget::SlotRebiasFTSensor(void)
{
    ForceSensor.RebiasForceTorque();
}
