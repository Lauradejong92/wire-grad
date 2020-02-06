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
#include "wire/core/Property.h"

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

void EvidenceStorage::cluster() {
    int setsize= 5;

    if (evidenceSet_.size()>=setsize){
        printf("Start clustering: \n");

        //For oldest set (= at time-setsize)
        EvidenceSet* origin_set = new EvidenceSet(**evidenceSet_.begin());
        for(EvidenceSet::const_iterator it_ev = origin_set->begin(); it_ev != origin_set->end(); ++it_ev) {
            //Evidence& seed = **it_ev;
            Evidence* seed = *it_ev;
            const Property* my_prop_e = seed->getProperty("position");
            const pbl::PDF& origin_pos = my_prop_e->getValue();


            std::cout << "Evidence: "<< my_prop_e->toString() << std::endl;
            //std::string mystring = my_prop_e->getValue().toString();
            //printf("so: %s \n",mystring);
            //std::cout << "Evidence: "<< seed->toString()<< std::endl;
            //std::vector<double *> listing= my_prop_e->getValue();


        }

        // zoek in 2 tm 5 of punt binnen noise
        // En rest buiten noise
        // roep: 'cluster'
    }

//    printf("Set consists of: \n");
//


//    for(std::vector<EvidenceSet*>::const_iterator it_set =evidenceSet_.begin(); it_set != evidenceSet_.end(); ++it_set) {
//        EvidenceSet* my_set = new EvidenceSet(**it_set);
//        printf("Set at: %i \n",my_set->size());
//        //printf("Time  : %f \n",my_set->getTimestamp());
//
//        for(EvidenceSet::const_iterator it_ev = my_set->begin(); it_ev != my_set->end(); ++it_ev) {
//            Evidence& evi = **it_ev;
//            printf("    -                  time: %f \n",evi.getTimestamp() );
//        }
//
//    }

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
