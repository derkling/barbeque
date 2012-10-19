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

#----- Deploy the 2PARMA DEMO3 script
configure_file (
	"${PROJECT_SOURCE_DIR}/demos/bbque2PARMADemo3.sh.in"
	"${PROJECT_BINARY_DIR}/demos/bbque2PARMADemo3.sh"
	@ONLY
)
install(PROGRAMS "${PROJECT_BINARY_DIR}/demos/bbque2PARMADemo3.sh"
	DESTINATION ${BBQUE_PATH_APPS}
	COMPONENT BarbequeDEMOS
	RENAME bbque-2parma-demo3)

#----- Deploy the 2PARMA OCVDemo1 script
configure_file (
	"${PROJECT_SOURCE_DIR}/demos/bbque2PARMAOCVDemo1.sh.in"
	"${PROJECT_BINARY_DIR}/demos/bbque2PARMAOCVDemo1.sh"
	@ONLY
)
install(PROGRAMS "${PROJECT_BINARY_DIR}/demos/bbque2PARMAOCVDemo1.sh"
	DESTINATION ${BBQUE_PATH_APPS}
	COMPONENT BarbequeDEMOS
	RENAME bbque-2parma-ocvdemo1)
