#pragma once

#define DPROTO_ENGINFO_API_VERSION_MAJOR 1
#define DPROTO_ENGINFO_API_VERSION_MINOR 0

#pragma pack(push, 4)
struct dp_enginfo_api_t {
	int version_major;
	int version_minor;

	//BEGIN verision 1.0
	unsigned int client_t_size;
	void* p_svs;
	void* p_clc_funcs;

	int* p_msg_readcount;
	int* p_msg_badread;
	void* p_net_message_addr;

	void* p_SZ_GetSpace;

	void* p_host_client;
	void* p_SV_DropClient;
	
	void* p_cvar_vars;

	//END version 1.0
};
#pragma pack(pop)

#define DPROTO_API_CVAR_NAME "dp_enginfo_api"
