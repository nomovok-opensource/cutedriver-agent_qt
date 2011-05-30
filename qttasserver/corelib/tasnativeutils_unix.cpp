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
 

#include "tasnativeutils.h"

#include <taslogger.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <signal.h>

    

int pidOfXWindow(Display* display, Window win) 
{
    int pid = -1;
    Atom atom;
    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    unsigned long bytes_after;
    unsigned char *prop = 0;
    int status;
    if (win != None && win != 0) {
        atom = XInternAtom(display, "_NET_WM_PID", True);
        if (atom != None) {
            status = XGetWindowProperty(display, win, atom, 0, 1024,
                                        False, AnyPropertyType,
                                        &actual_type,
                                        &actual_format, &nitems,
                                        &bytes_after,
                                        &prop);
            if (status!=0 || !prop || actual_format == None) {
                //NOP
            } else {
                pid = prop[1] * 256;
                pid += prop[0];  
            }
        }
    }  
    if (prop) XFree(prop);
    return pid;
}

//#include <cstdio>

/*!
  \class TasNativeUtils
  \brief Provides platform dependent operations.    
*/

// Window id of the currently active window



bool  dockWindow(Display* dpy, Window window) 
{
    unsigned long nitems, after;
    int format;
    Atom type = None;
    unsigned char* data = NULL;
    Atom _NET_WM_WINDOW_TYPE_DOCK = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK",
                                           False);
    Atom WINDOW_TYPE = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);    

    Atom curr_type;

    if (Success == XGetWindowProperty(dpy, window, WINDOW_TYPE, 0, 0x7FFFFFFF, False, AnyPropertyType,
                                      &type, &format, &nitems, &after, &data)) {
        if (!data) {
            return false;
        }
        for (int i = 0; i < (int)nitems; ++i) {
            curr_type = ((Atom*)data)[i];
            if (curr_type == _NET_WM_WINDOW_TYPE_DOCK) {
                XFree(data);
                return true;
            }
        }
        
    }

    if (data) XFree(data);
    return false;
}


long getstate(Display* display, Window window)
{
    static const long WM_STATE_ELEMENTS = 2L;
    
    unsigned long nitems;
    unsigned long leftover;
    Atom xa_WM_STATE, actual_type;
    int actual_format;
    int status;
    unsigned char* p = NULL;
    
    xa_WM_STATE = XInternAtom(display, "WM_STATE", True);

    if (window == None || window == 0) {
        return -1;
    }
    status = XGetWindowProperty(display, window,
                                xa_WM_STATE, 0L, WM_STATE_ELEMENTS,
                                False, xa_WM_STATE, &actual_type, &actual_format,
                                &nitems, &leftover, &p);
    long value = -1;
    
    if (status == 0) {
        if (p != NULL) {
            // Take first 8 bits
            // Wonder if this is the same on a 64 bit system?
            value = * (const unsigned long *) p & 0xffffffff; 
            XFree(p);
        }

        return value;
    }


    if (p) XFree(p);
    return -1;
}

Window queryStack(Display* dpy, Window root, const QList<QString>& pids)
{
    Atom STACK = XInternAtom(dpy, "_NET_CLIENT_LIST_STACKING", False);
    unsigned long nitems, after;
    int format;
    Atom type = None;
    unsigned char* data = NULL;
    Window child;
    int i;
            
    if (!STACK) {
        return root;
    }
    if (Success == XGetWindowProperty(dpy, root, STACK, 0, 0x7FFFFFFF, False, XA_WINDOW,
                                      &type, &format, &nitems, &after, &data)) {
        for (i = (nitems-1); i >= 0; --i) {
            child = ((Window*)data)[i];
            if (!dockWindow(dpy, child) && getstate(dpy, child) == 1 && 
                pids.contains(QString::number(pidOfXWindow(dpy, child)))) {

                if (data) XFree(data);
                return child;
            }
        }
            
    }
    if (data) XFree(data);
    return None;
    
}

