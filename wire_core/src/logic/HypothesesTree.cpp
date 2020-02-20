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

#include "wire/core/Evidence.h"
#include "wire/core/EvidenceSet.h"
#include "wire/core/Property.h"
#include "wire/core/ClassModel.h"
#include "wire/core/EvidenceStorage.h"
#include "wire/core/ClusterStorage.h"

#include <queue>
#include <cassert>
#include <float.h>

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
    Hypothesis* empty_hyp = new Hypothesis(t_last_update_, 1.0);

    // add empty hypothesis to leaf list and set as root
    leafs_.push_back(empty_hyp);
    root_ = empty_hyp;
    MAP_hypothesis_ = empty_hyp;
}

HypothesisTree::~HypothesisTree() {
    root_->deleteChildren();
    delete root_;
}

/* ****************************************************************************** */
/* *                          PUBLIC MHT OPERATIONS                             * */
/* ****************************************************************************** */

void HypothesisTree::addEvidence(const EvidenceSet& ev_set) {
    DEBUG_INFO("HypothesesTree::processMeasurements\n");
    static int setsize =5;

    if (ev_set.size() == 0) {
        return;
    }

#ifdef MHF_MEASURE_TIME
    timespec t_start_total, t_end_total;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_start_total);
#endif
    //Add evidence to storage ???
    EvidenceStorage::getInstance().add(ev_set,setsize);
    EvidenceStorage::getInstance().cluster(setsize);

    //** Propagate all objects, compute association probabilities and add all possible measurement-track assignments
    for(EvidenceSet::const_iterator it_ev = ev_set.begin(); it_ev != ev_set.end(); ++it_ev) {
        ObjectStorage::getInstance().match(**it_ev);
        //std::cout << "Evidence_right: " << *it_ev << std::endl;
    }

    t_last_update_ = ev_set.getTimestamp();

    expandTree(ev_set);

    //Clusterbased pruning
    //Cluster forming

    pruneClusterwise(setsize);

    pruneTree(ev_set.getTimestamp());

    applyAssignments();

    // clear old hypotheses leafs
    // The hypotheses will still be there to form a tree, but do not contain any objects anymore
    root_->clearInactive();

    root_ = root_->deleteSinglePaths();

    tree_height_ = root_->calculateHeigth();

    DEBUG_INFO("*** Free memory: assignment matrices ***\n");

    ++n_updates_;

    showStatistics();

#ifdef MHF_MEASURE_TIME
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_end_total);
    printf("Total update took %f seconds.\n", (t_end_total.tv_sec - t_start_total.tv_sec) + double(t_end_total.tv_nsec - t_start_total.tv_nsec) / 1e9);
#endif

    DEBUG_INFO("HypothesesTree::processMeasurements - end\n");
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
    DEBUG_INFO("applyAssignments - begin\n");

    // iterate over all leaf hypotheses
    for (std::list<Hypothesis*>::iterator it = leafs_.begin(); it != leafs_.end(); ++it) {
        DEBUG_INFO("  materializing hyp %p, with parent %p\n", (*it), (*it)->getParent());
        (*it)->applyAssignments();
    }

    DEBUG_INFO("applyAssignments - end\n");
}

void HypothesisTree::expandTree(const EvidenceSet& ev_set) {

    DEBUG_INFO("expandTree - start\n");

    //** Create new objects based on measurements

    std::list<Assignment*> new_assignments;
    std::list<Assignment*> clutter_assignments;
    for(EvidenceSet::const_iterator it_ev = ev_set.begin(); it_ev != ev_set.end(); ++it_ev) {
        // new
        new_assignments.push_back(new Assignment(Assignment::NEW, *it_ev, 0, KnowledgeDatabase::getInstance().getProbabilityNew(**it_ev)));

        // clutter
        clutter_assignments.push_back(new Assignment(Assignment::CLUTTER, *it_ev, 0, KnowledgeDatabase::getInstance().getProbabilityClutter(**it_ev)));
    }

#ifdef MHF_MEASURE_TIME
    timespec t_start, t_end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_start);
