#pragma once

#include "../dependencies/glm/glm/glm.hpp"
namespace Objects
{
	class GameObject
	{
	public:
		glm::vec2 pos;
		glm::vec4 colour;
		float radius = 10;
		glm::vec2 vel;
		int ID;
		bool dead = false;
		GameObject();
		~GameObject();
	};
}
