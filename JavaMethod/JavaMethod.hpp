#pragma once
#include <jni.h>
#include <string>
#include <memory>

#if defined(WIN32) || defined(_WIN32)
#define OS_WINDOWS 1
#endif // defined(WIN32) || defined(_WIN32)

#if OS_WINDOWS
#include <Windows.h>
#define CALL_STANDARD __stdcall
#else
#define CALL_STANDARD
#endif // defined(WIN32) || defined(_WIN32)

class DynamicLibraryLoader
{
	DynamicLibraryLoader(void* hInstance = nullptr) :m_hInstance(hInstance) {}
public:
	using DynamicLibraryLoaderPtr = std::shared_ptr<DynamicLibraryLoader>;
	static DynamicLibraryLoaderPtr Load(const std::string& libPath)
	{
#if OS_WINDOWS
		HINSTANCE hInstance = ::LoadLibraryExA(libPath.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
		if (nullptr == hInstance)
		{
			throw std::runtime_error(("Can not load dynamic library from " + libPath).c_str());
		}
		return DynamicLibraryLoaderPtr(new DynamicLibraryLoader(hInstance));
#endif // OS_WINDOWS
	}
public:

	~DynamicLibraryLoader()
	{
		if (nullptr != m_hInstance)
		{
			// MUST FREE m_hInstance !!!;
#if OS_WINDOWS
			::FreeLibrary((HINSTANCE)m_hInstance);
#endif // OS_WINDOWS

			m_hInstance = nullptr;
		}
	}

	template<typename ReturnType, typename ...Args>
	auto GetFunction(const std::string& functionName)
	{
		using FunctionType = ReturnType(CALL_STANDARD *)(Args...);
		FunctionType pFunction = nullptr;

		if (nullptr == this->m_hInstance)
		{
			throw std::runtime_error("DynamicLibraryLoader is not ready.");
		}

#if OS_WINDOWS
		pFunction = (FunctionType)::GetProcAddress((HINSTANCE)this->m_hInstance, functionName.c_str());
		if (nullptr == pFunction)
		{
			throw std::runtime_error(("Can not get function address from " + functionName).c_str());
		}
#else
		throw std::runtime_error("DynamicLibraryLoader is not supported in this platform.");
#endif // OS_WINDOWS

		return pFunction;
	}

private:
	void* m_hInstance;
};

namespace Java
{
	static JavaVM* g_jvm = nullptr;
	using JavaVMOptionPtr = std::shared_ptr<JavaVMOption>;
	using JavaVMInitArgsPtr = std::shared_ptr<JavaVMInitArgs>;
}

namespace Java
{
	enum TypeEnum
	{
		_Void = 0,
		_Int,
		_Long,
		_Object,
		_String
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
	DECLARE_TYPE(jstring, _String);
}

namespace Java
{
	class JniEnviroment;
	using JniEnviromentPtr = std::shared_ptr<JniEnviroment>;
	class JniEnviroment
	{
	public:
		static JniEnviromentPtr CreateJVMFromJar(const std::string& jvmPath, const std::string& jarPath, int jvmVersion = JNI_VERSION_1_8)
		{
			JavaVM* jvm = nullptr;
			JNIEnv* env = nullptr;
			JavaVMInitArgs vm_args;
			JavaVMOption options[3] = { 0 };
			std::string tmp = ("-Djava.class.path=.;" + jarPath);
			char* str = new char[tmp.length() + 1]();
			std::shared_ptr<char> scope_free_str(str);
			strcpy(str, tmp.c_str());
			options[0].optionString = "-Djava.compiler=NONE"; //Disabled JIT
			options[1].optionString = str;
			options[2].optionString = "-verbose:NONE";
			vm_args.version = jvmVersion;
			vm_args.nOptions = 3;
			vm_args.options = options;
			vm_args.ignoreUnrecognized = JNI_TRUE;

			auto pDLL = DynamicLibraryLoader::Load(jvmPath);
			auto pFunction = pDLL->GetFunction<jint, JavaVM **, void **, void *>("JNI_CreateJavaVM");
			long lRes = (*pFunction)(&jvm, (void**)&env, &vm_args);
			if (lRes < 0)
			{
				return nullptr;
			}
			auto ptr = std::make_shared<JniEnviroment>();
			ptr->m_jvm = jvm;
			ptr->m_env = env;
			return ptr;
		}
	public:
		JniEnviroment()
			: m_jvm(nullptr)
			, m_env(nullptr)
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
			if (nullptr != m_jvm)
			{
				m_jvm->DestroyJavaVM();
			}
		}
		JNIEnv* GetEnv()
		{
			return m_env;
		}
		operator JNIEnv* ()
		{
			return m_env;
		}
	private:
		JavaVM* m_jvm;
		JNIEnv* m_env;
	};
}

namespace Java
{
	class MethodBase
	{
	public:
		MethodBase(JniEnviromentPtr env = JniEnviromentPtr()) : m_env(env) {}
		~MethodBase() = default;
		virtual bool IsValid() const = 0;

