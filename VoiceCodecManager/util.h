#pragma once

#include "utiltypes.h"

#include <extdll.h>

#if 0
struct sv_t
{
	int Active;
	int Simulating;
	int Unks0;
	double tm;
	double prev_d_tm;
	int Unks01[3];
	char CurMapName[64];
	char PrevMapName[64];
	char MB_NextMapName[64];
	char CurMapFileName[64];
	model_t *BrushModel;
	int MapCRC;
	int Unks_a[49158];
	char *LocalModelNames[512];
	model_t *LocalModels[512];
	byte ModelsFlags[512];
	int Unks_a1[10818];
	int Max_Client;
	int NumEdicts;
	edict_t *Edicts;
	void *BaseLines;
	int SomeEntsData;
	int Unks_b[1];
	sizebuf_t ServerMsg;
	char ServerMsgBuffer[4000];
	sizebuf_t ServerReliableMsg;
	char ServerReliableBuffer[4000];
	sizebuf_t MulticastMsg;
	char MulticastBuffer[1024];
	sizebuf_t SpectatorMsg;
	char SpectatorBuffer[1024];
	sizebuf_t SignOnMsg;
	char SignOnBuffer[32768];
};
#endif

struct precachedevent_t
{
	int		m_iId;
	char *	m_pszName;
	int		m_iSize;
	void *	m_pFile;
};

typedef struct
{
	const char	*name;
	qboolean	fIsSound;
	int			iResId;
	int			iMD5Hash;
	FORCE_TYPE	force_state;
	vec3_t		mins, maxs;
} consistency2_t;

struct sizebuf_t
{
	char *descr;
	qboolean OverflowFlags;
	char *data;
	int maxsize;
	int cursize;
};

enum netsrc_t
{
	NS_CLIENT = 0x0,
	NS_SERVER = 0x1,
};

typedef struct fragbuf_s
{
	struct fragbuf_s	*next;
	int					bufferid;
	sizebuf_t			frag_message;
	byte				frag_message_buf[1400];
	qboolean			isfile;
	qboolean			isbuffer;
	char				filename[64];
	int					foffset;
	int					size;
} fragbuf_t;

typedef struct fragbufwaiting_s
{
	struct fragbufwaiting_s *next;
	int						fragbufcount;
	fragbuf_t *				fragbufs;
} fragbufwaiting_t;

typedef struct flowstats_s
{
	int		size;
	double	time;
} flowstats_t;

typedef struct flow_s
{
	flowstats_t stats[32];
	int			current;
	double		nextcompute;
	float		kbytespersec;
	float		avgkbytespersec;
} flow_t;

enum netadrtype_t
{
	NA_UNUSED = 0x0,
	NA_LOOPBACK = 0x1,
	NA_BROADCAST = 0x2,
	NA_IP = 0x3,
	NA_IPX = 0x4,
	NA_BROADCAST_IPX = 0x5,
};

struct netadr_t
{
	netadrtype_t type;
	byte ip[4];
	byte ipx[10];
	word port;
};

struct netchan_t
{
	int NetSrc;
	netadr_t adr;
	int ClientID;
	float last_recv;
	float connect_time;
	double rate;
	double clear_time;
	int incoming_sequence;
	int incoming_acknowledged;
	int incoming_reliable_acknowledged;
	int incoming_reliable_sequence;
	int outgoing_sequence;
	int reliable_sequence;
	int last_reliable_sequence;
	void *GetFragSize_param;
	void *GetFragSize_func;
	sizebuf_t msg;
	byte msg_buf[3992];
	int reliable_length;
	byte reliable_buf[3992];
	fragbufwaiting_t *waitlist[2];
	int reliable_fragment[2];
	unsigned int reliable_fragid[2];
	fragbuf_t *fragbufs[2];
	int fragbufcount[2];
	ushort frag_startpos[2];
	ushort frag_length[2];
	fragbuf_t *incomingbufs[2];
	qboolean incomingready[2];
	char incoming_filename[260];
	char *CustFileBuffer;
	int CustFileSize;
	flow_t flow[2];
};

struct server_t
{
	int					m_iActive;
	int					m_iPaused;
	int					m_iLoadGame;
	double				m_dbTime;
	double				m_dbLastFrameTime;
	int					m_iLastCheck;
	double				m_dbLastCheckTime;
	char				m_szMapName[64];
	char				m_szPrevMapName[64];
	char				m_szStartSpot[64];
	char				m_szFileMapName[64];
	void *				m_pWorldModel;
	int					m_iMapCRC;
	char				m_szClientDllMD5[16];
	resource_t			m_Resources[1280];
	int					m_nResources;
	consistency2_t		m_ConsistencyList[512];
	int					m_iNumConsistency;
	char *				m_pszPrecacheModels[512];
	void *				m_pModels[512];
	char				m_ModelFlags[512];
	precachedevent_t	m_PrecachedEvents[256];
	char *				m_pszPrecacheSound[512];
	void *				m_pSoundHashTable[512];
	int					m_fBuildedHashTable;
	char *				m_pszPrecacheGeneric[512];
	char				m_szMapResources[512][64];
	int					m_nMapResources;
	char *				m_pszLightStyles[64];
	int					m_iNumEdicts;
	int					m_iMaxEdicts;
	void *				m_pEdicts;
	void *				m_pBaselines;
	void *				m_pBaselineInstance;
	int					m_iServerState;
	sizebuf_t			m_ServerMsg;
	char				m_ServerMsgBuffer[4000];
	sizebuf_t			m_ServerReliableMsg;
	char				m_ServerReliableBuffer[4000];
	sizebuf_t			m_MulticastMsg;
	char				m_MulticastBuffer[1024];
	sizebuf_t			m_SpectatorMsg;
	char				m_SpectatorBuffer[1024];
	sizebuf_t			m_SignOnMsg;
	char				m_SignOnBuffer[32768];
};

