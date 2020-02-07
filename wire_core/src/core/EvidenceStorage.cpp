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
#include "wire/core/ClusterStorage.h"
#include "wire/core/Property.h"
#include "problib/conversions.h"

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

pbl::Vector EvidenceStorage::getPos(EvidenceSet::const_iterator it_ev){

    Evidence* seed = *it_ev;
    const Property* prop_seed = seed->getProperty("position");
    //printf("1");
//// TODO hier gaat het dus mis bij 1 evidence:
    const pbl::PDF& pdf_seed = prop_seed->getValue();
    //printf("2");
    //printf("check: \n");
    const pbl::Gaussian* gauss_seed = pbl::PDFtoGaussian(pdf_seed);
    const pbl::Vector& pos = gauss_seed->getMean();

    //printf(" - position: (%f,%f,%f) \n", pos(0), pos(1), pos(2));

    return pos;
}

void EvidenceStorage::cluster(int setsize) {

    //int setsize= 5;
    float sigma = 0.005;
    int scale = 5;//3

    if (evidenceSet_.size()>=setsize){
        //printf("Start clustering: \n");

        //For oldest set (= at time-setsize)
        EvidenceSet* origin_set = new EvidenceSet(**evidenceSet_.begin());
        //printf ("size: %i", origin_set->size());
        for(EvidenceSet::const_iterator it_ev = origin_set->begin(); it_ev != origin_set->end(); ++it_ev) {
            EvidenceSet* cluster = new EvidenceSet();
            cluster->add(*it_ev);

            //find position of cluster seed of
            pbl::Vector origin_pos =EvidenceStorage().getPos(it_ev);

            ////Compare to other points in time
            for (int count=1; count< setsize;count++){

                //open set
                EvidenceSet* comp_set = new EvidenceSet(**(evidenceSet_.begin()+count));
                int candidate = 0;
                for(EvidenceSet::const_iterator it_nextev = comp_set->begin(); it_nextev != comp_set->end(); ++it_nextev) {
                    //// Todo: If appearance is equal
                    pbl::Vector next_pos =EvidenceStorage().getPos(it_nextev);

                    //// TODO calculate distance in 3D
                    float distance= sqrt((origin_pos(0)-next_pos(0))*(origin_pos(0)-next_pos(0))+(origin_pos(1)-next_pos(1))*(origin_pos(1)-next_pos(1)));
                    //printf("distance = %f \n",distance);

                    if (candidate == 0){
                        if (distance<=sigma){
                            //printf("cluster root= (%f,%f) \n",origin_pos(0),origin_pos(1));
                            candidate=1;
                            cluster->add(*it_nextev);

                        } else if (distance<=scale*sigma){
                            //printf("cluster not free \n");
                            candidate=2;
                            it_nextev = comp_set->end()-1;
                        }
                    } else if (distance<=scale*sigma){
                        //printf("cluster not free \n");
                        candidate=2;
                        it_nextev = comp_set->end()-1;
                    }
                }

                if (candidate !=1){
                    //terminate early
                    count = setsize;
                }
            }

            //when full cluster is found, add to storage
            if (cluster->size()==setsize){
               ClusterStorage::getInstance().add(cluster);
               //printf("lalala %i \n",ClusterStorage::getInstance().size());
            }
        }


    }

}

    void EvidenceStorage::add(EvidenceSet* ev_set,int setsize) {
    //int setsize= 5; //Number of points in historic cluster

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
