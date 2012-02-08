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

#ifndef BBQUE_TEST_H_
#define BBQUE_TEST_H_

#define TEST_NAMESPACE "test."

namespace bbque { namespace plugins {

/**
 * @brief A module to support testing of other components implementation.
 *
 * This is a common interface for each testing module which can be
 * developed for the Barbeque framework.
 */
class TestIF {

public:

	virtual void Test() = 0;

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_TEST_H_