#endif

    //** expand all current leaf hypotheses

    std::priority_queue<AssignmentSet*, std::vector<AssignmentSet*>, compareAssignmentSets > assignment_sets;

    DEBUG_INFO(" - Create assignment matrices and assignment sets\n");

    for (std::list<Hypothesis*>::iterator it_hyp = leafs_.begin(); it_hyp != leafs_.end(); ++it_hyp) {
        Hypothesis* hyp = *it_hyp;

        // add new object assignments to current hypothesis
        for(std::list<Assignment*>::iterator it_ass = new_assignments.begin(); it_ass != new_assignments.end(); ++it_ass) {
            hyp->addPotentialAssignment(*it_ass);
        }

        // add clutter assignments to current hypothesis
        for(std::list<Assignment*>::iterator it_ass = clutter_assignments.begin(); it_ass != clutter_assignments.end(); ++it_ass) {
            hyp->addPotentialAssignment(*it_ass);
        }

        // evidence-to-object assignments are added in addEvidence() method already

        // sort assignment matrix based on assignment probabilities
        hyp->getAssignmentMatrix()->sortAssignments();

        // create empty assignment set, add assignments and update probability (product hypothesis and assignment probs)
        AssignmentSet* ass_set = new AssignmentSet(hyp, hyp->getAssignmentMatrix());
        assignment_sets.push(ass_set);
    }

    DEBUG_INFO(" - Generate hypotheses\n");

    double min_prob = 0;

    // set all current leafs to inactive
    for (std::list<Hypothesis*>::iterator it_hyp = leafs_.begin(); it_hyp != leafs_.end(); ++it_hyp) {
        (*it_hyp)->setInactive();
    }

    leafs_.clear();

    int n_iterations = 0;

    // add hypotheses as long as there are criteria are met
    while(!assignment_sets.empty() && leafs_.size() < num_max_hyps_ && assignment_sets.top()->getProbability() > min_prob) {
        // assignment_sets.top()->print();

        // Get most probable assignment
        ++n_iterations;
        DEBUG_INFO("  #assignment sets = %i\n", (int)assignment_sets.size());
        AssignmentSet* ass_set = assignment_sets.top();

        DEBUG_INFO("  inspecting assignment set %p with probability %.16f\n", ass_set, ass_set->getProbability());

        // remove most probable assignment (stored in ass_set pointer above)
        assignment_sets.pop();
        Hypothesis* hyp = ass_set->getHypothesis();

        if (ass_set->isValid()) {
            /* ************ assignment set is complete! Create hypothesis ************ */

            Hypothesis* hyp_child = new Hypothesis(ev_set.getTimestamp(), ass_set->getProbability());
            hyp_child->setAssignments(ass_set);
            hyp->addChildHypothesis(hyp_child);

            if (leafs_.empty()) {
                // first hypothesis found (and therefore the best one)
                min_prob = hyp_child->getProbability() * max_min_prob_ratio_;

                MAP_hypothesis_ = hyp_child;
            }

            /*
            if (leafs_.size() <= 3) {
                ass_set->print();
            }
            */

            //printf("%i: new leaf with prob %f\n", leafs_.size(), hyp_child->probability_);

            DEBUG_INFO(" NEW LEAF: %p\n", hyp_child);
            leafs_.push_back(hyp_child);
            DEBUG_INFO("  #leafs = %i, #old leafs = %i\n", (int)leafs_.size(), n_old_leafs);

            /* ************************************************************************* */
        }

        std::list<AssignmentSet*> child_assignment_sets;
        ass_set->expand(child_assignment_sets);
        for(std::list<AssignmentSet*>::iterator it_child = child_assignment_sets.begin(); it_child != child_assignment_sets.end(); ++it_child) {
            assignment_sets.push(*it_child);
        }

    }

    DEBUG_INFO(" - Free memory (remaining assignment sets)\n");

    assert(leafs_.size() > 0);

    // delete remaining assignment sets (the ones that where not used to generate hypotheses)
    while(!assignment_sets.empty()) {
        delete assignment_sets.top();
        assignment_sets.pop();
    }

    DEBUG_INFO(" ... done\n");

    ++tree_height_;

    normalizeProbabilities();

