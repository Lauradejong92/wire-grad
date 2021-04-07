/*
 * HypothesesTree.cpp
 *
 *  Created on: March, 2011
 *  Author: Jos Elfring, Sjoerd van den Dries
 *  Affiliation: Eindhoven University of Technology
 */

#include "wire/logic/HypothesesTree.h"

#include "wire/logic/Assignment.h"
#include "wire/logic/AssignmentSet.h"
#include "wire/logic/AssignmentMatrix.h"
#include "wire/logic/Hypothesis.h"

#include "wire/storage/KnowledgeDatabase.h"
#include "wire/storage/ObjectStorage.h"
#include "wire/storage/SemanticObject.h"
#include "wire/storage/EvidenceStorage.h"
#include "wire/storage/TrailStorage.h"

#include "wire/core/Evidence.h"
#include "wire/core/EvidenceSet.h"
#include "wire/core/Property.h"

#include <queue>
#include <cassert>

#include <iostream>
#include <ros/ros.h>

#ifdef MHF_MEASURE_TIME
    #include <time.h>
#endif

//#define DEBUG_INFO(_msg, ...) printf(_msg, ##__VA_ARGS__)
#define DEBUG_INFO(_msg, ...)

namespace mhf {

/* ****************************************************************************** */
/* *                        CONSTRUCTOR / DESTRUCTOR                            * */
/* ****************************************************************************** */

HypothesisTree::HypothesisTree(int num_max_hyps, double max_min_prob_ratio) : n_updates_(0), t_last_update_(-1),
        tree_height_(0), num_max_hyps_(num_max_hyps), max_min_prob_ratio_(max_min_prob_ratio) {

    // create empty hypothesis (contains no objects) with timestep 0
    auto* empty_hyp = new Hypothesis(t_last_update_, 1.0);

    // add empty hypothesis to leaf list and set as root
    leafs_.push_back(empty_hyp);
    root_ = empty_hyp;
    MAP_hypothesis_ = empty_hyp;

    //empty data files
        //objects
        std::ofstream myfile_obj;
        myfile_obj.open("/home/amigo/Documents/Data_collection/objects_mat.m");
        myfile_obj << " ";
        myfile_obj.close();
        //evidence
        std::ofstream myfile_ev;
        myfile_ev.open("/home/amigo/Documents/Data_collection/evidence_mat.m");
        myfile_ev << " ";
        myfile_ev.close();
        //map
        std::ofstream myfile_map;
        myfile_map.open("/home/amigo/Documents/Data_collection/map_mat.m");
        myfile_map << " ";
        myfile_map.close();
        //hypothesis
        std::ofstream myfile_hyp;
        myfile_hyp.open("/home/amigo/Documents/Data_collection/hyp_mat.m");
        myfile_hyp << " ";
        myfile_hyp.close();
        //trail
        std::ofstream myfile_tra;
        myfile_tra.open("/home/amigo/Documents/Data_collection/trail_mat.m");
        myfile_tra << " ";
        myfile_tra.close();

        //time
        std::ofstream myfile_time;
        myfile_time.open("/home/amigo/Documents/Data_collection/time_mat.m");
        myfile_time << " ";
        myfile_time.close();
}

HypothesisTree::~HypothesisTree() {
    root_->deleteChildren();
    delete root_;
}

/* ****************************************************************************** */
/* *                          PUBLIC MHT OPERATIONS                             * */
/* ****************************************************************************** */

void HypothesisTree::addEvidence(const EvidenceSet& ev_set) {
    DEBUG_INFO("HypothesesTree::processMeasurements\n")
    static int setsize =5;

    if (ev_set.size() == 0) {
        return;
    }

#ifdef MHF_MEASURE_TIME
    timespec t_start_total, t_end_total;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_start_total);
#endif

    showEvidence(ev_set);//print
    double tstart = ros::Time::now().toSec();

