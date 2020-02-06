#ifndef WM_EVIDENCESTORAGE_H_
#define WM_EVIDENCESTORAGE_H_

#include "wire/core/datatypes.h"

#include <vector>

namespace mhf {

class Evidence;

/**
 * EvidenceStorage.cpp
 *
 *  Created on: februari 3, 2020
 *      Author: ls de Jong
 *      origin: sdries
 */
class EvidenceStorage {

public:

    static EvidenceStorage& getInstance();


    virtual ~EvidenceStorage();

    void hello123();

    /**
     * @brief Adds evidence to the evidence set
     * @param ev The evidence
     */
    void add(EvidenceSet* ev_set);

    /**
     * @brief Returns the number of evidence items in the set
     * @return The number of evidence items in the set
     */
    unsigned int size() const;

    /**
     * @brief Returns the time from which all evidence in the set originates
     * @return The time from which all evidence in the set originates
     */
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
    EvidenceStorage();
    static EvidenceStorage* instance_;
    /// The time from which all evidence in the set originates
    Time timestamp_;

    /// Collection of evidence items
    std::vector<EvidenceSet*> evidenceSet_;

};

}

#endif /* EVIDENCESET_H_ */
