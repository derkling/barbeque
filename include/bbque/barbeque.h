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

#ifndef BBQUE_BARBEQUE_H_
#define BBQUE_BARBEQUE_H_

#include <cstdlib>
#include <iostream>

#include "bbque/config.h"
#include "bbque/version.h"

/**
 * \mainpage Introduction
 *
 * Resource management is becoming one of the most challenging topics for a
 * proper exploitation of upcoming many-core computing devices. These
 * devices, which are represented in first instance by general purpose GPUs
 * (GPGPUs), are characterized by an increasing number of symmetric
 * processing element (PE) which exposes a SIMD programming model allowing
 * to execute concurrently the same kernel code on a wide input data-set.
 *
 * This kind of massive data parallelization allows to speed-up the overall
 * processing time of a give workload by splitting the computational effort
 * among multiple hardware processing resources. Meanwhile new programming
 * paradigms and standards, like OpenCL, have been developed to extend the
 * functional capabilities of existing programming languages, such as for
 * example C/C++ and Fortran, and to support the developer on exploiting the
 * computational capabilities of these parallel processing devices.
 *
 * The <strong>BarbequeRTRM</strong>, which has been completely designed and
 * developed from scratch wihtin the context of the
 * <a href="http://www.2parma.eu">2PARMA Project</a>,
 * is the core of an <em>highly modular and extensible run-time resource
 * manager</em> which provide support for an easy integration and management
 * of multiple applications competing on the usage of one (or more) shared
 * MIMD many-core computation devices.
 *
 * The framework design, which exposes different plugin interfaces, provides
 * support for pluggable policies for both <em>resource scheduling</em> and
 * the management of <em>applications coordination and reconfiguration</em>.
 * Applications integrated with the frameworks gets “for-free” a suitable
 * instrumentation to support <em>Design-Space-Exploration (DSE)</em>
 * techniques, which could be used to profile application behaviors to either
 * optimize them at design time or support the identification of optimal QoS
 * requirements goals as well as their run-time monitoring. Suitable
 * <em>platform abstraction layers</em>, built on top of the Linux kernel
 * interfaces, allows an easily porting of the framework on different
 * platforms and its integration with specific execution environments such as
 * the Android run-time.
 *
 * Based on all these features the framework allows an easily coding of
 * resource management policies which support an optimized assignment of
 * resources to demanding applications considering:
 * - application properties, e.g. run-time requirements, operating modes
 *   and relative priorities;
 * - resources availability and state, e.g. power and thermal
 *   conditions;
 * - tunable run-time optimization goals, e.g. power reduction, energy
 *   optimization, reconfiguration overheads minimization and overall
 *   performances maximization.
 *
 * You could get an overall view of the framework capabilities by looking at
 * this screencast:<br>
 * <a href="http://youtu.be/B1TDNbtIKC8">
 *   2PARMA Project Demo - BarbequeRTRM v0.6 (Angus)
 * </a><br>
 * where the framework (last stable version, codename Angus) is showed in
 * action to manage a mixed workload with different resource requirements
 * and priorities.
 *
 * For further details on this project, please refers to these pages:
 * - @ref sec00_idx
 * - @ref sec00_src
 * - @ref sec01_ow
 */

/**
 * \page sec00_idx Organization of this Documentation
 *
 * The BarbequeRTRM comes with system deamon (barbeque) and a Run-Time Library
 * (RTLib)...<br>
 * This documentation provides a detailed description of the publics as well
 * as internal BarbequeRTRM API.
 *
 * If you are interested into the integration of new applications with the
 * BarbequeRTRM, than you should focus on the usage of the \ref rtlib and we
 * suggest you to start reading the following sections, preferably in this
 * order:
 * -# @ref rtlib_sec01_ow
 * -# @ref rtlib_sec02_aem
 * -# @ref rtlib_sec03_plain
 *
 * If instead you are interested into hacking the run-time manager or develop new
 * plugins, read following sections, preferably in this order:
 * -# @ref sec01_ow
 * -# @ref sec02_cm
 * -# @ref sec03_am
 * -# @ref sec04_rm
 * -# @ref sec05_sm
 * -# @ref sec06_ym
 *
 * The internals of the integration with applications, the integration with
 * target platforms as well as some utility functions available both for the
 * core and RTLib are discussed in these section:
 * - @ref sec10_ap
 * - @ref sec20_pp
 * - @ref sec40_ut
 * - @ref sec50_pl
 */

