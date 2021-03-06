/* =========================================================
		  V:Multiplayer - http://www.vmultiplayer.com

-- File: RPCFunctions.cpp
-- Project: Server
-- Author(s): m0niSx
-- Description: RPC functions source file
=============================================================*/

#include "Main.h"

extern CServer				*pServer;
extern CPlayerManager		*pPlayerManager;
extern CVehicleManager		*pVehicleManager;
extern CLuaInterface		*pLuaInterface;

void CRPCFunctions::RegisterFunctions()
{
	// Make sure we have a valid server pointer
	if(!pServer)
		return;

	// Register all the functions
	pServer->RegisterRPCFunction(ClientJoin, RPC_CLIENT_JOIN);
	pServer->RegisterRPCFunction(SpawnPlayer, RPC_SPAWN_PLAYER); 
	pServer->RegisterRPCFunction(EnterVehicle, RPC_ENTER_VEHICLE);
	pServer->RegisterRPCFunction(ExitVehicle, RPC_EXIT_VEHICLE);
	pServer->RegisterRPCFunction(VehicleEntryComplete, RPC_VEHICLE_ENTRY_COMPLETE);
	pServer->RegisterRPCFunction(VehicleExitComplete, RPC_VEHICLE_EXIT_COMPLETE);
	pServer->RegisterRPCFunction(RemoveFromVehicle, RPC_REMOVE_FROM_VEHICLE); 
	pServer->RegisterRPCFunction(ChatMessage, RPC_CHAT_MESSAGE); 
	pServer->RegisterRPCFunction(Command, RPC_COMMAND);
}

void CRPCFunctions::ClientJoin(BitStream *pBitStream, CSenderInfo *pSenderInfo)
{
	// Make sure we have a valid pointers
	if(!pBitStream || !pSenderInfo)
		return;

	EntityId senderId = pSenderInfo->GetSenderId();
	// Get the player name
	char szName[MAX_NAME_LENGTH];
	if(!pBitStream->Read(szName))
	{
		// Tell the server that the connection is rejected
		BitStream bitStream;
		bitStream.Write("Invalid name");
		pServer->RPC(RPC_JOIN_REJECTED, &bitStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, senderId, false);
		LogPrintf(true, "Connection rejected (Invalid name)");
		return;
	}
	// Make sure the player name is valid
	if(strlen(szName) <= 3 || strlen(szName) > MAX_NAME_LENGTH)
	{
		// Tell the server that the connection is rejected
		BitStream bitStream;
		bitStream.Write("Invalid name length (should be between 3 and 32");
		pServer->RPC(RPC_JOIN_REJECTED, &bitStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, senderId, false);
		LogPrintf(true, "Connection rejected (Invalid name length)");
		return;
	}
	// Make sure the player name is not already taken
	if(!pPlayerManager->IsNameExists(szName))
	{
		// Tell the server that the connection is rejected
		BitStream bitStream;
		bitStream.Write("Name already taken");
		pServer->RPC(RPC_JOIN_REJECTED, &bitStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, senderId, false);
		LogPrintf(true, "Connection rejected (Name already taken [%s])", szName);
		return;
	}
	// Add the player to the player manager
	pPlayerManager->AddPlayer(senderId, szName);
	// Get server name
	char szServerName[128];
	pServer->GetName(szServerName);
	// Tell the player that the connection is accepted
	BitStream bitStream;
	bitStream.Write(senderId);
	bitStream.Write(szServerName);
	pServer->RPC(RPC_JOIN_ACCEPTED, &bitStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, senderId, false);
	// Handle the player join
	pVehicleManager->HandlePlayerJoin(senderId);
	pPlayerManager->HandlePlayerJoin(senderId);
}

void CRPCFunctions::SpawnPlayer(BitStream *pBitStream, CSenderInfo *pSenderInfo)
{
	// Make sure we have a valid pointers
	if(!pSenderInfo)
		return;

	EntityId senderId = pSenderInfo->GetSenderId();
	// Make sure the player is connected
	if(senderId > MAX_PLAYERS || !pPlayerManager->IsPlayerConnected(senderId) || pPlayerManager->GetAt(senderId)->IsSpawned())
		return;

	// Spawn the player
	pPlayerManager->GetAt(senderId)->SpawnForWorld();
	// Get player name
	char szName[MAX_NAME_LENGTH];
	pPlayerManager->GetAt(senderId)->GetName(szName);
	// Display the log message
	LogPrintf(true, "[SPAWN] %s has spawned (ID: %d)", szName, senderId);
}

