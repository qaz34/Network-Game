#include <iostream>
#include <time.h>
#include <string>
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <BitStream.h>
#include "../bootstrap/GameMessages.h"
#include "../Client/GameObject.h"
#include "Agent.h"
#include <unordered_map>
#include <ctime>
#include "main.h"
#include <vector>
#include <mutex>
#include "Bullet.h"
#include <random>
#include <algorithm>
#include <lua.hpp>

using namespace Objects;
int gameMode = 0;
std::unordered_map<int, GameObject> ClientObjects;
std::unordered_map<int, Agent*> ClientAgents;
std::unordered_map<int, glm::vec2> ClientMoveToo;
std::unordered_map<int, glm::vec2> ClientAim;
std::unordered_map<int, glm::vec2> ClientSize;
std::unordered_map<int, Bullet*> Bullets;
lua_State* ls;
float sendDelay = .001f;
float sendDelayMax = .01f;
std::default_random_engine generator((unsigned int)time(NULL));
std::mutex mut;
int nxtBul = -1;
float TimeSinceReset = 0;
struct ColorChooser
{
	static glm::vec4 color(int id)
	{

		switch (id)
		{
		case 1:
			return glm::vec4(1, 0, 0, 1);

		case 2:
			return glm::vec4(0, 1, 0, 1);

		case 3:
			return glm::vec4(0, 0, 1, 1);

		case 4:
			return glm::vec4(1, 1, 0, 1);

		case 5:
			return glm::vec4(1, 0, 1, 1);
		case 6:
			return glm::vec4(1, 1, 1, 1);

		case 7:
			return glm::vec4(0, 1, 1, 1);
		default:
		{
			std::uniform_real_distribution<double> num(0, 1);
			return glm::vec4(num(generator), num(generator), num(generator), 1);
		}

		}
	}
};
glm::vec2 randomPos()
{
	std::uniform_int_distribution<int> x(10, 1270);
	std::uniform_int_distribution<int> y(10, 710);
	return glm::vec2(x(generator), y(generator));
}
void sendMessageToClients(RakNet::RakPeerInterface* pPeerInterface, std::string message)
{
	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)GameMessages::ID_SERVER_TEXT_MESSAGE);
	bs.Write(message.c_str());
	pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
};
void sendGameObject(RakNet::RakPeerInterface* pPeerInterface, GameMessages message, GameObject go, int id)
{
	RakNet::BitStream BS;
	BS.Write((RakNet::MessageID)message);
	BS.Write(id);
	BS.Write((char*)&go, sizeof(GameObject));
	pPeerInterface->Send(&BS, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}
void reset(int winner, RakNet::RakPeerInterface* pPeerInterface)
{
	ClientAgents[winner]->score++;
	for each (auto obj in ClientObjects)
	{
		ClientObjects[obj.first].pos = ClientAgents[obj.first]->pos = randomPos();
		ClientAgents[obj.first]->velocity = glm::vec2(0);
		ClientObjects[obj.first].dead = false;
		std::string str = "Player " + std::to_string(obj.first) + " has: " + std::to_string(ClientAgents[obj.first]->score) + '\n';
		std::cout << str.c_str();
		GameObject temp = obj.second;
		temp.pos /= ClientSize[0];
		sendGameObject(pPeerInterface, GameMessages::ID_SETGAMEOBJECTS, temp, obj.first);
		RakNet::BitStream bs;
		bs.Write((RakNet::MessageID)GameMessages::ID_SCORES);
		bs.Write(obj.first);
		bs.Write(ClientAgents[obj.first]->score);
		pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);

		sendMessageToClients(pPeerInterface, str);
	}
	for each(auto obj in Bullets)
	{
		obj.second->dead = true;
	}
	TimeSinceReset = 0;
}
void reset(RakNet::RakPeerInterface* pPeerInterface)
{
	std::cout << "Draw";
	for each (auto obj in ClientObjects)
	{
		ClientObjects[obj.first].pos = ClientAgents[obj.first]->pos = randomPos();
		ClientAgents[obj.first]->velocity = glm::vec2(0);
		ClientObjects[obj.first].dead = false;
		std::string str = "Player " + std::to_string(obj.first) + " has: " + std::to_string(ClientAgents[obj.first]->score) + '\n';
		std::cout << str.c_str();
		GameObject temp = obj.second;
		temp.pos /= ClientSize[0];
		sendGameObject(pPeerInterface, GameMessages::ID_SETGAMEOBJECTS, temp, obj.first);
		RakNet::BitStream bs;
		bs.Write((RakNet::MessageID)GameMessages::ID_SCORES);
		bs.Write(obj.first);
		bs.Write(ClientAgents[obj.first]->score);
		pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);

		sendMessageToClients(pPeerInterface, str);
	}
	for each(auto obj in Bullets)
	{
		obj.second->dead = true;
	}
	TimeSinceReset = 0;
}
void bulletUpdate(Bullet& bullet, float deltaTime, RakNet::RakPeerInterface* pPeerInterface)
{
	int hit = bullet.Update(deltaTime, ClientObjects);
	if (hit > 0)
	{
		RakNet::BitStream newBS;
		newBS.Write((RakNet::MessageID)GameMessages::ID_BULLET_HIT);
		newBS.Write(hit);
		newBS.Write(bullet.bulletNum);
		pPeerInterface->Send(&newBS, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
	}
	else if (hit == 0)
	{
		RakNet::BitStream newBS;
		newBS.Write((RakNet::MessageID)GameMessages::ID_BULLET_OUT);
		newBS.Write(bullet.bulletNum);
		pPeerInterface->Send(&newBS, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
	}
}
void Update(RakNet::RakPeerInterface* pPeerInterface)
{
	float lastTime = (float)clock();
	float deltaTime;

	while (true)
	{
		deltaTime = (clock() - lastTime) / CLOCKS_PER_SEC;
		lastTime = (float)clock();

		if (TimeSinceReset > 1 || TimeSinceReset == 0)
		{
			mut.lock();
			for each (auto bullet in Bullets)
			{
				bulletUpdate(*bullet.second, deltaTime, pPeerInterface);
			}
			for (auto& ClientGO : ClientAgents)
			{
				ClientGO.second->Update(deltaTime);
				ClientObjects[ClientGO.first].pos = ClientGO.second->pos;
				ClientObjects[ClientGO.first].vel = ClientGO.second->velocity;
				ClientAgents[ClientGO.first]->lastFired += deltaTime;
			}
			int i = -1;
			bool sending = false;
			for each (auto ClientObject in ClientObjects)
			{
				if (!ClientObject.second.dead)
				{
					i++;
					if (sendDelay > sendDelayMax || sending)
					{
						sending = true;
						sendDelay = 0;
						GameObject temp = ClientObject.second;
						temp.pos /= ClientSize[0];
						temp.vel = ClientAgents[ClientObject.first]->velocity;
						sendGameObject(pPeerInterface, GameMessages::ID_CLIENT_CLIENT_GAMEOBJECT, temp, ClientObject.first);
					}
				}
			}
			for (auto it = Bullets.begin(); it != Bullets.end();) {
				if (it->second->dead) {
					delete Bullets[it->first];
					it = Bullets.erase(it);
				}
				else
					it++;
			}
			if (i == 0 && ClientObjects.size() > 1)
			{
				for each (auto obj  in ClientObjects)
				{
					if (!obj.second.dead)
					{
						reset(obj.first, pPeerInterface);
						RakNet::BitStream newBS;
						newBS.Write((RakNet::MessageID)GameMessages::ID_RESET);
						pPeerInterface->Send(&newBS, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
						for each (auto obj  in ClientObjects)
						{
							GameObject temp = obj.second;
							temp.pos /= ClientSize[0];
							temp.vel /= ClientSize[0];
							sendGameObject(pPeerInterface, GameMessages::ID_SETGAMEOBJECTS, temp, obj.first);
						}

						break;
					}
				}
			}
			if (i == -1 && ClientObjects.size() > 1)
			{
				reset(pPeerInterface);
				RakNet::BitStream newBS;
				newBS.Write((RakNet::MessageID)GameMessages::ID_RESET);
				pPeerInterface->Send(&newBS, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);

			}
			mut.unlock();
		}
		TimeSinceReset += deltaTime;
		sendDelay += deltaTime;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}
void sendNewClientID(RakNet::RakPeerInterface* pPeerInterface, RakNet::SystemAddress& address)
{
	static int nextClientID = 1;
	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)GameMessages::ID_SERVER_SET_CLIENT_ID);
	bs.Write(nextClientID);
	mut.lock();
	ClientMoveToo[nextClientID] = ClientAim[nextClientID] = randomPos();

	ClientAgents[nextClientID] = new Agent(&ClientAim[nextClientID], &ClientMoveToo[nextClientID]);
	ClientAgents[nextClientID]->id = nextClientID;
	ClientObjects[nextClientID].pos = ClientAgents[nextClientID]->pos;
	ClientObjects[nextClientID].colour = ColorChooser::color(nextClientID);
	ClientObjects[nextClientID].ID = nextClientID;
	ClientObjects[nextClientID].dead = false;
	ClientObjects[nextClientID].vel = glm::vec2(0);
	ClientAgents[nextClientID]->GameModeChange(gameMode);
	ClientObjects[nextClientID].radius = ClientAgents[nextClientID]->rad;
	mut.unlock();
	nextClientID++;
	pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, address, false);

	for each (auto ClientObject in ClientObjects)
	{
		sendGameObject(pPeerInterface, GameMessages::ID_SETGAMEOBJECTS, ClientObject.second, ClientObject.first);
		sendGameObject(pPeerInterface, GameMessages::ID_CLIENT_CLIENT_GAMEOBJECT, ClientObject.second, ClientObject.first);
	}
}

int RecieveGO(RakNet::Packet * packet)
{
	RakNet::BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
	int clientID;
	bsIn.Read(clientID);
	GameObject clientData;
	bsIn.Read((char*)&clientData, sizeof(GameObject));

	ClientObjects[clientID] = clientData;

	return clientID;

}

int RecieveMP(RakNet::Packet * packet)
{
	RakNet::BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
	int clientID;
	bsIn.Read(clientID);
	glm::vec2 clientData;
	if (clientID > 0&& clientID < 100)
	{
		bsIn.Read((char*)&clientData, sizeof(glm::vec2));
		clientData /= ClientSize[clientID];
		if (!ClientAgents[clientID]->rmbDown)
			ClientMoveToo[clientID] = ClientAim[clientID] = clientData * ClientSize[0];
		else
		{
			ClientAim[clientID] = clientData * ClientSize[0];
		}
	}
	return clientID;

}

void HandleNetwork(RakNet::RakPeerInterface* pPeerInterface)
{

	RakNet::Packet* packet = nullptr;
	while (true)
	{
		for (packet = pPeerInterface->Receive(); packet;
			pPeerInterface->DeallocatePacket(packet),
			packet = pPeerInterface->Receive())
		{
			switch (packet->data[0])
			{
			case ID_NEW_INCOMING_CONNECTION:
				std::cout << "A connection is incoming.\n";
				sendNewClientID(pPeerInterface, packet->systemAddress);

				break;
			case ID_DISCONNECTION_NOTIFICATION:
				std::cout << "A client has disconnected.\n";
				break;
			case ID_CONNECTION_LOST:
				std::cout << "A client lost the connection.\n";
				break;
			case ID_CLIENT_CLIENT_GAMEOBJECT:
			{
				int clientID = RecieveGO(packet);
				for each (auto ClientObject in ClientObjects)
				{
					RakNet::BitStream newBS;
					newBS.Write((RakNet::MessageID)GameMessages::ID_CLIENT_CLIENT_GAMEOBJECT);
					newBS.Write(clientID);
					newBS.Write((char*)&ClientObject.second, sizeof(GameObject));
					pPeerInterface->Send(&newBS, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true);
				}

				break;
			}
			case ID_CLIENT_CLIENT_MOUSEPOS:
			{
				int clientID = RecieveMP(packet);
				break;
			}
			case ID_CLIENT_MOUSECLICK:
			{
				if (TimeSinceReset > 1)
				{
					mut.lock();
					RakNet::BitStream bsIn(packet->data, packet->length, false);
					bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
					int clientID;
					bsIn.Read(clientID);
					if (ClientAgents[clientID]->lastFired > ClientAgents[clientID]->GetFireDelay(gameMode))
					{
						Bullet* bul = ClientAgents[clientID]->Fire(ClientObjects[clientID].colour, ++nxtBul, gameMode);
						std::string str = "bullet" + std::to_string(clientID) + ".lua";
						if (!bul->LoadScript(str.c_str()))
						{
							delete bul;
							break;
						}
						Bullets[nxtBul] = bul;

						RakNet::BitStream newBS;
						newBS.Write((RakNet::MessageID)GameMessages::ID_BULLET_MADE);
						glm::vec2 pos = bul->pos;
						bul->pos = bul->pos / glm::vec2(1280.f, 720.f);
						newBS.Write((char*)bul, sizeof(Bullet));
						bul->pos = pos;
						pPeerInterface->Send(&newBS, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
						ClientAgents[clientID]->lastFired = 0;
					}
					mut.unlock();
				}
				break;
			}
			case ID_RMBDOWN:
			{
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				int clientID;
				bsIn.Read(clientID);
				ClientAgents[clientID]->rmbDown = true;
				break;
			}
			case ID_RMBUP:
			{
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				int clientID;
				bsIn.Read(clientID);
				ClientAgents[clientID]->rmbDown = false;
				break;
			}
			case ID_DISCONECT:
			{
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				pPeerInterface->Send(&bsIn, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				int clientID;
				bsIn.Read(clientID);
				ClientAgents.erase(clientID);
				ClientObjects.erase(clientID);
				break;
			}
			case ID_SENDHEIGHTBEINGUSED:
			{
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				//pPeerInterface->Send(&bsIn, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				int clientID;
				glm::vec2 size;
				bsIn.Read(clientID);
				bsIn.Read((char*)&size, sizeof(glm::vec2));
				ClientSize[clientID] = size;
				break;
			}
			case ID_LUASCRIPT:
			{
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				pPeerInterface->Send(&bsIn, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				int clientID;
				RakNet::RakString luaFile;
				bsIn.Read(clientID);
				bsIn.Read(luaFile);
				LuaObject::SaveScriptFromStringArray(luaFile.C_String(), clientID);
				for each (auto ClientObject in ClientObjects)
				{
					std::string str = "bullet" + std::to_string(ClientObject.first) + ".lua";
					LuaObject::SendScript(LuaObject::ScriptFromFileToString(str.c_str()), ClientObject.first, pPeerInterface);
				}
				break;
			}
			default:
				std::cout << "Received a message with a unknown id: " << packet->data[0];
				break;
			}
		}
	}
}
void CheckForGameMode()
{
	while (true)
	{
		std::string temp;
		std::getline(std::cin, temp);
		gameMode = (int)std::atoi(temp.c_str());
		for each (auto obj in ClientAgents)
		{
			ClientAgents[obj.first]->GameModeChange(gameMode);
			std::cout << std::to_string(obj.second->radius);
			ClientObjects[obj.first].radius = ClientAgents[obj.first]->rad;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}
int main()
{
	ls = luaL_newstate();

	const unsigned short PORT = 5456;
	RakNet::RakPeerInterface* pPeerInterface = nullptr;
	//Startup the server, and start it listening to clients
	std::cout << "Starting up the server..." << std::endl;
	//Initialize the Raknet peer interface first
	pPeerInterface = RakNet::RakPeerInterface::GetInstance();
	//Create a socket descriptor to describe this connection
	RakNet::SocketDescriptor sd(PORT, 0);
	//Now call startup - max of 32 connections, on the assigned port
	pPeerInterface->Startup(32, &sd, 1);
	pPeerInterface->SetMaximumIncomingConnections(32);
	std::thread updateThread(Update, pPeerInterface);
	std::thread gameModeThread(CheckForGameMode);
	ClientSize[0] = glm::vec2(1280, 720);
	HandleNetwork(pPeerInterface);



	return 0;
}