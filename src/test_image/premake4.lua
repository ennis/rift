project "test_image"
	use_librift()
	kind "ConsoleApp"
	location "../../build/test_image"
	language "C++"
	files {
		"**.cpp"
	}
	includedirs { "../../include/**" }
	use_gl()
	use_assimp()
	use_anttweakbar()