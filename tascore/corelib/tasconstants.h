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
 
 

#ifndef TASCONSTANTS_H
#define TASCONSTANTS_H

#include <QString>
    
/*!
  Services
 */
static const QString START_APPLICATION          = "startApplication";
static const QString CLOSE_APPLICATION          = "closeApplication";
static const QString UI_COMMAND                 = "uiCommand";
static const QString APPLICATION_STATE          = "uiState";
static const QString WEBKIT_COMMAND             = "webkitCommand";
static const QString SCREEN_SHOT                = "screenShot";
static const QString FIXTURE                    = "fixture";
static const QString REGISTER                   = "register";
static const QString SHELL_COMMAND              = "shellCommand";
static const QString KILL                       = "kill";
static const QString PLATFORM                   = "platformOperation";
static const QString COLLECT_EVENTS             = "collectEvents";
static const QString OBJECT_SERVICE             = "objectManipulation";
static const QString LIST_APPS                  = "listApps";
static const QString RECORD_EVENTS              = "recordEvents";
static const QString CONF_SERVICE               = "confService";
static const QString INFO_SERVICE               = "infoService";                                                  
static const QString LIST_CRASHED_APPS          = "listCrashedApps";
static const QString BRING_TO_FOREGROUND        = "bringToForeground";
static const QString SYSTEM_INFO_SERVICE        = "systemInfo";
static const QString RESOURCE_LOGGING_SERVICE   = "resourceLogging";

/*
 Message flags 
 */
static const quint8 ERROR_MSG = 0;
static const quint8 REQUEST_MSG = 1;
static const quint8 RESPONSE_MSG = 2;


/*
 Command codes
 */
static const quint8 OK_CODE = 0;
static const quint8 CHECKSUM_ERROR = 1;
static const quint8 INVALID_FLAG_ERROR = 2;
static const quint8 MESSAGE_READ_ERROR = 3;
static const quint8 COMMAND_UI_ERROR = 4;
static const quint8 UI_STATE_RESPONSE = 5;
static const quint8 CONNECTION_ERROR = 6;
static const quint8 INVALID_PLUGIN = 7;
static const quint8 ERROR_READING_RESPONSE = 8;
static const quint8 INVALID_RESPONSE = 9;

static const quint8 COMPRESSION_OFF = 1;
static const quint8 COMPRESSION_ON = 2;

/*
 Error Codes
 */
static const int TAS_ERROR_NONE =  0;
static const int TAS_ERROR_NOT_FOUND = -1;
static const int TAS_ERROR_PARAMETER = -2;
static const int TAS_ERROR_INVALID_STATE = -3;
static const int TAS_ERROR_NOT_IMPLEMENTED = -4;
static const int TAS_ERROR_CREATION_FAILED = -5;
static const int TAS_ERROR_FILE_ERROR = -6;

/*
  Message strings sent to the client.
 */
static const QString MESSAGE_CHECKSUM_ERROR_MSG = "Given checksum does not match the calculated one!";
static const QString INVALID_FLAG_ERROR_MSG = "The given flag is not supported!";
static const QString MESSAGE_READ_ERROR_MSG = "Unable to read message!";
static const QString UI_COMMAND_ERROR = "Unable to perform given ui commands!";
static const QString INVALID_PLUGIN_MSG = "No plugin found for the given id!";
static const QString PLUGIN_CONNECTION_FAILURE = "Connection to plugin failed.";
static const QString NOT_ABLE_TO_TAKE_SCREENSHOT = "Could not take UI screenshot (maybe no running applications or qttasserver in no gui mode).";
static const QString NO_COMMAND_TO_EXECUTE = "No command was given to be executed.";
static const QString OK_MESSAGE = "OK";
static const QString APP_CLOSE_FAILED_MSG = "Closing the application failed!";
static const QString PARSE_ERROR = "Could not parse the command xml.";

static const QString TRUE_MSG = "true";
static const QString FALSE_MSG = "false";

/*
  TasServer name used in QLocalServer   
 */
static const QString TAS_SERVER_NAME = "tas_server";

