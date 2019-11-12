#include "stdafx.h"
#include "ConfigurationManager.h"
#include "String.hpp"
#include <shlwapi.h>
#include <ShlObj.h>
#pragma comment(lib, "shlwapi.lib")

#define CONFIG_FILE_NAME "SignSealConfig.ini"

static std::string GetUserRoamingPath()
{
	static char strPath[1024 + MAX_PATH] = { 0 };
	if (strlen(strPath) > 0)
	{
		return strPath;
	}

	char cDataPath[MAX_PATH] = { 0 };
	if (cDataPath[0] == 0)
	{
		if (!SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, cDataPath)))
		{
			return ("");
		}
	}

	sprintf_s(strPath, ("%s\\NETCA\\SealSign\\"), cDataPath);

	if (!::PathIsDirectoryA(strPath))
	{
		if (0 == ::SHCreateDirectoryExA(NULL, strPath, NULL))
		{
			return ("");
		}
	}
	return strPath;
}
static std::string GetProgramDataPath()
{
	static char strPath[1024 + MAX_PATH] = { 0 };
	if (strlen(strPath) > 0)
	{
		return strPath;
	}

	char cDataPath[MAX_PATH] = { 0 };
	if (cDataPath[0] == 0)
	{
		if (!SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_CURRENT, cDataPath)))
		{
			return ("");
		}
	}

	sprintf_s(strPath, ("%s\\NETCA\\SealSign\\"), cDataPath);

	if (!::PathIsDirectoryA(strPath))
	{
		if (0 == ::SHCreateDirectoryExA(NULL, strPath, NULL))
		{
			return ("");
		}
	}
	return strPath;
}
static std::string GetModuleName()
{
	std::string filename;
	char path[MAX_PATH] = { 0 };
	if (GetModuleFileNameA(NULL, path, MAX_PATH) > 0)
	{
		filename = path;
		auto pos = filename.find_last_of("\\");
		if (std::string::npos == pos)
		{
			pos = filename.find_last_of("/");
		}
		if (std::string::npos != pos)
		{
			filename = filename.substr(pos);
		}
	}
	return filename;
}

template<typename T>
struct _ConfigurationReader
{
	static T GetValue(
		const std::string& filename,
		const std::string& section,
		const std::string& key,
		const T& defaultValue = T())
	{
		return defaultValue;
	}
};

template<>
struct _ConfigurationReader<int>
{
	static int GetValue(
		const std::string& filename,
		const std::string& section,
		const std::string& key,
		const int& defaultValue = 0)
	{
		return ::GetPrivateProfileIntA(section.c_str(), key.c_str(), defaultValue, filename.c_str());
	}
};

template<>
struct _ConfigurationReader<std::string>
{
	static std::string GetValue(
		const std::string& filename,
		const std::string& section,
		const std::string& key,
		const std::string& defaultValue = std::string())
	{
		char buffer[1024] = { 0 };
		auto nRes = ::GetPrivateProfileStringA(
			section.c_str(),
			key.c_str(),
			defaultValue.c_str(),
			buffer, sizeof(buffer) / sizeof(char),
			filename.c_str());
		return buffer;
	}
};

template<>
struct _ConfigurationReader<bool>
{
	static bool GetValue(
		const std::string& filename,
		const std::string& section,
		const std::string& key,
		const bool& defaultValue = false)
	{
		char buffer[1024] = { 0 };
		auto nRes = ::GetPrivateProfileStringA(
			section.c_str(),
			key.c_str(),
			"",
			buffer, sizeof(buffer) / sizeof(char),
			filename.c_str());

		auto str = common::text::string<char>(buffer);

		if (str.empty())
		{
			throw false;
		}

		if (common::text::string<char>(buffer).tolower() == "true")
		{
			return true;
		}

		return false;
	}
};

static std::string GetConfigStringValue(const std::string& section, const std::string& key, const std::string& defaultVal = "")
{
	auto userPath = GetUserRoamingPath().append(CONFIG_FILE_NAME);
	auto globalPath = GetProgramDataPath().append(CONFIG_FILE_NAME);
	std::string val = _ConfigurationReader<std::string>::GetValue(userPath, section, key);
	if (val.empty())
	{
		val = _ConfigurationReader<std::string>::GetValue(globalPath, section, key);
		//使用默认值
		if (val.empty())
		{
			val = defaultVal;
		}
	}
	return val;
}

static bool GetConfigBoolValue(const std::string& section, const std::string& key, bool defaultVal = false)
{
	auto userPath = GetUserRoamingPath().append(CONFIG_FILE_NAME);
	auto globalPath = GetProgramDataPath().append(CONFIG_FILE_NAME);
	bool val = false;
	try
	{
		val = _ConfigurationReader<bool>::GetValue(userPath, section, key);
	}
	catch (bool)
	{
		try
		{
			val = _ConfigurationReader<bool>::GetValue(globalPath, section, key);
		}
		catch (bool)
		{
			//使用默认值
			val = defaultVal;

		}
	}
	return val;
}

#define DEFINE_STRING_CONFIG(functionName, section, key, ...) \
std::string ConfigurationManager::functionName() const { return GetConfigStringValue(section, key, ##__VA_ARGS__); }

#define DEFINE_BOOL_CONFIG(functionName, section, key, ...) \
bool ConfigurationManager::functionName() const { return GetConfigBoolValue(section, key, ##__VA_ARGS__); }

ConfigurationManager::ConfigurationManager() :
	m_appName(GetModuleName())
{
}

ConfigurationManager & ConfigurationManager::Instance()
{
	static ConfigurationManager cm;
	return cm;
}

ConfigurationManager::~ConfigurationManager()
{
}

std::string ConfigurationManager::GetAppName() const
{
	return m_appName;
}

//DEFINE_BOOL_CONFIG(IsAutoUpdate, "Other", "IsAutoUpdate", false);
