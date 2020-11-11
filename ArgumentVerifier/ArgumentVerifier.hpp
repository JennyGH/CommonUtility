#pragma once
#include <string>

template<typename T>
static std::string SerializeMemory(const T* in, int size = sizeof(T))
{
	return "This is pointer of any type.";
}
template<>
static std::string SerializeMemory(const unsigned char* in, int size)
{
	return "This is bytes.";
}
template<typename T>
static void DeserializeMemory(const std::string& in, T* out, unsigned int* outSize)
{
}
template<>
static void DeserializeMemory(const std::string& in, unsigned char* out, unsigned int* outSize)
{
}
template<typename ... Args>
static std::string TypeConvert(Args... args)
{
	return SerializeMemory(args...);
}
template<typename From>
static typename std::enable_if<std::is_pointer<From>::value, std::string>::type TypeConvert(From from)
{
	return SerializeMemory<std::remove_pointer<From>::type>(from);
}
template<typename From>
static typename std::enable_if<!std::is_pointer<From>::value, From>::type TypeConvert(From from)
{
	return from;
}
template<typename T>
struct ArgumentVerifier
{
	static bool verify(const T& arg) { return 0 != arg; }
};
template<>
struct ArgumentVerifier<int>
{
	static bool verify(const int& arg) { return INT_MIN < arg&& arg < INT_MAX; }
};
template<>
struct ArgumentVerifier<unsigned int>
{
	static bool verify(const unsigned int& arg) { return arg < UINT_MAX; }
};
template<typename ... Args>
static bool VerifyArgument(const Args& ...args)
{
	return true;
}
template<typename T, typename... Rest>
static bool VerifyArgument(const T& arg, const Rest& ...rest)
{
	return ArgumentVerifier<T>::verify(arg) && VerifyArgument<Rest...>(rest...);
}

#define CHECK_INPUT_ARGUMENTS(...) VerifyArgument(__VA_ARGS__)
#define INPUT_ARGUMENT(json, arg, ...) (json[#arg] = TypeConvert(arg, ##__VA_ARGS__))