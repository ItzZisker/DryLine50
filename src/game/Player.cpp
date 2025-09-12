#include "Player.hpp"

#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "BulletCollision/CollisionShapes/btCapsuleShape.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"

#include "LinearMath/btDefaultMotionState.h"
#include "LinearMath/btVector3.h"

#include "Syngine/world/Coordination.hpp"
#include "Syngine/utils/GameUtils.hpp"

#include <glm/glm.hpp>

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

void GamePlay::Player::castFootstep() {
    
}