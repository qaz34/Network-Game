#include "Client.h"
#include "Gizmos.h"
#include "Input.h"
#include "../bootstrap/Texture.h"
#include "../bootstrap/Font.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <thread>
#include <iostream>
#include "../Server/LuaObject.h"
#include <vector>
#include <algorithm>
#include <math.h>
#include <imgui.h>
using glm::vec2;
using glm::vec4;
using glm::mat4;
using aie::Gizmos;
void Client::GetDesktopResolutio1n(int& horizontal, int& vertical)
{

	// Get a handle to the desktop window
	const HWND hDesktop = GetDesktopWindow();
	// Get the size of screen to the variable desktop
	GetWindowRect(hDesktop, &screen);
	// The top left corner will have coordinates (0,0)
	// and the bottom right corner will have coordinates
	// (horizontal, vertical)
	horizontal = screen.right;
	vertical = screen.bottom;
}
Client::Client()
{

}

Client::~Client() {
}
void Client::onSetClientIDPacket(RakNet::Packet* packet)
{
	RakNet::BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
	bsIn.Read(ID);
	std::cout << "Set my client ID to: " << ID << std::endl;

	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)GameMessages::ID_SENDHEIGHTBEINGUSED);
	bs.Write(ID);
	glm::vec2 temp = glm::vec2(h, w);
	bs.Write((char*)&temp, sizeof(glm::vec2));

	m_pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(IP.c_str()), true);
	LuaObject::SendScript(LuaObject::ScriptFromFileToString(bulletScriptPath), ID, m_pPeerInterface);
}
void Client::sendClientGameObject()
{
	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)GameMessages::ID_CLIENT_CLIENT_GAMEOBJECT);
	bs.Write(ID);
	bs.Write((char*)&m_GameObjects[ID], sizeof(GameObject));
	m_pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0,
		RakNet::SystemAddress(IP.c_str()), true);
}
void Client::sendClientMousePos(glm::vec2 mousePos)
{
	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)GameMessages::ID_CLIENT_CLIENT_MOUSEPOS);
	bs.Write(ID);
	bs.Write((char*)&mousePos, sizeof(glm::vec2));
	m_pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0,
		RakNet::SystemAddress(IP.c_str()), true);
}
void Client::sendClientMouseClick(GameMessages message)
{
	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)message);
	bs.Write(ID);
	m_pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(IP.c_str()), true);
}
bool Client::startup(std::string ip) {

	setBackgroundColour(0.25f, 0.25f, 0.25f);
	m_2dRenderer = new aie::Renderer2D();
	backGround = new aie::Texture("./textures/StarBackground.jpg");
	player = new aie::Texture("./textures/Character.png");
	m_font = new aie::Font("./font/consolas.ttf", 32);
	triangle = new aie::Texture("./textures/Triangle.png");
	// initialise gizmo primitive counts
	Gizmos::create(100000, 100000, 100000, 100000);
	ImGuiStyle* style = &ImGui::GetStyle();
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0, 0, 0, 0);
	IP = ip;
	handleNetworkConnection();
	GetDesktopResolutio1n(h, w);
	resolution = glm::vec2(h, w);
	return true;
}

void Client::shutdown() {

	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)GameMessages::ID_DISCONECT);
	bs.Write(ID);
	m_pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(IP.c_str()), true);
	Gizmos::destroy();
}
void Client::updateBullet()
{
	static float lastTime = (float)clock();
	float deltaTime = (clock() - lastTime) / CLOCKS_PER_SEC;
	lastTime = (float)clock();
	for each (auto bullet in bullets)
	{
		bullet.second->Update(deltaTime, glm::vec2(w, h));
	}
}
void Client::updatePlayers()
{
	static float lastTime = (float)clock();
	float deltaTime = (clock() - lastTime) / CLOCKS_PER_SEC;
	lastTime = (float)clock();
	for each (auto& obj in m_GameObjects)
	{
		// advance the server objects position along its current velocity
		glm::vec2 vel = m_ServerObjects[obj.first].vel / glm::vec2(1280, 720);
		m_ServerObjects[obj.first].pos += vel * deltaTime;

		// move the actual gameobject towards its server position
		glm::vec2 deltaPos = m_ServerObjects[obj.first].pos - obj.second.pos;
		m_GameObjects[obj.first].pos += deltaPos * .5f;

	}
}
void Client::handleNetworkConnection()
{

	m_pPeerInterface = RakNet::RakPeerInterface::GetInstance();
	initialiseClientConnection();
}

