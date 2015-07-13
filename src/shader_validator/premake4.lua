project "shader_validator"
	use_glslang()
	kind "ConsoleApp"
	location "../../build/shader_validator"
	language "C++"
	files {
		"**.hpp",
		"**.cpp"
	}
	includedirs { "." }
