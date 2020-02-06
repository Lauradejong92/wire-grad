/*
 * EvidenceStorage.cpp
 *
 *  Created on: februari 3, 2020
 *      Author: ls de Jong
 *      origin: sdries
 */

#include "wire/core/EvidenceSet.h"
#include "wire/core/Evidence.h"
#include "wire/core/EvidenceStorage.h"

namespace mhf {

EvidenceStorage::EvidenceStorage() : timestamp_(-1) {
}

EvidenceStorage* EvidenceStorage::instance_ = 0;

EvidenceStorage& EvidenceStorage::getInstance() {
        if (instance_) {
            return *instance_;
        }
        instance_ = new EvidenceStorage();
        return *instance_;
    }


EvidenceStorage::~EvidenceStorage() {
}

void EvidenceStorage::hello123() {
    printf("Set consists of: \n");

    for(std::vector<EvidenceSet*>::const_iterator it_set = EvidenceStorage().getInstance().begin(); it_set != EvidenceStorage().getInstance().end(); ++it_set) {
        EvidenceSet* my_set = new EvidenceSet(**it_set);
        printf("Set at: %i \n",my_set->size());
        printf("Time  : %f \n",my_set->getTimestamp());

        for(EvidenceSet::const_iterator it_ev = my_set->begin(); it_ev != my_set->end(); ++it_ev) {
            printf("    -Evidence at time: %f \n",it_ev );
        }

    }
}

void EvidenceStorage::add(EvidenceSet* ev_set) {
    int setsize= 5; //Number of points in historic cluster

    evidenceSet_.push_back(ev_set);

    if (evidenceSet_.size()>setsize){
        evidenceSet_.erase(evidenceSet_.begin());
    }

}

unsigned int EvidenceStorage::size() const {
    return evidenceSet_.size();
}

const Time& EvidenceStorage::getTimestamp() const {
        return timestamp_;
    }


std::vector<EvidenceSet*>::const_iterator EvidenceStorage::begin() const {
    return evidenceSet_.begin();
}

std::vector<EvidenceSet*>::const_iterator EvidenceStorage::end() const {
    return evidenceSet_.end();
}

}
