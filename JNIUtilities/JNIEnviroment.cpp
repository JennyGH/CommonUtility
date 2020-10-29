#include "JNIEnviroment.h"
#include <DynamicLibraryLoader.h>

static std::string _GetJVMDLLPath()
{
	const char* JAVA_HOME = ::getenv("JAVA_HOME");
	if (nullptr == JAVA_HOME)
	{
		return "";
	}
	std::string res(JAVA_HOME);
#if OS_WINDOWS
	res.append("\\jre\\bin\\server\\jvm.dll");
#else
	res.append("/jre/lib/amd64/server/libjvm.so");
#endif // OS_WINDOWS

	return res;
}

class OptionString
{
public:
	OptionString(const std::string& optionString)
		: m_data(nullptr)
		, m_length(optionString.length())
	{
		if (!optionString.empty())
		{
			this->m_data = new char[optionString.length() + 1]();
			::strcpy(this->m_data, optionString.c_str());
		}
	}
	~OptionString()
	{
		if (nullptr != this->m_data)
		{
			delete[] this->m_data;
			this->m_data = nullptr;
		}
	}

	operator char*()
	{
		return this->m_data;
	}

private:
	char*         m_data;
	std::size_t   m_length;
};

Java::JNIEnviroment Java::JNIEnviroment::GetEnviromentFromJVM(JavaVM * jvm, int jvmVersion)
{
	Java::JNIEnviroment ret;
	JNIEnv* env = nullptr;
	if (nullptr != jvm)
	{
		jvm->GetEnv((void**)&env, jvmVersion);
		jvm->AttachCurrentThread((void**)&env, nullptr);
	}
	ret.m_jvm.reset(jvm, [](JavaVM* ptr) { /* Do nothing. */ });
	ret.m_env.reset(env, [jvm](JNIEnv* ptr) { jvm->DetachCurrentThread(); });
	return ret;
}

Java::JNIEnviroment Java::JNIEnviroment::GetEnviromentFromJAR(
	const std::vector<std::string>& jarPaths,
	const std::vector<std::string>& libPaths,
	int jvmVersion)
{
	JNIEnviroment ret;
	JavaVM* jvm = nullptr;
	JNIEnv* env = nullptr;
	JavaVMInitArgs vm_args;
	std::vector<JavaVMOption> options;

	std::string classPaths = "-Djava.class.path=.";
	for (const auto& jarPath : jarPaths)
	{
		classPaths.append(";").append(jarPath);
	}
	OptionString classPathsOptionString(classPaths);

	std::string libraryPaths = "";
	if (!libPaths.empty())
	{
		libraryPaths = "-Djava.library.path=";
	}
	for (const auto& libPath : libPaths)
	{
		libraryPaths.append(libPath).append(";");
	}
	OptionString libraryPathsOptionString(libraryPaths);

	options.push_back(JavaVMOption{ "-Djava.compiler=NONE" });	 // disable JIT
	options.push_back(JavaVMOption{ classPathsOptionString });   // set class path
	options.push_back(JavaVMOption{ "-verbose:NONE" });
	if (!libraryPaths.empty())
	{
		options.push_back(JavaVMOption{ libraryPathsOptionString }); // set native library path
	}

	vm_args.version = jvmVersion;
	vm_args.nOptions = options.size();
	vm_args.options = options.data();
	vm_args.ignoreUnrecognized = JNI_TRUE;

	auto pDLL = DynamicLibraryLoader::Load(_GetJVMDLLPath());
	auto JNI_CreateJavaVM = pDLL->GetFunction<jint, JavaVM **, void **, void *>("JNI_CreateJavaVM");
	long lRes = JNI_CreateJavaVM(&jvm, (void**)&env, &vm_args);
	if (lRes < 0)
	{
		return ret;
	}
	ret.m_jvm.reset(jvm, [](JavaVM* ptr) { if (nullptr != ptr) { ptr->DestroyJavaVM(); } });
	ret.m_env.reset(env, [](JNIEnv* ptr) { /* Do nothing. */ });
	return ret;
}

Java::JNIEnviroment::JNIEnviroment()
	: m_jvm(nullptr)
	, m_env(nullptr)
{
	//if (nullptr != g_jvm)
	//{
	//	g_jvm->GetEnv((void**)&m_env, JNI_VERSION_1_8);
	//	g_jvm->AttachCurrentThread((void**)&m_env, nullptr);
	//}
}

Java::JNIEnviroment::~JNIEnviroment()
{
	//if (nullptr != g_jvm)
	//{
	//	g_jvm->DetachCurrentThread();
	//}
}

Java::JNIEnviroment::operator JNIEnv*()
{
	return m_env.get();
}

Java::JNIEnvPtr Java::JNIEnviroment::operator->()
{
	return m_env;
}

bool Java::JNIEnviroment::IsValid() const
{
	return m_env != nullptr;
}
