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
        if (NULL != m_ptr)
        {
            delete[] m_ptr;
            m_ptr = NULL;
        }
    }

private:
    char* m_ptr;
};


int main(int argc, char *argv[])
{
    size_t size = 3;

    TestClass** ptrs = new TestClass*[size]();

    for (size_t i = 0; i < size; i++)
    {
        ptrs[i] = (new TestClass());
    }

    delete ptrs;
    ptrs = NULL;

    return 0;
}