	protected:
		JniEnviromentPtr m_env;
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
	case Java::_String: return "Ljava/lang/String;";
	}
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
			MethodBase(),
			m_className(className),
			m_methodName(methodName),
			m_signature(sig),
			m_object(nullptr)
		{}

		Method(const std::string& className, const std::string& methodName, const std::string& sig, JniEnviromentPtr env) :
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

		Method(const std::string& className, const std::string& methodName, const std::string& sig, jobject object, JniEnviromentPtr env) :
			MethodBase(env),
			m_className(className),
			m_methodName(methodName),
			m_signature(sig),
			m_object(object)
		{}

		~Method() = default;

		bool IsValid() const
		{
			return !m_className.empty() && !m_methodName.empty() && !m_signature.empty() && m_env != nullptr && m_env->GetEnv() != nullptr;
		}

		_Return operator ()(Args...args)
		{
			return this->Invoke(args...);
		}

		_Return Invoke(Args...args)
		{
			return _Invoker::Invoke(m_env->GetEnv(), m_object, m_className, m_methodName, m_signature, args...);
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
("(" + _unpack(sizeof...(Args), _getTypeName(Java::Type<Args>::Name)...) + ")" + _getTypeName(Java::Type<_Return>::Name));

namespace Java
{
	template<typename _Return, typename ...Args>
	struct StaticMethodInvoker {};

#define DECLARE_INVOKER(returnType, functionName) \
template<typename ...Args>\
struct StaticMethodInvoker<returnType, Args...>\
{\
	static returnType Invoke(JNIEnv* env, jobject object, const std::string& className, const std::string& methodName, const std::string& sig, Args...args)\
	{\
		if(nullptr == env)\
		{\
			throw std::runtime_error("JNI enviroment is not ready");\
		}\
		auto clazz = env->FindClass(className.c_str());\
		if(nullptr == clazz)\
		{\
			throw std::runtime_error(("Can not find class: " + className).c_str());\
		}\
		auto methodId = env->GetStaticMethodID(clazz, methodName.c_str(), sig.c_str());\
		if (nullptr == methodId)\
		{\
			throw std::runtime_error(("Can not find method " + methodName + " from class: " + className).c_str());\
		}\
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
		StaticMethod(const std::string& className, const std::string& methodName, JniEnviromentPtr env) :BaseType(className, methodName, BaseType::Signature, env) {}
	};
}


namespace Java
{
	template<typename _Return, typename ...Args>
	struct MemberMethodInvoker {};

#define DECLARE_INVOKER(returnType, functionName) \
template<typename ...Args>\
struct MemberMethodInvoker<returnType, Args...>\
{\
	static returnType Invoke(JNIEnv* env, jobject object, const std::string& className, const std::string& methodName, const std::string& sig, Args...args)\
	{\
		if(nullptr == env)\
		{\
			throw std::runtime_error("JNI enviroment is not ready");\
		}\
		if(nullptr == object)\
		{\
			throw std::runtime_error("The caller of java member function is null.");\
		}\
		auto clazz = env->FindClass(className.c_str());\
		if(nullptr == clazz)\
		{\
			throw std::runtime_error(("Can not find class: " + className).c_str());\
		}\
		jmethodID methodId = env->GetMethodID(clazz, methodName.c_str(), sig.c_str());\
		return env->functionName(clazz, methodId, args...);\
	}\
}

	DECLARE_INVOKER(void, CallVoidMethod);
	DECLARE_INVOKER(jint, CallIntMethod);
	DECLARE_INVOKER(jbyte, CallByteMethod);
	DECLARE_INVOKER(jchar, CallCharMethod);
	DECLARE_INVOKER(jlong, CallLongMethod);
	DECLARE_INVOKER(jfloat, CallFloatMethod);
	DECLARE_INVOKER(jshort, CallShortMethod);
	DECLARE_INVOKER(jdouble, CallDoubleMethod);
	DECLARE_INVOKER(jboolean, CallBooleanMethod);

#undef DECLARE_INVOKER

	template<typename _Return, typename ...Args>
	class MemberMethod : public Method<MemberMethodInvoker<_Return, Args...>, jobject, _Return, Args...>
	{
		using BaseType = Method<MemberMethodInvoker<_Return, Args...>, jobject, _Return, Args...>;
	public:
		MemberMethod(jobject object, const std::string& className, const std::string& methodName) : Method<MemberMethodInvoker<_Return, Args...>, jobject, _Return, Args...>(className, methodName, BaseType::Signature, object) {}
		MemberMethod(jobject object, const std::string& className, const std::string& methodName, JniEnviromentPtr env) : Method<MemberMethodInvoker<_Return, Args...>, jobject, _Return, Args...>(className, methodName, BaseType::Signature, object, env) {}
	};
}



