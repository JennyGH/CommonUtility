#include "pch.h"
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
			m_arguments[str.substr(1)] << true;
		}
		else
		{
			m_arguments[str.substr(1, pos - 1)] << str.substr(pos + 1);
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

ArgumentNotFoundException::ArgumentNotFoundException(const std::string & moduleName, const std::string & optionName) :
	std::exception(std::logic_error(("The `" + optionName + "` is requeired.").c_str())),
	m_moduleName(moduleName)
{
}

ArgumentNotFoundException::~ArgumentNotFoundException()
{
}

const std::string & ArgumentNotFoundException::getModuleName() const
{
	return m_moduleName;
}

Manual::Manual()
{
}

Manual::~Manual()
{
}

Manual & Manual::Add(const std::string & optionName, bool isRequired, const std::string & defaultValue, const std::string & describtion)
{
#define ASSIGN_MAX_WIDTH(index) \
m_widths[index] = row[index].length() > m_widths[index] ? row[index].length() : m_widths[index]

	RowType row;
	row.push_back("\t" + optionName);
	row.push_back(isRequired ? "REQUIRED" : "OPTIONAL");
	row.push_back("DEFAULT(" + defaultValue + ")");
	row.push_back(describtion);

	if (m_widths.empty())
	{
		m_widths.resize(row.size());
		for (ColumnWidthValuesType::iterator iter = m_widths.begin(); iter != m_widths.end(); ++iter)
		{
			*iter = 0;
		}
	}

	for (std::size_t index = 0; index < row.size(); index++)
	{
		ASSIGN_MAX_WIDTH(index);
	}

	m_rows.push_back(row);

#undef ASSIGN_MAX_WIDTH

	return *this;
}

std::string Manual::ToString() const
{
	std::stringstream output;

	RowsType::const_iterator rowIter = m_rows.begin();
	RowsType::const_iterator rowEnd = m_rows.end();
	for (; rowIter != rowEnd; ++rowIter)
	{
		const RowType& row = *rowIter;
		for (std::size_t index = 0; index < row.size(); index++)
		{
			const std::string& column = row[index];
			ColumnWidthType columnWidth = m_widths[index];
			output << std::setiosflags(std::ios::left) << std::setw(columnWidth + 5) << column;
		}
		output << std::endl;
	}
	output << std::endl;

	return output.str();
}
