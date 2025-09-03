#pragma once

#include "BulletDynamics/Dynamics/btDynamicsWorld.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"

#include "Syngine/world/Coordination.hpp"

#include <glm/glm.hpp>

using namespace syng;

namespace GamePlay {
namespace Player {

btRigidBody* createEntity(Coordination coords, float mass, float radius, float height);
void skipStaircase(btRigidBody* body, btDynamicsWorld* world, const glm::vec3& velocity, float dt, float capsuleHeight);
void applyVelocity(btRigidBody *playerBody, const glm::vec3& targetVelocity);

}
}
