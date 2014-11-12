project "librift"
	kind "StaticLib"
	location "../../build/librift"
	language "C++"
	files {
		"../../include/rift/**.hpp",
		"**.cpp"
	}
	-- PCH fix for windows
	configuration { "windows" }
		buildoptions { "/FI".."\"stdafx.h\"" }
		pchheader "stdafx.h"
		pchsource "stdafx.cpp"
	configuration {}
	includedirs { "../../include/**" }
	use_gl()
	use_assimp()
	use_anttweakbar()