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

#include "bbque/res/usage.h"


namespace bbque { namespace res {


Usage::Usage(uint64_t usage_value):
	value(usage_value) {
}

Usage::~Usage() {
	bindings.clear();
}

uint64_t Usage::GetAmount() {
	return value;
}

ResourcePtrList_t & Usage::GetBindingList() {
	return bindings;
}

void Usage::SetBindingList(ResourcePtrList_t bind_list) {
	bindings = bind_list;
	first_bind = bind_list.begin();
	last_bind = bind_list.end();
}

bool Usage::EmptyBindingList() {
	return bindings.empty();
}

ResourcePtr_t Usage::GetFirstResource(
		ResourcePtrListIterator_t & it) {
	// Check if 'first_bind' points to a valid resource descriptor
	if (first_bind == bindings.end())
		return ResourcePtr_t();

	// Set the argument iterator and return the shared pointer to the
	// resource descriptor
	it = first_bind;
	return (*it);
}

ResourcePtr_t Usage::GetNextResource(
		ResourcePtrListIterator_t & it) {
	do {
		// Next resource used by the application
		++it;

		// Return null if there are no more resource bindings
		if ((it == bindings.end()) || (it == last_bind))
			return ResourcePtr_t();
	} while ((*it)->ApplicationUsage(own_app, view_tk) == 0);

	// Return the shared pointer to the resource descriptor
	return (*it);
}

Usage::ExitCode_t Usage::TrackFirstBinding(
		AppSPtr_t const & papp,
		ResourcePtrListIterator_t & first_it,
		RViewToken_t vtok) {
	if (!papp)
		return RU_ERR_NULL_POINTER;

	view_tk = vtok;
	own_app = papp;
	first_bind = first_it;

	return RU_OK;
}

Usage::ExitCode_t Usage::TrackLastBinding(
		AppSPtr_t const & papp,
		ResourcePtrListIterator_t & last_it,
		RViewToken_t vtok) {
	if (!papp)
		return RU_ERR_NULL_POINTER;

	if (!own_app)
		return RU_ERR_APP_MISMATCH;

	if (vtok != view_tk)
		return RU_ERR_VIEW_MISMATCH;

	last_bind = last_it;
	return RU_OK;
}

} // namespace res

} // namespace bbque
