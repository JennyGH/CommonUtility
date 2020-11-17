#include "ArgumentParser.h"

ArgumentParser::ArgumentParser(int argc, char ** argv) : m_application(argv[0])
{
    if (argc <= 1)
    {
        return;
    }
    for (int index = 1; index < argc; index++)
    {
        std::string str(argv[index]);
        std::size_t pos = str.find_first_of('=');
        if (std::string::npos == pos)
        {
            m_arguments[str.substr(1)] = "true";
        }
        else
        {
            m_arguments[str.substr(1, pos - 1)] = str.substr(pos + 1);
        }
    }
}

ArgumentParser::~ArgumentParser()
{

}

std::string ArgumentParser::getApplicationName() const
{
    return m_application;
}

ArgumentNotFoundException::ArgumentNotFoundException(const std::string & moduleName, const std::string & optionName)
    : m_moduleName(moduleName)
    , m_message("The argument `" + optionName + "` is required.")
{
}

ArgumentNotFoundException::~ArgumentNotFoundException()
{
}

const char* ArgumentNotFoundException::getModuleName() const
{
    return m_moduleName.c_str();
}

const char * ArgumentNotFoundException::what() const
{
    return m_message.c_str();
}
