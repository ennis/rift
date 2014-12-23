libdirs { "lib/", "lib/boost/ "}
includedirs { "include/" }
local thisDir = os.getcwd().."/";

function use_gl()
	dofile (thisDir.."opengl.lua")
	links {"glfw3", "glew32"}
	includedirs {thisDir.."include/GL"}
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
