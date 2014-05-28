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
 

#if ( NCP_COMMON_S60_VERSION_SUPPORT >= S60_VERSION_50 && NCP_COMMON_FAMILY_ID >= 70 )

#ifndef PWRINFO_H
#define PWRINFO_H


#include <e32std.h>
#include <hwrmpower.h>
#include <centralrepository.h>
#include <hwrm/hwrmpowerdomaincrkeys.h>
#include <QList>

#include "tasdeviceutils.h"



class PwrDetailsHandler : public CBase, public MHWRMBatteryPowerObserver, public PwrDetailsInterface
{
public:
   PwrDetailsHandler();
   ~PwrDetailsHandler();

   bool StartMeasurement();

   //MHWRMBatteryPowerObserver
   virtual void PowerMeasurement( TInt aErrCode, CHWRMPower::TBatteryPowerMeasurementData& aMeasurement);

   //PwrDetailsInterface
   virtual PwrDetails pwrData();

private:
   CHWRMPower* mPower;
   QList<PwrDetails> mDetailsList;
};
#endif //PWRINFO_H

#endif
