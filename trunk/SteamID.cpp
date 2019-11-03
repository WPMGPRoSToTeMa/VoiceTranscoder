#include "SteamID.h"

SteamID::SteamID(qword nQuadPart) {
	m_steamid.m_nQuadPart = nQuadPart;
}

qword SteamID::ConvertToQWord() const {
	return m_steamid.m_nQuadPart;
}

bool SteamID::IsValid() const {
	if (m_steamid.m_comp.m_accountType <= ACCOUNT_TYPE_INVALID || m_steamid.m_comp.m_accountType >= ACCOUNT_TYPE_MAX) {
		return false;
	}
	if (m_steamid.m_comp.m_universe <= UNIVERSE_INVALID || m_steamid.m_comp.m_universe >= UNIVERSE_MAX) {
		return false;
	}
	if (m_steamid.m_comp.m_accountType == ACCOUNT_TYPE_INDIVIDUAL) {
		if (m_steamid.m_comp.m_nAccountID == 0 || m_steamid.m_comp.m_nAccountInstance > STEAMUSER_WEBINSTANCE) {
			return false;
		}
	}
	if (m_steamid.m_comp.m_accountType == ACCOUNT_TYPE_CLAN) {
		if (m_steamid.m_comp.m_nAccountID == 0 || m_steamid.m_comp.m_nAccountInstance != 0) {
			return false;
		}
	}
	if (m_steamid.m_comp.m_accountType == ACCOUNT_TYPE_GAMESERVER) {
		if (m_steamid.m_comp.m_nAccountID == 0) {
			return false;
		}
		// Any limit on instances? We use them for local users and bots
	}

	return true;
}