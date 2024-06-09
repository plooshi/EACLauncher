#pragma once
#pragma section(".eac")
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <tlhelp32.h>
#include <psapi.h>
#include <functional>
using namespace std;

HANDLE launchProcess(filesystem::path& path, wstring cmd) {
	PROCESS_INFORMATION pi;
	STARTUPINFO si = { sizeof(si) };
	CreateProcessW(path.c_str(), (wchar_t*)(L"\"" + path.wstring() + L"\" " + cmd).c_str(), NULL, NULL, true, CREATE_NO_WINDOW, nullptr, path.parent_path().c_str(), &si, &pi);
	CloseHandle(pi.hThread);
	return pi.hProcess;
}

string getKey(vector<char>& buffer, string key) {
	auto keypos = search(buffer.begin(), buffer.end(), key.begin(), key.end());
	auto cPos = find(keypos, buffer.end(), ':');
	auto sPos = find(cPos, buffer.end(), '"');
	auto vStart = sPos + 1;
	auto vEnd = find(vStart, buffer.end(), '"');
	vector<char> val(vStart, vEnd);
	return val.data();
}

bool internalWriteFileIfNotExists(filesystem::path& path, const unsigned char* data, size_t size) {
	if (!filesystem::exists(path)) {
		ofstream set(path, ios::binary);
		set.write((const char*)data, size);
		set.close();
		return true;
	}
	return false;
}

template<size_t S>
bool writeFileIfNotExists(filesystem::path& path, const unsigned char(&data)[S]) {
	return internalWriteFileIfNotExists(path, data, S);
}

bool findProcessWithCheck(function<bool(PROCESSENTRY32& pe)> f) {
	bool exists = false;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	bool found = false;
	if (Process32First(snapshot, &entry))
		while (Process32Next(snapshot, &entry)) {
			if (f(entry)) {
				found = true;
				break;
			}
		}

	CloseHandle(snapshot);
	return found;
}

vector<char> readFile(filesystem::path& path) {
	ifstream set(path, ios::binary);
	set.seekg(0, ios::end);
	auto size = set.tellg();
	set.seekg(0, ios::beg);
	vector<char> buffer(size);
	set.read(buffer.data(), size);
	set.close();
	return buffer;
}