void CRPCFunctions::EnterVehicle(BitStream *pBitStream, CSenderInfo *pSenderInfo)
{
	// Make sure we have a valid pointers
	if(!pBitStream || !pSenderInfo)
		return;

	// Make sure the player is connected
	EntityId senderId = pSenderInfo->GetSenderId();
	if(senderId > MAX_PLAYERS || !pPlayerManager->IsPlayerConnected(senderId) || !pPlayerManager->GetAt(senderId)->IsSpawned())
		return;

	// Is he entering as a passenger
	bool bPassenger = false;
	if(!pBitStream->Read(bPassenger))
		bPassenger = false;

	// Make the player enter the vehicle
	pPlayerManager->GetAt(senderId)->EnterVehicle(bPassenger);
}

void CRPCFunctions::VehicleEntryComplete(BitStream *pBitStream, CSenderInfo *pSenderInfo)
{
	// Make sure we have a valid pointers
	if(!pBitStream || !pSenderInfo)
		return;

	// Make sure the player is connected
	EntityId senderId = pSenderInfo->GetSenderId();
	if(senderId > MAX_PLAYERS || !pPlayerManager->IsPlayerConnected(senderId) || !pPlayerManager->GetAt(senderId)->IsSpawned())
		return;

	// Make the player enter the vehicle
	pPlayerManager->GetAt(senderId)->VehicleEntryComplete();
}

void CRPCFunctions::ExitVehicle(BitStream *pBitStream, CSenderInfo *pSenderInfo)
{
	// Make sure we have a valid pointers
	if(!pBitStream || !pSenderInfo)
		return;

	EntityId senderId = pSenderInfo->GetSenderId();
	// Make sure the player is connected
	if(senderId > MAX_PLAYERS || !pPlayerManager->IsPlayerConnected(senderId) || !pPlayerManager->GetAt(senderId)->IsSpawned())
		return;

	// Make the player enter the vehicle
	pPlayerManager->GetAt(senderId)->ExitVehicle();
}

void CRPCFunctions::VehicleExitComplete(BitStream *pBitStream, CSenderInfo *pSenderInfo)
{
	// Make sure we have a valid pointers
	if(!pBitStream || !pSenderInfo)
		return;

	// Make sure the player is connected
	EntityId senderId = pSenderInfo->GetSenderId();
	if(senderId > MAX_PLAYERS || !pPlayerManager->IsPlayerConnected(senderId) || !pPlayerManager->GetAt(senderId)->IsSpawned())
		return;

	// Make the player enter the vehicle
	pPlayerManager->GetAt(senderId)->VehicleExitComplete();
}

void CRPCFunctions::RemoveFromVehicle(BitStream *pBitStream, CSenderInfo *pSenderInfo)
{
	// Make sure we have a valid pointers
	if(!pBitStream || !pSenderInfo)
		return;

	// Make sure the player is connected
	EntityId senderId = pSenderInfo->GetSenderId();
	if(senderId > MAX_PLAYERS || !pPlayerManager->IsPlayerConnected(senderId) || !pPlayerManager->GetAt(senderId)->IsSpawned())
		return;

	// Make the player enter the vehicle
	pPlayerManager->GetAt(senderId)->RemoveFromVehicle();
}

void CRPCFunctions::ChatMessage(BitStream *pBitStream, CSenderInfo *pSenderInfo)
{
	// Make sure we have a valid pointers
	if(!pBitStream || !pSenderInfo)
		return;

	EntityId senderId = pSenderInfo->GetSenderId();
	// Make sure the player is connected
	if(senderId > MAX_PLAYERS || !pPlayerManager->IsPlayerConnected(senderId))
		return;

	// Get the player message
	char szMessage[MAX_INPUT_MESSAGE];
	if(!pBitStream->Read(szMessage))
		return;

	// Call the playerChat event
	//if(pLuaInterface->CallEvent("playerChat", "ns", senderId, szMessage) > 0)
	//{
		// Get player name
		char szName[MAX_NAME_LENGTH];
		pPlayerManager->GetAt(senderId)->GetName(szName);
		// Send the clients the message
		BitStream bitStream;
		bitStream.Write(szName);
		bitStream.Write(szMessage);
		pServer->RPC(RPC_CHAT_MESSAGE, &bitStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, INVALID_PLAYER_ID, true);
		// Display the message
		LogPrintf(true, "[CHAT] %s(%d): %s", szName, senderId, szMessage);
	//}
}

void CRPCFunctions::Command(BitStream *pBitStream, CSenderInfo *pSenderInfo)
{
	// Make sure we have a valid pointers
	if(!pBitStream || !pSenderInfo)
		return;

	EntityId senderId = pSenderInfo->GetSenderId();
	// Make sure the player is connected
	if(senderId > MAX_PLAYERS || !pPlayerManager->IsPlayerConnected(senderId))
		return;

	// Read the command
	char szCommand[MAX_INPUT_MESSAGE];
	if(!pBitStream->Read(szCommand))
		return;

	// Call the playerCommand event
	pLuaInterface->CallEvent("playerCommand", "ns", senderId, szCommand);
}
