// Copyright 2021 TASK Aviation, LLC
// Author Jason Vincent.
// All rights reserved.
// Use of this software is not supported. Use at your own risk.
// Questions? Contact avidyne_plugin@taskaviation.org
// Xplane is Owned by Laminar Research.

// This XPlane11 plugin is simply one that launches an executable either by the menu, or by configured dataref status is set to 1. 
// The 

#include "XPLMPlugin.h"
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"
#include "XPLMUtilities.h"
#include <string.h>
#include <string>

using namespace std;

#if IBM
	#include <windows.h>
#endif

#ifndef XPLM300
	#error This is made to be compiled against the XPLM300 SDK
#endif

int g_menu_container_idx;
STARTUPINFO si;
PROCESS_INFORMATION pi;
LPSTR command;

XPLMMenuID g_menu_id;
//XPLMDataRef avionicsDataRef;
XPLMCommandRef onCommand;
XPLMCommandRef offCommand;

void menu_handler(void *, void *);
int datarefSwitchHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void* inRefcon);
int offSwitchHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void* inRefcon);

PLUGIN_API int XPluginStart(
						char *		outName,
						char *		outSig,
						char *		outDesc)
{
	strcpy(outName, "IFD Launcher Plugin");
	strcpy(outSig, "org.taskaviation.xplane.launcherplugin");
	strcpy(outDesc, "Plugin that executes configured executable in background when cmd is executed.");

	char path[512];
	memset(path, 0x00, 512);

	XPLMGetPluginInfo(XPLMGetMyID(), NULL, path, NULL, NULL);
	string cmd = "\"";
	cmd = cmd.append(path)
		.append(XPLMGetDirectorySeparator())
		.append("..")
		.append(XPLMGetDirectorySeparator())
		.append("..")
		.append(XPLMGetDirectorySeparator())
		.append("AviXplaneInterface.exe\"");
	command = _strdup(cmd.c_str());

	onCommand = XPLMFindCommand("sim/systems/avionics_on");
	offCommand = XPLMFindCommand("sim/systems/avionics_off");
	XPLMRegisterCommandHandler(onCommand, datarefSwitchHandler, 1, (void*)0);
	XPLMRegisterCommandHandler(offCommand, offSwitchHandler, 1, (void*)0);
	
	//avionicsDataRef = XPLMFindDataRef("sim/cockpit/electrical/avionics_on");

	g_menu_container_idx = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "Executable Launcher", 0, 0);
	g_menu_id = XPLMCreateMenu("Executable Launcher", XPLMFindPluginsMenu(), g_menu_container_idx, menu_handler, NULL);
	XPLMAppendMenuItem(g_menu_id, "Start Connector", (void *)"connect", 1);

	return 1;
}

PLUGIN_API void	XPluginStop(void)
{
	XPLMDestroyMenu(g_menu_id);

	TerminateProcess(pi.hProcess, 0);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	XPLMUnregisterCommandHandler(onCommand, datarefSwitchHandler, 1, (void*)0);
	XPLMUnregisterCommandHandler(offCommand, offSwitchHandler, 1, (void*)0);
}

PLUGIN_API void XPluginDisable(void) { }
PLUGIN_API int XPluginEnable(void) { return 1; }
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void * inParam) { }


void terminateProcess() {
	TerminateProcess(pi.hProcess, 0);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

int datarefSwitchHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void* inRefcon) {
	if (!CreateProcess(NULL, command, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
		printf("Plugin CreateProcess failed (%d).\n", GetLastError());
		terminateProcess();
	}
	return 1;
	
	
//	int status = XPLMGetDatai(avionicsDataRef);
//	if (status == 1) {
//		if (!CreateProcess(NULL, command, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
//			printf("Plugin CreateProcess failed (%d).\n", GetLastError());
//			terminateProcess();
//		}
//	} else {
//		terminateProcess();
//	}
//	return 1;
}

int offSwitchHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void* inRefcon) {
	terminateProcess();
	return 1;
}

void menu_handler(void * in_menu_ref, void * in_item_ref)
{
//    if (!strcmp((const char*)in_item_ref, "Restart Process"))
//	{
		terminateProcess();

		if (!CreateProcess(NULL, command, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
			printf("Plugin CreateProcess failed (%d).\n", GetLastError());
			terminateProcess();
			return;
		}
//	}
}
