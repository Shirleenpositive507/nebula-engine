#include "Entity.h"
#include "EntityManager.h"

namespace nebula {

void EntityHandle::destroy() {
    if (mManager && mEntity.isValid()) {
        mManager->destroyEntity(mEntity);
        mEntity = NULL_ENTITY;
        mManager = nullptr;
    }
}

}
