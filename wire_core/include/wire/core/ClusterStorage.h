#ifndef WM_CLUSTERSTORAGE_H_
#define WM_CLUSTERSTORAGE_H_

#include "wire/core/datatypes.h"

#include <vector>

namespace mhf {

class Evidence;

/**
 * EvidenceStorage.cpp
 *
 *  Created on: februari 7, 2020
 *      Author: ls de Jong
 *      origin: sdries
 */
class ClusterStorage {

public:

    static ClusterStorage& getInstance();


    virtual ~ClusterStorage();

    void add(EvidenceSet* ev_set);

    unsigned int size() const;

    const Time& getTimestamp() const;

    /**
     *  Returns a read-only (constant) iterator that points to the
     *  first evidence item in the evidence set
     */
    std::vector<EvidenceSet*>::const_iterator begin() const;

    /**
     *  Returns a read-only (constant) iterator that points one past the
     * last evidence item in the evidence set
     */
    std::vector<EvidenceSet*>::const_iterator end() const;

    typedef std::vector<EvidenceSet*>::const_iterator const_iterator;

protected:
    ClusterStorage();
    static ClusterStorage* instance_;
    /// The time from which all evidence in the set originates
    Time timestamp_;

    /// Collection of evidence items
    std::vector<EvidenceSet*> clusterSet_;

};

}

#endif /* EVIDENCESET_H_ */
