/*
 * Measurement.cpp
 *
 *  Created on: March, 2011
 *  Author: Jos Elfring, Sjoerd van den Dries
 *  Affiliation: Eindhoven University of Technology
 */

#include "wire/core/Evidence.h"

using namespace std;

namespace mhf {

int Evidence::N_EVIDENCE = 0;

Evidence::Evidence(Time timestamp) : adress_(0), PropertySet(timestamp) {
    ++N_EVIDENCE;
}

Evidence::~Evidence() {
    --N_EVIDENCE;
}

void Evidence::setAdress(Evidence* adres){
    adress_=adres;
}

Evidence* Evidence::getAdress() const{
        return adress_;
}

}
