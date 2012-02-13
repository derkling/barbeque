
# .:: Mandatory Tools required for the usage of the BarbequeRTRM
# These tools must be:
# - part of the BarbequeTOOLS componet
# - have a name starting by "bbque-"

#----- Deploy the PIL Layouts parser and CGroup generator script
install(PROGRAMS bbquePlatformLayoutParser.awk
	DESTINATION ${BBQUE_PATH_BBQ}
	COMPONENT BarbequeTOOLS
	RENAME bbque-plp)

# .:: Accessory Tools to simplify the usage of the BarbequeRTRM
# These tools must be:
# - part of the BarbequeUTILS componet
# - have a name starting by "bbque-"

#----- Deploy the BBQ Control Console script
install(PROGRAMS bbqueConsole.sh
	DESTINATION ${BBQUE_PATH_BBQ}
	COMPONENT BarbequeUTILS
	RENAME bbque-console)
