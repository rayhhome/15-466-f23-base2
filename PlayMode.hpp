#include "Mode.hpp"

#include "Scene.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, fcw, fcc, rcw, rcc, bcw, bcc;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//current rotation speed
	float rotation_speed = 5.0f;
	const float max_speed = 10.0f;
	const float min_speed = 1.0f;

	//max reachable radius
	float max_radius = 32.0f;

	//goal and head positions
	glm::vec3 goal_pos = glm::vec3(0.0f);
	glm::vec3 head_pos = glm::vec3(0.0f);

	//movable objects:
	Scene::Transform *goal = nullptr;
	Scene::Transform *handle_root = nullptr;
	Scene::Transform *handle_branch = nullptr;
	Scene::Transform *handle_head = nullptr;
	// float wobble = 0.0f;
	
	//camera:
	Scene::Camera *camera = nullptr;

};
