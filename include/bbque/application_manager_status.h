/**
 *       @file  application_manager_status.h
 *      @brief  Application manager "read-only" status interface
 *
 * This defines the interface of the Application Manager component for
 * querying the application runtime information.
 * Currently we are interested in getting a specific application descriptor,
 * the lowest priority level managed, and maps of application descriptors,
 * even querying by scheduling status or priority level.
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

#ifndef BBQUE_APPLICATION_MANAGER_STATUS_IF_H_
#define BBQUE_APPLICATION_MANAGER_STATUS_IF_H_

#include "bbque/app/application.h"

using bbque::app::ApplicationStatusIF;
using bbque::app::AppPrio_t;

namespace bbque {

/** A shared pointer to the Application descriptor */
typedef std::shared_ptr<app::Application> AppPtr_t;

/**
 * Map containing shared pointers to Application descriptors, where the key is
 * the application PID
 */
typedef std::map<AppUid_t, AppPtr_t > AppsUidMap_t;

/**
 * Map containing shared pointers to Application descriptors, where the key is
 * the application PID
 */
typedef std::multimap<AppPid_t, AppPtr_t > AppsMap_t;

/**
 * An entry of the Application Map
 */
typedef std::pair<AppPid_t, AppPtr_t> AppsMapEntry_t;

/**
 * An entry of the UIDS Map
 */
typedef std::pair<AppUid_t, AppPtr_t> UidsMapEntry_t;


/*******************************************************************************
 *     In-Loop Erase Safe Iterator support
 ******************************************************************************/

// Forward declaration
class AppsUidMapIt;

/**
 * @brief Retainer list of ILES iterators
 * @see AppsUidMapIt
 */
typedef std::list<struct AppsUidMapIt*> AppsUidMapItRetainer_t;

/**
 * @brief "In Loop Erase Safe" ILES iterator on an AppsUidMap_t maps
 *
 * This is an iterator wrapper object which is used to implement safe iterations on
 * mutable maps, where an erase could occours on a thread while another thread
 * is visiting the elements of the same container container.
 * A proper usage of such an ILES interator requires to visit the container
 * elements using a pair of provided functions "GetFirst" and "GetNext".
 * @see GetFirst, GetNext
 */
class AppsUidMapIt {

public:

	AppsUidMapIt() :
		updated(false),
		ret(NULL) {
	};
	~AppsUidMapIt() {
		Release();
	};

private:

	/** The map to visit */
	AppsUidMap_t *map;
	/** An interator on a UIDs map */
	AppsUidMap_t::iterator it;
	/** A flag to track iterator validity */
	bool updated;
	/** The retantion list on which this has been inserted */
	AppsUidMapItRetainer_t *ret;

	void Init(AppsUidMap_t & m, AppsUidMapItRetainer_t & rl) {
		map = &m;
		ret = &rl;
		it = map->begin();
	}
	void Retain() {
		ret->push_front(this);
	};
	void Release() {
		if (ret) ret->remove(this);
		ret = NULL;
	};
	void Update() {
		++it; updated = true;
	};
	void operator++(int) {
		if (!updated) ++it;
		updated = false;	
	};
	bool End() {
		return (it == map->end());
	};
	AppPtr_t Get() {
		return (*it).second;
	};

	friend class ApplicationManager;
};


/*******************************************************************************
 *     Application Manager Status Interface
 ******************************************************************************/

/**
 * @class ApplicationManagerStatusIF
 * @brief "Read-only" interface for ApplicationManager status
 */
class ApplicationManagerStatusIF {

public:

	/**
	 * @enum Exit code to return
	 */
	enum ExitCode_t {
		/** Success */
		AM_SUCCESS = 0,
		/** Application Execution Context not found */
		AM_EXC_NOT_FOUND,
		/** Execution of a method interrupted by an unexpected state in an
		 * internal data structure state */
		AM_DATA_CORRUPT,
		/** Method forced to exit */
		AM_ABORT
	};

	/**
	 * @brief Get the first element of UIDs map using an ILES iterator.
	 *
	 * Each time a module requires to visit the UID applications map, should
	 * use a pair of methods which ensure a proper handling of the container
	 * iterator. Indeed, for efficincy purposes, all the Barbeque containers
	 * are mutable and use fine-grained locking to better exploit the
	 * framework parallelism.
	 * Visiting a container requires to get its elements using a pair of
	 * function. This function is used to start the iteration thus getting the
	 * reference to the first element (if any).
	 *
	 * @param ait an ILES iterator instance, this is the same ILES iterator to
	 * be used to get the next elements
	 *
	 * @return a reference to the first application in the UIDs queue, or NULL
	 * if no applications are present.
	 *
	 * @see GetNext
	 */
	virtual AppPtr_t GetFirst(AppsUidMapIt & ait) = 0;

