#include "Player.hpp"

#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "BulletCollision/CollisionShapes/btCapsuleShape.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"

#include "LinearMath/btDefaultMotionState.h"
#include "LinearMath/btVector3.h"

#include "Syngine/modules/Mesh.hpp"
#include "Syngine/world/World.hpp"
#include "Syngine/world/entity/BT_EntityTriangleMesh.hpp"
#include "Syngine/world/Coordination.hpp"
#include "Syngine/utils/GameUtils.hpp"
#include "game/GameManager.hpp"
#include "game/Sound.hpp"

#include <glm/glm.hpp>
#include <iostream>
#include <ostream>

using namespace syng;
using namespace GamePlay;

btRigidBody* GamePlay::Player::createEntity(Coordination coords, float mass, float radius, float height) {
    btCollisionShape* shape = new btCapsuleShape(radius, height);

    btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(
        GameUtils::getBulletRotationFromTransform(coords.getTransform()),
        GameUtils::toBulletVector(coords.getPosition())
    ));

    btVector3 inertia(0, 0, 0);
    shape->calculateLocalInertia(mass, inertia);

    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, inertia);
    btRigidBody* body = new btRigidBody(rbInfo);

    body->setAngularFactor(btVector3(0.0f, 1.0f, 0.0f));
    body->setActivationState(DISABLE_DEACTIVATION);
    body->setDamping(0.1f, 0.3f);

    return body;
}

void GamePlay::Player::applyImpulse(btRigidBody *playerBody, const glm::vec3& targetVelocity) {
    glm::vec3 cpy = targetVelocity;
    if (playerBody->getLinearVelocity().length2() <= 5) {
        cpy *= 5 / targetVelocity.length();
    }
    playerBody->applyCentralImpulse({cpy.x, cpy.y, cpy.z});
}

void GamePlay::Player::castFootstep(btRigidBody *playerBody, BT_World *world, float rayLength) {
    btVector3 playerPos = playerBody->getWorldTransform().getOrigin();

    btVector3 rayFrom = playerPos;
    btVector3 rayTo = playerPos - btVector3(0, rayLength, 0);

    BT_ClosestRayResultCallbackTrigIdx rayCallback(rayFrom, rayTo);

    world->getDynamics()->rayTest(rayFrom, rayTo, rayCallback);
    if (!rayCallback.hasHit())return;

    const btCollisionObject* hitObj = rayCallback.m_collisionObject;
    BT_EntityTriangleMesh *entity = (BT_EntityTriangleMesh*) hitObj->getUserPointer();
    if (!entity) return;

    int triIndex = rayCallback.m_triangleIndex;
    if (triIndex == -1) return;

    btVector3 hitPoint = rayCallback.m_hitPointWorld;
    btVector3 hitNormal = rayCallback.m_hitNormalWorld;

    int materialID = entity->getTrigMaterial(triIndex);
    if (materialID == -1) return;

    Model *model = GameManager::getSceneColliderModelInst()->getModel();
    Material* material = model->materialById[materialID];

    std::string mat_name = material->getName();
    Sound::Footsteps::begin(
        GameManager::getWavSteps(mat_name),
        glm::vec3(playerPos.x(), playerPos.y() - 1.3f, playerPos.z()),
        glm::vec3(0.0f)
    );
}