project "test_image"
	links "librift"
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