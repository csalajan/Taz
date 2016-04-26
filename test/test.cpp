#include <iostream>
#include "stdafx.h"
#include "table_printer.h"
#include "windows.h"
#include "comdef.h"
#include <string>
#include <Wbemidl.h>
#include <sstream>

#pragma comment(lib, "wbemuuid.lib")

std::string GetRamUsage();
std::string GetCpuUsage();
std::string GetRamTemp();
std::string GetCpuTemp();
std::string ConvertToString(LONG num);
std::string ConvertToString(float num);
std::string ConvertToString(double num);

void buildTable();

HRESULT GetCpuTemperature(LPLONG pTemperature);

static float CalculateCPULoad(unsigned long long idleTicks, unsigned long long totalTicks);
static unsigned long long FileTimeToInt64(const FILETIME & ft);
float GetCPULoad();

using bprinter::TablePrinter;
int main()
{
	while (true) {
		system("cls");
		buildTable();
		Sleep(250);
	}
    
	
	return 1;
}

void buildTable() {
	TablePrinter table(&std::cout);

	table.AddColumn("Hardware", 10);
	table.AddColumn("Usage", 7);
	table.AddColumn("Temp", 7);

	table.PrintHeader();
	table << "RAM" << GetRamUsage() << GetRamTemp();
	table << "CPU" << GetCpuUsage() << GetCpuTemp();

	table.PrintFooter();
}

std::string GetRamUsage() {
	MEMORYSTATUSEX memStat;
	memStat.dwLength = sizeof(memStat);
	GlobalMemoryStatusEx(&memStat);

	LONG memUsage = memStat.dwMemoryLoad;
	return ConvertToString(memUsage) + "%";
}

std::string GetCpuUsage() {
	
	return ConvertToString(GetCPULoad()) + "%";
}

std::string GetRamTemp() {
	return "60\370";
}

std::string GetCpuTemp() {
	LONG temp;
	GetCpuTemperature(&temp);
	return ConvertToString((temp - 273.15)) + "\370";
	
}

std::string ConvertToString(double num) {
	std::stringstream ss;
	ss << num;
	return ss.str();
}

std::string ConvertToString(float num) {
	std::stringstream ss;
	ss << num;
	return ss.str();
}

std::string ConvertToString(LONG num) {
	std::stringstream ss;
	ss << num;
	return ss.str();
}

static float CalculateCPULoad(unsigned long long idleTicks, unsigned long long totalTicks)
{
	static unsigned long long _previousTotalTicks = 0;
	static unsigned long long _previousIdleTicks = 0;

	unsigned long long totalTicksSinceLastTime = totalTicks - _previousTotalTicks;
	unsigned long long idleTicksSinceLastTime = idleTicks - _previousIdleTicks;

	float ret = 1.0f - ((totalTicksSinceLastTime > 0) ? ((float)idleTicksSinceLastTime) / totalTicksSinceLastTime : 0);

	_previousTotalTicks = totalTicks;
	_previousIdleTicks = idleTicks;
	return ret;
}

static unsigned long long FileTimeToInt64(const FILETIME & ft) { return (((unsigned long long)(ft.dwHighDateTime)) << 32) | ((unsigned long long)ft.dwLowDateTime); }


float GetCPULoad()
{
	FILETIME idleTime, kernelTime, userTime;
	float cpuLoad = GetSystemTimes(&idleTime, &kernelTime, &userTime) ? CalculateCPULoad(FileTimeToInt64(idleTime), FileTimeToInt64(kernelTime) + FileTimeToInt64(userTime)) : -1.0f;
	return floor(cpuLoad * 100);
}

HRESULT GetCpuTemperature(LPLONG pTemperature) {
	if (pTemperature == NULL)
		return E_INVALIDARG;

	*pTemperature = -1;
	HRESULT ci = CoInitialize(NULL);
	HRESULT hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_NONE, RPC_C_IMP_LEVEL_DELEGATE, NULL, EOAC_NONE, NULL);
	if (SUCCEEDED(hr))
	{
		IWbemLocator *pLocator;
		hr = CoCreateInstance(CLSID_WbemAdministrativeLocator, NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLocator);
		if (SUCCEEDED(hr))
		{
			IWbemServices *pServices;
			BSTR ns = SysAllocString(L"root\\CIMV2");
			hr = pLocator->ConnectServer(ns, NULL, NULL, NULL, 0, NULL, NULL, &pServices);
			pLocator->Release();
			SysFreeString(ns);
			if (SUCCEEDED(hr))
			{
				BSTR query = bstr_t(L"SELECT * FROM Win32_PerfFormattedData_Counters_ThermalZoneInformation");
				BSTR wql = bstr_t(L"WQL");
				IEnumWbemClassObject *pEnum;
				hr = pServices->ExecQuery(wql, query, WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY, NULL, &pEnum);
				SysFreeString(wql);
				SysFreeString(query);
				pServices->Release();
				if (SUCCEEDED(hr))
				{
					IWbemClassObject *pObject;
					ULONG returned;
					hr = pEnum->Next(WBEM_INFINITE, 1, &pObject, &returned);
					pEnum->Release();
					if (SUCCEEDED(hr))
					{
						BSTR temp = SysAllocString(L"Temperature");
						VARIANT v;
						VariantInit(&v);
						hr = pObject->Get(temp, 0, &v, NULL, NULL);
						pObject->Release();
						SysFreeString(temp);
						if (SUCCEEDED(hr))
						{
							*pTemperature = V_I4(&v);
						}
						VariantClear(&v);
					}
				}
			}
			if (ci == S_OK)
			{
				CoUninitialize();
			}
		}
	}
	return hr;
}

