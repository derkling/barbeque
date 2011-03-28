/**
 *       @file  resource_tree.h
 *      @brief  Class for storing resource descriptors in a tree-like way
 *
 * This define a class for storing Resource descriptor in tree structure and
 * allow the lookup by a namespace-like approach (i.e. "arch.clusters.mem0").
 *
 *     @author  Giuseppe Massari (jumanix), joe.massanga@gmail.com
 *
 *   @internal
 *     Created  01/04/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Giuseppe Massari
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#ifndef BBQUE_RESOURCE_TREE_H_
#define BBQUE_RESOURCE_TREE_H_

#include <cstdint>
#include <list>
#include <map>
#include <string>

#include "bbque/res/resources.h"

namespace bbque { namespace res {

struct ResourceNode;
typedef std::list<ResourceNode *> ResourceNodeList_t;

/**
 * @struct ResourceNode
 *
 * The base node of the ResourceTree
 */
struct ResourceNode {

	/** Data node */
	ResourcePtr_t data;

	/** Children nodes */
	ResourceNodeList_t children;

	/** Parent node */
	ResourceNode * parent;

	/** Depth in the tree */
	uint16_t depth;
};


/**
 * @class ResourceTree
 *
 * The class allow the management of the resource descriptors in a hierachical
 * way. The hierarchy is structured as a tree. The access to the content is
 * based on a namespace-like approach (i.e. arch.clusters.cluster0.mem0 ...)
 */
class ResourceTree {

public:

	/**
	 * @brief Constructor
	 */
	ResourceTree();

	/**
	 * @brief Destructor
	 */
	~ResourceTree() {
		_clear_node(root);
	}

	/**
	 * @brief Insert a new resource
	 * @param rsrc_path Resource path
	 */
	ResourcePtr_t insert(std::string const & rsrc_path);

	/**
	 * @brief Find a resource by its pathname
	 * @param rsrc_path Resource path
	 * @return Resource descriptor shared pointer
	 */
	inline ResourcePtr_t find(std::string const & rsrc_path) const {
		return _find(rsrc_path, true);
	}

	/**
	 * @brief Find a resource by its template pathname.
	 *
	 * A template pathname is a path without the resource IDs.
	 * It is used to look for compatible resources.
	 *
	 * @param rsrc_path Resource path
	 * @return True if the path match a resource, false otherwise
	 */
	inline bool match_path(std::string const & rsrc_path) const {
		ResourcePtr_t res = _find(rsrc_path, false);
		if (res.get() != NULL)
			return true;
		return false;
	}

	inline uint16_t depth() {
		return max_depth;
	}

	/**
	 * @brief Print the tree content
	 */
	inline void print_tree() {
		_print_children(root, 0);
		std::cout << std::endl << "Max depth: " << max_depth << std::endl;
	}

	/**
	 * @brief Clear the tree
	 */
	inline void clear() {
		_clear_node(root);
	}

private:

	/** Pointer to the root of the tree*/
	ResourceNode *root;

	/** Maximum depth of the tree */
	uint16_t max_depth;

	/**
	 * @brief Find a resource by its pathname or template path
	 * @param rsrc_path Resource path
	 * @param exact_match If true try to lookup a specific resource, otherwise
	 * look for a compatible match
	 * @return A shared pointer to the resource descriptor found.
	 */
	ResourcePtr_t _find(std::string const & rsrc_path, bool exact_match) const;

	/**
	 * @brief Recursive method for printing nodes content in a tree-like form
	 * @param node Pointer to the starting tree node
	 * @param depth Node depth
	 */
	void _print_children(ResourceNode * node, int depth);

	/**
	 * @brief Clear a node of the tree
	 * @param node Pointer to the node to clear
	 */
	void _clear_node(ResourceNode * node);

};

}   // namespae test

}   // namespace bbque

#endif // BBQUE_RESOURCE_TREE_H_

