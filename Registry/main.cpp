#include "Registry.h"
#include <tchar.h>
#include <cstdio>
#include <cstdlib>

int main(int argc, char *argv[])
try
{
    RegistryKey& key = Registry::LocalMachine;
    RegistryKey& software = key.OpenSubKey(_T("SOFTWARE\\NETCA\\PKI\\Devices\\NETCAKeyMobileKey"), false);
    StringType val = software.GetValue(_T("UserName"), _T(""));
    StringArray valueNames = software.GetValueNames();
    StringArray keyNames = software.GetSubKeyNames();
    software.CreateSubKey(_T("A\\B\\B1"));
    software.CreateSubKey(_T("A\\B\\B2"));
    software.CreateSubKey(_T("A\\B\\B3"));
    software.CreateSubKey(_T("A\\B\\B4"));
    software.CreateSubKey(_T("A\\B\\C"));
    software.CreateSubKey(_T("A\\B\\C\\C1"));
    software.CreateSubKey(_T("A\\B\\C\\C2"));
    software.CreateSubKey(_T("A\\B\\C\\C3"));
    software.CreateSubKey(_T("A\\B\\C\\C4"));
    software.CreateSubKey(_T("A\\B\\C\\D"));
    software.CreateSubKey(_T("A\\B\\C\\D\\D1"));
    software.CreateSubKey(_T("A\\B\\C\\D\\D2"));
    software.CreateSubKey(_T("A\\B\\C\\D\\D3"));
    software.CreateSubKey(_T("A\\B\\C\\D\\D4"));
    software.DeleteSubKeyTree(_T("A\\"));
    return 0;
}
catch (const RegistryException& ex)
{
    _tprintf(ex.What().c_str());
    return -1;
}