void Client::initialiseClientConnection()
{

	RakNet::SocketDescriptor sd;
	m_pPeerInterface->Startup(1, &sd, 1);
	std::cout << "Connecting to server at: " << IP.c_str() << std::endl;
	RakNet::ConnectionAttemptResult res = m_pPeerInterface->Connect(IP.c_str(), PORT, nullptr, 0);
	if (res != RakNet::CONNECTION_ATTEMPT_STARTED)
	{
		std::cout << "Unable to start connection, Error number: " << res << std::endl;
	}
}

void Client::handleNetworkMessages()
{
	RakNet::Packet* packet;
	for (packet = m_pPeerInterface->Receive(); packet;
		m_pPeerInterface->DeallocatePacket(packet),
		packet = m_pPeerInterface->Receive())
	{
		switch (packet->data[0])
		{
		case ID_REMOTE_DISCONNECTION_NOTIFICATION:
			std::cout << "Another client has disconnected.\n";
			break;
		case ID_REMOTE_CONNECTION_LOST:
			std::cout << "Another client has lost the connection.\n";
			break;
		case ID_REMOTE_NEW_INCOMING_CONNECTION:
			std::cout << "Another client has connected.\n";
			break;
		case ID_CONNECTION_REQUEST_ACCEPTED:
			std::cout << "Our connection request has been accepted.\n";
			break;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			std::cout << "The server is full.\n";
			break;
		case ID_DISCONNECTION_NOTIFICATION:
			std::cout << "We have been disconnected.\n";
			break;
		case ID_CONNECTION_LOST:
			std::cout << "Connection lost.\n";
			break;
		case ID_SERVER_TEXT_MESSAGE:
		{
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
			RakNet::RakString str;
			bsIn.Read(str);
			std::cout << str.C_String() << std::endl;
			break;
		}
		case ID_SERVER_SET_CLIENT_ID:
			onSetClientIDPacket(packet);
			break;
		case ID_CLIENT_CLIENT_GAMEOBJECT:
			onReceivedClientDataPacket(packet);
			break;
		case ID_BULLET_MADE:
		{
			RakNet::BitStream bsIns(packet->data, packet->length, false);
			bsIns.IgnoreBytes(sizeof(RakNet::MessageID));
			Bullet* clientData = new Bullet();
			bsIns.Read((char*)&*clientData, sizeof(Bullet));
			clientData->m_luaState = luaL_newstate();
			luaL_openlibs(clientData->m_luaState);
			clientData->SetPointerVar(clientData->m_luaState, "self", clientData);
			std::string str = "bullet" + std::to_string(clientData->ID) + ".lua";
			if (!clientData->LoadScript(str.c_str()))
			{
				delete clientData;
				break;
			}
			clientData->RegisterLuaFunction("GetPos", clientData->luaGetPos);
			clientData->RegisterLuaFunction("GetVel", clientData->luaGetVel);
			clientData->RegisterLuaFunction("ApplyForce", clientData->LuaApplyForce);
			bullets[clientData->bulletNum] = clientData;

			break;
		}
		case ID_BULLET_HIT:
		{
			RakNet::BitStream bsIns(packet->data, packet->length, false);
			bsIns.IgnoreBytes(sizeof(RakNet::MessageID));
			int clientID;
			bsIns.Read(clientID);
			m_GameObjects[clientID].dead = true;
			int bullet;
			bsIns.Read(bullet);
			delete bullets[bullet];
			bullets.erase(bullet);
			break;
		}
		case ID_BULLET_OUT:
		{
			RakNet::BitStream bsIns(packet->data, packet->length, false);
			bsIns.IgnoreBytes(sizeof(RakNet::MessageID));
			int bullet;
			bsIns.Read(bullet);
			Bullet* tempDelete = bullets[bullet];
			bullets.erase(bullet);
			delete tempDelete;
			break;
		}
		case ID_RESET:
		{
			for (auto it = bullets.begin(); it != bullets.end();)
			{
				delete bullets[it->first];
				it = bullets.erase(it);
			}

			break;
		}
		case ID_DISCONECT:
		{
			RakNet::BitStream bsIns(packet->data, packet->length, false);
			bsIns.IgnoreBytes(sizeof(RakNet::MessageID));
			int clientID;
			bsIns.Read(clientID);
			m_GameObjects.erase(clientID);
			m_ServerObjects.erase(clientID);
			m_scores.erase(clientID);
			break;
		}
		case ID_SCORES:
		{
			RakNet::BitStream bsIns(packet->data, packet->length, false);
			bsIns.IgnoreBytes(sizeof(RakNet::MessageID));
			int clientID;
			int score;
			bsIns.Read(clientID);
			bsIns.Read(score);
			m_scores[clientID] = score;
			break;
		}
		case ID_SETGAMEOBJECTS:
		{
			RakNet::BitStream bsIns(packet->data, packet->length, false);
			bsIns.IgnoreBytes(sizeof(RakNet::MessageID));
			int clientID;
			bsIns.Read(clientID);
			GameObject clientData;
			bsIns.Read((char*)&clientData, sizeof(GameObject));
			m_GameObjects[clientID] = clientData;
			m_GameObjects[clientID].pos;// /= glm::vec2(1280, 720);
			m_GameObjects[clientID].vel = glm::vec2(0);
			m_ServerObjects[clientID] = clientData;
			m_ServerObjects[clientID].pos;// /= glm::vec2(1280, 720);
			m_ServerObjects[clientID].vel = glm::vec2(0);
			break;
		}
		case ID_LUASCRIPT:
		{
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
			int clientID;
			RakNet::RakString luaFile;
			bsIn.Read(clientID);
			bsIn.Read(luaFile);
			LuaObject::SaveScriptFromStringArray(luaFile.C_String(), clientID);
			break;
		}
		default:
			std::cout << "Received a message with a unknown id: " << packet->data[0];
			break;
		}
	}

}
void Client::onReceivedClientDataPacket(RakNet::Packet * packet)
{
	RakNet::BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
	int clientID;
	bsIn.Read(clientID);
	GameObject clientData;
	bsIn.Read((char*)&clientData, sizeof(GameObject));
	m_ServerObjects[clientID] = clientData;
}


