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

	root = new ResourceNode_t;
	root->data = ResourcePtr_t(new Resource("root"));
	root->parent = NULL;
	root->depth = 0;
}


ResourcePtr_t & ResourceTree::insert(std::string const & _rsrc_path) {
	// Extract the first namespace level in the resource path
	ResourceNode_t * curr_node = root;
	std::string ns_path(_rsrc_path);
	std::string curr_ns(SplitAndPop(ns_path));

	// For each namespace level...
	for (; !curr_ns.empty(); curr_ns = SplitAndPop(ns_path)) {

		// If the resource has not children create the first child using the
		// current namespace level
		if (curr_node->children.empty()) {
			curr_node = add_child(curr_node, curr_ns);

			// Update max depth value
			if (curr_node->depth > max_depth)
				max_depth = curr_node->depth;
			continue;
		}

		// Iterate over siblings
		ResourceNodesList_t::iterator it(curr_node->children.begin());
		ResourceNodesList_t::iterator end(curr_node->children.end());
		for (; it != end; ++it) {
			// Check if the current resource path level exists
			if ((*it)->data->Name().compare(curr_ns) != 0)
				continue;
			// Yes: move one level down
			curr_node = *it;
			break;
		}

		// No: add a new resource as sibling node
		if (it == end)
			curr_node = add_child(curr_node, curr_ns);
	}

	// Return the new object just created
	return curr_node->data;
}


bool ResourceTree::find_node(ResourceNode_t * curr_node,
		std::string const & rsrc_path,
		SearchOption_t opt,
		std::list<ResourcePtr_t> & matches) const {

	// Null node / empty children lsit check
	if ((!curr_node) || (curr_node->children.empty()))
		return false;

	// Extract the first node in the path, and save the remaining path string
	std::string next_path(rsrc_path);
	std::string curr_ns(SplitAndPop(next_path));
	if (curr_ns.empty())
		return false;

	// Children iterators
	ResourceNodesList_t::iterator it_child(curr_node->children.begin());
	ResourceNodesList_t::iterator end_child(curr_node->children.end());

	// Check if the current namespace node exists looking for it in the
	// list of children
	for (; it_child != end_child; ++it_child) {

		// Check if the current namespace to find is ID-based
		std::string res_name = (*it_child)->data->Name();
		size_t id_pos = std::string::npos;
		if (opt == RT_SET_MATCHES)
			id_pos = curr_ns.find_first_of("0123456789");

		// If not (path template search) remove the ID number from the current
		// resource namespace
		if ((opt != RT_EXACT_MATCH) && (id_pos == std::string::npos)) {
			id_pos = res_name.find_first_of("0123456789");
			res_name = res_name.substr(0, id_pos);
		}

		// Namespaces comparison
		if (curr_ns.compare(res_name) != 0)
			continue;

		// Matched. If we're at the end of the path, append the
		// resource descriptor into the list to return.
		if (next_path.empty())
			matches.push_back((*it_child)->data);
		else
			// ... Otherwise continue recursively
			find_node(*it_child, next_path, opt, matches);

		// If the search doesn't require all the matches we can stop
		if ((opt == RT_EXACT_MATCH) || (opt == RT_FIRST_MATCH))
			break;
	}

	// Return true if the list is not empty
	return !matches.empty();
}


ResourceTree::ResourceNode_t *
ResourceTree::add_child(ResourceNode_t * curr_node,
		std::string const & rsrc_name) {
	// Create the new resource node
	ResourceNode_t * _node = new ResourceNode_t;
	_node->data = ResourcePtr_t(new Resource(rsrc_name));

	// Set the parent and the depth
	_node->parent = curr_node;
	_node->depth = curr_node->depth + 1;

	// Append it as child of the current node
	curr_node->children.push_back(_node);
	return _node;
}


void ResourceTree::print_children(ResourceNode_t * _node, int _depth) {
	// Increase the level of depth
	++_depth;

	// Print all the children
	ResourceNodesList_t::iterator it(_node->children.begin());
	ResourceNodesList_t::iterator end(_node->children.end());
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


void ResourceTree::clear_node(ResourceNode_t * _node) {
	// Children iterators
	ResourceNodesList_t::iterator it(_node->children.begin());
	ResourceNodesList_t::iterator end(_node->children.end());

	// Recursive clear
	for (; it != end; ++it) {
		if (!(*it)->children.empty())
			clear_node(*it);
		(*it)->children.clear();
	}
}

}   // namespace res

}   // namespace bbque

