/*
 * Copyright (C) 2012  Politecnico di Milano
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <string>


#define COLOR_WHITE  "\033[1;37m"
#define COLOR_LGRAY  "\033[37m"
#define COLOR_GRAY   "\033[1;30m"
#define COLOR_BLACK  "\033[30m"
#define COLOR_RED    "\033[31m"
#define COLOR_LRED   "\033[1;31m"
#define COLOR_GREEN  "\033[32m"
#define COLOR_LGREEN "\033[1;32m"
#define COLOR_BROWN  "\033[33m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_BLUE   "\033[34m"
#define COLOR_LBLUE  "\033[1;34m"
#define COLOR_PURPLE "\033[35m"
#define COLOR_PINK   "\033[1;35m"
#define COLOR_CYAN   "\033[36m"
#define COLOR_LCYAN  "\033[1;36m"


#define TOKEN_OPEN_MARK		"@"
#define TOKEN_CLOSE_MARK	"@@"

#define TUTORIAL_MODE 		1

#ifdef TUTORIAL_MODE
#define TTR_MESSAGE(msg_token,color,stop)\
		showMessage(msgs_file, msg_token, color);\
		if (stop) {\
			std::cout << "\tcontinue..." << std::endl;\
			getchar();\
		}
#else
#define TTR_MESSAGE(msg_token,__VA_ARGS__) //
#endif


void clearScreen() {
	std::cout << "\033[0;0f" << std::endl;
	std::cout << "\033[2J" << std::endl;
}

// Display tutorial messages

void showMessage(std::ifstream & msgs_file,
		std::string const & msg_token,
		const char * color =  "\033[0m") {

	std::string line;
	std::string lkstr(TOKEN_OPEN_MARK);

	if (!msgs_file.is_open()) {
		std::cout << "Messages file error: not open." << std::endl;
		std::cout << msgs_file << std::endl;
		return;
	}

	// Lookup the message token
	do {
		std::getline(msgs_file, line);
	}
	while ((line.find(lkstr + msg_token) == std::string::npos)
			&& (!msgs_file.eof()));

	// Check if the token has been found
	if (msgs_file.eof()) {
		std::cout << "Messages file error: token not found." << std::endl;
		return;
	}

	// Print the message
	std::getline(msgs_file, line);
	while (((line.find(TOKEN_CLOSE_MARK)) == std::string::npos)
			&& (!msgs_file.eof())) {
		std::cout << color << line << "\033[0m" << std::endl;
		std::getline(msgs_file, line);
	}
}