    //Add evidence to storage ???
    EvidenceStorage::getInstance().add(ev_set,setsize);
    EvidenceStorage::getInstance().cluster(setsize);
        double t1 = ros::Time::now().toSec();

    //** Propagate all objects, compute association probabilities and add all possible measurement-track assignments
    for(auto it_ev : ev_set) {
        ObjectStorage::getInstance().match(*it_ev);
        //std::cout << "Evidence added: " << *it_ev << std::endl;
//        if (it_ev->getProperty("color")){
//        const Property* cluster_class = it_ev->getProperty("color");
//        std::cout << cluster_class->toString() << std::endl;
//        }
//
//        if (it_ev->getProperty("class_label")){
//            const Property* cluster_class = it_ev->getProperty("class_label");
//            std::cout << cluster_class->toString() << std::endl;
//        }

    }
        //std::cout << "Evidence added: " << ev_set.size() << std::endl;

    t_last_update_ = ev_set.getTimestamp();

    expandTree(ev_set);

    pruneTree(ev_set.getTimestamp());

    applyAssignments();
    ObjectStorage::getInstance().update(ev_set.getTimestamp());

    //printf("Hyps before pruning:                     %i \n",leafs_.size());
    //Clusterbased pruning
        double t2 = ros::Time::now().toSec();
    findTrailConflicts(setsize);
    pruneTree2(ev_set.getTimestamp());
        double t3 = ros::Time::now().toSec();


    // clear old hypotheses leafs
    // The hypotheses will still be there to form a tree, but do not contain any objects anymore
    root_->clearInactive();

    root_ = root_->deleteSinglePaths();

    tree_height_ = root_->calculateHeigth();

    DEBUG_INFO("*** Free memory: assignment matrices ***\n")

    ++n_updates_;

    double tend = ros::Time::now().toSec();
    showTime(tend-tstart, t1-tstart,t3-t2);
    showMAP();
    showHypP();
    showTrail();

    showStatistics();

//
    printf("Iter: %li \n ------------------------------------------- \n", n_updates_);

#ifdef MHF_MEASURE_TIME
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_end_total);
    printf("Total update took %f seconds.\n", (t_end_total.tv_sec - t_start_total.tv_sec) + double(t_end_total.tv_nsec - t_start_total.tv_nsec) / 1e9);
#endif

    DEBUG_INFO("HypothesesTree::processMeasurements - end\n")
}

/* ****************************************************************************** */
/* *                         PROTECTED MHT OPERATIONS                           * */
/* ****************************************************************************** */

    struct compareAssignmentSets {
    bool operator()(const AssignmentSet* a1, const AssignmentSet* a2) const {
        return a1->getProbability() < a2->getProbability();
   }
};

void HypothesisTree::applyAssignments() {
    DEBUG_INFO("applyAssignments - begin\n")

    // iterate over all leaf hypotheses
    for (auto & leaf : leafs_) {
        DEBUG_INFO("  materializing hyp %p, with parent %p\n", (*it), (*it)->getParent())
        leaf->applyAssignments();
    }

    DEBUG_INFO("applyAssignments - end\n")
}

void HypothesisTree::expandTree(const EvidenceSet& ev_set) {

    DEBUG_INFO("expandTree - start\n")

    //** Create new objects based on measurements

    std::list<Assignment*> new_assignments;
    std::list<Assignment*> clutter_assignments;
    for(auto it_ev : ev_set) {
        // new
        new_assignments.push_back(new Assignment(Assignment::NEW, it_ev, nullptr, KnowledgeDatabase::getInstance().getProbabilityNew(*it_ev)));

        // clutter
        clutter_assignments.push_back(new Assignment(Assignment::CLUTTER, it_ev, nullptr, KnowledgeDatabase::getInstance().getProbabilityClutter(*it_ev)));
    }

#ifdef MHF_MEASURE_TIME
    timespec t_start, t_end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_start);
