/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
 Author(s):  Marcin Balicki
 Created on: 2015-07-24
 
 (C) Copyright 2015 Johns Hopkins University (JHU), All Rights Reserved.
 
 --- begin cisst license - do not edit ---
 
 This software is provided "as is" under an open source license, with
 no warranty.  The complete license can be found in license.txt and
 http://www.cisst.org/cisst/license.txt.
 
 --- end cisst license ---
 */

#include <sawATIForceSensor/sawATINetFTSimulatorQtWidget.h>
#include <cisstVector/vctDynamicVectorTypes.h>

// system includes
#include <iostream>
#include <sstream>   //ostringstream istringstream

// Qt includes
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QString>
#include <QtGui>
#include <QLabel>
#include <QtGui>
#include <QApplication>


CMN_IMPLEMENT_SERVICES(sawATINetFTSimulatorQtWidget);

sawATINetFTSimulatorQtWidget::sawATINetFTSimulatorQtWidget(double periodInSeconds, std::string ip, int port):
cmnGenericObject(),
IP(ip),
Port(port),
Socket(osaSocket::UDP),
UpdatePeriod(periodInSeconds)
{
  
  //setup defaults.
  SetUpperLimits();
  SetLowerLimits();
  
  KeysPlus = vctInt6(Qt::Key_T, Qt::Key_Y, Qt::Key_U, Qt::Key_I, Qt::Key_O, Qt::Key_P);
  KeysMinus = vctInt6(Qt::Key_G, Qt::Key_H, Qt::Key_J, Qt::Key_K, Qt::Key_K, Qt::Key_L);
  
  IsKeyMinusDown.Zeros();
  IsKeyPlusDown.Zeros();
  
  for (unsigned int i = 0 ; i < 6; ++i) {
    State.ForceTorque[i] = 0;
  }
  
  State.HasError = 0;
  State.IsSaturated = 0;
  Socket.SetDestination(IP, Port);
}

sawATINetFTSimulatorQtWidget::~sawATINetFTSimulatorQtWidget(){
  
  Socket.Close();
}

void sawATINetFTSimulatorQtWidget::setupUi()
{
  QFont font;
  font.setBold(true);
  font.setPointSize(12);
  
  setFocusPolicy(Qt::ClickFocus);
  
  QVBoxLayout * mainLayout = new QVBoxLayout;
  // Vectors of values
  QGridLayout * labelsLayout = new QGridLayout;
  mainLayout->addLayout(labelsLayout);
  
  std::vector<QString> qFtlabels(6);
  qFtlabels[0] = QString("fx");
  qFtlabels[1] = QString("fy");
  qFtlabels[2] = QString("fz");
  qFtlabels[3] = QString("tx");
  qFtlabels[4] = QString("ty");
  qFtlabels[5] = QString("tz");
  
  for (unsigned int i = 0; i < 6; ++i){
    QString str = qFtlabels[i] + QString("[+") +
                  QKeySequence(KeysPlus[i]).toString() +QString(":-") +
                  QKeySequence(KeysMinus[i]).toString() + QString("]");
    str = str.toLower();
    
    QLabel * label = new QLabel(str, this);
    label->setAlignment(Qt::AlignCenter);
    
    labelsLayout->addWidget(label, 0, i);
  }

  QFTSensorValues = new vctQtWidgetDynamicVectorDoubleWrite(vctQtWidgetDynamicVectorDoubleWrite::SPINBOX_WIDGET);
  QFTSensorValues->SetPrecision(2);
  vctDoubleVec ft;
  ft.SetSize(6);
  QFTSensorValues->SetValue(ft);
  // assigned after columns are established.
  QFTSensorValues->SetRange(vctDoubleVec(LowerLimit), vctDoubleVec(UpperLimit));
  mainLayout->addWidget(QFTSensorValues);
  
  // Layout containing rebias button
  QHBoxLayout * buttonLayout = new QHBoxLayout;
  buttonLayout->addStretch();
  
  ConnectOnCheckBtn = new QCheckBox("ConnectOn", this);
  buttonLayout->addWidget(ConnectOnCheckBtn);
  ConnectOnCheckBtn->setChecked(true);
  
  SaturationOnCheckBtn = new QCheckBox("SaturationOn", this);
  buttonLayout->addWidget(SaturationOnCheckBtn);
  SaturationOnCheckBtn->setChecked(false);
  
  ErrorOnCheckBtn = new QCheckBox("ErrorOn", this);
  buttonLayout->addWidget(ErrorOnCheckBtn);
  ErrorOnCheckBtn->setChecked(false);
  
  SpringOnCheckBtn = new QCheckBox("SpringKOn", this);
  buttonLayout->addWidget(SpringOnCheckBtn);
  SpringOnCheckBtn->setChecked(true);
  
  mainLayout->addLayout(buttonLayout);
  
  SpringKSpinBox = new QDoubleSpinBox(this);
  buttonLayout->addWidget(SpringKSpinBox);
  SpringKSpinBox->setMinimum(0.1);
  SpringKSpinBox->setMaximum(10000.0);
  SpringKSpinBox->setValue(100.0);
 
  
  QString ipPort = QString ("Sending to: ") + QString::fromStdString(IP) + QString(':') + QString::number(Port);
  QLabel *ipLabel = new QLabel(ipPort, this);
  
  mainLayout->addWidget(ipLabel);
  
  setLayout(mainLayout);
  setWindowTitle("ATI NET FT Simulator");
  resize(sizeHint());
  resize(500,100);
  
  this->show();
  startTimer(UpdatePeriod * 1000);
  
}

