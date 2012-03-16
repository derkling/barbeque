#----- Deploy the 2PARMA DEMO2 script
configure_file (
	"${PROJECT_SOURCE_DIR}/demos/bbque2PARMADemo2.sh.in"
	"${PROJECT_BINARY_DIR}/demos/bbque2PARMADemo2.sh"
	@ONLY
)
install(PROGRAMS "${PROJECT_BINARY_DIR}/demos/bbque2PARMADemo2.sh"
	DESTINATION ${BBQUE_PATH_APPS}
	COMPONENT BarbequeDEMOS
	RENAME bbque-2parma-demo2)
