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
 



#ifndef TASNATIVEUTILS_H
#define TASNATIVEUTILS_H

#include "tasclientmanager.h"

const char* const ROTATE_TOP_UP   = "rotate_top_up";
const char* const ROTATE_RIGHT_UP = "rotate_right_up";

class TasNativeUtils {
public:
    /*! Pid of the active window, only ids registered to qttas are taken
     * into account.
     */
    static int pidOfActiveWindow(const QHash<QString, TasClient*> clients);
    static int bringAppToForeground(quint64 pid);
    static void changeOrientation(QString);
	static bool killProcess(quint64 pid);
	static bool verifyProcess(quint64 pid);
	static bool processExitStatus(quint64 pid, int& status);

};

#endif
