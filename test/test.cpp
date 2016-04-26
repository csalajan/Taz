// test.cpp : Defines the entry point for the console application.
//
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

void buildTable();

HRESULT GetCpuTemperature(LPLONG pTemperature);

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
	return "5%";
}

std::string GetCpuUsage() {
	return "10%";
}

std::string GetRamTemp() {
	return "60\370";
}

std::string GetCpuTemp() {
	LONG temp;
	GetCpuTemperature(&temp);
	return ConvertToString(temp) + "\370";
}

std::string ConvertToString(LONG num) {
	std::stringstream ss;
	ss << num;
	return ss.str();
}


HRESULT GetCpuTemperature(LPLONG pTemperature) {
	if (pTemperature == NULL)
		return E_INVALIDARG;

	*pTemperature = -1;
	HRESULT ci = CoInitialize(NULL);
	HRESULT hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
	if (SUCCEEDED(hr))
	{
		IWbemLocator *pLocator;
		hr = CoCreateInstance(CLSID_WbemAdministrativeLocator, NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLocator);
		if (SUCCEEDED(hr))
		{
			IWbemServices *pServices;
			BSTR ns = SysAllocString(L"root\\WMI");
			hr = pLocator->ConnectServer(ns, NULL, NULL, NULL, 0, NULL, NULL, &pServices);
			pLocator->Release();
			SysFreeString(ns);
			if (SUCCEEDED(hr))
			{
				BSTR query = SysAllocString(L"SELECT * FROM CIM_TemperatureSensor");
				BSTR wql = SysAllocString(L"WQL");
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
						BSTR temp = SysAllocString(L"CurrentReading");
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

