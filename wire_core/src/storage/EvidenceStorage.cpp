/*
 * EvidenceStorage.cpp
 *
 *  Created on: februari 3, 2020
 *      Author: ls de Jong
 *      origin: sdries
 */

#include "wire/core/EvidenceSet.h"
#include "wire/core/Evidence.h"
#include "wire/storage/EvidenceStorage.h"
#include "wire/storage/TrailStorage.h"
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
        if (evidenceMap.size()>=setsize){
            // Variables
            double chi_2_inner=16.266; //99.9%
            double chi_2_outer=80; // 2*chi 99.9
            float r_in =0.01; //1cm
            float r_out = 0.02; //2cm

            //Covariance matrix
            arma::mat S(3,3);
            S <<0.0655e-5<<-0.0015e-5<<-0.0366e-5<<arma::endr <<-0.0015e-5 <<  0.0319e-5 << 0.0087e-5 << arma::endr << -0.0366e-5 << 0.0087e-5 << 0.5385e-5;


//                //// Radius gives MD
//            arma::vec diff(3);
//            diff << 0.0025 <<arma::endr << 0 << arma::endr <<0;
//            double mahalanobis_dist_sq = arma::dot(arma::inv(S) * diff, diff);
//            printf("maha: %f \n", mahalanobis_dist_sq);


            // remove old clusters
            TrailStorage::getInstance().clear();

            //For oldest set (= at time-setsize)
            for (const auto seed_ev : evidenceMap.begin()->second){ //todo test met: prev(evidenceMap.end()) ipv evidenceMap.begin()
                std::vector<Evidence> cluster_vector;

                //todo: check appearance similarity
                //find position of cluster seed of
                pbl::Vector origin_pos =EvidenceStorage().getPos(&seed_ev);

                //Rough pre-selection check
                for(const auto next_ev_set : evidenceMap){ //evidence set from time t
                    int candidate = 0;
                    for (const auto next_ev: next_ev_set.second){ //evidence from time t
                        pbl::Vector next_pos =EvidenceStorage().getPos(&next_ev);
                        float distance= sqrt((origin_pos(0)-next_pos(0))*(origin_pos(0)-next_pos(0))+(origin_pos(1)-next_pos(1))*(origin_pos(1)-next_pos(1)));
                        //double mahalanobis_dist_sq = arma::dot(arma::inv(S) * (origin_pos-next_pos), (origin_pos-next_pos));
                         //printf("maha: %f \n", mahalanobis_dist_sq);
                        //printf("distance = %f \n",distance);
                        if (candidate == 0){
                            if (distance<=r_in){
                                candidate=1;
                                cluster_vector.emplace_back(next_ev);
                                //double mahalanobis_dist_sq = arma::dot(arma::inv(S) * diff, diff);
                            } else if (distance<=r_out){
                                candidate=2;
                                break;
                            }
                        } else if (distance<=r_out){
                            candidate=2;
                            break;
                        }
                    }

                    if (candidate !=1){
                        //terminate early
                        cluster_vector.clear();
                        break;
                    }
                    //TrailStorage::getInstance().add(cluster_vector);
                }

                //True check
                int cluster_unfit=0;
                if (cluster_vector.size()){
                    //find mean
                    pbl::Vector mean_pos={0,0,0};
                    for (const auto clustered_ev: cluster_vector){
                        mean_pos=mean_pos+EvidenceStorage().getPos(&clustered_ev);
                    }
                    mean_pos=mean_pos/setsize;
                    //printf("Points: %f, %f, %f \n", mean_pos(0), mean_pos(1),mean_pos(2));

                    //check if inner radius< 95 cert.
                    for (const auto clustered_ev: cluster_vector){
                        pbl::Vector diff =mean_pos-EvidenceStorage().getPos(&clustered_ev);
                        double mahalanobis_dist_sq = arma::dot(arma::inv(S) * diff, diff);

                        if (mahalanobis_dist_sq > chi_2_inner){
                            printf("too far off with %f, %f, %f \n",diff[0],diff[1],diff[2]);
                            cluster_unfit=1;
                        } else {
                            //printf("md: %f \n", mahalanobis_dist_sq);
                        }
                    }


                    //check if outer radius is empty.


                    if (cluster_unfit){
                        cluster_vector.clear();
                        break;
                    }

                    TrailStorage::getInstance().add(cluster_vector);
                }
            }
        }

    }

    void EvidenceStorage::add(const EvidenceSet& ev_set,int setsize) {
        std::vector<Evidence> evidence_vector;
        for (const auto& ev : ev_set)
        {
            ev->setAdress(ev);
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