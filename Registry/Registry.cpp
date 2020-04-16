#include "Registry.h"
#include <Windows.h>

#ifndef _T
#ifdef UNICODE
#define _T(x) L ## x
#else
#define _T(x) x
#endif // UNICODE
#endif // !_T(x)

#ifdef _WIN64
#define IS_X86 0
#else
#define IS_X86 1
#endif // _WIN32

#define DECLARE_REGISTRY_KEY(member, key) RegistryKey Registry::member((LONG)key, _T(#key))

Registry::RegistryKeyMap Registry::m_usingKeys;

DECLARE_REGISTRY_KEY(ClassesRoot, HKEY_CLASSES_ROOT);
DECLARE_REGISTRY_KEY(CurrentUser, HKEY_CURRENT_USER);
DECLARE_REGISTRY_KEY(LocalMachine, HKEY_LOCAL_MACHINE);
DECLARE_REGISTRY_KEY(User, HKEY_USERS);
DECLARE_REGISTRY_KEY(CurrentConfig, HKEY_CURRENT_CONFIG);
DECLARE_REGISTRY_KEY(DynData, HKEY_DYN_DATA);
DECLARE_REGISTRY_KEY(PerformanceData, HKEY_PERFORMANCE_DATA);


StringType GetKeyNameOfPath(const StringType& path);

LONG _OpenKey(
	__in HKEY hKey,
	__in_opt LPCTSTR lpSubKey,
	__reserved DWORD ulOptions,
	__in REGSAM samDesired,
	__out PHKEY phkResult,
	__in BOOL bIsWin32Key = IS_X86)
{
	if (bIsWin32Key)
	{
		return ::RegOpenKeyEx(hKey, lpSubKey, ulOptions, samDesired | KEY_WOW64_32KEY, phkResult);
	}
	else
	{
		return ::RegOpenKeyEx(hKey, lpSubKey, ulOptions, samDesired | KEY_WOW64_64KEY, phkResult);
	}
}

LONG _DeleteKey(
	__in HKEY hKey,
	__in LPCTSTR lpSubKey,
	__in REGSAM samDesired,
	__reserved DWORD Reserved,
	__in BOOL bIsWin32Key = IS_X86)
{
	if (bIsWin32Key)
	{
		return ::RegDeleteKeyEx(hKey, lpSubKey, samDesired | KEY_WOW64_32KEY, Reserved);
	}
	else
	{
		return ::RegDeleteKeyEx(hKey, lpSubKey, samDesired | KEY_WOW64_64KEY, Reserved);
	}
}

LONG _CreateKey(
	__in HKEY hKey,
	__in LPCTSTR lpSubKey,
	__reserved DWORD Reserved,
	__in_opt LPTSTR lpClass,
	__in DWORD dwOptions,
	__in REGSAM samDesired,
	__in_opt CONST LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	__out PHKEY phkResult,
	__out_opt LPDWORD lpdwDisposition,
	__in BOOL bIsWin32Key = IS_X86)
{
	if (bIsWin32Key)
	{
		return ::RegCreateKeyEx(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired | KEY_WOW64_32KEY, lpSecurityAttributes, phkResult, lpdwDisposition);
	}
	else
	{
		return ::RegCreateKeyEx(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired | KEY_WOW64_64KEY, lpSecurityAttributes, phkResult, lpdwDisposition);
	}
}


Registry::Registry()
{
}

RegistryKey* Registry::AddUsingRegistryKey(const StringType & path, RegistryKey * key)
{
	RemoveUsingRegistryKey(path);
	m_usingKeys[path] = key;
	return key;
}

void Registry::RemoveUsingRegistryKey(const StringType & path)
{
	RegistryKeyMap::iterator iter = m_usingKeys.find(path);
	if (iter != m_usingKeys.end())
	{
		delete iter->second;
		iter->second = NULL;
		m_usingKeys.erase(path);
	}
}


Registry::~Registry()
{
}

RegistryException::RegistryException(NumberType code) :
	m_code(code)
{
#define REGISTRY_ERROR_INVALID_HKEY  -5

	if (code > 0)
	{
		//Windows error message.
		LPTSTR lpMsgBuf = NULL;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			m_code,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, NULL);

		m_what.assign(lpMsgBuf);

		::LocalFree(lpMsgBuf);
	}
	else
	{
		//Custom error message.
		switch (code)
		{
		case REGISTRY_ERROR_INVALID_HKEY:  m_what = _T("ע���������ѹرջ���ɾ��"); break;
		default:
			break;
		}
	}
}

RegistryException::~RegistryException()
{
}

NumberType RegistryException::Code() const
{
	return m_code;
}

StringType RegistryException::What() const
{
	return m_what;
}

