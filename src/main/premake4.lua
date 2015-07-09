project "main"
	use_librift()
	kind "ConsoleApp"
	location "../../build/main"
	language "C++"
	files {
		"../../include/rift/**.hpp",
		"**.cpp"
	}
	includedirs { "../../include/**" }
	use_gl()
	use_anttweakbar()