#endif

    //** expand all current leaf hypotheses

    std::priority_queue<AssignmentSet*, std::vector<AssignmentSet*>, compareAssignmentSets > assignment_sets;

    DEBUG_INFO(" - Create assignment matrices and assignment sets\n")

    for (auto hyp : leafs_) {
        // add new object assignments to current hypothesis
        for(auto & new_assignment : new_assignments) {
            hyp->addPotentialAssignment(new_assignment);
        }

        // add clutter assignments to current hypothesis
        for(auto & clutter_assignment : clutter_assignments) {
            hyp->addPotentialAssignment(clutter_assignment);
        }

        // evidence-to-object assignments are added in addEvidence() method already

        // sort assignment matrix based on assignment probabilities
        hyp->getAssignmentMatrix()->sortAssignments();

        // create empty assignment set, add assignments and update probability (product hypothesis and assignment probs)
        auto* ass_set = new AssignmentSet(hyp, hyp->getAssignmentMatrix());
        assignment_sets.push(ass_set);
    }

    DEBUG_INFO(" - Generate hypotheses\n")

    double min_prob = 0;

    // set all current leafs to inactive
    for (auto & leaf : leafs_) {
        leaf->setInactive();
    }

    leafs_.clear();

    int n_iterations = 0;

    // add hypotheses as long as there are criteria are met
    while(!assignment_sets.empty() && leafs_.size() < num_max_hyps_ && assignment_sets.top()->getProbability() > min_prob) {
        // assignment_sets.top()->print();

        // Get most probable assignment
        ++n_iterations;
        DEBUG_INFO("  #assignment sets = %i\n", (int)assignment_sets.size())
        AssignmentSet* ass_set = assignment_sets.top();

        DEBUG_INFO("  inspecting assignment set %p with probability %.16f\n", ass_set, ass_set->getProbability())

        // remove most probable assignment (stored in ass_set pointer above)
        assignment_sets.pop();
        Hypothesis* hyp = ass_set->getHypothesis();

        if (ass_set->isValid()) {
            /* ************ assignment set is complete! Create hypothesis ************ */

            auto* hyp_child = new Hypothesis(ev_set.getTimestamp(), ass_set->getProbability());
            hyp_child->setAssignments(ass_set);
            hyp->addChildHypothesis(hyp_child);

            if (leafs_.empty()) {
                // first hypothesis found (and therefore the best one)
                min_prob = hyp_child->getProbability() * max_min_prob_ratio_;

                MAP_hypothesis_ = hyp_child;
                //printf("check %f \n", MAP_hypothesis_->getProbability());
            }

            //printf("%i: new leaf with prob %f\n", leafs_.size(), hyp_child->probability_);

            DEBUG_INFO(" NEW LEAF: %p\n", hyp_child)
            leafs_.push_back(hyp_child);
            DEBUG_INFO("  #leafs = %i, #old leafs = %i\n", (int)leafs_.size(), n_old_leafs)

            /* ************************************************************************* */
        }

        std::list<AssignmentSet*> child_assignment_sets;
        ass_set->expand(child_assignment_sets);
        for(auto & child_assignment_set : child_assignment_sets) {
            assignment_sets.push(child_assignment_set);
        }

    }

    DEBUG_INFO(" - Free memory (remaining assignment sets)\n")

    assert(!leafs_.empty());

    // delete remaining assignment sets (the ones that where not used to generate hypotheses)
    while(!assignment_sets.empty()) {
        delete assignment_sets.top();
        assignment_sets.pop();
    }

    DEBUG_INFO(" ... done\n")

    ++tree_height_;

    normalizeProbabilities();

#ifdef MHF_MEASURE_TIME
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_end);
    printf("Expansion of hypotheses took %f seconds.\n", (t_end.tv_sec - t_start.tv_sec) + double(t_end.tv_nsec - t_start.tv_nsec) / 1e9);
#endif

    DEBUG_INFO("expandTree - done\n")
}

