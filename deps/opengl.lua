
configuration "macosx"
		includedirs{"/usr/include"
				,"/usr/local/include",
				"/Developer/SDKs/MacOSX10.6.sdk/usr/include/", 
				"/Developer/SDKs/MacOSX10.6.sdk/usr/include/X11/include/"
				,"'/Developer/GPU Computing/C/common/inc/'" --from CUDA..??
				}
		libdirs {
				"/usr/lib"
		,"/usr/local/lib"
		,"/Developer/SDKs/MacOSX10.6.sdk/usr/lib"
		,"/Developer/SDKs/MacOSX10.6.sdk/usr/X11/lib"
		--from CUDA..??	
		,"'/Developer/GPU Computing/C/common/lib/'"
		,"'/Developer/GPU Computing/shared/lib/darwin/'"
		}
configuration "linux"
		includedirs{"//usr/include","//usr/local/include"}
configuration {"macosx"}
		links {"GL", "GLEW", "GLU"}
configuration {"linux"}
		links {"GL", "glut", "GLEW", "GLU"}
configuration {"windows", "x32"}
		links {"opengl32","gdi32"}
configuration {"windows", "x64"}
		links {"opengl32","gdi32"}
configuration {}
