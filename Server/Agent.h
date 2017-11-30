#pragma once

#include <RakPeerInterface.h>
#include "../dependencies/glm/glm/glm.hpp"
#include <MessageIdentifiers.h>
#include "Bullet.h"
namespace Objects
{
	class Agent
	{
	public:
		glm::vec2 pos;
		glm::vec2 velocity;
		glm::vec2 heading;
		glm::vec2* aimPos;
		glm::vec2* mousePos;
		bool rmbDown = false;
		int id;
		int score = 0;
		float fireDelay = 0;
		float lastFired = 0;
		float maxVelocity = 600;
		float radius = 50;
		float rad = 10;
		float GetFireDelay(int gamemode);
		void GameModeChange(int gamemode);
		void Update(float deltaTime);
		Bullet * Fire(glm::vec4 colour, int _id, int gameMode);
		Agent(glm::vec2* aim, glm::vec2* mouseFollow);
		~Agent();
	};
}

