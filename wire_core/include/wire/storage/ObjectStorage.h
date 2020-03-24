#ifndef MHT_OBJECT_STORAGE_H_
#define MHT_OBJECT_STORAGE_H_

#include <list>
#include <wire/core/datatypes.h>
#include "wire/core/PropertySet.h"

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

    std::list<SemanticObject*> getObjects();

    long getUniqueID();

    void match(const Evidence& ev);

    void update(const Time& timestamp);

protected:

    ObjectStorage();

    static ObjectStorage* instance_;


    long ID_;

    std::list<SemanticObject*> objects_;

    const KnowledgeDatabase& knowledge_db_;   

};

}

#endif
