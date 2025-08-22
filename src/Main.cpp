#include "Syngine/Syngine.hpp"
#include "Syngine/engine/Concurrency.hpp"

#include "game/GameManager.hpp"

using namespace syng;

int main() {
	Concurrency::initMainThread();
	GameWindow window("Test", {1024, 768});
	window.addInitTask([&](GameWindow *window){ GameManager::start(window); });
	window.addRenderTask([&](GameWindow *window){ GameManager::render(window); });
	window.addCleanupTask([&](GameWindow *window){ GameManager::shutdown(window); });
	return window.initLoop();
}