	/**
	 * @brief Get the next element of UIDs map using an ILES iterator.
	 *
	 * Visiting a Barbeque mutable container requires to get its elements
	 * using a pair of function. This function is used to continue a
	 * previously started iteration thus getting a reference to the next
	 * element (if any).
	 *
	 * @param ait an ILES iterator instance, this is the same ILES iterator
	 * used to get the first elements
	 *
	 * @return a reference to the next application in the UIDs queue, or NULL
	 * if no applications are present.
	 *
	 * @see GetFirst
	 */
	virtual AppPtr_t GetNext(AppsUidMapIt & ait) = 0;

	/**
	 * @brief Get the next element of PRIO queue using an ILES iterator.
	 *
	 * @param prio the priority queue to visit
	 * @param ait an ILES iterator instance, this is the same ILES iterator to
	 * be used to get the next elements
	 *
	 * @return a reference to the first application in the specified PRIO
	 * queue, or NULL if no applications are present.
	 */
	virtual AppPtr_t GetFirst(AppPrio_t prio, AppsUidMapIt & ait) = 0;

	/**
	 * @brief Get the next element of PRIO queue using an ILES iterator.
	 *
	 * @param prio the priority queue to visit
	 * @param ait an ILES iterator instance, this is the same ILES iterator
	 * used to get the first elements
	 *
	 * @return a reference to the next application in the UIDs queue, or NULL
	 * if no applications are present.
	 */
	virtual AppPtr_t GetNext(AppPrio_t prio, AppsUidMapIt & ait) = 0;

	/**
	 * @brief Get the next element of STATUS queue using an ILES iterator.
	 *
	 * @param prio the status queue to visit
	 * @param ait an ILES iterator instance, this is the same ILES iterator to
	 * be used to get the next elements
	 *
	 * @return a reference to the first application in the specified STATUS
	 * queue, or NULL if no applications are present.
	 */
	virtual AppPtr_t GetFirst(ApplicationStatusIF::State_t state,
			AppsUidMapIt & ait) = 0;

	/**
	 * @brief Get the next element of STATUS queue using an ILES iterator.
	 *
	 * @param prio the status queue to visit
	 * @param ait an ILES iterator instance, this is the same ILES iterator
	 * used to get the first elements
	 *
	 * @return a reference to the next application in the STATUS queue, or NULL
	 * if no applications are present.
	 */
	virtual AppPtr_t GetNext(ApplicationStatusIF::State_t state,
			AppsUidMapIt & ait) = 0;

	/**
	 * @brief Get the next element of SYNC queue using an ILES iterator.
	 *
	 * @param prio the sync queue to visit
	 * @param ait an ILES iterator instance, this is the same ILES iterator to
	 * be used to get the next elements
	 *
	 * @return a reference to the first application in the specified SYNC
	 * queue, or NULL if no applications are present.
	 */
	virtual AppPtr_t GetFirst(ApplicationStatusIF::SyncState_t state,
			AppsUidMapIt & ait) = 0;

	/**
	 * @brief Get the next element of SYNC queue using an ILES iterator.
	 *
	 * @param prio the sync queue to visit
	 * @param ait an ILES iterator instance, this is the same ILES iterator
	 * used to get the first elements
	 *
	 * @return a reference to the next application in the SYNC queue, or NULL
	 * if no applications are present.
	 */
	virtual AppPtr_t GetNext(ApplicationStatusIF::SyncState_t state,
			AppsUidMapIt & ait) = 0;

	/**
	 * @brief Check if the specified PRIO queue has applications
	 */
	virtual bool HasApplications (AppPrio_t prio) = 0;

	/**
	 * @brief Check if the specified STATUS queue has applications
	 */
	virtual bool HasApplications (ApplicationStatusIF::State_t state) = 0;

	/**
	 * @brief Check if the specified SYNC queue has applications
	 */
	virtual bool HasApplications (ApplicationStatusIF::SyncState_t state) = 0;

	/**
	 * @brief Retrieve an application descriptor (shared pointer) by PID and
	 * Excution Context
	 * @param pid Application PID
	 * @param exc_id Execution Contetx ID
	 */
	virtual AppPtr_t const GetApplication(AppPid_t pid,
			uint8_t exc_id) = 0;

	/**
	 * @brief Retrieve an application descriptor (shared pointer) by UID
	 *
	 * @param uid Application UID
	 */
	virtual AppPtr_t const GetApplication(AppUid_t uid) = 0;

	/**
	 * @brief Lowest application priority
	 * @return The maximum integer value for the (lowest) priority level
	 */
	virtual app::AppPrio_t LowestPriority() const = 0;

	/**
	 * @brief Dump a logline to report all applications status
	 */
	virtual void PrintStatusReport() = 0;

};

} // namespace bbque

#endif // BBQUE_APPLICATION_MANAGER_STATUS_IF_H_

