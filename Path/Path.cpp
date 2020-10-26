#include "Path.h"
#include <list>
#include <regex>
#include <codecvt>
#include <io.h>
#include <direct.h>

#if defined(WIN32) || defined(_WIN32)
#define HOME_ENV "USERPROFILE"
#define PATH_DIVIDER '\\'
#define WRONG_PATH_DIVIDER '/'
#define LPATH_DIVIDER L'\\'
#define LWRONG_PATH_DIVIDER L'/'
#else
#define HOME_ENV "HOME"
#define PATH_DIVIDER '/'
#define WRONG_PATH_DIVIDER '\\'
#define LPATH_DIVIDER L'/'
#define LWRONG_PATH_DIVIDER L'\\'
#endif // defined(WIN32) || defined(_WIN32)

using gbk_convertor_type = std::codecvt_byname<wchar_t, char, std::mbstate_t>;
static std::string wstring_to_string(const std::wstring & str, const std::string & locale = "chs")
{
	static std::wstring_convert<gbk_convertor_type> convertor(new gbk_convertor_type(locale));
	return convertor.to_bytes(str);
}

static std::wstring string_to_wstring(const std::string & str, const std::string & locale = "chs")
{
	static std::wstring_convert<gbk_convertor_type> convertor(new gbk_convertor_type(locale));
	return convertor.from_bytes(str);
}

static std::list<std::string> SplitString(const std::string& src, const std::string& separator)
{
	std::list<std::string> output;
	typename std::string::size_type current = 0;
	typename std::string::size_type end = -1;
	typename std::string::size_type len = separator.length();
	while (true)
	{
		end = src.find(separator, current);
		if (end == std::string::npos)
		{
			if (current < src.length())
			{
				output.push_back(src.substr(current));
			}
			break;
		}
		else
		{
			if (end >= current)
			{
				output.push_back(src.substr(current, end - current));
			}
			current = end + len;
		}
	}

	if (src.find_last_of(separator) == src.length() - separator.length())
	{
		output.push_back("");
	}

	return output;
}

static std::string ReplaceString(const std::string& src, const std::string& from, const std::string& to)
{
	std::string res = src;
	typedef std::list<std::string> split_res_t;
	typename std::string::size_type total = 0;
	split_res_t split_res = SplitString(src, from);
	for (typename split_res_t::const_iterator iter = split_res.begin(); iter != split_res.end(); ++iter)
	{
		total += iter->length();
	}
	total += ((split_res.size() - 1) * to.length());
	res.clear();
	res.reserve(total + 1);
	for (typename split_res_t::const_iterator iter = split_res.begin(); iter != split_res.end(); ++iter)
	{
		if (iter != split_res.begin())
		{
			res += to;
		}
		res += *iter;
	}
	return res;
}

static std::regex PatternToRegex(const std::string& pattern)
{
	std::string tmp = pattern;
	tmp = ReplaceString(tmp, ".", "\\.");
	tmp = ReplaceString(tmp, "*", "(.*)");
	return std::regex(tmp);
}

static bool IsPatternMatched(const std::string& input, const std::regex& regex)
{
	try
	{
		return std::regex_match(input, regex);
	}
	catch (const std::exception& ex)
	{
		printf(ex.what());
		return false;
	}
}

static void ReplacePathDivider(const std::string& path, std::vector<std::string>& splited)
{
	if (path.empty())
	{
		return;
	}
	auto wstr = string_to_wstring(path);
	auto length = wstr.length();
	std::size_t offset = 0;
	for (std::size_t index = 0; index < length; index++)
	{
		if (wstr[index] == LWRONG_PATH_DIVIDER || wstr[index] == LPATH_DIVIDER)
		{
			auto sub = wstr.substr(offset, index - offset);
			if (!sub.empty())
			{
				splited.push_back(wstring_to_string(sub));
			}
			offset = index + 1;
		}
	}
	auto sub = wstr.substr(offset);
	if (!sub.empty())
	{
		splited.push_back(wstring_to_string(sub));
	}
}

static void WalkDirectory(const Path& path, const std::regex& regex, Path::GetSubPathsCallback callback, void* callbackContext, bool getDir, bool rescured = false)
{
	if (!path.Exists())
	{
		return;
	}
	intptr_t handle = 0;
	_finddata_t findData;
	memset(&findData, 0, sizeof(findData));
	std::string searchPath = path.Join("*");
	handle = _findfirst(searchPath.c_str(), &findData);
	if (handle == -1)
	{
		return;
	}
	do
	{
		Path currentPath = path.Join(findData.name);
		if (findData.attrib & _A_SUBDIR)
		{
			if (strcmp(findData.name, ".") != 0 && strcmp(findData.name, "..") != 0)
			{
				if (getDir)
				{
					callback(callbackContext, currentPath);
				}
				if (rescured)
				{
					WalkDirectory(currentPath, regex, callback, callbackContext, getDir, rescured);
				}
			}
		}
		else
		{
			if (IsPatternMatched(findData.name, regex))
			{
				callback(callbackContext, currentPath);
			}
		}
	} while (_findnext(handle, &findData) == 0);
	_findclose(handle);
}

