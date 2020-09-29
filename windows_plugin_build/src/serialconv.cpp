/**************************************************************************
*
*              Copyright (c) 2014-2020 by Wizapply.
*
*  This software is copyrighted by and is the sole property of Wizapply
*  All rights, title, ownership, or other interests in the software
*  remain the property of Wizapply.  This software may only be used
*  in accordance with the corresponding license agreement.
*  Any unauthorized use, duplication, transmission, distribution,
*  or disclosure of this software is expressly forbidden.
*
*  This Copyright notice may not be removed or modified without prior
*  written consent of Wizapply.
*
*  Wizpply reserves the right to modify this software without notice.
*
*  Wizapply                                info@wizapply.com
*  5F, KS Building,                        http://wizapply.com
*  3-7-10 Ichiokamotomachi, Minato-ku,
*  Osaka, 552-0002, Japan
*
***************************************************************************/

/**************************************************************************
*
*  Language is 'C++' code source
*
*  File Name : serialconv.cpp
*
***************************************************************************/

//Exporter


#pragma once
#define CSHARP_EXPORT __declspec(dllexport)
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <lmcons.h> // UNLEN
#include <direct.h> // _wmkdir
#include <shlobj.h>
#include <SDKDDKVer.h>
#include <WS2tcpip.h>
#include <cfgmgr32.h>

#include <stdio.h>
#include <setupapi.h>

//Standard lib
#include <iostream>
#include <vector>
#include <codecvt>
#include <locale>

#ifdef WIN32
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
#endif

std::string WStringToString(std::wstring oWString)
{
	// wstring -> SJIS
	int iBufferSize = WideCharToMultiByte(CP_OEMCP, 0, oWString.c_str()
		, -1, (char*)NULL, 0, NULL, NULL);

	CHAR* cpMultiByte = new CHAR[iBufferSize];
	WideCharToMultiByte(CP_OEMCP, 0, oWString.c_str(), -1, cpMultiByte
		, iBufferSize, NULL, NULL);

	std::string oRet(cpMultiByte, cpMultiByte + iBufferSize - 1);
	delete[] cpMultiByte;

	return(oRet);
}

//C language
#ifdef __cplusplus
extern "C" {
#endif

    int CSHARP_EXPORT scvGetSerialCOM(const char* vId, const char* pId, char* deviceInfo_buffer, int buffer_size)
	{
		std::wstring names = L"";
		int nameNum = 0;

		std::string venderId = "VID_" + std::string(vId);
		std::string productId = "PID_" + std::string(pId);

		venderId.erase(remove(venderId.begin(), venderId.end(), ' '), venderId.end());
		productId.erase(remove(productId.begin(), productId.end(), ' '), productId.end());

		HDEVINFO hDevInfo;
		SP_DEVINFO_DATA dData = { sizeof(SP_DEVINFO_DATA) };
		BYTE buffer[512];
		DWORD length = 0;

		hDevInfo = SetupDiGetClassDevs(NULL, 0, 0, DIGCF_PRESENT | DIGCF_ALLCLASSES); // デバイス情報セットを取得
		if (hDevInfo == 0) //      デバイス情報セットが取得できなかった場合
			return false;
		dData.cbSize = sizeof(dData);

		int max = 0;
		while (SetupDiEnumDeviceInfo(hDevInfo, max, &dData)) {  // デバイスインターフェイスの取得
			SetupDiGetDeviceRegistryProperty(hDevInfo, &dData, SPDRP_CLASS, NULL, buffer, sizeof(buffer), &length);
			// COMを検出
			if (wcsstr((wchar_t*)buffer, L"Ports") > 0) {
				HKEY key = SetupDiOpenDevRegKey(hDevInfo, &dData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_QUERY_VALUE);
				if (key) {
					TCHAR name[256];
					CHAR name_search[256];
					DWORD type = 0;
					DWORD comsize = sizeof(name);
					DWORD ssize = sizeof(name_search);
					RegQueryValueEx(key, L"PortName", NULL, &type, (LPBYTE)name, &comsize);
					SetupDiGetDeviceInstanceIdA(hDevInfo, &dData, (char*)name_search, sizeof(name_search), &ssize);	//ascii

					// PCI USB BTH not self
					bool seart = true;
					if (name_search[0] == 'U' && name_search[1] == 'S' && name_search[2] == 'B' && name_search[3] == '\\') {
						seart = false;
					}
					if (name_search[0] == 'P' && name_search[1] == 'C' && name_search[2] == 'I' && name_search[3] == '\\') {
						seart = false;
					}
					if (name_search[0] == 'B' && name_search[1] == 'T' && name_search[2] == 'H' && name_search[3] == '\\') {
						seart = false;
					}
					if (name_search[0] == 'B' && name_search[1] == 'T' && name_search[2] == 'M' && name_search[3] == 'U' && name_search[4] == 'S' && name_search[5] == 'B' && name_search[6] == '\\') {
						seart = false;
					}
					if (name_search[0] == '{') {
						seart = false;	//bluetooth
					}

					if (seart) {
						DEVINST devInst;	//Parent
						CONFIGRET cRet = CM_Get_Parent(&devInst, dData.DevInst, 0);
						cRet = CM_Get_Device_IDA(devInst, name_search, sizeof(name_search), 0);
					}

					if (strstr(name_search, venderId.c_str()) != NULL || venderId.empty()) {
						if (strstr(name_search, productId.c_str()) != NULL || productId.empty()) {
							names += std::wstring(name) + L";";
							++nameNum;
						}
					}
				}

				if(key != 0)
					RegCloseKey(key);
			}
			++max;
		}

		SetupDiDestroyDeviceInfoList(hDevInfo); // デバイス情報セットを解放

		const std::string names_buffer =  WStringToString(names);
		int i;
		for (i = 0; i < (int)names_buffer.size() && i < (buffer_size - 1); ++i)
			deviceInfo_buffer[i] = names_buffer.c_str()[i];
		deviceInfo_buffer[i] = '\0';

		return nameNum;
	}
	

#ifdef __cplusplus
}
#endif