void HypothesisTree::normalizeProbabilities() {
    // Calculate sum of all probabilities
    double p_total = 0;
    for(auto & leaf : leafs_) {
        p_total += leaf->getProbability();
    }

    // Normalize all probabilities
    for(auto & leaf : leafs_) {
        leaf->setProbability(leaf->getProbability() / p_total);
    }

    root_->calculateBranchProbabilities();
}

void HypothesisTree::pruneTree(const Time& timestamp) {
    DEBUG_INFO("pruneTree - begin\n")
    DEBUG_INFO("   old #leafs = %i\n", (int)leafs_.size())

    // determine best branch leaf per hypothesis

    DEBUG_INFO(" - determine best leaf per branch\n")
    root_->determineBestLeaf();
    DEBUG_INFO("   ... done\n")

    double prob_ratios[] = {1e-8, 1e-7, 1e-6, 1e-5, 1e-5, 1e-4, 1};

    std::list<Hypothesis*> hyp_stack;
    hyp_stack.push_back(root_);

    while(!hyp_stack.empty()) {
        Hypothesis* hyp = hyp_stack.front();
        hyp_stack.pop_front();

        auto& children = hyp->getChildHypotheses();
        if (!children.empty()) {

            // determine best branch of root hypothesis (highest sum of leaf probabilities)
            Hypothesis* best_child = *children.begin();
            for (std::list<Hypothesis*>::const_iterator it_child = children.begin(); it_child != children.end(); ++it_child) {
                if ((*it_child)->getProbability() > best_child->getProbability()) {
                    best_child = *it_child;
                }
                //printf(" (%p, %f)", *it_child, (*it_child)->getProbability());
            }

            double prob_ratio = 0;

            if (hyp->getHeight() > 6) {
                prob_ratio = 0;
            } else {
                prob_ratio = prob_ratios[hyp->getHeight()];
            }

            double min_prob = best_child->getProbability() * prob_ratio;

            for (auto it_child = children.begin(); it_child != children.end();) {
                bool prune_child = false;

                if (*it_child != best_child) {
                    if ((*it_child)->getProbability() != 0) {
                        if (hyp->getHeight() > 6) {
                            DEBUG_INFO(" - Determine hyp similarity between %p and %p\n", best_child->getBestLeaf(),
                                       (*it_child)->getBestLeaf())
                            double similarity = 1;
                            DEBUG_INFO("   ... done\n")

                            //printf("  similarity = %f\n", similarity);

                            prune_child = (similarity > 0.5);
                        } else if ((*it_child)->getProbability() < min_prob) {
                            prune_child = true;
                        }
                    } else {
                        prune_child = true;
                    }
                }

                if (prune_child) {
                    // prune hypothesis and all child hypothesis
                    (*it_child)->deleteChildren();
                    delete (*it_child);
                    it_child = children.erase(it_child);
                } else {
                    hyp_stack.push_front(*it_child);
                    ++it_child;
                }
            }

        }
    }

    // clear leaf list and add new leafs of tree
    leafs_.clear();
    root_->findActiveLeafs(leafs_);

    normalizeProbabilities();

    DEBUG_INFO("   #leafs after pruning = %i\n", (int)leafs_.size())
    DEBUG_INFO("pruneTree - end\n")
}

