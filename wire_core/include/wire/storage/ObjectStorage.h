#ifndef MHT_OBJECT_STORAGE_H_
#define MHT_OBJECT_STORAGE_H_

#include <list>

namespace mhf {

class SemanticObject;
class Evidence;
class KnowledgeDatabase;

class ObjectStorage {

public:

    static ObjectStorage& getInstance();

    virtual ~ObjectStorage();

    void addObject(SemanticObject* obj);

    void removeObject(SemanticObject& obj);

    int getStorageSize( int cycle);

    int getExisting();

    long getUniqueID();

    void match(const Evidence& ev);

protected:

    ObjectStorage();

    static ObjectStorage* instance_;


    long ID_;
    int nExisting_;

    std::list<SemanticObject*> objects_;

    const KnowledgeDatabase& knowledge_db_;   

};

}

#endif
