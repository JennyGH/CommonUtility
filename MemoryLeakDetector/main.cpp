#include <string>
#include <vector>
#include "MemoryLeakDetector.h"

class TestClass
{
public:
	TestClass() :
		m_ptr(new char[10]())
	{
	}
	~TestClass()
	{
		if (nullptr != m_ptr)
		{
			delete[] m_ptr;
			m_ptr = nullptr;
		}
	}

private:
	char* m_ptr;
};


int main(int argc, char *argv[])
{
	MemoryLeakDetector& memoryLeakDetector = MemoryLeakDetector::GetGlobalDetector();

	size_t size = 100;

	TestClass** ptrs = new TestClass*[size]();

	for (size_t i = 0; i < size; i++)
	{
		ptrs[i] = (new TestClass());
	}

	//for (size_t i = 0; i < size; i++)
	//{
	//	delete ptrs[i];
	//	ptrs[i] = nullptr;
	//}

	//#pragma push_macro("new")
	//#undef new
		//TestClass* ptr = new(ptrs) TestClass();
	//#pragma pop_macro("new")

	//delete[] ptrs;
	ptrs = nullptr;

	return 0;
}