void HypothesisTree::findTrailConflicts(int setsize) {

    for (const auto& cluster : TrailStorage::getInstance().getTrail()) {
        std::list<Hypothesis*> weak_hyps;
        //printf("     next cluster:\n");
        //std::cout << "          Evidence:" << cluster[4].getAdress() << std::endl;
        //printf("       Storage size: %i \n",ObjectStorage::getInstance().getObjects().size());
        for (const auto object : ObjectStorage::getInstance().getObjects()){
            bool reject=false;
           if (object->getEvMap().size()==setsize){
               //std::cout << "               Object:" << object->getEvMap()[4] << std::endl;
                //find if object matches to cluster (old to new)
                for(int time=0; time<setsize; time++){
                    //std::cout << "     Evidence:" << cluster[setsize-time-1].getAdress() << std::endl;
                    //std::cout << "Object:"<< object->getEvMap()[time].getAdress() << std::endl;

                    if (cluster[time].getAdress() != object->getEvMap()[time]){
                        //std::cout << "    No match:" << cluster[time].getAdress() << std::endl;
                        reject=time;
                        break;
                    } else {
                        //printf("^");
                    }
                }
               //printf("\n");

                if (reject>0){
                    ///printf("*");
                    //object conflicts with cluster:
                    for (const auto weak_hyp: object->getParents()){
                        weak_hyps.push_back(weak_hyp);
                        //printf("           parent prob: %f, to map: %f\n",strong_hyp->getProbability(), getMAPHypothesis().getProbability());
                    }
                }
            }
        }

        //printf("Strong hyps found for this cluster: %i \n",strong_hyps.size());
        //now prune all unmarked parents
        if (!weak_hyps.empty()) {
            //int count =0;
            for (const auto leaf_hyp: leafs_) {
                for (const auto weak_hyp: weak_hyps) {
                    if (leaf_hyp == weak_hyp) {
                        //printf("            leaf: %f and strong: %f \n",strong_hyp->getHeight(), leaf_hyp->getProbability(),strong_hyp->getProbability());
                        leaf_hyp->setProbability(leaf_hyp->getProbability()*1e-8);
                        //leaf_hyp->setProbability(0);
                        //count++;
                    }
                }
            }
            //printf("Weak hyps found: %i \n",count);
        }
        //set mark to unmarked;


    }

   // printf("new MAP %f",getMAPHypothesis().getProbability());
}

    void HypothesisTree::pruneTree2(const Time& timestamp) {
        double prob_ratio = 1e-8;

        // determine best hypothesis
        Hypothesis* best_hyp = leafs_.front();
        for (const auto leaf: leafs_){
            if (best_hyp->getProbability()<leaf->getProbability()){
                best_hyp=leaf;
            }
        }
        MAP_hypothesis_=best_hyp;
        //printf("MAP prob: %f \n", best_hyp->getProbability());

        // now remove too large drops
        double min_prob = MAP_hypothesis_->getProbability()*prob_ratio;

        for (const auto leaf: leafs_){
            if (leaf->getProbability()< min_prob){
                leaf->setInactive();
            }
        }

    // clear leaf list and add new leafs of tree
    leafs_.clear();
    root_->findActiveLeafs(leafs_);
    normalizeProbabilities();
    }

/* ****************************************************************************** */
/* *                                GETTERS                                     * */
/* ****************************************************************************** */

const std::list<Hypothesis*>& HypothesisTree::getHypotheses() const {
    return leafs_;
}


    const Hypothesis& HypothesisTree::getMAPHypothesis() const {
    return *MAP_hypothesis_;
}

const std::list<SemanticObject*>& HypothesisTree::getMAPObjects() const {
    DEBUG_INFO("getMAPObjects - begin\n")
    return getMAPHypothesis().getObjects();
}


/* ****************************************************************************** */
/* *                              PRINT METHODS                                 * */
/* ****************************************************************************** */

