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
    printf("Hello123 \n");
}

void EvidenceStorage::add(Evidence* ev) {
    evidence_.push_back(ev);
}

unsigned int EvidenceStorage::size() const {
    return evidence_.size();
}

const Time& EvidenceStorage::getTimestamp() const {
        return timestamp_;
    }


std::vector<Evidence*>::const_iterator EvidenceStorage::begin() const {
    return evidence_.begin();
}

std::vector<Evidence*>::const_iterator EvidenceStorage::end() const {
    return evidence_.end();
}

}
