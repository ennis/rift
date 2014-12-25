project "test_model"
	kind "ConsoleApp"
	location "../../build/test_model"
	language "C++"
	files {
		"**.cpp"
	}
	includedirs { "../../include/**" }
	use_librift()
	use_gl()
	use_assimp()
	use_anttweakbar()