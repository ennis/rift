project "glslang"
	kind "StaticLib"
	location "../../build/glslang"
	language "C++"
	files {
		"glslang/Public/ShaderLang.h",
		"glslang/Include/arrays.h",
		"glslang/Include/BaseTypes.h",
		"glslang/Include/Common.h",
		"glslang/Include/ConstantUnion.h",
		"glslang/Include/InfoSink.h",
		"glslang/Include/InitializeGlobals.h",
		"glslang/Include/intermediate.h",
		"glslang/Include/PoolAlloc.h",
		"glslang/Include/ResourceLimits.h",
		"glslang/Include/revision.h",
		"glslang/Include/ShHandle.h",
		"glslang/Include/Types.h",
		"glslang/MachineIndependent/gl_types.h",
		"glslang/MachineIndependent/Initialize.h",
		"glslang/MachineIndependent/localintermediate.h",
		"glslang/MachineIndependent/ParseHelper.h",
		"glslang/MachineIndependent/reflection.h",
		"glslang/MachineIndependent/RemoveTree.h",
		"glslang/MachineIndependent/Scan.h",
		"glslang/MachineIndependent/ScanContext.h",
		"glslang/MachineIndependent/SymbolTable.h",
		"glslang/MachineIndependent/unistd.h",
		"glslang/MachineIndependent/Versions.h",
		"glslang/MachineIndependent/preprocessor/PpContext.h",
		"glslang/MachineIndependent/preprocessor/PpTokens.h",
		"glslang/MachineIndependent/Constant.cpp",
		"glslang/MachineIndependent/InfoSink.cpp",
		"glslang/MachineIndependent/Initialize.cpp",
		"glslang/MachineIndependent/IntermTraverse.cpp",
		"glslang/MachineIndependent/Intermediate.cpp",
		"glslang/MachineIndependent/ParseHelper.cpp",
		"glslang/MachineIndependent/PoolAlloc.cpp",
		"glslang/MachineIndependent/RemoveTree.cpp",
		"glslang/MachineIndependent/Scan.cpp",
		"glslang/MachineIndependent/ShaderLang.cpp",
		"glslang/MachineIndependent/SymbolTable.cpp",
		"glslang/MachineIndependent/Versions.cpp",
		"glslang/MachineIndependent/intermOut.cpp",
		"glslang/MachineIndependent/limits.cpp",
		"glslang/MachineIndependent/linkValidate.cpp",
		"glslang/MachineIndependent/parseConst.cpp",
		"glslang/MachineIndependent/reflection.cpp",
		"glslang/MachineIndependent/preprocessor/Pp.cpp",
		"glslang/MachineIndependent/preprocessor/PpAtom.cpp",
		"glslang/MachineIndependent/preprocessor/PpContext.cpp",
		"glslang/MachineIndependent/preprocessor/PpMemory.cpp",
		"glslang/MachineIndependent/preprocessor/PpScanner.cpp",
		"glslang/MachineIndependent/preprocessor/PpSymbols.cpp",
		"glslang/MachineIndependent/preprocessor/PpTokens.cpp",
		"glslang/GenericCodeGen/CodeGen.cpp",
		"glslang/GenericCodeGen/Link.cpp",
		-- TODO OS config
		"glslang/OSDependent/Windows/ossource.cpp",
		"glslang/OSDependent/Windows/osinclude.h",
		"OGLCompilersDLL/InitializeDll.h",
		"OGLCompilersDLL/InitializeDll.cpp",
		-- TODO bison
		"glslang/MachineIndependent/glslang_tab.cpp"
	}
	-- TODO OS-specific config?
	includedirs { ".", "glslang/OSDependent/Windows" }