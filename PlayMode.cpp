#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <cstdlib>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <random>

#include <cassert>

auto generate_unit = []() -> glm::vec3 {
	srand(time(0));
	float x = (float)rand();
	float y = (float)rand();
	float z = (float)rand();
	float mag = sqrt(x*x + y*y + z*z);
	return glm::vec3(x/mag, y/mag, z/mag);
};

// the reaching arm program GL object name
GLuint arm_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > arm_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("data/arm.pnct"));
	arm_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > arm_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("data/arm.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = arm_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = arm_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;
	});
});

PlayMode::PlayMode() : scene(*arm_scene) {
	//get pointers to all objects that moves:
	for (auto &transform : scene.transforms) {
		if (transform.name == "Goal") goal = &transform;
		else if (transform.name == "HandleRoot") handle_root = &transform;
		else if (transform.name == "HandleBranch") handle_branch = &transform;
		else if (transform.name == "Head") handle_head = &transform;
	}
	if (goal == nullptr) throw std::runtime_error("Goal not found.");
	if (handle_root == nullptr) throw std::runtime_error("Handle root not found.");
	if (handle_branch == nullptr) throw std::runtime_error("Handle branch not found.");
	if (handle_head == nullptr) throw std::runtime_error("Handle head not found.");

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	// generate random goal position
	goal_pos = generate_unit() * max_radius;
	std::cout << goal->position.x << ", " << goal->position.y << ", " << goal->position.z << std::endl;
	goal_pos = goal->position;
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	// user key press actions
	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			// SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) { // left
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) { // right
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) { // up
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) { // down
			down.downs += 1;
			down.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_m) { // horizontal clockwise motion
			fcw.downs += 1;
			fcw.pressed = true;
		} else if (evt.key.keysym.sym == SDLK_n) { // horizontal counter-clockwise
			fcc.downs += 1;
			fcc.pressed = true;
		} else if (evt.key.keysym.sym == SDLK_k) { // root clockwise motion
			rcw.downs += 1;
			rcw.pressed = true;
		} else if (evt.key.keysym.sym == SDLK_j) { // root counter-clockwise
			rcc.downs += 1;
			rcc.pressed = true;
		} else if (evt.key.keysym.sym == SDLK_o) { // branch clockwise motion
			bcw.downs += 1;
			bcw.pressed = true;
		} else if (evt.key.keysym.sym == SDLK_i) { // branch counter-clockwise
			bcc.downs += 1;
			bcc.pressed = true;
		} 
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_m) {
			fcw.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_n) {
			fcc.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_k) {
			rcw.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_j) {
			rcc.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_o) {
			bcw.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_i) {
			bcc.pressed = false;
			return true;
		}
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			// SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	} else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			// glm::vec2 motion = glm::vec2(
			// 	evt.motion.xrel / float(window_size.y),
			// 	-evt.motion.yrel / float(window_size.y)
			// );
			// camera->transform->rotation = glm::normalize(
			// 	camera->transform->rotation
			// 	* glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f))
			// 	* glm::angleAxis(motion.y * camera->fovy, glm::vec3(1.0f, 0.0f, 0.0f))
			// );
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

	//slowly rotates through [0,1):
	// wobble += elapsed / 10.0f;
	// wobble -= std::floor(wobble);

	// hip->rotation = hip_base_rotation * glm::angleAxis(
	// 	glm::radians(5.0f * std::sin(wobble * 2.0f * float(M_PI))),
	// 	glm::vec3(0.0f, 1.0f, 0.0f)
	// );
	// upper_leg->rotation = upper_leg_base_rotation * glm::angleAxis(
	// 	glm::radians(7.0f * std::sin(wobble * 2.0f * 2.0f * float(M_PI))),
	// 	glm::vec3(0.0f, 0.0f, 1.0f)
	// );
	// lower_leg->rotation = lower_leg_base_rotation * glm::angleAxis(
	// 	glm::radians(10.0f * std::sin(wobble * 3.0f * 2.0f * float(M_PI))),
	// 	glm::vec3(0.0f, 0.0f, 1.0f)
	// );

	// movement extent
	float extent = elapsed * rotation_speed * float(M_PI);

	// arm movement according to key input
	if (fcw.pressed && !fcc.pressed) {
		handle_root->rotation = handle_root->rotation * glm::angleAxis(
			glm::radians(10.0f * extent),
			glm::vec3(0.0f, 0.0f, 1.0f)
		);
	}
	if (!fcw.pressed && fcc.pressed) {
		handle_root->rotation = handle_root->rotation * glm::angleAxis(
			glm::radians(-10.0f * extent),
			glm::vec3(0.0f, 0.0f, 1.0f)
		);
	}
	if (rcw.pressed && !rcc.pressed) {
		handle_root->rotation = handle_root->rotation * glm::angleAxis(
			glm::radians(20.0f * elapsed),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);
	}
	if (!rcw.pressed && rcc.pressed) {
		handle_root->rotation = handle_root->rotation * glm::angleAxis(
			glm::radians(-20.0f * elapsed),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);
	}
	if (bcw.pressed && !bcc.pressed) {
		handle_branch->rotation = handle_branch->rotation * glm::angleAxis(
			glm::radians(20.0f * elapsed),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);
	}
	if (!bcw.pressed && bcc.pressed) {
		handle_branch->rotation = handle_branch->rotation * glm::angleAxis(
			glm::radians(-20.0f * elapsed),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);
	}

	// the code for camera movement same as in the base code
	//move camera:
	{

		//combine inputs into a move:
		constexpr float PlayerSpeed = 30.0f;
		glm::vec2 move = glm::vec2(0.0f);
		if (left.pressed && !right.pressed) move.x =-1.0f;
		if (!left.pressed && right.pressed) move.x = 1.0f;
		if (down.pressed && !up.pressed) move.y =-1.0f;
		if (!down.pressed && up.pressed) move.y = 1.0f;

		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 frame_right = frame[0];
		//glm::vec3 up = frame[1];
		glm::vec3 frame_forward = -frame[2];

		camera->transform->position += move.x * frame_right + move.y * frame_forward;
	}

	std::cout << "handle head position: " << handle_head->position.x << ", " << handle_head->position.y << ", " << handle_head->position.z << std::endl;
	std::cout << "goal position: " << goal->position.x << ", " << goal->position.y << ", " << goal->position.z << std::endl;

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	GL_ERRORS(); //print any errors produced by this setup code

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		// constexpr float H = 0.09f;
		// lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
		// 	glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
		// 	glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
		// 	glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		// float ofs = 2.0f / drawable_size.y;
		// lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
		// 	glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
		// 	glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
		// 	glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
}
