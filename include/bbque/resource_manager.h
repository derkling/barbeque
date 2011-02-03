/**
 *       @file  resource_manager.h
 *      @brief  The Barbeque Run-Time Resource Manager
 *
 * This class provides the implementation of the Run-Time Resource Manager
 * (RTRM), which is the main barbeque module implementing its glue code.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  02/01/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */


#ifndef BBQUE_RESOURCE_MANAGER_H_
#define BBQUE_RESOURCE_MANAGER_H_

namespace bbque {

class ResourceManager {

public:
  static ResourceManager & GetInstance();
  void Go();
  
private:
  ResourceManager();
  ~ResourceManager();

  void ControlLoop();

private:
  bool done;

};

} // namespace bbque

#endif // BBQUE_RESOURCE_MANAGER_H_

