//Stellaris Mod Tracker
//#include "stdafx.h"
 
#define _MODURL       (L"http://steamcommunity.com/sharedfiles/filedetails/?id=")
 
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <tchar.h>
#include <thread>
 
using namespace std;
 
 
wstring cloudPath;
wstring localPath;
bool skipAutoSaves = true;
bool recordModIDs = true;

void recordMods(wstring newFile) {
	if (newFile.find(L".sav") != string::npos && (newFile.find(L"autosave") == string::npos || !skipAutoSaves)) {
		
		wstring outPath = newFile.substr(0, newFile.find_last_of(L"\\")) + L"\\modlist.txt";
		wifstream inFile;
		wofstream outFile;
		wstring lineIn;
		vector<wstring> modIdList;

		inFile.open(localPath + L"\\settings.txt", ifstream::in);
		outFile.open(outPath, ofstream::app);
		if (inFile.is_open()) {
			outFile << newFile.substr(newFile.find_last_of(L"\\") + 1, newFile.length() - newFile.find_last_of(L"\\")) << L"\n\n";
			while (getline(inFile, lineIn)) {
				if (lineIn.find(L"\"mod") != string::npos) {
					if (recordModIDs){
						outFile << lineIn << '\n';
						outFile.flush();
					}
					lineIn = lineIn.substr(lineIn.find(L"_") + 1, lineIn.find(L".") - lineIn.find(L"_") - 1);
					modIdList.push_back(lineIn);
				}
			}
			if (recordModIDs){
				outFile << L"\n\n";
			}
			
			inFile.close();
			vector<wstring> modNameList;

			unsigned int longestModName = 0;
			for (unsigned int i = 0; i < modIdList.size(); ++i) {
				inFile.open(localPath + L"\\mod\\ugc_" + modIdList[i] + L".mod", ifstream::in);
				if (inFile.is_open()) {
					getline(inFile, lineIn);
					lineIn = lineIn.substr(lineIn.find(L"\"") + 1, lineIn.find_last_of(L"\"") - lineIn.find(L"\"") - 1);
					if (lineIn.length() > longestModName){
						longestModName = lineIn.length();
					}
					modNameList.push_back(lineIn);
					inFile.close();
				}
			}
			
			for (unsigned int i = 0; i < modNameList.size(); ++i) {
				wstring pad(longestModName - modNameList[i].length() + 5, '.');
				modNameList[i] += (pad + _MODURL + modIdList[i]);
			}

			sort(modNameList.begin(), modNameList.end());

			for (unsigned int i = 0; i < modNameList.size(); ++i) {
				outFile << modNameList[i] << '\n';
				outFile.flush();
			}
			outFile << L"\n__________________________________________________________________________________________________________________________________________________________________________________________\n\n\n\n";
			outFile.flush();
			outFile.close();
		}
	}
}

int checkForNewFiles(wchar_t * Dir) {
	HANDLE DirectoryHandle = CreateFileW(Dir, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	OVERLAPPED Overlapped = {};
	Overlapped.hEvent = CreateEvent(NULL, NULL, NULL, NULL);
	FILE_NOTIFY_INFORMATION NotifyInformation[1024];

	while (true) {
		ZeroMemory(NotifyInformation, 1024);
		DWORD BytesReturned = 0;

		DWORD FileNotifyAttributes = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SECURITY;

		ReadDirectoryChangesW(DirectoryHandle, NotifyInformation, sizeof(FILE_NOTIFY_INFORMATION) * 1024, TRUE, FileNotifyAttributes, &BytesReturned, &Overlapped, NULL);
		GetOverlappedResult(DirectoryHandle, &Overlapped, &BytesReturned, FALSE);

		if (NotifyInformation[0].Action == FILE_ACTION_ADDED) {
				wstring newFile(Dir);
				newFile += L"\\";
				newFile += NotifyInformation[0].FileName;
				recordMods(newFile);
		}
		Sleep(100);
	}
	return 0;
}

int main() {
 
    wchar_t cloud[256];
    wchar_t local[256];
	wchar_t settings[256];

    GetPrivateProfileStringW(L"Paths", L"LocalPath", L"", local, 256, L".\\ModTrackerConfig.ini");
    GetPrivateProfileStringW(L"Paths", L"CloudPath", L"", cloud, 256, L".\\ModTrackerConfig.ini");
	GetPrivateProfileStringW(L"Settings", L"Ignore Autosaves", L"true", settings, 256, L".\\ModTrackerConfig.ini");
	wstring temp = settings;
	
	if (temp.find(L"true") != string:: npos)	skipAutoSaves = true;
	else	skipAutoSaves = false;
	
	ZeroMemory(settings, 256);
	GetPrivateProfileStringW(L"Settings", L"Record Mod IDs", L"true", settings, 256, L".\\ModTrackerConfig.ini");
	temp = settings;
	
	if (temp.find(L"true") != string:: npos)	recordModIDs = true;
	else	recordModIDs = false;
	
    localPath = local;
    localPath = localPath.substr(0, localPath.find_last_of(L"\\"));
    cloudPath = cloud;
	
    //ShowWindow(GetConsoleWindow(), SW_HIDE);
	thread checkCloud(checkForNewFiles, cloud);
    checkForNewFiles(local);
    return 0;
}