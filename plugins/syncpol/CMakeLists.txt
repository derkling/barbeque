
#----- Add "sasb" target dynamic library
set(PLUGIN_SASB_SRC sasb_syncpol sasb_plugin)
add_library(bbque_syncpol_sasb MODULE ${PLUGIN_SASB_SRC})
install(TARGETS bbque_syncpol_sasb LIBRARY
		DESTINATION ${BBQUE_PATH_PLUGINS}
		COMPONENT BarbequeRTRM)

