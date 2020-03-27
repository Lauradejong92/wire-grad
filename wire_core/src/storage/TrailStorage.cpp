/*
 * EvidenceStorage.cpp
 *
 *  Created on: februari 7, 2020
 *      Author: ls de Jong
 *      origin: sdries
 */

#include "wire/core/EvidenceSet.h"
#include "wire/core/Evidence.h"
#include "wire/storage/TrailStorage.h"
#include "wire/core/Property.h"
#include "problib/conversions.h"

namespace mhf {

TrailStorage::TrailStorage() : timestamp_(-1) {
}

TrailStorage* TrailStorage::instance_ = 0;

TrailStorage& TrailStorage::getInstance() {
        if (instance_) {
            return *instance_;
        }
        instance_ = new TrailStorage();
        return *instance_;
    }

    TrailStorage::~TrailStorage() {
    }

//const Evidence* TrailStorage::getEvidence (int set, int place) const {
//        return &clusterSet_[set][place];
//    }



void TrailStorage::add(std::vector<Evidence> ev_set) {
    trailSet_.push_back(ev_set);
}

void TrailStorage::clear() {
        trailSet_.clear();
    }

unsigned int TrailStorage::size() const {
    return trailSet_.size();
}

const Time& TrailStorage::getTimestamp() const {
        return timestamp_;
    }

const std::vector<std:: vector<Evidence>> TrailStorage::getTrail() const {
    return trailSet_;
}


    std::vector<std:: vector<Evidence>>::const_iterator TrailStorage::begin() const {
    return trailSet_.begin();
}

    std::vector<std:: vector<Evidence>>::const_iterator TrailStorage::end() const {
    return trailSet_.end();
}

}
