#include <iostream>
#include <filesystem>
#include <fstream>
#include "nlohmann/json.hpp"
#include "config.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#define WINVER 0x0601
#define _WIN32_WINNT 0x0601
#include <windows.h>
#include <ShlObj.h>


// Get real path to .asi file, since the working dir will be the dir of the .exe that loaded it.
// And that won't match the .asi file's path if it is loaded from inside the "mods" subdirectory.
// ref: https://gist.github.com/pwm1234/05280cf2e462853e183d
//      https://stackoverflow.com/questions/6924195/get-dll-path-at-runtime
static std::filesystem::path get_module_path(void *address)
{
	WCHAR path[MAX_PATH];
	HMODULE hm = NULL;

	if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		(LPCWSTR)address,
		&hm))
	{
		std::ostringstream oss;
		oss << "GetModuleHandle returned " << GetLastError();
		throw std::runtime_error(oss.str());
	}
	GetModuleFileNameW(hm, path, MAX_PATH);

	std::wstring p(path);
	return std::filesystem::path(p);
}


namespace config {
	using namespace std;
	namespace fs = std::filesystem;
	using json = nlohmann::ordered_json;

	void to_json(json &j, const Config &e) {
		j = json();
		j["Version"] = e.Version;
		j["Enable"] = e.Enable;
		j["Force"] = e.Force;
	}

	void from_json(const json &j, Config &e) {
		j.at("Version").get_to(e.Version);
		j.at("Enable").get_to(e.Enable);
		j.at("Force").get_to(e.Force);
	}

	// write prettified JSON to ostream
	bool writeJson(ostream &os, const json &j) {
		const auto old_width = os.width();
		// dump() produces warning C28020 and doesn't seem fixable? Probably a false positive anyways.
		// So suppress warning for 1 line
		// ref: https://stackoverflow.com/a/25447795
#pragma warning(suppress: 28020)
		os << setw(4) << j << endl << setw(old_width);
		os.flush();
		return !os.fail();
	}

	Config loadConfig() {
		ifstream ifs;
		ofstream ofs;
		fs::path configPath(format("{:s}{:s}", rsc_Name, ".json"));
		if (!fs::exists(configPath)) {
			const fs::path asiDir = get_module_path(get_module_path).parent_path();
			configPath = fs::path(asiDir / configPath.filename());
		}
		
		Config defaults = Config();
		try {
			Config conf;
			if (!fs::exists(configPath)) {
				// create default config next to .asi file
				ofs = ofstream(configPath, ios::out | ios::binary | ios::trunc);
				writeJson(ofs, defaults);
				conf = defaults;
			}
			else {
				// read existing config
				ifs = ifstream(configPath, ios::in | ios::binary);
				const json data = json::parse(ifs);
				ifs.close();
				
				uint32_t version = 0;
				if (data.contains("Version")) {
					version = data["Version"];
				}
				if (version == defaults.Version) {
					// config version matches, should be able to parse
					conf = data;
				}
				else {
					// replace outdated config with new defaults
					const fs::path defaultBakPath(format("{:s}{:s}", configPath.string(), ".bak"));
					fs::path bakPath(defaultBakPath);
					int bakCounter = 1;
					while (fs::exists(bakPath)) {
						bakPath = fs::path(format("{:s}{:d}", defaultBakPath.string(), bakCounter++));
					}
					fs::rename(configPath, bakPath);
					ofs = ofstream(configPath, ios::out | ios::binary | ios::trunc);
					writeJson(ofs, defaults);
					conf = defaults;
				}
			}
			conf.path = configPath.string();
			return conf;
		}
		catch (const exception &err) {
			//throw err;
			cerr << err.what();
			return defaults;
		}
	}
}
