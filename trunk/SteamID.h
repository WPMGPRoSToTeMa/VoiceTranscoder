#pragma once

#include "UtilTypes.h"

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
	SteamID(qword nQuadPart);

	qword ConvertToQWord() const;

	bool IsValid() const;
private:
	union steamID_t {
		struct steamIDComponent_t {
			size_t m_nAccountID : 32; // unique account identifier
			size_t m_nAccountInstance : 20; // dynamic instance ID
			accountType_t m_accountType : 4; // type of account
			universe_t m_universe : 8; // universe this account belongs to
		} m_comp;
		qword m_nQuadPart;
	} m_steamid;
};