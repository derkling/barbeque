
#----- Add "random" target static library
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(PLUGIN_RANDOM_SRC  random_schedpol random_plugin)
	add_library(bbque_schedpol_random MODULE ${PLUGIN_RANDOM_SRC})
	install(TARGETS bbque_schedpol_random LIBRARY
			DESTINATION ${BBQUE_PATH_PLUGINS})
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

#----- Add "yamca" target static library
set(PLUGIN_YAMCA_SRC  yamca_schedpol yamca_plugin)
add_library(bbque_schedpol_yamca MODULE ${PLUGIN_YAMCA_SRC})
install(TARGETS bbque_schedpol_yamca LIBRARY
		DESTINATION ${BBQUE_PATH_PLUGINS}
		COMPONENT BarbequeRTRM)

