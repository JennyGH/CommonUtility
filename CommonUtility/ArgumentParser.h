#pragma once
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

typedef std::ios_base&(io_opt_t)(std::ios_base&);


class ArgumentNotFoundException : public std::exception
{
public:
	ArgumentNotFoundException(const std::string& moduleName, const std::string& optionName);
	~ArgumentNotFoundException();
	const std::string& getModuleName() const;
private:
	std::string	m_moduleName;
};

class ArgumentParser
{
	void _checkArgument(const std::string& argumentName);
public:
	ArgumentParser(int argc, char** argv);

	~ArgumentParser();


	template<typename T>
	T get(const std::string& argumentName)
	{
		_checkArgument(argumentName);
		T val = T();
		m_arguments[argumentName] >> val;
		return val;
	}

	template<typename T>
	T get(const std::string& argumentName, T defaultValue)
	{
		T val = defaultValue;
		m_arguments[argumentName] >> val;
		return val;
	}

	template<typename T>
	T get(const std::string& argumentName, io_opt_t opt)
	{
		_checkArgument(argumentName);
		T val = T();
		if (nullptr != opt)
		{
			m_arguments[argumentName] >> opt >> val;
		}
		else
		{
			m_arguments[argumentName] >> val;
		}
		return val;
	}

	template<typename T>
	T get(const std::string& argumentName, io_opt_t opt, T defaultValue)
	{
		T val = defaultValue;
		if (nullptr != opt)
		{
			m_arguments[argumentName] >> opt >> val;
		}
		else
		{
			m_arguments[argumentName] >> val;
		}
		return val;
	}

	std::string getApplicationName() const;

private:
	std::string m_application;
	std::map<std::string, std::stringstream> m_arguments;
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
