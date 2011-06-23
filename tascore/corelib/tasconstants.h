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

#include <QtGlobal>

/*!
  Services
 */
const char* const START_APPLICATION          = "startApplication";
const char* const CLOSE_APPLICATION          = "closeApplication";
const char* const UI_COMMAND                 = "uiCommand";
const char* const APPLICATION_STATE          = "uiState";
const char* const CHANGE_ORIENTATION         = "newOrientation";
const char* const WEBKIT_COMMAND             = "webkitCommand";
const char* const SCREEN_SHOT                = "screenShot";
const char* const FIXTURE                    = "fixture";
const char* const REGISTER                   = "register";
const char* const SHELL_COMMAND              = "shellCommand";
const char* const KILL                       = "kill";
const char* const PLATFORM                   = "platformOperation";
const char* const COLLECT_EVENTS             = "collectEvents";
const char* const OBJECT_SERVICE             = "objectManipulation";
const char* const LIST_APPS                  = "listApps";
const char* const STARTED_APPS               = "startedApps";
const char* const RECORD_EVENTS              = "recordEvents";
const char* const CONF_SERVICE               = "confService";
const char* const INFO_SERVICE               = "infoService";
const char* const LIST_CRASHED_APPS          = "listCrashedApps";
const char* const BRING_TO_FOREGROUND        = "bringToForeground";
const char* const SYSTEM_INFO_SERVICE        = "systemInfo";
const char* const RESOURCE_LOGGING_SERVICE   = "resourceLogging";
const char* const FIND_OBJECT_SERVICE        = "findObject";
const char* const CPU_LOAD_SERVICE           = "cpuLoad";
const char* const VERSION_SERVICE            = "versionService";
const char* const ENABLE_TIMESTAMPS          = "enableTimestamps";

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
const char* const MESSAGE_CHECKSUM_ERROR_MSG = "Given checksum does not match the calculated one!";
const char* const INVALID_FLAG_ERROR_MSG = "The given flag is not supported!";
const char* const MESSAGE_READ_ERROR_MSG = "Unable to read message!";
const char* const UI_COMMAND_ERROR = "Unable to perform given ui commands!";
const char* const INVALID_PLUGIN_MSG = "No plugin found for the given id!";
const char* const PLUGIN_CONNECTION_FAILURE = "Connection to plugin failed.";
const char* const NOT_ABLE_TO_TAKE_SCREENSHOT = "Could not take UI screenshot (maybe no running applications or qttasserver in no gui mode).";
const char* const NO_COMMAND_TO_EXECUTE = "No command was given to be executed.";
const char* const OK_MESSAGE = "OK";
const char* const APP_CLOSE_FAILED_MSG = "Closing the application failed!";
const char* const PARSE_ERROR = "Could not parse the command xml.";

const char* const TRUE_MSG = "true";
const char* const FALSE_MSG = "false";

/*
  TasServer name used in QLocalServer
 */
const char* const TAS_SERVER_NAME = "tas_server";

/*!
 Dynamic property name set to qApp to detect plugin existence.
 Used to avoid double plugin load.
 */
const char* const PLUGIN_ATTR = "testability_plugin";

/*
  Host names and ports
 */

const char* const SERVER_EXECUTABLE = "qttasserver";
const char* const QT_SERVER_NAME = "127.0.0.1";
static const int QT_SERVER_PORT = 55534;
static const int QT_SERVER_PORT_OUT = 55535;
static const int QT_CUCUMBER_SERVER_OUT = 55536;

#ifdef TAS_MAEMO
// Duo to bug in security system explicit declaration. Remove if works.
const char* const LOCAL_SERVER_NAME = "/var/tmp/qttasserver_pipe";
#else
const char* const LOCAL_SERVER_NAME = "qttasserver_pipe";
#endif


const char* const DATE_FORMAT = "yyyyMMddhhmmsszzz";

//header length
static const int HEADER_LENGTH = 12;


const char* const SIGNAL_KEY = "signal";

const char* const PROCESS_START_TIME = "ProcessStarted";

/*!
  Timer based system file paths
 */
const char* const COMMAND_FILE= "C:\\TasCommands.xml";
const char* const UI_STATE_FILE = "C:\\QTDumpUi.xml";

/*!
  Object types. Graphics type object (GraphicsItem) need special
  operations to enable event sending.
  Application view means application level (e.g screenshot)
*/
const char* const TYPE_GRAPHICS_VIEW = "Graphics";
const char* const TYPE_STANDARD_VIEW = "Standard";
const char* const TYPE_APPLICATION_VIEW = "Application";
const char* const TYPE_ACTION_VIEW = "Action";
const char* const TYPE_LAYOUT = "Layout";
const char* const TYPE_LAYOUT_ITEM = "LayoutItem";
const char* const TYPE_WEB = "Web";
const char* const TYPE_QWEB = "QWeb";
/*!
  Command Xml strings
 */
const char* const COMMAND_TARGET = "Target";
const char* const COMMAND_ROOT = "TasCommands";
const char* const COMMAND_SERVICE = "service";
const char* const COMMAND_TYPE = "Command";
const char* const COMMAND_TYPE_NAME = "name";
const char* const COMMAND_TARGET_ID = "TasId";
const char* const APPLICATION_TARGET = "Application";
const char* const COMMAND_REGISTER = "registerPlugin";
const char* const COMMAND_UNREGISTER = "unRegisterPlugin";
const char* const PLUGIN_ID = "processId";
const char* const PLUGIN_NAME = "processName";
const char* const PLUGIN_TYPE = "pluginType";
const char* const TAS_PLUGIN = "tasPlugin";
const char* const LOG_MEM_SRV = "memLogSrv";
const char* const APP_UID = "applicationUid";
const char* const CONFIGURE_LOGGER_COMMAND = "configureLogger";
const char* const CONFIGURE_LOGEVENTS_COMMAND = "configureEventLogging";

const char* const CLEAR_LOG = "clearLog";
const char* const LOG_LEVEL = "logLevel";
const char* const LOG_TO_QDEBUG = "logToQDebug";
const char* const LOG_FOLDER = "logFolder";
const char* const LOG_FILE = "logFile";
const char* const LOG_QDEBUG = "logQDebug";
const char* const LOG_FILE_SIZE = "logSize";
const char* const LOG_ENABLE = "logEnabled";
const char* const LOG_EVENTS = "logEvents";

/*!
  webkit service commands
  */
const char* const COMMAND_EXEC_JS_ON_OBJ = "ExecuteJavaScriptOnWebElement";
const char* const COMMAND_EXEC_JS_ON_QWEBFRAME = "ExecuteJavaScriptOnQWebFrame";
const char* const COMMAND_EXEC_SCROLL_QWEBFRAME = "ScrollQWebFrame";

//settings
const char* const BLACK_LISTED = "black_list";
const char* const AUTO_START = "autostart";
const char* const VISIBILITY_CHECK = "visibility_check";
const char* const VISIBILITY_BLACKLIST = "visibility_blacklist";


const char* const PENINPUT_SERVER = "peninputserver";
const char* const VKB_IDENTIFIER = "vkb_app";

#ifdef TDTASSERVER
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

#define TAS_USE_CUCUMBER_WIRE_PROTOCOL


#endif
