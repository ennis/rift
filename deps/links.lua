libdirs { "lib/", "lib/boost/ "}
includedirs { "include/" }
local thisDir = os.getcwd().."/";

-- generate an opengl header
-- TODO maybe we could execute the script directly from premake
os.execute("lua "..
		thisDir.."glLoadGen_2_0_2/LoadGen.lua "..
		thisDir.."include/core_4_4 "..
		"-style=func_cpp "..
		"-spec=gl ".. 
		"-version=4.4 "..
		"-profile=core core_4_4 "..
		"-exts EXT_direct_state_access EXT_texture_compression_s3tc ")

function use_gl()
	dofile (thisDir.."opengl.lua")
	links {"glfw3"}
	includedirs {thisDir.."include/GL"}
	files {thisDir.."include/gl_core_4_4.cpp"}
end

function use_assimp()
	includedirs {thisDir.."include/assimp"}
	links {"assimp"}
end

function use_librift()
	links "librift"
	--no need for that, boost already adds itself to the linker. sneaky
	--use_boost_module("filesystem")
	--use_boost_module("system")
end


function use_anttweakbar()
	includedirs {thisDir.."include/anttweakbar"}
	links {"AntTweakBar64"}
end

function use_boost()
	-- nothing to do
end

-- only for VS
function use_boost_module(name)
	configuration "Debug"
		links { "libboost_"..name.."-vc120-mt-s-1_57" }
	configuration {}
		links { "libboost_"..name.."-vc120-mt-sgd-1_57" }
end
