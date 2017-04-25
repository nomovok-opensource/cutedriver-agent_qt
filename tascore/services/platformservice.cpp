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



#include <QApplication>
#include <QPoint>
#include <QRect>

#include "platformservice.h"

#if (defined(Q_OS_WIN32) || defined(Q_OS_WINCE))
#include <windows.h>
#include <commctrl.h>
#endif

#ifdef Q_WS_MAC
#include <Carbon/Carbon.h>
#endif


/*!
  \class PlatformService
  \brief PlatformService closes the application

*/

PlatformService::PlatformService()
{
}

PlatformService::~PlatformService()
{
}

bool PlatformService::executeService(TasCommandModel& model, TasResponse& response)
{
    Q_UNUSED(response);
    TasLogger::logger()->debug("PlatformService::executeService service:" + model.service() + " and name: " + model.name());
    if(model.service() == serviceName() ){
        doPlatformSpecificCommands(model);
        return true;
    }
    else{
        return false;
    }
}

/*!

  Checks the xml for commands that require platform specific actions.
  Such actions are open file and save file operations. Returns true if
  such actions are found and performed.

 */
void PlatformService::doPlatformSpecificCommands(TasCommandModel& commandModel)
{
    QListIterator<TasTarget*> i(commandModel.targetList());
    while (i.hasNext()){
        TasTarget* commandTarget = i.next();
        QListIterator<TasCommand*> j(commandTarget->commandList());
        while (j.hasNext()){
            TasCommand* command = j.next();
            TasLogger::logger()->debug("PlatformService::doPlatformSpecificCommands: " + commandModel.service());
            if(command->name() == "OpenFile"){
                QString dialogName = command->parameter("dialogName");
                QString filePath = command->parameter("filePath");
                QString buttonName = command->parameter("dialogButton");
                //send to plat specific operation
#ifdef USE_TAS_OS_SPEC
                sendFileOpenMessage(dialogName, filePath, buttonName);
#endif
                //one supported currently
                break;
            }
            else if(command->name() == "PressEnter"){
                pressEnter();
            }
        }
    }
}

#ifdef USE_TAS_OS_SPEC

/*!

    File open dialogs in windows and osx are implemented using platform services.
    This makes it impossible for us to control them in the same manner as all of
    the other components. For this reason we need to use platform apis to control
    open file services.

 */
void PlatformService::sendFileOpenMessage(const QString& dialogName, const QString& filePath, const QString& buttonName)
{
#if (defined(Q_OS_WIN32) || defined(Q_OS_WINCE))
    HWND WindowHandle;
    HWND ButtonHandle;
    HWND TextBoxHandle;
    wchar_t* dialog = converToWChar(dialogName);
    WindowHandle = FindWindow(NULL, dialog);
    delete dialog;
    //look for the correct control and send a set text message to it.
    if (WindowHandle){
        TextBoxHandle = FindWindowEx(WindowHandle, 0, L"ComboBoxEx32", NULL);
        if (TextBoxHandle){
            HWND ComboBoxHandle = FindWindowEx(TextBoxHandle, 0, L"ComboBox", NULL);
            if (ComboBoxHandle){
                HWND EditHandle = FindWindowEx(ComboBoxHandle, 0, L"Edit", NULL);
                if (EditHandle){
                    wchar_t* file = converToWChar(filePath);
                    SendMessage(EditHandle, WM_SETTEXT, 0, (LPARAM)file);
                    delete file;
                }
            }
        }
        //send button click to open button
        wchar_t* button = converToWChar(buttonName);
        ButtonHandle = FindWindowEx(WindowHandle, 0, L"Button", button);
        delete button;
        if (ButtonHandle){
            SendMessage (ButtonHandle, BM_CLICK, 0 , 0);
        }
    }
#elif defined(Q_WS_MAC)
    WindowRef window = NULL;
    //window = GetFrontWindowOfClass(kDocumentWindowClass, false);
    window = FrontWindow();
    if(window){
      HIViewRef hiview = HIViewGetRoot(window);
      if (hiview){
    }
      CollapseWindow(window, true);
    }
#else
    Q_UNUSED(dialogName)
    Q_UNUSED(filePath)
    Q_UNUSED(buttonName)
#endif
}

#if (defined(Q_OS_WIN32) || defined(Q_OS_WINCE))
wchar_t* PlatformService::converToWChar(const QString& aString)
{
    wchar_t* wString = new wchar_t[aString.size()+1];
    aString.toWCharArray(wString);
    wString[aString.size()] = 0;//no null added by default so need to do it
    return wString;
}
#endif

#if (defined(Q_OS_WIN32) || defined(Q_OS_WINCE))
void PlatformService::pressEnter()
{
    keybd_event(VK_RETURN,0,0,0);
}
#endif

#endif


