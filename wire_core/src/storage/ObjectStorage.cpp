#include "wire/storage/ObjectStorage.h"

#include "wire/storage/KnowledgeDatabase.h"
#include "wire/storage/SemanticObject.h"
#include "wire/core/Evidence.h"
#include "wire/core/Property.h"

using namespace std;

namespace mhf {

ObjectStorage* ObjectStorage::instance_ = 0;

ObjectStorage& ObjectStorage::getInstance() {
    if (instance_) {
        return *instance_;
    }
    instance_ = new ObjectStorage();
    return *instance_;
}

ObjectStorage::ObjectStorage() : ID_(0), nExisting_(0), knowledge_db_(KnowledgeDatabase::getInstance()) {

}

ObjectStorage::~ObjectStorage() {

}

void ObjectStorage::addObject(SemanticObject* obj) {
    objects_.push_back(obj);
    obj->it_obj_storage_ = --objects_.end();
    //cout << "Object added: " << obj->getID() << endl;
}

void ObjectStorage::removeObject(SemanticObject& obj) {
    //cout << "Object removed: " << obj.getID() << endl;
    objects_.erase(obj.it_obj_storage_);
}

int ObjectStorage::getStorageSize(){
    //*
    //init telvector
    vector<int> count_count;

    for (int i = 1; i <= ID_; i++)
        count_count.push_back(0);

//    cout << "   Leafs containing objects: " << endl;
    for(list<SemanticObject*>::iterator it_obj = objects_.begin(); it_obj != objects_.end(); ++it_obj) {
        SemanticObject& obj = **it_obj;
        count_count[obj.getID()]++;

        const Property* my_prop = obj.getProperty("position");
        //cout << my_prop->toString()<< endl;
        //cout << "     -Obj: " <<obj.getID() <<" at "<< my_prop->toString() << endl;
    }

    // print overzicht:
    //cout << "   The object storage consists of: " << endl;
    for (int k = 0; k <= ID_ -1; k++){
        if (count_count[k]!=0)
            //cout << "     -Object " << k <<", stored " << count_count[k] << " times" << endl;
            cout << ", " << k <<", " << count_count[k] << endl;
    }
     //*/

    return objects_.size();
    }

    int ObjectStorage::getExisting(){
    return nExisting_;
}

long ObjectStorage::getUniqueID() {
    return ID_++;
}



void ObjectStorage::match(const Evidence& ev) {
    static int nExist;
    //cout << endl << "ObjectStorage::match" << endl;

    for(list<SemanticObject*>::iterator it_obj = objects_.begin(); it_obj != objects_.end(); ++it_obj) {
        SemanticObject& obj = **it_obj;
        obj.propagate(ev.getTimestamp());
    }

    for(list<SemanticObject*>::iterator it_obj = objects_.begin(); it_obj != objects_.end(); ++it_obj) {
        SemanticObject& obj = **it_obj;

        double prob_existing = KnowledgeDatabase::getInstance().getProbabilityExisting(ev, obj);
        if (prob_existing > 0) {

            //cout << "Adding evidence " << &ev << " to object " << &obj << endl;

            obj.getExpectedObjectModel();
            //const Property* myProp = obj.getProperty("position");
            //IStateEstimator myStat=myProp->getEstimator();
            //cout << "test" << &myState << endl;

            nExist= obj.addPotentialAssignment(ev, prob_existing);

        } else {
            cout << "not compatible " << endl;
        }

    }
    nExisting_=nExist;
}

}
