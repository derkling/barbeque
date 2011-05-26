/**
 *       @file  resource_tree.cc
 *      @brief  Resource Tree for resource descriptors
 *
 * This implements a class for storing Resource descriptors in a tree based
 * structure. Such class allow the lookup of descriptors using a
 * namespace-like approach (i.e.  "arch.clusters.mem0").
 *
 *     @author  Giuseppe Massari (jumanix), joe.massanga@gmail.com
 *
 *   @internal
 *     Created  04/04/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Giuseppe Massari
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#include "bbque/res/resource_tree.h"

#include <iostream>

#include "bbque/modules_factory.h"
#include "bbque/res/resources.h"
#include "bbque/utils/utility.h"

namespace bp = bbque::plugins;

namespace bbque { namespace res {

ResourceTree::ResourceTree():
	max_depth(0) {

	// Get a logger
	bp::LoggerIF::Configuration conf(RESOURCE_THREE_NAMESPACE);
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
	assert(logger);

	root = new ResourceNode;
	root->data = ResourcePtr_t(new Resource("root"));
	root->parent = NULL;
	root->depth = 0;
}


ResourcePtr_t & ResourceTree::insert(std::string const & _rsrc_path) {
	// Start from the root
	ResourceNode * curr_node = root;

	// Extract the first namespace level in the resource path
	std::string ns_path	= _rsrc_path;
	std::string curr_ns = SplitAndPop(ns_path);

	// For each namespace level...
	while (!curr_ns.empty()) {

		// If the resource has not children, create the first child using the
		// current namespace level as name
		 if (curr_node->children.empty()) {
			curr_node = add_child(curr_node, curr_ns);
			// Update the max depth value
			if (curr_node->depth > max_depth)
				max_depth = curr_node->depth;
		}
		else {
			// Children node iterators
			ResourceNodesList_t::iterator it = curr_node->children.begin();
			ResourceNodesList_t::iterator end = curr_node->children.end();

			// Check if the current resource path level exists
			for (; it != end; ++it) {
				// Yes: move one level down
				if ((*it)->data->Name().compare(curr_ns) == 0) {
					curr_node = *it;
					break;
				}
			}

			// No: add a new resource as sibling node
			if (it == end)
				curr_node = add_child(curr_node, curr_ns);
		}

		// Pop next level/namespace in the path
		curr_ns = SplitAndPop(ns_path);
	}
	// Return the new object just created
	return curr_node->data;
}


bool ResourceTree::find_node(ResourceNode * curr_node,
		std::string const & rsrc_path, SearchOption_t opt,
		std::list<ResourcePtr_t> & matches) const {

	// Null node check
	if (curr_node == NULL)
		return false;

	// Extract the first node in the path, and save the remaining path string
	std::string next_path = rsrc_path;
	std::string curr_ns = SplitAndPop(next_path);

	// Parse the path
	if (!curr_ns.empty()) {
		// Stop search if children list is empty
		if (curr_node->children.empty())
			return 0;

		// Children iterators
		ResourceNodesList_t::iterator it_child = curr_node->children.begin();
		ResourceNodesList_t::iterator end_child = curr_node->children.end();

		// Check if the current namespace node exists looking for it in the
		// list of children
		for (; it_child != end_child; ++it_child) {

			// Check if the current namespace to find is ID-based
			std::string res_name = (*it_child)->data->Name();
			size_t id_pos;
			if (opt == RT_SET_MATCHES)
				id_pos = curr_ns.find_first_of("0123456789");

			// If the search is not ID based (path template search)
			// remove the ID numbre from the current resource namespace
			if ((opt == RT_FIRST_MATCH) || (opt == RT_ALL_MATCHES)
				|| ((opt == RT_SET_MATCHES) && (id_pos == std::string::npos))) {
				id_pos = res_name.find_first_of("0123456789");
				res_name = res_name.substr(0, id_pos);
			}

			// Namespaces comparison
			if (curr_ns.compare(res_name) == 0) {

				// Matched. If we're at the end of the path, append the
				// resource descriptor into the list to return. Otherwise
				// continue recursively
				if (next_path.empty())
					matches.push_back((*it_child)->data);
				else
					find_node(*it_child, next_path, opt, matches);

				// If the search doesn't require all the matches we can stop
				if ((opt == RT_EXACT_MATCH) || (opt == RT_FIRST_MATCH))
					break;
			}
		}
	}
	// Return true if the list is not empty
	return !matches.empty();
}


ResourceNode * ResourceTree::add_child(ResourceNode * curr_node,
		std::string const & rsrc_name) {

	// Create the new resource node
	ResourceNode * _node = new ResourceNode;
	_node->data = ResourcePtr_t(new Resource(rsrc_name));

	// Set the parent and the depth
	_node->parent = curr_node;
	_node->depth = curr_node->depth + 1;

	// Append it as child of the current node
	curr_node->children.push_back(_node);
	return _node;
}


void ResourceTree::print_children(ResourceNode * _node, int _depth) {

	// Increase the level of depth
	++_depth;

	// Children iterators
	ResourceNodesList_t::iterator it = _node->children.begin();
	ResourceNodesList_t::iterator end = _node->children.end();

	// Print all the children
	for (; it != end; ++it) {
		// Child name
		for (int i= 0; i < _depth-1; ++i)
			logger->Debug("\t");

		logger->Debug("|-------%s", (*it)->data->Name().c_str());

		// Recursive call if there are some children
		if (!(*it)->children.empty())
			print_children(*it, _depth);
	}
}


void ResourceTree::clear_node(ResourceNode * _node) {

	// Children iterators
	ResourceNodesList_t::iterator it = _node->children.begin();
	ResourceNodesList_t::iterator end = _node->children.end();

	// Recursive clear
	for (; it != end; ++it) {
		if (!(*it)->children.empty())
			clear_node(*it);
		(*it)->children.clear();
	}
}

}   // namespace res

}   // namespace bbque

