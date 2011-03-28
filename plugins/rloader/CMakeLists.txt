
#----- Check for required TiCPP library
#
# TiCPP is a C++ wrapper for library TinyXML

find_path(TICPP_INCLUDE_DIRS ticpp.h tinyxml.h
	PATHS
		"$ENV{LOG4CPP}/include"
		/usr/local/include
		/usr/include
)

find_library(TICPP_LIBRARIES ticpp
	PATHS
		"$ENV{LOG4CPP}/lib"
		/usr/local/lib
		/usr/lib
		)

if (TICPP_INCLUDE_DIRS)
	if (TICPP_LIBRARIES)
		include_directories(${TICPP_INCLUDE_DIRS})
		link_directories(${TICPP_LIBRARIES})
		message("-- Found TiCPP")
	endif (TICPP_LIBRARIES)
endif (TICPP_INCLUDE_DIRS)

#----- Add target static library
set(PLUGIN_RECIPELOAD_XML_SRC  xml_rloader xml_plugin)
add_library(plugin_recipe_loader_xml STATIC ${PLUGIN_RECIPELOAD_XML_SRC})

#----- Add library specific flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

# Linking dependencies
target_link_libraries(plugin_recipe_loader_xml ${TICPP_LIBRARIES})
