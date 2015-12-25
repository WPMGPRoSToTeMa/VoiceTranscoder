#include "SteamID.h"

SteamID::SteamID(uint64_t quadPart) {
	Set(quadPart);
}

uint64_t SteamID::ToUInt64() const {
	return m_steamid.m_quadPart;
}

bool SteamID::IsValid() const {
	if (m_steamid.m_comp.m_accountType <= ACCOUNT_TYPE_INVALID || m_steamid.m_comp.m_accountType >= ACCOUNT_TYPE_MAX) {
		return false;
	}
	if (m_steamid.m_comp.m_universe <= UNIVERSE_INVALID || m_steamid.m_comp.m_universe >= UNIVERSE_MAX) {
		return false;
	}
	if (m_steamid.m_comp.m_accountType == ACCOUNT_TYPE_INDIVIDUAL) {
		if (m_steamid.m_comp.m_accountID == 0 || m_steamid.m_comp.m_accountInstance > STEAMUSER_WEBINSTANCE) {
			return false;
		}
	}
	if (m_steamid.m_comp.m_accountType == ACCOUNT_TYPE_CLAN) {
		if (m_steamid.m_comp.m_accountID == 0 || m_steamid.m_comp.m_accountInstance != 0) {
			return false;
		}
	}
	if (m_steamid.m_comp.m_accountType == ACCOUNT_TYPE_GAMESERVER) {
		if (m_steamid.m_comp.m_accountID == 0) {
			return false;
		}
		// Any limit on instances? We use them for local users and bots
	}

	return true;
}

void SteamID::Set(uint64_t quadPart) {
	m_steamid.m_quadPart = quadPart;
}

void SteamID::SetUniverse(universe_t universe) {
	m_steamid.m_comp.m_universe = universe;
}

void SteamID::SetAccountType(accountType_t accountType) {
	m_steamid.m_comp.m_accountType = accountType;
}

void SteamID::SetAccountId(size_t accountId) {
	m_steamid.m_comp.m_accountID = accountId;
}