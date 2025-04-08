cmake_minimum_required(VERSION 3.5)
project(example)

# * 1 *   Header directory "../include"
include_directories(${CMAKE_SOURCE_DIR}/../../include_fade25d/)

# * 2 *   Add the executable for WIN x64 and Linux
if(MSVC)
	### WINDOWS ###
	
	# Parallel compilation
	add_compile_options(/MP)
	# TOOLNAME v140, v141, v142 (VS2015, VS2017, VS2019)
	set(TOOLNAME "v${MSVC_TOOLSET_VERSION}")

	# Link directories
	link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../x64)
	
	# Fade library version
	set(FADELIBNAME 
		debug "fade25D_x64_${TOOLNAME}_Debug"
		optimized "fade25D_x64_${TOOLNAME}_Release"
		)
	add_executable(remTriangles main.cpp )
	TARGET_LINK_LIBRARIES(remTriangles ${FADELIBNAME}  )							
	MESSAGE("CMAKE_BINARY_DIR is ${CMAKE_BINARY_DIR}")	
	set(CONFIG  $<$<CONFIG:Release>:Release>$<$<CONFIG:Debug>:Debug>)
	add_custom_command(	TARGET remTriangles POST_BUILD  
				COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../x64/fade25d_x64_${TOOLNAME}_${CONFIG}.dll " ${CMAKE_BINARY_DIR}/${CONFIG}
				COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/point_cloud.bin " ${CMAKE_BINARY_DIR}/${CONFIG}
				COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../x64/libgmp-10.dll " ${CMAKE_BINARY_DIR}/${CONFIG}
				) 

	
else()
	### LINUX ###
	link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../lib_ubuntu22.04_x86_64/)
	add_executable(remTriangles main.cpp )
	TARGET_LINK_LIBRARIES(remTriangles fade25d pthread  )
	add_custom_command(	TARGET remTriangles POST_BUILD  
				COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/point_cloud.bin" ${CMAKE_BINARY_DIR})
endif()