void Client::update(float deltaTime) {

	float time = getTime();
	updateBullet();
	updatePlayers();
	Gizmos::clear();
	handleNetworkMessages();
	aie::Input* input = aie::Input::getInstance();
	ClipCursor(&screen);
	static int x, y;
	static glm::vec2 mousePos;
	input->getMouseXY(&x, &y);
	if (mousePos.x != x || mousePos.y != y)
	{
		mousePos.x = (float)x;
		mousePos.y = (float)y;
		sendClientMousePos(mousePos);
	}
	if (input->wasMouseButtonPressed(aie::INPUT_MOUSE_BUTTON_LEFT) && !m_GameObjects[ID].dead && ID > 0 && ID < 32)
		sendClientMouseClick(GameMessages::ID_CLIENT_MOUSECLICK);
	if (input->wasMouseButtonPressed(aie::INPUT_MOUSE_BUTTON_RIGHT) && ID > 0)
		sendClientMouseClick(GameMessages::ID_RMBDOWN);
	if (input->wasMouseButtonReleased(aie::INPUT_MOUSE_BUTTON_RIGHT) && ID > 0)
	{
		sendClientMouseClick(GameMessages::ID_RMBUP);
		mousePos.x = (float)x;
		mousePos.y = (float)y;
		sendClientMousePos(mousePos);
	}
	if (input->isKeyDown(aie::INPUT_KEY_ESCAPE))
		quit();
}
void drawTri(vec2 start, vec2 backDir, float width, float height, vec4 colour)
{
	Gizmos::add2DTri(start, start + backDir * height + glm::vec2(width / 2, -width / 2) * vec2(backDir.y, backDir.x), start + backDir * height + glm::vec2(-width / 2, width / 2) * vec2(backDir.y, backDir.x), colour);
}
void Client::draw()
{
	clearScreen();
	m_2dRenderer->begin();
	m_2dRenderer->setRenderColour(1, 1, 1, .5f);
	m_2dRenderer->drawSprite(backGround, resolution.x / 2, resolution.y / 2, resolution.x, resolution.y);
	int movement = 0;
	for each (auto var in m_ServerObjects)
	{
		m_2dRenderer->setRenderColour(m_ServerObjects[var.first].colour.x, m_ServerObjects[var.first].colour.y, m_ServerObjects[var.first].colour.z, m_ServerObjects[var.first].colour.w);
		m_2dRenderer->drawText(m_font, std::to_string(m_scores[var.first]).c_str(), 10 + (float)movement * 20, resolution.y - 50);
		movement += std::to_string(m_scores[var.first]).size();
	}
	for (auto& otherClient : m_GameObjects)
	{
		m_2dRenderer->setRenderColour(m_ServerObjects[otherClient.first].colour.x, m_ServerObjects[otherClient.first].colour.y, m_ServerObjects[otherClient.first].colour.z, m_ServerObjects[otherClient.first].colour.w);
		vec2 transPos = otherClient.second.pos * resolution;
		if (otherClient.second.pos.y < 0 || otherClient.second.pos.y > 1 || otherClient.second.pos.x < 0 || otherClient.second.pos.x > 1)
		{
			vec2 pos = transPos;
			//pos.x = std::clamp(pos.x, 10, resolution.x - 10);
			//pos.y = std::clamp(pos.y, 10, resolution.y - 10);
			if (otherClient.second.pos.y > 1) { pos.y = resolution.y - 10; }
			if (otherClient.second.pos.y < 0) { pos.y = 10; }
			if (otherClient.second.pos.x > 1) { pos.x = resolution.x - 10; }
			if (otherClient.second.pos.x < 0) { pos.x = 10; }
			float angle = std::atan2(transPos.x - pos.x, pos.y - transPos.y);
			m_2dRenderer->drawSprite(triangle, pos.x, pos.y, .15f * glm::distance(transPos, pos), .2f * glm::distance(transPos, pos), angle, 0, .5f, 0);
		}
		if (otherClient.second.dead)
		{
			m_2dRenderer->setRenderColour(m_ServerObjects[otherClient.first].colour.x, m_ServerObjects[otherClient.first].colour.y, m_ServerObjects[otherClient.first].colour.z, m_ServerObjects[otherClient.first].colour.w/ 4);
			m_2dRenderer->drawSprite(player, otherClient.second.pos.x * resolution.x, otherClient.second.pos.y * resolution.y, otherClient.second.radius * 2, otherClient.second.radius * 2);
		}
		else

			m_2dRenderer->drawSprite(player, otherClient.second.pos.x * resolution.x, otherClient.second.pos.y * resolution.y, otherClient.second.radius * 2, otherClient.second.radius * 2);
	}
	for (auto bullet : bullets)
	{
		m_2dRenderer->setRenderColour(bullet.second->colour.x, bullet.second->colour.y, bullet.second->colour.z, bullet.second->colour.w);
		m_2dRenderer->drawSprite(player, bullet.second->pos.x * resolution.x, bullet.second->pos.y * resolution.y, bullet.second->rad * 2, bullet.second->rad * 2);
	}
	m_2dRenderer->end();
}

