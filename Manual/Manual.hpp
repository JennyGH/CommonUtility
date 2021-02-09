#pragma once
#include <tuple>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

class manual
{
public:
    static constexpr std::size_t padding = 2;
    using string = std::string;
    using string_ref = std::string&;
    using const_string_ref = const std::string&;

    using row = std::tuple<
        string, // name
        string, // requirement
        string, // default
        string  // describtion
    >;
    using rows = std::vector<row>;

    using column_size = std::size_t;
    using column_sizes = std::tuple<column_size, column_size, column_size, column_size>;

    using header = string;
    using headers = std::tuple<header, header, header, header>;

private:
    static string _serialize_value(int val)
    {
        return std::to_string(val);
    }
    static string _serialize_value(bool val)
    {
        return val ? "true" : "false";
    }
    static string _serialize_value(const_string_ref val)
    {
        return val;
    }

public:
    manual(const_string_ref filename)
        : m_file_name(filename)
        , m_headers(std::make_tuple("NAME", "REQUIREMENT", "DEFAULT", "DESCRIBTION"))
        , m_column_size(std::make_tuple(
            std::get<0>(m_headers).length(),
            std::get<1>(m_headers).length(),
            std::get<2>(m_headers).length(),
            std::get<3>(m_headers).length()
        ))
    {
    }

    template<typename T>
    void push_back(
        const_string_ref name,
        bool             requirement,
        const T&         defaultValue,
        const_string_ref describtion
    );

    void push_back(
        const_string_ref name,
        bool             requirement,
        const_string_ref describtion
    );

    string serialize() const
    {
        std::stringstream serialized;
        serialized.setf(std::ios::left);
        serialized << "Usage: " << m_file_name << " [-<OPTION>[=<VALUE>]]" << std::endl;

#define __set_output_width__(index)\
serialized.width(std::get<index>(m_column_size) + manual::padding)

#define __output_line__(index) \
do {\
    __set_output_width__(index);\
    serialized << "=";\
} while (0)

#define __output_header__(index) \
do {\
    __set_output_width__(index);\
    serialized << std::get<index>(m_headers);\
} while (0)

#define __output_column__(index) \
do {\
    __set_output_width__(index);\
    serialized << std::boolalpha << std::get<index>(row);\
} while (0)

#define __output_all_lines__() \
do {\
    __output_line__(0);\
    __output_line__(1);\
    __output_line__(2);\
    __output_line__(3);\
} while (0)

#define __output_all_headers() \
do {\
    __output_header__(0);\
    __output_header__(1);\
    __output_header__(2);\
    __output_header__(3);\
} while (0)

#define __output_all_columns__() \
do {\
    __output_column__(0);\
    __output_column__(1);\
    __output_column__(2);\
    __output_column__(3);\
} while (0)

        // output header.
        {
            serialized.fill(' ');
            __output_all_headers();
            serialized << std::endl;
        }

        // output line.
        {
            serialized.fill('=');
            __output_all_lines__();
            serialized << std::endl;
        }

        // output columns.
        for (const auto& row : m_rows)
        {
            serialized.fill(' ');
            __output_all_columns__();
            serialized << std::endl;
        }

        // output line.
        {
            serialized.fill('=');
            __output_all_lines__();
            serialized << std::endl;
        }

        return serialized.str();

#undef __output_line__
#undef __output_header__
#undef __output_column__
#undef __output_all_lines__
#undef __output_all_headers__
#undef __output_all_columns__
#undef __set_output_width__
    }

private:
    const_string_ref m_file_name;
    headers          m_headers;
    column_sizes     m_column_size;
    rows             m_rows;
};

template<typename T>
inline void manual::push_back(const_string_ref name, bool requirement, const T & defaultValue, const_string_ref describtion)
{
    m_rows.emplace_back(std::make_tuple(
        name,
        manual::_serialize_value(requirement),
        manual::_serialize_value(defaultValue),
        describtion
    ));
    const row& last_row = m_rows.back();

    // update column size: 
#define __max_column_size__(index) \
std::max(std::get<index>(last_row).length(), std::get<index>(m_column_size))

    m_column_size = std::make_tuple(
        __max_column_size__(0),
        __max_column_size__(1),
        __max_column_size__(2),
        __max_column_size__(3)
    );

#undef __max_column_size__
}

inline void manual::push_back(const_string_ref name, bool requirement, const_string_ref describtion)
{
    push_back(name, requirement, std::string(), describtion);
}
