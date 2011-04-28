/*************************************************************************** 
** 
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). 
** All rights reserved. 
** Contact: Nokia Corporation (testabilitydriver@nokia.com) 
** 
** This file is part of Testability Driver Qt Agent
** 
** If you have questions regarding the use of this file, please contact 
** Nokia at testabilitydriver@nokia.com . 
** 
** This library is free software; you can redistribute it and/or 
** modify it under the terms of the GNU Lesser General Public 
** License version 2.1 as published by the Free Software Foundation 
** and appearing in the file LICENSE.LGPL included in the packaging 
** of this file. 
** 
****************************************************************************/ 
 


#include "pwrinfo_symbian.h"
#include "taslogger.h"

#if ( NCP_COMMON_S60_VERSION_SUPPORT >= S60_VERSION_50 && NCP_COMMON_FAMILY_ID >= 70 )

PwrDetailsHandler::PwrDetailsHandler()
  : mPower(NULL)
{
    StartMeasurement();
}

PwrDetailsHandler::~PwrDetailsHandler()
{
  if(mPower)
  {
    TRAPD(err,
       mPower->StopAveragePowerReportingL();
    );//TRAPD
    Q_UNUSED(err);
    delete mPower;
    mPower = 0;
  }
}

bool PwrDetailsHandler::StartMeasurement()
{

  TRAPD(err,
    //centRep object, clean after set poll with PopAndDestroy
    CRepository* centRep = CRepository::NewLC(KCRUidPowerSettings);
    // Instantiate power object, cleaned on destructor
    mPower = CHWRMPower::NewL();
    //set poll values for infinite time
    User::LeaveIfError(centRep->Set(KPowerMaxReportingPeriod, 0));
    CleanupStack::PopAndDestroy(centRep);

    mPower->SetPowerReportObserver(this);
    TRequestStatus status(KErrNone);
    // Start the power consumption attribute notification
    TInt ret = mPower->StartAveragePowerReporting(status, 1);
    User::WaitForRequest(status);
    User::LeaveIfError(status.Int());
  );//TRAPD
  return err == KErrNone;
}

void PwrDetailsHandler::PowerMeasurement( TInt aErrCode, CHWRMPower::TBatteryPowerMeasurementData& aMeasurement)
{
  //TasLogger::logger()->debug(QString(__FUNCTION__) + " callback");

  PwrDetails details;
  details.voltage = aMeasurement.iAverageVoltage;
  details.current = aMeasurement.iAverageCurrent;
  mDetailsList.append(details);
}

PwrDetails PwrDetailsHandler::pwrData()
{
  static PwrDetails details;
  int count = mDetailsList.count();
  if(count>0){
    details.voltage=0;
    details.current=0;
    details.isValid=true;

    while(!mDetailsList.isEmpty()){
      PwrDetails det = mDetailsList.takeFirst();
      details.voltage += det.voltage;
      details.current += det.current;
    }
    details.voltage /= count;
    details.current /= count;
  }
  return details;
}

#endif
