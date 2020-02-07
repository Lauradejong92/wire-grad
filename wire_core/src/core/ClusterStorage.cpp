/*
 * EvidenceStorage.cpp
 *
 *  Created on: februari 7, 2020
 *      Author: ls de Jong
 *      origin: sdries
 */

#include "wire/core/EvidenceSet.h"
#include "wire/core/Evidence.h"
#include "wire/core/ClusterStorage.h"
#include "wire/core/Property.h"
#include "problib/conversions.h"

namespace mhf {

ClusterStorage::ClusterStorage() : timestamp_(-1) {
}

ClusterStorage* ClusterStorage::instance_ = 0;

ClusterStorage& ClusterStorage::getInstance() {
        if (instance_) {
            return *instance_;
        }
        instance_ = new ClusterStorage();
        return *instance_;
    }


ClusterStorage::~ClusterStorage() {
}

void ClusterStorage::add(EvidenceSet* ev_set) {
   clusterSet_.push_back(ev_set);

}

unsigned int ClusterStorage::size() const {
    return clusterSet_.size();
}

const Time& ClusterStorage::getTimestamp() const {
        return timestamp_;
    }


std::vector<EvidenceSet*>::const_iterator ClusterStorage::begin() const {
    return clusterSet_.begin();
}

std::vector<EvidenceSet*>::const_iterator ClusterStorage::end() const {
    return clusterSet_.end();
}

}
