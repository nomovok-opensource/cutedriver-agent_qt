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
 

 

#include "tasdeviceutils.h"

#include <taslogger.h>
#include <stdlib.h>
#include <sys/resource.h>

#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>

#include <QEvent>

#ifdef TAS_MAEMO
#include <qmsystem/qmdisplaystate.h>
#endif


TasDeviceUtils::TasDeviceUtils()
{
    gpuDetailsHandler = 0;
}

void TasDeviceUtils::resetInactivity() 
{
#ifdef TAS_MAEMO
    TasLogger::logger()->debug("TasDeviceUtils:: resetting inactivity");
    Maemo::QmDisplayState state; 
  
    if (!state.set(Maemo::QmDisplayState::On)) {
        TasLogger::logger()->warning("TasDeviceUtils:: setting displaystate failed!");
    }    

    if (!state.setBlankingPause()) {
        TasLogger::logger()->warning("TasDeviceUtils:: setBlankingPause failed!");
    }    
#endif
}

GpuMemDetails TasDeviceUtils::gpuMemDetails()
{
    GpuMemDetails details;
    details.isValid = false;
    return details;
}

// TODO remove the duplicate code (maemo version)
 
/*!
  Returns the heap size of the process. 
  -1 means not supported.
*/
int TasDeviceUtils::currentProcessHeapSize()
{
    char buf[30];
    unsigned size = -1; //       total program size
    // If needed later.
//    unsigned resident = -1;//   resident set size
//    unsigned share;//      shared pages
//    unsigned text;//       text (code)
//    unsigned lib;//        library
//    unsigned data;//       data/stack
//    unsigned dt;//         dirty pages (unused in Linux 2.6)
    

    snprintf(buf, 30, "/proc/%u/statm", (unsigned)getpid());
    FILE* pf = fopen(buf, "r");
    if (pf) {
        fscanf(pf, "%u "/* %u %u %u %u %u " */, &size /*,&resident, &share, &text, &lib, &data */);
    }
    fclose(pf);
    return size;
}

/*!
 * /proc/cpuinfo data
*/
void TasDeviceUtils::addSystemInformation(TasObject& object)
{
    QFile file("/proc/cpuinfo");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    TasObject& child = object.addObject();
    child.setName("/proc/cpuinfo");
    QTextStream in(&file);
    QString line = in.readLine();
    while (!line.isNull()) {
        QStringList list = line.split(":");
        if (list.size() == 2) {
            child.addAttribute(list[0].trimmed(),list[1].trimmed());
        }
        line = in.readLine();
    }
}

qreal TasDeviceUtils::currentProcessCpuTime()
{
    return -1;
}


/*!
  Not supported
*/
void TasDeviceUtils::addSystemMemoryStatus(TasObject& object)
{

    FILE *meminfo = fopen("/proc/meminfo", "r");
    int ram = -1;
    int ramFree = -1;
    int cached = -1;

    if(meminfo == NULL) {

    } else {
        char line[256];
        while(fgets(line, sizeof(line), meminfo)) {
            if (ram == -1) {
                sscanf(line, "MemTotal: %d kB", &ram);
            } else if (ramFree == -1) {
                sscanf(line, "MemFree: %d kB", &ramFree);
            } else if (cached == -1) {
                sscanf(line, "Cached: %d kB", &cached);
                break;
            }
        }

        object.addAttribute("total",QString::number(ram));
        object.addAttribute("available",QString::number(ramFree));
        object.addAttribute("cached",QString::number(ramFree));



        // If we got here, then we couldn't find the proper line in the meminfo file:
        // do something appropriate like return an error code, throw an exception, etc.
        fclose(meminfo);
    }

}


// void TasDeviceUtils::tapScreen(int x, int y, int duration)
// {    

// }


void TasDeviceUtils::sendMouseEvent(int x, int y, Qt::MouseButton button, QEvent::Type type)
{
    Display* dpy = 0;
    Window root = None;
    dpy = XOpenDisplay(NULL);       
    root = DefaultRootWindow(dpy);    
    if (!dpy) {
        TasLogger::logger()->error("TasDeviceUtils::sendMouseEvent No Display detected! Unable to run X commands");
        return;
    }
    
    int keycode = Button1;
    switch (button) {
    case Qt::LeftButton:
        keycode = Button1;
        break;
    case Qt::MidButton:
        keycode = Button2;
        break;
    case Qt::RightButton:
        keycode = Button3;
        break;
    default:
        break;
    }


    bool down = type == QEvent::MouseButtonPress ||
        type == QEvent::GraphicsSceneMousePress;
    if (down || type == QEvent::MouseMove) { // TODO how about dblclick?
        // Move the Cursor to given coords
        XWarpPointer(dpy, None, root, 0, 0, 0, 0, 
                     x,y);
        XFlush(dpy);        
    } 
    
    if (type == QEvent::MouseButtonPress ||
        type == QEvent::MouseButtonRelease ||
        type == QEvent::GraphicsSceneMousePress ||
        type == QEvent::GraphicsSceneMouseRelease) {
        XTestFakeButtonEvent(dpy, keycode, down, CurrentTime);
        XFlush(dpy);
    }
    

    XCloseDisplay(dpy);
}


