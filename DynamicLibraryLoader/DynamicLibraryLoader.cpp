#include "DynamicLibraryLoader.h"
#include <stdexcept>
#if __cplusplus >= 201103L
DynamicLibraryLoader::DynamicLibraryLoader(void * hInstance) :m_hDLL(hInstance) {}

DynamicLibraryLoaderPtr DynamicLibraryLoader::Load(const std::string & libPath)
{
    void* hDLL = nullptr;
#if OS_WINDOWS
    hDLL = ::LoadLibraryExA(libPath.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
#else
    hDLL = ::dlopen(libPath.c_str(), RTLD_LAZY);
#endif // OS_WINDOWS
    if (nullptr == hDLL)
    {
        throw std::runtime_error(("Can not load dynamic library from " + libPath).c_str());
    }
    return DynamicLibraryLoaderPtr(new DynamicLibraryLoader(hDLL));
}

DynamicLibraryLoader::~DynamicLibraryLoader()
{
    if (nullptr != m_hDLL)
    {
        // MUST FREE m_hDLL !!!;
#if OS_WINDOWS
        ::FreeLibrary((HMODULE)m_hDLL);
#else
        ::dlclose(m_hDLL);
#endif // OS_WINDOWS

        m_hDLL = nullptr;
    }
}
#endif // __cplusplus >= 201103L