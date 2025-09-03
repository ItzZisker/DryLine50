#include "Player.hpp"

#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "BulletCollision/CollisionShapes/btCapsuleShape.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"

#include "LinearMath/btDefaultMotionState.h"

#include "LinearMath/btVector3.h"
#include "Syngine/world/Coordination.hpp"
#include "Syngine/utils/GameUtils.hpp"

#include <cmath>
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

void GamePlay::Player::skipStaircase(btRigidBody* body, btDynamicsWorld* world,
                                     const glm::vec3& velocity, float dt,
                                     float capsuleHeight) {
    const float stepMin = 0.07f;       // minimum ledge height to climb
    const float stepMax = 1.0f;       // maximum climbable step
    const float stepForwardOffset = 0.5f; // how far forward to check for steps
    const float climbSpeed = 6.0f;     // vertical climb speed (units/sec)
    const float rayStartFraction = 0.7f; // % of capsule height (0.0 = feet, 1.0 = head)

    btVector3 position = body->getWorldTransform().getOrigin();

    btVector3 forward(velocity.x, 0, velocity.z);
    if (forward.length2() < SIMD_EPSILON) return;

    for (int i = 0; i < 5; i++) {
        btVector3 forwardCpy = forward;

        float rot = M_PI / 6 * (i - 2);
        forwardCpy.normalize();
        float fx = forwardCpy.x(), fz = forwardCpy.z();
        forwardCpy.setX(fx * cos(rot) - fz * sin(rot));
        forwardCpy.setZ(fx * sin(rot) + fz * cos(rot));

        float rayStartHeight = capsuleHeight * rayStartFraction;
        btVector3 rayFrom = position + forwardCpy * stepForwardOffset + btVector3(0, rayStartHeight - 1.3f, 0);
        btVector3 rayTo   = rayFrom - btVector3(0, rayStartHeight, 0);

        btCollisionWorld::ClosestRayResultCallback cb(rayFrom, rayTo);
        world->rayTest(rayFrom, rayTo, cb);
        if (!cb.hasHit()) continue;

        float capsuleBottomY = position.getY() - 1.3f;
        float stepTopY = cb.m_hitPointWorld.getY();
        float delta = stepTopY - capsuleBottomY;

        if (delta > stepMin && delta <= stepMax) {
            float maxStepThisFrame = climbSpeed * dt;
            float stepAmount = btClamped(delta, 0.0f, maxStepThisFrame);

            btTransform t = body->getWorldTransform();
            btVector3 origin = t.getOrigin();
            origin.setY(origin.getY() + stepAmount);
            t.setOrigin(origin);

            body->setWorldTransform(t);
            if (body->getMotionState())
                body->getMotionState()->setWorldTransform(t);
            break;
        }
    }

    btVector3 vel = body->getLinearVelocity();
    vel.setX(velocity.x);
    vel.setZ(velocity.z);
    body->setLinearVelocity(vel);
}


void GamePlay::Player::applyVelocity(btRigidBody *playerBody, const glm::vec3& targetVelocity) {
    glm::vec3 cpy = targetVelocity;
    if (playerBody->getLinearVelocity().length2() <= 5) {
        cpy *= 5 / targetVelocity.length();
    }
    playerBody->applyCentralImpulse({cpy.x, cpy.y, cpy.z});
}