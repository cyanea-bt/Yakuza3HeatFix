workspace "Yakuza3Patches"
	platforms { "Win64" }

project "Yakuza3HeatFix"
	kind "SharedLib"
	targetextension ".asi"
	language "C++"

	defines { 
	    "rsc_FullName=\"Yakuza3HeatFix\"",
	    "rsc_MajorVersion=0",
	    "rsc_MinorVersion=0",
	    "rsc_RevisionID=1",
	    "rsc_BuildID=0",
	}

project "Yakuza3EasySpam"
	kind "SharedLib"
	targetextension ".asi"
	language "C++"

	defines { 
	    "rsc_FullName=\"Yakuza3EasySpam\"",
	    "rsc_MajorVersion=0",
	    "rsc_MinorVersion=0",
	    "rsc_RevisionID=1",
	    "rsc_BuildID=0",
	}


workspace "*"
	configurations { "Debug", "Release", "Master" }
	location "build"

	vpaths { 
		["Headers/*"] = "source/**.h",
		["Sources/*"] = { "source/**.c", "source/**.cpp" },
		["Resources"] = "source/**.rc"
	}

	files { "**/MemoryMgr.h", "**/Trampoline.h", "**/Patterns.cpp", "**/Patterns.h", "**/HookInit.hpp", "**/json.hpp", "**/json_fwd.hpp" }
	files { "source/*.h", "source/*.cpp", "source/%{prj.name}/*.h", "source/%{prj.name}/*.cpp", "source/resources/*.rc" }
	includedirs { "source" }

	-- Disable exceptions in WIL
	defines { "WIL_SUPPRESS_EXCEPTIONS" }

	cppdialect "C++20"
	staticruntime "on"
	buildoptions { "/sdl" }
	warnings "Extra"

	-- Automated defines for resources
	defines { 
		"rsc_Extension=\"%{prj.targetextension}\"",
		"rsc_Name=\"%{prj.name}\"",
		"rsc_Repository=\"https://github.com/cyanea-bt/Yakuza3HeatFix\"",
		"rsc_UpdateURL=\"https://github.com/cyanea-bt/Yakuza3HeatFix/releases\"",
		"rsc_Copyright=\"2025 cyanea-bt\""
	}

filter "configurations:Debug"
	defines { "DEBUG", "_DEBUG" }
	runtime "Debug"

 filter "configurations:Master"
	symbols "Off"

filter "configurations:not Debug"
	defines { "NDEBUG", "_NDEBUG" }
	optimize "Speed"
	functionlevellinking "on"
	linktimeoptimization "on"

filter { "platforms:Win32" }
	system "Windows"
	architecture "x86"

filter { "platforms:Win64" }
	system "Windows"
	architecture "x86_64"

filter { "toolset:*_xp"}
	defines { "WINVER=0x0501", "_WIN32_WINNT=0x0501" } -- Target WinXP
	buildoptions { "/Zc:threadSafeInit-" }

filter { "toolset:not *_xp"}
	defines { "WINVER=0x0601", "_WIN32_WINNT=0x0601" } -- Target Win7
	buildoptions { "/permissive-" }