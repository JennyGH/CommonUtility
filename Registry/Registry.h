#pragma once
#include <string>
#include <vector>
#include <map>

class Registry;
class RegistryKey;
class RegistryException;

typedef long NumberType;
#ifdef UNICODE
typedef std::wstring StringType;
#else
typedef std::string StringType;
#endif // UNICODE
typedef std::vector<StringType> StringArray;

//×¢²á±í
class Registry
{
	typedef std::map<StringType, RegistryKey*> RegistryKeyMap;
	friend class RegistryKey;
	Registry();
	static RegistryKey* AddUsingRegistryKey(const StringType& path, RegistryKey* key);
	static void RemoveUsingRegistryKey(const StringType& path);
public:
	~Registry();
public:
	static RegistryKey ClassesRoot;
	static RegistryKey CurrentUser;
	static RegistryKey LocalMachine;
	static RegistryKey User;
	static RegistryKey CurrentConfig;
	static RegistryKey DynData;
	static RegistryKey PerformanceData;

private:
	static RegistryKeyMap m_usingKeys;
};

//×¢²á±í¼ü
class RegistryKey
{
	RegistryKey(NumberType key, const StringType& keyName);
	RegistryKey(void* hKey, const StringType& keyName);
	RegistryKey(const RegistryKey& that);
	RegistryKey& operator= (const RegistryKey& that);
	friend class Registry;
public:
	~RegistryKey();
	StringType GetName() const;
	void Close();
	RegistryKey& CreateSubKey(const StringType& subKeyPath);
	void DeleteSubKey(const StringType& subKeyName);
	void DeleteSubKeyTree(const StringType& subKeyNode);
	void DeleteValue(const StringType& valueName);
	StringArray GetSubKeyNames() const;
	NumberType GetValue(const StringType& name, NumberType defaultValue = 0) const;
	StringType GetValue(const StringType& name, const StringType& defaultValue = StringType()) const;
	StringArray GetValueNames() const;
	RegistryKey& OpenSubKey(const StringType& subKeyPath, bool writable = false);
	void SetValue(const StringType& valueName, const StringType& val);
	void SetValue(const StringType& valueName, const NumberType& val);
private:
	StringType m_name;
	void* m_hSubKey;
};

//×¢²á±íÒì³£
class RegistryException
{
public:
	RegistryException(NumberType code);
	~RegistryException();
	NumberType Code() const;
	StringType What() const;
private:
	NumberType m_code;
	StringType m_what;
};