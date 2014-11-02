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

//    for (int i = 0; i < 6; ++i) {

//        QFTSensorValues[i] = new QDoubleSpinBox;
//        QFTSensorValues[i]->setReadOnly(true);
//        QFTSensorValues[i]->setDecimals(6);
//        QFTSensorValues[i]->setValue(0.0);
//        QFTSensorValues[i]->setSingleStep(0.10);
//        QFTSensorValues[i]->setMinimum(-100.0);
//        QFTSensorValues[i]->setMaximum(100.0);
//        ftValuesLayout->addWidget(QFTSensorValues[i]);
//    }
    ftValuesLayout->addWidget(QFTSensorValues);
    spinBoxLayout->addLayout(ftValuesLayout);

    QHBoxLayout * buttonLayout = new QHBoxLayout;
    RebiasButton = new QPushButton("Rebias");
    buttonLayout->addWidget(RebiasButton);
    buttonLayout->addStretch();

    QVBoxLayout * sensorPlotLayout = new QVBoxLayout;
    SensorRTPlotFPS = new QLabel("0 FPS");
    SensorRTPlot = SetupRealTimePlot("Time", "Force");        
    SensorRTPlot->setFixedHeight(200);
    SensorRTPlot->replot();    

    sensorPlotLayout->addWidget(SensorRTPlot);
    sensorPlotLayout->addWidget(SensorRTPlotFPS);

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
    tabWidget->addTab(tab2, "Intervel Stats");
    tabWidget->show();

    setWindowTitle("ATI Force Sensor(N, N-mm)");    
    resize(sizeHint());

    // setup Qt Connection
    connect(RebiasButton, SIGNAL(clicked()), this, SLOT(RebiasFTSensor()));    
}

QCustomPlot* mtsATINetFTQtWidget::SetupRealTimePlot(const std::string XAxis, const std::string Yaxis)
{
    QCustomPlot * plot = new QCustomPlot;
    plot->addGraph();
    plot->graph(0)->setPen(QPen(Qt::blue));
    plot->graph(0)->setBrush(QBrush(QColor(240, 255, 200)));
    plot->graph(0)->setAntialiasedFill(false);
    plot->addGraph(); // blue dot
    plot->graph(1)->setPen(QPen(Qt::blue));
    plot->graph(1)->setLineStyle(QCPGraph::lsNone);
    plot->graph(1)->setScatterStyle(QCPScatterStyle::ssDisc);

    plot->xAxis->setLabel(XAxis.c_str());
    plot->yAxis->setLabel(Yaxis.c_str());

    plot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    plot->xAxis->setDateTimeFormat("hh:mm:ss");
    plot->xAxis->setAutoTickStep(false);
    plot->xAxis->setTickStep(2);
    plot->axisRect()->setupFullAxesBox();

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(plot->xAxis, SIGNAL(rangeChanged(QCPRange)), plot->xAxis2, SLOT(setRange(QCPRange)));
    connect(plot->yAxis, SIGNAL(rangeChanged(QCPRange)), plot->yAxis2, SLOT(setRange(QCPRange)));

    return plot;
}

void mtsATINetFTQtWidget::UpdateRealTimePlot(QCustomPlot *plot, double key, double value)
{
    plot->graph(0)->addData(key, value);       // add data to lines:
    plot->graph(1)->clearData();
    plot->graph(1)->addData(key, value);       // set data of dots:
    plot->graph(0)->removeDataBefore(key-10);    // remove data of lines that's outside visible range:
    plot->graph(0)->rescaleValueAxis();         // rescale value (vertical) axis to fit the current data:

    // make key axis range scroll with the data (at a constant range size of 10):
    plot->xAxis->setRange(key+0.25, 10, Qt::AlignRight);
    plot->replot();

    // calculate frames per second:
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;

    if (key-lastFpsKey > 2) // average fps over 2 seconds
    {
        SensorRTPlotFPS->setText(
            QString("%1 FPS")
            .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)            
            );
      lastFpsKey = key;
      frameCount = 0;
    }
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
    double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
    vctDoubleVec forceOnly(3,0.0);
    forceOnly[0] = QFTSensorValues->itemAt(0,0)->data(0).toReal();
    forceOnly[1] = QFTSensorValues->itemAt(0,1)->data(0).toReal();
    forceOnly[2] = QFTSensorValues->itemAt(0,2)->data(0).toReal();
    UpdateRealTimePlot(SensorRTPlot, key, forceOnly.Norm());

    ForceSensor.GetPeriodStatistics(IntervalStatistics);
    QMIntervalStatistics->SetValue(IntervalStatistics);
}

void mtsATINetFTQtWidget::RebiasFTSensor(void)
{
    ForceSensor.RebiasFTData();
}

