#pragma once
#include <jni.h>
#include <string>

namespace Java
{
	enum TypeEnum
	{
		_Void = 0,
		_Int,
		_Long,
		_Object
	};
}

namespace Java
{
	template<typename T>
	struct Type { };
#define DECLARE_TYPE(type, enumVal) \
template<> struct Type<type> { static const TypeEnum Name = enumVal; }

	DECLARE_TYPE(int, _Int);
	DECLARE_TYPE(long, _Long);
	DECLARE_TYPE(void, _Void);
	DECLARE_TYPE(jobject, _Object);
}

namespace Java
{
	class MethodBase
	{
	public:
		MethodBase(JNIEnv* env, jmethodID methodId) :m_env(env), m_methodId(methodId) {}
		~MethodBase() = default;
		virtual bool IsValid() const = 0;
	protected:
		JNIEnv* m_env;
		jmethodID m_methodId;
	};
}

static std::string _unpack(int count, const char* begin, ...)
{
	std::string res;
	va_list arglist;
	va_start(arglist, begin);
	if (nullptr != begin)
	{
		res.append(begin);
	}
	for (int index = 0; index < count - 1; index++)
	{
		auto val = va_arg(arglist, decltype(begin));
		if (nullptr != val)
		{
			res.append(val);
		}
	}
	va_end(arglist);
	return res;
}

static const char* _getTypeName(Java::TypeEnum type)
{
	switch (type)
	{
	case Java::_Void: return "V";
	case Java::_Int: return "I";
	case Java::_Long:  return "J";
	case Java::_Object: return "Ljava/lang/Object;";
	}
	return "";
}

template<typename _Return, typename ...Args>
static std::string _getReflectionString()
{
	return ("(" + _unpack(sizeof...(Args), _getTypeName(Java::Type<Args>::Name)...) + ")" + _getTypeName(Java::Type<_Return>::Name));
}

namespace Java
{
	template<typename _Return, typename ...Args>
	struct StaticMethodInvoker {};
#define DECLARE_INVOKER(returnType, methodName) \
template<typename ...Args>\
struct StaticMethodInvoker<returnType, Args...>\
{\
	static returnType Invoke(JNIEnv* env, jclass clazz, jmethodID methodId, Args...args) { return env->methodName(clazz, methodId, args...); }\
}
	DECLARE_INVOKER(void, CallStaticVoidMethod);
	DECLARE_INVOKER(jint, CallStaticIntMethod);
	DECLARE_INVOKER(jbyte, CallStaticByteMethod);
	DECLARE_INVOKER(jchar, CallStaticCharMethod);
	DECLARE_INVOKER(jlong, CallStaticLongMethod);
	DECLARE_INVOKER(jfloat, CallStaticFloatMethod);
	DECLARE_INVOKER(jshort, CallStaticShortMethod);
	DECLARE_INVOKER(jdouble, CallStaticDoubleMethod);
	DECLARE_INVOKER(jboolean, CallStaticBooleanMethod);

	template<typename _Return, typename ...Args>
	class StaticMethod : public MethodBase
	{
	public:
		StaticMethod(JNIEnv* env, const std::string& className, const std::string& methodName) :
			MethodBase(env, nullptr),
			m_clazz(nullptr)
		{
			m_clazz = env->FindClass(className.c_str());
			if (nullptr != m_clazz)
			{
				m_methodId = env->GetStaticMethodID(m_clazz, methodName.c_str(), _getReflectionString<_Return, Args...>().c_str());
			}
		}

		~StaticMethod() = default;

		bool IsValid() const
		{
			return
				nullptr != m_env &&
				nullptr != m_methodId &&
				nullptr != m_clazz;
		}

		_Return Invoke(Args...args) const
		{
			return StaticMethodInvoker<_Return, Args...>::Invoke(m_env, m_clazz, m_methodId, args...);
		}

	protected:
		jclass m_clazz;
	};
}



