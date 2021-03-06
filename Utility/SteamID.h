#pragma once

#include "UtilTypes.h"

const size_t STEAMUSER_DESKTOPINSTANCE = 1;
const size_t STEAMUSER_WEBINSTANCE = 4;

// Steam universes. Each universe is a self-contained Steam instance.
enum universe_t : size_t {
	UNIVERSE_INVALID,
	UNIVERSE_PUBLIC,
	UNIVERSE_BETA,
	UNIVERSE_INTERNAL,
	UNIVERSE_DEV,
	// UNIVERSE_RC, // deprecated
	UNIVERSE_MAX
};

// Steam account types
enum accountType_t : size_t {
	ACCOUNT_TYPE_INVALID,
	ACCOUNT_TYPE_INDIVIDUAL, // single user account
	ACCOUNT_TYPE_MULTISEAT, // multiseat (e.g. cybercafe) account
	ACCOUNT_TYPE_GAMESERVER, // game server account
	ACCOUNT_TYPE_ANONGAMESERVER, // anonymous game server account
	ACCOUNT_TYPE_PENDING, // pending
	ACCOUNT_TYPE_CONTENTSERVER, // content server
	ACCOUNT_TYPE_CLAN,
	ACCOUNT_TYPE_CHAT,
	ACCOUNT_TYPE_CONSOLEUSER, // Fake SteamID for local PSN account on PS3 or Live account on 360, etc.
	ACCOUNT_TYPE_ANONUSER,
	// Max of 16 items in this field
	ACCOUNT_TYPE_MAX
};

class SteamID {
public:
	SteamID(uint64_t quadPart);
	SteamID() : SteamID(0) {}

	uint64_t ToUInt64() const;

	bool IsValid() const;

	void Set(uint64_t quadPart);
	void SetUniverse(universe_t universe);
	void SetAccountType(accountType_t accountType);
	void SetAccountId(size_t accountId);
	void SetAccountInstance(size_t instance);
private:
	union steamID_t {
		struct steamIDComponent_t {
			size_t m_accountID : 32; // unique account identifier
			size_t m_accountInstance : 20; // dynamic instance ID
			accountType_t m_accountType : 4; // type of account
			universe_t m_universe : 8; // universe this account belongs to
		} m_comp;
		uint64_t m_quadPart;
	} m_steamid;
};