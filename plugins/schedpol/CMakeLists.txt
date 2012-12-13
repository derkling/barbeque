
#----- Add "Random" target dynamic library
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(PLUGIN_RANDOM_SRC  random_schedpol random_plugin)
  add_library(bbque_schedpol_random MODULE ${PLUGIN_RANDOM_SRC})
  install(TARGETS bbque_schedpol_random LIBRARY
		DESTINATION ${BBQUE_PATH_PLUGINS})
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

#----- Add "YaMCA" target dynamic library
set(PLUGIN_YAMCA_SRC  yamca_schedpol yamca_plugin)
add_library(bbque_schedpol_yamca MODULE ${PLUGIN_YAMCA_SRC})
target_link_libraries(
	bbque_schedpol_yamca
	${Boost_LIBRARIES}
)
install(TARGETS bbque_schedpol_yamca LIBRARY
		DESTINATION ${BBQUE_PATH_PLUGINS}
		COMPONENT BarbequeRTRM)

#----- Add "YaMS" target dynamic library
add_subdirectory(contrib)
set(PLUGIN_YAMS_SRC  yams_schedpol yams_plugin)
add_library(bbque_schedpol_yams MODULE ${PLUGIN_YAMS_SRC})
target_link_libraries(
	bbque_schedpol_yams
	bbque_sched_contribs
	${Boost_LIBRARIES}
)
install(TARGETS bbque_schedpol_yams LIBRARY
		DESTINATION ${BBQUE_PATH_PLUGINS}
		COMPONENT BarbequeRTRM)
