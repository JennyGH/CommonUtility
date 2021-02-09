#pragma once
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>

typedef std::map<std::string, std::string> _ArgumentMap;

class ArgumentNotFoundException
{
public:
    ArgumentNotFoundException(const std::string& moduleName, const std::string& optionName);
    ~ArgumentNotFoundException();
    const char* getModuleName() const;
    const char* what() const;

private:
    std::string m_moduleName;
    std::string m_message;
};

static void
_IsArgumentExist(const std::string& argumentName, const std::string& applicationName, _ArgumentMap& arguments)
{
    if (arguments.find(argumentName) == arguments.end())
    {
        throw ArgumentNotFoundException(applicationName, argumentName);
    }
}

template <typename ValueType>
struct _ArgumentGetter
{
    static ValueType
    ValueOf(const std::string& argumentName, const std::string& applicationName, _ArgumentMap& arguments)
    {
        _IsArgumentExist(argumentName, applicationName, arguments);
        ValueType val = ValueType();
        arguments[argumentName] >> val;
        return val;
    }
    static ValueType ValueOf(
        const std::string& argumentName,
        const std::string& applicationName,
        _ArgumentMap&      arguments,
        ValueType          defaultValue)
    {
        ValueType val = defaultValue;
        if (arguments.find(argumentName) != arguments.end())
        {
            std::stringstream ss;
            ss << arguments[argumentName];
            ss >> val;
        }
        return val;
    }
};

template <>
struct _ArgumentGetter<bool>
{
    static bool ValueOf(const std::string& argumentName, const std::string& applicationName, _ArgumentMap& arguments)
    {
        _IsArgumentExist(argumentName, applicationName, arguments);
        bool val = false;
        if (arguments.find(argumentName) != arguments.end())
        {
            val = arguments[argumentName] == "true";
        }
        return val;
    }
    static bool ValueOf(
        const std::string& argumentName,
        const std::string& applicationName,
        _ArgumentMap&      arguments,
        bool               defaultValue)
    {
        bool val = defaultValue;
        if (arguments.find(argumentName) != arguments.end())
        {
            val = arguments[argumentName] == "true";
        }
        return val;
    }
};

template <>
struct _ArgumentGetter<std::string>
{
    static std::string
    ValueOf(const std::string& argumentName, const std::string& applicationName, _ArgumentMap& arguments)
    {
        _IsArgumentExist(argumentName, applicationName, arguments);
        return arguments[argumentName];
    }
    static std::string ValueOf(
        const std::string& argumentName,
        const std::string& applicationName,
        _ArgumentMap&      arguments,
        std::string        defaultValue)
    {
        std::string val = defaultValue;
        if (arguments.find(argumentName) == arguments.end())
        {
            return val;
        }
        return arguments[argumentName];
    }
};

class ArgumentParser
{
public:
    ArgumentParser(int argc, char** argv);

    ~ArgumentParser();

    template <typename T>
    T get(const std::string& argumentName)
    {
        return _ArgumentGetter<T>::ValueOf(argumentName, m_application, m_arguments);
    }

    template <typename T>
    T get(const std::string& argumentName, T defaultValue)
    {
        return _ArgumentGetter<T>::ValueOf(argumentName, m_application, m_arguments, defaultValue);
    }

    std::string getApplicationName() const;

private:
    std::string  m_application;
    _ArgumentMap m_arguments;
};