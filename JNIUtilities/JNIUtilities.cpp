#include "JNIUtilities.h"
#include <codecvt>

static std::string AsciiToUtf8String(const std::string & ascii)
{
	using convertor_type_1 = std::codecvt_byname<wchar_t, char, std::mbstate_t>;
	using convertor_type_2 = std::codecvt_utf8<wchar_t>;
	static std::wstring_convert<convertor_type_1> convertor_1(new convertor_type_1("zh-cn"));
	static std::wstring_convert<convertor_type_2> convertor_2;
	auto wstr = convertor_1.from_bytes(ascii);
	return convertor_2.to_bytes(wstr);
}

jbyteArray Java::ToJBytes(JNIEnv * env, const uint8_t data[], int64_t len)
{
	if (data == nullptr || len <= 0)
	{
		return nullptr;
	}
	jbyte *by = (jbyte*)data;
	jbyteArray jarray = env->NewByteArray(len);
	env->SetByteArrayRegion(jarray, 0, len, by);
	return jarray;
}

Java::ByteArray Java::FromJBytes(JNIEnv * env, jbyteArray jbytes, jint offset)
{
	if (nullptr == jbytes)
	{
		return Java::ByteArray();
	}

	jint len = env->GetArrayLength(jbytes);
	if (offset > len - 1)
	{
		return Java::ByteArray();
	}

	uint8_t* cbytes = (uint8_t*)(env->GetByteArrayElements(jbytes, 0));
	return Java::ByteArray(cbytes, cbytes + offset + len);
}

jstring Java::ToJString(JNIEnv * env, const std::string & str)
{
	return env->NewStringUTF(AsciiToUtf8String(str).c_str());
}

std::string Java::FromJString(JNIEnv * env, jstring jstr)
{
	jsize len = env->GetStringUTFLength(jstr);
	const char *str = env->GetStringUTFChars(jstr, 0);
	if (0 == len || nullptr == str)
	{
		return "";
	}
	return std::string(str, len);
}
