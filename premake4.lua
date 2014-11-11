solution "rift"
	location "build/"
	debugdir "."
	defines { "_CRT_SECURE_NO_WARNINGS", "NOMINMAX", "GLM_FORCE_RADIANS"}
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
	include "src/model_converter"