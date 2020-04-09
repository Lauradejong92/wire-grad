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
class TrailStorage {

public:

    static TrailStorage& getInstance();

    virtual ~TrailStorage();

    //const Evidence* getEvidence (int set, int place) const;

    void add(std::vector<Evidence> ev_set);

    void clear();

    unsigned int size() const;

    const Time& getTimestamp() const;

    const std::vector<std:: vector<Evidence>> getTrail() const;

    /**
     *  Returns a read-only (constant) iterator that points to the
     *  first evidence item in the evidence set
     */
    std::vector<std:: vector<Evidence>>::const_iterator begin() const;

    /**
     *  Returns a read-only (constant) iterator that points one past the
     * last evidence item in the evidence set
     */
    std::vector<std:: vector<Evidence>>::const_iterator end() const;

    typedef std::vector<std:: vector<Evidence>>::const_iterator const_iterator;

protected:
    TrailStorage();
    static TrailStorage* instance_;
    /// The time from which all evidence in the set originates
    Time timestamp_;

    /// Collection of evidence items
    std::vector<std:: vector<Evidence>> trailSet_;

};

}

#endif /* EVIDENCESET_H_ */