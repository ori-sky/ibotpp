find_package(LLVM REQUIRED)

if(NOT Clang_FIND_COMPONENTS)
	message(FATAL_ERROR "Must specify which Clang components to find")
endif()

foreach(COMPONENT ${Clang_FIND_COMPONENTS})
	find_library(CLANG_${COMPONENT}_LIB ${COMPONENT}
		PATHS ${LLVM_LIBRARY_DIRS} ${CLANG_LIBRARY_DIRS}
	)

	if(NOT CLANG_${COMPONENT}_LIB AND Clang_FIND_REQUIRED_${COMPONENT})
		message(FATAL_ERROR "Could NOT find required Clang component: ${COMPONENT}")
	endif()

	message(STATUS "Found Clang component: ${COMPONENT}")
	set(CLANG_LIBS ${CLANG_LIBS} ${CLANG_${COMPONENT}_LIB})
	set(CLANG_LDFLAGS ${CLANG_LDFLAGS} "-l$COMPONENT}")
endforeach(COMPONENT)

find_path(CLANG_INCLUDE_DIRS clang/Basic/Version.h HINTS ${LLVM_INCLUDE_DIRS})
if(CLANG_LIBS AND CLANG_INCLUDE_DIRS)
	message(STATUS "Found Clang libs: ${CLANG_LIBS}")
	message(STATUS "Found Clang includes: ${CLANG_INCLUDES}")
elseif(Clang_FIND_REQUIRED)
	message(FATAL_ERROR "Could NOT find Clang")
endif()
