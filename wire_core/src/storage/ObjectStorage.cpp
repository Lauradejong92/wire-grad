#include "wire/storage/ObjectStorage.h"

#include "wire/storage/KnowledgeDatabase.h"
#include "wire/storage/SemanticObject.h"
#include "wire/core/Evidence.h"
#include "wire/core/PropertySet.h"
#include "wire/core/Property.h"

using namespace std;

namespace mhf {

    ObjectStorage *ObjectStorage::instance_ = nullptr;

    ObjectStorage &ObjectStorage::getInstance() {
        if (instance_) {
            return *instance_;
        }
        instance_ = new ObjectStorage();
        return *instance_;
    }

    ObjectStorage::ObjectStorage() : ID_(0), knowledge_db_(KnowledgeDatabase::getInstance()) {

    }

    ObjectStorage::~ObjectStorage() = default;

    void ObjectStorage::addObject(SemanticObject *obj) {
        objects_.push_back(obj);
        obj->it_obj_storage_ = --objects_.end();
    }

    void ObjectStorage::removeObject(SemanticObject &obj) {
        objects_.erase(obj.it_obj_storage_);
    }

    std::list<SemanticObject *> ObjectStorage::getObjects() {
        return objects_;
    }

    long ObjectStorage::getUniqueID() {
        return ID_++;
    }

    void ObjectStorage::match(const Evidence &ev) {

        //cout << endl << "ObjectStorage::match" << endl;

        for (auto & object : objects_) {
            SemanticObject &obj = *object;
            obj.propagate(ev.getTimestamp());
        }

        for (auto & object : objects_) {
            SemanticObject &obj = *object;

            double prob_existing = KnowledgeDatabase::getInstance().getProbabilityExisting(ev, obj);
            if (prob_existing > 0) {

                //cout << "Adding evidence " << &ev << " to object " << &obj << endl;

                obj.addPotentialAssignment(ev, prob_existing);
            }
        }
    }

    void ObjectStorage::update(const Time &timestamp) {
        for (const auto it_obj: objects_) {
            if (timestamp != it_obj->getLastUpdateTime()) {
                it_obj->updateEvidenceMap();
            }
        }
    }

    int ObjectStorage::getStorageSize(int cycle){
        //*
        //init telvector
        vector<int> count_count;

        std::ofstream myfile_obj;
        myfile_obj.open("/home/amigo/Documents/Data_collection/objects_mat.m", std::ios::app);
        myfile_obj << "objects{"<< cycle<<"}=["<<"\n";

        for (int i = 1; i <= ID_; i++)
            count_count.push_back(0);

        //schrijf naar file
        for(auto & object : objects_) {
            SemanticObject& obj = *object;
            count_count[obj.getID()]++;

            const Property* my_prop = obj.getProperty("position");
            //cout << my_prop->toString()<< endl;
            myfile_obj << obj.getID() <<","<< my_prop->toString() <<endl;
        }
        myfile_obj << "];"<<"\n";
        myfile_obj.close();

//        // print overzicht:
//        cout << "   The object storage consists of: " << endl;
//        for (int k = 0; k <= ID_ -1; k++){
//            if (count_count[k]!=0)
//                cout << "     -Object " << k <<", stored " << count_count[k] << " times" << endl;
//        }
//        //*/

        return objects_.size();
    }
}
