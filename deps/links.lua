libdirs { "lib/" }
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

function use_anttweakbar()
	includedirs {thisDir.."include/anttweakbar"}
	links {"AntTweakBar64"}
end

function use_anttweakbar()
	includedirs {thisDir.."include/msgpack"}
end