#ifdef MHF_MEASURE_TIME
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_end);
    printf("Expansion of hypotheses took %f seconds.\n", (t_end.tv_sec - t_start.tv_sec) + double(t_end.tv_nsec - t_start.tv_nsec) / 1e9);
#endif

    DEBUG_INFO("expandTree - done\n");
}

void HypothesisTree::normalizeProbabilities() {
    // Calculate sum of all probabilities
    double p_total = 0;
    for(std::list<Hypothesis* >::iterator it_hyp = leafs_.begin(); it_hyp != leafs_.end(); ++it_hyp) {
        p_total += (*it_hyp)->getProbability();
    }

    // Normalize all probabilities
    for(std::list<Hypothesis* >::iterator it_hyp = leafs_.begin(); it_hyp != leafs_.end(); ++it_hyp) {
        (*it_hyp)->setProbability((*it_hyp)->getProbability() / p_total);
    }

    root_->calculateBranchProbabilities();
}

void HypothesisTree::pruneTree(const Time& timestamp) {
    //return;


    DEBUG_INFO("pruneTree - begin\n");

    DEBUG_INFO("   old #leafs = %i\n", (int)leafs_.size());

    // determine best branch leaf per hypothesis

    DEBUG_INFO(" - determine best leaf per branch\n");
    root_->determineBestLeaf();
    DEBUG_INFO("   ... done\n");

    double prob_ratios[] = {1e-8, 1e-7, 1e-6, 1e-5, 1e-5, 1e-4, 1};

    std::list<Hypothesis*> hyp_stack;
    hyp_stack.push_back(root_);

    while(!hyp_stack.empty()) {
        Hypothesis* hyp = hyp_stack.front();
        hyp_stack.pop_front();

        std::list<Hypothesis*>& children = hyp->getChildHypotheses();
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

            for (std::list<Hypothesis*>::iterator it_child = children.begin(); it_child != children.end();) {
                bool prune_child = false;

                if (*it_child != best_child) {
                    if ((*it_child)->getProbability() == 0) {
                        prune_child = true;
                    } else if (hyp->getHeight() > 6) {
                        DEBUG_INFO(" - Determine hyp similarity between %p and %p\n", best_child->getBestLeaf(), (*it_child)->getBestLeaf());
                        double similarity = 1;
                        DEBUG_INFO("   ... done\n");

                        //printf("  similarity = %f\n", similarity);

                        prune_child = (similarity > 0.5);
                    } else if ((*it_child)->getProbability() < min_prob) {
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

    DEBUG_INFO("   #leafs after pruning = %i\n", (int)leafs_.size());

    DEBUG_INFO("pruneTree - end\n");
}

void HypothesisTree::pruneClusterwise(int setsize) {

    if (root_->getHeight()>=setsize && ClusterStorage::getInstance().size()>0){ //only clustering when enough datapoints & a cluster is available

        // Find hypothesis of time t-setsize
        std::map<int,Hypothesis*> hyp_stack;
        hyp_stack[root_->getHeight()]=root_;
        while(!hyp_stack.empty()) {

            //quit when right set of hypothesis from time t-setsize is found
            if (hyp_stack.rbegin()->first < setsize) {
                //printf("stacksize ends = %i \n", hyp_stack.size());
                break;
            }

            //Save and remove hyp that is too old
            Hypothesis* hyp = hyp_stack.rbegin()->second;
            hyp_stack.erase(prev(hyp_stack.end()));
            //printf("height: %i \n",hyp->getHeight());

            //Extend set with children of the hyp that is too old
            std::list<Hypothesis*>& children = hyp->getChildHypotheses();
            if (!children.empty()) {
                for (const auto it_child: children) {
                    if (it_child->getHeight()>=setsize-1) {
                        //printf("add height: %i \n", it_child->getHeight());
                        hyp_stack[it_child->getHeight()] = it_child;
                    }
                }
            }

        }

 //       Search for hypotheses confirming the cluster
        for (int n_cluster=0; n_cluster<ClusterStorage::getInstance().size(); n_cluster++) {

            //find seed
            const Evidence* clusterev = ClusterStorage::getInstance().getEvidence(n_cluster, setsize-1);
            std::cout << "Cluster seed new:" << clusterev->getAdress() << std::endl;

            //todo:reverse naar van hoog naar laag; loop naar alleen children height 4
            //for all hyps
            for(const auto hyps : hyp_stack){
                printf("   hyp height: %i with prob: %f \n",hyps.second->getHeight(),hyps.second->getProbability());

                //for all evidences in the hypothesis
                for (int k = 0; k<hyps.second->getAssignmentMatrix()->getNumMeasurements();k++) {
                    const Assignment &ev_ass = hyps.second->getAssignmentMatrix()->getAssignment(k, 0);
                    if (ev_ass.getEvidence() == clusterev->getAdress()) {
                        printf("      hoera! Now get objects\n");

                        //all objects assigned to this evidence
                        for (int l = 0; l<hyps.second->getAssignmentMatrix()->getNumAssignments(k);l++) {
                            const Assignment &myassi = hyps.second->getAssignmentMatrix()->getAssignment(k, l);
                            //std::cout << "Evidence:" << myassi.getEvidence() << std::endl;
                            //std::cout << "Object:"<< myassi.getTarget() << std::endl;

                            // First, determine wich objects the seed can be referring to
                            if (myassi.getTarget()) {
                                std::cout << "       Object:" << myassi.getTarget() << " with "<<myassi.getTarget()->getID()<<std::endl;
                                // in candidatenset
                            }

                        }
                    }

                }
                // if candidatenset, doorsturen naar spoeling

            }

            //check if a child obays all



                //std::cout << "Evidence:"<< ClusterStorage::getInstance().getEvidence(n_cluster,t) << std::endl;
                //Hypothesis *hyp = hyp_stack.front();
        }
                    //first:
                    // if clusterev == hypothesis evidence
                        //get object id
                    //second:
                    // if clusterev == hypothesis
                        //if obj_id = savedobj_id




                //voor alle evidence

                        //save object adress

                        //nu kids doorzoeken
                            //for all kids
                            // for ev = cluster(tx, nummer 1)
                            // if verwijzing is naar objectadres
                                // op naar de volgende
                       // niets gevonden, dan break;

           //de juiste gevonden?
           //iets doen met alle onjuisten

    }


    //Todo: voorbeelde voor 1 leaf:
//    Hypothesis* hyp = leafs_.front();
//    //printf("lala%i", hyp->getHeight());
//
//    const AssignmentSet* myset= hyp->getAssignments();
//    for (int k=0; k<myset->getNumMeasurements();k++ ) {
//        const Assignment &myassi = myset->getMeasurementAssignment(k);
//        //std::cout << "Evidence:"<< myassi.getEvidence() << std::endl;
//        //std::cout << "Object:"<< myassi.getTarget() << std::endl;
//    }
//
//    const Hypothesis* par = hyp->getParent();
    //if evidence gelijk
    //if target nummer is als eerder
}

/* ****************************************************************************** */
/* *                                GETTERS                                     * */
/* ****************************************************************************** */

const std::list<Hypothesis*>& HypothesisTree::getHypotheses() const {
    return leafs_;
}

int HypothesisTree::getHeight() const {
    return tree_height_;
}


const Hypothesis& HypothesisTree::getMAPHypothesis() const {
    return *MAP_hypothesis_;
}

const std::list<SemanticObject*>& HypothesisTree::getMAPObjects() const {
    DEBUG_INFO("getMAPObjects - begin\n");
    return getMAPHypothesis().getObjects();
}


/* ****************************************************************************** */
/* *                              PRINT METHODS                                 * */
/* ****************************************************************************** */

void HypothesisTree::showStatistics() {
    std::cout << "   Number of hypotheses        = " << leafs_.size() << std::endl;
    //std::cout << "   Max probability             = " << getMAPHypothesis().getProbability() << std::endl;
    //std::cout << "   Tree height                 = " << tree_height_ << std::endl;
    std::cout << "----" << std::endl;
}

}
