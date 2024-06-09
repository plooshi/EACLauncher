#include "utils.h"
#include "eacStrapper.h"
#include "eacSetup.h"

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd) {
	wchar_t path[MAX_PATH];
	GetModuleFileNameW(0, path, MAX_PATH);
	filesystem::path gamePath = filesystem::path(path).parent_path();
	auto settings = gamePath / L"EasyAntiCheat\\Settings.json";
	if (filesystem::exists(settings)) {
		auto buffer = readFile(settings);
		auto pid2 = getKey(buffer, "productid");
		wchar_t pid[33] = { 0 };
		transform(pid2.begin(), pid2.end(), pid, [](char c) { return c; });
		pid[32] = 0;
		auto EACb = gamePath / L"EACStrapper.exe";
		if (!writeFileIfNotExists(EACb, Strapper)) {
			if (findProcessWithCheck([EACb](PROCESSENTRY32& entry) { 
				if (!wcscmp(entry.szExeFile, L"EACStrapper.exe")) {
					auto proc = OpenProcess(PROCESS_ALL_ACCESS, false, entry.th32ProcessID);
					wchar_t path[MAX_PATH];
					GetModuleFileNameExW(proc, 0, path, MAX_PATH);
					CloseHandle(proc);
					return EACb.wstring() == path;
				}
				return false; 
			})) return 0;
		}

		auto EACI = filesystem::temp_directory_path() / L"EACSetup.exe";
		writeFileIfNotExists(EACI, Setup);
		auto EACSetup = launchProcess(EACI, L"verify-and-install " + wstring(pid));
		WaitForSingleObject(EACSetup, -1);
		DWORD exitCode = -1;
		GetExitCodeProcess(EACSetup, &exitCode);
		CloseHandle(EACSetup);


		if (!exitCode) CloseHandle(launchProcess(EACb, lpCmdLine));
	}
	return 0;
}