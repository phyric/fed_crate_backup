#include "Crate.h"
#include "VMEModule.h"
#include "VMEController.h"
#include "CrateSetup.h"
#include "Singleton.h"
#include "DDU.h"
#include "DCC.h"


Crate::Crate(int number, VMEController * controller) : 
  theNumber(number),  
  theModules(24),
  theController(controller) 
{
  Singleton<CrateSetup>::instance()->addCrate(number, this);
}


Crate::~Crate() {
  for(unsigned i = 0; i < theModules.size(); ++i) {
    delete theModules[i];
  }
  delete theController;
}


void Crate::addModule(VMEModule * module) {
  theModules[module->slot()] = module;
}


vector<DDU *> Crate::ddus() const {
  vector<DDU *> result;
  for(unsigned i = 0; i < theModules.size(); ++i) {
    DDU * ddu = dynamic_cast<DDU *>(theModules[i]);
    if(ddu != 0) result.push_back(ddu);
  }
  return result;
}

vector<DCC *> Crate::dccs() const {
  vector<DCC *> result;
  for(unsigned i = 0; i < theModules.size(); ++i) {
    DCC * dcc = dynamic_cast<DCC *>(theModules[i]);
    if(dcc != 0) result.push_back(dcc);
  }
  return result;
}