/**
 * \page sec00_src Getting the Source Code
 *
 * To get the Barbeque Open Source Project sources, you’ll need to get familiar
 * with Git and Repo.<br>
 * A good reference and starting point is provided by the Android
 * <a href="http://www.omappedia.org/wiki/Android_Miscellaneous">wiki</a>.
 *
 *
 * \section sec00_src_repo Installing Repo
 *
 * You need a working repo binary in order to setup the BOSP repository. You
 * could get one by downloading it from the Android repository and installing it
 * locally (e.g. in a @c bin sub-folder of you home directory). Than, ensure to
 * make it executable:
 * \verbatim
 $ curl https://dl-ssl.google.com/dl/googlesource/git-repo/repo > ~/bin/repo
 $ chmod a+x ~/bin/repo \endverbatim
 *
 *
 * \section sec00_src_wd Creating a Working Directory
 *
 * The complete BOSP sources are downloaded and compiled into a user-defined
 * working directory. Thus, start by creating such a folder, wherever you prefer
 * and enter it:
 * \verbatim
 $ cd ~
 $ mkdir working-directory-name
 $ cd working-directory-name \endverbatim
 *
 * For the purposes of this tutorial, thereafter I will assume your working
 * directory is @c /BOSP
 *
 *
 * \section sec00_src_init Initialize repo in your working directory
 *
 * To initialize your local repository using the Barbeque Open Source Project
 * trees, use this command from inside your working directory:
 * \verbatim
 $ repo init -u https://bitbucket.org/bosp/manifest.git -b master \endverbatim
 *
 * where @c -b is the command line option for branch, and I'm assuming you want the
 * ‘master’ branch.<br>
 * This command will download the BOSP manifest file which contains all the
 * information to download the required source trees.
 * At the end, this command ask also you some information about your name and
 * email. These are pre-configured to allows you to submit patches to the
 * project.
 *
 *
 * \section sec00_src_sync Sync up with the remote repository
 *
 * Once the manifest has been initialized, you could /synchronize/ you local
 * BOSP source tree with the main repository. This allows to download all the
 * last version of the provided source packages. To that purpose, form within
 * you working directory, simply run:
 * \verbatim
 $ repo sync \endverbatim
 *
 * This synchronization downloads all the most recent version of the sources,
 * for each library and code related to the BOSP.
 * Once the synchronization has completed, you get a complete building tree of
 * all required component.
 *
 *
 * \section sec00_src_req Compilation requirements
 *
 * The BOSP is quite self-contained, indeed it cares about download,
 * configure and install not only the Barbeque RTRM framework itself, but also
 * required libraries as well. However, at least for the time being, some tools
 * are still required to successfully build all these projects.
 *
 * This is a list of mandatory tools:
 * - Essential builing tools (e.g., the “build-essential” packages on Debian
 *    based systems)
 * - GIT
 * - CMake (&gt;= v2.6)
 * - Autotools (i.e. autoconf, automake and libtoolize)
 * - Doxygen
 * - GCC/G++ and libstdc++-v3 (both not older than v4.5)
 *
 * For example, in Ubuntu 11.10 you could get a working environment
 * suitable for BOSP compilation running this command:
 * \verbatim
 $ aptitude install build-essential autoconf automake libtool cmake \
	 git-core doxygen graphviz \endverbatim
 *
 * The provided building system will do a check for these components
 * at the beginning and should inform you about missing ones.

 * Please \b note that (for the time being) the BOSP
 * building system does not provide cross compilation support: thus, it
 * is still possible to cross-compile each component but this must be
 * done by hand.
 *
 * For further instructions on how to compile and use the BOSP, please refers
 * to the <strong>Readme.txt</strong> file you could find in your working
 * directory, after a synchronization has completed.  While, to have an
 * introducetion on the framework architecture have a look at the section
 * \ref sec01_ow.
 */

/**
 * \page sec01_ow Overall Arcitecture View
 *
 * The Barbeque RTRM is a modular and efficient resource allocator for
 * many-core clustered accelerator engines, which provide support for:
 * - mixed workload management
 * - dynamic resource partitioning
 * - resources abstraction
 * - multi-objectve optimization policy
 * - low-overheads run-time management
 *
 * Modern many-core computing platform are targeted to the execution of a
 * <em>mixed-workload</em> where multiple applications, with different
 * requirements and criticality levels, run concurrently thus competing on
 * accessing a reduce set of shared resource. Provided that each of these
 * application is associated with a proper description, which should specify
 * the mapping between an expected Quality-of-Service (QoS) level and the
 * correposnding resources demand, the Barbeque RTRM allows to arrange for an
 * "optimal" resources allocation to all the active applications.
 *
 * The main goal of the <em>dynamic resource partitioning</em> support is to
 * grant resources to critical workloads while dynamically yield these
 * resources to best-effort workloads when (and only while) they are not
 * required by critical ones, thus optimize resource usage and fairness.
 *
 * In order to improve applications portability among different acceleration
 * platforms, the Barbeque RTRM provides support for <em>resource
 * abstraction</em> which allows to exploit a decoupled perspective of the
 * resources between the users and the underlying hardware.  The user
 * applications will see virtual resources, e.g., the number of processing
 * elements available, but they will not be aware of which of the physical
 * resources are effectively available. At run-time the RTRM will perform
 * the virtual-to-physical mapping according to the current <em>multi-objective
 * optimization function</em> (low power, high performance, etc..) and run-time
 * phenomena (process variation, temporal and spatial temperature gradients,
 * hardware failures and, above all, workload variation).
 *
 * Finally, introducing a low overhead on managing resource at run-time, while
 * still granting a valuable solution for the aformentioned issues is one more
 * goal of the Barbeque RTRM framework. This goals also as required the
 * development if this RTLib run-time library which applications should
 * exploit in order to take advantages from the Barbeque RTRM.
 *
 *
 * ADD MORE DETAILS HERE
 *
 * @}
 */

#endif // BBQUE_BARBEQUE_H_
