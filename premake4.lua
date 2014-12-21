solution "rift"
	location "build/"
	objdir "build/obj"
	targetdir "build/"
	debugdir "."
	defines { "_CRT_SECURE_NO_WARNINGS", "_SCL_SECURE_NO_WARNINGS", "NOMINMAX", "GLM_FORCE_RADIANS"}
	platforms { "x64" }
	configurations {"Debug", "Release"}
	configuration "Debug"
		flags { "Symbols" }
	configuration "Release"
		flags { "Optimize" }
	configuration {}
	dofile "deps/links.lua"
	include "src/rift"
	include "src/main"
	include "src/test_mesh"