void sawATINetFTSimulatorQtWidget::timerEvent(QTimerEvent * event){
  
  vctDoubleVec ft;
  ft.SetSize(6);
  
  
  /// Constant for spring
  vct6 SpringK;
  
  SpringK[0] = SpringKSpinBox->value();
  SpringK[1] = SpringKSpinBox->value();
  SpringK[2] = SpringKSpinBox->value();
  SpringK[3] = SpringK[0] * 10.0;
  SpringK[4] = SpringK[1] * 10.0;
  SpringK[5] = SpringK[2] * 10.0;
  
  // go through each force reading and based on keyboard events increase/decrease up to a limit.
  // if not keyboard key pressed then go back to zero if (springON)
  for (unsigned int i = 0 ; i < 6; ++i) {
    //check if key down:
    double k = SpringK[i] * UpdatePeriod * 2.0;
    double kInput = SpringK[i] * UpdatePeriod * 1.0;
    
    
    if (IsKeyPlusDown[i]) {
      State.ForceTorque[i] += kInput;
      if (State.ForceTorque[i] > UpperLimit[i] )
      {
        State.ForceTorque[i] = UpperLimit[i];
      }
    }
    else if (IsKeyMinusDown[i]) {
      State.ForceTorque[i] -= kInput;
      if (State.ForceTorque[i] < LowerLimit[i] )
      {
        State.ForceTorque[i] = LowerLimit[i];
      }
    }
    else if (SpringOnCheckBtn->isChecked()) {
      if (State.ForceTorque[i] > k  )
      {
        State.ForceTorque[i] -= k;
      }
      else if (State.ForceTorque[i] < -k)
      {
        State.ForceTorque[i] += k;
      }
      else {
        State.ForceTorque[i] = 0;
      }
    }
    else {  //otherwise get the double spin box input.
      vctDoubleVec v(6);
      QFTSensorValues->GetValue(v);
      State.ForceTorque[i] = v[i];
    }
    ft[i] = State.ForceTorque[i];
  }
  
  QFTSensorValues->SetValue(ft);
  
  //special events.
  if (SaturationOnCheckBtn->isChecked())
    State.IsSaturated = 1;
  else
    State.IsSaturated = 0;
  
  if (ErrorOnCheckBtn->isChecked())
    State.HasError = 1;
  else
    State.HasError = 0;
  
  if(ConnectOnCheckBtn->isChecked()) {
    
    //      *(uint16*)&(Data->Request)[0] = htons(0x1234);
    //      *(uint16*)&(Data->Request)[2] = htons(0); /* Stop streaming */
    //      *(uint32*)&(Data->Request)[4] = htonl(ATI_NUM_SAMPLES);
    /// \todo FIX NETWORK BYTE ORDER
    // try to send, but timeout after 10 ms
    int result = Socket.Send((const char *)(&State), sizeof(sawATINetFTSimulatorQtWidget::StateType), 10.0 * cmn_ms);
    
    //std::cout << sizeof(sawATINetFTSimulatorQtWidget::StateType) << std::endl;
    
    
    if (result == -1) {
      CMN_LOG_CLASS_RUN_WARNING << "Cleanup: UDP send failed" << std::endl;
      return;
    }
  }
  
  CMN_LOG_CLASS_RUN_DEBUG << GetStatus() << std::endl;
}

void sawATINetFTSimulatorQtWidget::keyPressEvent(QKeyEvent *event) {
  
  
  for (unsigned int i = 0; i < 6; i ++) {
    if (event->key() == KeysPlus[i]) {
      IsKeyPlusDown[i] = true;
    }
    if (event->key() == KeysMinus[i]) {
      IsKeyMinusDown[i] = true;
    }
  }
  event->accept();
}


void sawATINetFTSimulatorQtWidget::keyReleaseEvent(QKeyEvent *event) {
  
  for (unsigned int i = 0; i < 6; i++) {
    if (event->key() == KeysPlus[i]) {
      IsKeyPlusDown[i] = false;
    }
    if (event->key() == KeysMinus[i]) {
      IsKeyMinusDown[i] = false;
    }
  }
  event->accept();
}

void sawATINetFTSimulatorQtWidget::closeEvent(QCloseEvent *event)
{
  event->accept();
  CMN_LOG_CLASS_RUN_ERROR << "QUITING QT" << std::endl;
  QApplication::quit();
}

//these should be called before
void sawATINetFTSimulatorQtWidget::SetUpperLimits(double fx, double fy, double fz,
                    double tx, double ty, double tz) {
  UpperLimit = vct6(fx, fy, fz, tx, ty, tz);
  CMN_LOG_CLASS_INIT_VERBOSE <<  "Upper limit : " << UpperLimit;
}
void sawATINetFTSimulatorQtWidget::SetLowerLimits(double fx, double fy, double fz ,
                    double tx, double ty, double tz) {
  LowerLimit = vct6(fx, fy, fz, tx, ty, tz);
  CMN_LOG_CLASS_INIT_VERBOSE <<  "Lower limit : " << LowerLimit;
}

std::string sawATINetFTSimulatorQtWidget::GetStatus() {
  
  std::stringstream ss;
  char delim = ' ';
  ss << std::setiosflags(std::ios::fixed)
  <<std::setprecision(1);
  
  for (unsigned int i = 0; i < 6; ++i){
    ss << State.ForceTorque[i] << delim;
  }
  ss << State.HasError << delim;
  ss << State.IsSaturated;
  return ss.str();
  
}



