#pragma once
#include <jni.h>
#include <string>

namespace Java
{
	using IntArray = std::basic_string<int32_t>;
	using ByteArray = std::basic_string<uint8_t>;
	jbyteArray ToJBytes(JNIEnv * env, const uint8_t data[], int64_t len);
	ByteArray FromJBytes(JNIEnv * env, jbyteArray jbytes, jint offset = 0);
	jstring ToJString(JNIEnv * env, const std::string& str);
	std::string FromJString(JNIEnv * env, jstring jstr);
}

