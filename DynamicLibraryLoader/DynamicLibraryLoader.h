#pragma once
#include <string> // std::string
#include <memory> // std::shared_ptr

#if (defined(WIN32) || defined(_WIN32)) && !defined(OS_WINDOWS)
#define OS_WINDOWS 1
#endif // defined(WIN32) || defined(_WIN32)

#if OS_WINDOWS
#include <Windows.h>
#define CALL_STANDARD __stdcall
#else
#include<dlfcn.h>
#define CALL_STANDARD
#endif // OS_WINDOWS

class DynamicLibraryLoader;
using DynamicLibraryLoaderPtr = std::shared_ptr<DynamicLibraryLoader>;
class DynamicLibraryLoader
{
	DynamicLibraryLoader(void* hInstance = nullptr);
public:
	static DynamicLibraryLoaderPtr Load(const std::string& libPath);
public:
	~DynamicLibraryLoader();

	template<typename ReturnType, typename ...Args>
	auto GetFunction(const std::string& functionName)
	{
		using FunctionType = ReturnType(CALL_STANDARD *)(Args...);
		FunctionType pFunction = nullptr;

		if (nullptr == m_hDLL)
		{
			throw std::runtime_error("DynamicLibraryLoader is not ready.");
		}

#if OS_WINDOWS
		pFunction = (FunctionType)::GetProcAddress((HINSTANCE)m_hDLL, functionName.c_str());
#else
		//throw std::runtime_error("DynamicLibraryLoader is not supported in this platform.");
		pFunction = (FunctionType)::dlsym(hDLL, functionName.c_str());
#endif // OS_WINDOWS

		if (nullptr == pFunction)
		{
			throw std::runtime_error(("Can not get function address from " + functionName).c_str());
		}

		return pFunction;
	}

private:
	void* m_hDLL;
};