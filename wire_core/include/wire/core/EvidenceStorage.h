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

    EvidenceStorage();

    virtual ~EvidenceStorage();

    void hello123();

    /**
     * @brief Adds evidence to the evidence set
     * @param ev The evidence
     */
    void add(Evidence* ev);

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
    std::vector<Evidence*>::const_iterator begin() const;

    /**
     *  Returns a read-only (constant) iterator that points one past the
     * last evidence item in the evidence set
     */
    std::vector<Evidence*>::const_iterator end() const;

    typedef std::vector<Evidence*>::const_iterator const_iterator;

protected:

    /// The time from which all evidence in the set originates
    Time timestamp_;

    /// Collection of evidence items
    std::vector<Evidence*> evidence_;

};

}

#endif /* EVIDENCESET_H_ */
