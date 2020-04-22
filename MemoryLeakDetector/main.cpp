#include <string>
#include <vector>
#include <thread>
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
	size_t size = 3;

	auto thread_function = [size]()
	{
		TestClass** ptrs = new TestClass*[size]();

		for (size_t i = 0; i < size; i++)
		{
			ptrs[i] = (new TestClass());
		}

		for (size_t i = 0; i < size; i++)
		{
			//delete ptrs[i];
			ptrs[i] = nullptr;
		}

		delete[] ptrs;
		ptrs = nullptr;
	};


	std::vector<std::thread> threads;

	threads.emplace_back(thread_function);
	threads.emplace_back(thread_function);
	threads.emplace_back(thread_function);

	for (auto& thread : threads)
	{
		thread.detach();
	}

	TestClass** ptrs = new TestClass*[size]();

	for (size_t i = 0; i < size; i++)
	{
		ptrs[i] = (new TestClass());
	}

	for (size_t i = 0; i < size; i++)
	{
		delete ptrs[i];
		ptrs[i] = nullptr;
	}

	delete[] ptrs;
	ptrs = nullptr;

	std::this_thread::sleep_for(std::chrono::seconds(5));

	return 0;
}