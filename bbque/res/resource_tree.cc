/**
 *       @file  resource_tree.cc
 *      @brief  Implementation of the ResourceTree class
 *
 * This provides the implementation of the class managing the storage of
 * resource descriptors.
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


ResourcePtr_t ResourceTree::insert(std::string const & _rsrc_path) {

	// Current node pointer
	ResourceNode * curr_node = root;

	// Extract the first "node" in the resource path
	std::string ns_path	= _rsrc_path;
	std::string curr_ns = PopPathLevel(ns_path);

	// For each namespace in the path...
	while (!curr_ns.empty()) {

		// Current node children list is empty
		if (curr_node->children.size() == 0) {
			// Insert the resource as a child node
			curr_node = _insert_child(curr_node, curr_ns);
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
				curr_node = _insert_child(curr_node, curr_ns);
		}
		// Next node
		curr_ns = PopPathLevel(ns_path);
	}
	// Return the object created for finalizing purpose
	return curr_node->data;
}


ResourcePtr_t ResourceTree::_find(std::string const & _rsrc_path,
		bool exact_match) const {

	ResourcePtr_t null_ptr;
	null_ptr.reset();

	// Root is null
	if (root == NULL) {
		std::cout << "ResourceTree: root is null" << std::endl;
		return null_ptr;
	}
	// Start from root
	ResourceNode * curr_node = root;

	// Extract the first node in the path
	std::string ns_path	= _rsrc_path;
	std::string curr_ns = PopPathLevel(ns_path);

	// True if the namespace in the path matches a level in the resource tree
	bool node_found;

	// Parse the path
	while (!curr_ns.empty()) {

		// Current node children list is empty
		if (curr_node->children.size() == 0)
			return null_ptr;

		std::list<ResourceNode *>::iterator it =
			curr_node->children.begin();
		std::list<ResourceNode *>::iterator end =
			curr_node->children.end();
		node_found = false;

		// Check if the current namespace node exists looking for it in the
		// list of children
		for (; it != end; ++it) {
			// Current node name (child)
			std::string res_name = (*it)->data->Name();

			if (!exact_match) {
				// Remove the ID from the name
				int id = res_name.find_first_of("0123456789");
				res_name = res_name.substr(0, id);
			}

			// Compare to the searching name
			if (curr_ns.compare(res_name) == 0) {
				curr_node = *it;
				node_found = true;
				break;
			}
		}
		// node_found = false means we didn't found any matches at the current
		// level, thus we can nterrupt the search and return
		if (!node_found)
			return null_ptr;

		// Next node
		curr_ns = PopPathLevel(ns_path);
	}
	// Return the resource descriptor successfully
	return curr_node->data;
}


ResourceNode * ResourceTree::_insert_child(ResourceNode * curr_node,
		std::string const & curr_ns) {

	ResourceNode * _node;
	// Create the new resource node
	_node = new ResourceNode;
	_node->data = ResourcePtr_t(new Resource(curr_ns));

	// Set the parent
	_node->parent = curr_node;

	// Set the depth
	_node->depth = curr_node->depth + 1;

	// Append it as child of the current node
	curr_node->children.push_back(_node);
	return _node;
}


void ResourceTree::_print_children(ResourceNode * _node, int _depth) {

	std::list<ResourceNode *>::iterator it = _node->children.begin();
	std::list<ResourceNode *>::iterator end = _node->children.end();

	// Increase the level of depth
	++_depth;

	// Print all the children
	for (; it != end; ++it) {

		for (int i= 0; i < _depth-1; ++i)
			std::cout << "\t";
		std::cout << "|-------" << (*it)->data->Name() << std::endl;

		// Recursive call if there are children
		if (!(*it)->children.empty())
			_print_children(*it, _depth);
	}
}


void ResourceTree::_clear_node(ResourceNode * _node) {

	std::list<ResourceNode *>::iterator it = _node->children.begin();
	std::list<ResourceNode *>::iterator end = _node->children.end();

	// Clear all the children
	for (; it != end; ++it) {

		// Recursive call if there are children
		if (!(*it)->children.empty())
			_clear_node(*it);
		(*it)->children.clear();
	}
}

}   // namespace res

}   // namespace bbque