Path Path::GetHomePath()
{
	const char* homePath = ::getenv(HOME_ENV);
	if (nullptr == homePath)
	{
		return "";
	}
	return homePath;
}

Path Path::GetCurrentPath()
{
	char buffer[1024] = { 0 };
	::getcwd(buffer, sizeof(buffer) / sizeof(char));
	return buffer;
}

Path::Path(const std::string & path)
{
	ReplacePathDivider(path, m_paths);
}

Path::Path(const Path & that)
	:m_paths(that.m_paths)
{
}

Path::~Path()
{
}

Path Path::Join(const std::string& subPath) const
{
	Path path(*this);
	ReplacePathDivider(subPath, path.m_paths);
	return path;
}

Path Path::Rename(const std::string & newName) const
{
	Path newPath(this->operator std::string());
	newPath.m_paths.back() = newName;
	int rv = ::rename(this->operator std::string().c_str(), newPath.operator std::string().c_str());
	if (rv >= 0)
	{
		return newPath;
	}
	return *this;
}

Path & Path::Remove()
{
	int rv = ::rmdir(this->operator std::string().c_str());
	return *this;
}

Path Path::Parent() const
{
	if (this->m_paths.empty())
	{
		return "";
	}
	std::string merged;
	std::size_t length = this->m_paths.size() - 1;
	for (std::size_t index = 0; index < length; index++)
	{
		merged.append(this->m_paths[index]);
		if (index < length - 1)
		{
			merged.push_back(PATH_DIVIDER);
		}
	}
	return merged;
}

bool Path::Exists() const
{
	std::string path = *this;
	int rv = 0;
	rv = ::access(path.c_str(), 0);
	return rv >= 0;
}

bool Path::IsFile() const
{
	struct stat stat;
	int rv = ::stat(this->operator std::string().c_str(), &stat);
	if (rv < 0)
	{
		return false;
	}
	return (stat.st_mode & S_IFREG) != 0;
}

bool Path::IsDirectory() const
{
	struct stat stat;
	int rv = ::stat(this->operator std::string().c_str(), &stat);
	if (rv < 0)
	{
		return false;
	}
	return (stat.st_mode & S_IFDIR) != 0;
}

bool Path::CreatePath(bool rescured) const
{
	if (rescured)
	{
		if (!this->Parent().Exists())
		{
			if (!this->Parent().CreatePath(rescured))
			{
				return false;
			}
		}
	}
	if (this->Exists())
	{
		return true;
	}
	int rv = ::mkdir(this->operator std::string().c_str());
	return rv >= 0;
}

std::string Path::GetName() const
{
	return this->m_paths.back();
}

std::string Path::GetSuffix() const
{
	const std::string& fileName = this->m_paths.back();
	std::size_t pos = fileName.find_last_of('.');
	if (std::string::npos == pos || fileName.length() - 1 == pos)
	{
		return "";
	}
	return fileName.substr(pos);
}

Path::operator std::string() const
{
	if (this->m_paths.empty())
	{
		return "";
	}
	std::string merged;
	std::size_t length = this->m_paths.size();
	for (std::size_t index = 0; index < length; index++)
	{
		merged.append(this->m_paths[index]);
		if (index < length - 1)
		{
			merged.push_back(PATH_DIVIDER);
		}
	}
	if (merged.front() == '~')
	{
		return Path::GetHomePath().operator std::string() + merged.substr(1);
	}
	return merged;
}

Paths Path::GetSubPaths(const std::string & pattern, bool rescured)
{
	Paths paths;
	if (pattern.empty())
	{
		return paths;
	}
	bool getDir = pattern.find_last_of('.') == std::string::npos;
	WalkDirectory(*this, PatternToRegex(pattern), [](void* context, const Path& path) {
		Paths& paths = *(Paths*)(context);
		paths.push_back(path);
	}, &paths, getDir, rescured);
	return paths;
}

void Path::GetSubPaths(const std::string & pattern, GetSubPathsCallback callback, void* callbackContext, bool rescured)
{
	Paths paths;
	if (pattern.empty() || nullptr == callback)
	{
		return;
	}
	bool getDir = pattern.find_last_of('.') == std::string::npos;
	WalkDirectory(*this, PatternToRegex(pattern), callback, callbackContext, getDir, rescured);
}
