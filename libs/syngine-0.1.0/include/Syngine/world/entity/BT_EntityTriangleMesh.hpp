#pragma once
#ifdef USE_BULLET

#include "Syngine/modules/MeshInstance.hpp"
#include "Syngine/modules/ModelInstance.hpp"
#include "Syngine/world/Coordination.hpp"
#include "Syngine/world/entity/BT_Entity.hpp"
#include "Syngine/engine/RenderTable.hpp"

#include <Bullet3Common/b3Vector3.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btTriangleInfoMap.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>

#include <string>
#include <unordered_map>

namespace syng
{

struct BT_ClosestRayResultCallbackTrigIdx : public btCollisionWorld::ClosestRayResultCallback {
    int m_triangleIndex;

    BT_ClosestRayResultCallbackTrigIdx(const btVector3& from, const btVector3& to)
        : btCollisionWorld::ClosestRayResultCallback(from, to), m_triangleIndex(-1) {}

    btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace) override {
        m_triangleIndex = rayResult.m_localShapeInfo ? rayResult.m_localShapeInfo->m_triangleIndex : -1;
        return ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
    }
};

class BT_EntityTriangleMesh : public BT_Entity
{
private:
    std::vector<int> triangleMaterials;
    std::unordered_map<std::string, MeshInstance*> meshes;
    btTriangleMesh* triangleMesh;
    btTriangleInfoMap* triangleInfoMap;
    btBvhTriangleMeshShape* shape;
public:
    Coordination coords;

    BT_EntityTriangleMesh(ModelInstance* model);
    BT_EntityTriangleMesh(MeshInstance* mesh);
    BT_EntityTriangleMesh(std::unordered_map<std::string, MeshInstance*> meshes);
    ~BT_EntityTriangleMesh();

    int getTrigMaterial(int trigIdx) const;

    const glm::mat4 onMotionState() override;
    void load(bool useQuantiziedAabbCompression = true);
    
    btBvhTriangleMeshShape* getShape() {
        return this->shape;
    }

    const std::unordered_map<std::string, MeshInstance*>& getMeshes() {
        return this->meshes;
    }
};
}

#endif