#pragma once
#include <jni.h>
#include <map>
#include <string>

enum JavaTypeName
{
	_Void = 0,
	_Int,
	_Long,
	_Object
};
static std::map<JavaTypeName, const char*> g_javaTypeNameMap = {
	{_Void,			"V"},
	{_Int,			"I"},
	{_Long,			"J"},
	{_Object,		"Ljava/lang/Object;"},
};

template<typename T>
struct JavaType { };
template<>
struct JavaType<int> { static const JavaTypeName Name = _Int; };
template<>
struct JavaType<long> { static const JavaTypeName Name = _Long; };
template<>
struct JavaType<void> { static const JavaTypeName Name = _Void; };
template<>
struct JavaType<jobject> { static const JavaTypeName Name = _Object; };

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

class JavaMethod
{
public:
	JavaMethod(JNIEnv* env, jmethodID methodId) :
		m_env(env),
		m_methodId(methodId)
	{}
	~JavaMethod() = default;

	virtual bool IsValid() const = 0;

protected:
	JNIEnv* m_env;
	jmethodID m_methodId;
};

template<typename _Return, typename ...Args>
class JavaStaticMethod : public JavaMethod
{
public:
	JavaStaticMethod(JNIEnv* env, const std::string& className, const std::string& methodName) :
		JavaMethod(env, nullptr),
		m_clazz(env->FindClass(className.c_str())),
		m_methodSignature("(" + _unpack(sizeof...(Args), g_javaTypeNameMap[JavaType<Args>::Name]...) + ")" + g_javaTypeNameMap[JavaType<_Return>::Name])
	{
		if (nullptr != m_clazz)
		{
			m_methodId = env->GetStaticMethodID(m_clazz, methodName.c_str(), m_methodSignature.c_str());
		}
	}

	~JavaStaticMethod() = default;

	bool IsValid() const
	{
		return
			nullptr != m_env &&
			nullptr != m_clazz &&
			nullptr != m_methodId &&
			!m_methodSignature.empty();

	}

	void Invoke(Args...args) const
	{
		if (IsValid())
		{
			m_env->CallStaticVoidMethod(m_clazz, m_methodId, args...);
		}
	}
private:
	jclass m_clazz;
	std::string m_methodSignature;
};

