#include <ctime>
#include <iostream>
#include "Path.h"

int main(int argc, char *argv[])
{
	Path path = "C:";
	//path.CreatePath(true);
	//path.Rename("3");
	std::size_t count = 0;
	time_t begin = time(NULL);
	path.GetSubPaths("*.*", [](void* context, const Path& path) {
		(*(std::size_t*)context)++;
		std::cout << std::string(path) << std::endl;
	}, &count, true);
	time_t end = time(NULL);
	std::cout << "Get sub paths completed, path count: " << count << ", cost time: " << end - begin << "s" << std::endl;
	//for (const auto& subPath : subPaths)
	//{
	//	std::cout << std::string(subPath) << std::endl;
	//}
	std::cout
		<< "Current path: " << std::string(Path::GetCurrentPath()) << std::endl
		<< "Home path: " << std::string(Path::GetHomePath()) << std::endl
		<< "path: " << std::string(path) << std::endl
		<< ", is dir: " << path.IsDirectory() << std::endl
		<< ", is file: " << path.IsFile() << std::endl
		<< ", file name: " << path.GetName() << std::endl
		<< ", file suffix: " << path.GetSuffix() << std::endl
		<< std::endl;
	return 0;
}