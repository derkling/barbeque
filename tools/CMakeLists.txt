
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
configure_file (
	"${PROJECT_SOURCE_DIR}/tools/bbqueConsole.sh.in"
	"${PROJECT_BINARY_DIR}/tools/bbqueConsole.sh"
	@ONLY
)
install(PROGRAMS "${PROJECT_BINARY_DIR}/tools/bbqueConsole.sh"
	DESTINATION ${BBQUE_PATH_BBQ}
	COMPONENT BarbequeUTILS
	RENAME bbque-console)

#----- Deploy the BBQ Stress Test script
configure_file (
	"${PROJECT_SOURCE_DIR}/tools/bbqueStressTest.sh.in"
	"${PROJECT_BINARY_DIR}/tools/bbqueStressTest.sh"
	@ONLY
)
install(PROGRAMS "${PROJECT_BINARY_DIR}/tools/bbqueStressTest.sh"
	DESTINATION ${BBQUE_PATH_BBQ}
	COMPONENT BarbequeUTILS
	RENAME bbque-stest)

#----- Deploy the BBQ OP List generator script
install(PROGRAMS "${PROJECT_SOURCE_DIR}/tools/bbqueOpListBuilder.awk"
	DESTINATION ${BBQUE_PATH_BBQ}
	COMPONENT BarbequeUTILS
	RENAME bbque-oplb)

#----- Deploy the BBQ Metrics Plotting scripts
configure_file (
	"${PROJECT_SOURCE_DIR}/tools/bbqueLogParser.awk.in"
	"${PROJECT_BINARY_DIR}/tools/bbqueLogParser.awk"
	@ONLY
)

install(PROGRAMS "${PROJECT_BINARY_DIR}/tools/bbqueLogParser.awk"
	DESTINATION ${BBQUE_PATH_BBQ}
	COMPONENT BarbequeUTILS
	RENAME bbque-logparser)
