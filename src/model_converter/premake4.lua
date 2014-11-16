project "model_converter"
	links "librift"
	kind "ConsoleApp"
	location "../../build/model_converter"
	language "C++"
	files {
		"../../include/rift/**.hpp",
		"**.cpp"
	}
	includedirs { "../../include/**", "." }
	use_gl()
	use_assimp()
	use_anttweakbar()
	use_msgpack()