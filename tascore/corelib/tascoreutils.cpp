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
 


#include <QStringList>
#include <QTime>
#include <QCoreApplication>
#include <QThread>
#include <QVariant>

#if defined(Q_OS_UNIX) && !defined(Q_OS_SYMBIAN)
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#endif

#if defined(Q_OS_UNIX) && !defined(Q_OS_SYMBIAN)
#include <stdlib.h>
#include <stdio.h>
#endif

#include "tascoreutils.h"
#include "tasdeviceutils.h"
#include "testabilitysettings.h"

void Sleeper::sleep(int ms)
{
        QThread::msleep(ms);
}


QString TasCoreUtils::getApplicationName()
{

#if defined(Q_OS_UNIX) && !defined(Q_OS_SYMBIAN)
    QString name = "unk";
    unsigned pid = -1;
    unsigned bytes = -1;
    char buf[50];
    char buf_name[300];
    snprintf(buf, 50, "/proc/%u/cmdline", (unsigned)getpid());
    FILE* pf = fopen(buf, "r");

    if (pf) {
	bytes = fread(buf_name, 1, 300, pf);

	if (bytes) {
        	name = buf_name;
	}
	fclose(pf);
    }
#else
    QString name = QCoreApplication::applicationFilePath();
#endif
    return parseExecutable(name);

}

QString TasCoreUtils::parseExecutable(QString completePath)
{
    QString name = "unk";

    completePath = completePath.replace("\\","/");
    QStringList list = completePath.split("/");
    name = list.last();
    name = name.remove(".exe", Qt::CaseInsensitive);

    return name;
}


