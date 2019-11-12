#pragma once

class ConfigurationManager
{
	ConfigurationManager();
 
public:
	static ConfigurationManager& Instance();
	~ConfigurationManager()  ;

	std::string GetAppName() const;

private:
	std::string m_appName;
};