void HypothesisTree::showStatistics() {
    std::cout << "   Number of hypotheses        = " << leafs_.size() << std::endl;
//    std::cout << "   Max probability             = " << getMAPHypothesis().getProbability() << std::endl;
//    std::cout << "   Tree height                 = " << tree_height_ << std::endl;
//    std::cout << "----" << std::endl;
//    for (const auto hyp: leafs_){
//        std::cout << "             Hyp: P="<< hyp->getProbability()<< std::endl;
//        for(const auto obj: hyp->getObjects()){
//            //SemanticObject& obj = **it_obj;
//            const Property* my_prop = obj->getProperty("position");
//            std::cout << "           Obj: " << obj->getID()<< " at " <<my_prop->toString()<<std::endl;
//            std::cout << "       Trail: " <<obj->getEvMap()[0] << "   "<<obj->getEvMap()[1] << "   "<<obj->getEvMap()[2] << "   "<<obj->getEvMap()[3] << "   "<<obj->getEvMap()[4] << std::endl;
//        }
//    }
        std::cout << "---------------------------------------------------------------------------" <<  std::endl;
}

    void HypothesisTree::showEvidence(const EvidenceSet& ev_set){
        //std::cout << "---------------------------------------------------------------------------" <<  std::endl;
        //printf("   Evidence size                  = %i \n", ev_set.size());
        std::ofstream myfile_ev;
        myfile_ev.open("/home/amigo/Documents/Data_collection/evidence_mat.m", std::ios::app);
        myfile_ev << "evidence{"<< n_updates_+1<<"}=[";
        for(auto myEvid : ev_set) {
            //Plot evidence:
            const Property* my_prop_e = myEvid->getProperty("position");
            myfile_ev << my_prop_e->toString()<<"\n";
            //std::cout << "Evidence: "<< my_prop_e->toString() << std::endl;
        }
        myfile_ev << "];"<<"\n";
        myfile_ev.close();
    }

    void HypothesisTree::showMAP() {
        int updates;
        updates = n_updates_;
        std::cout << "   Object storage size            = " << ObjectStorage::getInstance().getStorageSize(updates) << std::endl;

        std::cout << "   MAP Hypothesis objects         = " << getMAPObjects().size()<< std::endl;
        std::list<SemanticObject*> objects = getMAPObjects();
        std::ofstream myfile_map;
        myfile_map.open("/home/amigo/Documents/Data_collection/map_mat.m", std::ios::app);
        myfile_map << "MAP{"<< n_updates_<<"}=[";
        for(auto & object : objects) {
            SemanticObject& obj = *object;
            const Property* my_prop = obj.getProperty("position");
            myfile_map << obj.getID()<<", "<<my_prop->toString()<<"\n";
//            const Property* myprop2 = obj.getProperty("color");
            std::cout << "object"<< obj.getID()<<std::endl;
        }
        myfile_map << "];"<<"\n";
        myfile_map.close();
    }

    void HypothesisTree::showHypP(){
        std::ofstream myfile_hyp;
        myfile_hyp.open("/home/amigo/Documents/Data_collection/hyp_mat.m", std::ios::app);
        myfile_hyp << "hyp{"<< n_updates_<<"}=[";
        std::list<Hypothesis *> allHyps = getHypotheses();
        for (auto & allHyp : allHyps) {
            Hypothesis &myHyp = *allHyp;
            myfile_hyp << myHyp.getProbability() << ",";
        }
        myfile_hyp<<"];"<<"\n";
        myfile_hyp.close();
        printf("        nHyps = %zu \n",leafs_.size());
    }

    void HypothesisTree::showTrail(){
        std::ofstream myfile_tra;
        myfile_tra.open("/home/amigo/Documents/Data_collection/trail_mat.m", std::ios::app);
        myfile_tra << "trail{"<< n_updates_<<"}=[";
        myfile_tra << TrailStorage::getInstance().getTrail().size();
        myfile_tra<<"];"<<"\n";
        myfile_tra.close();
        printf("        Trail = %i \n",TrailStorage::getInstance().getTrail().size());
    }
    void HypothesisTree::showTime(double delta_t, double delta_cluster, double delta_prune) {
        std::ofstream myfile_time;
        myfile_time.open("/home/amigo/Documents/Data_collection/time_mat.m", std::ios::app);
        myfile_time << "time{" << n_updates_ << "}=[";
        myfile_time << delta_t<<", "<<delta_cluster<<", "<<delta_prune;
        myfile_time << "];" << "\n";
        myfile_time.close();
    }
}
