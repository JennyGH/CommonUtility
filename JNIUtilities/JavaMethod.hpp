#pragma once
#include <jni.h>
#include <string>
#include <memory>

#define DECLARE_JAVA_CLASS_REFLECTION(type, typeName)\
namespace Java\
{\
	template<>\
	struct TypeReflection<type>\
	{\
		static const char* GetTypeName() { return "L" typeName ";"; }\
	};\
}\

namespace Java
{
	template<typename T>
	struct TypeReflection { static const char* GetTypeName() { return "Ljava/lang/Object;"; } }; // Default java type reflection.
#define SPECIALIZED_TYPE_TEMPLATE(jtype, typeName) \
template<>\
struct TypeReflection<jtype> { static const char* GetTypeName() { return typeName; } }\

#define SPECIALIZED_ARRAY_TEMPLATE(jtype, typeName) \
SPECIALIZED_TYPE_TEMPLATE(jtype, typeName);\
template<>\
struct TypeReflection<jtype##Array> { static const char* GetTypeName() { return "[" typeName; } }\

	/* ************************ Specialize templates ************************ */
	SPECIALIZED_TYPE_TEMPLATE(void, /*******/ "V");
	SPECIALIZED_TYPE_TEMPLATE(jstring, /****/ "Ljava/lang/String;");
	SPECIALIZED_ARRAY_TEMPLATE(jint, /******/ "I");
	SPECIALIZED_ARRAY_TEMPLATE(jlong, /*****/ "J");
	SPECIALIZED_ARRAY_TEMPLATE(jboolean, /**/ "Z");
	SPECIALIZED_ARRAY_TEMPLATE(jbyte, /*****/ "B");
	SPECIALIZED_ARRAY_TEMPLATE(jchar, /*****/ "C");
	SPECIALIZED_ARRAY_TEMPLATE(jshort, /****/ "S");
	SPECIALIZED_ARRAY_TEMPLATE(jfloat, /*****/"F");
	SPECIALIZED_ARRAY_TEMPLATE(jdouble, /***/ "D");
	/* *********************************************************************** */

#undef SPECIALIZED_ARRAY_TEMPLATE
#undef SPECIALIZED_TYPE_TEMPLATE
}

namespace Java
{
	class MethodBase
	{
	public:
		MethodBase(JNIEnv* env) : m_env(env) {}
		~MethodBase() = default;
		virtual bool IsValid() const = 0;

	protected:
		JNIEnv* m_env;
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
			m_className(className),
			m_methodName(methodName),
			m_signature(sig),
			m_object(nullptr)
		{}

		Method(const std::string& className, const std::string& methodName, const std::string& sig, JNIEnv* env) :
			MethodBase(env),
			m_className(className),
			m_methodName(methodName),
			m_signature(sig),
			m_object(nullptr)
		{}

		Method(const std::string& className, const std::string& methodName, const std::string& sig, jobject object) :
			MethodBase(),
			m_className(className),
			m_methodName(methodName),
			m_signature(sig),
			m_object(object)
		{}

		Method(const std::string& className, const std::string& methodName, const std::string& sig, jobject object, JNIEnv* env) :
			MethodBase(env),
			m_className(className),
			m_methodName(methodName),
			m_signature(sig),
			m_object(object)
		{}

		~Method() = default;

		bool IsValid() const
		{
			return !m_className.empty() && !m_methodName.empty() && !m_signature.empty() && nullptr != m_env;
		}

		_Return operator ()(Args...args)
		{
			return this->Invoke(args...);
		}

		_Return Invoke(Args...args)
		{
			return _Invoker::Invoke(m_env, m_object, m_className, m_methodName, m_signature, args...);
		}

	protected:
		std::string m_className;
		std::string m_methodName;
		std::string m_signature;
		jobject     m_object;
	};
}

template<class _Invoker, typename _Caller, typename _Return, typename ...Args>
const std::string Java::Method<_Invoker, _Caller, _Return, Args...>::Signature =
("(" + _unpack(sizeof...(Args), Java::TypeReflection<Args>::GetTypeName()...) + ")" + Java::TypeReflection<_Return>::GetTypeName());

namespace Java
{
#define DECLARE_INVOKE_FUNCTION(returnType, functionName) \
static returnType Invoke(JNIEnv* env, jobject object, const std::string& className, const std::string& methodName, const std::string& sig, Args...args)\
{\
	if(nullptr == env)\
	{\
		throw std::runtime_error("JNI enviroment is not ready");\
	}\
	auto clazz = env->FindClass(className.c_str());\
	if(nullptr == clazz)\
	{\
		if (env->ExceptionCheck())\
		{\
			env->ExceptionDescribe();\
			env->ExceptionClear();\
		}\
		throw std::runtime_error(("Can not find class: " + className).c_str());\
	}\
	auto methodId = env->GetStaticMethodID(clazz, methodName.c_str(), sig.c_str());\
	if (nullptr == methodId)\
	{\
		if (env->ExceptionCheck())\
		{\
			env->ExceptionDescribe();\
			env->ExceptionClear();\
		}\
		throw std::runtime_error(("Can not find static method \"" + methodName + "\" in class: \"" + className + "\", sig: " + sig).c_str());\
	}\
	return env->functionName(clazz, methodId, args...);\
}\

