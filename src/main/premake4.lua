project "main"
	links "librift"
	kind "ConsoleApp"
	location "../../build/main"
	language "C++"
	files {
		"../../include/rift/**.hpp",
		"**.cpp"
	}
	includedirs { "../../include/**" }
	use_gl()
	use_assimp()
	use_anttweakbar()