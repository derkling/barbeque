/**
 *       @file  resource_tree.h
 *      @brief  Resource Tree for resource descriptors
 *
 * This defines a class for storing Resource descriptors in a tree based
 * structure. Such class allow the lookup of descriptors using a
 * namespace-like approach (i.e.  "arch.clusters.mem0").
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

// Forward declaration
struct ResourceNode;

/** Shared pointer to ResourceNode */
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
	 * @enum SearchOptions_t
	 *
	 * Enumerate all the possible search options for the resource descriptors
	 * in the tree
	 */
	enum SearchOption_t {
		/**
		 * Lookup exactly the descriptor of the resource specified in the
		 * path
		 */
		RT_EXACT_MATCH = 0,
		/**
		 * Lookup the first resource descriptor having a path compatible
		 * with the template specified
		 */
		RT_FIRST_MATCH,
		/**
		 * Return (in a list) all the descriptors matching the template path
		 * to search
		 */
		RT_ALL_MATCHES
	};

	/**
	 * @brief Constructor
	 */
	ResourceTree();

	/**
	 * @brief Destructor
	 */
	~ResourceTree() {
		clear_node(root);
	}

	/**
	 * @brief Insert a new resource
	 * @param rsrc_path Resource path
	 * @return A shared pointer to the resource descriptor just created
	 */
	ResourcePtr_t & insert(std::string const & rsrc_path);

	/**
	 * @brief Find a resource by its pathname
	 *
	 * This returns a descriptor (field 'data' in the ResourceNode)
	 * of the of the resource specified in the path.
	 *
	 * @param rsrc_path Resource path
	 * @return A shared pointer to the resource descriptor found
	 */
	inline ResourcePtr_t find(std::string const & rsrc_path) const {
		// Null shared pointer
		ResourcePtr_t null_ptr;
		null_ptr.reset();

		// List of matches to be filled
		std::list<ResourcePtr_t> matches;
		if (find_node(root, rsrc_path, RT_EXACT_MATCH, matches))
			return (*(matches.begin()));
		return null_ptr;
	}

	/**
	 * @brief Find all the resources matching a template path
	 *
	 * It fills a list with all the resource descriptors matching the template
	 * path.
	 *
	 * @param temp_path Template path to match
	 * @return The list of the resource descriptors
	 */
	inline std::list<ResourcePtr_t> findAll(std::string const & temp_path)
		const {
		// List of matches to return
		std::list<ResourcePtr_t> matches;

		// Start the recursive search
		find_node(root, temp_path, RT_ALL_MATCHES, matches);
		return matches;
	}

	/**
	 * @brief Check resource existance by its template pathname.
	 *
	 * A template pathname is a path without the resource IDs.
	 * It is used to look for compatible resources.
	 *
	 * @param temp_path Resource path
	 * @return True if the path match a resource, false otherwise
	 */
	inline bool existPath(std::string const & temp_path) const {
		// List of matches to be filled
		std::list<ResourcePtr_t> matches;

		// Start the recursive search
		return find_node(root, temp_path, RT_FIRST_MATCH, matches);
	}

	/**
	 * @brief Maximum depth of the tree
	 * @return The maxim depth value
	 */
	inline uint16_t depth() {
		return max_depth;
	}

	/**
	 * @brief Print the tree content
	 */
	inline void printTree() {
		print_children(root, 0);
		std::cout << std::endl << "Max depth: " << max_depth << std::endl;
	}

	/**
	 * @brief Clear the tree
	 */
	inline void clear() {
		clear_node(root);
	}

private:

	/** Pointer to the root of the tree*/
	ResourceNode * root;

	/** Maximum depth of the tree */
	uint16_t max_depth;

	/**
	 * @brief Find a node by its pathname or template path
	 *
	 * This manages three types of search (@see SearchOption_t):
	 * 1) An exact matching for looking up a well defined resource descriptor.
	 * 2) A template matching for checking the existance of a resource
	 * compatible with the template path specified
	 * 3) A matching of all the resource descriptors matching the template
	 * path
	 *
	 * @param curr_node The root node from which start
	 * @param rsrc_path Resource path (or template path)
	 * @param opt Specify the type of search (@see SearchOption_t)
	 * @param matches A list to fill with the descriptors matching the path.
	 *
	 * @return True if the search have found some matchings.
	 */
	bool find_node(ResourceNode * curr_node, std::string const & rsrc_path,
			SearchOption_t opt, std::list<ResourcePtr_t> & matches) const;

	/**
	 * @brief Append a child to the current node
	 * @param curr_node Current resource node
	 * @param curr_ns Current pathlevel (namespace) name
	 * @return The child node just created
	 */
	ResourceNode * insert_child(ResourceNode * curr_node,
			std::string const & curr_ns);

	/**
	 * @brief Recursive method for printing nodes content in a tree-like form
	 * @param node Pointer to the starting tree node
	 * @param depth Node depth
	 */
	void print_children(ResourceNode * node, int depth);

	/**
	 * @brief Clear a node of the tree
	 * @param node Pointer to the node to clear
	 */
	void clear_node(ResourceNode * node);

};

}   // namespace res

}   // namespace bbque

#endif // BBQUE_RESOURCE_TREE_H_

