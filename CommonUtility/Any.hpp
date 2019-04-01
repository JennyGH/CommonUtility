#pragma once

#include <string>

class bad_any_cast;
class any
{
public:

public:
	any();
	template<typename _Val>
	any(const _Val& val);
	any(const any& that);
	any& operator= (const any& that);
	template<typename _Val>
	any& operator= (const _Val& val);
	~any();

	void reset() throw();
	bool has_value() const throw();
	const std::type_info& type() const throw();

	template<typename _Val>
	const _Val& cast() const throw(const bad_any_cast&);

private:
	class basic_placeholder
	{
	public:
		virtual ~basic_placeholder() {};
		virtual basic_placeholder* clone() const = 0;
		virtual const std::type_info& type_info() const throw() = 0;
	};
	template<typename _Val>
	class placeholder : public basic_placeholder
	{
	public:
		placeholder(const _Val& val);
		~placeholder();
		basic_placeholder* clone() const;
		const std::type_info& type_info() const throw();
	public:
		_Val Value;
	};

private:
	template<typename _Val>
	const _Val& get_value() const
	{
		if (this->type() == typeid(_Val))
		{
			return static_cast<any::placeholder<_Val>*>(m_holder)->Value;
		}
		throw bad_any_cast(this->type(), typeid(_Val));
	}
private:
	basic_placeholder* m_holder;
};


class bad_any_cast
{
public:
	bad_any_cast(const std::type_info& src, const std::type_info& dest)
	{
		char _buffer[1024] = { 0 };
		sprintf_s(_buffer, "Can not convert value from `%s` to `%s`.", src.name(), dest.name());
		m_what.assign(_buffer);
	}
	const char* what() const
	{
		return m_what.c_str();
	}
	~bad_any_cast() {}

private:
	std::string m_what;
};

inline any::any() : m_holder(NULL) {}

inline any::any(const any & that) : m_holder(NULL)
{
	if (that.m_holder != NULL)
	{
		m_holder = that.m_holder->clone();
	}
}

inline any& any::operator= (const any& that)
{
	if (&that != this)
	{
		reset();
		if (that.m_holder != NULL)
		{
			m_holder = that.m_holder->clone();
		}
	}
	return *this;
}

template<typename _Val>
inline any& any::operator= (const _Val& val)
{
	reset();
	m_holder = new placeholder<_Val>(val);
	return *this;
}


inline any::~any()
{
	if (NULL != m_holder)
	{
		delete m_holder;
		m_holder = NULL;
	}
}

void any::reset() throw()
{
	if (NULL != m_holder)
	{
		delete m_holder;
		m_holder = NULL;
	}
}

bool any::has_value() const throw()
{
	return NULL != m_holder;
}

const std::type_info& any::type() const throw()
{
	if (NULL == m_holder)
	{
		return typeid(void);
	}
	return m_holder->type_info();
}

template<typename _Val>
inline const _Val& any::cast() const throw(const bad_any_cast &)
{
	return get_value<_Val>();
}

template<typename _Val>
inline any::any(const _Val & val) : m_holder(new any::placeholder<_Val>(val))
{
}

template<typename _Val>
inline any::placeholder<_Val>::placeholder(const _Val & val) :
	Value(val)
{
}

template<typename _Val>
inline any::placeholder<_Val>::~placeholder()
{
}

template<typename _Val>
inline any::basic_placeholder * any::placeholder<_Val>::clone() const
{
	return new any::placeholder<_Val>(Value);
}

template<typename _Val>
inline const std::type_info & any::placeholder<_Val>::type_info() const throw()
{
	return typeid(_Val);
}