RegistryKey::RegistryKey(NumberType key, const StringType& keyName) :
	m_hSubKey(NULL)
{
	HKEY hKey = ((HKEY)(ULONG_PTR)((LONG)key));
	HKEY hOpenKey = NULL;
	LONG rv = _OpenKey(hKey, NULL, 0, KEY_READ, &hOpenKey);
	if (ERROR_SUCCESS == rv)
	{
		m_hSubKey = hOpenKey;
		m_name = keyName;
	}
}

RegistryKey::RegistryKey(void * hKey, const StringType & keyName) :
	m_hSubKey(hKey),
	m_name(keyName)
{
}

RegistryKey::~RegistryKey()
{
	//Close();
	if (NULL != m_hSubKey)
	{
		HKEY hKey = (HKEY)m_hSubKey;
		::RegCloseKey(hKey);
		m_hSubKey = NULL;
	}
}

StringType RegistryKey::GetName() const
{
	return m_name;
}

void RegistryKey::Close()
{
	Registry::RemoveUsingRegistryKey(m_name);
}

RegistryKey & RegistryKey::CreateSubKey(const StringType & subKeyPath)
{
	/*
	 * dw = REG_CREATED_NEW_KEY(0x00000001L) : ˵���ü����´���
	 * dw = REG_OPENED_EXISTING_KEY(0x00000002L) : ˵���ü���������
	 */

	if (NULL == m_hSubKey)
	{
		throw RegistryException(REGISTRY_ERROR_INVALID_HKEY);
	}

	DWORD dw = 0;
	LONG rv = ERROR_SUCCESS;
	HKEY hOpenKey = NULL;
	rv = _CreateKey((HKEY)m_hSubKey, subKeyPath.c_str(), 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_READ, NULL, &hOpenKey, &dw);
	if (ERROR_SUCCESS != rv)
	{
		throw RegistryException(rv);
	}

	return *Registry::AddUsingRegistryKey(
		subKeyPath,
		new RegistryKey(
			hOpenKey,
			GetKeyNameOfPath(subKeyPath)));
}

void RegistryKey::DeleteSubKey(const StringType & subKeyPath)
{
	if (NULL == m_hSubKey)
	{
		throw RegistryException(REGISTRY_ERROR_INVALID_HKEY);
	}
	LONG rv = ERROR_SUCCESS;
	if (ERROR_SUCCESS != (rv = _DeleteKey((HKEY)m_hSubKey, subKeyPath.c_str(), KEY_READ | KEY_WRITE, 0)))
	{
		throw RegistryException(rv);
	}
}

void RegistryKey::DeleteSubKeyTree(const StringType & subKeyNode)
{
	RegistryKey& subKey = this->OpenSubKey(subKeyNode);
	StringArray subkeyNames = subKey.GetSubKeyNames();
	StringArray::const_iterator iter = subkeyNames.begin();
	StringArray::const_iterator end = subkeyNames.end();
	for (; iter != end; ++iter)
	{
		subKey.DeleteSubKeyTree(*iter);
	}

	DeleteSubKey(subKeyNode);
	subKey.Close();
}

void RegistryKey::DeleteValue(const StringType & valueName)
{
	if (NULL == m_hSubKey)
	{
		throw RegistryException(REGISTRY_ERROR_INVALID_HKEY);
	}
	LONG rv = ERROR_SUCCESS;
	if (ERROR_SUCCESS != (rv = ::RegDeleteValue((HKEY)m_hSubKey, valueName.c_str())))
	{
		throw RegistryException(rv);
	}
}

StringArray RegistryKey::GetSubKeyNames() const
{
	if (NULL == m_hSubKey)
	{
		throw RegistryException(REGISTRY_ERROR_INVALID_HKEY);
	}

	StringArray keys;

	DWORD dwIndexs = 0; //��Ҫ������������� 
	TCHAR keyName[MAX_PATH] = { 0 }; //�����Ӽ������� 
	DWORD charLength = ARRAYSIZE(keyName);  //��Ҫ��ȡ�����ֽڲ�����ʵ�ʶ�ȡ�����ַ�����
	LONG rv = ERROR_SUCCESS;
	while (ERROR_SUCCESS == (rv = RegEnumKeyEx((HKEY)m_hSubKey, dwIndexs, keyName, &charLength, NULL, NULL, NULL, NULL)))
	{
		++dwIndexs;
		charLength = ARRAYSIZE(keyName);
		keys.push_back(keyName);
	}

	return keys;
}