	template<typename _Return, typename ...Args>
	struct StaticMethodInvoker
	{
		DECLARE_INVOKE_FUNCTION(_Return, CallStaticObjectMethod)
	};

#define DECLARE_INVOKER(returnType, functionName) \
template<typename ...Args>\
struct StaticMethodInvoker<returnType, Args...>\
{\
	DECLARE_INVOKE_FUNCTION(returnType, functionName)\
}

	DECLARE_INVOKER(void, CallStaticVoidMethod);
	DECLARE_INVOKER(jint, CallStaticIntMethod);
	DECLARE_INVOKER(jbyte, CallStaticByteMethod);
	DECLARE_INVOKER(jchar, CallStaticCharMethod);
	DECLARE_INVOKER(jlong, CallStaticLongMethod);
	DECLARE_INVOKER(jfloat, CallStaticFloatMethod);
	DECLARE_INVOKER(jshort, CallStaticShortMethod);
	DECLARE_INVOKER(jdouble, CallStaticDoubleMethod);
	DECLARE_INVOKER(jobject, CallStaticObjectMethod);
	DECLARE_INVOKER(jboolean, CallStaticBooleanMethod);

#undef DECLARE_INVOKER
#undef DECLARE_INVOKE_FUNCTION


	template<typename _Return, typename ...Args>
	class StaticMethod : public Method<StaticMethodInvoker<_Return, Args...>, jclass, _Return, Args...>
	{
		using BaseType = Method<StaticMethodInvoker<_Return, Args...>, jclass, _Return, Args...>;
	public:
		StaticMethod(const std::string& className, const std::string& methodName) :
			BaseType(className, methodName, BaseType::Signature)
		{}
		StaticMethod(const std::string& className, const std::string& methodName, JNIEnv* env) :
			BaseType(className, methodName, BaseType::Signature, env)
		{}
	};
}


namespace Java
{
#define DECLARE_INVOKE_FUNCTION(returnType, functionName)\
static returnType Invoke(JNIEnv* env, jobject object, const std::string& className, const std::string& methodName, const std::string& sig, Args...args)\
{\
	if(nullptr == env)\
	{\
		throw std::runtime_error("JNI enviroment is not ready");\
	}\
	if(nullptr == object)\
	{\
		if (env->ExceptionCheck())\
		{\
			env->ExceptionDescribe();\
			env->ExceptionClear();\
		}\
		throw std::runtime_error("The caller of java member function is null.");\
	}\
	auto clazz = env->FindClass(className.c_str());\
	if(nullptr == clazz)\
	{\
		if (env->ExceptionCheck())\
		{\
			env->ExceptionDescribe();\
			env->ExceptionClear();\
		}\
		throw std::runtime_error(("Can not find class: " + className).c_str());\
	}\
	auto methodId = env->GetMethodID(clazz, methodName.c_str(), sig.c_str());\
	if(nullptr == methodId)\
	{\
		if (env->ExceptionCheck())\
		{\
			env->ExceptionDescribe();\
			env->ExceptionClear();\
		}\
		throw std::runtime_error(("Can not find member method \"" + methodName + "\" in class \"" + className + "\", sig: " + sig).c_str());\
	}\
	return env->functionName(clazz, methodId, args...);\
}\

	template<typename _Return, typename ...Args>
	struct MemberMethodInvoker {
		DECLARE_INVOKE_FUNCTION(_Return, CallObjectMethod)
	};

#define DECLARE_INVOKER(returnType, functionName) \
template<typename ...Args>\
struct MemberMethodInvoker<returnType, Args...>\
{\
	DECLARE_INVOKE_FUNCTION(returnType, functionName)\
}

	DECLARE_INVOKER(void, CallVoidMethod);
	DECLARE_INVOKER(jint, CallIntMethod);
	DECLARE_INVOKER(jbyte, CallByteMethod);
	DECLARE_INVOKER(jchar, CallCharMethod);
	DECLARE_INVOKER(jlong, CallLongMethod);
	DECLARE_INVOKER(jfloat, CallFloatMethod);
	DECLARE_INVOKER(jshort, CallShortMethod);
	DECLARE_INVOKER(jdouble, CallDoubleMethod);
	DECLARE_INVOKER(jobject, CallObjectMethod);
	DECLARE_INVOKER(jboolean, CallBooleanMethod);

#undef DECLARE_INVOKER

	template<typename _Return, typename ...Args>
	class MemberMethod : public Method<MemberMethodInvoker<_Return, Args...>, jobject, _Return, Args...>
	{
		using BaseType = Method<MemberMethodInvoker<_Return, Args...>, jobject, _Return, Args...>;
	public:
		MemberMethod(jobject object, const std::string& className, const std::string& methodName) :
			Method<MemberMethodInvoker<_Return, Args...>, jobject, _Return, Args...>(className, methodName, BaseType::Signature, object)
		{}
		MemberMethod(jobject object, const std::string& className, const std::string& methodName, JNIEnv* env) :
			Method<MemberMethodInvoker<_Return, Args...>, jobject, _Return, Args...>(className, methodName, BaseType::Signature, object, env)
		{}
	};
}