/*!
 Dynamic property name set to qApp to detect plugin existence.
 Used to avoid double plugin load.
 */
static const QString PLUGIN_ATTR = "testability_plugin";

/*
  Host names and ports
 */

static const QString SERVER_EXECUTABLE = "qttasserver";

static const QString QT_SERVER_NAME = "127.0.0.1";
static const int QT_SERVER_PORT = 55534;
static const int QT_SERVER_PORT_OUT = 55535;

static const QString LOCAL_SERVER_NAME = "qttasserver_pipe";

static const QString DATE_FORMAT = "yyyyMMddhhmmsszzz";

//header length
static const int HEADER_LENGTH = 12;


static const QString SIGNAL_KEY = "signal";

static const QString PROCESS_START_TIME = "ProcessStarted";

/*!
  Timer based system file paths  
 */
static const QString COMMAND_FILE= "C:\\TasCommands.xml";
static const QString UI_STATE_FILE = "C:\\QTDumpUi.xml";

/*!
  Object types. Graphics type object (GraphicsItem) need special 
  operations to enable event sending.
  Application view means application level (e.g screenshot)
*/
static const QString TYPE_GRAPHICS_VIEW = "Graphics";
static const QString TYPE_STANDARD_VIEW = "Standard";
static const QString TYPE_APPLICATION_VIEW = "Application";
static const QString TYPE_ACTION_VIEW = "Action";
static const QString TYPE_LAYOUT = "Layout";
static const QString TYPE_LAYOUT_ITEM = "LayoutItem";
static const QString TYPE_WEB = "Web";
static const QString TYPE_QWEBFRAME = "QWebFrame";

/*!
  Command Xml strings 
 */
static const QString COMMAND_TARGET = "Target";
static const QString COMMAND_ROOT = "TasCommands";
static const QString COMMAND_SERVICE = "service";
static const QString COMMAND_TYPE = "Command";
static const QString COMMAND_TYPE_NAME = "name";
static const QString COMMAND_TARGET_ID = "TasId";
static const QString APPLICATION_TARGET = "Application";
static const QString COMMAND_REGISTER = "registerPlugin";
static const QString COMMAND_UNREGISTER = "unRegisterPlugin";
static const QString PLUGIN_ID = "processId";
static const QString PLUGIN_NAME = "processName";
static const QString PLUGIN_TYPE = "pluginType";
static const QString TAS_PLUGIN = "tasPlugin";
static const QString LOG_MEM_SRV = "memLogSrv";
static const QString APP_UID = "applicationUid";
static const QString CONFIGURE_LOGGER_COMMAND = "configureLogger";
static const QString CONFIGURE_LOGEVENTS_COMMAND = "configureEventLogging";

static const QString CLEAR_LOG = "clearLog";
static const QString LOG_LEVEL = "logLevel";
static const QString LOG_TO_QDEBUG = "logToQDebug";
static const QString LOG_FOLDER = "logFolder";
static const QString LOG_FILE = "logFile";
static const QString LOG_QDEBUG = "logQDebug";
static const QString LOG_FILE_SIZE = "logSize";
static const QString LOG_ENABLE = "logEnabled";
static const QString LOG_EVENTS = "logEvents";

/*!
  webkit service commands
  */
static const QString COMMAND_EXEC_JS_ON_OBJ = "ExecuteJavaScriptOnWebElement";
static const QString COMMAND_EXEC_JS_ON_QWEBFRAME = "ExecuteJavaScriptOnQWebFrame";

//settings
static const QString BLACK_LISTED = "black_list";


#ifdef QTTASSERVER
 #define TAS_EXPORT 
#else
 #ifdef BUILD_TAS
  #define TAS_EXPORT Q_DECL_EXPORT
 #else
  #define TAS_EXPORT Q_DECL_IMPORT
 #endif
#endif


// no local sockets for 0.5.0 release

//#ifdef Q_OS_SYMBIAN
//#define TAS_NOLOCALSOCKET 
//#else
#define TAS_USELOCALSOCKET 
//#endif



#endif
