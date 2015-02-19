project "test_mesh"
	kind "ConsoleApp"
	location "../../build/test_mesh"
	language "C++"
	files {
		"**.cpp"
	}
	includedirs { "../../include/**" }
	use_librift()
	use_gl()
	use_assimp()
	use_anttweakbar()