typedef struct usercmd_s
{
	short			lerp_msec;
	byte			msec;
	vec3_t			viewangles;
	float			forwardmove;
	float			sidemove;
	float			upmove;
	byte			lightlevel;
	unsigned short	buttons;
	byte			impulse;
	byte			weaponselect;
	int				impact_index;
	vec3_t			impact_position;
} usercmd_t;

typedef struct event_args_s
{
	int		flags;
	int		entindex;
	float	origin[3];
	float	angles[3];
	float	velocity[3];
	int		ducking;
	float	fparam1;
	float	fparam2;
	int		iparam1;
	int		iparam2;
	int		bparam1;
	int		bparam2;
} event_args_t;

typedef struct event_info_s
{
	unsigned short	index;
	short			packet_index;
	short			entity_index;
	float			fire_time;
	event_args_t	args;
	int				flags;
} event_info_t;

struct clientid_t
{
	int ID_Type;
#ifdef _WIN32
	int ID_UNK1;
#endif
	int ID_Ident1;
	int ID_Ident2;
	int ID_IPAddr;
#ifdef _WIN32
	int ID_UNK2;
#endif
};

struct client_t
{
	qboolean		m_fActive;
	qboolean		m_fSpawned;
	qboolean		m_fZombie;
	qboolean		m_fConnected;
	qboolean		m_fResListReceived;
	qboolean		m_fNewReceived;
	qboolean		m_fConsistencyListSend;
	netchan_t		m_netchan;
	int				m_nChokeCount;
	int				m_iDeltaSequence;
	qboolean		m_fFakeClient;
	qboolean		m_fHltv;
	usercmd_t		m_ucmdLast;
	double			m_dbLastCmdTime;
	double			m_dbLastMoveTime;
	double			m_dbNextMoveTime;
	float			m_flPing;
	float			m_flPacketLoss;
	double			unk_0; // ???
	double			m_dbNextCheckPingTime;
	double			m_dbTimeBase;
	sizebuf_t		m_Datagram;
	char			m_szDatagramBuffer[4000];
	double			m_dbConnectionStarted;
	double			m_dbNextMessageTime;
	double			m_dbNextMessageInterval;
	qboolean		m_fSendMessage;
	qboolean		m_fSkipMessage;
	void *			frames;
	event_info_t	events[64];
	void *			edict;
	const void *	target_edict;
	int				m_iUserID;
	clientid_t		m_clientID;
	char			m_szUserInfo[256];
	qboolean		m_fUpdateUserInfo;
	float			m_flNextUserInfoUpdate;
	char			m_szHashedCDKey[64];
	char			m_szPlayerName[32];
	int				m_iTopColor;
	int				m_iBottomColor;
	int				unk_2;
	resource_t		player_customization;
	resource_t		resource_list;
	int				unk_3;
	customization_t customization_list;
	customization_t *customization_ptr;
	int				m_iUserMapCRC;
	qboolean		m_fLocalWeapons;
	qboolean		m_fLagCompensation;
	char			m_szPhysInfo[256];
	bool			m_bLoopback;
	unsigned long	m_bsVoiceStreams[2];
#ifndef _WIN32
	double			m_dbLastReceivedVoice;
	int				m_iSendResReceived;
#endif
};

typedef struct svstats_s
{
	double	m_dbActive;
	double	m_dbIdle;
	int		m_iCount;
	int		m_iTotalSituatMaxPlayers;
	int		m_iTotalSituatMinPlayers;
	float	m_flAverageSituatMaxPlayers;
	float	m_flAverageSituatMinPlayers;
	int		m_iMinPlayers;
	int		m_iMaxPlayers;
	float	m_flTotalPlayerPercentage;
	float	m_flAveragePlayerPercentage;
	int		m_iUnk8;
	int		m_iUnk9;
	int		m_iUnk10;
	int		m_iUnk11;
	int		m_iUnk12;
} svstats_t;

struct server_static_t
{
	qboolean			m_fDllInitialized;
	client_t *			m_pClients;
	int					m_iMaxClients;
	int					m_iMaxClientsLimit;
	int					m_iSpawnCount;
	int					m_iServerFlags;
	qboolean			m_fLogToFile;
	qboolean			m_fRemoteLog;
	netadr_t			m_RemoteLogAddr;
	void *				m_pLogFile;
	svstats_t			m_Stats;
	int					m_iVAC;
};