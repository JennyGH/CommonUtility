#pragma once
#include <map>
#include <vector>
#include <string>
#include <varargs.h>
class console_table
{
public:
	template<typename T>
	struct type_to_format_string;

#define declare_type_to_format_string(t, format_string) \
template<> struct type_to_format_string<t> { using type = t; static constexpr const char* format = format_string; }
	declare_type_to_format_string(int, "d");
	declare_type_to_format_string(short, "hd");
	declare_type_to_format_string(long, "ld");
	declare_type_to_format_string(long long, "lld");
	declare_type_to_format_string(float, "f");
	declare_type_to_format_string(double, "lf");
	declare_type_to_format_string(std::string, "s");
	declare_type_to_format_string(const char*, "s");
#undef declare_type_to_format_string

private:
	template<typename T>
	static T _max(T a, T b)
	{
		if (a > b)
		{
			return a;
		}
		return b;
	}

	template<typename T>
	void _handl_value(int index, T value)
	{
		char buffer[1024] = { 0 };
		auto column = this->m_column_names[index];
		sprintf_s(buffer, std::string("%").append(type_to_format_string<T>::format).c_str(), value);
		this->m_datas[column].push_back(buffer);
		this->m_column_widths[column] = _max(this->m_column_widths[column], std::string(buffer).length());
	}

	template<>
	void _handl_value(int index, const std::string& value)
	{
		auto column = this->m_column_names[index];
		this->m_datas[column].push_back(value);
		this->m_column_widths[column] = _max(this->m_column_widths[column], value.length());
	}

	template<typename T>
	void _process_values(int index, T value)
	{
		_handl_value(index, value);
	}

	template<typename T, typename ... Ts>
	void _process_values(int index, T value, Ts ... values)
	{
		_handl_value(index, value);
		if (index < this->m_column_names.size() - 1)
		{
			_process_values(index + 1, values ...); // unpack
		}
	}

public:
	console_table() = default;
	~console_table() = default;

	template<typename data_type>
	console_table& add_column(const std::string& column_name)
	{
		if (!column_name.empty())
		{
			this->m_column_names.push_back(column_name);
		}
		return *this;
	}

	template<typename ... types>
	void add_row(const types& ... datas)
	{
		_process_values(0, datas...);
	}

	void print() const
	{
		//¥Ú”°±Ì∏Ò
	}

private:
	std::vector<std::string> m_column_names;
	std::map<std::string, std::size_t> m_column_widths;
	std::map<std::string, std::vector<std::string>> m_datas;
};
