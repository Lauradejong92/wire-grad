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

    pbl::Vector EvidenceStorage::getPos(const Evidence* ev){

        //Evidence* seed = *ev;
        const Property* prop = ev->getProperty("position");
        //printf("1");
//// TODO hier gaat het dus mis bij 1 evidence:
        const pbl::PDF& pdf = prop->getValue();
        //printf("2");
        //printf("check: \n");
        const pbl::Gaussian* gauss = pbl::PDFtoGaussian(pdf);
        const pbl::Vector& pos = gauss->getMean();

        //printf(" - position: (%f,%f,%f) \n", pos(0), pos(1), pos(2));

        return pos;
    }

    void EvidenceStorage::cluster(int setsize) {

        //int setsize= 5;
        float sigma = 0.005;
        int scale = 5;//3

        if (evidenceMap.size()>=setsize){
            // remove old clusters
            ClusterStorage::getInstance().clear();
            //printf("Start clustering: \n");

            //For oldest set (= at time-setsize)
            for (const auto seed_ev : evidenceMap.begin()->second){
                std::vector<Evidence> cluster_vector;
                //cluster_vector.emplace_back(seed_ev);
                //printf("cluster size: %i",cluster_vector.size());

                //todo: check appearance similarity
                //find position of cluster seed of
                pbl::Vector origin_pos =EvidenceStorage().getPos(&seed_ev);

                //Compare to other points in time
                for(const auto next_ev_set : evidenceMap){ //evidence set from time t
                    int candidate = 0;
                    for (const auto next_ev: next_ev_set.second){ //evidence from time t
                        //printf("a");
                        //printf("loop 3");
                        pbl::Vector next_pos =EvidenceStorage().getPos(&next_ev);
                        float distance= sqrt((origin_pos(0)-next_pos(0))*(origin_pos(0)-next_pos(0))+(origin_pos(1)-next_pos(1))*(origin_pos(1)-next_pos(1)));
                        //printf("distance = %f \n",distance);
                        //todo: more fancy clustering & comparison than first position compared to all
                        if (candidate == 0){
                            if (distance<=sigma){
                                //printf("cluster root= (%f,%f) \n",origin_pos(0),origin_pos(1));
                                candidate=1;
                                cluster_vector.emplace_back(next_ev);

                            } else if (distance<=scale*sigma){
                                printf("cluster not free 1 \n");
                                candidate=2;
                                break;
                            }
                        } else if (distance<=scale*sigma){
                            printf("cluster not free 2\n");
                            candidate=2;
                            break;

                        }
                    }
                    //printf("Flag \n");

                    if (candidate !=1){
                        //terminate early
                        //printf("Seed forms no cluster \n");
                        cluster_vector.clear();
                        break;
                    }

                }

                if (cluster_vector.size()){
                    //printf("cluster found! \n");
                    ClusterStorage::getInstance().add(cluster_vector);
                }
            }
        }

    }

    void EvidenceStorage::add(const EvidenceSet& ev_set,int setsize) {
        std::vector<Evidence> evidence_vector;
        for (const auto& ev : ev_set)
        {
            evidence_vector.emplace_back(*ev);
        }
        evidenceMap[ev_set.getTimestamp()] = evidence_vector;

//        for (const auto ev : evidence_vector)
//        {
//            if (!ev.getProperty("position"))
//            {
//                printf("no position added \n");
//                //deze zie je dus nooit
//            } else{
//                printf("Position added \n");
//            }
//        }


//      printf("evidenceMap size is %zu \n", evidenceMap.size());
        if (evidenceMap.size()>setsize){
            //evidenceSet_.erase(evidenceSet_.begin());
            evidenceMap.erase(evidenceMap.begin());
        }
    }

    unsigned int EvidenceStorage::size() const {
        return evidenceSet_.size();
    }

    const Time& EvidenceStorage::getTimestamp() const {
        return timestamp_;
    }


    std::vector<EvidenceSet>::const_iterator EvidenceStorage::begin() const {
        return evidenceSet_.begin();
    }

    std::vector<EvidenceSet>::const_iterator EvidenceStorage::end() const {
        return evidenceSet_.end();
    }

}