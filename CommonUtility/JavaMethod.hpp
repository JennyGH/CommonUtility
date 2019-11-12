#pragma once
#include <jni.h>
#include <string>

namespace Java
{
	static JavaVM* g_jvm = nullptr;
}

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

	DECLARE_TYPE(jint, _Int);
	DECLARE_TYPE(jlong, _Long);
	DECLARE_TYPE(void, _Void);
	DECLARE_TYPE(jobject, _Object);
}

namespace Java
{
	class MethodBase
	{
	public:
		MethodBase() {}
		~MethodBase() = default;
		virtual bool IsValid() const = 0;
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

static std::string _unpack(int count)
{
	return "";
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

namespace Java
{
	class JniEnviroment
	{
	public:
		JniEnviroment() : m_env(nullptr)
		{
			if (nullptr != g_jvm)
			{
				g_jvm->GetEnv((void**)&m_env, JNI_VERSION_1_8);
				g_jvm->AttachCurrentThread((void**)&m_env, nullptr);
			}
		}
		~JniEnviroment()
		{
			if (nullptr != g_jvm)
			{
				g_jvm->DetachCurrentThread();
			}
		}
		operator JNIEnv* ()
		{
			return m_env;
		}
	private:
		JNIEnv* m_env;
	};
}

namespace Java
{
	template<class _Invoker, typename _Caller, typename _Return, typename ...Args>
	class Method : public MethodBase
	{
	public:
		using Invoker = _Invoker;
	public:
		static const std::string Signature;
	public:
		Method(const std::string& className, const std::string& methodName, const std::string& sig) :
			MethodBase(),
			m_className(className),
			m_methodName(methodName),
			m_signature(sig)
		{}

		~Method() = default;

		bool IsValid() const
		{
			return !m_className.empty() && !m_methodName.empty() && !m_signature.empty();
		}

		_Return Invoke(Args...args) const
		{
			return _Invoker::Invoke(m_className, m_methodName, m_signature, args...);
		}

	protected:
		std::string m_className;
		std::string m_methodName;
		std::string m_signature;
	};
}
template<class _Invoker, typename _Caller, typename _Return, typename ...Args>
const std::string Java::Method<_Invoker, _Caller, _Return, Args...>::Signature =
("(" + _unpack(sizeof...(Args), _getTypeName(Java::Type<Args>::Name)...) + ")" + _getTypeName(Java::Type<_Return>::Name));

namespace Java
{
	template<typename _Return, typename ...Args>
	struct StaticMethodInvoker {};

#define DECLARE_INVOKER(returnType, functionName) \
template<typename ...Args>\
struct StaticMethodInvoker<returnType, Args...>\
{\
	static returnType Invoke(const std::string& className, const std::string& methodName, const std::string& sig, Args...args)\
	{\
		Java::JniEnviroment enviroment;\
		JNIEnv* env = enviroment;\
		auto clazz = env->FindClass(className.c_str());\
		auto methodId = env->GetStaticMethodID(clazz, methodName.c_str(), sig.c_str());\
		return env->functionName(clazz, methodId, args...);\
	}\
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

#undef DECLARE_INVOKER


	template<typename _Return, typename ...Args>
	class StaticMethod : public Method<StaticMethodInvoker<_Return, Args...>, jclass, _Return, Args...>
	{
		using BaseType = Method<StaticMethodInvoker<_Return, Args...>, jclass, _Return, Args...>;
	public:
		StaticMethod(const std::string& className, const std::string& methodName) :BaseType(className, methodName, BaseType::Signature) {}
	};
}

//
//namespace Java
//{
//	template<typename _Return, typename ...Args>
//	struct MemberMethodInvoker {};
//
//#define DECLARE_INVOKER(returnType, methodName) \
//template<typename ...Args>\
//struct MemberMethodInvoker<returnType, Args...>\
//{static returnType Invoke(JNIEnv* env, jobject caller, jmethodID methodId, Args...args) { return env->methodName(caller, methodId, args...); }}
//
//	DECLARE_INVOKER(void, CallVoidMethod);
//	DECLARE_INVOKER(jint, CallIntMethod);
//	DECLARE_INVOKER(jbyte, CallByteMethod);
//	DECLARE_INVOKER(jchar, CallCharMethod);
//	DECLARE_INVOKER(jlong, CallLongMethod);
//	DECLARE_INVOKER(jfloat, CallFloatMethod);
//	DECLARE_INVOKER(jshort, CallShortMethod);
//	DECLARE_INVOKER(jdouble, CallDoubleMethod);
//	DECLARE_INVOKER(jboolean, CallBooleanMethod);
//
//#undef DECLARE_INVOKER
//
//	template<typename _Return, typename ...Args>
//	class MemberMethod : public Method<MemberMethodInvoker<_Return, Args...>, jobject, _Return, Args...>
//	{
//	public:
//		MemberMethod(JNIEnv* env, jobject caller, const std::string& className, const std::string& methodName) :
//			Method<MemberMethodInvoker<_Return, Args...>, jobject, _Return, Args...>(env, className, methodName)
//		{
//			using BaseType = Method<MemberMethodInvoker<_Return, Args...>, jobject, _Return, Args...>;
//			BaseType::m_caller = caller;
//			auto clazz = BaseType::m_clazz;
//			if (nullptr != clazz)
//			{
//				Java::MethodBase::m_methodId = env->GetMethodID(clazz, methodName.c_str(), BaseType::Signature.c_str());
//			}
//		}
//	};
//}