NumberType RegistryKey::GetValue(const StringType& name, NumberType defaultValue) const
{
	if (NULL == m_hSubKey)
	{
		throw RegistryException(REGISTRY_ERROR_INVALID_HKEY);
	}

	DWORD dwValue = 0;
	DWORD dwType = REG_DWORD;
	DWORD dwSize = sizeof(dwValue);
	LONG rv = ::RegQueryValueEx((HKEY)m_hSubKey, name.c_str(), 0, &dwType, (LPBYTE)&dwValue, &dwSize);
	if (ERROR_SUCCESS != rv)
	{
		return defaultValue;
	}

	return NumberType(dwValue);
}

StringType RegistryKey::GetValue(const StringType & name, const StringType & defaultValue) const
{
	if (NULL == m_hSubKey)
	{
		throw RegistryException(REGISTRY_ERROR_INVALID_HKEY);
	}

	LONG rv = ERROR_SUCCESS;
	TCHAR *buffer = NULL;
	DWORD dwType = REG_SZ;
	DWORD dwSize = 0;
	rv = ::RegQueryValueEx((HKEY)m_hSubKey, name.c_str(), 0, &dwType, (LPBYTE)buffer, &dwSize);
	if (dwSize == 0)
	{
		return defaultValue;
	}
	buffer = new TCHAR[dwSize]();
	rv = ::RegQueryValueEx((HKEY)m_hSubKey, name.c_str(), 0, &dwType, (LPBYTE)buffer, &dwSize);
	if (ERROR_SUCCESS != rv)
	{
		if (NULL != buffer)
		{
			delete buffer;
			buffer = NULL;
		}
		return defaultValue;
	}

	StringType res(buffer);
	if (NULL != buffer)
	{
		delete buffer;
		buffer = NULL;
	}

	return res;
}

StringArray RegistryKey::GetValueNames() const
{
	if (NULL == m_hSubKey)
	{
		throw RegistryException(REGISTRY_ERROR_INVALID_HKEY);
	}

	StringArray valueNames;

	DWORD dwIndexs = 0; //��Ҫ������������� 
	TCHAR valueName[MAX_PATH] = { 0 }; //�����Ӽ������� 
	DWORD dwValueNameLength = ARRAYSIZE(valueName);  //��Ҫ��ȡ�����ֽڲ�����ʵ�ʶ�ȡ�����ַ�����
	LONG rv = ERROR_SUCCESS;
	while (ERROR_SUCCESS == (rv = RegEnumValue((HKEY)m_hSubKey, dwIndexs, valueName, &dwValueNameLength, NULL, NULL, NULL, NULL)))
	{
		++dwIndexs;
		dwValueNameLength = ARRAYSIZE(valueName);
		valueNames.push_back(valueName);
	}

	return valueNames;
}

RegistryKey & RegistryKey::OpenSubKey(const StringType & subKeyPath, bool writable)
{
	if (NULL == m_hSubKey)
	{
		throw RegistryException(REGISTRY_ERROR_INVALID_HKEY);
	}

	HKEY hOpenKey = NULL;
	LONG rv = _OpenKey((HKEY)m_hSubKey, subKeyPath.c_str(), 0, writable ? (KEY_WRITE | KEY_READ) : KEY_READ, &hOpenKey);
	if (ERROR_SUCCESS != rv)
	{
		throw RegistryException(rv);
	}

	return *Registry::AddUsingRegistryKey(
		subKeyPath,
		new RegistryKey(
			hOpenKey,
			GetKeyNameOfPath(subKeyPath)));
}

void RegistryKey::SetValue(const StringType & valueName, const StringType & val)
{
	if (NULL == m_hSubKey)
	{
		throw RegistryException(REGISTRY_ERROR_INVALID_HKEY);
	}

	LONG rv = ::RegSetValueEx((HKEY)m_hSubKey, valueName.c_str(), 0, REG_SZ, (const BYTE*)val.c_str(), val.length());
	if (ERROR_SUCCESS != rv)
	{
		throw RegistryException(rv);
	}
}

void RegistryKey::SetValue(const StringType & valueName, const NumberType & val)
{
	if (NULL == m_hSubKey)
	{
		throw RegistryException(REGISTRY_ERROR_INVALID_HKEY);
	}

	LONG rv = ::RegSetValueEx((HKEY)m_hSubKey, valueName.c_str(), 0, REG_DWORD, (const BYTE*)&val, sizeof(val));
	if (ERROR_SUCCESS != rv)
	{
		throw RegistryException(rv);
	}
}



StringType GetKeyNameOfPath(const StringType& path)
{
	if (path.empty())
	{
		return StringType();
	}

	StringType temp = path;
	TCHAR lastChar = temp[temp.length() - 1];
	if (lastChar == _T('\\'))
	{
		temp = temp.substr(0, temp.length() - 1);
	}

	std::size_t pos = temp.find_last_of(_T('\\'));
	if (StringType::npos == pos)
	{
		return temp;
	}

	return temp.substr(pos + 1);
}