QString TasCoreUtils::eventType(QEvent* event)
{
    switch(event->type())
    {
        case QEvent::None: // 0 Not an event. 
            return QString("Unkown");
        case QEvent::AccessibilityDescription: // 130 Used to query accessibility description texts (QAccessibleEvent). 
            return QString("AccessibilityDescription");
        case QEvent::AccessibilityHelp: // 119 Used to query accessibility help texts (QAccessibleEvent). 
            return QString("AccessibilityHelp");
        case QEvent::AccessibilityPrepare: // 86 Accessibility information is requested. 
            return QString("AccessibilityPrepare");
        case QEvent::ActionAdded: // 114 A new action has been added (QActionEvent). 
            return QString("AccessibilityPrepare");
        case QEvent::ActionChanged: // 113 An action has been changed (QActionEvent). 
            return QString("ActionChanged");
        case QEvent::ActionRemoved: // 115 An action has been removed (QActionEvent). 
            return QString("ActionRemoved");
        case QEvent::ActivationChange: // 99 A widget's toplevel window activation state has changed. 
            return QString("ActivationChange");
        case QEvent::ApplicationActivate: // 121 The application has been made available to the user. 
            return QString("ApplicationActivate");
        case QEvent::ApplicationDeactivate: // 122 The application has been suspended, and is unavailable to the user. 
            return QString("ApplicationDeactivate");
        case QEvent::ApplicationFontChange: // 36 The default application font has changed. 
            return QString("ApplicationFontChange");
        case QEvent::ApplicationLayoutDirectionChange: // 37 The default application layout direction has changed. 
            return QString("ApplicationLayoutDirectionChange");
        case QEvent::ApplicationPaletteChange: // 38 The default application palette has changed. 
            return QString("ApplicationPaletteChange");
        case QEvent::ApplicationWindowIconChange: // 35 The application's icon has changed. 
            return QString("ApplicationWindowIconChange");
        case QEvent::ChildAdded: // 68 An object gets a child (QChildEvent). 
            return QString("ChildAdded");
        case QEvent::ChildPolished: // 69 A widget child gets polished (QChildEvent). 
            return QString("ChildPolished");
        case QEvent::ChildRemoved: // 71 An object loses a child (QChildEvent). 
            return QString("ChildRemoved");
        case QEvent::Clipboard: // 40 The clipboard contents have changed (QClipboardEvent). 
            return QString("Clipboard");
        case QEvent::Close: // 19 Widget was closed (QCloseEvent). 
            return QString("Close");
        case QEvent::ContentsRectChange: // 178 The margins of the widget's content rect changed. 
            return QString("ContentsRectChange");
        case QEvent::ContextMenu: // 82 Context popup menu (QContextMenuEvent). 
            return QString("ContextMenu");
        case QEvent::CursorChange: // 183 The widget's cursor has changed. 
            return QString("CursorChange");
        case QEvent::DeferredDelete: // 52 The object will be deleted after it has cleaned up. 
            return QString("DeferredDelete");
        case QEvent::DragEnter: // 60 The cursor enters a widget during a drag and drop operation (QDragEnterEvent). 
            return QString("DragEnter");
        case QEvent::DragLeave: // 62 The cursor leaves a widget during a drag and drop operation (QDragLeaveEvent)sleep.
            return QString("DragLeave");
        case QEvent::DragMove: // 61 A drag and drop operation is in progress (QDragMoveEvent). 
            return QString("DragMove");
        case QEvent::Drop: // 63 A drag and drop operation is completed (QDropEvent). 
            return QString("Drop");
        case QEvent::EnabledChange: // 98 Widget's enabled state has changed. 
            return QString("EnabledChange");
        case QEvent::Enter: // 10 Mouse enters widget's boundaries. 
            return QString("Enter");
        case QEvent::EnterWhatsThisMode: // 124 Send to toplevel widgets when the application enters "What's This?" mode. 
            return QString("EnterWhatsThisMode");
        case QEvent::FileOpen: // 116 File open request (QFileOpenEvent). 
            return QString("FileOpen");
        case QEvent::FocusIn: // 8 Widget gains keyboard focus (QFocusEvent). 
            return QString("FocusIn");
        case QEvent::FocusOut: // 9 Widget loses keyboard focus (QFocusEvent). 
            return QString("FocusOut");
        case QEvent::FontChange: // 97 Widget's font has changed. 
            return QString("FontChange");
        case QEvent::GrabKeyboard: // 188 Item gains keyboard grab (QGraphicsItem only). 
            return QString("GrabKeyboard");
        case QEvent::GrabMouse: // 186 Item gains mouse grab (QGraphicsItem only). 
            return QString("GrabMouse");
        case QEvent::GraphicsSceneContextMenu: // 159 Context popup menu over a graphics scene (QGraphicsSceneContextMenuEvent). 
            return QString("GraphicsSceneContextMenu");
        case QEvent::GraphicsSceneDragEnter: // 164 The cursor enters a graphics scene during a drag and drop operation. 
            return QString("GraphicsSceneDragEnter");
        case QEvent::GraphicsSceneDragLeave: // 166 The cursor leaves a graphics scene during a drag and drop operation. 
            return QString("GraphicsSceneDragLeave");
        case QEvent::GraphicsSceneDragMove: // 165 A drag and drop operation is in progress over a scene. 
            return QString("GraphicsSceneDragMove");
        case QEvent::GraphicsSceneDrop: // 167 A drag and drop operation is completed over a scene. 
            return QString("GraphicsSceneDrop");
        case QEvent::GraphicsSceneHelp: // 163 The user requests help for a graphics scene (QHelpEvent). 
            return QString("GraphicsSceneDrop");
        case QEvent::GraphicsSceneHoverEnter: // 160 The mouse cursor enters a hover item in a graphics scene (QGraphicsSceneHoverEvent). 
            return QString("GraphicsSceneDrop");
        case QEvent::GraphicsSceneHoverLeave: // 162 The mouse cursor leaves a hover item in a graphics scene (QGraphicsSceneHoverEvent). 
            return QString("GraphicsSceneHoverLeave");
        case QEvent::GraphicsSceneHoverMove: // 161 The mouse cursor moves inside a hover item in a graphics scene (QGraphicsSceneHoverEvent). 
            return QString("GraphicsSceneHoverMove");
        case QEvent::GraphicsSceneMouseDoubleClick: // 158 Mouse press again (double click) in a graphics scene (QGraphicsSceneMouseEvent). 
            return QString("GraphicsSceneMouseDoubleClick");
        case QEvent::GraphicsSceneMouseMove: // 155 Move mouse in a graphics scene (QGraphicsSceneMouseEvent). 
            return QString("GraphicsSceneMouseMove");
        case QEvent::GraphicsSceneMousePress: // 156 Mouse press in a graphics scene (QGraphicsSceneMouseEvent). 
            return QString("GraphicsSceneMousePress");
        case QEvent::GraphicsSceneMouseRelease : //157 Mouse release in a graphics scene (QGraphicsSceneMouseEvent). 
            return QString("GraphicsSceneMouseRelease");
        case QEvent::GraphicsSceneMove: // 182 Widget was moved (QGraphicsSceneMoveEvent). 
            return QString("GraphicsSceneMove");
        case QEvent::GraphicsSceneResize: // 181 Widget was resized (QGraphicsSceneResizeEvent). 
            return QString("GraphicsSceneResize");
        case QEvent::GraphicsSceneWheel: // 168 Mouse wheel rolled in a graphics scene (QGraphicsSceneWheelEvent). 
            return QString("GraphicsSceneWheel");
        case QEvent::Hide: // 18 Widget was hidden (QHideEvent). 
            return QString("Hide");
        case QEvent::HideToParent: // 27 A child widget has been hidden. 
            return QString("HideToParent");
        case QEvent::HoverEnter: // 127 The mouse cursor enters a hover widget (QHoverEvent). 
            return QString("HoverEnter");
        case QEvent::HoverLeave: // 128 The mouse cursor leaves a hover widget (QHoverEvent). 
            return QString("HoverLeave");
        case QEvent::HoverMove: // 129 The mouse cursor moves inside a hover widget (QHoverEvent). 
            return QString("HoverMove");
        case QEvent::IconDrag: // 96 The main icon of a window has been dragged away (QIconDragEvent). 
            return QString("IconDrag");
        case QEvent::IconTextChange: // 101 Widget's icon text has been changed. 
            return QString("IconTextChange");
        case QEvent::InputMethod: // 83 An input method is being used (QInputMethodEvent). 
            return QString("InputMethod");
        case QEvent::KeyPress: // 6 Key press (QKeyEvent). 
            return QString("KeyPress");
        case QEvent::KeyRelease: // 7 Key release (QKeyEvent). 
            return QString("KeyRelease");
        case QEvent::LanguageChange: // 89 The application translation changed. 
            return QString("LanguageChange");
        case QEvent::LayoutDirectionChange: // 90 The direction of layouts changed. 
            return QString("LayoutDirectionChange");
        case QEvent::LayoutRequest: // 76 Widget layout needs to be redone. 
            return QString("LayoutRequest");
        case QEvent::Leave: // 11 Mouse leaves widget's boundaries. 
            return QString("Leave");
        case QEvent::LeaveWhatsThisMode: // 125 Send to toplevel widgets when the application leaves "What's This?" mode. 
            return QString("LeaveWhatsThisMode");
        case QEvent::LocaleChange: // 88 The system locale has changed.     
            return QString("LocaleChange");
        case QEvent::NonClientAreaMouseButtonDblClick: // 176 A mouse double click occurred outside the client area. 
            return QString("NonClientAreaMouseButtonDblClick");
        case QEvent::NonClientAreaMouseButtonPress: // 174 A mouse button press occurred outside the client area. 
            return QString("NonClientAreaMouseButtonPress");
        case QEvent::NonClientAreaMouseButtonRelease: // 175 A mouse button release occurred outside the client area. 
            return QString("NonClientAreaMouseButtonRelease");
        case QEvent::NonClientAreaMouseMove: // 173 A mouse move occurred outside the client area. 
            return QString("NonClientAreaMouseMove");
        case QEvent::MacSizeChange: // 177 The user changed his widget sizes (Mac OS X only). 
            return QString("MacSizeChange");
        case QEvent::MenubarUpdated: // 153 The window's menu bar has been updated. 
            return QString("MenubarUpdated");
        case QEvent::MetaCall: // 43 An asynchronous method invocation via QMetaObject::invokeMethod(). 
            return QString("MetaCall");
        case QEvent::ModifiedChange: // 102 Widgets modification state has been changed. 
            return QString("ModifiedChange");
        case QEvent::MouseButtonDblClick: // 4 Mouse press again (QMouseEvent). 
            return QString("MouseButtonDblClick");
        case QEvent::MouseButtonPress: // Mouse press (QMouseEvent). 
            return QString("MouseButtonPress");
        case QEvent::MouseButtonRelease: // 3 Mouse release (QMouseEvent). 
            return QString("MouseButtonRelease");
        case QEvent::MouseMove: // 5 Mouse move (QMouseEvent).                        
            return QString("MouseMove");                         
        case QEvent::MouseTrackingChange: // 109 The mouse tracking state has changed. 
            return QString("MouseTrackingChange");
        case QEvent::Move: // 13 Widget's position changed (QMoveEvent). 
            return QString("Move");
        case QEvent::Paint: // 12 Screen update necessary (QPaintEvent). 
            return QString("Paint");
        case QEvent::PaletteChange: // 39 Palette of the widget changed. 
            return QString("PaletteChange");
        case QEvent::ParentAboutToChange: // 131 The widget parent is about to change. 
            return QString("ParentAboutToChange");
        case QEvent::ParentChange: // 21 The widget parent has changed. 
            return QString("ParentChange");
        case QEvent::Polish: // 75 The widget is polished. 
            return QString("Polish");
        case QEvent::PolishRequest: // 74 The widget should be polished. 
            return QString("PolishRequest");
        case QEvent::QueryWhatsThis: // 123 The widget should accept the event if it has "What's This?" help. 
            return QString("QueryWhatsThis");
        case QEvent::Resize: // 14 Widget's size changed (QResizeEvent). 
            return QString("Resize");
        case QEvent::Shortcut: // 117 Key press in child for shortcut key handling (QShortcutEvent). 
            return QString("Shortcut");
        case QEvent::ShortcutOverride: // 51 Key press in child, for overriding shortcut key handling (QKeyEvent). 
            return QString("ShortcutOverride");
        case QEvent::Show: // 17 Widget was shown on screen (QShowEvent). 
            return QString("Show");
        case QEvent::ShowToParent: // 26 A child widget has been shown. 
            return QString("ShowToParent");
        case QEvent::SockAct: // 50 Socket activated, used to implement QSocketNotifier. 
            return QString("SockAct");
        case QEvent::StatusTip: // 112 A status tip is requested (QStatusTipEvent). 
            return QString("StatusTip");
        case QEvent::StyleChange: // 100 Widget's style has been changed. 
            return QString("StyleChange");
        case QEvent::TabletMove: // 87 Wacom tablet move (QTabletEvent). 
            return QString("TabletMove");
        case QEvent::TabletPress: // 92 Wacom tablet press (QTabletEvent). 
            return QString("TabletPress");
        case QEvent::TabletRelease: // 93 Wacom tablet release (QTabletEvent). 
            return QString("TabletRelease");
        case QEvent::OkRequest: // 94 Ok button in decoration pressed. Supported only for Windows CE. 
            return QString("OkRequest");
        case QEvent::TabletEnterProximity: // 171 Wacom tablet enter proximity event (QTabletEvent), sent to QApplication. 
            return QString("TabletEnterProximity");
        case QEvent::TabletLeaveProximity: // 172 Wacom tablet leave proximity event (QTabletEvent), sent to QApplication. 
            return QString("TabletLeaveProximity");
        case QEvent::Timer: // 1 Regular timer events (QTimerEvent). 
            return QString("Timer");
        case QEvent::ToolBarChange: // 120 The toolbar button is toggled on Mac OS X. 
            return QString("ToolBarChange");
        case QEvent::ToolTip: // 110 A tooltip was requested (QHelpEvent). 
            return QString("ToolTip");
        case QEvent::ToolTipChange: // 184 The widget's tooltip has changed. 
            return QString("ToolTipChange");
        case QEvent::UngrabKeyboard: // 189 Item loses keyboard grab (QGraphicsItem only). 
            return QString("UngrabKeyboard");
        case QEvent::UngrabMouse: // 187 Item loses mouse grab (QGraphicsItem only). 
            return QString("UngrabMouse");
        case QEvent::UpdateLater: // 78 The widget should be queued to be repainted at a later time. 
            return QString("UpdateLater");
        case QEvent::UpdateRequest: // 77 The widget should be repainted. 
            return QString("UpdateRequest");
        case QEvent::WhatsThis: // 111 The widget should reveal "What's This?" help (QHelpEvent). 
            return QString("WhatsThis");
        case QEvent::WhatsThisClicked: // 118 A link in a widget's "What's This?" help was clicked. 
            return QString("WhatsThisClicked");
        case QEvent::Wheel: // 31 Mouse wheel rolled (QWheelEvent). 
            return QString("Wheel");
        case QEvent::WinEventAct: // 132 A Windowsspecific activation event has occurred. 
            return QString("WinEventAct");
        case QEvent::WindowActivate: // 24 Window was activated. 
            return QString("WindowActivate");
        case QEvent::WindowBlocked: // 103 The window is blocked by a modal dialog. 
            return QString("WindowBlocked");
        case QEvent::WindowDeactivate: // 25 Window was deactivated. 
            return QString("WindowDeactivate");
        case QEvent::WindowIconChange: // 34 The window's icon has changed. 
            return QString("WindowIconChange");
        case QEvent::WindowStateChange: // 105 The window's state (minimized, maximized or fullscreen) has changed (QWindowStateChangeEvent). 
            return QString("WindowStateChange");
        case QEvent::WindowTitleChange: // 33 The window title has changed. 
            return QString("WindowTitleChange");
        case QEvent::WindowUnblocked: // 104 The window is unblocked after a modal dialog exited. 
            return QString("WindowUnblocked");
        case QEvent::ZOrderChange: // 126 The widget's zorder has changed. This event is never sent to top level windows. 
            return QString("ZOrderChange");
        case QEvent::KeyboardLayoutChange: // 169 The keyboard layout has changed. 
            return QString("KeyboardLayoutChange");
        case QEvent::DynamicPropertyChange:
            return QString("DynamicPropertyChange");
        case QEvent::TouchBegin: //194            
            return QString("TouchBegin"); //+ touchEventDetails(static_cast<QTouchEvent*>(event));
        case QEvent::TouchUpdate: //195
            return QString("TouchUpdate"); //  + touchEventDetails(static_cast<QTouchEvent*>(event));
        case QEvent::TouchEnd: //196
            return QString("TouchEnd"); //  + touchEventDetails(static_cast<QTouchEvent*>(event));
        case QEvent::Gesture: //198
            return QString("Gesture");
        case QEvent::GestureOverride: //202
            return QString("GestureOverride");
        default:
            return QString::number(event->type());
    }
}

void TasCoreUtils::wait(int millis)
{
    Sleeper::sleep(millis);
}

QString TasCoreUtils::objectId(QObject* object)
{
    if(object){
        return QString::number((quintptr)object);
    }
    return "";
}

QString TasCoreUtils::pointerId(void* ptr)
{
    if(ptr){
        return QString::number((quintptr)ptr);
    }
    return "";
}

/*!
 * Start the server if not already running.
 * Server is only started in environments that support isServerRunning method
 * default implementation of the method will return true and server not being started.
 */
bool TasCoreUtils::startServer()
{
    bool started = false;
    if(!TasDeviceUtils::isServerRunning()){
        //must be in path
        started = QProcess::startDetached("qttasserver");
    }
    return started;
}

bool TasCoreUtils::autostart()
{
    QVariant value = TestabilitySettings::settings()->getValue(AUTO_START);
    if(value.isValid() && value.canConvert<QString>()){
        if(value.toString() == "on"){
            return true;
        }
    }
    return false;
}
