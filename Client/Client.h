#pragma once

#include "Application.h"
#include <glm/mat4x4.hpp>
#include <iostream>
#include <string>
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <BitStream.h>
#include "../bootstrap/GameMessages.h"
#include "GameObject.h"
#include <vector>
#include "Camera.h"
#include <thread>
#include <unordered_map>
#include "../Server/Bullet.h"
#include "../Server/LuaObject.h"
#include "Renderer2D.h"
#include "Audio.h"

using namespace Objects;
class Client : public aie::Application {
public:

	Client();
	virtual ~Client();

	virtual bool startup(std::string ip);
	virtual void shutdown();

	void handleNetworkConnection();
	void initialiseClientConnection();
	// Handle incoming packets
	void handleNetworkMessages();
	void onSetClientIDPacket(RakNet::Packet* packet);
	void onReceivedClientDataPacket(RakNet::Packet * packet);
	void sendClientMousePos(glm::vec2 mousePos);
	void sendClientGameObject();
	void sendClientMouseClick(GameMessages message);
	virtual void update(float deltaTime);
	virtual void draw();
	void updateBullet();
	void updatePlayers();
protected:
	int ID;
	RakNet::RakPeerInterface* m_pPeerInterface;
	aie::Renderer2D*	m_2dRenderer;
	aie::Font*			m_font;
	aie::Texture* backGround;
	aie::Texture* player;
	aie::Texture* triangle;
	std::unordered_map<int, GameObject> m_ServerObjects;
	std::unordered_map<int, GameObject> m_GameObjects;
	std::unordered_map<int, int> m_scores;
	std::unordered_map<int, Bullet*> bullets;
	void GetDesktopResolutio1n(int& horizontal, int& vertical);
	int h, w;
	glm::vec2 resolution;
	RECT screen;
	glm::vec2 mousePos;
	std::string IP = "127.0.0.1";
	const unsigned short PORT = 5456;
	const char* bulletScriptPath = "Scripts/bullet.lua";
	Camera* camera;
};