project "librift"
	kind "StaticLib"
	location "../../build/librift"
	language "C++"
	files {
		"../../include/rift/**.hpp",
		"**.cpp"
	}
	includedirs { "../../include/**" }
	use_gl()
	use_assimp()
	use_anttweakbar()