project "test_mesh"
	links "librift"
	kind "ConsoleApp"
	location "../../build/test_mesh"
	language "C++"
	files {
		"**.cpp"
	}
	includedirs { "../../include/**" }
	use_gl()
	use_assimp()
	use_anttweakbar()