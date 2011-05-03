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
#include "bbque/res/resources.h"
#include "bbque/utils/utility.h"

namespace bbque { namespace res {


ResourceTree::ResourceTree():
	max_depth(0) {

	root = new ResourceNode;
	root->data = ResourcePtr_t(new Resource("root"));
	root->parent = NULL;
	root->depth = 0;
}


ResourcePtr_t & ResourceTree::insert(std::string const & _rsrc_path) {

	// Current node pointer
	ResourceNode * curr_node = root;

	// Extract the first "node" in the resource path
	std::string ns_path	= _rsrc_path;
	std::string curr_ns = SplitAndPop(ns_path);

	// For each namespace in the path...
	while (!curr_ns.empty()) {
		// Current node children list is empty
		if (curr_node->children.empty()) {
			// Insert the resource as a child node
			curr_node = insert_child(curr_node, curr_ns);
			// Update tree depth
			if (curr_node->depth > max_depth)
				max_depth = curr_node->depth;
		}
		else {
			// Can we insert it as a children sibling ?
			std::list<ResourceNode *>::iterator it =
				curr_node->children.begin();
			std::list<ResourceNode *>::iterator end =
				curr_node->children.end();

			// Check if the current resource exists yet between the current
			// node children
			for (; it != end; ++it) {
				// If yes, descends one level, continuing from the current
				// child node
				if ((*it)->data->Name().compare(curr_ns) == 0) {
					curr_node = *it;
					break;
				}
			}
			// If not, add the resource as a children sibling
			if (it == end)
				curr_node = insert_child(curr_node, curr_ns);
		}
		// Next namespace in the path
		curr_ns = SplitAndPop(ns_path);
	}
	// Return the new object just created
	return curr_node->data;
}


bool ResourceTree::find_node(ResourceNode * curr_node,
		std::string const & rsrc_path, SearchOption_t opt,
		std::list<ResourcePtr_t> & matches) const {

	if (curr_node == NULL)
		return false;

	// Save the input path
	std::string next_path = rsrc_path;

	// Current namespace to match:
	// Extract the first node in the path, and save the remaining path string
	std::string curr_ns = SplitAndPop(next_path);

	// Parse the path
	if (!curr_ns.empty()) {

		// Current node children list is empty
		if (curr_node->children.empty())
			return 0;

		// Check if the current namespace node exists looking for it in the
		// list of children
		std::list<ResourceNode *>::iterator it_child =
			curr_node->children.begin();
		std::list<ResourceNode *>::iterator end_child =
			curr_node->children.end();

		for (; it_child != end_child; ++it_child) {
			// Current resource namespace
			std::string res_name = (*it_child)->data->Name();

			// RT_SET_MATCHES:
			// Check if the current namespace to find is ID-based
			int16_t id_pos;
			if (opt == RT_SET_MATCHES)
				// If curr_id_pos == -1 => not ID-based
				id_pos = curr_ns.find_first_of("0123456789");

			// If the search is not ID based (path template search)
			if ((opt == RT_FIRST_MATCH) || (opt == RT_ALL_MATCHES)
				|| ((opt == RT_SET_MATCHES) && (id_pos == -1))) {
				// Remove the ID from the current resource namespace
				id_pos = res_name.find_first_of("0123456789");
				res_name = res_name.substr(0, id_pos);
			}

			// Compare the current namespace to find to the current resource
			// node namespace
			if (curr_ns.compare(res_name) == 0) {
				// If we are at the end of the resource path to lookup...
				if (next_path.empty())
					// Add the resource descriptor to the list
					matches.push_back((*it_child)->data);
				else
					// Continue recursively
					find_node(*it_child, next_path, opt, matches);

				// If the search doesn't require all the matches we can stop it
				if ((opt == RT_EXACT_MATCH) || (opt == RT_FIRST_MATCH))
					break;
			}
		}
	}
	// Return true if the list is not empty
	return !matches.empty();
}


ResourceNode * ResourceTree::insert_child(ResourceNode * curr_node,
		std::string const & curr_ns) {

	// Create the new resource node
	ResourceNode * _node = new ResourceNode;
	_node->data = ResourcePtr_t(new Resource(curr_ns));

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

	// Print all the children
	std::list<ResourceNode *>::iterator it = _node->children.begin();
	std::list<ResourceNode *>::iterator end = _node->children.end();

	for (; it != end; ++it) {
		// Print the child name
		for (int i= 0; i < _depth-1; ++i)
			std::cout << "\t";
		std::cout << "|-------" << (*it)->data->Name() << std::endl;

		// Recursive call if there are some children
		if (!(*it)->children.empty())
			print_children(*it, _depth);
	}
}


void ResourceTree::clear_node(ResourceNode * _node) {

	std::list<ResourceNode *>::iterator it = _node->children.begin();
	std::list<ResourceNode *>::iterator end = _node->children.end();

	for (; it != end; ++it) {
		// Recursive call if there are children
		if (!(*it)->children.empty())
			clear_node(*it);
		// Clear the children list
		(*it)->children.clear();
	}
}

}   // namespace res

}   // namespace bbque

