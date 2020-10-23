#pragma once
#include <jni.h>
#include <string>
#include <vector>
namespace Java
{
	using JavaVMPtr = std::shared_ptr<JavaVM>;
	using JNIEnvPtr = std::shared_ptr<JNIEnv>;
	using JavaVMOptionPtr = std::shared_ptr<JavaVMOption>;
	using JavaVMInitArgsPtr = std::shared_ptr<JavaVMInitArgs>;
	class JNIEnviroment
	{
	public:
		static JNIEnviroment GetEnviromentFromJVM(
			JavaVM* jvm,
			int jvmVersion = JNI_VERSION_1_8);
		static JNIEnviroment GetEnviromentFromJAR(
			const std::vector<std::string>& jarPaths,
			const std::vector<std::string>& libPaths = {},
			int jvmVersion = JNI_VERSION_1_8);
	public:
		JNIEnviroment();
		~JNIEnviroment();
		operator JNIEnv*();
		JNIEnvPtr operator->();
		bool IsValid() const;
	private:
		JavaVMPtr m_jvm;
		JNIEnvPtr m_env;
	};
}
