#include "utils/paths.h"

#include <SDL.h>

#include "utils/file_util.h"
#include "utils/log.hpp"
#include "utils/sdl_ptrs.h"

#ifdef USE_SDL1
#include "utils/sdl2_to_1_2_backports.h"
#endif

namespace devilution {

namespace paths {

namespace {

std::optional<std::string> appPath;
std::optional<std::string> basePath;
std::optional<std::string> prefPath;
std::optional<std::string> configPath;
std::optional<std::string> assetsPath;
std::optional<std::string> mpqDir;

void AddTrailingSlash(std::string &path)
{
#ifdef _WIN32
	if (!path.empty() && path.back() != '\\')
		path += '\\';
#else
	if (!path.empty() && path.back() != '/')
		path += '/';
#endif
}

std::string FromSDL(char *s)
{
	SDLUniquePtr<char> pinned(s);
	std::string result = (s != nullptr ? s : "");
	if (s == nullptr) {
		Log("{}", SDL_GetError());
		SDL_ClearError();
	}
	return result;
}

} // namespace

const std::string &AppPath()
{
	if (!appPath) {
		appPath = FromSDL(SDL_GetBasePath());
	}
	return *appPath;
}

const std::string &BasePath()
{
	if (!basePath) {
		basePath = AppPath();
	}
	return *basePath;
}

const std::string &PrefPath()
{
	if (!prefPath) {
		prefPath = FromSDL(SDL_GetPrefPath("diasurgical", "devilution"));
		if (FileExistsAndIsWriteable("diablo.ini")) {
			prefPath = std::string("./");
		}
	}
	return *prefPath;
}

const std::string &ConfigPath()
{
	if (!configPath) {
		configPath = FromSDL(SDL_GetPrefPath("diasurgical", "devilution"));
		if (FileExistsAndIsWriteable("diablo.ini")) {
			configPath = std::string("./");
		}
	}
	return *configPath;
}

const std::string &AssetsPath()
{
	if (!assetsPath)
		assetsPath.emplace(AppPath() + "assets/");
	return *assetsPath;
}

const std::optional<std::string> &MpqDir()
{
	return mpqDir;
}

void SetBasePath(const std::string &path)
{
	basePath = path;
	AddTrailingSlash(*basePath);
}

void SetPrefPath(const std::string &path)
{
	prefPath = path;
	AddTrailingSlash(*prefPath);
}

void SetConfigPath(const std::string &path)
{
	configPath = path;
	AddTrailingSlash(*configPath);
}

void SetMpqDir(const std::string &path)
{
	mpqDir = std::string(path);
}

} // namespace paths

} // namespace devilution