Window tryChildren(Display *dpy, Window win)
{
    Window root, parent;
    Window *children;
    unsigned int nchildren;
    int i;
    Window inf = 0;
    Window child;
    if (!XQueryTree(dpy, win, &root, &parent, &children, &nchildren)) {
        return 0;
    }
    // Traverse bottom-up direction
//    for (i = 0; !inf && (i < nchildren); i++) {
    for (i = nchildren-1; !inf && (i >= 0); i--) {
        // NormalState per ICCM
        if (getstate(dpy, children[i]) == 1) {
            inf = children[i];
        } else {
            child = tryChildren(dpy, children[i]);
            if (child) {
                inf = child;
            }
        }
    }
    if (children) {
        XFree(children);
    }

    return inf;
}

Window findActiveWindow(Display *dpy, Window win)
{
    Window inf;
    if (getstate(dpy, win) == 1) {
        return win;
    }
    inf = tryChildren(dpy, win);
    if (!inf)
        inf = win;
    return inf;
}

// Ignore errors related to the Window calls (of course, the pid is not resolved)
int errorHandler(Display*, XErrorEvent* e) 
{
    TasLogger::logger()->warning("TasNativeUtils::errorHandler received X Error during request");
    if (e->error_code == BadWindow) {
        // More verbose on the most common one
        TasLogger::logger()->warning("TasNativeUtils::errorHandler BadWindow");
    } else {
        TasLogger::logger()->warning("TasNativeUtils::errorHandler code " + 
                                   QString::number(e->error_code));
    }
    // XGetErrorText contains more info, if necessary
    return 0;
}


// Meego specific window attribute
// Try that before anything else.
int pidOfMeegoWindow(Display* display, Window root)
{
    unsigned long nitems, after;
    int format;
    Atom type = None;
    unsigned char* data = NULL;
    int pid = -1;
    Window child;

    Atom atom = XInternAtom(display, "_MEEGOTOUCH_CURRENT_APP_WINDOW", True);
    if (atom != None) {
        if (Success == XGetWindowProperty(display, root, atom, 0, 0x7FFFFFFF, False, XA_WINDOW,
                                          &type, &format, &nitems, &after, &data)) {
            if (nitems > 0) {
                child = ((Window*)data)[0];
                pid = pidOfXWindow(display,child);
                if (data) XFree(data);
            }
        }
    }
    return pid;
}


int TasNativeUtils::pidOfActiveWindow(const QHash<QString, TasClient*> clients)
{
    const QList<QString>& pids = clients.keys();
    TasLogger::logger()->debug("TasNativeUtils::pidOfActiveWindow Querying for active window");
    int pid = -1;
    Display* display = 0;
    display = XOpenDisplay(0); 
    XSetErrorHandler(errorHandler);
    if (!display) return -1;

    int screen = XDefaultScreen(display);
    int window = RootWindow(display, screen);

    // First try to use whatever MeeGo tells us
    //pid = pidOfMeegoWindow(display, window);
    if (pid != -1) {
        TasLogger::logger()->debug("TasNativeUtils::pidOfActiveWindow Found MeeGo pid: " + QString::number(pid));
    } else {
        Window win = queryStack(display, window, pids);
        pid = pidOfXWindow(display, win);
    }
    XCloseDisplay(display);    

    TasLogger::logger()->debug("TasNativeUtils::pidOfActiveWindow Resolved " + QString::number(pid));
    return pid;
}



int TasNativeUtils::bringAppToForeground(quint64 pid)
{
    Q_UNUSED(pid);
    return -1;
}

void TasNativeUtils::changeOrientation(QString)
{}

bool TasNativeUtils::killProcess(quint64 pid)
{ 
    kill(pid, 9);
    return true;
}


bool TasNativeUtils::verifyProcess(quint64 pid)
{
    // kill(pid,0) ?
    char path[256];
    sprintf(path, "/proc/%d", int(pid));
    return access(path, F_OK) != -1;
}


bool TasNativeUtils::processExitStatus(quint64 pid, int &status)
{
    if (verifyProcess(pid)) return false;
    status = 0;
    return true;
}

void TasNativeUtils::runningProcesses(TasObject& applist)
{
}

