/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
 Author(s):  Marcin Balicki
 Created on: 2015-07-21
 
 (C) Copyright 2013-2015 Johns Hopkins University (JHU), All Rights Reserved.
 
 --- begin cisst license - do not edit ---
 
 This software is provided "as is" under an open source license, with
 no warranty.  The complete license can be found in license.txt and
 http://www.cisst.org/cisst/license.txt.
 
 --- end cisst license ---
 */

#ifndef _sawATINetFTSimulatorQtWidget_h
#define _sawATINetFTSimulatorQtWidget_h

#include <cisstCommon/cmnUnits.h>
#include <cisstVector/vctFixedSizeVectorTypes.h>
#include <cisstVector/vctQtWidgetDynamicVector.h>
#include <cisstMultiTask/mtsComponent.h>

#include <QWidget>
#include <QEvent>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <cisstOSAbstraction/osaSocket.h>

// Always include last
#include <sawATIForceSensor/sawATIForceSensorQtExport.h>

class CISST_EXPORT sawATINetFTSimulatorQtWidget: public QWidget, public mtsComponent
{
  Q_OBJECT;
  CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_ALLOW_ALL);
  
public:
  
  /// update rate, ip and port where to send simulated FT data using UDP
  sawATINetFTSimulatorQtWidget(double periodInSeconds = 50.0 * cmn_ms,
                               std::string ip = "127.0.0.1", int port = 5555);
  ~sawATINetFTSimulatorQtWidget();
   
  //these should be called before
  void SetUpperLimits(double fx = 50, double fy = 50, double fz = 70,
                      double tx = 500, double ty = 500, double tz = 500);
  void SetLowerLimits(double fx = -50, double fy = -50, double fz = -70,
                      double tx = -500, double ty = -500, double tz = -500);
  
  void Startup(void);
  void Cleanup(void);
  void Configure(const std::string & filename = "");

  std::string GetStatus();

  
protected:
  virtual void closeEvent(QCloseEvent * event);
  virtual void keyPressEvent(QKeyEvent *event);
  virtual void keyReleaseEvent(QKeyEvent *event);
  virtual void focusOutEvent(QFocusEvent* event);
  
  typedef struct {
    vct6    ForceTorque;
    int     HasError;
    int     IsSaturated;
  } StateType;
  
  StateType State;

  QCheckBox  * ConnectOnCheckBtn;
  QCheckBox  * SaturationOnCheckBtn;
  QCheckBox  * SpringOnCheckBtn;
  QCheckBox  * ErrorOnCheckBtn;
  QDoubleSpinBox * SpringKSpinBox;
  QPushButton * ZeroBtn;
  
  vctQtWidgetDynamicVectorDoubleWrite * QFTSensorValues;

  vct6 UpperLimit;
  vct6 LowerLimit;

  vctInt6 KeysPlus;
  vctInt6 KeysMinus;
  
  vctBool6 IsKeyPlusDown;
  vctBool6 IsKeyMinusDown;
  
  std::string IP;
  int Port;
  
  osaSocket Socket;

  double UpdatePeriod;
  
private slots:
  void timerEvent(QTimerEvent * event);
  void SlotZeroBtnClicked();
  
};

CMN_DECLARE_SERVICES_INSTANTIATION(sawATINetFTSimulatorQtWidget);

#endif // _sawATINetFTSimulatorQtWidget_h
