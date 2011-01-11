/**
 *       @file  main.cpp
 *      @brief  The RTRM protorype implementation for 2PARMA EU FP7 project
 *
 * Detailed description starts here.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  01/11/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#include <bbque/barbeque.h>

using namespace std;

int main(int argc, char *argv[])
{
	cout << "\t\t.:: Barbeque RTRM (ver. " << ::g_git_version << ") ::." << endl;
	cout << "Built: " << __DATE__ << " " << __TIME__ << endl;
	
	return EXIT_SUCCESS;
}

