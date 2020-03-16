#pragma once
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

typedef std::ios_base& (io_opt_t)(std::ios_base&);
typedef std::map<std::string, std::stringstream> ArgumentMap;

class ArgumentNotFoundException : public std::exception
{
public:
	ArgumentNotFoundException(const std::string& moduleName, const std::string& optionName);
	~ArgumentNotFoundException();
	const std::string& getModuleName() const;
private:
	std::string	m_moduleName;
};


static bool _IsArgumentExist(const std::string& argumentName, const std::string& applicationName, ArgumentMap& arguments)
{
	if (arguments.find(argumentName) == arguments.end())
	{
		throw ArgumentNotFoundException(applicationName, argumentName);
	}
}

template<typename ValueType>
struct _ArgumentGetter
{
	static ValueType ValueOf(const std::string& argumentName, const std::string& applicationName, ArgumentMap& arguments)
	{
		_IsArgumentExist(argumentName, applicationName, arguments);
		ValueType val = ValueType();
		arguments[argumentName] >> val;
		return val;
	}
	static ValueType ValueOf(const std::string& argumentName, const std::string& applicationName, ArgumentMap& arguments, ValueType defaultValue)
	{
		ValueType val = defaultValue;
		if (arguments.find(argumentName) != arguments.end())
		{
			arguments[argumentName] >> val;
		}
		return val;
	}
};

template<>
struct _ArgumentGetter<bool>
{
	static bool ValueOf(const std::string& argumentName, const std::string& applicationName, ArgumentMap& arguments)
	{
		bool val = false;
		if (arguments.find(argumentName) != arguments.end())
		{
			arguments[argumentName] >> val;
		}
		return val;
	}
	static bool ValueOf(const std::string& argumentName, const std::string& applicationName, ArgumentMap& arguments, bool defaultValue)
	{
		bool val = defaultValue;
		if (arguments.find(argumentName) != arguments.end())
		{
			arguments[argumentName] >> val;
		}
		return val;
	}
};

template<>
struct _ArgumentGetter<std::string>
{
	static std::string ValueOf(const std::string& argumentName, const std::string& applicationName, ArgumentMap& arguments)
	{
		_IsArgumentExist(argumentName, applicationName, arguments);
		return arguments[argumentName].str();
	}
	static std::string ValueOf(const std::string& argumentName, const std::string& applicationName, ArgumentMap& arguments, std::string defaultValue)
	{
		std::string val = defaultValue;
		if (arguments.find(argumentName) == arguments.end())
		{
			return val;
		}
		return arguments[argumentName].str();
	}
};

class ArgumentParser
{
public:
	ArgumentParser(int argc, char** argv);

	~ArgumentParser();

	template<typename T>
	T get(const std::string& argumentName)
	{
		return _ArgumentGetter<T>::ValueOf(argumentName, m_application, m_arguments);
	}

	template<typename T>
	T get(const std::string& argumentName, T defaultValue)
	{
		return _ArgumentGetter<T>::ValueOf(argumentName, m_application, m_arguments, defaultValue);
	}

	std::string getApplicationName() const;

private:
	std::string m_application;
	ArgumentMap m_arguments;
};

class Manual
{
	typedef std::string ColumnType;
	typedef std::vector<std::string> RowType;
	typedef std::vector<RowType> RowsType;
	typedef std::size_t ColumnWidthType;
	typedef std::vector<ColumnWidthType> ColumnWidthValuesType;
public:
	Manual();
	~Manual();

	Manual& Add(
		const std::string& optionName,
		bool isRequired,
		const std::string& defaultValue,
		const std::string& describtion);

	std::string ToString() const;

private:
	RowsType m_rows;
	ColumnWidthValuesType m_widths;
};
