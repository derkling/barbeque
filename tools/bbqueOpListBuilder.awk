#!/usr/bin/awk -f
#
# Copyright (C) 2012  Politecnico di Milano
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

############################## USAGE NOTES #####################################
# This is a simple filter to translate an XML OPs file into a statically
# allocates vector of operating points, which type is the OperatingPointsList
# class as defined by bbque/monitors/operating_point.h
#
# Given an input XML file of OPs, this script produce in output a C source file
# which is suitable to be compiled and linked with the OP consumer code, which
# requires just to define an external reference to this variable, i.e.:
#    extern OperatingPointsList opList;
#
# The name of the generated vector could be customized by passing a proper value
# with the BBQUE_RTLIB_OPLIST variable.
#
# The generation of this C source file could be automatized via CMake by adding
# the CMakeList.txt a command like e.g.:
# add_custom_command (OUTPUT opList.cc
#      COMMAND ${PROJECT_SOURCE_DIR}/build_ops.awk ${BBQUE_OPS_XML} >
#              ${PROJECT_BINARY_DIR}/${BBQUE_OPS_C}
#      DEPENDS opList.xml
#      COMMENT "Updating [${BBQUE_OPS_C}] OPs list using [${BBQUE_OPS_XML}]...")
# NOTE: the generated output (i.e. opList.cc) should be listed as a source
# file in the target where this sould is part of.
################################################################################

BEGIN {
	# Setup Filter Variables
	if (!length(BBQUE_RTLIB_OPLIST))  BBQUE_RTLIB_OPLIST="opList"

	# Dump Source header
	printf "/* This file has been automatically generated using */\n"
	printf "/* the bbque-opp Operating Points parser script. */\n"
	printf "#include <bbque/monitors/operating_point.h>\n"
	printf "using bbque::rtlib::as::OperatingPointsList;\n"
	printf "OperatingPointsList %s = {\n", BBQUE_RTLIB_OPLIST;
}

/<parameters>/ {
	printf "  { //===== OP %03d =====\n", OP_COUNT++
	printf "    { //=== Parameters\n"
}
match($0, /name="([^"]+).+value="([^"]+)/, o) {
	printf "      {\"%s\", %s},\n", o[1], o[2]
}
/<system_metrics>/ {
	printf "    },\n"
	printf "    { //=== Metrics\n"
}
/<\/system_metrics>/ {
	printf "    },\n"
	printf "  },\n";
}
END {
	printf "};\n\n";
}
