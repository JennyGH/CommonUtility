#pragma once
#include <vector>
#include <string>
class Path;
using Paths = std::vector<Path>;
class Path
{
public:
	using GetSubPathsCallback = void(*)(void* context, const Path& path);

public:
	static Path GetHomePath();
	static Path GetCurrentPath();

public:
	Path(const std::string& path = "");
	Path(const Path& that);
	~Path();

	Path  Join(const std::string& subPath) const;
	Path  Rename(const std::string& newName) const;
	Path& Remove();
	Path  Parent()  const;

	bool Exists()      const;
	bool IsFile()      const;
	bool IsDirectory() const;
	bool CreatePath(bool rescured = false)  const;

	std::string GetName()   const;
	std::string GetSuffix() const;

	operator std::string() const;

	Paths GetSubPaths(
		const std::string& pattern,
		bool rescured = false);
	void GetSubPaths(
		const std::string& pattern,
		GetSubPathsCallback callback,
		void* callbackContext,
		bool rescured = false);

private:
	std::vector<std::string> m_paths;
};