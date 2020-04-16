#pragma once
#include <map>
#include <vector>
#include <string>
#include <varargs.h>

#if !defined(WIN32) && !defined(_WIN32)
#ifndef sprintf_s
#define sprintf_s(buffer, size, format, ...) sprintf(buffer, format, ##__VA_ARGS__)
#endif // !sprintf_s
#endif // !defined(WIN32) && !defined(_WIN32)

class console_table
{
	template<typename T>
	struct type_to_format_string;

#define declare_type_to_format_string(t, format_string) \
template<> struct type_to_format_string<t> { using type = t; static constexpr const char* format = format_string; }
	declare_type_to_format_string(int, "%d");
	declare_type_to_format_string(short, "%hd");
	declare_type_to_format_string(long, "%ld");
	declare_type_to_format_string(long long, "%lld");
	declare_type_to_format_string(float, "%f");
	declare_type_to_format_string(double, "%lf");
	declare_type_to_format_string(std::string, "%s");
	declare_type_to_format_string(const char*, "%s");
	declare_type_to_format_string(bool, "%s");
	declare_type_to_format_string(decltype(nullptr), "%s");
#undef declare_type_to_format_string

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
	static T _min(T a, T b)
	{
		if (a < b)
		{
			return a;
		}
		return b;
	}

	template<typename T>
	void _handle_value(int index, T value)
	{
		char buffer[1024] = { 0 };
		auto column = this->m_column_names[index];
		sprintf_s(buffer, sizeof(buffer) / sizeof(buffer[0]), type_to_format_string<T>::format, value);
		this->m_datas[column].push_back(buffer);
		this->m_column_widths[column] = _max(this->m_column_widths[column], std::string(buffer).length());
	}

	template<>
	void _handle_value(int index, const std::string& value)
	{
		auto column = this->m_column_names[index];
		this->m_datas[column].push_back(value);
		this->m_column_widths[column] = _max(this->m_column_widths[column], value.length());
	}

	template<>
	void _handle_value(int index, bool value)
	{
		char buffer[1024] = { 0 };
		auto column = this->m_column_names[index];
		sprintf_s(buffer, sizeof(buffer) / sizeof(buffer[0]), type_to_format_string<bool>::format, value ? "true" : "false");
		this->m_datas[column].push_back(buffer);
		this->m_column_widths[column] = _max(this->m_column_widths[column], std::string(buffer).length());
	}

	template<>
	void _handle_value(int index, decltype(nullptr) value)
	{
		char buffer[1024] = { 0 };
		auto column = this->m_column_names[index];
		sprintf_s(buffer, sizeof(buffer) / sizeof(buffer[0]), type_to_format_string<decltype(nullptr)>::format, "NULL");
		this->m_datas[column].push_back(buffer);
		this->m_column_widths[column] = _max(this->m_column_widths[column], std::string(buffer).length());
	}

	template<typename T>
	void _unpack_values(int index, T value)
	{
		_handle_value(index, value);
	}

	template<typename T, typename ... Ts>
	void _unpack_values(int index, T value, Ts ... values)
	{
		_handle_value(index, value);
		if (index < this->m_column_names.size() - 1)
		{
			_unpack_values(index + 1, values ...); // unpack
		}
	}

public:
	console_table() = default;
	~console_table() = default;

	console_table& add_column(const std::string& column_name)
	{
		if (!column_name.empty())
		{
			this->m_column_names.push_back(column_name);
			this->m_column_widths[column_name] = column_name.length();
		}
		return *this;
	}

	template<typename ... types>
	console_table& add_row(const types& ... datas)
	{
		_unpack_values(0, datas...);
		return *this;
	}

	void print() const
	{
#define print_line printf(("-" + formats[column_name] + "--").c_str(), _line)

#define print_column_line \
do {\
	for (const auto& column_name : this->m_column_names)\
	{\
		print_line; \
	}\
} while (0)

		static char _line[256] = { 0 };
		memset(_line, '-', sizeof(_line) / sizeof(_line[0]) - 1);

		std::size_t min_rows_count = 0xffffffff;
		std::map<std::string, std::string> formats;
		// print top border
		for (const auto& column_name : this->m_column_names)
		{
			auto width = this->m_column_widths.find(column_name)->second + 1;
			char pre_format[256] = { 0 };
			sprintf_s(pre_format, "%%-%d.%ds", width, width);
			formats[column_name] = pre_format;
			print_line;
			auto row = this->m_datas.find(column_name);
			if (row != this->m_datas.end())
			{
				min_rows_count = _min(min_rows_count, row->second.size());
			}
		}
		printf("\n");


		// print table head
		for (const auto& column_name : this->m_column_names)
		{
			printf(std::string(" ").append(formats[column_name].c_str()).append("| ").c_str(), column_name.c_str());
		}
		printf("\n");

		print_column_line;
		printf("\n");

		// print table body
		for (std::size_t index = 0; index < min_rows_count; index++)
		{
			for (const auto& column_name : this->m_column_names)
			{
				auto row = this->m_datas.find(column_name);
				if (row != this->m_datas.end())
				{
					printf(std::string(" ").append(formats[column_name].c_str()).append("| ").c_str(), row->second[index].c_str());
				}
			}
			printf("\n");
		}

		// print bottom border
		print_column_line;
		printf("\n");

#undef print_line
	}

private:
	std::vector<std::string> m_column_names;
	std::map<std::string, std::size_t> m_column_widths;
	std::map<std::string, std::vector<std::string>> m_datas;
};
