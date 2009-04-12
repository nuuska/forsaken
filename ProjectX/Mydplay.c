
/*==========================================================================
 *  File:       mydplay.c
 *  Content:    Direct Play Multiplayer Stuff
 ***************************************************************************/

#define WIN32_EXTRA_LEAN

#include "main.h"
#include <stdio.h>
#include "typedefs.h"
#include <dplay.h>
#include "new3d.h"
#include "quat.h"
#include "CompObjects.h"
#include "bgobjects.h"
#include "Object.h"
#include "mydplay.h"
#include "2dtextures.h"
#include "mload.h"
#include "triggers.h"
#include "pickups.h"
#include "primary.h"
#include "secondary.h"
#include "mload.h"
#include "text.h"
#include "models.h"
#include "d3dmain.h"
#include "d3dappi.h"
#include "comm.h"
#include "Lights.h"
#include "title.h"
#include "multiplayer.h"
#include "controls.h"
#include "screenpolys.h"
#include "goal.h"
#include <stdio.h>
#include "mydplay2.h"
#include "XMem.h"
#include "Local.h"
#include "stats.h"
#include "version.h"

extern BOOL Debug;

BOOL	UseSendAsync			= TRUE;
BOOL	SessionGuidExists		= FALSE;

int	PacketGot[256];
int	PacketSize[256];

#ifdef MANUAL_SESSIONDESC_PROPAGATE
extern LPDPSESSIONDESC2                    glpdpSD_copy;
#endif

uint32 RecPacketSize				= 0;
uint32 MaxRecPacketSize				= 0;
uint32 BytesPerSecRec				= 0;
uint32 BytesPerSecSent				= 0;
uint32 CurrentBytesPerSecRec		= 0;
uint32 CurrentBytesPerSecSent		= 0;
uint32 MaxCurrentBytesPerSecRec		= 0;
uint32 MaxCurrentBytesPerSecSent	= 0;
float BytesPerSecTimer				= 0.0F;

// registry.c
extern LONG RegGet(LPCTSTR lptszName, LPBYTE lpData, LPDWORD lpdwDataSize);
extern LONG RegSet(LPCTSTR lptszName, CONST BYTE * lpData, DWORD dwSize);
extern LONG RegSetA(LPCTSTR lptszName, CONST BYTE * lpData, DWORD dwSize);

extern int FontHeight;
extern LIST	SessionsList;

extern DPID	PlayerIDs[ MAX_PLAYERS ];

extern	int16	PickupsGot[ MAXPICKUPTYPES ];
extern	uint32	TeamFlagMask[ MAX_TEAMS ];
extern	int8	TeamFlagPickup[ MAX_TEAMS ];

extern int16	NumPrimaryPickups;

extern GUID	ServiceProvidersGuids[];
extern	LIST	ServiceProvidersList;

extern	BOOL	CountDownOn;
extern char LevelNames[MAXLEVELS][128];

GUID autojoin_session_guid;
float FindSessionTimeout;
extern	SLIDER	MaxPlayersSlider;

extern	LONGLONG	LargeTime;
extern SLIDER	GoalScoreSlider;

extern LIST	LevelList;
extern	SLIDER	TimeLimit;
extern	int		GameType;
extern	BOOL	MyBrightShips;
extern BOOL BountyBonus;
extern SLIDER	BountyBonusSlider;

extern	BOOL	DS;

#define YourVersion "YOUR VERSION: " ProjectXVersion

#define	ONEOVER32767 (1.0F / 32767.0F)
#define	ONEOVER256 (1.0F / 256.0F)

#define	SHORTBANKMODIFIER (32767.0F / MAXBANKANGLE )
#define	SHORTANGLEMODIFIERPACK (32767.0F / 45.0F)
#define	SHORTANGLEMODIFIERUNPACK (45.0F / 32767.0F)

extern char CTFMessage[];
extern	BOOL IMustQuit;

extern BOOL NeedFlagAtHome;
extern BOOL OwnFlagTeleportsHome;
extern BOOL CanCarryOwnFlag;
extern SLIDER CTFSlider;

void SetTeamGoals( uint16 *TeamGoals );

uint32 BuildShipFlags( BYTE Player );
void UnPackShipFlags( BYTE Player , uint32 Flags );
BOOL	CheckIfPlayerCheats( BYTE player );

extern int FlagsToGenerate;
extern  uint32  Host_Flags[ MAX_PLAYERS ];

extern BOOL BountyHunt;
extern int BountyBonusInterval;

extern BOOL	CTF;
extern BOOL CaptureTheFlag;
extern int GoalScore;
extern int GameCompleted;
extern USERCONFIG	*player_config;

char *TeamName[];

extern int	AllowedBike[ ALLOWED_BIKETYPES ];
extern int32 ColPerspective;
extern BOOL Panel;

extern BOOL	BrightShips;
extern BOOL	BikeExhausts;
extern	BOOL IllegalTime;
extern	float	Countdown_Float;
extern	SLIDER  PacketsSlider;

extern char CurrentTauntVariant;

float DPlayUpdateInterval = 4.0F;
int OldPPSValue;
int OldUseShortPackets;
int OldColPerspective;
float PacketDelay = 4.0F;					// How long before I start to Declerate him.....
float HostDutyTimer = 0.0F;
float DPlayUpdateIntervalHostDuties = 30.0F;
void SetShipBankAndMat( OBJECT * ShipObjPnt );

#ifdef OPT_ON
#pragma optimize( "gty", on )
#endif


extern	SLIDER	MaxKillsSlider;
int16	MaxKills = 0;
extern	LONGLONG	DemoStartedTime;		// when the game started
extern	LONGLONG	DemoEndedTime;		// when the game started
extern	int32		DemoGameLoops;
extern	float	DemoAvgFps;
extern	LONGLONG	TimeDiff;

extern	LONGLONG	Freq;
extern	BOOL	Buffer1InUse;
extern	int		Buffer1Count;
extern	DWORD	Buffer1Offset;
extern	BYTE *	Buffer1Pnt;

extern	BOOL	Buffer2InUse;
extern	int		Buffer2Count;
extern	DWORD	Buffer2Offset;
extern	BYTE *	Buffer2Pnt;
extern	BOOL	DplayRecieveThread;

extern	BOOL	RecordDemo;
extern	BOOL	PlayDemo;
extern	BOOL	RecordDemoToRam;
extern	FILE	*	DemoFp;
extern	FILE	*	DemoFpClean;
extern	LIST	DemoList;
extern	LONGLONG	GameCurrentTime;		// How long the game has been going...
extern	char	ShortLevelNames[MAXLEVELS][32];
extern BOOL	bSoundEnabled;

extern	BOOL BadConnection[ MAX_PLAYERS+1 ];

extern	DPSESSIONDESC2	Sessions[MAXSESSIONS];

extern float framelag;

extern	float	Start_Shield;
extern	float	Start_Hull;

LONGLONG PingRequestTime;					// used to reply the time it was sent...
uint16		PingTimes[MAX_PLAYERS];		// How long does it take for a ping???

void SpecialDestroyGame( void );
void GetLevelName( char *buf, int bufsize, int level );
void StoreLevelNameInSessionDesc( char *str );

BOOL	CanDoDamage[MAX_PLAYERS+1];

extern	MATRIX			MATRIX_Identity;

extern	TEXT	MacroText1;
extern	TEXT	MacroText2;
extern	TEXT	MacroText3;
extern	TEXT	MacroText4;
extern  TEXT	QuickText;
extern  TEXT	QuickTextWhisper;
extern	int8	PrimaryToFireLookup[ MAXPRIMARYWEAPONS ];
extern	int8	SecondaryToFireLookup[ MAXSECONDARYWEAPONS ];
extern	float	GlobalFramelagAddition;
extern	BOOL ResetKillsPerLevel;

BOOL		JustGenerated = FALSE;
BOOL		JustPickedUpShield = FALSE;

int16	NextworkOldBikeNum = -1;

float		Interval = 0.0F;

extern	LONGLONG	GameStartedTime;
extern	LONGLONG	GameElapsedTime;
LONGLONG	TempTime;

extern	uint16		Seed1;
extern	uint16		Seed2;
extern	uint16		CopyOfSeed1;
extern	uint16		CopyOfSeed2;
extern	BOOL		RandomPickups;
extern	BOOL		HarmTeamMates;


int16	BikeModels[ MAXBIKETYPES ] = {

	MODEL_Borg,
	MODEL_Beard,
	MODEL_LaJay,
	MODEL_Excop,
	MODEL_Trucker,
	MODEL_Foetoid,
	MODEL_JapBird,
	MODEL_Nutter,
	MODEL_Rhesus,
	MODEL_Joe,
	MODEL_Shark,
	MODEL_HK5,
	MODEL_Nubia,
	MODEL_Mofisto,
	MODEL_Cerbero,
	MODEL_Slick,
	MODEL_Borg,

};

char					MyName[ 32 ] = "Default game";
char					NickName[ 32 ] = "Default game";
char					tempstr[256];
int16					NamesAreLegal = 1;
SHORTNAMETYPE			Names;	// all the players short Names....
BYTE					MyGameStatus = STATUS_Normal;//
BYTE					PreDemoEndMyGameStatus = 0;
BYTE					OverallGameStatus = STATUS_Null;
BYTE					GameStatus[MAX_PLAYERS + 1];	// Game Status for every Ship...
														// this tells the drones what status the host thinks hes in..
BYTE					OldGameStatus[MAX_PLAYERS + 1];	// Game Status for every Ship...
int16					Lives = 3;
int16					StatsCount = -1;

DPID PseudoHostDPID;
DPID HostDPID;

void SfxForCollectPickup( uint16 Owner, uint16 ID );

extern	LPDPSESSIONDESC2                    glpdpSD;            // current session description

void CreateReGen( uint16 ship );
BOOL InitLevels( char *levels_list );
extern	MODEL	Models[MAXNUMOFMODELS];

BOOL	HostDuties = FALSE;

LPDIRECTPLAY4A		glpDP=NULL;     // directplay object pointer

DPID						dcoID=0;        // our DirectPlay ID
LPGUID					g_lpGuid = NULL;
HANDLE					dphEvent = NULL;
BOOL						IsHost = TRUE;
BOOL						IsPseudoHost = FALSE;
BYTE						WhoIAm = 0;

BYTE					Current_Camera_View = 0;		// which object is currently using the camera view....
BOOL					RemoteCameraActive = FALSE;

PRIMBULLPOSDIR				TempPrimBullPosDir;
SECBULLPOSDIR				TempSecBullPosDir;
SHORTGLOBALSHIP			ShortGlobalShip;
VERYSHORTGLOBALSHIP	VeryShortGlobalShip;
SHIPDIEDINFO					TempDied;
SETTIME							TempTimeSet;
TITANBITS						TempTitanBits;

FSHORTGLOBALSHIP			FShortGlobalShip;
FVERYSHORTGLOBALSHIP	FVeryShortGlobalShip;
GROUPONLY_FVERYSHORTGLOBALSHIP	GroupOnly_FVeryShortGlobalShip;

PICKUPINFO						TempPickup;
VERYSHORTPICKUPINFO		VeryShortTempPickup;
KILLPICKUPINFO				TempKillPickup;
EXPSECONDARYINFO			TempExplodeSecondary;
TEAMGOALSINFO				TempTeamGoals;
SHOCKWAVEINFO				TempShockwave;
BGOUPDATEINFO				TempBGOUpdate;
SHIPHIT							TempShipHit;
SHORTSHIPHIT					ShortTempShipHit;

MISSEDPICKUPMSG			MissedPickups[ MAXMISSEDPICKUPS ];
SHORTPICKUP					MissedInitPickups[ MAXMISSEDPICKUPS ];
SHORTKILLPICKUP				MissedKillPickups[ MAXMISSEDPICKUPS ];
int16								NumMissedPickups = 0;
int16								NumMissedInitPickups = 0;
int16								NumMissedKillPickups = 0;

MISSEDMINEMSG				MissedMines[ MAXMISSEDMINES ];
SHORTMINE						MissedInitMines[ MAXMISSEDMINES ];
SHORTKILLMINE				MissedKillMines[ MAXMISSEDMINES ];
int16								NumMissedMines = 0;
int16								NumMissedInitMines = 0;
int16								NumMissedKillMines = 0;
BOOL								DPlayTest	= FALSE;

LONGLONG	DemoTimeSoFar = 0;

extern uint16	num_start_positions;

GLOBALSHIP              Ships[MAX_PLAYERS+1];
BOOL	DemoShipInit[MAX_PLAYERS+1];

LONGLONG	LastPacketTime[MAX_PLAYERS+1];
BYTE		CommBuff[MAX_BUFFER_SIZE];

BYTE	ReceiveCommBuff[MAXBIGPACKETBUFFERSIZE];
int		RealPacketSize[256];

PLAYERINFO PlayerInfo[MAX_PLAYERS];	// used by host to manage player numbers

#ifdef	GUARANTEEDMESSAGES
typedef struct GUARANTEEDMSGHEADER
{
	uint32		Ack;
	uint32		ID;
	LONGLONG	OverallTime;
	LONGLONG	Time;
	BYTE		MsgType;
	BOOL		OverideOlderMessage;
	BOOL		AllPlayers;
	int			MessageLength;
	BYTE		Count;
struct GUARANTEEDMSGHEADER * Next;
struct GUARANTEEDMSGHEADER * Prev;
	BYTE		Message;
}GUARANTEEDMSGHEADER, *LPGUARANTEEDMSGHEADER;

GUARANTEEDMSGHEADER * GMs = NULL;
GUARANTEEDMSGHEADER * OldestGMs = NULL;
int GuaranteedMessagesActive = 0;
int GuaranteedMessagesActiveMax = 0;
uint32 GuaranteedMessagesID = 1234567890;
int GuaranteedMessagesTime = 1;					// How many seconds before sending the message again...
int GuaranteedMessagesOverallTime = 10;			// How long to keep the message around for...
#endif


BOOL	ItsAGuranteed = FALSE;


extern	VECTOR			Forward;
extern	VECTOR			Backward;
extern	VECTOR			SlideUp;
extern	VECTOR			SlideDown;
extern	VECTOR			SlideLeft;
extern	VECTOR			SlideRight;
extern	VECTOR			NullVector;
extern	MLOADHEADER Mloadheader;
extern	int16 LevelNum;
extern	int16 NewLevelNum;
extern	float PowerLevel;
extern	SECONDARYWEAPONATTRIB SecondaryWeaponAttribs[ TOTALSECONDARYWEAPONS ];
extern	SECONDARYWEAPONBULLET SecBulls[MAXSECONDARYWEAPONBULLETS];
extern	SHIPCONTROL control;
extern	char * Messages[];
extern	int16	SelectedBike;
extern	BOOL	GodMode;

extern	BOOL	TeamGame;
extern	BYTE	TeamNumber[MAX_PLAYERS];
extern	uint16	RandomStartPosModify;

BOOL	ChAngleevel( void );
void	DebugPrintf( const char * format, ... );
void AddTitleMessage(LPTEXTMSG LpTextMsg);

MATRIX	TempMatrix = {
				1.0F, 0.0F, 0.0F, 0.0F,
				0.0F, 1.0F, 0.0F, 0.0F,
				0.0F, 0.0F, 1.0F, 0.0F,
				0.0F, 0.0F, 0.0F, 1.0F };

char	namebuf[256];
char	tempadd[1024];
DWORD	tempsize;

extern	char biker_name[256];
extern	DWORD                   Old_WhoIAm;
extern	DPSESSIONDESC2			Old_Session;
extern	DWORD					Old_Kills;
extern	DWORD					Old_Deaths;
extern	DWORD					Old_TeamNumber;
extern	char					Old_Name[256];
extern	BOOL					Rejoining;

extern	LONGLONG		Time_LastValue;
extern	BOOL			GoreGuts;
extern	int16			NumRegenPoints;
extern	int				NumOfTrigVars;
extern	int				NumOfTriggers;

DPID	from_dcoID;
BOOL	UseShortPackets = TRUE;//FALSE;

extern	int16	NumOrbs;
extern	PRIMARYWEAPONATTRIB PrimaryWeaponAttribs[ TOTALPRIMARYWEAPONS ];
extern	char *SecondaryDescription[MAXSECONDARYWEAPONS];
extern	char *PrimaryDescription[];
extern	int16	Host_PrimaryWeaponsGot[ MAX_PLAYERS ][ MAXPRIMARYWEAPONS ];
extern	int16	Host_SecondaryWeaponsGot[ MAX_PLAYERS ][ MAXSECONDARYWEAPONS ];
extern	float	Host_GeneralAmmo[ MAX_PLAYERS ];
extern	float	Host_PyroliteAmmo[ MAX_PLAYERS ];
extern	float	Host_SussGunAmmo[ MAX_PLAYERS ];
//extern	float	Host_GeneralAmmoUsed[ MAX_PLAYERS ];
extern	float	Host_GeneralAmmoUsed; //[ MAX_PLAYERS ];
//extern	float	Host_PyroliteAmmoUsed[ MAX_PLAYERS ];
extern	float	Host_PyroliteAmmoUsed;
//extern	float	Host_SussGunAmmoUsed[ MAX_PLAYERS ];
extern	float	Host_SussGunAmmoUsed;
extern	int16	Host_SecondaryAmmo[ MAX_PLAYERS ][ MAXSECONDARYWEAPONS ];
//extern	int16	Host_SecAmmoUsed[ MAX_PLAYERS ][ MAXSECONDARYWEAPONS ];
extern	int16	Host_SecAmmoUsed[ MAXSECONDARYWEAPONS ];
extern	int8	Host_SecWeaponsGot[ MAX_PLAYERS ][ MAXPICKUPS ];
extern	float	Host_NitroFuel[ MAX_PLAYERS ];
//extern	float	Host_NitroFuelUsed[ MAX_PLAYERS ];
extern	float	Host_NitroFuelUsed;
extern	int16	Host_PowerLevel[ MAX_PLAYERS ];
extern	float	Host_OrbAmmo[ MAX_PLAYERS ][ MAXMULTIPLES ];
extern	BYTE	Host_NumOfOrbitals[ MAX_PLAYERS ];
extern	BYTE	Host_CopyOfNumOfOrbitals[ MAX_PLAYERS ];
extern	float	Host_CopyOfOrbAmmo[ MAX_PLAYERS ][ MAXMULTIPLES ];
extern	int16	Host_CopyOfPowerLevel[ MAX_PLAYERS ];
extern	float	Host_CopyOfNitroFuel[ MAX_PLAYERS ];
extern	float	Host_CopyOfGeneralAmmo[ MAX_PLAYERS ];
extern	float	Host_CopyOfSussGunAmmo[ MAX_PLAYERS ];
extern	float	Host_CopyOfPyroliteAmmo[ MAX_PLAYERS ];
extern	int16	Host_PickupsGot[ MAX_PLAYERS ][ MAXPICKUPTYPES ];
extern	PICKUP	Pickups[ MAXPICKUPS ];
extern	int16	SecondaryFromPickupTab[ MAXSECONDARYWEAPONS * 2 ];

// statistics (stats.c)
extern void UpdateKillStats(int Killer, int Victim, int WeaponType, int Weapon);	// update the statistics

// message colours (Title.c)
extern int KillMessageColour; 
extern int SystemMessageColour;
extern int FlagMessageColour;
extern int PlayerMessageColour;
extern int PickupMessageColour;
extern int TauntMessageColour;
extern int MyMessageColour;

// sfx volume (Title.c)
extern SLIDER FlagSfxSlider;
float FlagVolume;

// watch player mode select player (Title.c)
extern SLIDER WatchPlayerSelect;

// calculates distance between you and a bike (Sfx.c)
float ReturnDistanceVolumeVector( VECTOR *sfxpos, uint16 sfxgroup, VECTOR *listenerpos, uint16 listenergroup, long *vol, VECTOR *sfxvector );

// stats used to send to other players if you are the host (stats.c)
extern uint8	KillStats[MAX_PLAYERS][MAX_PLAYERS];	
extern uint8	BonusStats[MAX_PLAYERS];
extern uint8	KillCounter[MAX_PLAYERS];	

char* msg_to_str( int msg_type )
{
	switch( msg_type )
	{
    case MSG_YOUQUIT:
		return "MSG_YOUQUIT";
		break;
    case MSG_BIKENUM:
		return "MSG_BIKENUM";
		break;
    case MSG_NAME:
		return "MSG_NAME";
		break;
	case MSG_HEREIAM:
		return "MSG_HEREIAM";
        break;
    case MSG_INIT:
		return "MSG_INIT";
		break;
    case MSG_VERYSHORTUPDATE:
		return "MSG_VERYSHORTUPDATE";
        break;
    case MSG_UPDATE:
		return "MSG_UPDATE";
        break;
    case MSG_FUPDATE:
		return "MSG_FUPDATE";
        break;
    case MSG_VERYSHORTFUPDATE:
		return "MSG_VERYSHORTFUPDATE";
        break;
    case MSG_GROUPONLY_VERYSHORTFUPDATE:
		return "MSG_GROUPONLY_VERYSHORTFUPDATE";
        break;
    case MSG_DROPPICKUP:
		return "MSG_DROPPICKUP";
        break;
    case MSG_VERYSHORTDROPPICKUP:
		return "MSG_VERYSHORTDROPPICKUP";
        break;
    case MSG_KILLPICKUP:
		return "MSG_KILLPICKUP";
        break;
    case MSG_TEAMGOALS:
		return "MSG_TEAMGOALS";
        break;
    case MSG_SHOCKWAVE:
		return "MSG_SHOCKWAVE";
        break;
    case MSG_BGOUPDATE:
		return "MSG_BGOUPDATE";
        break;
    case MSG_PRIMBULLPOSDIR:
		return "MSG_PRIMBULLPOSDIR";
        break;
    case MSG_SECBULLPOSDIR:
		return "MSG_SECBULLPOSDIR";
        break;
    case MSG_TITANBITS:
		return "MSG_TITANBITS";
        break;
    case MSG_SHIPHIT:
		return "MSG_SHIPHIT";
        break;
    case MSG_SHORTSHIPHIT:
		return "MSG_SHORTSHIPHIT";
        break;
    case MSG_SHIPDIED:
		return "MSG_SHIPDIED";
        break;
    case MSG_REQTIME:
		return "MSG_REQTIME";
        break;
    case MSG_SETTIME:
		return "MSG_SETTIME";
        break;
    case MSG_STATUS:
		return "MSG_STATUS";
        break;
	case MSG_DPLAYUPDATE:
		return "MSG_DPLAYUPDATE";
		break;
    case MSG_LONGSTATUS:
		return "MSG_LONGSTATUS";
        break;
    case MSG_SHORTPICKUP:
		return "MSG_SHORTPICKUP";
        break;
    case MSG_SHORTREGENSLOT:
		return "MSG_SHORTREGENSLOT";
        break;
    case MSG_SHORTTRIGGER:
		return "MSG_SHORTTRIGGER";
        break;
    case MSG_SHORTTRIGVAR:
		return "MSG_SHORTTRIGVAR";
        break;
    case MSG_SHORTMINE:
		return "MSG_SHORTMINE";
        break;
    case MSG_TEXTMSG:
		return "MSG_TEXTMSG";
		break;
    case MSG_PINGREQUEST:
		return "MSG_PINGREQUEST";
		break;
    case MSG_PINGREPLY:
		return "MSG_PINGREPLY";
		break;
    case MSG_ACKMSG:
		return "MSG_ACKMSG";
		break;
#ifdef MANUAL_SESSIONDESC_PROPAGATE
	case MSG_SESSIONDESC:
		return "MSG_SESSIONDESC";
		break;
#endif
	}
	return "UNKNOWN";
}


BOOL CheckForName( BYTE Player )
{
	char	*			NamePnt;
	char	*			NamePnt2;
    HRESULT				hr;
	int					i;
	LPDPNAME			lpDpName;

	if( Names[Player][0] == 0 )
	{
		tempsize = 256;
		hr = IDirectPlayX_GetPlayerName( glpDP , from_dcoID , (LPVOID) &namebuf[0] , (LPDWORD) &tempsize );
	
		if( hr == DP_OK )
		{
			lpDpName = (LPDPNAME) &namebuf[0];
			lpDpName->dwSize = sizeof(DPNAME);
			NamePnt = (char*) &Names[Player][0];			
			NamePnt2 = (char*) lpDpName->lpszShortNameA;
			for( i = 0 ; i < 7 ; i++ )
			{
				*NamePnt++ = *NamePnt2++;
			}
			Names[Player][7] = 0;
		}
		return TRUE;
	}
	return FALSE;
}

void SendANormalUpdate( void )
{
	VECTOR	Move_Off;

	if( !UseShortPackets )
	{
		ShortGlobalShip.Flags = BuildShipFlags(WhoIAm);

		ShortGlobalShip.GroupImIn	= (BYTE) Ships[WhoIAm].Object.Group;    // 
		ShortGlobalShip.Pos			= Ships[WhoIAm].Object.Pos		;	  // x , y , z position
		ShortGlobalShip.Move_Off	= Ships[WhoIAm].Move_Off;	  // Last Movement..x , y , z
		ShortGlobalShip.Quat		= Ships[WhoIAm].Object.Quat;	  // Final inverse view Quat.......after banking.. 
		ShortGlobalShip.Status		= MyGameStatus;
		ShortGlobalShip.Angle		= Ships[WhoIAm].Object.Angle;
#ifdef	SHORTBANK
		ShortGlobalShip.Bank			= (int16) (Ships[ WhoIAm ].Object.Bank * SHORTBANKMODIFIER);
#else
		ShortGlobalShip.Bank = Ships[ WhoIAm ].Object.Bank;
#endif
		SendGameMessage(MSG_UPDATE, 0, 0, 0, 0);
	}
	else
	{
		VeryShortGlobalShip.Flags = BuildShipFlags(WhoIAm);
		VeryShortGlobalShip.GroupImIn			= (BYTE) Ships[WhoIAm].Object.Group;    // 
		VeryShortGlobalShip.Pos.x					= (int16) Ships[WhoIAm].Object.Pos.x;
		VeryShortGlobalShip.Pos.y					= (int16) Ships[WhoIAm].Object.Pos.y;
		VeryShortGlobalShip.Pos.z					= (int16) Ships[WhoIAm].Object.Pos.z;
		Move_Off = Ships[WhoIAm].Move_Off;
		NormaliseVector( &Move_Off );
		VeryShortGlobalShip.Move_Off_Scalar	= (uint16) ( 256.0F * VectorLength( &Ships[WhoIAm].Move_Off ) );
		VeryShortGlobalShip.Move_Off.x			= (int16) (Move_Off.x * 32767.0F);
		VeryShortGlobalShip.Move_Off.y			= (int16) (Move_Off.y * 32767.0F);
		VeryShortGlobalShip.Move_Off.z			= (int16) (Move_Off.z * 32767.0F);
		VeryShortGlobalShip.Quat.w				= (int16) (Ships[WhoIAm].Object.Quat.w * 32767.0F);
		VeryShortGlobalShip.Quat.x				= (int16) (Ships[WhoIAm].Object.Quat.x * 32767.0F);
		VeryShortGlobalShip.Quat.y				= (int16) (Ships[WhoIAm].Object.Quat.y * 32767.0F);
		VeryShortGlobalShip.Quat.z				= (int16) (Ships[WhoIAm].Object.Quat.z * 32767.0F);
		VeryShortGlobalShip.Status				= MyGameStatus;
		VeryShortGlobalShip.Angle.x				= (int16) (Ships[WhoIAm].Object.Angle.x * SHORTANGLEMODIFIERPACK );
		VeryShortGlobalShip.Angle.y				= (int16) (Ships[WhoIAm].Object.Angle.y * SHORTANGLEMODIFIERPACK );
		VeryShortGlobalShip.Angle.z				= (int16) (Ships[WhoIAm].Object.Angle.z * SHORTANGLEMODIFIERPACK );
		VeryShortGlobalShip.Bank					= (int16) (Ships[ WhoIAm ].Object.Bank * SHORTBANKMODIFIER);
		SendGameMessage(MSG_VERYSHORTUPDATE, 0, 0, 0, 0);
	}
}

 
void DplayGameUpdate()
{
	int i;
	VECTOR	Move_Off;

	if( ( Ships[WhoIAm].Object.Flags & ( SHIP_PrimFire | SHIP_SecFire | SHIP_MulFire ) ) )
		Ships[WhoIAm].Object.Noise = 1.0F;

	if( dcoID == 0 )
	{
		// Has to be done to stop missiles getting stuck in walls....!!!!!!!!!!!!!
		Ships[ WhoIAm ].Object.Flags &=  ~( SHIP_PrimFire | SHIP_SecFire | SHIP_MulFire );
	}
	else
	{
		ReceiveGameMessages();
		
		if( ( Ships[WhoIAm].Object.Flags & ( SHIP_PrimFire | SHIP_SecFire | SHIP_MulFire ) ) )
		{
			if( !UseShortPackets )
			{
				FShortGlobalShip.Flags					= BuildShipFlags(WhoIAm);
				FShortGlobalShip.GroupImIn			= (BYTE) Ships[WhoIAm].Object.Group;    // 
				FShortGlobalShip.Primary				= PrimaryToFireLookup[ Ships[WhoIAm].Primary ];
				FShortGlobalShip.Secondary			= SecondaryToFireLookup[ Ships[WhoIAm].Secondary ];
				FShortGlobalShip.Pos						= Ships[WhoIAm].Object.Pos		;	  // x , y , z position
				FShortGlobalShip.Move_Off				= Ships[WhoIAm].Move_Off;	  // Last Movement..x , y , z
				FShortGlobalShip.Quat					= Ships[WhoIAm].Object.Quat;// Final inverse view Quat.......after banking.. 
				FShortGlobalShip.PrimPowerLevel		= (BYTE) Ships[ WhoIAm ].PrimPowerLevel;
				FShortGlobalShip.Angle					= Ships[ WhoIAm ].Object.Angle;
#ifdef	SHORTBANK
				FShortGlobalShip.Bank					= (int16) (Ships[ WhoIAm ].Object.Bank * SHORTBANKMODIFIER);
#else
				FShortGlobalShip.Bank					= Ships[ WhoIAm ].Object.Bank;
#endif
				SendGameMessage(MSG_FUPDATE, 0, 0, 0, 0);
			}
			else
			{
				FVeryShortGlobalShip.Flags					= BuildShipFlags(WhoIAm);
				FVeryShortGlobalShip.GroupImIn				= (BYTE) Ships[WhoIAm].Object.Group;    // 
				FVeryShortGlobalShip.Pos.x					= (int16) Ships[WhoIAm].Object.Pos.x;
				FVeryShortGlobalShip.Pos.y					= (int16) Ships[WhoIAm].Object.Pos.y;
				FVeryShortGlobalShip.Pos.z					= (int16) Ships[WhoIAm].Object.Pos.z;
				Move_Off = Ships[WhoIAm].Move_Off;
				NormaliseVector( &Move_Off );
				FVeryShortGlobalShip.Move_Off_Scalar		= (uint16) ( 256.0F * VectorLength( &Ships[WhoIAm].Move_Off ) );
				FVeryShortGlobalShip.Move_Off.x			= (int16) (Move_Off.x * 32767.0F);
				FVeryShortGlobalShip.Move_Off.y			= (int16) (Move_Off.y * 32767.0F);
				FVeryShortGlobalShip.Move_Off.z			= (int16) (Move_Off.z * 32767.0F);
				FVeryShortGlobalShip.Quat.w					= (int16) (Ships[WhoIAm].Object.Quat.w * 32767.0F);
				FVeryShortGlobalShip.Quat.x					= (int16) (Ships[WhoIAm].Object.Quat.x * 32767.0F);
				FVeryShortGlobalShip.Quat.y					= (int16) (Ships[WhoIAm].Object.Quat.y * 32767.0F);
				FVeryShortGlobalShip.Quat.z					= (int16) (Ships[WhoIAm].Object.Quat.z * 32767.0F);
				FVeryShortGlobalShip.Angle.x					= (int16) (Ships[WhoIAm].Object.Angle.x * SHORTANGLEMODIFIERPACK );
				FVeryShortGlobalShip.Angle.y					= (int16) (Ships[WhoIAm].Object.Angle.y * SHORTANGLEMODIFIERPACK );
				FVeryShortGlobalShip.Angle.z					= (int16) (Ships[WhoIAm].Object.Angle.z * SHORTANGLEMODIFIERPACK );
				FVeryShortGlobalShip.Bank						= (int16) (Ships[ WhoIAm ].Object.Bank * SHORTBANKMODIFIER);
				FVeryShortGlobalShip.Primary					= PrimaryToFireLookup[ Ships[WhoIAm].Primary ];
				FVeryShortGlobalShip.Secondary				= SecondaryToFireLookup[ Ships[WhoIAm].Secondary ];
				FVeryShortGlobalShip.PrimPowerLevel		= (BYTE) Ships[ WhoIAm ].PrimPowerLevel;
				
				SendGameMessage(MSG_VERYSHORTFUPDATE, 0, 0, 0, 0);
			}
			Ships[ WhoIAm ].Object.Flags &=  ~( SHIP_PrimFire | SHIP_SecFire | SHIP_MulFire );
			Interval = DPlayUpdateInterval;
		
		}
		else
		{
			Interval -= framelag;
			if( Interval <= 0.0F )
			{
				Interval = DPlayUpdateInterval;
				SendANormalUpdate();
			}
		}

 		HostDutyTimer -= framelag;

		if( HostDutyTimer <= 0.0F )
		{
			HostDutyTimer = DPlayUpdateIntervalHostDuties;
			HostDuties = FALSE;
			// If someone has joined or somebody requests it then send everyone a stats update
			if( IsHost == TRUE )
			{			
				for( i = 0 ; i < MAX_PLAYERS ; i++ )
				{
					if( ( i != WhoIAm ) && (GameStatus[i] == STATUS_Joining ) ) 
					{
						if( Ships[i].RegenSlots != 0 )
						{
							SendGameMessage( MSG_SHORTREGENSLOT, 0, (BYTE) i, 0, 0 );
							HostDuties = TRUE;
							Ships[i].RegenSlots = 0;
						}
						else if( Ships[i].Triggers != 0 ) 
						{
							SendGameMessage( MSG_SHORTTRIGGER, 0, (BYTE) i, 0, 0 );
							HostDuties = TRUE;
							Ships[i].Triggers = 0;
						}
						else if( Ships[i].TrigVars != 0 )
						{
							SendGameMessage( MSG_SHORTTRIGVAR, 0, (BYTE) i, 0, 0 );
							HostDuties = TRUE;
							Ships[i].TrigVars = 0;
						}
						else if( Ships[i].Mines != 0 )
						{
							SendGameMessage( MSG_SHORTMINE, 0, (BYTE) i, 0, 0 );
							HostDuties = TRUE;
							Ships[i].Mines = 0;
						}
						else if(	Ships[i].Pickups != 0 )
						{
							SendGameMessage( MSG_SHORTPICKUP, 0, (BYTE) i, 0, 0 );
							HostDuties = TRUE;
							Ships[i].Pickups = 0;
						}
						// reset them untill we get another request.....
					}
				}
			}
		}
	}
	if( IsHost )
	{
		if( GMs )
		{
			// as the host you must wait for guarenteed message to finish....
			HostDuties = TRUE;
		}
	}else{
		if( GMs )
		{
			// Even if im not the host I should not quit if guarenteed messages have been qued...
			HostDuties = TRUE;
		}else{
			HostDuties = FALSE;
		}
	}

	// The Host can Dynamicaly change settings
	if( IsHost && !CurrentMenu )
	{
		// changed packet rate
		if ( OldPPSValue != PacketsSlider.value )
		{
			OldPPSValue = PacketsSlider.value;
			DPlayUpdateInterval	= (60.0F / PacketsSlider.value);
			SendGameMessage(MSG_DPLAYUPDATE, 0, 0, 0, 0);
			AddColourMessageToQue(SystemMessageColour, "%d %s" , PacketsSlider.value , PACKETS_PER_SECOND_SET );
		}
		// changed collision perspective
		if ( OldColPerspective != ColPerspective )
		{
			OldColPerspective = ColPerspective;
			SendGameMessage(MSG_DPLAYUPDATE, 0, 0, 0, 0);
			if(ColPerspective == COLPERS_Forsaken)
				AddColourMessageToQue( SystemMessageColour, "SHOOTER NOW DECIDES COLLISIONS" );
			else if(ColPerspective == COLPERS_Descent)
				AddColourMessageToQue( SystemMessageColour, "TARGET NOW DECIDES COLLISIONS" );
					
		}
		// changed short packets
		if ( OldUseShortPackets != UseShortPackets )
		{
			OldUseShortPackets = UseShortPackets;
			SendGameMessage(MSG_DPLAYUPDATE, 0, 0, 0, 0);
			if(UseShortPackets)
				AddColourMessageToQue( SystemMessageColour, "SHORT PACKETS ENABLED" );
			else
				AddColourMessageToQue( SystemMessageColour, "SHORT PACKETS DISABLED" );
				
		}
	}
}


void PrimBullPosDirSend( uint16 OwnerType, uint16 OwnerID, uint16 BulletID, int8 Weapon,
						uint16 Group, VECTOR * Pos, VECTOR * Offset, VECTOR * Dir, VECTOR * Up,
						int16 PowerLevel, float PLevel )
{
	if( dcoID != 0 )
	{

		TempPrimBullPosDir.OwnerType = OwnerType;
		TempPrimBullPosDir.OwnerID = OwnerID;
		TempPrimBullPosDir.BulletID = BulletID;
		TempPrimBullPosDir.Weapon = Weapon;
		TempPrimBullPosDir.Group = Group;
		TempPrimBullPosDir.Pos = *Pos;
		TempPrimBullPosDir.Offset = *Offset;
		TempPrimBullPosDir.Dir = *Dir;
		TempPrimBullPosDir.Up = *Up;
		TempPrimBullPosDir.PowerLevel = PowerLevel;
		TempPrimBullPosDir.PLevel = PLevel;
		SendGameMessage(MSG_PRIMBULLPOSDIR, 0, 0, 0, 0);
	}
}

void SecBullPosDirSend( uint16 OwnerType, uint16 Owner, uint16 BulletID, uint16 Group,
					    VECTOR * Pos, VECTOR * Offset, VECTOR * Dir, VECTOR * UpVector,
						VECTOR * DropDir, int8 Weapon )
{
	if( dcoID != 0 )
	{
		TempSecBullPosDir.OwnerType = OwnerType;
		TempSecBullPosDir.Owner = Owner;
		TempSecBullPosDir.BulletID = BulletID;
		TempSecBullPosDir.Group = Group;
    	TempSecBullPosDir.Pos = *Pos;
		TempSecBullPosDir.Offset = *Offset;
		TempSecBullPosDir.Dir = *Dir;
		TempSecBullPosDir.UpVector = *UpVector;
		TempSecBullPosDir.DropDir = *DropDir;
		TempSecBullPosDir.Weapon = Weapon;
		SendGameMessage(MSG_SECBULLPOSDIR, 0, 0, 0, 0);
	}
}

void TitanBitsSend( uint16 OwnerType, uint16 Owner, uint16 BulletID, uint16 Group,
					    VECTOR * Pos, VECTOR * Offset, VECTOR * UpVector,
						VECTOR * DropDir, int8 Weapon, VECTOR * Directions )
{
	int16	Count;

	if( dcoID != 0 )
	{
		TempTitanBits.OwnerType = OwnerType;
		TempTitanBits.Owner		= Owner;
		TempTitanBits.BulletID		= BulletID;
		TempTitanBits.Group			= Group;
    	TempTitanBits.Pos			= *Pos;
		TempTitanBits.Offset		= *Offset;
		TempTitanBits.UpVector	= *UpVector;
		TempTitanBits.DropDir		= *DropDir;
		TempTitanBits.Weapon		= Weapon;
		for( Count = 0; Count < NUMTITANBITS; Count++ )
			TempTitanBits.Directions[ Count ] = Directions[ Count ];

		SendGameMessage(MSG_TITANBITS, 0, 0, 0, 0);
	}
}

void	IHitYou( BYTE you, float Damage, VECTOR * Recoil, VECTOR * Point, VECTOR * Dir, float Force, BYTE WeaponType, BYTE Weapon, BOOL FramelagRecoil )
{
	VECTOR	Recoil_Off;		
	if( Ships[ you ].Object.Mode != LIMBO_MODE )
	{
		if( dcoID != 0 )
		{
			if( !UseShortPackets )
			{
				if( FramelagRecoil ) TempShipHit.OneOffExternalForce = TRUE;
				else TempShipHit.OneOffExternalForce = FALSE;
				TempShipHit.Damage = Damage;
				TempShipHit.Recoil = *Recoil;
				TempShipHit.Point = *Point;
				TempShipHit.Point.x -= Ships[ you ].Object.Pos.x;
				TempShipHit.Point.y -= Ships[ you ].Object.Pos.y;
				TempShipHit.Point.z -= Ships[ you ].Object.Pos.z;
				TempShipHit.Dir = *Dir;
				TempShipHit.Force = Force;
				TempShipHit.WeaponType = WeaponType;
				TempShipHit.Weapon = Weapon;
				SendGameMessage( MSG_SHIPHIT , Ships[you].dcoID , you , 0 , 0 );
			}
			else
			{
				if( FramelagRecoil ) ShortTempShipHit.OneOffExternalForce = 1;
				else ShortTempShipHit.OneOffExternalForce = 0;
				ShortTempShipHit.Damage = Damage;

				Recoil_Off = *Recoil;
				NormaliseVector( &Recoil_Off );
				ShortTempShipHit.Recoil_Scalar = (uint16) ( 256.0F * VectorLength( Recoil ) );
				ShortTempShipHit.Recoil.x = (int16) (Recoil_Off.x * 32767.0F);
				ShortTempShipHit.Recoil.y = (int16) (Recoil_Off.y * 32767.0F);
				ShortTempShipHit.Recoil.z = (int16) (Recoil_Off.z * 32767.0F);
				ShortTempShipHit.Point.x = (int16)(Point->x - Ships[ you ].Object.Pos.x);
				ShortTempShipHit.Point.y = (int16)(Point->y - Ships[ you ].Object.Pos.y);
				ShortTempShipHit.Point.z = (int16)(Point->z - Ships[ you ].Object.Pos.z);
				ShortTempShipHit.Dir.x = (int16) (Dir->x * 32767.0F);
				ShortTempShipHit.Dir.y = (int16) (Dir->y * 32767.0F);
				ShortTempShipHit.Dir.z = (int16) (Dir->z * 32767.0F);
				ShortTempShipHit.Force = Force;
				ShortTempShipHit.WeaponType = WeaponType;
				ShortTempShipHit.Weapon = Weapon;
				SendGameMessage( MSG_SHORTSHIPHIT , Ships[you].dcoID , you , 0 , 0 );

			}
		}
	}
}


void	DropPickupSend( VECTOR * Pos, uint16 Group, VECTOR * Dir, float Speed, int16 Type, uint16 IDCount, int16 RegenSlot, BOOL Sparkle, float LifeCount, uint16 TriggerMod )
{
	if( dcoID != 0 )
	{
		if( !UseShortPackets )
		{
			TempPickup.IDCount = IDCount;
			TempPickup.Type = Type;
			TempPickup.Group = Group;
			TempPickup.Pos = *Pos;
			TempPickup.Dir = *Dir;
			TempPickup.Speed = Speed;
			TempPickup.RegenSlot = RegenSlot;
			TempPickup.Sparkle = Sparkle;
			TempPickup.LifeCount = LifeCount;
			TempPickup.TriggerMod = TriggerMod;
			SendGameMessage( MSG_DROPPICKUP, 0, 0, 0, 0 );
		}else{
			VeryShortTempPickup.IDCount = IDCount;
			VeryShortTempPickup.Type = (BYTE)Type;
			VeryShortTempPickup.Group = (BYTE) Group;
			VeryShortTempPickup.Pos.x = (int16) Pos->x;
			VeryShortTempPickup.Pos.y = (int16) Pos->y;
			VeryShortTempPickup.Pos.z = (int16) Pos->z;
			VeryShortTempPickup.Dir.x = (int16) (Dir->x * 32767.0F);
			VeryShortTempPickup.Dir.y = (int16) (Dir->y * 32767.0F);
			VeryShortTempPickup.Dir.z = (int16) (Dir->z * 32767.0F);
			VeryShortTempPickup.Speed = Speed;
			VeryShortTempPickup.RegenSlot = (BYTE) RegenSlot;
			VeryShortTempPickup.Sparkle = Sparkle;
			VeryShortTempPickup.LifeCount = LifeCount;
			VeryShortTempPickup.TriggerMod = TriggerMod;
			SendGameMessage( MSG_VERYSHORTDROPPICKUP, 0, 0, 0, 0 );
		}
	}
}

void	KillPickupSend( uint16 Owner, uint16 IDCount, int16 Style )
{
	if( dcoID != 0 )
	{
		TempKillPickup.Owner = Owner;
		TempKillPickup.IDCount = IDCount;
		TempKillPickup.Style = Style;
		SendGameMessage( MSG_KILLPICKUP, 0, 0, 0, 0 );
	}
}

void	TeamGoalsSend( uint16 * TeamGoals )
{
	if( dcoID != 0 )
	{
		int i;

		for( i = 0; i < MAX_TEAMS; i++ ) TempTeamGoals.TeamGoals[ i ] = TeamGoals[ i ];
		SendGameMessage( MSG_TEAMGOALS, 0, 0, 0, 0 );
	}
}

void	CreateShockwaveSend( uint16 OwnerShip, uint16 Owner, VECTOR * Pos, uint16 Group, float ShockwaveSize, BYTE Weapon )
{
	float dist;
	int i;
	//char mess[30];

	if( dcoID != 0 )
	{
		TempShockwave.Owner = Owner;
		TempShockwave.Pos = *Pos;
		TempShockwave.Group = Group;
		TempShockwave.Weapon = Weapon;
		TempShockwave.ShockwaveSize = ShockwaveSize;


		// for every player in the game
		for(i = 0; i < MAX_PLAYERS; i++)
		{
			// excluding myself and those who aren't in normal mode
			if((GameStatus[ i ] ==  0x0a/*STATUS_NORMAL*/) && (i != WhoIAm))
			{
				// if they are close to me
				dist = ReturnDistanceVolumeVector( &Ships[ i ].Object.Pos, Ships[ i ].Object.Group, &Ships[ Current_Camera_View ].Object.Pos, Ships[ Current_Camera_View ].Object.Group, NULL, NULL );
				if( (dist >= 0.0F) && ( dist <= 500.0F ))
				{
					// send them shockwave
					SendGameMessage(MSG_SHOCKWAVE, Ships[i].dcoID, 0, 0, 0 );
				}
			}
		}
	}
}

void	UpdateBGObjectSend( uint16 BGObject, int16 State, float Time )
{
	if( dcoID != 0 )
	{
		TempBGOUpdate.BGObject = BGObject;
		TempBGOUpdate.State = State;
		TempBGOUpdate.Time = Time;
		SendGameMessage( MSG_BGOUPDATE, 0, 0, 0, 0 );
	}
}

void	ShipDiedSend( BYTE WeaponType, BYTE Weapon )
{
	if( dcoID != 0 )
	{
		TempDied.WeaponType = WeaponType;
		TempDied.Weapon = Weapon;
		SendGameMessage( MSG_SHIPDIED, 0, 0, 0, 0 );
	}
}

void	RequestTime( void  )
{
	if( dcoID != 0 )
	{
		SendGameMessage( MSG_REQTIME, 0, 0, 0, 0 );
	}
}

void	SetTime( float Time )
{
	if( dcoID != 0 )
	{
		TempTimeSet.Time = Time;
		SendGameMessage( MSG_SETTIME, 0, 0, 0, 0 );
	}
}

// (stats.c)
extern void InitScoreSortTab(int Player); 

void SetupDplayGame()
{
	int16 i,Count;

	for( i = 0 ; i < 256 ; i++ )
	{
		RealPacketSize[i] = 0;
	}

	RealPacketSize[MSG_UPDATE								] = sizeof( UPDATEMSG							);	
	RealPacketSize[MSG_HEREIAM							] = sizeof( HEREIAMMSG						);	
	RealPacketSize[MSG_INIT									] = sizeof( INITMSG								);	 
	RealPacketSize[MSG_SHIPHIT								] = sizeof( SHIPHITMSG							);	
	RealPacketSize[MSG_PRIMBULLPOSDIR					] = sizeof( PRIMBULLPOSDIRMSG				);	
	RealPacketSize[MSG_SECBULLPOSDIR					] = sizeof( SECBULLPOSDIRMSG				);	
	RealPacketSize[MSG_SHIPDIED							] = sizeof( SHIPDIEDMSG						);	 
	RealPacketSize[MSG_DROPPICKUP						] = sizeof( DROPPICKUPMSG					);	
	RealPacketSize[MSG_KILLPICKUP							] = sizeof( KILLPICKUPMSG						);	
	RealPacketSize[MSG_STATUS								] = sizeof( STATUSMSG							);	
	RealPacketSize[MSG_DPLAYUPDATE						] = sizeof( DPLAYUPDATEMSG					);
	RealPacketSize[MSG_SHORTPICKUP						] = sizeof( SHORTPICKUPMSG					);	
	RealPacketSize[MSG_SHOCKWAVE						] = sizeof( SHOCKWAVEMSG					);	
	RealPacketSize[MSG_FUPDATE							] = sizeof( FUPDATEMSG						);	
	RealPacketSize[MSG_SHORTMINE						] = sizeof( SHORTMINEMSG					);	
	RealPacketSize[MSG_TEXTMSG							] = sizeof( TEXTMSG								);	
	RealPacketSize[MSG_SHORTREGENSLOT				] = sizeof( SHORTREGENSLOTMSG			);	
	RealPacketSize[MSG_SHORTTRIGGER					] = sizeof( SHORTTRIGGERMSG				);	 
	RealPacketSize[MSG_SHORTTRIGVAR					] = sizeof( SHORTTRIGVARMSG				);	 
	RealPacketSize[MSG_NAME									] = sizeof( NAMEMSG							);	 
	RealPacketSize[MSG_INTERPOLATE						] = sizeof( INTERPOLATEMSG					);	
	RealPacketSize[MSG_BGOUPDATE						] = sizeof( BGOUPDATEMSG					);	
	RealPacketSize[MSG_PINGREQUEST						] = sizeof( PINGMSG								);	
	RealPacketSize[MSG_PINGREPLY							] = sizeof( PINGMSG								);	
	RealPacketSize[MSG_LONGSTATUS						] = sizeof( LONGSTATUSMSG					);	
	RealPacketSize[MSG_SETTIME							] = sizeof( SETTIMEMSG						);	
	RealPacketSize[MSG_REQTIME							] = sizeof( REQTIMEMSG						);	
	RealPacketSize[MSG_ACKMSG								] = sizeof( ACKMSG								);	
	RealPacketSize[MSG_GUARANTEEDMSG					] = sizeof( GUARANTEEDMSG   				);	
	RealPacketSize[MSG_BIKENUM							] = sizeof( BIKENUMMSG		);	
	RealPacketSize[MSG_VERYSHORTUPDATE				] = sizeof( VERYSHORTUPDATEMSG			);	
	RealPacketSize[MSG_VERYSHORTFUPDATE			] = sizeof( VERYSHORTFUPDATEMSG		);	 
	RealPacketSize[MSG_VERYSHORTINTERPOLATE		] = sizeof( VERYSHORTINTERPOLATEMSG  );	 
	RealPacketSize[MSG_TEAMGOALS						] = sizeof( TEAMGOALSMSG					);	
	RealPacketSize[MSG_YOUQUIT							] = sizeof( YOUQUITMSG						);	
	RealPacketSize[MSG_SHORTSHIPHIT					] = sizeof( SHORTSHIPHITMSG				);	 
	RealPacketSize[MSG_TITANBITS							] = sizeof( TITANBITSMSG						);	
#ifdef MANUAL_SESSIONDESC_PROPAGATE
	RealPacketSize[MSG_SESSIONDESC						] = sizeof( SESSIONDESCMSG					);	
#endif
	RealPacketSize[MSG_TRACKERINFO						] = sizeof( TRACKERINFOMSG					);	
	RealPacketSize[MSG_GROUPONLY_VERYSHORTFUPDATE		 ] = sizeof( GROUPONLY_VERYSHORTFUPDATEMSG );	 
	RealPacketSize[MSG_VERYSHORTDROPPICKUP		] = sizeof( VERYSHORTDROPPICKUPMSG	);	
	

	if ( Debug )
		for( i = 0 ; i < 256 ; i++ )
		{
			PacketGot[i] = 0;
			PacketSize[i] = 0;

		}

	InitAcknowledgeMessageQue();

	memset(&Ships[0], 0, ( sizeof(GLOBALSHIP) * ( MAX_PLAYERS + 1 ) ) );
	memset(&Names, 0, sizeof(SHORTNAMETYPE) );

	DebugPrintf( "SetupDPlayGame()\n ");

	JustGenerated = TRUE;
	
	for(i=0; i<(MAX_PLAYERS+1); i++)
	{
		// everyone starts off normal....
		GameStatus[i] = STATUS_Null;
		BadConnection[ i ] = FALSE;
		CanDoDamage[i] = TRUE;
		Ships[i].BikeNum = ( i % MAXBIKETYPES );
		Ships[i].ModelNum = (uint16) -1;
		Ships[i].Object.Type = OBJECT_TYPE_SHIP;
		Ships[i].Object.Mode = NORMAL_MODE;
		Ships[i].Object.Bank = 0.0F;
		Ships[i].Object.Shield	= Start_Shield;
		Ships[i].Object.Hull	= Start_Hull;
		Ships[i].JustRecievedPacket = TRUE;
		Ships[i].Object.light = (uint16) -1;
		for( Count = 0; Count < MAXMULTIPLES; Count++ ) Ships[i].OrbModels[ Count ] = (uint16) -1;
		Ships[i].NumMultiples = 0;
		MakeQuat( 0.0F, 0.0F, 0.0F, &Ships[i].Object.Quat );
		Ships[i].Object.Pos.x = 0.0F;;
		Ships[i].Object.Pos.y = 0.0F;;
		Ships[i].Object.Pos.z = 0.0F;;
		Ships[i].Object.Group = 0;
		Ships[i].enable = 0;
		Ships[i].Object.Mat = TempMatrix;
		Ships[i].Object.FinalMat = TempMatrix;
		Ships[i].Object.FinalInvMat = TempMatrix;
		Ships[i].FirstPacketRecieved = TRUE;
		// reset external and internal force vectors to ship movement
		Ships[i].Object.ExternalForce.x = 0.0F;
		Ships[i].Object.ExternalForce.y = 0.0F;
		Ships[i].Object.ExternalForce.z = 0.0F;
		Ships[i].Object.InternalForce.x = 0.0F;
		Ships[i].Object.InternalForce.y = 0.0F;
		Ships[i].Object.InternalForce.z = 0.0F;
		Ships[i].Object.Autolevel = 0.0F;
		Ships[i].Object.BobCount = 0.0F;

		for( Count = 0; Count < 12; Count++ ) Ships[i].TempLines[ Count ] = (uint16) -1;
	
		// (stats.c)
		InitScoreSortTab((int) i);
	}
}

void InitShipStructure( int i , BOOL ResetScore )
{
	int16 Count;


//	memset(&Ships[i], 0, sizeof(GLOBALSHIP) );

	CanDoDamage[i] = TRUE;
	BadConnection[ i ] = FALSE;

	Ships[i].BikeNum = ( i % MAXBIKETYPES );
	Ships[i].ShieldHullCount = 0;
	
	Ships[i].Object.Type = OBJECT_TYPE_SHIP;
	Ships[i].Object.Mode = NORMAL_MODE;
	Ships[i].Object.Bank = 0.0F;
	Ships[i].Object.Shield	= Start_Shield;
	Ships[i].Object.Hull	= Start_Hull;
	Ships[i].JustRecievedPacket = TRUE;
	Ships[i].Object.light = (uint16) -1;
	for( Count = 0; Count < MAXMULTIPLES; Count++ ) Ships[i].OrbModels[ Count ] = (uint16) -1;
	Ships[i].NumMultiples = 0;
	MakeQuat( 0.0F, 0.0F, 0.0F, &Ships[i].Object.Quat );
	
	Ships[i].Object.Pos.x = 0.0F;;
	Ships[i].Object.Pos.y = 0.0F;;
	Ships[i].Object.Pos.z = 0.0F;;
	Ships[i].Object.Group = 0;
	
	Ships[i].enable = 0;

	Ships[i].Object.Mat = TempMatrix;
	Ships[i].Object.FinalMat = TempMatrix;
	Ships[i].Object.FinalInvMat = TempMatrix;

	Ships[i].FirstPacketRecieved = TRUE;
	// reset external and internal force vectors to ship movement
	Ships[i].Object.ExternalForce.x = 0.0F;
	Ships[i].Object.ExternalForce.y = 0.0F;
	Ships[i].Object.ExternalForce.z = 0.0F;
	Ships[i].Object.InternalForce.x = 0.0F;
	Ships[i].Object.InternalForce.y = 0.0F;
	Ships[i].Object.InternalForce.z = 0.0F;
	Ships[i].Object.Autolevel = 0.0F;
	Ships[i].Object.BobCount = 0.0F;
	
	if( ResetScore )
		ResetIndividualStats(i); // (stats.c)

	for( Count = 0; Count < 12; Count++ ) Ships[i].TempLines[ Count ] = (uint16) -1;
}




void DestroyGame( void )
{
	int	i;
	VECTOR	DirVector = { 0.0F, 0.0F, 0.0F };

	if( PlayDemo )
		return;
	
	Ships[WhoIAm].enable = 0;
	MyGameStatus = STATUS_Left;
	IsHost = FALSE;

    if ( ( glpDP != NULL ) && ( dcoID != 0 ) && ( WhoIAm < MAX_PLAYERS ) )
    {
		DebugPrintf("Destroy game pos 1\n");

		KillOwnersSecBulls( WhoIAm );

		if( GodMode ) LoseAllWeapons();

		ScatterWeapons( &DirVector, MAXPICKUPS );
		RegeneratePickups();

		ResetAllStats(); // stats.c

		ProcessGuaranteedMessages( FALSE , TRUE , TRUE );
		
		DPlayGetSessionDesc();
		DPlayDestroyPlayer(dcoID);
		DPlayRelease();
		dcoID = 0;

		Old_Session = *glpdpSD;
		Old_WhoIAm = WhoIAm;

		if( TeamGame || CTF || CaptureTheFlag || BountyHunt || MaxKills )
		{
			Old_Kills = 0;
			Old_TeamNumber = 0;
		}
		else
		{
			Old_Kills = GetScoreStats(WhoIAm);
			Old_TeamNumber = TeamNumber[WhoIAm];
		}
		for( i = 0 ; i < 256 ; i++ )
		{
			Old_Name[i] = biker_name[i];
		}
		// Stores all the info needed in the registry...
		SetLastGameInfo();

		if (glpdpSD) 
		{
			DebugPrintf("Destroy game pos 2\n");
			free(glpdpSD);
			glpdpSD = NULL;
		}


	}
	else
	{

		DebugPrintf("Destroy game pos a\n");
		if( dcoID )
		{
			DebugPrintf("Destroy game pos b\n");
			DPlayDestroyPlayer(dcoID);
			DPlayRelease();
			dcoID = 0;
		}

		if (glpdpSD) 
		{
			DebugPrintf("Destroy game pos c\n");
			free(glpdpSD);
			glpdpSD = NULL;
		}
	}

	if( Debug && DS )
		for( i = 0 ; i < 256 ; i++ )
			if( PacketGot[i] )
				DebugPrintf("num %3d quantity %12d size %12d\n", i, PacketGot[i] , PacketSize[i] );

}



void initShip( uint16 i )
{
	uint16	spos;
	int16	Count;

	Ships[i].Object.NodeNetwork = 0;
	Ships[i].Object.NearestNode = NULL;
	Ships[i].Object.Type = OBJECT_TYPE_SHIP;

    Ships[i].enable = 1;
    Ships[i].Pickups = ( ( MAXPICKUPS + ( MAXGENPICKUPCOUNT - 1 ) ) / MAXGENPICKUPCOUNT	);
    Ships[i].RegenSlots = ( ( NumRegenPoints + ( MAXGENREGENSLOTCOUNT - 1 ) ) / MAXGENREGENSLOTCOUNT );
    Ships[i].Mines = ( ( MAXSECONDARYWEAPONBULLETS + ( MAXGENMINECOUNT - 1 ) ) / MAXGENMINECOUNT );
    Ships[i].Triggers = ( ( NumOfTriggers + ( MAXGENTRIGGERCOUNT - 1 ) ) / MAXGENTRIGGERCOUNT );
    Ships[i].TrigVars = ( ( NumOfTrigVars + ( MAXGENTRIGVARCOUNT - 1 ) ) / MAXGENTRIGVARCOUNT );

    Ships[i].NumMultiples = 0;
	for( Count = 0; Count < MAXMULTIPLES; Count++ ) Ships[i].OrbModels[ Count ] = (uint16) -1;

	if ( Mloadheader.state == TRUE)
	{
		spos = (i+RandomStartPosModify) % num_start_positions;
		InitShipStartPos( i, spos );
	}
	else
	{
		InitShipStartPos( i, 0 );
	}

	KillAllPickups();
}
void smallinitShip( uint16 i )
{
	uint16	spos;
	int16	Count;

	Ships[i].Object.NodeNetwork = 0;
	Ships[i].Object.NearestNode = NULL;
	Ships[i].Object.Type = OBJECT_TYPE_SHIP;

    Ships[i].enable = 1;
    Ships[i].Pickups = 0;
    Ships[i].RegenSlots = 0;
    Ships[i].Mines = 0;
    Ships[i].Triggers = 0;
    Ships[i].TrigVars = 0;
    Ships[i].NumMultiples = 0;
	for( Count = 0; Count < MAXMULTIPLES; Count++ ) Ships[i].OrbModels[ Count ] = (uint16) -1;

	NumMissedPickups = 0;
	NumMissedInitPickups = 0;
	NumMissedKillPickups = 0;

	NumMissedMines = 0;
	NumMissedInitMines = 0;
	NumMissedKillMines = 0;

	if ( Mloadheader.state == TRUE)
	{
		spos = (i+RandomStartPosModify) % num_start_positions;
		InitShipStartPos( i, spos );
	}
	else
	{
		InitShipStartPos( i, 0 );
	}
}


void ReceiveGameMessages( void )
{
	DPID			dcoReceiveID;
    DWORD		nBytes;
	BYTE			* BufferPnt;
	DWORD		* dwordpnt;
	DWORD		offset = 0;
	LONGLONG	* longpnt;
	HRESULT		status;
	int i;

	for( i = 0 ; i < MAX_PLAYERS ; i++ )
		OldGameStatus[i] = GameStatus[i];

	if( MyGameStatus == STATUS_Normal )
	{
		BytesPerSecTimer -= framelag;
		if( BytesPerSecTimer < 0.0F )
		{
			BytesPerSecTimer = 71.0F;
			CurrentBytesPerSecRec = BytesPerSecRec;
			CurrentBytesPerSecSent = BytesPerSecSent;

			if( CurrentBytesPerSecRec > MaxCurrentBytesPerSecRec )
				MaxCurrentBytesPerSecRec = CurrentBytesPerSecRec;
			if( CurrentBytesPerSecSent > MaxCurrentBytesPerSecSent )
				MaxCurrentBytesPerSecSent = CurrentBytesPerSecSent;
			BytesPerSecRec = 0;
			BytesPerSecSent = 0;
		}

		// Stuff to handle Kills And deaths....
		if( NextworkOldBikeNum != Ships[WhoIAm].BikeNum )
		{
			NextworkOldBikeNum = Ships[WhoIAm].BikeNum;
			SendGameMessage( MSG_BIKENUM, 0, 0, 0, 0 );
		}
	}
	else
	{
		BytesPerSecRec					= 0;
		BytesPerSecSent					= 0;
		CurrentBytesPerSecRec			= 0;
		CurrentBytesPerSecSent		= 0;
		MaxCurrentBytesPerSecRec	= 0;
		MaxCurrentBytesPerSecSent	= 0;
		RecPacketSize = MaxRecPacketSize = 0;
		BytesPerSecTimer = 71.0F;
	}

	ProcessGuaranteedMessages( FALSE , FALSE , FALSE );
	ProcessAcknowledgeMessageQue();
	BuildReliabilityTab();

	if( DplayRecieveThread )
	{
		if( Buffer2Count && !Buffer2InUse )
		{
			Buffer2InUse = TRUE;

			while( Buffer2Count )
			{
				longpnt = (LONGLONG*) (Buffer2Pnt+offset);
				TempTime = *longpnt++;
				dwordpnt = (DWORD*) longpnt;
				nBytes = *dwordpnt++;
				from_dcoID = *dwordpnt++;
				BufferPnt = (BYTE*) dwordpnt;

				if( RecordDemo && ( MyGameStatus == STATUS_Normal ) && ( from_dcoID != DPID_SYSMSG ) )
				{
					if( *BufferPnt != MSG_GUARANTEEDMSG && *BufferPnt != MSG_ACKMSG )
						Demo_fwrite( (Buffer2Pnt+offset), nBytes + (sizeof(DWORD)*2) + sizeof(LONGLONG), 1, DemoFp );
				}
				
				offset += sizeof(DWORD) + (sizeof(DPID)*1) + nBytes + sizeof(LONGLONG);
				if ( from_dcoID == DPID_SYSMSG )    EvalSysMessage( nBytes , BufferPnt );
				else EvaluateMessage( nBytes , BufferPnt );
				Buffer2Count-=1;

			}
			Buffer2Offset = 0;
			Buffer2InUse = FALSE;
		}
		offset = 0;
		if( Buffer1Count && !Buffer1InUse)
		{
			Buffer1InUse = TRUE;

			while( Buffer1Count )
			{
				longpnt = (LONGLONG*) (Buffer1Pnt+offset);
				TempTime = *longpnt++;
				dwordpnt = (DWORD*) longpnt;
				nBytes = *dwordpnt++;
				from_dcoID = *dwordpnt++;
				BufferPnt = (BYTE*) dwordpnt;

				if( RecordDemo && ( MyGameStatus == STATUS_Normal ) && ( from_dcoID != DPID_SYSMSG ) )
				{
					if( *BufferPnt != MSG_GUARANTEEDMSG && *BufferPnt != MSG_ACKMSG )
						Demo_fwrite( (Buffer1Pnt+offset), nBytes + (sizeof(DWORD)*2) + sizeof(LONGLONG), 1, DemoFp );
				}
				
				offset += sizeof(DWORD) + (sizeof(DPID)*1) + nBytes + sizeof(LONGLONG);

				if ( from_dcoID == DPID_SYSMSG )    EvalSysMessage( nBytes , BufferPnt );
				else EvaluateMessage( nBytes , BufferPnt );

				Buffer1Count-=1;
			}
			Buffer1Offset = 0;
			Buffer1InUse = FALSE;
		}
	}
	else
	{

		// The old non Thread method......
		if ( glpDP )
		{
			// read all messages in queue
			while(1)
			{
				nBytes = MAXBIGPACKETBUFFERSIZE;
				status = glpDP->lpVtbl->Receive( glpDP,
							&from_dcoID,
							&dcoReceiveID,
							DPRECEIVE_ALL,
							&ReceiveCommBuff[0],

							&nBytes);
				if( status == DP_OK )
				{
					QueryPerformanceCounter((LARGE_INTEGER *) &TempTime);
					if( RecordDemo && ( MyGameStatus == STATUS_Normal ) && ( from_dcoID != DPID_SYSMSG ) )
					{
						TempTime -= GameStartedTime;

						if( ReceiveCommBuff[0] != MSG_GUARANTEEDMSG && ReceiveCommBuff[0] != MSG_ACKMSG )
						{
							Demo_fwrite( &TempTime, sizeof(LONGLONG), 1, DemoFp );
							Demo_fwrite( &nBytes, sizeof(nBytes), 1, DemoFp );
							Demo_fwrite( &from_dcoID, sizeof(from_dcoID), 1, DemoFp );
							Demo_fwrite( &ReceiveCommBuff[0], nBytes , 1, DemoFp );
						}
					}
					RecPacketSize = nBytes;
					if ( RecPacketSize > MaxRecPacketSize )
						MaxRecPacketSize = RecPacketSize;

					BytesPerSecRec += nBytes;

					if ( from_dcoID == DPID_SYSMSG )    EvalSysMessage( nBytes , &ReceiveCommBuff[0] );
					else EvaluateMessage( nBytes , &ReceiveCommBuff[0]);
				}
				else
				{
					// Error condition of some kind - we just stop
					// checking for now
					return;
				}
			}
		}

	}

	for( i = 0 ; i < MAX_PLAYERS ; i++ )
	{
		if( OldGameStatus[i] == STATUS_Joining && GameStatus[i] != STATUS_Joining )
		{
			// $$$
			NextworkOldBikeNum = -1;
		}
	}
}

/*
 * EvalSysMessage
 *
 * Evaluates system messages and performs appropriate actions
 */
void EvalSysMessage( DWORD len , BYTE * MsgPnt)
{
	int i;
	LPDPMSG_GENERIC lpMsg = (LPDPMSG_GENERIC) MsgPnt;
	LPDPMSG_DESTROYPLAYERORGROUP lpDestroyMsg;
	LPDPMSG_CREATEPLAYERORGROUP lpAddMsg;
//	LPDPMSG_SETSESSIONDESC lpSetSessionMsg;
    
	if (!lpMsg)
		return;
    switch( lpMsg->dwType)
    {
	case DPSYS_SENDCOMPLETE:
		{
			char * result = "";
			LPDPMSG_SENDCOMPLETE msg = (LPDPMSG_SENDCOMPLETE) lpMsg;
			switch( msg->hr )
			{
			case DP_OK:
				result = "SENT";
				break;
			case DPERR_ABORTED:
				result = "ABORTED";
				break;
			case DPERR_CANCELLED:
				result = "CANCELLED";
				break;
			case DPERR_GENERIC:
				result = "DPERR_GENERIC";
				break;
			case DPERR_TIMEOUT:
				result = "TIMEOUT";
				break;
			}
			DebugPrintf("dplay  %30s   %13s   duration= %3lu ms   timeout= %3lu\n", (char*) msg->lpvContext, result, msg->dwSendTime, msg->dwTimeout);
		}
		break;
	case DPSYS_CREATEPLAYERORGROUP:
		DebugPrintf("DPSYS_CREATEPLAYERORGROUP recieved\n");
		if( MyGameStatus == STATUS_Normal && !TeamGame)
		{
			lpAddMsg = (LPDPMSG_CREATEPLAYERORGROUP) lpMsg;
			sprintf( (char*) &tempstr[0] ,"%s %s", lpAddMsg->dpnName.lpszShortNameA , IS_JOINING_THE_GAME );
   			AddColourMessageToQue(SystemMessageColour, (char*)&tempstr[0] );
		}
		for( i = 0 ; i < MAX_PLAYERS ; i++ )
		{
			if( ( i != WhoIAm ) && ( GameStatus[i] != MyGameStatus ) )
			{
				Names[i][0] = 0;
			}
		}

//			if( IsHost)
//			{
//			        SendGameMessage(MSG_INIT, lpAddMsg->dpId, 0, 0, 0);
//			}
		break;
	case DPSYS_SESSIONLOST:
		DebugPrintf("DPSYS_SESSIONLOST recieved\n");
		// The Whole Game has been Lost....Oops...
		AddColourMessageToQue( SystemMessageColour, THE_SESSION_HAS_BEEN_LOST_PLEASE_QUIT );
		AddColourMessageToQue( SystemMessageColour, THE_SESSION_HAS_BEEN_LOST_PLEASE_QUIT );
		AddColourMessageToQue( SystemMessageColour, THE_SESSION_HAS_BEEN_LOST_PLEASE_QUIT );
		break;

    case DPSYS_HOST:
		DebugPrintf("DPSYS_HOST recieved\n");

		switch ( MyGameStatus )
		{
		case STATUS_StartingMultiplayer:
			if ( TeamGame )
				PrintErrorMessage ( YOU_HAVE_BECOME_THE_HOST , 0, NULL, ERROR_DONTUSE_MENUFUNCS );
			else
				PrintErrorMessage ( YOU_HAVE_BECOME_THE_HOST , 1, &MENU_NEW_HostWaitingToStart, ERROR_DONTUSE_MENUFUNCS );
			if ( DPlayGetSessionDesc() != DP_OK)
			{
				Msg("Mydplay.c: EvalSysMessage() unable to get new session description\n");
				exit(1);
			}
			DPlayTest = TRUE;
			break;
		default:
			AddColourMessageToQue( SystemMessageColour, YOU_HAVE_BECOME_THE_HOST );
		}

		IsHost = TRUE;					// I have Become the host

		if( !RecordDemoToRam )
			RecordDemo = FALSE;				// But I cant record a demo cos none of the files are open...
		PacketsSlider.value = (int) (60.0F / DPlayUpdateInterval);
		for( i = 0 ; i < MAX_PLAYERS ; i++ )
		{
			if( ( i != WhoIAm ) && ( Ships[i].Object.Flags & SHIP_IsHost ) )
			{
				Ships[i].enable = 0;
				GameStatus[i] = STATUS_Left;
				FreeAllPlayersAcknowledgeMessageQue( (BYTE)i );
			}
		}

		break;
	case DPSYS_DESTROYPLAYERORGROUP:
		DebugPrintf("DPSYS_DESTROYPLAYERORGROUP recieved\n");
		lpDestroyMsg = ( LPDPMSG_DESTROYPLAYERORGROUP ) lpMsg;
		if( lpDestroyMsg->dwPlayerType == DPPLAYERTYPE_PLAYER )
		{			
			for( i = 0 ; i < MAX_PLAYERS ; i++ )
			{
				if( ( i != WhoIAm ) && (lpDestroyMsg->dpId == Ships[i].dcoID) )
				{
					
					if( MyGameStatus == STATUS_Normal )
					{
						sprintf( (char*) &tempstr[0] ,"%s %s", &Names[i][0] , HAS_LEFT_THE_GAME );
				   		AddColourMessageToQue(SystemMessageColour, (char*)&tempstr[0] );
					}

					if( Ships[i].Object.light != (uint16) -1  )
					{
//						KillUsedXLight(Ships[i].Object.light);
						Ships[i].Object.light = (uint16) -1;
					}
//						KillOwnersSecBulls( (uint16) i );
					Ships[i].enable = 0;
					FreeAllPlayersAcknowledgeMessageQue( (BYTE)i );

					if ( GameStatus[ i ] == STATUS_StartingMultiplayer )
					{
						GameStatus[ i ] = STATUS_Null;	// ensure slot is freed up if player has quit from titles
					}else
					{
						if( GameStatus[i] != STATUS_Left )
						{
							GameStatus[i] = STATUS_LeftCrashed;
						}
					}

					InitShipStructure(i , FALSE );
					break;
				}
			}
		}
		break;
#if 1
	case DPSYS_SETSESSIONDESC:
		DebugPrintf("DPSYS_SETSESSIONDESC recieved\n");
		break;
#endif
	case DPSYS_ADDGROUPTOGROUP:
		DebugPrintf("DPSYS_ADDGROUPTOGROUP recieved\n");
		break;
	case DPSYS_ADDPLAYERTOGROUP:
		DebugPrintf("DPSYS_ADDPLAYERTOGROUP recieved\n");
		break;
	case DPSYS_CHAT:
		DebugPrintf("DPSYS_CHAT recieved\n");
		break;
	case DPSYS_DELETEGROUPFROMGROUP:
		DebugPrintf("DPSYS_DELETEGROUPFROMGROUP recieved\n");
		break;
	case DPSYS_DELETEPLAYERFROMGROUP:
		DebugPrintf("DPSYS_DELETEPLAYERFROMGROUP recieved\n");
		break;
	case DPSYS_SECUREMESSAGE:
		DebugPrintf("DPSYS_SECUREMESSAGE recieved\n");
		break;
	case DPSYS_SETPLAYERORGROUPDATA:
		DebugPrintf("DPSYS_SETPLAYERORGROUPDATA recieved\n");
		break;
	case DPSYS_SETPLAYERORGROUPNAME:
		DebugPrintf("DPSYS_SETPLAYERORGROUPNAME recieved\n");
		break;
	case DPSYS_STARTSESSION:
		DebugPrintf("DPSYS_STARTSESSION recieved\n");
		break;
	default:
		DebugPrintf("unknown DPlay sys message recieved ( %x )\n", *MsgPnt);
		break;
	}
}

void EvaluateMessage( DWORD len , BYTE * MsgPnt )
{
    LPUPDATEMSG							lpUpdate;
    LPVERYSHORTUPDATEMSG			lpVeryShortUpdate;
    LPFUPDATEMSG						lpFUpdate;
    LPVERYSHORTFUPDATEMSG		lpVeryShortFUpdate;
    LPGROUPONLY_VERYSHORTFUPDATEMSG		lpGroupOnly_VeryShortFUpdate;
    LPHEREIAMMSG						lpHereIAm;
    LPINITMSG								lpInit;
    LPSHIPHITMSG							lpShipHit;
    LPSHORTSHIPHITMSG				lpShortShipHit;
    LPSHIPDIEDMSG						lpShipDied;
    LPPRIMBULLPOSDIRMSG				lpPrimBullPosDir;
    LPSECBULLPOSDIRMSG				lpSecBullPosDir;
    LPTITANBITSMSG						lpTitanBits;
    LPDROPPICKUPMSG					lpDropPickup;
    LPVERYSHORTDROPPICKUPMSG   lpVeryShortDropPickup;
    LPKILLPICKUPMSG						lpKillPickup;
    LPTEAMGOALSMSG					lpTeamGoals;
    LPSHOCKWAVEMSG					lpShockwave;
    LPBGOUPDATEMSG					lpBGOUpdate;
    LPSTATUSMSG							lpStatus;
    LPLONGSTATUSMSG					lpLongStatus;
    LPSHORTPICKUPMSG					lpShortPickup;
    LPSHORTREGENSLOTMSG			lpShortRegenSlot;
    LPSHORTTRIGGERMSG				lpShortTrigger;
    LPSHORTTRIGVARMSG				lpShortTrigVar;
    LPSHORTMINEMSG					lpShortMine;
    LPTEXTMSG								lpTextMsg;
	LPNAMEMSG							lpName;
	LPINTERPOLATEMSG					lpInterpolate;
	LPVERYSHORTINTERPOLATEMSG	lpVeryShortInterpolate;
	LPPINGMSG								lpPingMsg;
	LPACKMSG								lpAckMsg;
	LPGUARANTEEDMSG					lpGuaranteedMsg;
	LPBIKENUMMSG						lpBikeNumMsg;
	LPYOUQUITMSG						lpYouQuitMsg;
	LPDPLAYUPDATEMSG					lpDplayUpdateMsg;
    char				dBuf[256];
	int					i;
	BYTE				OldMode;
	BYTE				OldStatus;
	int16				Count;
    char				methodstr[256];
	char				teamstr[256];
	int					size;				// special for demo recording
	BYTE				msg;				// special for demo recording
	LONGLONG			TempLongLong;
    LPSETTIMEMSG	lpSetTime;
    LPREQTIMEMSG	lpReqTime;
	uint32	ID;
	BYTE	AckWhoIAm;
	VECTOR	Point;
	VECTOR	Recoil;
	VECTOR	Dir;
	float	Force;
	uint16	Pickup;

#ifdef MANUAL_SESSIONDESC_PROPAGATE
	LPSESSIONDESCMSG	lpSessionDescMsg;
#endif

	// set flag sfx volume
	FlagVolume = FlagSfxSlider.value / ( FlagSfxSlider.max / GLOBAL_MAX_SFX );


	/*
	if ( *MsgPnt == 0xe0 )
	{
		Msg("tracker msg recieved ( gamestatus = %x)\n", MyGameStatus);
		Msg("test");
	}
	*/
	
	if( *(MsgPnt+1) < MAX_PLAYERS )
	{
		LastPacketTime[*(MsgPnt+1)] = TempTime;

		if ( Debug )
		{
			PacketGot[*MsgPnt]++;
			PacketSize[*MsgPnt]+=len;
		}

	}

	if( *MsgPnt == MSG_GUARANTEEDMSG )
	{
		lpGuaranteedMsg = (LPGUARANTEEDMSG)MsgPnt;
		if( lpGuaranteedMsg->Ack & ( 1 << WhoIAm ) )
		{
			ID = lpGuaranteedMsg->ID;
			AckWhoIAm = lpGuaranteedMsg->WhoIAm;

			if( AddAcknowledgeMessageQue( AckWhoIAm , ID ) )
			{
				if( RecordDemo && ( MyGameStatus == STATUS_Normal ) )
				{
					Demo_fwrite( &TempTime, sizeof(LONGLONG), 1, DemoFp );
					len = RealPacketSize[lpGuaranteedMsg->StartOfMessage];
					Demo_fwrite( &len, sizeof(len), 1, DemoFp );
					Demo_fwrite( &from_dcoID, sizeof(from_dcoID), 1, DemoFp );
					Demo_fwrite( &lpGuaranteedMsg->StartOfMessage, len , 1, DemoFp );
				}

				ItsAGuranteed = TRUE;
				EvaluateMessage( len , &lpGuaranteedMsg->StartOfMessage );
				ItsAGuranteed = FALSE;

			}
			SendGameMessage( MSG_ACKMSG, ID, AckWhoIAm, 0, 0 );
		}
		return;
	}
		
	if( MyGameStatus == STATUS_GetPlayerNum )
	{
 		if ( ( *MsgPnt != MSG_INIT ) && ( *MsgPnt != MSG_TRACKERINFO ) )
			return;
	}

	if  ( ( ( ( MyGameStatus >= STATUS_InitView_0 ) && ( MyGameStatus <= STATUS_InitView_9 ) ) ||
		( MyGameStatus == STATUS_Joining ) ||
		( MyGameStatus == STATUS_PreStartingMultiplayerSynch ) ||
		( MyGameStatus == STATUS_WaitingToStartMultiPlayerClient ) ||
		( MyGameStatus == STATUS_WaitingToStartTeamGame ) ) )
	{
		switch (*MsgPnt)
		{
		    case MSG_SECBULLPOSDIR:
				lpSecBullPosDir = (LPSECBULLPOSDIRMSG)MsgPnt;

				if( SecondaryWeaponAttribs[ lpSecBullPosDir->SecBullPosDir.Weapon ].SecType == SEC_MINE )
				{
					if( NumMissedMines != MAXMISSEDMINES )
					{
						DebugPrintf( "MissedInitMine() %d\n", NumMissedMines );

						MissedInitMines[ NumMissedInitMines ].OwnerType	= lpSecBullPosDir->SecBullPosDir.OwnerType;
						MissedInitMines[ NumMissedInitMines ].Owner		= lpSecBullPosDir->SecBullPosDir.Owner;
						MissedInitMines[ NumMissedInitMines ].BulletID		= lpSecBullPosDir->SecBullPosDir.BulletID;
						MissedInitMines[ NumMissedInitMines ].Group			= lpSecBullPosDir->SecBullPosDir.Group;
						MissedInitMines[ NumMissedInitMines ].Pos			= lpSecBullPosDir->SecBullPosDir.Pos;
						MissedInitMines[ NumMissedInitMines ].Offset		= lpSecBullPosDir->SecBullPosDir.Offset;
						MissedInitMines[ NumMissedInitMines ].Dir				= lpSecBullPosDir->SecBullPosDir.Dir;
						MissedInitMines[ NumMissedInitMines ].UpVector	= lpSecBullPosDir->SecBullPosDir.UpVector;
						MissedInitMines[ NumMissedInitMines ].DropDir		= lpSecBullPosDir->SecBullPosDir.DropDir;
						MissedInitMines[ NumMissedInitMines ].Weapon		= lpSecBullPosDir->SecBullPosDir.Weapon;
						MissedMines[ NumMissedMines ].Type = MM_TYPE_INIT;
						MissedMines[ NumMissedMines ].Struct = &MissedInitMines[ NumMissedInitMines ];
						NumMissedInitMines++;
						NumMissedMines++;
					}
					else
					{
			   			DebugPrintf( "Overflowed mine que ( INIT )\n" );
					}
				}
				return;
				break;

			case MSG_DROPPICKUP:
				lpDropPickup = (LPDROPPICKUPMSG)MsgPnt;

				if( NumMissedPickups != MAXMISSEDPICKUPS )
		 		{
		 			MissedInitPickups[ NumMissedInitPickups ].Pos				= lpDropPickup->PickupInfo.Pos;
		 			MissedInitPickups[ NumMissedInitPickups ].Group			= lpDropPickup->PickupInfo.Group;
		 			MissedInitPickups[ NumMissedInitPickups ].Dir				= lpDropPickup->PickupInfo.Dir;
		 			MissedInitPickups[ NumMissedInitPickups ].Speed			= lpDropPickup->PickupInfo.Speed;
		 			MissedInitPickups[ NumMissedInitPickups ].Type				= lpDropPickup->PickupInfo.Type;
		 			MissedInitPickups[ NumMissedInitPickups ].Owner			= lpDropPickup->WhoIAm;
		 			MissedInitPickups[ NumMissedInitPickups ].ID					= lpDropPickup->PickupInfo.IDCount;
		 			MissedInitPickups[ NumMissedInitPickups ].RegenSlot		= lpDropPickup->PickupInfo.RegenSlot;
		 			MissedInitPickups[ NumMissedInitPickups ].Sparkle			= lpDropPickup->PickupInfo.Sparkle;
		 			MissedInitPickups[ NumMissedInitPickups ].LifeCount		= lpDropPickup->PickupInfo.LifeCount;
		 			MissedInitPickups[ NumMissedInitPickups ].TriggerMod		= lpDropPickup->PickupInfo.TriggerMod;
		 			MissedPickups[ NumMissedPickups ].Type		= MPU_TYPE_INIT;
		 			MissedPickups[ NumMissedPickups ].Struct	= &MissedInitPickups[ NumMissedInitPickups ];
#if DEBUG_PICKUPS
	   	 	   		DebugPrintf( "Added Init '%s' to Que\n", Messages[ lpDropPickup->PickupInfo.Type ] );
#endif
		 	   		NumMissedInitPickups++;
		 	   		NumMissedPickups++;
		 	   	}
		 	   	else
		 	   	{
#if DEBUG_PICKUPS
   	 				DebugPrintf( "Overflowed pickup que ( INIT )\n" );
#endif
	 			}
				return;
				break;

			case MSG_VERYSHORTDROPPICKUP:
				lpVeryShortDropPickup = (LPVERYSHORTDROPPICKUPMSG)MsgPnt;

				if( NumMissedPickups != MAXMISSEDPICKUPS )
		 		{
		 			MissedInitPickups[ NumMissedInitPickups ].Pos.x			= (float)lpVeryShortDropPickup->PickupInfo.Pos.x;
		 			MissedInitPickups[ NumMissedInitPickups ].Pos.y			= (float)lpVeryShortDropPickup->PickupInfo.Pos.y;
		 			MissedInitPickups[ NumMissedInitPickups ].Pos.z			= (float)lpVeryShortDropPickup->PickupInfo.Pos.z;
		 			MissedInitPickups[ NumMissedInitPickups ].Dir.x				= lpVeryShortDropPickup->PickupInfo.Dir.x * ONEOVER32767;
		 			MissedInitPickups[ NumMissedInitPickups ].Dir.y				= lpVeryShortDropPickup->PickupInfo.Dir.y * ONEOVER32767;
		 			MissedInitPickups[ NumMissedInitPickups ].Dir.z				= lpVeryShortDropPickup->PickupInfo.Dir.z * ONEOVER32767;
		 			MissedInitPickups[ NumMissedInitPickups ].Group			= lpVeryShortDropPickup->PickupInfo.Group;
		 			MissedInitPickups[ NumMissedInitPickups ].Speed			= lpVeryShortDropPickup->PickupInfo.Speed;
		 			MissedInitPickups[ NumMissedInitPickups ].Type				= lpVeryShortDropPickup->PickupInfo.Type;
		 			MissedInitPickups[ NumMissedInitPickups ].Owner			= lpVeryShortDropPickup->WhoIAm;
		 			MissedInitPickups[ NumMissedInitPickups ].ID					= lpVeryShortDropPickup->PickupInfo.IDCount;
		 			MissedInitPickups[ NumMissedInitPickups ].RegenSlot		= lpVeryShortDropPickup->PickupInfo.RegenSlot;
		 			MissedInitPickups[ NumMissedInitPickups ].Sparkle			= lpVeryShortDropPickup->PickupInfo.Sparkle;
		 			MissedInitPickups[ NumMissedInitPickups ].LifeCount		= lpVeryShortDropPickup->PickupInfo.LifeCount;
		 			MissedInitPickups[ NumMissedInitPickups ].TriggerMod		= lpVeryShortDropPickup->PickupInfo.TriggerMod;
		 			MissedPickups[ NumMissedPickups ].Type		= MPU_TYPE_INIT;
		 			MissedPickups[ NumMissedPickups ].Struct	= &MissedInitPickups[ NumMissedInitPickups ];
#if DEBUG_PICKUPS
	   	 	   		DebugPrintf( "Added Init '%s' to Que\n", Messages[ lpVeryShortDropPickup->PickupInfo.Type ] );
#endif
		 	   		NumMissedInitPickups++;
		 	   		NumMissedPickups++;
		 	   	}
		 	   	else
		 	   	{
#if DEBUG_PICKUPS
   	 				DebugPrintf( "Overflowed pickup que ( INIT )\n" );
#endif
	 			}
				return;
				break;

			case MSG_KILLPICKUP:
   				lpKillPickup = (LPKILLPICKUPMSG)MsgPnt;
   				if( NumMissedPickups != MAXMISSEDPICKUPS )
   				{
   					MissedKillPickups[ NumMissedKillPickups ].Owner = lpKillPickup->KillPickupInfo.Owner;
   					MissedKillPickups[ NumMissedKillPickups ].ID		= lpKillPickup->KillPickupInfo.IDCount;
   					MissedKillPickups[ NumMissedKillPickups ].Style	= lpKillPickup->KillPickupInfo.Style;
   					MissedPickups[ NumMissedPickups ].Type			= MPU_TYPE_KILL;
   					MissedPickups[ NumMissedPickups ].Struct		= &MissedKillPickups[ NumMissedKillPickups ];
#if DEBUG_PICKUPS
					DebugPrintf( "Added Kill pickup of Owner '%s', ID %d, to que\n",
								&Names[ lpKillPickup->KillPickupInfo.Owner ][ 0 ],
								lpKillPickup->KillPickupInfo.IDCount );
#endif
   					NumMissedKillPickups++;
   					NumMissedPickups++;
   				}
   				else
   				{
#if DEBUG_PICKUPS
   					DebugPrintf( "Overflowed pickup que ( KILL )\n" );
#endif
				}
				return;
				break;


			case MSG_SHORTREGENSLOT:
			case MSG_SHORTTRIGGER:
			case MSG_SHORTTRIGVAR:
			case MSG_SHORTMINE:
			case MSG_SHORTPICKUP:
			case MSG_YOUQUIT:
			case MSG_NAME:
			case MSG_ACKMSG:
			case MSG_BIKENUM:
			case MSG_STATUS:
			case MSG_LONGSTATUS:
			case MSG_TRACKERINFO:
			case MSG_TEAMGOALS:
				break;

			default:
				return;
		}
	}
	else
	{
		// Some messages should be ignored when im not in normal multiplayer mode
		if(
			MyGameStatus != STATUS_Normal &&
			MyGameStatus != STATUS_PlayingDemo
		)
		{
			switch (*MsgPnt)
			{
		    case MSG_SECBULLPOSDIR:
			case MSG_DROPPICKUP:
			case MSG_VERYSHORTDROPPICKUP:
			case MSG_KILLPICKUP:
			case MSG_VERYSHORTFUPDATE:
			case MSG_GROUPONLY_VERYSHORTFUPDATE:
			case MSG_FUPDATE:
			case MSG_TEAMGOALS:
				return;

			default:
				break;
			}
		}
	}


	if (MyGameStatus == STATUS_StartingMultiplayer)
	{
		switch (*MsgPnt)
		{
		case MSG_VERYSHORTUPDATE:
		case MSG_UPDATE:
		case MSG_INIT:
		case MSG_STATUS:
		case MSG_LONGSTATUS:
		case MSG_HEREIAM:
		case MSG_TEXTMSG:
		case MSG_YOUQUIT:
		case MSG_NAME:
 		case MSG_BIKENUM:
#ifdef MANUAL_SESSIONDESC_PROPAGATE
		case MSG_SESSIONDESC:
#endif
		case MSG_ACKMSG:
		case MSG_TRACKERINFO:
			break;
		default:
			return;
		}
	}
    switch( *MsgPnt )
    {

	case MSG_YOUQUIT:

		// I have been asked by the host to quit......
		lpYouQuitMsg = (LPYOUQUITMSG)MsgPnt;
		if( lpYouQuitMsg->You == WhoIAm )
			IMustQuit = TRUE;
		SetTitleNotify( KICKED_HOST );
		return;


    case MSG_VERYSHORTFUPDATE:

		lpVeryShortFUpdate = (LPVERYSHORTFUPDATEMSG)MsgPnt;
		if( lpVeryShortFUpdate->WhoIAm != WhoIAm )
		{
			OldMode = Ships[lpVeryShortFUpdate->WhoIAm].Object.Mode;
			Ships[lpVeryShortFUpdate->WhoIAm].PacketDelay = PacketDelay;
			CheckCharging( lpVeryShortFUpdate->WhoIAm, Ships[ lpVeryShortFUpdate->WhoIAm ].Object.Flags, lpVeryShortFUpdate->ShortGlobalShip.Flags );
			if( !( Ships[lpVeryShortFUpdate->WhoIAm].Object.Flags & SHIP_Multiple ) && ( lpVeryShortFUpdate->ShortGlobalShip.Flags & SHIP_Multiple ) )
			{
				for( Count = 0; Count < MAXMULTIPLES; Count++ ) Ships[ lpVeryShortFUpdate->WhoIAm ].OrbModels[ Count ] = (uint16) -1;
				Ships[lpVeryShortFUpdate->WhoIAm].NumMultiples = (BYTE)((lpVeryShortFUpdate->ShortGlobalShip.Flags >> SHIP_NumMultiples_Bit1 ) & 15);
				CreateOrbitPulsar( lpVeryShortFUpdate->WhoIAm );
			}
			else
			{
				if( ( Ships[ lpVeryShortFUpdate->WhoIAm ].Object.Flags & SHIP_Multiple ) &&
					( ((BYTE)((lpVeryShortFUpdate->ShortGlobalShip.Flags >> SHIP_NumMultiples_Bit1 ) & 15)) != Ships[lpVeryShortFUpdate->WhoIAm].NumMultiples ) )
				{
					Ships[lpVeryShortFUpdate->WhoIAm].NumMultiples = (BYTE)((lpVeryShortFUpdate->ShortGlobalShip.Flags >> SHIP_NumMultiples_Bit1 ) & 15);
					CreateOrbitPulsar( lpVeryShortFUpdate->WhoIAm );
				}
			}
			UnPackShipFlags( lpVeryShortFUpdate->WhoIAm , lpVeryShortFUpdate->ShortGlobalShip.Flags );
			Ships[lpVeryShortFUpdate->WhoIAm].Object.Pos.x		= (float) lpVeryShortFUpdate->ShortGlobalShip.Pos.x;
			Ships[lpVeryShortFUpdate->WhoIAm].Object.Pos.y		= (float) lpVeryShortFUpdate->ShortGlobalShip.Pos.y;
			Ships[lpVeryShortFUpdate->WhoIAm].Object.Pos.z		= (float) lpVeryShortFUpdate->ShortGlobalShip.Pos.z;
			Ships[lpVeryShortFUpdate->WhoIAm].Move_Off.x		= (lpVeryShortFUpdate->ShortGlobalShip.Move_Off.x * ONEOVER32767) * (lpVeryShortFUpdate->ShortGlobalShip.Move_Off_Scalar * ONEOVER256 );
			Ships[lpVeryShortFUpdate->WhoIAm].Move_Off.y		= (lpVeryShortFUpdate->ShortGlobalShip.Move_Off.y * ONEOVER32767) * (lpVeryShortFUpdate->ShortGlobalShip.Move_Off_Scalar * ONEOVER256 );
			Ships[lpVeryShortFUpdate->WhoIAm].Move_Off.z		= (lpVeryShortFUpdate->ShortGlobalShip.Move_Off.z * ONEOVER32767) * (lpVeryShortFUpdate->ShortGlobalShip.Move_Off_Scalar * ONEOVER256 ); 
			Ships[lpVeryShortFUpdate->WhoIAm].Object.Group	= (int16) lpVeryShortFUpdate->ShortGlobalShip.GroupImIn;
			Ships[lpVeryShortFUpdate->WhoIAm].Object.Quat.w	= (float)(lpVeryShortFUpdate->ShortGlobalShip.Quat.w * ONEOVER32767 );
			Ships[lpVeryShortFUpdate->WhoIAm].Object.Quat.x	= (float)(lpVeryShortFUpdate->ShortGlobalShip.Quat.x * ONEOVER32767 );
			Ships[lpVeryShortFUpdate->WhoIAm].Object.Quat.y	= (float)(lpVeryShortFUpdate->ShortGlobalShip.Quat.y * ONEOVER32767 );
			Ships[lpVeryShortFUpdate->WhoIAm].Object.Quat.z	= (float)(lpVeryShortFUpdate->ShortGlobalShip.Quat.z * ONEOVER32767 );
			Ships[lpVeryShortFUpdate->WhoIAm].LastAngle.x		= (float)(lpVeryShortFUpdate->ShortGlobalShip.Angle.x * SHORTANGLEMODIFIERUNPACK );
			Ships[lpVeryShortFUpdate->WhoIAm].LastAngle.y		= (float)(lpVeryShortFUpdate->ShortGlobalShip.Angle.y * SHORTANGLEMODIFIERUNPACK );
			Ships[lpVeryShortFUpdate->WhoIAm].LastAngle.z		= (float)(lpVeryShortFUpdate->ShortGlobalShip.Angle.z * SHORTANGLEMODIFIERUNPACK );
			Ships[lpVeryShortFUpdate->WhoIAm].Object.Bank		= (float) (lpVeryShortFUpdate->ShortGlobalShip.Bank / SHORTBANKMODIFIER);
			Ships[lpVeryShortFUpdate->WhoIAm].Primary			= lpVeryShortFUpdate->ShortGlobalShip.Primary;
			Ships[lpVeryShortFUpdate->WhoIAm].Secondary		= lpVeryShortFUpdate->ShortGlobalShip.Secondary;
			Ships[lpVeryShortFUpdate->WhoIAm].PrimPowerLevel = (float) lpVeryShortFUpdate->ShortGlobalShip.PrimPowerLevel;
			Ships[lpVeryShortFUpdate->WhoIAm].JustRecievedPacket = TRUE;
			Ships[lpVeryShortFUpdate->WhoIAm].Object.Noise		= 1.0F;
			

			// Need This for missiles to work....
			SetShipBankAndMat( &Ships[lpVeryShortFUpdate->WhoIAm].Object );
			
			if( ( Ships[ lpVeryShortFUpdate->WhoIAm ].Object.Flags & SHIP_PrimFire ) || ( Ships[ lpVeryShortFUpdate->WhoIAm ].Object.Flags & SHIP_MulFire ) )
				FirePrimaryWeapons( lpVeryShortFUpdate->WhoIAm );					// Fire Primary Weapons	
			if( ( Ships[ lpVeryShortFUpdate->WhoIAm ].Object.Flags & SHIP_SecFire ) )
				FireSecondaryWeapons( lpVeryShortFUpdate->WhoIAm );					// Fire Secondary Weapons

			Ships[ lpVeryShortFUpdate->WhoIAm ].Primary		= ConvPrimToNormWeapon( Ships[ lpVeryShortFUpdate->WhoIAm ].Primary );
			Ships[ lpVeryShortFUpdate->WhoIAm ].Secondary = ConvSecToNormWeapon( Ships[ lpVeryShortFUpdate->WhoIAm ].Secondary );

/* Added by DC 30/6/98 */
			if( ( OldMode == DEATH_MODE ) && ( Ships[lpVeryShortFUpdate->WhoIAm].Object.Mode == LIMBO_MODE ) ||
				( OldMode == NORMAL_MODE ) && ( Ships[lpVeryShortFUpdate->WhoIAm].Object.Mode == LIMBO_MODE ) )
			{
				if( GoreGuts ) ThrowOutRider( lpVeryShortFUpdate->WhoIAm );
				PlayPannedSfx( SFX_BikeExplode, Ships[ lpVeryShortFUpdate->WhoIAm ].Object.Group , &Ships[lpVeryShortFUpdate->WhoIAm].Object.Pos, 0.0F );
				StopTaunt();
			}
/* ------------------- */
			DemoShipInit[ lpVeryShortFUpdate->WhoIAm ] = TRUE;
			return;
		}
		else
		{
			wsprintf(dBuf, "someone else is in your slot..%d\n", lpVeryShortFUpdate->WhoIAm);
			OutputDebugString( dBuf );
			return;
		}


    case MSG_GROUPONLY_VERYSHORTFUPDATE:

		lpGroupOnly_VeryShortFUpdate = (LPGROUPONLY_VERYSHORTFUPDATEMSG)MsgPnt;
		if( lpGroupOnly_VeryShortFUpdate->WhoIAm != WhoIAm )
		{
			OldMode = Ships[lpGroupOnly_VeryShortFUpdate->WhoIAm].Object.Mode;
			CheckCharging( lpGroupOnly_VeryShortFUpdate->WhoIAm, Ships[ lpGroupOnly_VeryShortFUpdate->WhoIAm ].Object.Flags, lpGroupOnly_VeryShortFUpdate->ShortGlobalShip.Flags );

			if( !( Ships[lpGroupOnly_VeryShortFUpdate->WhoIAm].Object.Flags & SHIP_Multiple ) && ( lpGroupOnly_VeryShortFUpdate->ShortGlobalShip.Flags & SHIP_Multiple ) )
			{
				for( Count = 0; Count < MAXMULTIPLES; Count++ ) Ships[ lpGroupOnly_VeryShortFUpdate->WhoIAm ].OrbModels[ Count ] = (uint16) -1;
				Ships[lpGroupOnly_VeryShortFUpdate->WhoIAm].NumMultiples = (BYTE)((lpGroupOnly_VeryShortFUpdate->ShortGlobalShip.Flags >> SHIP_NumMultiples_Bit1 ) & 15);
				CreateOrbitPulsar( lpGroupOnly_VeryShortFUpdate->WhoIAm );
			}
			else
			{
				if( ( Ships[ lpGroupOnly_VeryShortFUpdate->WhoIAm ].Object.Flags & SHIP_Multiple ) &&
					( ((BYTE)((lpGroupOnly_VeryShortFUpdate->ShortGlobalShip.Flags >> SHIP_NumMultiples_Bit1 ) & 15)) != Ships[lpGroupOnly_VeryShortFUpdate->WhoIAm].NumMultiples ) )
				{
					Ships[lpGroupOnly_VeryShortFUpdate->WhoIAm].NumMultiples = (BYTE)((lpGroupOnly_VeryShortFUpdate->ShortGlobalShip.Flags >> SHIP_NumMultiples_Bit1 ) & 15);
					CreateOrbitPulsar( lpGroupOnly_VeryShortFUpdate->WhoIAm );
				}
			}
			UnPackShipFlags( lpGroupOnly_VeryShortFUpdate->WhoIAm , lpGroupOnly_VeryShortFUpdate->ShortGlobalShip.Flags );
			Ships[lpGroupOnly_VeryShortFUpdate->WhoIAm].Object.Pos.x		= (float) lpGroupOnly_VeryShortFUpdate->ShortGlobalShip.Pos.x;
			Ships[lpGroupOnly_VeryShortFUpdate->WhoIAm].Object.Pos.y		= (float) lpGroupOnly_VeryShortFUpdate->ShortGlobalShip.Pos.y;
			Ships[lpGroupOnly_VeryShortFUpdate->WhoIAm].Object.Pos.z		= (float) lpGroupOnly_VeryShortFUpdate->ShortGlobalShip.Pos.z;
			Ships[lpGroupOnly_VeryShortFUpdate->WhoIAm].Object.Group		= (int16) lpGroupOnly_VeryShortFUpdate->ShortGlobalShip.GroupImIn;
			Ships[lpGroupOnly_VeryShortFUpdate->WhoIAm].Object.Quat.w	= (float)(lpGroupOnly_VeryShortFUpdate->ShortGlobalShip.Quat.w * ONEOVER32767 );
			Ships[lpGroupOnly_VeryShortFUpdate->WhoIAm].Object.Quat.x	= (float)(lpGroupOnly_VeryShortFUpdate->ShortGlobalShip.Quat.x * ONEOVER32767 );
			Ships[lpGroupOnly_VeryShortFUpdate->WhoIAm].Object.Quat.y	= (float)(lpGroupOnly_VeryShortFUpdate->ShortGlobalShip.Quat.y * ONEOVER32767 );
			Ships[lpGroupOnly_VeryShortFUpdate->WhoIAm].Object.Quat.z	= (float)(lpGroupOnly_VeryShortFUpdate->ShortGlobalShip.Quat.z * ONEOVER32767 );
			Ships[lpGroupOnly_VeryShortFUpdate->WhoIAm].Object.Bank		= (float) (lpGroupOnly_VeryShortFUpdate->ShortGlobalShip.Bank / SHORTBANKMODIFIER);
			Ships[lpGroupOnly_VeryShortFUpdate->WhoIAm].Primary				= lpGroupOnly_VeryShortFUpdate->ShortGlobalShip.Primary;
			Ships[lpGroupOnly_VeryShortFUpdate->WhoIAm].Secondary			= lpGroupOnly_VeryShortFUpdate->ShortGlobalShip.Secondary;
			Ships[lpGroupOnly_VeryShortFUpdate->WhoIAm].PrimPowerLevel	= (float) lpGroupOnly_VeryShortFUpdate->ShortGlobalShip.PrimPowerLevel;
			Ships[lpGroupOnly_VeryShortFUpdate->WhoIAm].Object.Noise		= 1.0F;
			
			// Need This for missiles to work....
			SetShipBankAndMat( &Ships[lpGroupOnly_VeryShortFUpdate->WhoIAm].Object );
			
			if( ( Ships[ lpGroupOnly_VeryShortFUpdate->WhoIAm ].Object.Flags & SHIP_PrimFire ) || ( Ships[ lpGroupOnly_VeryShortFUpdate->WhoIAm ].Object.Flags & SHIP_MulFire ) )
				FirePrimaryWeapons( lpGroupOnly_VeryShortFUpdate->WhoIAm );					// Fire Primary Weapons	
			if( ( Ships[ lpGroupOnly_VeryShortFUpdate->WhoIAm ].Object.Flags & SHIP_SecFire ) )
				FireSecondaryWeapons( lpGroupOnly_VeryShortFUpdate->WhoIAm );					// Fire Secondary Weapons

			Ships[ lpGroupOnly_VeryShortFUpdate->WhoIAm ].Primary = ConvPrimToNormWeapon( Ships[ lpGroupOnly_VeryShortFUpdate->WhoIAm ].Primary );
			Ships[ lpGroupOnly_VeryShortFUpdate->WhoIAm ].Secondary = ConvSecToNormWeapon( Ships[ lpGroupOnly_VeryShortFUpdate->WhoIAm ].Secondary );

			if( ( OldMode == DEATH_MODE ) && ( Ships[lpGroupOnly_VeryShortFUpdate->WhoIAm].Object.Mode == LIMBO_MODE ) ||
				( OldMode == NORMAL_MODE ) && ( Ships[lpGroupOnly_VeryShortFUpdate->WhoIAm].Object.Mode == LIMBO_MODE ) )
			{
				if( GoreGuts ) ThrowOutRider( lpGroupOnly_VeryShortFUpdate->WhoIAm );
				PlayPannedSfx( SFX_BikeExplode, Ships[ lpGroupOnly_VeryShortFUpdate->WhoIAm ].Object.Group , &Ships[lpGroupOnly_VeryShortFUpdate->WhoIAm].Object.Pos, 0.0F );
				StopTaunt();
			}

			DemoShipInit[ lpGroupOnly_VeryShortFUpdate->WhoIAm ] = TRUE;
			return;
		}
		else
		{
			wsprintf(dBuf, "someone else is in your slot..%d\n", lpGroupOnly_VeryShortFUpdate->WhoIAm);
			OutputDebugString( dBuf );
			return;
		}


    case MSG_VERYSHORTUPDATE:

		if (MyGameStatus == STATUS_StartingMultiplayer)
		{
			// only need ship scores...
			lpVeryShortUpdate = (LPVERYSHORTUPDATEMSG) MsgPnt;
			// ...and need to get host game status...
			if( lpVeryShortUpdate->ShortGlobalShip.Flags & SHIP_IsHost  )
				OverallGameStatus = lpVeryShortUpdate->ShortGlobalShip.Status;
			// ...and need to get names...
			CheckForName( lpVeryShortUpdate->WhoIAm );
			// ...and fill out GameStatus
			GameStatus[lpVeryShortUpdate->WhoIAm] = lpVeryShortUpdate->ShortGlobalShip.Status;
			return;
		}
		else
		{
			lpVeryShortUpdate = (LPVERYSHORTUPDATEMSG) MsgPnt;
			if( lpVeryShortUpdate->WhoIAm != WhoIAm )
			{
				Ships[lpVeryShortUpdate->WhoIAm].PacketDelay = PacketDelay;
				CheckCharging( lpVeryShortUpdate->WhoIAm, Ships[ lpVeryShortUpdate->WhoIAm ].Object.Flags, lpVeryShortUpdate->ShortGlobalShip.Flags );
				OldMode = Ships[lpVeryShortUpdate->WhoIAm].Object.Mode;
				OldStatus = GameStatus[lpVeryShortUpdate->WhoIAm];
				if( !( Ships[lpVeryShortUpdate->WhoIAm].Object.Flags & SHIP_Multiple ) && ( lpVeryShortUpdate->ShortGlobalShip.Flags & SHIP_Multiple ) )
				{
					for( Count = 0; Count < MAXMULTIPLES; Count++ ) Ships[ lpVeryShortUpdate->WhoIAm ].OrbModels[ Count ] = (uint16) -1;
					Ships[lpVeryShortUpdate->WhoIAm].NumMultiples = (BYTE)((lpVeryShortUpdate->ShortGlobalShip.Flags >> SHIP_NumMultiples_Bit1 ) & 15);
					CreateOrbitPulsar( lpVeryShortUpdate->WhoIAm );
				}
				else
				{
					if( ( Ships[ lpVeryShortUpdate->WhoIAm ].Object.Flags & SHIP_Multiple ) &&
						( ((BYTE)((lpVeryShortUpdate->ShortGlobalShip.Flags >> SHIP_NumMultiples_Bit1 ) & 15)) != Ships[lpVeryShortUpdate->WhoIAm].NumMultiples ) )
					{
						Ships[lpVeryShortUpdate->WhoIAm].NumMultiples = (BYTE)((lpVeryShortUpdate->ShortGlobalShip.Flags >> SHIP_NumMultiples_Bit1 ) & 15);
						CreateOrbitPulsar( lpVeryShortUpdate->WhoIAm );
					}
				}
				UnPackShipFlags( lpVeryShortUpdate->WhoIAm , lpVeryShortUpdate->ShortGlobalShip.Flags );
				Ships[lpVeryShortUpdate->WhoIAm].Object.Pos.x		= (float) lpVeryShortUpdate->ShortGlobalShip.Pos.x;
				Ships[lpVeryShortUpdate->WhoIAm].Object.Pos.y		= (float) lpVeryShortUpdate->ShortGlobalShip.Pos.y;
				Ships[lpVeryShortUpdate->WhoIAm].Object.Pos.z		= (float) lpVeryShortUpdate->ShortGlobalShip.Pos.z;
				Ships[lpVeryShortUpdate->WhoIAm].Move_Off.x		= (lpVeryShortUpdate->ShortGlobalShip.Move_Off.x * ONEOVER32767) * (lpVeryShortUpdate->ShortGlobalShip.Move_Off_Scalar * ONEOVER256 );
				Ships[lpVeryShortUpdate->WhoIAm].Move_Off.y		= (lpVeryShortUpdate->ShortGlobalShip.Move_Off.y * ONEOVER32767) * (lpVeryShortUpdate->ShortGlobalShip.Move_Off_Scalar * ONEOVER256 );
				Ships[lpVeryShortUpdate->WhoIAm].Move_Off.z		= (lpVeryShortUpdate->ShortGlobalShip.Move_Off.z * ONEOVER32767) * (lpVeryShortUpdate->ShortGlobalShip.Move_Off_Scalar * ONEOVER256 ); 
				Ships[lpVeryShortUpdate->WhoIAm].Object.Group		= (int16) lpVeryShortUpdate->ShortGlobalShip.GroupImIn;
				Ships[lpVeryShortUpdate->WhoIAm].Object.Quat.w	= (float)(lpVeryShortUpdate->ShortGlobalShip.Quat.w * ONEOVER32767 );
				Ships[lpVeryShortUpdate->WhoIAm].Object.Quat.x		= (float)(lpVeryShortUpdate->ShortGlobalShip.Quat.x * ONEOVER32767 );
				Ships[lpVeryShortUpdate->WhoIAm].Object.Quat.y	= (float)(lpVeryShortUpdate->ShortGlobalShip.Quat.y * ONEOVER32767 );
				Ships[lpVeryShortUpdate->WhoIAm].Object.Quat.z		= (float)(lpVeryShortUpdate->ShortGlobalShip.Quat.z * ONEOVER32767 );
				Ships[lpVeryShortUpdate->WhoIAm].LastAngle.x		= (float)(lpVeryShortUpdate->ShortGlobalShip.Angle.x * SHORTANGLEMODIFIERUNPACK );
				Ships[lpVeryShortUpdate->WhoIAm].LastAngle.y		= (float)(lpVeryShortUpdate->ShortGlobalShip.Angle.y * SHORTANGLEMODIFIERUNPACK );
				Ships[lpVeryShortUpdate->WhoIAm].LastAngle.z		= (float)(lpVeryShortUpdate->ShortGlobalShip.Angle.z * SHORTANGLEMODIFIERUNPACK );
				Ships[lpVeryShortUpdate->WhoIAm].Object.Bank		= (float) (lpVeryShortUpdate->ShortGlobalShip.Bank / SHORTBANKMODIFIER);
				Ships[lpVeryShortUpdate->WhoIAm].JustRecievedPacket = TRUE;
				GameStatus[lpVeryShortUpdate->WhoIAm]				= lpVeryShortUpdate->ShortGlobalShip.Status;

				if( lpVeryShortUpdate->ShortGlobalShip.Flags & SHIP_IsHost  )
					OverallGameStatus = lpVeryShortUpdate->ShortGlobalShip.Status;
				
				Ships[lpVeryShortUpdate->WhoIAm].dcoID = from_dcoID;
				
				if( glpDP )
				{
					if( CheckForName( lpVeryShortUpdate->WhoIAm ) )
					{
						if( RecordDemo )
						{
							// Best way I can Think of to send the names to the demo file...
							TempTime = 1;
							Demo_fwrite( &TempTime, sizeof(LONGLONG), 1, DemoFp );
							size = sizeof( NAMEMSG );
							Demo_fwrite( &size, sizeof(int), 1, DemoFp );
							Demo_fwrite( &dcoID, sizeof(DPID), 1, DemoFp );
							msg = MSG_NAME;
							Demo_fwrite( &msg, sizeof(BYTE), 1, DemoFp );
							Demo_fwrite( &lpVeryShortUpdate->WhoIAm, sizeof(BYTE), 1, DemoFp );
							Demo_fwrite( &Names[lpVeryShortUpdate->WhoIAm][0], 8, 1, DemoFp );
						}
					}
				}

				if( ( OldMode == DEATH_MODE ) && ( Ships[lpVeryShortUpdate->WhoIAm].Object.Mode == LIMBO_MODE ) ||
					( OldMode == NORMAL_MODE ) && ( Ships[lpVeryShortUpdate->WhoIAm].Object.Mode == LIMBO_MODE ) )
				{
					if( GoreGuts ) ThrowOutRider( lpVeryShortUpdate->WhoIAm );
					PlayPannedSfx( SFX_BikeExplode, Ships[ lpVeryShortUpdate->WhoIAm ].Object.Group , &Ships[lpVeryShortUpdate->WhoIAm].Object.Pos, 0.0F );
					StopTaunt();
				}
				
				if( ( ( OldMode == LIMBO_MODE ) && ( Ships[lpVeryShortUpdate->WhoIAm].Object.Mode == NORMAL_MODE ) )
					|| ( ( OldStatus != STATUS_Normal) && (GameStatus[lpVeryShortUpdate->WhoIAm] == STATUS_Normal ) ) )
				{
					CreateReGen( lpVeryShortUpdate->WhoIAm );
				}

				DemoShipInit[ lpVeryShortUpdate->WhoIAm ] = TRUE;				
				return;
			}
			else
			{
				wsprintf(dBuf, "someone else is in your slot..%d\n", lpVeryShortUpdate->WhoIAm);
				OutputDebugString( dBuf );
				return;
			}
		}


    case MSG_UPDATE:

		if (MyGameStatus == STATUS_StartingMultiplayer)
		{
			// only need ship scores...
			lpUpdate = (LPUPDATEMSG) MsgPnt;
			// ...and need to get host game status...
			if( lpUpdate->ShortGlobalShip.Flags & SHIP_IsHost  )
				OverallGameStatus = lpUpdate->ShortGlobalShip.Status;
			// ...and need to get names...
			CheckForName( lpUpdate->WhoIAm );
			return;
		}
		else
		{
			lpUpdate = (LPUPDATEMSG) MsgPnt;
			if( lpUpdate->WhoIAm != WhoIAm )
			{
				Ships[lpUpdate->WhoIAm].PacketDelay = PacketDelay;
				CheckCharging( lpUpdate->WhoIAm, Ships[ lpUpdate->WhoIAm ].Object.Flags, lpUpdate->ShortGlobalShip.Flags );
				OldMode = Ships[lpUpdate->WhoIAm].Object.Mode;
				OldStatus = GameStatus[lpUpdate->WhoIAm];
				if( !( Ships[lpUpdate->WhoIAm].Object.Flags & SHIP_Multiple ) && ( lpUpdate->ShortGlobalShip.Flags & SHIP_Multiple ) )
				{
					for( Count = 0; Count < MAXMULTIPLES; Count++ ) Ships[ lpUpdate->WhoIAm ].OrbModels[ Count ] = (uint16) -1;
					Ships[lpUpdate->WhoIAm].NumMultiples = (BYTE)((lpUpdate->ShortGlobalShip.Flags >> SHIP_NumMultiples_Bit1 ) & 15);
					CreateOrbitPulsar( lpUpdate->WhoIAm );
				}
				else
				{
					if( ( Ships[ lpUpdate->WhoIAm ].Object.Flags & SHIP_Multiple ) &&
						( ((BYTE)((lpUpdate->ShortGlobalShip.Flags >> SHIP_NumMultiples_Bit1 ) & 15)) != Ships[lpUpdate->WhoIAm].NumMultiples ) )
					{
						Ships[lpUpdate->WhoIAm].NumMultiples = (BYTE)((lpUpdate->ShortGlobalShip.Flags >> SHIP_NumMultiples_Bit1 ) & 15);
						CreateOrbitPulsar( lpUpdate->WhoIAm );
					}
				}

				UnPackShipFlags( lpUpdate->WhoIAm , lpUpdate->ShortGlobalShip.Flags );
				Ships[lpUpdate->WhoIAm].Object.Pos		= lpUpdate->ShortGlobalShip.Pos;
				Ships[lpUpdate->WhoIAm].Move_Off		= lpUpdate->ShortGlobalShip.Move_Off;
				Ships[lpUpdate->WhoIAm].Object.Group	= (int16) lpUpdate->ShortGlobalShip.GroupImIn;
				Ships[lpUpdate->WhoIAm].Object.Quat	= lpUpdate->ShortGlobalShip.Quat;
				Ships[lpUpdate->WhoIAm].LastAngle		= lpUpdate->ShortGlobalShip.Angle;
#ifdef	SHORTBANK
				Ships[lpUpdate->WhoIAm].Object.Bank	= (float) (lpUpdate->ShortGlobalShip.Bank / SHORTBANKMODIFIER);
#else
				Ships[lpUpdate->WhoIAm].Object.Bank	= lpUpdate->ShortGlobalShip.Bank;
#endif
				Ships[lpUpdate->WhoIAm].JustRecievedPacket = TRUE;
				GameStatus[lpUpdate->WhoIAm] = lpUpdate->ShortGlobalShip.Status;

				if( lpUpdate->ShortGlobalShip.Flags & SHIP_IsHost  )
					OverallGameStatus = lpUpdate->ShortGlobalShip.Status;
				
				Ships[lpUpdate->WhoIAm].dcoID	= from_dcoID;

				if( !Ships[lpUpdate->WhoIAm].FirstPacketRecieved  )
				{
#ifdef DEBUG_ON
					if( ((int8)( Ships[lpUpdate->WhoIAm].LastPacketID + 1)) != lpUpdate->ShortGlobalShip.LastPacketID )
					{
						wsprintf(dBuf, "Missed %d Packets From %s\n", (lpUpdate->ShortGlobalShip.LastPacketID + 1 - Ships[lpUpdate->WhoIAm].LastPacketID) ,
													 &Names[lpUpdate->WhoIAm][0] );
						OutputDebugString( dBuf );
					}
					Ships[lpUpdate->WhoIAm].LastPacketID  = lpUpdate->ShortGlobalShip.LastPacketID;
#endif
				}
				else
				{
					Ships[lpUpdate->WhoIAm].FirstPacketRecieved = FALSE;
#ifdef DEBUG_ON
					Ships[lpUpdate->WhoIAm].LastPacketID  = lpUpdate->ShortGlobalShip.LastPacketID;
#endif
				}

				if( glpDP )
				{
					if( CheckForName( lpUpdate->WhoIAm ) )
					{
						if( RecordDemo )
						{
							// Best way I can Think of to send the names to the demo file...
							TempTime = 1;
							Demo_fwrite( &TempTime, sizeof(LONGLONG), 1, DemoFp );
							size = sizeof( NAMEMSG );
							Demo_fwrite( &size, sizeof(int), 1, DemoFp );
							Demo_fwrite( &dcoID, sizeof(DPID), 1, DemoFp );
							msg = MSG_NAME;
							Demo_fwrite( &msg, sizeof(BYTE), 1, DemoFp );
							Demo_fwrite( &lpUpdate->WhoIAm, sizeof(BYTE), 1, DemoFp );
							Demo_fwrite( &Names[lpUpdate->WhoIAm][0], 8, 1, DemoFp );
						}
					}
				}

				if( ( OldMode == DEATH_MODE ) && ( Ships[lpUpdate->WhoIAm].Object.Mode == LIMBO_MODE ) ||
					( OldMode == NORMAL_MODE ) && ( Ships[lpUpdate->WhoIAm].Object.Mode == LIMBO_MODE ) )
				{
					if( GoreGuts ) ThrowOutRider( lpUpdate->WhoIAm );
					PlayPannedSfx( SFX_BikeExplode, Ships[ lpUpdate->WhoIAm ].Object.Group , &Ships[lpUpdate->WhoIAm].Object.Pos, 0.0F );
					StopTaunt();
				}

				if( ( ( OldMode == LIMBO_MODE ) && ( Ships[lpUpdate->WhoIAm].Object.Mode == NORMAL_MODE ) )
					|| ( ( OldStatus != STATUS_Normal) && (GameStatus[lpUpdate->WhoIAm] == STATUS_Normal ) ) )
				{
					CreateReGen( lpUpdate->WhoIAm );
				}

				DemoShipInit[ lpUpdate->WhoIAm ] = TRUE;				
				return;
			}
			else
			{
				wsprintf(dBuf, "someone else is in your slot..%d\n", lpUpdate->WhoIAm);
				OutputDebugString( dBuf );
				return;
			}
		}


    case MSG_NAME:

		lpName = (LPNAMEMSG)MsgPnt;
		if( lpName->WhoIAm == WhoIAm )
			return;

		for( i = 0 ; i < 8 ; i++ )
		{
			Names[lpName->WhoIAm][i] = lpName->Name[i];
			if( WhoIAm < MAX_PLAYERS )
				Names[WhoIAm][i] = biker_name[i];
		}
		Names[lpName->WhoIAm][7] = 0;
		Names[WhoIAm][7] = 0;
		DebugPrintf("Recieved name %s from player %d\n" , &Names[lpName->WhoIAm][0] , lpName->WhoIAm );
		NextworkOldBikeNum = -1;
		return;


    case MSG_FUPDATE:

		lpFUpdate = (LPFUPDATEMSG)MsgPnt;
		if( lpFUpdate->WhoIAm != WhoIAm )
		{
			OldMode = Ships[lpFUpdate->WhoIAm].Object.Mode;
			Ships[lpFUpdate->WhoIAm].PacketDelay = PacketDelay;
			CheckCharging( lpFUpdate->WhoIAm, Ships[ lpFUpdate->WhoIAm ].Object.Flags, lpFUpdate->ShortGlobalShip.Flags );

			if( !( Ships[lpFUpdate->WhoIAm].Object.Flags & SHIP_Multiple ) && ( lpFUpdate->ShortGlobalShip.Flags & SHIP_Multiple ) )
			{
				for( Count = 0; Count < MAXMULTIPLES; Count++ ) Ships[ lpFUpdate->WhoIAm ].OrbModels[ Count ] = (uint16) -1;
				Ships[lpFUpdate->WhoIAm].NumMultiples = (BYTE)((lpFUpdate->ShortGlobalShip.Flags >> SHIP_NumMultiples_Bit1 ) & 15);
				CreateOrbitPulsar( lpFUpdate->WhoIAm );
			}
			else
			{
				if( ( Ships[ lpFUpdate->WhoIAm ].Object.Flags & SHIP_Multiple ) &&
					( ((BYTE)((lpFUpdate->ShortGlobalShip.Flags >> SHIP_NumMultiples_Bit1 ) & 15)) != Ships[lpFUpdate->WhoIAm].NumMultiples ) )
				{
					Ships[lpFUpdate->WhoIAm].NumMultiples = (BYTE)((lpFUpdate->ShortGlobalShip.Flags >> SHIP_NumMultiples_Bit1 ) & 15);
					CreateOrbitPulsar( lpFUpdate->WhoIAm );
				}
			}
			UnPackShipFlags( lpFUpdate->WhoIAm , lpFUpdate->ShortGlobalShip.Flags );
			Ships[lpFUpdate->WhoIAm].Object.Pos		= lpFUpdate->ShortGlobalShip.Pos;
			Ships[lpFUpdate->WhoIAm].Move_Off	= lpFUpdate->ShortGlobalShip.Move_Off;
			Ships[lpFUpdate->WhoIAm].Object.Group  = lpFUpdate->ShortGlobalShip.GroupImIn;
			Ships[lpFUpdate->WhoIAm].Primary	= lpFUpdate->ShortGlobalShip.Primary;
			Ships[lpFUpdate->WhoIAm].Secondary	= lpFUpdate->ShortGlobalShip.Secondary;
			Ships[lpFUpdate->WhoIAm].Object.Quat	= lpFUpdate->ShortGlobalShip.Quat;
			Ships[lpFUpdate->WhoIAm].PrimPowerLevel = (float) lpFUpdate->ShortGlobalShip.PrimPowerLevel;
			Ships[lpFUpdate->WhoIAm].LastAngle	= lpFUpdate->ShortGlobalShip.Angle;
#ifdef	SHORTBANK
			Ships[lpFUpdate->WhoIAm].Object.Bank	= (float) (lpFUpdate->ShortGlobalShip.Bank / SHORTBANKMODIFIER);
#else
			Ships[lpFUpdate->WhoIAm].Object.Bank	= lpFUpdate->ShortGlobalShip.Bank;
#endif
			Ships[lpFUpdate->WhoIAm].JustRecievedPacket = TRUE;

			// Need This for missiles to work....
			SetShipBankAndMat( &Ships[lpFUpdate->WhoIAm].Object );
			Ships[lpFUpdate->WhoIAm].Object.Noise = 1.0F;
			
			if( ( Ships[ lpFUpdate->WhoIAm ].Object.Flags & SHIP_PrimFire ) || ( Ships[ lpFUpdate->WhoIAm ].Object.Flags & SHIP_MulFire ) )
				FirePrimaryWeapons( lpFUpdate->WhoIAm );					// Fire Primary Weapons	
			if( ( Ships[ lpFUpdate->WhoIAm ].Object.Flags & SHIP_SecFire ) )
				FireSecondaryWeapons( lpFUpdate->WhoIAm );					// Fire Secondary Weapons

			Ships[ lpFUpdate->WhoIAm ].Primary = ConvPrimToNormWeapon( Ships[ lpFUpdate->WhoIAm ].Primary );
			Ships[ lpFUpdate->WhoIAm ].Secondary = ConvSecToNormWeapon( Ships[ lpFUpdate->WhoIAm ].Secondary );

			if( ( OldMode == DEATH_MODE ) && ( Ships[lpFUpdate->WhoIAm].Object.Mode == LIMBO_MODE ) ||
				( OldMode == NORMAL_MODE ) && ( Ships[lpFUpdate->WhoIAm].Object.Mode == LIMBO_MODE ) )
			{
				if( GoreGuts ) ThrowOutRider( lpFUpdate->WhoIAm );
				PlayPannedSfx( SFX_BikeExplode, Ships[ lpFUpdate->WhoIAm ].Object.Group , &Ships[lpFUpdate->WhoIAm].Object.Pos, 0.0F );
				StopTaunt();
			}

			DemoShipInit[ lpFUpdate->WhoIAm ] = TRUE;
			return;
		}
		else
		{
			wsprintf(dBuf, "someone else is in your slot..%d\n", lpFUpdate->WhoIAm);
			OutputDebugString( dBuf );
			return;
		}


	case MSG_HEREIAM:

		if( IsHost && !PlayDemo )
		{
			BOOL Done = FALSE;
			lpHereIAm = (LPHEREIAMMSG)MsgPnt;
			DPlayGetSessionDesc();

			for( i = 0 ; i < MAX_PLAYERS ; i++ )
			{
				// if we have allready heard from them then we have to send back a special message..
				if( ( Ships[i].dcoID == from_dcoID ) && ( GameStatus[i] == STATUS_GetPlayerNum ) )
				{
					SendGameMessage(MSG_INIT, lpHereIAm->ID, (BYTE) i , lpHereIAm->Old_TeamNumber, 1);
					SendGameMessage(MSG_STATUS, 0, 0, 0, 0);
					Done = TRUE;
				}										  
			}

			if ( !Done )		    
			{
				if( lpHereIAm->Rejoining )
				{
					SendGameMessage(MSG_INIT, lpHereIAm->ID, lpHereIAm->Old_WhoIAm, lpHereIAm->Old_TeamNumber, 0);
					SendGameMessage(MSG_STATUS, 0, 0, 0, 0);
				}
				// new player joining the game
				else
				{
					SendGameMessage(MSG_INIT, lpHereIAm->ID, MAX_PLAYERS, 0, 0);
					SendGameMessage(MSG_STATUS, 0, 0, 0, 0);
				}
			}
		}
		return;


    case MSG_INIT:

		lpInit = (LPINITMSG) MsgPnt;
		HostDPID = lpInit->FromDpid;
		MaxKills = lpInit->MaxKills;
		OverallGameStatus = lpInit->Status;
		DPlayUpdateInterval = lpInit->PacketsPerSecond;
		PacketsSlider.value = (int) (60.0F / DPlayUpdateInterval);

		for( i = 0 ; i < MAX_PLAYERS ; i++ )			
		{															
			PlayerReady[i] = lpInit->PlayerReady[i];
			if( i != WhoIAm && i != lpInit->YouAre )
				GameStatus[i] = lpInit->GameStatus[i];
		}															
		if ( WhoIAm == 0xff )
		{
			// no need to do any of this if pseudo host
			if( !IsHost )
			{
				WhoIAm = lpInit->YouAre;
				Current_Camera_View = WhoIAm;
				WatchPlayerSelect.value = WhoIAm;

				HarmTeamMates = lpInit->HarmTeamMates;
				BrightShips = lpInit->BrightShips;
				BikeExhausts = lpInit->BikeExhausts;
				ColPerspective = lpInit->Collisions;

				UnpackPickupInfo( lpInit->PickupFlags );
				DebugPrintf("host says level is %s\n", lpInit->LevelName );

				NewLevelNum = FindSameLevel( &lpInit->LevelName[0] );
				if( NewLevelNum == -1 )
				{
					DebugPrintf("client cannot find level %s\n", lpInit->LevelName );
					WhoIAm = MAX_PLAYERS+1;
					return;
				}

				for( i = 0 ; i < MAX_PLAYERS ; i++ )
					TeamNumber[i] = lpInit->TeamNumber[i];
			}
		}
		else
		{
			// team selection order from the host..copies it for every one...
			memset (TeamNumber, 255, sizeof(BYTE) * MAX_PLAYERS);
			for( i = 0 ; i < MAX_PLAYERS ; i++ )
				TeamNumber[i] = lpInit->TeamNumber[i];
			TeamNumber[WhoIAm] = 0;
		}

		if ( BountyHunt )
			BountyBonusInterval = lpInit->BountyBonusInterval;
		if ( CaptureTheFlag || CTF )
			GoalScore = lpInit->GoalScore;

		NumPrimaryPickups = lpInit->PrimaryPickups;

		if ( CTF )
		{					
			switch ( lpInit->CTF_Type )
			{
			case CTF_STANDARD:
				NeedFlagAtHome			= TRUE;
				OwnFlagTeleportsHome	= TRUE;
				CanCarryOwnFlag			= FALSE;
				break;
			case CTF_MUST_CARRY:
				NeedFlagAtHome			= TRUE;
				OwnFlagTeleportsHome	= FALSE;
				CanCarryOwnFlag			= TRUE;
				break;
			case CTF_NO_RETURN:
				NeedFlagAtHome			= FALSE;
				OwnFlagTeleportsHome	= FALSE;
				CanCarryOwnFlag			= FALSE;
				break;
			case CTF_CAN_CARRY:
				NeedFlagAtHome			= FALSE;
				OwnFlagTeleportsHome	= FALSE;
				CanCarryOwnFlag			= TRUE;
				break;
			case CTF_ONE_TOUCH:
				NeedFlagAtHome			= FALSE;
				OwnFlagTeleportsHome	= TRUE;
				CanCarryOwnFlag			= FALSE;
				break;
			}
		}
	
		RandomPickups = lpInit->RandomPickups;
		CopyOfSeed1 = lpInit->Seed1;
		CopyOfSeed2 = lpInit->Seed2;

		// copy the stats
		memcpy( KillStats, lpInit->KillStats, sizeof(KillStats));
		memcpy( KillCounter, lpInit->KillCounter, sizeof(KillCounter));
		memcpy( BonusStats, lpInit->BonusStats, sizeof(BonusStats));

		return;


    case MSG_DROPPICKUP:

		lpDropPickup = (LPDROPPICKUPMSG)MsgPnt;
#if DEBUG_PICKUPS
		if( lpDropPickup->WhoIAm != (uint16) -1 ) DebugPrintf( "PACKET OWNER '%s' : Init '%s', Owner '%s', ID %d\n", &Names[ lpDropPickup->WhoIAm ][ 0 ], Messages[ lpDropPickup->PickupInfo.Type ], &Names[ lpDropPickup->WhoIAm ][ 0 ], lpDropPickup->PickupInfo.IDCount );
		else DebugPrintf( "PACKET OWNER '%s' : Init '%s', No Owner, ID %d\n", &Names[ lpDropPickup->WhoIAm ][ 0 ], Messages[ lpDropPickup->PickupInfo.Type ], lpDropPickup->PickupInfo.IDCount );
#endif
	 	Pickup = InitOnePickup( &lpDropPickup->PickupInfo.Pos, lpDropPickup->PickupInfo.Group,
		 				   &lpDropPickup->PickupInfo.Dir, lpDropPickup->PickupInfo.Speed,
		 				   lpDropPickup->PickupInfo.Type, lpDropPickup->WhoIAm,
	 					   lpDropPickup->PickupInfo.IDCount, lpDropPickup->PickupInfo.RegenSlot,
	 					   lpDropPickup->PickupInfo.Sparkle, lpDropPickup->PickupInfo.LifeCount,
						   lpDropPickup->PickupInfo.TriggerMod );
		return;


    case MSG_VERYSHORTDROPPICKUP:

		lpVeryShortDropPickup = (LPVERYSHORTDROPPICKUPMSG)MsgPnt;
		TempPickup.Pos.x = (float) lpVeryShortDropPickup->PickupInfo.Pos.x;
		TempPickup.Pos.y = (float) lpVeryShortDropPickup->PickupInfo.Pos.y;
		TempPickup.Pos.z = (float) lpVeryShortDropPickup->PickupInfo.Pos.z;
		TempPickup.Dir.x = lpVeryShortDropPickup->PickupInfo.Dir.x * ONEOVER32767;
		TempPickup.Dir.y = lpVeryShortDropPickup->PickupInfo.Dir.y * ONEOVER32767;
		TempPickup.Dir.z = lpVeryShortDropPickup->PickupInfo.Dir.z * ONEOVER32767;
		TempPickup.IDCount = lpVeryShortDropPickup->PickupInfo.IDCount;
		TempPickup.Type = lpVeryShortDropPickup->PickupInfo.Type;
		TempPickup.Group = lpVeryShortDropPickup->PickupInfo.Group;
		TempPickup.Speed = lpVeryShortDropPickup->PickupInfo.Speed;
		TempPickup.RegenSlot = lpVeryShortDropPickup->PickupInfo.RegenSlot;
		TempPickup.Sparkle = lpVeryShortDropPickup->PickupInfo.Sparkle;
		TempPickup.LifeCount = lpVeryShortDropPickup->PickupInfo.LifeCount;
		TempPickup.TriggerMod = lpVeryShortDropPickup->PickupInfo.TriggerMod;
#if DEBUG_PICKUPS
		if( lpDropPickup->WhoIAm != (uint16) -1 ) DebugPrintf( "PACKET OWNER '%s' : Init '%s', Owner '%s', ID %d\n", &Names[ lpDropPickup->WhoIAm ][ 0 ], Messages[ TempPickup.Type ], &Names[ lpDropPickup->WhoIAm ][ 0 ], TempPickup.IDCount );
		else DebugPrintf( "PACKET OWNER '%s' : Init '%s', No Owner, ID %d\n", &Names[ lpDropPickup->WhoIAm ][ 0 ], Messages[ TempPickup.Type ], TempPickup.IDCount );
#endif
	 	Pickup = InitOnePickup( &TempPickup.Pos, TempPickup.Group,
		 				   &TempPickup.Dir, TempPickup.Speed,
		 				   TempPickup.Type, lpVeryShortDropPickup->WhoIAm,
	 					   TempPickup.IDCount, TempPickup.RegenSlot,
	 					   TempPickup.Sparkle, TempPickup.LifeCount,
						   TempPickup.TriggerMod );
		return;


    case MSG_KILLPICKUP:

   		lpKillPickup = (LPKILLPICKUPMSG)MsgPnt;
		if( PlayDemo && ( Current_Camera_View == lpKillPickup->WhoIAm ) && ( lpKillPickup->KillPickupInfo.Style == PICKUPKILL_Immediate ) )
		{
			SfxForCollectPickup( lpKillPickup->KillPickupInfo.Owner, lpKillPickup->KillPickupInfo.IDCount );
		}
#if DEBUG_PICKUPS
		if( lpKillPickup->KillPickupInfo.Owner != (uint16) -1 ) DebugPrintf( "PACKET OWNER '%s' : Kill Pickup Owner '%s', ID %d\n", &Names[ lpKillPickup->WhoIAm ][ 0 ], &Names[ lpKillPickup->KillPickupInfo.Owner ][ 0 ], lpKillPickup->KillPickupInfo.IDCount );
		else DebugPrintf( "PACKET OWNER '%s' : Kill Pickup No Owner, ID %d\n", &Names[ lpKillPickup->WhoIAm ][ 0 ], lpKillPickup->KillPickupInfo.IDCount );
#endif
		KillPickup( lpKillPickup->KillPickupInfo.Owner, lpKillPickup->KillPickupInfo.IDCount, lpKillPickup->KillPickupInfo.Style );
		return;

    case MSG_TEAMGOALS:
		lpTeamGoals = (LPTEAMGOALSMSG)MsgPnt;
		SetTeamGoals( &lpTeamGoals->TeamGoalsInfo.TeamGoals[ 0 ] );
   		return;

    case MSG_SHOCKWAVE:
   		lpShockwave = (LPSHOCKWAVEMSG)MsgPnt;
   		CreateShockwave( OWNER_SHIP,
						 lpShockwave->ShockwaveInfo.Owner,
   						 &lpShockwave->ShockwaveInfo.Pos,
   						 lpShockwave->ShockwaveInfo.Group,
   						 lpShockwave->ShockwaveInfo.ShockwaveSize,
						 lpShockwave->ShockwaveInfo.Weapon );
   		return;

    case MSG_BGOUPDATE:
   		lpBGOUpdate = (LPBGOUPDATEMSG)MsgPnt;
   		UpdateBGObject( lpBGOUpdate->BGOUpdateInfo.BGObject,
						lpBGOUpdate->BGOUpdateInfo.State,
						lpBGOUpdate->BGOUpdateInfo.Time );
   		return;

    case MSG_PRIMBULLPOSDIR:
   		lpPrimBullPosDir = (LPPRIMBULLPOSDIRMSG)MsgPnt;
		InitOnePrimBull( lpPrimBullPosDir->PrimBullPosDir.OwnerType,
						lpPrimBullPosDir->PrimBullPosDir.OwnerID,
						lpPrimBullPosDir->PrimBullPosDir.BulletID,
						lpPrimBullPosDir->PrimBullPosDir.Weapon,
						lpPrimBullPosDir->PrimBullPosDir.Group,
						&lpPrimBullPosDir->PrimBullPosDir.Pos,
						&lpPrimBullPosDir->PrimBullPosDir.Offset,
						&lpPrimBullPosDir->PrimBullPosDir.Dir,
						&lpPrimBullPosDir->PrimBullPosDir.Up,
						lpPrimBullPosDir->PrimBullPosDir.PowerLevel,
						lpPrimBullPosDir->PrimBullPosDir.PLevel, FALSE );
   		return;


    case MSG_SECBULLPOSDIR:

   		lpSecBullPosDir = (LPSECBULLPOSDIRMSG)MsgPnt;
		InitOneSecBull( lpSecBullPosDir->SecBullPosDir.OwnerType,
						lpSecBullPosDir->SecBullPosDir.Owner,
						lpSecBullPosDir->SecBullPosDir.BulletID,
						lpSecBullPosDir->SecBullPosDir.Group,
						&lpSecBullPosDir->SecBullPosDir.Pos,
						&lpSecBullPosDir->SecBullPosDir.Offset,
						&lpSecBullPosDir->SecBullPosDir.Dir,
						&lpSecBullPosDir->SecBullPosDir.UpVector,
						&lpSecBullPosDir->SecBullPosDir.DropDir,
						lpSecBullPosDir->SecBullPosDir.Weapon, FALSE );
		return;


    case MSG_TITANBITS:

   		lpTitanBits = (LPTITANBITSMSG)MsgPnt;
		for( Count = 0; Count < NUMTITANBITS; Count++ )
		{
			 i = InitOneSecBull( lpTitanBits->TitanBits.OwnerType,
								lpTitanBits->TitanBits.Owner,
								(uint16) ( Count + lpTitanBits->TitanBits.BulletID ),
								lpTitanBits->TitanBits.Group,
								&lpTitanBits->TitanBits.Pos,
								&lpTitanBits->TitanBits.Offset,
								&lpTitanBits->TitanBits.Directions[ Count ],
								&lpTitanBits->TitanBits.UpVector,
								&lpTitanBits->TitanBits.DropDir,
								lpTitanBits->TitanBits.Weapon, FALSE );
			 if( i != (uint16) -1 )
				 SecBulls[i].FramelagAddition = GlobalFramelagAddition;
		}
		return;


	// Someone is claiming to have hit me...
    case MSG_SHIPHIT:

   		lpShipHit = (LPSHIPHITMSG)MsgPnt;

		if( lpShipHit->You == WhoIAm )
   		{
			lpShipHit->ShipHit.Point.x += Ships[ WhoIAm ].Object.Pos.x;
			lpShipHit->ShipHit.Point.y += Ships[ WhoIAm ].Object.Pos.y;
			lpShipHit->ShipHit.Point.z += Ships[ WhoIAm ].Object.Pos.z;

			if( ( Ships[ WhoIAm ].Object.Mode == NORMAL_MODE ) || ( Ships[ WhoIAm ].Object.Mode == DEATH_MODE ) )
			{
				if( Ships[ WhoIAm ].Object.Mode == DEATH_MODE )
				{
					lpShipHit->ShipHit.Recoil.x *= 0.1F;
					lpShipHit->ShipHit.Recoil.y *= 0.1F;
					lpShipHit->ShipHit.Recoil.z *= 0.1F;
					lpShipHit->ShipHit.Force *= 0.1F;
				}
				if( lpShipHit->ShipHit.OneOffExternalForce ) ForceExternalOneOff( WhoIAm, &lpShipHit->ShipHit.Recoil );
				else ForceExternal( WhoIAm, &lpShipHit->ShipHit.Recoil );
				if( lpShipHit->ShipHit.Force )
					RotateExternal( WhoIAm, &lpShipHit->ShipHit.Point, &lpShipHit->ShipHit.Dir, lpShipHit->ShipHit.Force );
			}
			if( Ships[ WhoIAm ].Object.Mode == NORMAL_MODE )
			{

   				if( Random_Range( 16 ) )
					PlayPannedSfx( SFX_ShipHit, Ships[ WhoIAm ].Object.Group , &lpShipHit->ShipHit.Point, 0.0F );
				else
   					PlaySfx( SFX_BikerPain , 1.0F );

				Ships[WhoIAm].ShipThatLastHitMe = lpShipHit->WhoHitYou;

				// set damage argument for DoDamage() function
   				Ships[WhoIAm].Damage = lpShipHit->ShipHit.Damage;

   				// do the damage...
   				if ( DoDamage( DONT_OVERRIDE_INVUL ) == 1 )
   				{
					// if I died...
   					Ships[WhoIAm].ShipThatLastKilledMe = lpShipHit->WhoHitYou;
   					Ships[WhoIAm].Object.Mode = DEATH_MODE;
   					Ships[WhoIAm].Timer = 0.0F;
   					// print up who killed me
					GetDeathString( lpShipHit->ShipHit.WeaponType, lpShipHit->ShipHit.Weapon, &methodstr[0] );

					if (TeamGame && ( TeamNumber[Ships[WhoIAm].ShipThatLastKilledMe] == TeamNumber[WhoIAm]))
						strcpy(&teamstr[0], FROM_YOUR_OWN_TEAM );
					else
						strcpy(&teamstr[0], "");

   					sprintf( (char*)&tempstr[0] ,"%s %s %s %s", &Names[Ships[WhoIAm].ShipThatLastKilledMe][0], "KILLED YOU WITH ", &methodstr[0], &teamstr );
   					AddColourMessageToQue(KillMessageColour, (char*)&tempstr[0] );
					ShipDiedSend( lpShipHit->ShipHit.WeaponType, lpShipHit->ShipHit.Weapon );
   				}
			}
   		}
   		return;


	// Someone is claiming to have hit me...
    case MSG_SHORTSHIPHIT:
		
   		lpShortShipHit = (LPSHORTSHIPHITMSG)MsgPnt;

		if( lpShortShipHit->You == WhoIAm )
   		{
			Point.x = lpShortShipHit->ShipHit.Point.x + Ships[ WhoIAm ].Object.Pos.x;
			Point.y = lpShortShipHit->ShipHit.Point.y + Ships[ WhoIAm ].Object.Pos.y;
			Point.z = lpShortShipHit->ShipHit.Point.z + Ships[ WhoIAm ].Object.Pos.z;
			Recoil.x = (lpShortShipHit->ShipHit.Recoil.x * ONEOVER32767) * (lpShortShipHit->ShipHit.Recoil_Scalar * ONEOVER256 );
			Recoil.y = (lpShortShipHit->ShipHit.Recoil.y * ONEOVER32767) * (lpShortShipHit->ShipHit.Recoil_Scalar * ONEOVER256 );
			Recoil.z = (lpShortShipHit->ShipHit.Recoil.z * ONEOVER32767) * (lpShortShipHit->ShipHit.Recoil_Scalar * ONEOVER256 ); 
			Force = lpShortShipHit->ShipHit.Force;
			Dir.x = (float)(lpShortShipHit->ShipHit.Dir.x * ONEOVER32767);
			Dir.y = (float)(lpShortShipHit->ShipHit.Dir.y * ONEOVER32767);
			Dir.z = (float)(lpShortShipHit->ShipHit.Dir.z * ONEOVER32767);

			if( ( Ships[ WhoIAm ].Object.Mode == NORMAL_MODE ) || ( Ships[ WhoIAm ].Object.Mode == DEATH_MODE ) )
			{
				if( Ships[ WhoIAm ].Object.Mode == DEATH_MODE )
				{
					Recoil.x *= 0.1F;
					Recoil.y *= 0.1F;
					Recoil.z *= 0.1F;
					Force *= 0.1F;
				}
				if( lpShortShipHit->ShipHit.OneOffExternalForce != 0 ) ForceExternalOneOff( WhoIAm, &Recoil );
				else ForceExternal( WhoIAm, &Recoil );
				if( Force ) RotateExternal( WhoIAm, &Point, &Dir, Force );
			}
			if( Ships[ WhoIAm ].Object.Mode == NORMAL_MODE )
			{
				if( ( lpShortShipHit->ShipHit.WeaponType == WEPTYPE_Secondary ) &&
					( lpShortShipHit->ShipHit.Weapon == SCATTERMISSILE ) )
				{
					switch( ColPerspective )
					{
						case COLPERS_Forsaken:
							if( !GodMode )
							{
								ScatterWeapons( &NullVector, MAXSCATTERED );
								PlayPannedSfx( SFX_Scattered, Ships[ WhoIAm ].Object.Group , &Ships[ WhoIAm ].Object.Pos, 0.0F );
							}
							break;

						case COLPERS_Descent:
							break;
					}
				}

   				if( Random_Range( 16 ) )
					PlayPannedSfx( SFX_ShipHit, Ships[ WhoIAm ].Object.Group , &Point, 0.0F );
				else
   					PlaySfx( SFX_BikerPain , 1.0F );

				Ships[WhoIAm].ShipThatLastHitMe	= lpShortShipHit->WhoHitYou;
   				Ships[WhoIAm].Damage					= lpShortShipHit->ShipHit.Damage;

   				// do the damage...
   				if ( DoDamage( DONT_OVERRIDE_INVUL ) == 1 )
   				{
					// if I died...
   					Ships[WhoIAm].ShipThatLastKilledMe	= lpShortShipHit->WhoHitYou;
   					Ships[WhoIAm].Object.Mode				= DEATH_MODE;
   					Ships[WhoIAm].Timer						= 0.0F;
   					// print up who killed me
					GetDeathString( lpShortShipHit->ShipHit.WeaponType, lpShortShipHit->ShipHit.Weapon, &methodstr[0] );

					if (TeamGame && ( TeamNumber[Ships[WhoIAm].ShipThatLastKilledMe] == TeamNumber[WhoIAm]))
						strcpy(&teamstr[0], FROM_YOUR_OWN_TEAM );
					else
						strcpy(&teamstr[0], "");

					// called in TOL OFF multiplayer!!
					sprintf( (char*)&tempstr[0], "%s %s %s %s", &Names[Ships[WhoIAm].ShipThatLastKilledMe][0], "KILLED YOU WITH", &methodstr[0]  ,&teamstr );
   					AddColourMessageToQue( KillMessageColour, (char*)&tempstr[0] );
					// update stats 1 (stats.c) -- somebody killed me
					UpdateKillStats(lpShortShipHit->WhoHitYou,WhoIAm,lpShortShipHit->ShipHit.WeaponType, lpShortShipHit->ShipHit.Weapon);
					// kill me :(
					ShipDiedSend( lpShortShipHit->ShipHit.WeaponType, lpShortShipHit->ShipHit.Weapon );
   				}
			}
   		}
   		return;


	// Someone has Died....Did I kill Them ??
	// this is called in multiplayer
    case MSG_SHIPDIED:

		lpShipDied = (LPSHIPDIEDMSG)MsgPnt;

		// print a message reflecting the weapon
		GetDeathString( lpShipDied->WeaponType, lpShipDied->Weapon, &methodstr[0] );
		
		// if you killed them
		if( WhoIAm == lpShipDied->WhoKilledMe )
		{
			if( TeamGame )
			{
				if( TeamNumber[lpShipDied->WhoIAm] == TeamNumber[WhoIAm] )
				{
					if( !GodMode )
					{
						// you killed someone on your own team
						AddColourMessageToQue( KillMessageColour, "%s %s %s %s" " %s", "YOU KILLED", &Names[lpShipDied->WhoIAm][0], "WITH", &methodstr[0], ON_YOUR_OWN_TEAM );
						// update stats 7 (stats.c) -- you killed someone on your own team
						UpdateKillStats(WhoIAm, lpShipDied->WhoIAm, lpShipDied->WeaponType, lpShipDied->Weapon);
					}
				}
				// if they weren't on your team
				else
				{
					// bounty team game
					if ( BountyHunt )
					{
						if ( ( Ships[ WhoIAm ].Object.Flags | Ships[ lpShipDied->WhoIAm ].Object.Flags ) & SHIP_CarryingBounty )
						{
							// you killed the person who was carrying the bounty
							if ( Ships[ lpShipDied->WhoIAm ].Object.Flags & SHIP_CarryingBounty )
								AddColourMessageToQue( KillMessageColour, "%s %s %s %s" "%s", "YOU KILLED", &Names[lpShipDied->WhoIAm][0], WITH_THE_BOUNTY, "WITH ", &methodstr[0] );
							// you had the bounty and killed someone
							else
								AddColourMessageToQue( KillMessageColour, "%s %s %s" "%s", "YOU KILLED", &Names[lpShipDied->WhoIAm][0], "WITH ", &methodstr[0] );
							
							PlaySfx( SFX_BIKER_VP, 1.0F );
							AddKill();
							// update stats 8 (stats.c) -- you killed someone in a team bounty game
							UpdateKillStats(WhoIAm, lpShipDied->WhoIAm, lpShipDied->WeaponType, lpShipDied->Weapon);
						}
						else
						{
							// you killed someone who wasnt carrying the bounty (team bounty) (no points for you!!)
							AddColourMessageToQue( KillMessageColour, NO_POINTS_FOR_KILLING_PLAYER_WITHOUT_THE_BOUNTY,
								Names[ lpShipDied->WhoIAm ] );
						}
					}
					else if( !GodMode )
					// any other team game
					{
						// you killed someone in a team game
						AddColourMessageToQue( KillMessageColour, "%s %s %s" "%s", "YOU KILLED", &Names[lpShipDied->WhoIAm][0], "WITH ", &methodstr[0] );
						AddKill();
						// update stats 9 (stats.c) -- you killed someone in a team game
						UpdateKillStats(WhoIAm, lpShipDied->WhoIAm, lpShipDied->WeaponType, lpShipDied->Weapon);
						
						if ( !Random_Range( 4 ) )
							PlaySfx( SFX_BIKER_VP, 1.0F );
					}
				}
			}
			// if not  team game
			else
			{
				// standard bounty hunt
				if ( BountyHunt )
				{
					if ( ( Ships[ WhoIAm ].Object.Flags | Ships[ lpShipDied->WhoIAm ].Object.Flags ) & SHIP_CarryingBounty )
					{
						// you killed the person who was carrying the bounty
						if ( Ships[ lpShipDied->WhoIAm ].Object.Flags & SHIP_CarryingBounty )
							AddColourMessageToQue( KillMessageColour, "%s %s %s %s" "%s", "YOU KILLED", &Names[lpShipDied->WhoIAm][0], WITH_THE_BOUNTY, "WITH ", &methodstr[0] );
						// you had the bounty and killed someone
						else
							AddColourMessageToQue( KillMessageColour, "%s %s %s" "%s", "YOU KILLED", &Names[lpShipDied->WhoIAm][0], "WITH ", &methodstr[0] );
						
						PlaySfx( SFX_BIKER_VP, 1.0F );
						AddKill();
						// update stats 10 (stats.c) -- you killed someone in a bounty game
						UpdateKillStats(WhoIAm, lpShipDied->WhoIAm, lpShipDied->WeaponType, lpShipDied->Weapon);
					}
					else
					{
						// you killed someone who wasnt carrying the bounty (no points for you!!)
						AddColourMessageToQue( KillMessageColour, NO_POINTS_FOR_KILLING_PLAYER_WITHOUT_THE_BOUNTY,
							Names[ lpShipDied->WhoIAm ] );
					}
				}
				// not a team game or bounty hunt
				else if( !GodMode )
				{
   					// you killed someone
					AddColourMessageToQue( KillMessageColour, "%s %s %s" "%s", "YOU KILLED", &Names[lpShipDied->WhoIAm][0], "WITH ", &methodstr[0] );
					AddKill();
					// update stats 2 (stats.c) -- you killed someone 
					UpdateKillStats(WhoIAm,lpShipDied->WhoIAm,lpShipDied->WeaponType,lpShipDied->Weapon); 
					if ( !Random_Range( 4 ) )
						PlaySfx( SFX_BIKER_VP, 1.0F );
				}
			}
		}
		// if you were not who killed them
		else
		{				
			if( lpShipDied->WhoIAm == lpShipDied->WhoKilledMe )
			{
				// gee someone killed themselves...
				sprintf( (char*) &tempstr[0] ,"%s %s %s", &Names[lpShipDied->WhoIAm][0], "KILLED HIMSELF WITH", &methodstr[0] );
				AddColourMessageToQue(KillMessageColour, (char*)&tempstr[0] );
			}
			else
			{
				if (TeamGame && (TeamNumber[lpShipDied->WhoIAm] == TeamNumber[lpShipDied->WhoKilledMe]))
					strcpy (&teamstr[0], ON_HIS_OWN_TEAM);
				else
					strcpy (&teamstr[0], "");

				// gee someone killed somebody...who cares...
				sprintf( (char*) &tempstr[0] ,"%s %s %s %s %s" " %s", &Names[lpShipDied->WhoKilledMe][0], "KILLED", &Names[lpShipDied->WhoIAm][0], "WITH", &methodstr[0], &teamstr );
				AddColourMessageToQue( KillMessageColour, (char*)&tempstr[0] );
			}

			// update stats 3 (stats.c) -- somebody killed someone
			UpdateKillStats(lpShipDied->WhoKilledMe,lpShipDied->WhoIAm,lpShipDied->WeaponType,lpShipDied->Weapon);
		}
		return;


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	This is where the host gets told what state he thinks we are in
ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
    case MSG_STATUS:
   		lpStatus = (LPSTATUSMSG)MsgPnt;

		if( ( GameStatus[lpStatus->WhoIAm] != STATUS_Normal) && (lpStatus->Status == STATUS_Normal ) ) 
			CreateReGen( lpStatus->WhoIAm );

		//DebugPrintf("setting status for player %d to %x\n", lpStatus->WhoIAm, lpStatus->Status );
		GameStatus[lpStatus->WhoIAm] = lpStatus->Status;

		if( GameStatus[lpStatus->WhoIAm] == STATUS_Left )
		{
			// This player has quit...
			Ships[lpStatus->WhoIAm].enable		= 0;
			Ships[lpStatus->WhoIAm].Pickups		= 0;
			Ships[lpStatus->WhoIAm].RegenSlots	= 0;
			Ships[lpStatus->WhoIAm].Mines			= 0;
			Ships[lpStatus->WhoIAm].Triggers		= 0;
			Ships[lpStatus->WhoIAm].TrigVars		= 0;
			FreeAllPlayersAcknowledgeMessageQue( lpStatus->WhoIAm );
		}
		else
		{
	//		DebugPrintf("%s status = %d\n", Names[lpStatus->WhoIAm], GameStatus[lpStatus->WhoIAm] );
			Ships[lpStatus->WhoIAm].Pickups		= lpStatus->Pickups;
			Ships[lpStatus->WhoIAm].RegenSlots	= lpStatus->RegenSlots;
			Ships[lpStatus->WhoIAm].Mines			= lpStatus->Mines;
			Ships[lpStatus->WhoIAm].Triggers		= lpStatus->Triggers;
			Ships[lpStatus->WhoIAm].TrigVars		= lpStatus->TrigVars;

			if( !IsHost )
   			{
   				if( lpStatus->IsHost )
   				{
					BYTE tempstatus;
					Ships[lpStatus->WhoIAm].Object.Flags |= SHIP_IsHost;
					
					// its a status change from the Host...
					tempstatus = OverallGameStatus;
   					OverallGameStatus = lpStatus->Status;
					if ( tempstatus != OverallGameStatus )
						DebugPrintf("Setting overall game status to %x\n", lpStatus->Status );
				}
   			}

			TeamNumber[lpStatus->WhoIAm]	= lpStatus->TeamNumber;
			PlayerReady[lpStatus->WhoIAm]		= lpStatus->IAmReady;

			if( !PlayDemo )
			{
				if( CheckForName( lpStatus->WhoIAm ) )
				{
					if( RecordDemo )
					{
						// Best way I can Think of to send the names to the demo file...
						TempTime = 1;
						Demo_fwrite( &TempTime, sizeof(LONGLONG), 1, DemoFp );
						size = sizeof( NAMEMSG );
						Demo_fwrite( &size, sizeof(int), 1, DemoFp );
						Demo_fwrite( &dcoID, sizeof(DPID), 1, DemoFp );
						msg = MSG_NAME;
						Demo_fwrite( &msg, sizeof(BYTE), 1, DemoFp );
						Demo_fwrite( &lpStatus->WhoIAm, sizeof(BYTE), 1, DemoFp );
						Demo_fwrite( &Names[lpStatus->WhoIAm][0], 8, 1, DemoFp );
					}
				}
			}
		}
		return;


	case MSG_DPLAYUPDATE:

			lpDplayUpdateMsg = (LPDPLAYUPDATEMSG)MsgPnt;

			// clients must update their dplay settings
			if(!IsHost)
			{
				// only the host can change the settings
				if(lpDplayUpdateMsg->IsHost)
				{
					// if in normal mode display text message of changes
					if( MyGameStatus == STATUS_Normal )
					{
						// collision perspective changed by the host
						if( ColPerspective != lpDplayUpdateMsg->CollisionPerspective)
						{
							// shooter decides
							if( lpDplayUpdateMsg->CollisionPerspective == COLPERS_Forsaken)
								AddColourMessageToQue(SystemMessageColour, "SHOOTER NOW DECIDES COLLISIONS");
							// target decides
							else
								AddColourMessageToQue(SystemMessageColour, "TARGET NOW DECIDES COLLISIONS");
						}
						// packet rate changed by the host
						if( DPlayUpdateInterval != lpDplayUpdateMsg->PacketsPerSecond )
							AddColourMessageToQue(SystemMessageColour, "%2.2f %s" , ( 60.0F / lpDplayUpdateMsg->PacketsPerSecond ) , PACKETS_PER_SECOND_SET );
						// short packets changed by the host
						if( UseShortPackets != lpDplayUpdateMsg->ShortPackets)
						{
							// short packets enabled
							if( lpDplayUpdateMsg->ShortPackets == TRUE)
								AddColourMessageToQue(SystemMessageColour, "SHORT PACKETS ENABLED BY HOST");
							// short packets disabled
							else
								AddColourMessageToQue(SystemMessageColour, "SHORT PACKETS DISABLED BY HOST");
						}
					}
					ColPerspective = lpDplayUpdateMsg->CollisionPerspective;
					UseShortPackets = lpDplayUpdateMsg->ShortPackets;
					DPlayUpdateInterval = lpDplayUpdateMsg->PacketsPerSecond;
					PacketsSlider.value = (int) (60.0F / DPlayUpdateInterval);
				}
			}
			return;

    case MSG_LONGSTATUS:

   		lpLongStatus = (LPLONGSTATUSMSG)MsgPnt;

		if( ( GameStatus[lpLongStatus->Status.WhoIAm] != STATUS_Normal) && (lpLongStatus->Status.Status == STATUS_Normal ) ) 
			CreateReGen( lpLongStatus->Status.WhoIAm );
		GameStatus[lpLongStatus->Status.WhoIAm]			= lpLongStatus->Status.Status;
		DebugPrintf("%s LongStatus = %d\n", Names[lpLongStatus->Status.WhoIAm], GameStatus[lpLongStatus->Status.WhoIAm] );
		Ships[lpLongStatus->Status.WhoIAm].Pickups		= lpLongStatus->Status.Pickups;
		Ships[lpLongStatus->Status.WhoIAm].RegenSlots	= lpLongStatus->Status.RegenSlots;
		Ships[lpLongStatus->Status.WhoIAm].Mines			= lpLongStatus->Status.Mines;
		Ships[lpLongStatus->Status.WhoIAm].Triggers		= lpLongStatus->Status.Triggers;
		Ships[lpLongStatus->Status.WhoIAm].TrigVars		= lpLongStatus->Status.TrigVars;
		
		if( !IsHost )
   		{
   			if( lpLongStatus->Status.IsHost )
   			{
				RandomStartPosModify = lpLongStatus->RandomStartPosModify;
				Ships[lpLongStatus->WhoIAm].Object.Flags |= SHIP_IsHost;
				NewLevelNum = FindSameLevel( &lpLongStatus->LevelName[0] );
				if( NewLevelNum == -1 )
				{
					char buf[ 128 ];
					sprintf( buf, DID_NOT_HAVE_LEVEL /*"you did not have the level chosen by the host ( %s )"*/,
						lpLongStatus->LevelName );
					SetTitleNotify( buf );
					SpecialDestroyGame();
					return;
				}
				LevelList.selected_item = NewLevelNum;
				// its a status change from the Host...
   				OverallGameStatus = lpLongStatus->Status.Status;
			}
   		}
		TeamNumber[lpLongStatus->Status.WhoIAm] = lpLongStatus->Status.TeamNumber;
		PlayerReady[lpLongStatus->Status.WhoIAm] = lpLongStatus->Status.IAmReady;

		if( !PlayDemo )
		{
			if( CheckForName( lpLongStatus->Status.WhoIAm ) )
			{
				if( RecordDemo )
				{
					// Best way I can Think of to send the names to the demo file...
					TempTime = 1;
					Demo_fwrite( &TempTime, sizeof(LONGLONG), 1, DemoFp );
					size = sizeof( NAMEMSG );
					Demo_fwrite( &size, sizeof(int), 1, DemoFp );
					Demo_fwrite( &dcoID, sizeof(DPID), 1, DemoFp );
					msg = MSG_NAME;
					Demo_fwrite( &msg, sizeof(BYTE), 1, DemoFp );
					Demo_fwrite( &lpLongStatus->Status.WhoIAm, sizeof(BYTE), 1, DemoFp );
					Demo_fwrite( &Names[lpLongStatus->Status.WhoIAm][0], 8, 1, DemoFp );
				}
			}
		}
		return;


    case MSG_SHORTPICKUP:

   		lpShortPickup = (LPSHORTPICKUPMSG)MsgPnt;
   		if( lpShortPickup->Pickups == Ships[WhoIAm].Pickups )
   		{
#if DEBUG_PICKUPS
			DebugPrintf( "Message from host to init pickups\n" );
#endif
			if( lpShortPickup->HowManyPickups != 0 )
			{
				Ships[WhoIAm].Pickups -= 1;
				RegenPickupList( &lpShortPickup->ShortPickup[0] , lpShortPickup->HowManyPickups );
			}
			else
				Ships[WhoIAm].Pickups = 0;
		}
		return;

    case MSG_SHORTREGENSLOT:
		lpShortRegenSlot = (LPSHORTREGENSLOTMSG)MsgPnt;
   		if( lpShortRegenSlot->RegenSlots == Ships[WhoIAm].RegenSlots )
   		{
#if DEBUG_PICKUPS
			DebugPrintf( "Message from host to init regenslots\n" );
#endif
			if( lpShortRegenSlot->HowManyRegenSlots != 0 )
			{
				Ships[WhoIAm].RegenSlots -= 1;
				RegenRegenSlotList( &lpShortRegenSlot->ShortRegenSlot[0] , lpShortRegenSlot->HowManyRegenSlots );
			}
			else
				Ships[WhoIAm].RegenSlots = 0;
		}
		return;


    case MSG_SHORTTRIGGER:

		lpShortTrigger = (LPSHORTTRIGGERMSG)MsgPnt;
   		if( lpShortTrigger->Triggers == Ships[WhoIAm].Triggers )
   		{
			if( lpShortTrigger->HowManyTriggers != 0 )
			{
				Ships[WhoIAm].Triggers -= 1;
				RegenTriggerList( &lpShortTrigger->ShortTrigger[0] , lpShortTrigger->HowManyTriggers );
			}
			else
				Ships[WhoIAm].Triggers -= 1;
		}
		return;


    case MSG_SHORTTRIGVAR:

		lpShortTrigVar = (LPSHORTTRIGVARMSG)MsgPnt;
   		if( lpShortTrigVar->TrigVars == Ships[WhoIAm].TrigVars )
   		{
			if( lpShortTrigVar->HowManyTrigVars != 0 )
			{
				Ships[WhoIAm].TrigVars -= 1;
				RegenTrigVarList( &lpShortTrigVar->ShortTrigVar[0] , lpShortTrigVar->HowManyTrigVars );
			}
			else
				Ships[WhoIAm].TrigVars -= 1;
		}
		return;


    case MSG_SHORTMINE:

   		lpShortMine = (LPSHORTMINEMSG)MsgPnt;
   		if( lpShortMine->Mines == Ships[WhoIAm].Mines )
   		{
			if( Ships[WhoIAm].Mines == ( ( MAXSECONDARYWEAPONBULLETS / MAXGENMINECOUNT	) + 1 ) )
				QueryPerformanceCounter( (LARGE_INTEGER *) &Time_LastValue );

  			DebugPrintf( "Message from host to init mines\n" );

			if( lpShortMine->HowManyMines != 0 )
			{
	   			Ships[WhoIAm].Mines -= 1;
	   			RegenMineList( &lpShortMine->ShortMine[0] , lpShortMine->HowManyMines );
			}
			else
				Ships[WhoIAm].Mines = 0;
   		}
   		return;


    case MSG_TEXTMSG:

		lpTextMsg = (LPTEXTMSG)MsgPnt;
		switch (lpTextMsg->TextMsgType)
		{
			case TEXTMSGTYPE_QuickTauntWhisper:
				if( TeamNumber[WhoIAm] != TeamNumber[lpTextMsg->WhoIAm] )
					return;
				sprintf( (char*) &tempstr[0] ,"%s whispers %s", &Names[lpTextMsg->WhoIAm][0],  &lpTextMsg->Text[0] );
				AddColourMessageToQue(PlayerMessageColour,  (char*)&tempstr[0] );
				return;
			case TEXTMSGTYPE_Taunt1:
			case TEXTMSGTYPE_Taunt2:
			case TEXTMSGTYPE_Taunt3:
				sprintf( (char*) &tempstr[0] ,"%s says %s", &Names[lpTextMsg->WhoIAm][0],  &lpTextMsg->Text[0] );
				AddColourMessageToQue(TauntMessageColour, (char*)&tempstr[0] );
				return;
			case TEXTMSGTYPE_QuickTaunt:
				sprintf( (char*) &tempstr[0] ,"%s says %s", &Names[lpTextMsg->WhoIAm][0],  &lpTextMsg->Text[0] );
				AddColourMessageToQue(PlayerMessageColour, (char*)&tempstr[0] );
				// received version request
				if(strcmp(&lpTextMsg->Text[0], (const char *) "VERSION") == 0)
				{
					// display my version number
					AddColourMessageToQue(SystemMessageColour, YourVersion );
					// send my version number back
					strncpy( (char *)&QuickText.text, ProjectXVersion , sizeof(ProjectXVersion) );
					SendGameMessage(MSG_TEXTMSG, 0, 0, TEXTMSGTYPE_QuickTaunt, 0);
					QuickText.text[0] = 0; // clean message buffer
				}
				return;
			case TEXTMSGTYPE_JoiningTeamGame:
				sprintf( (char*) &tempstr[0] ,"%s %s", &Names[lpTextMsg->WhoIAm][0] ,IS_JOINING_THE_GAME);
				AddColourMessageToQue(SystemMessageColour, (char*)&tempstr[0] );
				return;
			case TEXTMSGTYPE_EnteredWatchMode:
				sprintf( (char*) &tempstr[0] ,"%s %s", &Names[lpTextMsg->WhoIAm][0] ,"ENTERED WATCH MODE");
				AddColourMessageToQue(SystemMessageColour, (char*)&tempstr[0] );
				return;
			case TEXTMSGTYPE_ExitedWatchMode:
				sprintf( (char*) &tempstr[0] ,"%s %s", &Names[lpTextMsg->WhoIAm][0] ,"EXITED WATCH MODE");
				AddColourMessageToQue(SystemMessageColour, (char*)&tempstr[0] );
				return;
			case TEXTMSGTYPE_TitleMessage:
				AddTitleMessage(lpTextMsg);
				return;
			case TEXTMSGTYPE_CaptureFlagMessage:
				sprintf( (char*) tempstr ,"%s on the %s team has got the flag", Names[ lpTextMsg->WhoIAm ], TeamName[ TeamNumber[ lpTextMsg->WhoIAm ] ] );
				AddColourMessageToQue(FlagMessageColour, lpTextMsg->Text );
				if ( TeamNumber[ WhoIAm ] == TeamNumber[ lpTextMsg->WhoIAm ] )
					PlaySfx( SFX_MyTeamGotFlag, FlagVolume );
				else
					PlaySfx( SFX_OtherTeamGotFlag , FlagVolume );
				return;
			case TEXTMSGTYPE_ScoredWithFlag:
				sprintf( (char*) tempstr ,THE_COLOUR_TEAM_HAVE_SCORED, TeamName[ TeamNumber[ lpTextMsg->WhoIAm ] ] );
				AddColourMessageToQue(FlagMessageColour, (char*)&tempstr[0] );
				if ( TeamNumber[ WhoIAm ] == TeamNumber[ lpTextMsg->WhoIAm ] )
					PlaySfx( SFX_MyTeamScored, FlagVolume );
				else
					PlaySfx( SFX_OtherTeamScored , FlagVolume );
				// update bonus 6 (stats.c) -- someone else scored with flag
				UpdateBonusStats((int) lpTextMsg->WhoIAm, GoalScoreSlider.value);
				return;
			case TEXTMSGTYPE_ReturningFlag:
				if ( lpTextMsg->WhoIAm != WhoIAm )
				{
					sprintf( (char*) tempstr ,THE_COLOUR_TEAM_ARE_RETURNING_THEIR_FLAG,TeamName[ TeamNumber[ lpTextMsg->WhoIAm ] ]);
					AddColourMessageToQue(FlagMessageColour, (char*)&tempstr[0] );
				}
				return;
			case TEXTMSGTYPE_ReturnedFlag:
					sprintf( (char*) tempstr ,THE_COLOUR_TEAM_FLAG_HAS_BEEN_RETURNED,TeamName[ TeamNumber[ lpTextMsg->WhoIAm ] ] );
					AddColourMessageToQue(FlagMessageColour, (char*)&tempstr[0] );
				return;
			case TEXTMSGTYPE_FlagDriftedIn:
				sprintf( (char*) tempstr ,THE_COLOUR_TEAM_FLAG_HAS_DRIFTED_INTO_THEIR_GOAL,TeamName[ TeamNumber[ lpTextMsg->WhoIAm ] ] );
				AddColourMessageToQue(FlagMessageColour, (char*)&tempstr[0] );
				return;
			case TEXTMSGTYPE_FlagEscaped:
				sprintf( (char*) tempstr ,THE_COLOUR_TEAM_FLAG_HAS_ESCAPED_FROM_THEIR_GOAL, TeamName[ TeamNumber[ lpTextMsg->WhoIAm ] ] );
				AddColourMessageToQue(FlagMessageColour, (char*)&tempstr[0] );
				return;
			case TEXTMSGTYPE_BountyMessage:
				sprintf( (char*) tempstr ,"%s %s", Names[ lpTextMsg->WhoIAm ] , HAS_GOT_THE_BOUNTY);
				AddColourMessageToQue(FlagMessageColour, (char*)&tempstr[0] );
				PlaySfx( SFX_OtherTeamGotFlag, FlagVolume );
				return;
			case TEXTMSGTYPE_SpeechTaunt:
				PlayRecievedSpeechTaunt( lpTextMsg->WhoIAm, lpTextMsg->Text[ 0 ] );
				return;
		}
		return;


    case MSG_INTERPOLATE:

		lpInterpolate = (LPINTERPOLATEMSG)MsgPnt;
		Ships[lpInterpolate->WhoIAm].OldPos = Ships[lpInterpolate->WhoIAm].Object.Pos;
		Ships[lpInterpolate->WhoIAm].NextPos = lpInterpolate->NextPos;
		Ships[lpInterpolate->WhoIAm].OldQuat = Ships[lpInterpolate->WhoIAm].Object.Quat;
		Ships[lpInterpolate->WhoIAm].NextQuat = lpInterpolate->NextQuat;
		Ships[lpInterpolate->WhoIAm].OldBank = Ships[lpInterpolate->WhoIAm].Object.Bank;
		Ships[lpInterpolate->WhoIAm].NextBank = lpInterpolate->NextBank;
//		Ships[lpInterpolate->WhoIAm].OldTime = GameElapsedTime;
		Ships[lpInterpolate->WhoIAm].OldTime = DemoTimeSoFar;
		Ships[lpInterpolate->WhoIAm].NextTime = lpInterpolate->NextTime;
		Ships[lpInterpolate->WhoIAm].DemoInterpolate = TRUE;
		return;


    case MSG_VERYSHORTINTERPOLATE:

		lpVeryShortInterpolate = (LPVERYSHORTINTERPOLATEMSG)MsgPnt;
		Ships[lpVeryShortInterpolate->WhoIAm].NextPos.x		= (float) lpVeryShortInterpolate->NextPos.x;
		Ships[lpVeryShortInterpolate->WhoIAm].NextPos.y		= (float) lpVeryShortInterpolate->NextPos.y;
		Ships[lpVeryShortInterpolate->WhoIAm].NextPos.z		= (float) lpVeryShortInterpolate->NextPos.z;
		Ships[lpVeryShortInterpolate->WhoIAm].NextQuat.w	= (float)(lpVeryShortInterpolate->NextQuat.w * ONEOVER32767 );
		Ships[lpVeryShortInterpolate->WhoIAm].NextQuat.x	= (float)(lpVeryShortInterpolate->NextQuat.x * ONEOVER32767 );
		Ships[lpVeryShortInterpolate->WhoIAm].NextQuat.y	= (float)(lpVeryShortInterpolate->NextQuat.y * ONEOVER32767 );
		Ships[lpVeryShortInterpolate->WhoIAm].NextQuat.z	= (float)(lpVeryShortInterpolate->NextQuat.z * ONEOVER32767 );
		Ships[lpVeryShortInterpolate->WhoIAm].NextBank		= (float) (lpVeryShortInterpolate->NextBank / SHORTBANKMODIFIER);
		Ships[lpVeryShortInterpolate->WhoIAm].NextTime		= lpVeryShortInterpolate->NextTime;
		Ships[lpVeryShortInterpolate->WhoIAm].DemoInterpolate = TRUE;
		Ships[lpVeryShortInterpolate->WhoIAm].OldPos			= Ships[lpVeryShortInterpolate->WhoIAm].Object.Pos;
		Ships[lpVeryShortInterpolate->WhoIAm].OldQuat		= Ships[lpVeryShortInterpolate->WhoIAm].Object.Quat;
		Ships[lpVeryShortInterpolate->WhoIAm].OldBank		= Ships[lpVeryShortInterpolate->WhoIAm].Object.Bank;
		Ships[lpVeryShortInterpolate->WhoIAm].OldTime		= DemoTimeSoFar;
		return;

    case MSG_PINGREQUEST:

		lpPingMsg = (LPPINGMSG)MsgPnt;
		PingRequestTime = lpPingMsg->Time;
		// can we send ping reply to just that user ?
		SendGameMessage(MSG_PINGREPLY, 0, 0 , lpPingMsg->WhoIAm , 0);
		return;


    case MSG_PINGREPLY:

		lpPingMsg = (LPPINGMSG)MsgPnt;
		if( lpPingMsg->ToYou == WhoIAm )
		{
			float clocks;
			float seconds;
			uint16 milliseconds;

			// get current clocks
			QueryPerformanceCounter( (LARGE_INTEGER *) &TempLongLong );

			// number of clocks passed
			clocks = (float) (TempLongLong - lpPingMsg->Time);
			
			// Freq == clocks per second

			// convert clocks to seconds
			seconds = clocks / (float) Freq;

			// convert seconds to milliseconds
			milliseconds = (uint16) (seconds * 1000.0F);

			// store value
			PingTimes[lpPingMsg->WhoIAm] = milliseconds;

		}
		return;


    case MSG_SETTIME:

		lpSetTime = (LPSETTIMEMSG)MsgPnt;
		if( IllegalTime )
		{
			Countdown_Float = lpSetTime->TimeInfo.Time;
			IllegalTime = FALSE;
		}
		return;


    case MSG_REQTIME:

		lpReqTime = (LPREQTIMEMSG)MsgPnt;
		if( IsHost ) SetTime( Countdown_Float );
		return;


    case MSG_ACKMSG:

		lpAckMsg = (LPACKMSG)MsgPnt;
		if( lpAckMsg->AckTo == WhoIAm )
			AcknowledgeMessage( lpAckMsg->ID , 1 << lpAckMsg->WhoIAm , lpAckMsg->WhoIAm );
		return;


    case MSG_BIKENUM:

		lpBikeNumMsg = (LPBIKENUMMSG)MsgPnt;

	  	if( PlayDemo )
		{
			// if we are playing end game demo, we want to substitute the displayed bike for the players bike
			switch( GameCompleted )
			{
			case GAMECOMPLETE_WithoutAllCrystals:
			case GAMECOMPLETE_WithAllCrystalsExceptLast:
				Ships[lpBikeNumMsg->WhoIAm].BikeNum = player_config->bike;
				break;
			default:
				Ships[lpBikeNumMsg->WhoIAm].BikeNum = (int16) lpBikeNumMsg->BikeNum;
			}
		}
		else
			Ships[lpBikeNumMsg->WhoIAm].BikeNum = (int16) lpBikeNumMsg->BikeNum;

		return;

#ifdef MANUAL_SESSIONDESC_PROPAGATE
	case MSG_SESSIONDESC:
		DebugPrintf("about to evaluate session desc packet\n");
		lpSessionDescMsg = ( LPSESSIONDESCMSG )MsgPnt;
		if ( !IsHost )
		{
            if ( glpdpSD_copy )
			{
				free ( glpdpSD_copy );
				glpdpSD_copy = NULL;
			}
			
			DPlayGetSessionDesc();

			glpdpSD_copy = (LPDPSESSIONDESC2) malloc( sizeof( *glpdpSD_copy ) );
			if ( glpdpSD_copy )
			{
				*glpdpSD_copy = *glpdpSD;
				glpdpSD_copy->dwUser1 = lpSessionDescMsg->dwUser1;
				glpdpSD_copy->dwUser2 = lpSessionDescMsg->dwUser2;
				glpdpSD_copy->dwUser3 = lpSessionDescMsg->dwUser3;
				glpdpSD_copy->dwUser4 = lpSessionDescMsg->dwUser4;
			}
		}
		return;
#endif
	}

	wsprintf(dBuf, "corrupt message: %d\n", *MsgPnt);
	OutputDebugString( dBuf );
}
 
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	Procedure	:		Send a message to all or just one..
	Input		:		BYTE msg ,
						DWORD to (DplayID) ,
						BYTE ShipNum ,
						BYTE Type,
						BYTE MASK
	Output		:		nothing
ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void SendGameMessage( BYTE msg, DWORD to, BYTE ShipNum, BYTE Type, BYTE mask )
{
    LPVERYSHORTUPDATEMSG				lpVeryShortUpdate;
    LPUPDATEMSG							lpUpdate;
    LPFUPDATEMSG						lpFUpdate;
	LPVERYSHORTFUPDATEMSG				lpVeryShortFUpdate;
	LPGROUPONLY_VERYSHORTFUPDATEMSG		lpGroupOnly_VeryShortFUpdate;
    LPHEREIAMMSG						lpHereIAm;
    LPINITMSG							lpInit;
    LPPRIMBULLPOSDIRMSG					lpPrimBullPosDir;
    LPSECBULLPOSDIRMSG					lpSecBullPosDir;
    LPTITANBITSMSG						lpTitanBits;
    LPSHIPHITMSG						lpShipHit;
    LPSHORTSHIPHITMSG					lpShortShipHit;
    LPSHIPDIEDMSG						lpShipDied;
    LPDROPPICKUPMSG						lpDropPickup;
    LPVERYSHORTDROPPICKUPMSG			lpVeryShortDropPickup;
    LPKILLPICKUPMSG						lpKillPickup;
    LPTEAMGOALSMSG						lpTeamGoals;
    LPSHOCKWAVEMSG						lpShockwave;
    LPBGOUPDATEMSG						lpBGOUpdate;
    LPSTATUSMSG							lpStatus;
    LPLONGSTATUSMSG						lpLongStatus;
    LPSHORTPICKUPMSG					lpShortPickup;
    LPSHORTREGENSLOTMSG					lpShortRegenSlot;
    LPSHORTTRIGGERMSG					lpShortTrigger;
    LPSHORTTRIGVARMSG					lpShortTrigVar;
    LPSHORTMINEMSG						lpShortMine;
    LPTEXTMSG							lpTextMsg;
    LPPINGMSG							lpPingMsg;
	LPNAMEMSG							lpName;
	LPACKMSG							lpAckMsg;
	LPBIKENUMMSG						lpBikeNumMsg;
	LPYOUQUITMSG						lpYouQuitMsg;
	LPSETTIMEMSG						lpSetTime;
	LPREQTIMEMSG						lpReqTime;
	LPDPLAYUPDATEMSG					lpDplayUpdateMsg;
#ifdef MANUAL_SESSIONDESC_PROPAGATE
	LPSESSIONDESCMSG					lpSessionDescMsg;
#endif

	DPID			send_to		= 0;
	DWORD			Flags		= 0;
	DWORD			nBytes		= 0;
	DWORD			dwPriority	= 0;
	DWORD			dwTimeout	= 0;

	HRESULT			hr;
	int				i;
	int				Count;
	int				MessageColour = 2; // default message colour is light green
	char			VersionMessage[30];
	BOOL			Rejoining = FALSE;
	char			IPAdd[16];

	// set flag sfx volume
	FlagVolume = FlagSfxSlider.value / ( FlagSfxSlider.max / GLOBAL_MAX_SFX );

	if( PlayDemo || !glpDP )
		return;
	
	//DebugPrintf("about to send msg %x\n",msg);	
	switch( msg )
    {

    case MSG_YOUQUIT:

		lpYouQuitMsg = (LPYOUQUITMSG)&CommBuff[0];
        lpYouQuitMsg->MsgCode = msg;
		lpYouQuitMsg->WhoIAm = WhoIAm;
		lpYouQuitMsg->You = ShipNum;
		nBytes = sizeof( YOUQUITMSG );
#ifdef	GUARANTEEDMESSAGES
		AddGuaranteedMessage( nBytes , (void*) &CommBuff[0] , MSG_YOUQUIT , FALSE , FALSE);
		return;
#endif
		break;


    case MSG_BIKENUM:

		lpBikeNumMsg = (LPBIKENUMMSG)&CommBuff[0];
        lpBikeNumMsg->MsgCode = msg;
		lpBikeNumMsg->WhoIAm = WhoIAm;
		lpBikeNumMsg->BikeNum = (BYTE) Ships[WhoIAm].BikeNum;
		nBytes = sizeof( BIKENUMMSG );
#ifdef	GUARANTEEDMESSAGES
		AddGuaranteedMessage( nBytes , (void*) &CommBuff[0] , MSG_BIKENUM , FALSE , TRUE);
		return;
#endif
		break;


    case MSG_NAME:

		DPlaySetPlayerName(dcoID, &biker_name[0], 0);
		lpName = (LPNAMEMSG)&CommBuff[0];
        lpName->MsgCode = msg;
		lpName->WhoIAm = WhoIAm;
		for( i = 0 ; i < 8 ; i++ )
		{
			lpName->Name[i] = biker_name[i];
			Names[WhoIAm][i] = biker_name[i];
		}
		lpName->Name[7] = 0;
		Names[WhoIAm][7] = 0;
		nBytes = sizeof( NAMEMSG );
#ifdef	GUARANTEEDMESSAGES
		AddGuaranteedMessage( nBytes , (void*) &CommBuff[0] , MSG_NAME , FALSE, TRUE );
		return;
#endif
		break;
	

	case MSG_HEREIAM:

        lpHereIAm = (LPHEREIAMMSG)&CommBuff[0];
        lpHereIAm->MsgCode = msg;
        lpHereIAm->WhoIAm = WhoIAm;
        lpHereIAm->ID = (DWORD)dcoID;
        lpHereIAm->Rejoining = Rejoining;
        lpHereIAm->Old_WhoIAm = (BYTE) Old_WhoIAm;
        lpHereIAm->Old_TeamNumber = (BYTE) Old_TeamNumber;
		nBytes = sizeof( HEREIAMMSG );
        break;


    case MSG_INIT:

		if( !IsHost )
			return;
		lpInit = (LPINITMSG)&CommBuff[0];
		lpInit->FromDpid = dcoID;
        lpInit->MsgCode = msg;
        lpInit->WhoIAm = WhoIAm;
		DPlayGetSessionDesc();
		lpInit->dwUser3 = glpdpSD->dwUser3;
        send_to = to;
		lpInit->RandomPickups = RandomPickups;
		lpInit->Seed1 = CopyOfSeed1;
		lpInit->Seed2 = CopyOfSeed2;
		lpInit->HarmTeamMates = HarmTeamMates;
		lpInit->BrightShips = BrightShips;
		lpInit->BikeExhausts = BikeExhausts;
		lpInit->Collisions = ColPerspective;
		lpInit->MaxKills = MaxKills;
		lpInit->PacketsPerSecond = DPlayUpdateInterval;
		PackPickupInfo( lpInit->PickupFlags );
		lpInit->GoalScore = GoalScore;
		lpInit->BountyBonusInterval = BountyBonusInterval;
		lpInit->CTF_Type = CTFSlider.value;
		lpInit->PrimaryPickups = NumPrimaryPickups;

		// current game stats
		memcpy( lpInit->KillStats, KillStats, sizeof(lpInit->KillStats));
		memcpy( lpInit->KillCounter, KillCounter, sizeof(lpInit->KillCounter));
		memcpy( lpInit->BonusStats, BonusStats, sizeof(lpInit->KillCounter));

		for( Count = 0 ; Count < 32 ; Count++ )
			lpInit->LevelName[Count] = ShortLevelNames[NewLevelNum][Count];

		if( ( MyGameStatus == STATUS_StartingMultiplayer ) || ( MyGameStatus == STATUS_Normal ) )
		{
			// tell the player who is host thinks is ready.
			for( Count = 0 ; Count < MAX_PLAYERS ; Count++ )			
			{															
				lpInit->PlayerReady[Count] = PlayerReady[Count];
				lpInit->GameStatus[Count] = GameStatus[Count];
			}	
			
			// try to find a brand new slot
			Rejoining = FALSE;
			for( i = 0; i < MAX_PLAYERS; i++ )
			{
			    if( i != WhoIAm )
			    {
					GetIPFromDP( IPAdd, to );
					// rejoining
					if(strcmp(&PlayerInfo[i].IP[0], &IPAdd[0]) == 0)
					{
						//AddColourMessageToQue(1,"REJOINING!!!");
						Rejoining = TRUE;
						break;
					}
					// found a never used before slot
					else if(PlayerInfo[i].IP == NULL)
						break;
				}
			}

			// couldn't find a new slot so find one from a player who left
			if( i == MAX_PLAYERS)
			{
			    for( i = 0; i < MAX_PLAYERS; i++ )
			    {
					if( i != WhoIAm )
					{
		    			if(	( GameStatus[i] == STATUS_Left ) || ( GameStatus[i] == STATUS_LeftCrashed ) || ( GameStatus[i] == STATUS_Null ) )
		    				break;
					}
			    }
			}

			// rejoining
			if(Rejoining && i != MAX_PLAYERS)
				InitShipStructure(i , FALSE);	// don't reset score
			// new
			else if(!Rejoining && i != MAX_PLAYERS)
				InitShipStructure(i , TRUE);	// reset score
			
			// got a valid player number
			if(i != MAX_PLAYERS)
			{
				GameStatus[i] = STATUS_GetPlayerNum;
				lpInit->YouAre = (BYTE) i;
				DebugPrintf("YouAre set to %d at point 2\n", i);
				Names[i][0] = 0;
				Ships[i].dcoID = to;

				// Host must store info related to all players who join
				GetIPFromDP( PlayerInfo[i].IP, to );

				if( Type == (BYTE) -1 )
					TeamNumber[i] = 0;
				else
					TeamNumber[i] = Type;
				
				for( Count = 0 ; Count < MAX_PLAYERS ; Count++ )											
					lpInit->TeamNumber[Count] = TeamNumber[Count];						
				
				lpInit->Status = MyGameStatus;
				
				// getplayer
				// over the next few frames send the current stats table...
				if(	MyGameStatus == STATUS_Normal )
				{
					// StatsCount = MAX_PLAYERS;
					CopyPickups( (uint16) i );
					CopyRegenSlots( (uint16) i );
					CopyTriggers( (uint16) i );
					CopyTrigVars( (uint16) i );
					CopyMines( (uint16) i );
				}
			}
			// The game is currently full or nearly so dont let anyone else join...
			else
				lpInit->YouAre = MAX_PLAYERS+2;
		}

		DebugPrintf("MSG_INIT being sent: lpInit->YouAre = %d\n", lpInit->YouAre);
		nBytes = sizeof( INITMSG );
		break;


    case MSG_VERYSHORTUPDATE: // short packets on

        lpVeryShortUpdate = (LPVERYSHORTUPDATEMSG)&CommBuff[0];
        lpVeryShortUpdate->MsgCode = msg;
        lpVeryShortUpdate->WhoIAm = WhoIAm;
		lpVeryShortUpdate->ShortGlobalShip = VeryShortGlobalShip;
        nBytes = sizeof( VERYSHORTUPDATEMSG );
		dwTimeout = 100; //ms
        break;


    case MSG_UPDATE: // short packets off

        lpUpdate = (LPUPDATEMSG)&CommBuff[0];
        lpUpdate->MsgCode = msg;
        lpUpdate->WhoIAm = WhoIAm;
		lpUpdate->ShortGlobalShip = ShortGlobalShip;
        nBytes = sizeof( UPDATEMSG );
		dwTimeout = 100; //ms
        break;


    case MSG_FUPDATE: // short packets off + fire weapon data

        lpFUpdate = (LPFUPDATEMSG)&CommBuff[0];
        lpFUpdate->MsgCode = msg;
        lpFUpdate->WhoIAm = WhoIAm;
		lpFUpdate->ShortGlobalShip = FShortGlobalShip;
        nBytes = sizeof( FUPDATEMSG );
        break;


    case MSG_VERYSHORTFUPDATE: // short packets on + fire weapon data

        lpVeryShortFUpdate = (LPVERYSHORTFUPDATEMSG)&CommBuff[0];
        lpVeryShortFUpdate->MsgCode = msg;
        lpVeryShortFUpdate->WhoIAm = WhoIAm;
		lpVeryShortFUpdate->ShortGlobalShip = FVeryShortGlobalShip;
        nBytes = sizeof( VERYSHORTFUPDATEMSG );
        break;


    case MSG_GROUPONLY_VERYSHORTFUPDATE:

        lpGroupOnly_VeryShortFUpdate = (LPGROUPONLY_VERYSHORTFUPDATEMSG)&CommBuff[0];
        lpGroupOnly_VeryShortFUpdate->MsgCode = msg;
        lpGroupOnly_VeryShortFUpdate->WhoIAm = WhoIAm;
		lpGroupOnly_VeryShortFUpdate->ShortGlobalShip = GroupOnly_FVeryShortGlobalShip;
        nBytes = sizeof( GROUPONLY_VERYSHORTFUPDATEMSG );
        break;


    case MSG_DROPPICKUP:

        lpDropPickup = (LPDROPPICKUPMSG) &CommBuff[0];
        lpDropPickup->MsgCode = msg;
        lpDropPickup->WhoIAm = WhoIAm;
        lpDropPickup->PickupInfo = TempPickup;
        nBytes = sizeof( DROPPICKUPMSG );
#ifdef	GUARANTEEDMESSAGES
		AddGuaranteedMessage( nBytes , (void*) &CommBuff[0] , MSG_DROPPICKUP , FALSE , FALSE);
		return;
#endif
        break;


    case MSG_VERYSHORTDROPPICKUP:

        lpVeryShortDropPickup = (LPVERYSHORTDROPPICKUPMSG) &CommBuff[0];
        lpVeryShortDropPickup->MsgCode = msg;
        lpVeryShortDropPickup->WhoIAm = WhoIAm;
        lpVeryShortDropPickup->PickupInfo = VeryShortTempPickup;
        nBytes = sizeof( VERYSHORTDROPPICKUPMSG );
#ifdef	GUARANTEEDMESSAGES
		AddGuaranteedMessage( nBytes , (void*) &CommBuff[0] , MSG_VERYSHORTDROPPICKUP , FALSE , FALSE);
		return;
#endif
        break;


    case MSG_KILLPICKUP:

        lpKillPickup = (LPKILLPICKUPMSG) &CommBuff[0];
        lpKillPickup->MsgCode = msg;
        lpKillPickup->WhoIAm = WhoIAm;
        lpKillPickup->KillPickupInfo = TempKillPickup;
        nBytes = sizeof( KILLPICKUPMSG );
#ifdef	GUARANTEEDMESSAGES
		AddGuaranteedMessage( nBytes , (void*) &CommBuff[0] , MSG_KILLPICKUP , FALSE , FALSE);
		return;
#endif
        break;


    case MSG_TEAMGOALS:

        lpTeamGoals = (LPTEAMGOALSMSG) &CommBuff[0];
        lpTeamGoals->MsgCode = msg;
        lpTeamGoals->WhoIAm = WhoIAm;
        lpTeamGoals->TeamGoalsInfo = TempTeamGoals;
        nBytes = sizeof( TEAMGOALSMSG );
#ifdef	GUARANTEEDMESSAGES
		AddGuaranteedMessage( nBytes , (void*) &CommBuff[0] , MSG_TEAMGOALS, FALSE , FALSE);
		return;
#endif
        break;


    case MSG_SHOCKWAVE:

        lpShockwave = (LPSHOCKWAVEMSG) &CommBuff[0];
        lpShockwave->MsgCode = msg;
        lpShockwave->WhoIAm = WhoIAm;
        lpShockwave->ShockwaveInfo = TempShockwave;
		send_to = to;
        nBytes = sizeof( SHOCKWAVEMSG );
        break;


    case MSG_BGOUPDATE:

        lpBGOUpdate = (LPBGOUPDATEMSG) &CommBuff[0];
        lpBGOUpdate->MsgCode = msg;
        lpBGOUpdate->WhoIAm = WhoIAm;
        lpBGOUpdate->BGOUpdateInfo = TempBGOUpdate;
        nBytes = sizeof( BGOUPDATEMSG );
        break;


    case MSG_PRIMBULLPOSDIR:

        lpPrimBullPosDir = (LPPRIMBULLPOSDIRMSG)&CommBuff[0];
        lpPrimBullPosDir->MsgCode = msg;
        lpPrimBullPosDir->WhoIAm = WhoIAm;
        lpPrimBullPosDir->PrimBullPosDir = TempPrimBullPosDir;
        nBytes = sizeof( PRIMBULLPOSDIRMSG );
        break;


    case MSG_SECBULLPOSDIR:

        lpSecBullPosDir = (LPSECBULLPOSDIRMSG)&CommBuff[0];
        lpSecBullPosDir->MsgCode = msg;
        lpSecBullPosDir->WhoIAm = WhoIAm;
        lpSecBullPosDir->SecBullPosDir = TempSecBullPosDir;
        nBytes = sizeof( SECBULLPOSDIRMSG );
#ifdef	GUARANTEEDMESSAGES
		AddGuaranteedMessage( nBytes , (void*) &CommBuff[0], MSG_SECBULLPOSDIR, FALSE , FALSE);
		return;
#endif
        break;


    case MSG_TITANBITS:

        lpTitanBits = (LPTITANBITSMSG)&CommBuff[0];
        lpTitanBits->MsgCode = msg;
        lpTitanBits->WhoIAm = WhoIAm;
        lpTitanBits->TitanBits = TempTitanBits;
        nBytes = sizeof( TITANBITSMSG );
#ifdef	GUARANTEEDMESSAGES
		AddGuaranteedMessage( nBytes , (void*) &CommBuff[0], MSG_TITANBITS, FALSE , FALSE);
		return;
#endif
        break;


    case MSG_SHIPHIT:
		
        lpShipHit = (LPSHIPHITMSG)&CommBuff[0];
        lpShipHit->MsgCode = msg;
        lpShipHit->You = ShipNum;
	    lpShipHit->WhoHitYou = WhoIAm;
        lpShipHit->ShipHit = TempShipHit;
        nBytes = sizeof( SHIPHITMSG );
        break;


    case MSG_SHORTSHIPHIT:

        lpShortShipHit = (LPSHORTSHIPHITMSG)&CommBuff[0];
        lpShortShipHit->MsgCode = msg;
        lpShortShipHit->You = ShipNum;
		lpShortShipHit->WhoHitYou = WhoIAm;
        lpShortShipHit->ShipHit = ShortTempShipHit;
        nBytes = sizeof( SHORTSHIPHITMSG );
        break;


    case MSG_SHIPDIED:

        lpShipDied = (LPSHIPDIEDMSG)&CommBuff[0];
        lpShipDied->MsgCode = msg;
        lpShipDied->WhoIAm = WhoIAm;
		lpShipDied->Pos = Ships[WhoIAm].Object.Pos;
		lpShipDied->WhoKilledMe = Ships[WhoIAm].ShipThatLastKilledMe;
		lpShipDied->Type	= Type;
		lpShipDied->WeaponType = TempDied.WeaponType;
		lpShipDied->Weapon = TempDied.Weapon;
		nBytes = sizeof( SHIPDIEDMSG );
#ifdef	GUARANTEEDMESSAGES
		AddGuaranteedMessage( nBytes , (void*) &CommBuff[0], MSG_SHIPDIED , FALSE , FALSE);
		return;
#endif
        break;


    case MSG_REQTIME:
        lpReqTime = (LPREQTIMEMSG)&CommBuff[0];
        lpReqTime->MsgCode = msg;
        lpReqTime->WhoIAm = WhoIAm;
		nBytes = sizeof( REQTIMEMSG );
        break;


    case MSG_SETTIME:

        lpSetTime = (LPSETTIMEMSG)&CommBuff[0];
        lpSetTime->MsgCode = msg;
        lpSetTime->WhoIAm = WhoIAm;
		lpSetTime->TimeInfo = TempTimeSet;
		nBytes = sizeof( SETTIMEMSG );
        break;

    case MSG_STATUS:

		lpStatus									= (LPSTATUSMSG)&CommBuff[0];
        lpStatus->MsgCode					= msg;
        lpStatus->WhoIAm					= WhoIAm;
        lpStatus->IsHost						= IsHost;
		lpStatus->Status						= MyGameStatus;
		lpStatus->TeamNumber				= TeamNumber[WhoIAm];
		lpStatus->IAmReady					= PlayerReady[WhoIAm];
		lpStatus->Pickups					= Ships[WhoIAm].Pickups;	 
		lpStatus->RegenSlots				= Ships[WhoIAm].RegenSlots;
		lpStatus->Mines						= Ships[WhoIAm].Mines;		 
		lpStatus->Triggers					= Ships[WhoIAm].Triggers;	 
		lpStatus->TrigVars					= Ships[WhoIAm].TrigVars;	 
		nBytes = sizeof( STATUSMSG );

#ifdef	GUARANTEEDMESSAGES
		if ( MyGameStatus == STATUS_Left )	// send last status msg guaranteed
		{
			AddGuaranteedMessage( nBytes , (void*) &CommBuff[0] , MSG_STATUS , TRUE, TRUE);
			return;
		}
#endif
        break;

	case MSG_DPLAYUPDATE:

		lpDplayUpdateMsg					= (LPDPLAYUPDATEMSG)&CommBuff[0];
        lpDplayUpdateMsg->MsgCode					= msg;
        lpDplayUpdateMsg->WhoIAm					= WhoIAm;
        lpDplayUpdateMsg->IsHost						= IsHost;
		lpDplayUpdateMsg->PacketsPerSecond		= DPlayUpdateInterval;
		lpDplayUpdateMsg->CollisionPerspective	= ColPerspective;
		lpDplayUpdateMsg->ShortPackets			= UseShortPackets;
		break;

    case MSG_LONGSTATUS:

		lpLongStatus = (LPLONGSTATUSMSG)&CommBuff[0];
        lpLongStatus->MsgCode			= msg;
        lpLongStatus->WhoIAm				= WhoIAm;
        lpLongStatus->Status.MsgCode	= msg;
        lpLongStatus->Status.WhoIAm	= WhoIAm;
        lpLongStatus->Status.IsHost		= IsHost;
		// telling everyone what I am currently doing....
		lpLongStatus->Status.Status			= MyGameStatus;
		lpLongStatus->Status.TeamNumber = TeamNumber[WhoIAm];
		lpLongStatus->Status.IAmReady		= PlayerReady[WhoIAm];
		lpLongStatus->Status.Pickups		= Ships[WhoIAm].Pickups;	 
		lpLongStatus->Status.RegenSlots	= Ships[WhoIAm].RegenSlots;
		lpLongStatus->Status.Mines			= Ships[WhoIAm].Mines;		 
		lpLongStatus->Status.Triggers		= Ships[WhoIAm].Triggers;	 
		lpLongStatus->Status.TrigVars		= Ships[WhoIAm].TrigVars;	 

		for( Count = 0 ; Count < 32 ; Count++ )
			lpLongStatus->LevelName[Count] = ShortLevelNames[NewLevelNum][Count];
		lpLongStatus->RandomStartPosModify = RandomStartPosModify;
		nBytes = sizeof( LONGSTATUSMSG );
        break;


    case MSG_SHORTPICKUP:

		lpShortPickup = (LPSHORTPICKUPMSG)&CommBuff[0];
        lpShortPickup->MsgCode = msg;
        lpShortPickup->WhoIAm = WhoIAm;
        lpShortPickup->Pickups = Ships[ShipNum].Pickups;
		GenPickupList( ShipNum, &lpShortPickup->ShortPickup[0] , &lpShortPickup->HowManyPickups , Ships[ShipNum].Pickups );
		nBytes = sizeof( SHORTPICKUPMSG );
		send_to = Ships[ShipNum].dcoID;
        break;


    case MSG_SHORTREGENSLOT:

		lpShortRegenSlot = (LPSHORTREGENSLOTMSG)&CommBuff[0];
        lpShortRegenSlot->MsgCode		= msg;
        lpShortRegenSlot->WhoIAm		= WhoIAm;
        lpShortRegenSlot->RegenSlots	= Ships[ShipNum].RegenSlots;
		GenRegenSlotList( ShipNum, &lpShortRegenSlot->ShortRegenSlot[0] , &lpShortRegenSlot->HowManyRegenSlots , Ships[ShipNum].RegenSlots );
		nBytes	= sizeof( SHORTREGENSLOTMSG );
		send_to = Ships[ShipNum].dcoID;
        break;


    case MSG_SHORTTRIGGER:

		lpShortTrigger = (LPSHORTTRIGGERMSG)&CommBuff[0];
        lpShortTrigger->MsgCode	= msg;
        lpShortTrigger->WhoIAm	= WhoIAm;
        lpShortTrigger->Triggers	= Ships[ShipNum].Triggers;
		GenTriggerList( ShipNum, &lpShortTrigger->ShortTrigger[0] , &lpShortTrigger->HowManyTriggers, Ships[ShipNum].Triggers );
		nBytes	= sizeof( SHORTTRIGGERMSG );
		send_to	= Ships[ShipNum].dcoID;
        break;


    case MSG_SHORTTRIGVAR:

		lpShortTrigVar = (LPSHORTTRIGVARMSG)&CommBuff[0];
        lpShortTrigVar->MsgCode	= msg;
        lpShortTrigVar->WhoIAm	= WhoIAm;
        lpShortTrigVar->TrigVars	= Ships[ShipNum].TrigVars;
		GenTrigVarList( ShipNum, &lpShortTrigVar->ShortTrigVar[0] , &lpShortTrigVar->HowManyTrigVars, Ships[ShipNum].TrigVars );
        nBytes = sizeof( SHORTTRIGVARMSG );
		send_to = Ships[ShipNum].dcoID;
        break;


    case MSG_SHORTMINE:

		lpShortMine					= (LPSHORTMINEMSG)&CommBuff[0];
        lpShortMine->MsgCode	= msg;
        lpShortMine->WhoIAm	= WhoIAm;
        lpShortMine->Mines		= Ships[ShipNum].Mines;
		GenMineList( ShipNum, &lpShortMine->ShortMine[0] , &lpShortMine->HowManyMines, Ships[ShipNum].Mines );
		nBytes	= sizeof( SHORTMINEMSG );
		send_to = Ships[ShipNum].dcoID;
        break;

    case MSG_TEXTMSG:

		lpTextMsg = (LPTEXTMSG)&CommBuff[0];
        lpTextMsg->MsgCode	= msg;
        lpTextMsg->WhoIAm		= WhoIAm;
        
		switch( Type )
		{
			case TEXTMSGTYPE_Taunt1:
				strncpy( &lpTextMsg->Text[0]	, &MacroText1.text[0] , MAXTEXTMSG );
				lpTextMsg->TextMsgType = Type;
				MessageColour = TauntMessageColour;
				break;
			case TEXTMSGTYPE_Taunt2:
				strncpy( &lpTextMsg->Text[0]	, &MacroText2.text[0] , MAXTEXTMSG );
				lpTextMsg->TextMsgType = Type;
				MessageColour = TauntMessageColour;
				break;
			case TEXTMSGTYPE_Taunt3:
				strncpy( &lpTextMsg->Text[0]	, &MacroText3.text[0] , MAXTEXTMSG );
				lpTextMsg->TextMsgType = Type;
				MessageColour = TauntMessageColour;
				break;
			case TEXTMSGTYPE_TitleMessage:
				strncpy( &lpTextMsg->Text[0]	, &MacroText4.text[0] , MAXTEXTMSG );
				lpTextMsg->TextMsgType = Type;
				MessageColour = SystemMessageColour;
				break;
			case TEXTMSGTYPE_JoiningTeamGame:
	//			strncpy( &lpTextMsg->Text[0]	, "%s is joining the game" , MAXTEXTMSG );
				lpTextMsg->TextMsgType = Type;
				MessageColour = SystemMessageColour;
				break;
			case TEXTMSGTYPE_EnteredWatchMode:
	//			strncpy( &lpTextMsg->Text[0]	, "%s entered watch mode" , MAXTEXTMSG );
				lpTextMsg->TextMsgType = Type;
				MessageColour = SystemMessageColour;
				break;
			case TEXTMSGTYPE_ExitedWatchMode:
	//			strncpy( &lpTextMsg->Text[0]	, "%s exited watch mode" , MAXTEXTMSG );
				lpTextMsg->TextMsgType = Type;
				MessageColour = SystemMessageColour;
				break;
			case TEXTMSGTYPE_QuickTaunt:
				strncpy( &lpTextMsg->Text[0]	, &QuickText.text[0] , MAXTEXTMSG );
				lpTextMsg->TextMsgType = Type;
				MessageColour = MyMessageColour;
				AddColourMessageToQue( MessageColour, (char*) &lpTextMsg->Text[0] );
				// sending version request
				if(strcmp(&lpTextMsg->Text[0], (const char *) "VERSION") == 0)
				{
					// display my version number
					AddColourMessageToQue(SystemMessageColour, YourVersion );
					VersionMessage[0] = 0;
				}
				break;
			case TEXTMSGTYPE_QuickTauntWhisper:
				strncpy( &lpTextMsg->Text[0]	, &QuickTextWhisper.text[0] , MAXTEXTMSG );
				lpTextMsg->TextMsgType = Type;
				MessageColour = MyMessageColour;
				break;
			case TEXTMSGTYPE_CaptureFlagMessage:
				lpTextMsg->TextMsgType = Type;
				strncpy( &lpTextMsg->Text[0]	, CTFMessage , MAXTEXTMSG );
				PlaySfx( SFX_MyTeamGotFlag , FlagVolume );
				MessageColour = FlagMessageColour;
				break;
			case TEXTMSGTYPE_ScoredWithFlag:
				lpTextMsg->TextMsgType = Type;
				lpTextMsg->Text[0] = 0;
				PlaySfx( SFX_MyTeamScored, FlagVolume );
				MessageColour = FlagMessageColour;
				break;
			case TEXTMSGTYPE_ReturningFlag:
				lpTextMsg->TextMsgType = Type;
				lpTextMsg->Text[0] = 0;
				MessageColour = FlagMessageColour;
				break;
			case TEXTMSGTYPE_ReturnedFlag:
				lpTextMsg->TextMsgType = Type;
				lpTextMsg->Text[0] = 0;
				MessageColour = FlagMessageColour;
				break;
			case TEXTMSGTYPE_FlagDriftedIn:
				lpTextMsg->TextMsgType = Type;
				lpTextMsg->Text[0] = 0;
				MessageColour = FlagMessageColour;
				break;
			case TEXTMSGTYPE_FlagEscaped:
				lpTextMsg->TextMsgType = Type;
				lpTextMsg->Text[0] = 0;
				MessageColour = FlagMessageColour;
				break;
			case TEXTMSGTYPE_BountyMessage:
				lpTextMsg->TextMsgType = Type;
				lpTextMsg->Text[0] = 0;
				PlaySfx( SFX_MyTeamGotFlag , FlagVolume );
				MessageColour = FlagMessageColour;
				break;
			case TEXTMSGTYPE_SpeechTaunt:
				lpTextMsg->TextMsgType = Type;
				lpTextMsg->Text[0] = CurrentTauntVariant;
				MessageColour = PlayerMessageColour;
				send_to = to;
				break;
			default:
				lpTextMsg->TextMsgType = TEXTMSGTYPE_Taunt1;
		}
		// put a null at the end just in case...
		lpTextMsg->Text[MAXTEXTMSG-1] = 0;
		nBytes = sizeof( TEXTMSG );

		// quick taunt already delt with
		if (MyGameStatus != STATUS_StartingMultiplayer && Type != TEXTMSGTYPE_QuickTaunt)
			AddColourMessageToQue( MessageColour, (char*) &lpTextMsg->Text[0] );
		break;


    case MSG_PINGREQUEST:
		lpPingMsg = (LPPINGMSG)&CommBuff[0];
        lpPingMsg->MsgCode = msg;
        lpPingMsg->WhoIAm = WhoIAm;
		QueryPerformanceCounter( (LARGE_INTEGER *) &lpPingMsg->Time);
		nBytes = sizeof( PINGMSG );
		break;


    case MSG_PINGREPLY:

		lpPingMsg = (LPPINGMSG)&CommBuff[0];
        lpPingMsg->MsgCode = msg;
        lpPingMsg->WhoIAm = WhoIAm;
        lpPingMsg->ToYou = Type;
        lpPingMsg->Time = PingRequestTime;
		nBytes = sizeof( PINGMSG );
		send_to = (DWORD) Type;
		break;


    case MSG_ACKMSG:

		lpAckMsg = (LPACKMSG)&CommBuff[0];
        lpAckMsg->MsgCode = msg;
        lpAckMsg->WhoIAm = WhoIAm;
        lpAckMsg->ID = (uint32) to;
		lpAckMsg->AckTo = ShipNum;
		nBytes = sizeof( ACKMSG );
		break;


#ifdef MANUAL_SESSIONDESC_PROPAGATE
	case MSG_SESSIONDESC:

		DebugPrintf("about to manually send session desc\n");
		lpSessionDescMsg = ( LPSESSIONDESCMSG )&CommBuff[ 0 ];
        lpSessionDescMsg->MsgCode = msg;
        lpSessionDescMsg->WhoIAm = WhoIAm;
		DPlayGetSessionDesc();

		if ( glpdpSD )
		{
			lpSessionDescMsg->dwUser1 = glpdpSD->dwUser1;
			lpSessionDescMsg->dwUser2 = glpdpSD->dwUser2;
			lpSessionDescMsg->dwUser3 = glpdpSD->dwUser3;
			lpSessionDescMsg->dwUser4 = glpdpSD->dwUser4;
		}
		else
			return;	// no SD to send!!

		nBytes = sizeof( SESSIONDESCMSG );
#ifdef	GUARANTEEDMESSAGES
		AddGuaranteedMessage( nBytes , (void*) &CommBuff[0] , MSG_SESSIONDESC , FALSE , TRUE);
		return;
#endif
		break;
#endif
	}
	
	// only record if message is sent to whole of the group....
	if( RecordDemo )
	{
		if(
		   ( MyGameStatus == STATUS_Normal )
		&& ( !send_to )
		&& ( msg != MSG_ACKMSG )
		&& ( msg != MSG_INIT ) 
		&& ( msg != MSG_HEREIAM ) 
		&& ( msg != MSG_SHORTPICKUP    ) 
		&& ( msg != MSG_SHORTREGENSLOT ) 
		&& ( msg != MSG_SHORTTRIGGER   ) 
		&& ( msg != MSG_SHORTTRIGVAR   ) 
		&& ( msg != MSG_SHORTMINE      ) )
		{
			QueryPerformanceCounter((LARGE_INTEGER *) &TempTime);
			TempTime = TempTime - GameStartedTime;
			Demo_fwrite( &TempTime, sizeof(LONGLONG), 1, DemoFp );
			Demo_fwrite( &nBytes, sizeof(int), 1, DemoFp );
			Demo_fwrite( &dcoID, sizeof(DPID), 1, DemoFp );
			Demo_fwrite( &CommBuff[0], nBytes, 1, DemoFp );
		}
	}

	BytesPerSecSent += nBytes;

	// send the message

	if( UseSendAsync )
		Flags |= DPSEND_ASYNC;

#ifdef DEBUG_ON
	hr = IDirectPlayX_SendEx(
								glpDP,
								dcoID,					// From
								send_to,				// send to
								Flags,					// send flags
								(LPSTR)&CommBuff[0],	// data
								nBytes,					// sizeof data
								dwPriority,				// dwPriority
								dwTimeout,				//
								(LPVOID)msg_to_str(msg),// lpContext = Packet Type
								NULL					// lpdwMsgID
								);
#else
	Flags |= DPSEND_NOSENDCOMPLETEMSG;
	hr = IDirectPlayX_SendEx(
								glpDP,
								dcoID,					// From
								send_to,				// send to
								Flags ,					// send flags
								(LPSTR)&CommBuff[0],	// data
								nBytes,					// sizeof data
								dwPriority,				// dwPriority
								dwTimeout,				//
								NULL,					// lpContext
								NULL					// lpdwMsgID
								);
#endif

	switch ( hr )
	{
	case DP_OK:
	case DPERR_PENDING: // DPSEND_ASYNC so packet went to queue
		break;
	case DPERR_BUSY:
		DebugPrintf( "directplay DPERR_BUSY\n");
		break;
	case DPERR_CONNECTIONLOST:
		DebugPrintf( "directplay DPERR_CONNECTIONLOST\n");
		break;
	case DPERR_INVALIDFLAGS:
		DebugPrintf( "directplay DPERR_INVALIDFLAGS\n");
		break;
	case DPERR_INVALIDPARAMS:
		DebugPrintf( "directplay DPERR_INVALIDPARAMS\n");
		break;
	case DPERR_INVALIDPLAYER:
		DebugPrintf( "directplay DPERR_INVALIDPLAYER\n");
		break;
	case DPERR_INVALIDPRIORITY:
		DebugPrintf( "directplay DPERR_INVALIDPRIORITY\n");
		break; 
	case DPERR_NOTLOGGEDIN:
		DebugPrintf( "directplay DPERR_NOTLOGGEDIN\n");
		break;
	case DPERR_SENDTOOBIG:
		DebugPrintf( "directplay DPERR_SENDTOOBIG\n");
		break;
	case DPERR_UNSUPPORTED:
		DebugPrintf( "directplay DPERR_UNSUPPORTED\n");
		break;
	default:
		DebugPrintf( "directplay unknown send error\n");
	}
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	Procedure	:		Read packet stuff from a file and pass it on..
	Input		:		nothing
	Output		:		nothing
ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void DemoPlayingDplayGameUpdate()
{
    DWORD               nBytes;
	int i;
	size_t	size;

	while( 1 && DemoFp)
	{
		if( DemoTimeSoFar )
		{
			if( DemoTimeSoFar > GameElapsedTime )
			{
			 	if (ferror(DemoFp) || feof(DemoFp) )
				{
					fclose( DemoFp );
					DemoFp = NULL;
					SpecialDestroyGame();
					return;
				}
				return;
			}
		}else{
			size = fread( &DemoTimeSoFar , sizeof(LONGLONG), 1, DemoFp );
			if( size != 1 || ferror(DemoFp) || feof(DemoFp) ) 
			{
				PreDemoEndMyGameStatus = MyGameStatus;

				if( Debug && DS )
					for( i = 0 ; i < 256 ; i++ )
						if( PacketGot[i] )
							DebugPrintf("num %3d quantity %12d size %12d\n", i, PacketGot[i] , PacketSize[i] );
								
				QueryPerformanceCounter((LARGE_INTEGER *) &DemoEndedTime);

				fclose( DemoFp );
				DemoFp = NULL;

				SpecialDestroyGame();

				TimeDiff = DemoEndedTime - DemoStartedTime;
				DemoAvgFps = DemoGameLoops /  ( (float) TimeDiff / (float) Freq );
//				DebugPrintf("Average Demo Framrate %d.%02d\n",   (int)( DemoAvgFps * 100 ) / 100,
//															 	(int)( DemoAvgFps * 100 ) % 100 
//															 	);                       
//				DebugPrintf("In %d Gameloops\n",DemoGameLoops);
			
				
				return;
			}
	
			if( DemoTimeSoFar > GameElapsedTime )
				return;
		}
		
		fread( &nBytes , sizeof(DWORD), 1, DemoFp );
		fread( &from_dcoID , sizeof(DPID), 1, DemoFp );
		fread( &ReceiveCommBuff[0] , nBytes , 1, DemoFp );
		
		// During Demo Playback we dont want to interperate any System messages....
		if ( from_dcoID != DPID_SYSMSG ) EvaluateMessage( nBytes , &ReceiveCommBuff[0] );
		DemoTimeSoFar = 0;

  
	}
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	Procedure	:		Read packet stuff from a file if its a ship update..
						scan ahead to find the next one and write in a
						interpolate msg..
	Input		:		nothing
	Output		:		nothing
ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
void DemoClean( void )
{
    DWORD       nBytes;
	size_t		size;
	fpos_t		Currentpos; 
	LONGLONG	DemoTimeSoFar2;
	DPID		from_dcoID2;
    DWORD       nBytes2;
    LPUPDATEMSG			lpUpdate;
    LPFUPDATEMSG		lpFUpdate;
    LPVERYSHORTUPDATEMSG lpVeryShortUpdate;
    LPVERYSHORTFUPDATEMSG		lpVeryShortFUpdate;
	INTERPOLATEMSG Interpolate;
	VERYSHORTINTERPOLATEMSG VeryShortInterpolate;
	BOOL		IsShortPackets = FALSE;
	int			FoundOne;

	while(1)
	{
		size = fread( &DemoTimeSoFar , sizeof(LONGLONG), 1, DemoFp );
   		// check for end of file...
		if( !size )	return;
		fread( &nBytes , sizeof(DWORD), 1, DemoFp );
		fread( &from_dcoID , sizeof(DPID), 1, DemoFp );
		fread( &CommBuff[0] , nBytes , 1, DemoFp );

#if 0
		// Special model num correction stuff...And Power Level Stuff
		switch( CommBuff[0] )
		{
		case MSG_UPDATE:
			lpUpdate = (LPUPDATEMSG) &CommBuff[0];
			break;
		case MSG_FUPDATE:
			break;
		}
#endif
		
		// write out the message..
		fwrite( &DemoTimeSoFar, sizeof(LONGLONG), 1, DemoFpClean );
		fwrite( &nBytes, sizeof(int), 1, DemoFpClean );
		fwrite( &from_dcoID, sizeof(DPID), 1, DemoFpClean );
		fwrite( &CommBuff[0], nBytes, 1, DemoFpClean );

		if ( ( from_dcoID != DPID_SYSMSG ) && ( CommBuff[0] == MSG_INTERPOLATE ) )
		{
			// Has allready been cleaned...
			return;
		}

		// check if its an update message....
		if ( ( from_dcoID != DPID_SYSMSG ) && ( ( CommBuff[0] == MSG_UPDATE ) ||
												( CommBuff[0] == MSG_FUPDATE ) ||
												( CommBuff[0] == MSG_VERYSHORTUPDATE ) ||
												( CommBuff[0] == MSG_VERYSHORTFUPDATE ) ) )
		{
			// we have one update message we now have to search for another...
			Interpolate.MsgCode = MSG_INTERPOLATE;
			VeryShortInterpolate.MsgCode = MSG_VERYSHORTINTERPOLATE;
			switch( CommBuff[0] )
			{
			case MSG_UPDATE:
				lpUpdate = (LPUPDATEMSG) &CommBuff[0];
				Interpolate.WhoIAm = lpUpdate->WhoIAm;
				break;
			case MSG_FUPDATE:
				lpFUpdate = (LPFUPDATEMSG) &CommBuff[0];
				Interpolate.WhoIAm = lpFUpdate->WhoIAm;
				break;
			case MSG_VERYSHORTUPDATE:
				lpVeryShortUpdate = (LPVERYSHORTUPDATEMSG) &CommBuff[0];
				VeryShortInterpolate.WhoIAm = lpVeryShortUpdate->WhoIAm;
				IsShortPackets = TRUE;
				break;
			case MSG_VERYSHORTFUPDATE:
				lpVeryShortFUpdate = (LPVERYSHORTFUPDATEMSG) &CommBuff[0];
				VeryShortInterpolate.WhoIAm = lpVeryShortFUpdate->WhoIAm;
				IsShortPackets = TRUE;
				break;
			}
			fgetpos( DemoFp , &Currentpos );	// store the current position..so we can go back to it..

			FoundOne = 0;
			while( FoundOne == 0 )
			{
				size = fread( &DemoTimeSoFar2 , sizeof(LONGLONG), 1, DemoFp );
				// check for end of file...
				if( !size )
				{
					FoundOne = -1;
					break;
				}

				fread( &nBytes2 , sizeof(DWORD), 1, DemoFp );
				fread( &from_dcoID2 , sizeof(DPID), 1, DemoFp );
				fread( &CommBuff[0] , nBytes2 , 1, DemoFp );


				if ( ( from_dcoID2 != DPID_SYSMSG ) && ( ( CommBuff[0] == MSG_UPDATE ) ||
														( CommBuff[0] == MSG_FUPDATE ) ||
														( CommBuff[0] == MSG_VERYSHORTUPDATE ) ||
														( CommBuff[0] == MSG_VERYSHORTFUPDATE ) ) )
				{
					switch( CommBuff[0] )
					{
					case MSG_UPDATE:
						lpUpdate =(LPUPDATEMSG) &CommBuff[0];
						if( Interpolate.WhoIAm == lpUpdate->WhoIAm )
						{
							// found next update for this ship
							Interpolate.NextPos = lpUpdate->ShortGlobalShip.Pos;
							Interpolate.NextQuat = lpUpdate->ShortGlobalShip.Quat;
#ifdef	SHORTBANK
							Interpolate.NextBank	= (float) (lpUpdate->ShortGlobalShip.Bank / SHORTBANKMODIFIER);
#else
							Interpolate.NextBank	= lpUpdate->ShortGlobalShip.Bank;
#endif
							Interpolate.NextTime = DemoTimeSoFar2;
							FoundOne = 1;
						}
						break;
					case MSG_FUPDATE:
						lpFUpdate = (LPFUPDATEMSG) &CommBuff[0];
						if( Interpolate.WhoIAm == lpFUpdate->WhoIAm )
						{
							// found next update for this ship
							Interpolate.NextPos = lpFUpdate->ShortGlobalShip.Pos;
							Interpolate.NextQuat = lpFUpdate->ShortGlobalShip.Quat;
#ifdef	SHORTBANK
							Interpolate.NextBank	= (float) (lpFUpdate->ShortGlobalShip.Bank / SHORTBANKMODIFIER);
#else
							Interpolate.NextBank	= lpFUpdate->ShortGlobalShip.Bank;
#endif
							Interpolate.NextTime = DemoTimeSoFar2;
							FoundOne = 1;
						}
						break;
					case MSG_VERYSHORTUPDATE:
						lpVeryShortUpdate =(LPVERYSHORTUPDATEMSG) &CommBuff[0];
						if( VeryShortInterpolate.WhoIAm == lpVeryShortUpdate->WhoIAm )
						{
							// found next update for this ship
							VeryShortInterpolate.NextPos = lpVeryShortUpdate->ShortGlobalShip.Pos;
							VeryShortInterpolate.NextQuat	= lpVeryShortUpdate->ShortGlobalShip.Quat;
							VeryShortInterpolate.NextBank	= lpVeryShortUpdate->ShortGlobalShip.Bank;
							VeryShortInterpolate.NextTime = DemoTimeSoFar2;
							FoundOne = 1;
						}
						break;
					case MSG_VERYSHORTFUPDATE:
						lpVeryShortFUpdate = (LPVERYSHORTFUPDATEMSG) &CommBuff[0];
						if( VeryShortInterpolate.WhoIAm == lpVeryShortFUpdate->WhoIAm )
						{
							// found next update for this ship
							VeryShortInterpolate.NextPos = lpVeryShortFUpdate->ShortGlobalShip.Pos;
							VeryShortInterpolate.NextQuat	= lpVeryShortFUpdate->ShortGlobalShip.Quat;
							VeryShortInterpolate.NextBank	= lpVeryShortFUpdate->ShortGlobalShip.Bank;
							VeryShortInterpolate.NextTime = DemoTimeSoFar2;
							FoundOne = 1;
						}
						break;
					}
				}
			}
			if( FoundOne == 1 )
			{
				if( !IsShortPackets )
				{
					fwrite( &DemoTimeSoFar, sizeof(LONGLONG), 1, DemoFpClean );
					nBytes = sizeof( INTERPOLATEMSG );
					fwrite( &nBytes, sizeof(int), 1, DemoFpClean );
					fwrite( &from_dcoID, sizeof(DPID), 1, DemoFpClean );
					fwrite( &Interpolate, nBytes, 1, DemoFpClean );
				}else{
					fwrite( &DemoTimeSoFar, sizeof(LONGLONG), 1, DemoFpClean );
					nBytes = sizeof( VERYSHORTINTERPOLATEMSG );
					fwrite( &nBytes, sizeof(int), 1, DemoFpClean );
					fwrite( &from_dcoID, sizeof(DPID), 1, DemoFpClean );
					fwrite( &VeryShortInterpolate, nBytes, 1, DemoFpClean );
				}
			}
			// set the position of the file back...
			fsetpos( DemoFp , &Currentpos );
		}
	}
	
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	Procedure	:		Create a regen effect...
	Input		:		uint16 ship....
	Output		:		nothing
ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/


void CreateReGen( uint16 ship )
{
	uint16	Model;

	if( ship == WhoIAm )
	{
		if ( !Random_Range( 4 ) )
			PlaySfxWithTrigger( SFX_PlayerGenerate, SFX_BIKER_GP );
		else
			PlaySfx( SFX_PlayerGenerate, 1.0F );
	}else{
		PlayPannedSfx( SFX_PlayerGenerate, Ships[ ship ].Object.Group , &Ships[ ship ].Object.Pos, 0.0F );
	}

	Model =	FindFreeModel();
	if( Model != (uint16 ) -1 )
	{
		Models[ Model ].OwnerType = OWNER_SHIP;
		Models[ Model ].Owner = ship;
		Models[ Model ].ModelNum = MODEL_Sphere;
		Models[ Model ].Type = MODTYPE_Field;
		Models[ Model ].Flags = MODFLAG_Nothing;
		Models[ Model ].Visible = TRUE;
		Models[ Model ].Pos = Ships[ship].Object.Pos;
		Models[ Model ].Mat = MATRIX_Identity;
		MatrixTranspose( &Models[ Model ].Mat, &Models[ Model ].InvMat );
		Models[ Model ].Func = MODFUNC_Regen;
		Models[ Model ].MaxScale = 2.0F;
		Models[ Model ].Scale = 1.0F;
		Models[ Model ].Group = Ships[ship].Object.Group;
		Models[ Model ].LifeCount = 600.0F;
	}
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	Procedure	:		FindSameLevel....
	Input		:		char * Name
	Output		:		int -1 no level....
ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
int FindSameLevel( char * Name )
{
	int i;
    for (i = 0; i < NumLevels; i++)
	{
		if( _stricmp( (char*) &ShortLevelNames[i][0] , Name ) == 0 ) 
		{
			return i;
		}
    }
	return -1;
}



#define DEMORAMBUFFERSIZE ( 1024 * 1024 * 6 )
char * DemoRamBuffer = NULL;
int		DemoRamBufferCurrentSize = 0;

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	Procedure	:		X_fwrite....Writes out some info to a predetermined place
				:		either to a file or ram.....
	Input		:		const void *buffer, size_t size, size_t count
	Output		:		int -1 no level....
ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
void Demo_fwrite( const void *buffer, size_t size, size_t count , FILE *stream )
{
	char * charpnt;
	char * charpnt2;
	size_t i,e;

	if( !RecordDemo || !DemoFp )
		return;

	if( RecordDemoToRam )
	{
		if( !DemoRamBuffer )
		{
			DemoRamBuffer = (char*) malloc(DEMORAMBUFFERSIZE);
			if( !DemoRamBuffer )
			{
				Msg( "Couldnt Malloc Demo Ram Buffer.\n" );
				RecordDemo = FALSE;
				return;
			}
			DemoRamBufferCurrentSize = 0;
		}
		if( (DemoRamBufferCurrentSize + (size * count) ) > DEMORAMBUFFERSIZE )
			return;
		charpnt = DemoRamBuffer + DemoRamBufferCurrentSize;
		charpnt2 = (char*) buffer;

		for( e = 0 ; e < count ; e++ )
		{
			for( i = 0 ; i < size ; i++ )
			{
				*charpnt++ = *charpnt2++;
			}
		}
		DemoRamBufferCurrentSize += (size * count);
	}else{
		fwrite( buffer, size , count , stream );
	}
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	Procedure	:		StopDemoRecording
	Input		:		char * Name
	Output		:		int -1 no level....
ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
void StopDemoRecording( void )
{

	if( DemoFp && RecordDemoToRam && DemoRamBuffer )
	{
		fwrite( DemoRamBuffer, DemoRamBufferCurrentSize, 1, DemoFp );
		RecordDemoToRam = FALSE;
	}
	if( DemoFp )	// make sure that changing level stop any demo from recording!!!!
	{
		fclose( DemoFp );
		DemoFp = NULL;
		RecordDemo = FALSE;
		PlayDemo = FALSE;
	}
	if( DemoRamBuffer )
	{
		free(DemoRamBuffer);
		DemoRamBuffer = NULL;
	}
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	Procedure	:		Add Guaranteed Message...
	Input		:		int MessageLength, void * Message
	Output		:		BOOL TRUE/FALSE
ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
BOOL AddGuaranteedMessage( int MessageLength , void * Message , BYTE MsgType, BOOL OverideOlderMessage, BOOL AllPlayers )
{
#ifdef	GUARANTEEDMESSAGES
	GUARANTEEDMSGHEADER * GM = NULL;
	GUARANTEEDMSGHEADER * GM2 = NULL;
	GUARANTEEDMSGHEADER * LastGM;
	GUARANTEEDMSG * GMm;
	DWORD send_to = 0;
	LPYOUQUITMSG lpYouQuitMsg;
	uint32 Ack;
	int i;

	// bank if we are playing a demo
	if( PlayDemo )
		return TRUE;

	// only record if message is sent to whole of the group....
	if( RecordDemo )
	{
		if( ( MyGameStatus == STATUS_Normal )
		&& ( MsgType != MSG_INIT ) 
		&& ( MsgType != MSG_HEREIAM ) 
		&& ( MsgType != MSG_SHORTPICKUP    ) 
		&& ( MsgType != MSG_SHORTREGENSLOT ) 
		&& ( MsgType != MSG_SHORTTRIGGER   ) 
		&& ( MsgType != MSG_SHORTTRIGVAR   )  
		&& ( MsgType != MSG_TRACKERINFO    )  
		&& ( MsgType != MSG_SHORTMINE      ) )
		{
			// write time since game started to file
			QueryPerformanceCounter((LARGE_INTEGER *) &TempTime);
			TempTime = TempTime - GameStartedTime;
			Demo_fwrite( &TempTime, sizeof(LONGLONG), 1, DemoFp );
			// write the message length
			Demo_fwrite( &MessageLength, sizeof(int), 1, DemoFp );
			// write dplay id of local player to file
			Demo_fwrite( &dcoID, sizeof(DPID), 1, DemoFp );
			// write the message packet to file
			Demo_fwrite( Message, MessageLength, 1, DemoFp );
		}
	}

	// replace existing packets in queue with new packets if they are the same type
	// this would be good for something like our ship location or health etc...
	if( OverideOlderMessage )
	{
		// set GM2 to the list of GMs (gaurented messages)
		GM2 = GMs;

		// loop over the messages
		while( GM2 )
		{
			// if we found a message of the same type
			if( MsgType == GM2->MsgType )
			{
				// save the old messages to GM
				// later we will overide values in this message instead of recreating a message
				GM = GM2;

				// stop looping
				break;
			}
			// go to the next message
			GM2 = GM2->Next;
		}
	}

	// if we didn't find an older message
	if( !GM )
	{
		// create a new message
		GM = (GUARANTEEDMSGHEADER*) malloc( sizeof(GUARANTEEDMSGHEADER) + MessageLength + sizeof( GUARANTEEDMSG ) -1 );
		if( !GM ) return FALSE;

		// increment number of messages
		GuaranteedMessagesActive++;

		// increment counter of highest number of messages
		if( GuaranteedMessagesActive > GuaranteedMessagesActiveMax )
			GuaranteedMessagesActiveMax = GuaranteedMessagesActive;

		// set first default message to last packet assigned
		// GMs is a dumy global message which points to all other ones based on ->prev ->next
		LastGM = GMs;

		// if there was a last packet we go behind it in order
		if( LastGM )
			LastGM->Prev = GM;

		// last packet is in front of us
		GM->Next = LastGM;

		// nothing behind us yet ;]
		GM->Prev = NULL;

		// set global to us
		GMs = GM;

		// if packet was never assigned save it
		if( !OldestGMs )
			OldestGMs = GM;
	}

	// pointer to the message object
	GMm = (GUARANTEEDMSG*) &GM->Message;

	// copy in the new message data 
	memcpy( &GMm->StartOfMessage, Message, MessageLength );

	// set the message type
	GM->MsgType = MsgType;

	// set the times
	QueryPerformanceCounter((LARGE_INTEGER *) &GM->Time);
	QueryPerformanceCounter((LARGE_INTEGER *) &GM->OverallTime);

	// set id to next id
	GM->ID = GuaranteedMessagesID++;

	// why is this being saved?
	GM->OverideOlderMessage = OverideOlderMessage;

	// flag to send to all players
	GM->AllPlayers = AllPlayers;

	// counting what?
	GM->Count = 0;

	// if the message is a quit message
	if( MsgType == MSG_YOUQUIT )
	{
		lpYouQuitMsg = (LPYOUQUITMSG) Message;

		// 
		Ack = 1 << lpYouQuitMsg->You;

		// no idea what's going on
		GM->OverallTime += Freq * 60 * GuaranteedMessagesOverallTime;

	// not a quit message
	}else{

		// no idea what's going on
		GM->OverallTime += Freq * GuaranteedMessagesOverallTime;

		// if send to all players
		if( AllPlayers )
		{
			// 
			Ack = 0xffffffff;
			Ack &= ~(1<<WhoIAm);

		// to specific player
		}else{
			Ack = 0;
			for( i = 0 ; i < MAX_PLAYERS ; i++ )
				if(
					( 
					  (GameStatus[i]!=STATUS_GetPlayerNum) &&
					  (GameStatus[i]!=STATUS_LeftCrashed)  &&
					  (GameStatus[i]!=STATUS_Left)         &&
					  (GameStatus[i]!=STATUS_Null)
					 ) &&
				    (i!=WhoIAm) 
				  )
					Ack |= 1<<i;
		}
	}
	
	// set the ack bits
	GM->Ack = Ack;

	// set message length
	GM->MessageLength = MessageLength + sizeof( GUARANTEEDMSG ) -1;

	// finished message is in queue
#endif
	return TRUE;
}



/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	Procedure	:		Process Qued Guaranteed Messages...
	Input		:		BOOL True == Flush all of them.....
	Output		:		void
ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
void ProcessGuaranteedMessages( BOOL ReleaseMessages , BOOL IgnoreTime , BOOL SendGuaranteed )
{
#ifdef	GUARANTEEDMESSAGES
	GUARANTEEDMSGHEADER * GM;
	GUARANTEEDMSGHEADER * OldGM;
	GUARANTEEDMSGHEADER * NextGM;
	GUARANTEEDMSGHEADER * PrevGM;
	GUARANTEEDMSG * GMm;
	HRESULT				hr;
    DWORD				send_to = 0;
	DWORD				Flags = 0;
	int i;

	// update flags
	if( SendGuaranteed )
		Flags |= DPSEND_GUARANTEED;

	// get start time
	QueryPerformanceCounter((LARGE_INTEGER *) &TempTime);

//	GM = GMs;
	GM = OldestGMs; // first real packet

	// while we have a packet
	while( GM )
	{
//		OldGM = GM->Next;
		OldGM = GM->Prev;

		// 
		if( !GM->Ack || ( (GM->OverallTime < TempTime) && (GM->Count >= GuaranteedMessagesOverallTime ) ) || !glpDP || PlayDemo || ReleaseMessages )
		{

			// This message is past it.....
			// how does this happen if we just checked for ! ?
			if( GM->Ack )
			{
				for( i = 0 ; i < MAX_PLAYERS ; i++ )
				{
					if( GM->Ack & ( 1 << i ) )
					{
						// set player flag to bad
						BadConnection[i] = TRUE;

						// don't go here in release mode
						if( !ReleaseMessages )

							// if in valid game still and player is not me
							if(
								( 
									(GameStatus[i]!=STATUS_GetPlayerNum) &&
									(GameStatus[i]!=STATUS_LeftCrashed)  &&
									(GameStatus[i]!=STATUS_Left)         &&
									(GameStatus[i]!=STATUS_Null) 
								) &&
								(i!=WhoIAm)
							)

								// bad bad bad
								DebugPrintf( "Legal %x Player didnt ack a Guaranteed %x Message\n" , i, GM->MsgType );
					}
				}
			}

			// pluck us out of the chain and link prev<->next together
			NextGM = GM->Next;
			PrevGM = GM->Prev;
			if( NextGM ) NextGM->Prev = PrevGM;
			if( PrevGM ) PrevGM->Next = NextGM;

			// reset gms to next
			if( GM == GMs ) GMs = NextGM;

			// reset oldest to next
			if( GM == OldestGMs )
				OldestGMs = PrevGM;

			// kill the packet
			free( GM );

			// lower count
			GuaranteedMessagesActive--;

		// message is still good to be resent!
		}else{

			// stupid
			if( GM->Time < TempTime || IgnoreTime )
			{
				//Time to re-send....
				GM->Time = TempTime + (GuaranteedMessagesTime*Freq);

				// pointer to message object
				GMm = (GUARANTEEDMSG*) &GM->Message;

				// set settings
				GMm->MsgCode = MSG_GUARANTEEDMSG;
				GMm->WhoIAm = WhoIAm;
				GMm->Ack = GM->Ack;		// Tell Which players this message applies to...
				GMm->ID = GM->ID;		// Tell Them the ID of the message...
				GMm->AllPlayers = GM->AllPlayers;
				GMm->OverideOlderMessage = GM->OverideOlderMessage;

				// up resend count
				GMm->Count = GM->Count++;

				// send and wait for success/error
				if( !UseSendAsync || SendGuaranteed )
				{	
					hr = glpDP->lpVtbl->Send( glpDP,
											  dcoID,   // From
											  send_to, // send to everybody
											  Flags ,
											  &GM->Message,
											  GM->MessageLength);

				// just send it and return right away so we can continue working
				}else{
					Flags |= DPSEND_ASYNC | DPSEND_NOSENDCOMPLETEMSG;
					hr = IDirectPlayX_SendEx( glpDP,
											  dcoID,   // From
											  send_to, // send to
											  Flags ,
											  &GM->Message,
											  GM->MessageLength,
											  0,		// dwPriority
											  0,		// dwTimeout
											  NULL,		// lpContext
											  NULL		// lpdwMsgID
											  );
				}

				// failed to send packet
				if( hr != DP_OK && hr != DPERR_PENDING )
					OutputDebugString( "Dplay Send Error" );

			}
		}

		// go to next message
		GM = OldGM;
	}
#endif
}
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	Procedure	:		Remove 1 player from a qued Guaranteed message...
	Input		:		uint32 ID, uint32 Player
	Output		:		BOOL TRUE/FALSE
ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
void AcknowledgeMessage( uint32 ID , uint32 Player , BYTE PlayerNum )
{
#ifdef	GUARANTEEDMESSAGES
	GUARANTEEDMSGHEADER * GM;
	GM = GMs;
	while( GM )
	{
		if( ID == GM->ID )
		{
			GM->Ack &= ~Player;
			BadConnection[PlayerNum] = FALSE;
			return;
		}
		GM = GM->Next;
	}
#endif
}


#ifdef	GUARANTEEDMESSAGES
#define	MAXACKGUARANTEEDMSGS 4096
typedef struct ACKGUARANTEEDMSGHEADER
{
	uint32		ID;
struct ACKGUARANTEEDMSGHEADER * Next;
	LONGLONG	Time;
}ACKGUARANTEEDMSGHEADER, *LPACKGUARANTEEDMSGHEADER;

ACKGUARANTEEDMSGHEADER AckMsgs[MAXACKGUARANTEEDMSGS];
ACKGUARANTEEDMSGHEADER * FirstFreeAckMsg = NULL;
ACKGUARANTEEDMSGHEADER * FirstUsedAckMsg[MAX_PLAYERS];
int AckMsgsActive = 0;
int AckMsgsActiveMax = 0;
#endif

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	Procedure	:		InitAckMsgQue
	Input		:		void
	Output		:		void
ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
void InitAcknowledgeMessageQue( void )
{
#ifdef	GUARANTEEDMESSAGES
	int i;
	for( i = 0 ; i < MAXACKGUARANTEEDMSGS ; i++ )
	{
		AckMsgs[i].Time = 0;
		AckMsgs[i].ID = 0;
		if( i < MAXACKGUARANTEEDMSGS-1 )
		{
			AckMsgs[i].Next = &AckMsgs[i+1];
		}else{
			AckMsgs[i].Next = NULL;
		}
	}
	FirstFreeAckMsg = &AckMsgs[0];
	for( i = 0 ; i < MAX_PLAYERS ; i++ )
	{
		FirstUsedAckMsg[i] = NULL;
	}
	AckMsgsActive = 0;
	AckMsgsActiveMax = 0;

#endif
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	Procedure	:		Free All players AckMsgQue
	Input		:		void
	Output		:		void
ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
void FreeAllPlayersAcknowledgeMessageQue( BYTE Player )
{
#ifdef	GUARANTEEDMESSAGES
	ACKGUARANTEEDMSGHEADER * AckMsg;
	ACKGUARANTEEDMSGHEADER * NextAckMsg;
	AckMsg = FirstUsedAckMsg[Player];
	while( AckMsg )
	{
		AckMsgsActive--;

		NextAckMsg = AckMsg->Next;
		AckMsg->Next = FirstFreeAckMsg;
		FirstFreeAckMsg = AckMsg;
		AckMsg = NextAckMsg;
	}
	FirstUsedAckMsg[Player] = NULL;
#endif
}
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	Procedure	:		Free All players AckMsgQue Based on time..
	Input		:		void
	Output		:		void
ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
void FreeTimedOutAllPlayersAcknowledgeMessageQue( BYTE Player , LONGLONG Time )
{
#ifdef	GUARANTEEDMESSAGES
	ACKGUARANTEEDMSGHEADER * AckMsg;
	ACKGUARANTEEDMSGHEADER * NextAckMsg;
	ACKGUARANTEEDMSGHEADER * LastAckMsg = NULL;
	AckMsg = FirstUsedAckMsg[Player];
	while( AckMsg )
	{
		NextAckMsg = AckMsg->Next;
		if( Time > AckMsg->Time )
		{
			// We found 1 that is out of time all the remaining will be out of time...
			break;
		}
		LastAckMsg = AckMsg;
		AckMsg = NextAckMsg;
	}

	if( AckMsg && LastAckMsg )
		// the last legal one doesnt have any more to follow...
		LastAckMsg->Next = NULL;
	
	while( AckMsg )
	{
		NextAckMsg = AckMsg->Next;
		AckMsgsActive--;
		AckMsg->Next = FirstFreeAckMsg;
		FirstFreeAckMsg = AckMsg;
		if( AckMsg == FirstUsedAckMsg[Player] )
		{
			FirstUsedAckMsg[Player] = NextAckMsg;
		}
		AckMsg = NextAckMsg;
	}


#endif
}
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	Procedure	:		ProcessAckMsgQue
	Input		:		void
	Output		:		void
ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
void ProcessAcknowledgeMessageQue( void )
{
#ifdef	GUARANTEEDMESSAGES
	BYTE i;

	for( i = 0 ; i < MAX_PLAYERS ; i++ )
	{
		if( PlayDemo )
		{
			FreeTimedOutAllPlayersAcknowledgeMessageQue( i , GameElapsedTime );
		}else{
			QueryPerformanceCounter((LARGE_INTEGER *) &TempTime);
			FreeTimedOutAllPlayersAcknowledgeMessageQue( i , TempTime );
		}
	}
#endif
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	Procedure	:		Find Free players AckMsgQue
	Input		:		BYTE Player
	Output		:		ACKGUARANTEEDMSGHEADER *
ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
ACKGUARANTEEDMSGHEADER * FindFreeAcknowledgeMessageQue( BYTE Player )
{
#ifdef	GUARANTEEDMESSAGES
	ACKGUARANTEEDMSGHEADER * AckMsg;
	AckMsg = FirstFreeAckMsg;
	if( AckMsg )
	{
		FirstFreeAckMsg = AckMsg->Next;
		AckMsg->Next = FirstUsedAckMsg[Player];
		FirstUsedAckMsg[Player] = AckMsg;
		AckMsgsActive++;
	}
	return AckMsg;
#endif
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	Procedure	:		Compare AckMsgQue
	Input		:		BYTE Player , uint32 ID
	Output		:		void
ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
BOOL CompareAcknowledgeMessageQue( BYTE Player , uint32 ID)
{
#ifdef	GUARANTEEDMESSAGES
	ACKGUARANTEEDMSGHEADER * AckMsg;
	AckMsg = FirstUsedAckMsg[Player];
	while( AckMsg )
	{
		if( AckMsg->ID == ID )
			return TRUE;
		AckMsg = AckMsg->Next;
	}
#endif
	return FALSE;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	Procedure	:		Add a Message To players AckMsgQue
	Input		:		BYTE Player
	Output		:		BOOL FALSE if we have got this message before or the Que is full...
ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
BOOL AddAcknowledgeMessageQue( BYTE Player , uint32 ID )
{
#ifdef	GUARANTEEDMESSAGES
	ACKGUARANTEEDMSGHEADER * AckMsg;

	if( CompareAcknowledgeMessageQue( Player , ID) )
		return FALSE;

	AckMsg = FindFreeAcknowledgeMessageQue( Player );

	if( AckMsg )
	{
		AckMsg->ID = ID;
		if( PlayDemo )
		{
			AckMsg->Time = GameElapsedTime + ( ((LONGLONG)(GuaranteedMessagesOverallTime) * 2) * Freq );
		}else{
			QueryPerformanceCounter((LARGE_INTEGER *) &TempTime);
			AckMsg->Time = TempTime + ( ((LONGLONG)(GuaranteedMessagesOverallTime) * 2) * Freq );
		}
		if( AckMsgsActive > AckMsgsActiveMax )
		{
			AckMsgsActiveMax = AckMsgsActive;
			if( AckMsgsActiveMax >= (MAXACKGUARANTEEDMSGS *0.95F) )
			{
	   			AddColourMessageToQue(SystemMessageColour, "Ninety Five Percent of the AckMsgQue is full!" );
			}
		}
		return TRUE;
	}
#endif
	return FALSE;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	Procedure	:		Build Ship Flags...
	Input		:		BYTE Player
	Output		:		uint32 Flags
ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
uint32 BuildShipFlags( BYTE Player )
{
	uint32 Flags;

	Flags = Ships[Player].Object.Flags;    // Flags

	Flags &= ~( SHIP_IsHost + SHIP_Enable + SHIP_Invul + SHIP_PowerLevelBit1+SHIP_PowerLevelBit2 + SHIP_ModeBit1 + SHIP_ModeBit2 +
				SHIP_Shield + SHIP_HullBit1+SHIP_HullBit2 + SHIP_NumMultiplesBit1 + SHIP_NumMultiplesBit2 + SHIP_NumMultiplesBit3 + SHIP_NumMultiplesBit4 );
	if( IsHost ) Flags |= SHIP_IsHost;
	if( Ships[ Player ].enable ) Flags |= SHIP_Enable;
	if( Ships[Player].InvulTimer ) Flags |= SHIP_Invul;
	Flags |= (Ships[Player].Object.PowerLevel&3) << SHIP_PowerLevel_Bit1;
	Flags |= (Ships[Player].Object.Mode&3) << SHIP_Mode_Bit1;
	if( Ships[Player].Object.Shield > 0.0F )
	{
		Flags |= SHIP_Shield;
	}
	if( Ships[Player].Object.Hull > 64.0F )
	{
		Flags |= SHIP_HullBit1+SHIP_HullBit2;
	}else{
		Flags |= ( (uint32) ( Ships[Player].Object.Hull * (1.0F/16.0F) ) ) << SHIP_Hull_Bit1;
	}
	Flags |= (Ships[ Player ].NumMultiples&15) << SHIP_NumMultiples_Bit1;
	return Flags;
}



/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	Procedure	:		Build Ship Flags...
	Input		:		BYTE Player
	Output		:		uint32 Flags
ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
void UnPackShipFlags( BYTE Player , uint32 Flags )
{
	Ships[ Player ].Object.Flags = Flags;
	if( ( Flags & SHIP_Enable ) ) Ships[ Player ].enable = 1;
	else Ships[ Player ].enable = 0;
	if( ( Flags & SHIP_Invul ) ) Ships[ Player ].Invul = TRUE;
	else Ships[ Player ].Invul = FALSE;

	Ships[Player].Object.PowerLevel = (int16)((Flags >> SHIP_PowerLevel_Bit1 ) & 3);
	Ships[Player].Object.Mode = (BYTE)((Flags >> SHIP_Mode_Bit1 ) & 3);

	if( ( Flags & SHIP_Shield ) ) Ships[ Player ].Object.Shield = 1.0F;
	else Ships[ Player ].Object.Shield = 0.0F;
 	Ships[Player].Object.Hull = ( ( (Flags>>SHIP_Hull_Bit1) & 3 ) + 1 ) * 16.0F;

	Ships[Player].NumMultiples = (BYTE)((Flags >> SHIP_NumMultiples_Bit1 ) & 15);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	Procedure	:		Set Ship Bank and Mat..
	Input		:		OBJECT *
	Output		:		void
ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
void SetShipBankAndMat( OBJECT * ShipObjPnt )
{
	QUAT	StepQuat;
	MakeQuat( 0.0F , 0.0F , ShipObjPnt->Bank, &StepQuat );
	QuatMultiply(  &ShipObjPnt->Quat , &StepQuat , &ShipObjPnt->FinalQuat );
	QuatToMatrix( &ShipObjPnt->FinalQuat, &ShipObjPnt->FinalMat );
	MatrixTranspose( &ShipObjPnt->FinalMat, &ShipObjPnt->FinalInvMat );
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	Procedure	:		Update Ammo And Validate Message...
	Input		:		void	*	Message
	Output		:		BOOL		True/False ( Valid Message )
ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
BOOL UpdateAmmoAndValidateMessage( void * Message )
{
	BYTE					Weapon;
	BYTE					TempWeapon;
	BYTE					Player;
	BYTE		*			MsgPnt;
	BYTE					MsgType;
    LPDROPPICKUPMSG			lpDropPickup;
    LPVERYSHORTDROPPICKUPMSG lpVeryShortDropPickup;
    LPFUPDATEMSG			lpFUpdate;
    LPVERYSHORTFUPDATEMSG	lpVeryShortFUpdate;
    LPGROUPONLY_VERYSHORTFUPDATEMSG	lpGroupOnly_VeryShortFUpdate;
    LPPRIMBULLPOSDIRMSG		lpPrimBullPosDir;
    LPSECBULLPOSDIRMSG		lpSecBullPosDir;
	LPGUARANTEEDMSG			lpGuaranteedMsg;
	int16					PowerLevel;
	float					Ammo;
	int16					Count, Count2;
	float					TempFloat;
	BYTE					NumMultiples;

	MsgPnt = (BYTE *) Message;
	MsgType = MsgPnt[ 0 ];
	Player = MsgPnt[ 1 ];

	PowerLevel = Host_PowerLevel[ Player ]; //Ships[ Player ].Object.PowerLevel;

	switch( MsgType )
    {
		case MSG_DROPPICKUP:
			lpDropPickup = (LPDROPPICKUPMSG)MsgPnt;

			switch( lpDropPickup->PickupInfo.Type )
			{
				case PICKUP_GeneralAmmo:
					Host_GeneralAmmo[ lpDropPickup->WhoIAm ] -= GENERALAMMOPERPOD;
					if( Host_GeneralAmmo[ lpDropPickup->WhoIAm ] < 0.0F ) Host_GeneralAmmo[ lpDropPickup->WhoIAm ] = 0.0F;
					break;

				case PICKUP_PyroliteAmmo:
					Host_PyroliteAmmo[ lpDropPickup->WhoIAm ] -= PYROLITEAMMOPERPOD;
					if( Host_PyroliteAmmo[ lpDropPickup->WhoIAm ] < 0.0F ) Host_PyroliteAmmo[ lpDropPickup->WhoIAm ] = 0.0F;
					break;

				case PICKUP_SussGunAmmo:
					Host_SussGunAmmo[ lpDropPickup->WhoIAm ] -= SUSSGUNAMMOPERPOD;
					if( Host_SussGunAmmo[ lpDropPickup->WhoIAm ] < 0.0F ) Host_SussGunAmmo[ lpDropPickup->WhoIAm ] = 0.0F;
					break;

				case PICKUP_Trojax: 
				case PICKUP_Pyrolite:
				case PICKUP_Transpulse:
				case PICKUP_SussGun:
				case PICKUP_Laser:
					Host_PrimaryWeaponsGot[ Player ][ ( lpDropPickup->PickupInfo.Type - PICKUP_Trojax ) + 1 ] = 0;
					break;

				case PICKUP_Mugs:
				case PICKUP_HeatseakerPickup:
				case PICKUP_Thief:
				case PICKUP_Scatter:
				case PICKUP_Gravgon:
				case PICKUP_Launcher:
				case PICKUP_TitanStar:
				case PICKUP_PurgePickup:
				case PICKUP_PinePickup:
				case PICKUP_QuantumPickup:
				case PICKUP_SpiderPod:
				case PICKUP_Parasite:
					for( Count = 0; Count < MAXSECONDARYWEAPONS; Count++ )
					{
						if( SecondaryFromPickupTab[ ( Count * 2 ) ] == lpDropPickup->PickupInfo.Type )
						{
							Weapon = (BYTE) Count;
							break;
						}
					}

					if( Host_SecondaryAmmo[ Player ][ Weapon ] > SecondaryFromPickupTab[ ( Weapon * 2 ) + 1 ] )
						Host_SecondaryAmmo[ Player ][ Weapon ] -= SecondaryFromPickupTab[ ( Weapon * 2 ) + 1 ];
					else
					{
						Host_SecondaryAmmo[ Player ][ Weapon ] = 0;
						Host_SecondaryWeaponsGot[ Player ][ Weapon ] = 0;
					}
					break;
				default:
					break;
			}
			break;


		case MSG_VERYSHORTDROPPICKUP:
			lpVeryShortDropPickup = (LPVERYSHORTDROPPICKUPMSG)MsgPnt;

			switch( lpVeryShortDropPickup->PickupInfo.Type )
			{
				case PICKUP_GeneralAmmo:
					Host_GeneralAmmo[ lpVeryShortDropPickup->WhoIAm ] -= GENERALAMMOPERPOD;
					if( Host_GeneralAmmo[ lpVeryShortDropPickup->WhoIAm ] < 0.0F ) Host_GeneralAmmo[ lpVeryShortDropPickup->WhoIAm ] = 0.0F;
					break;

				case PICKUP_PyroliteAmmo:
					Host_PyroliteAmmo[ lpVeryShortDropPickup->WhoIAm ] -= PYROLITEAMMOPERPOD;
					if( Host_PyroliteAmmo[ lpVeryShortDropPickup->WhoIAm ] < 0.0F ) Host_PyroliteAmmo[ lpVeryShortDropPickup->WhoIAm ] = 0.0F;
					break;

				case PICKUP_SussGunAmmo:
					Host_SussGunAmmo[ lpVeryShortDropPickup->WhoIAm ] -= SUSSGUNAMMOPERPOD;
					if( Host_SussGunAmmo[ lpVeryShortDropPickup->WhoIAm ] < 0.0F ) Host_SussGunAmmo[ lpVeryShortDropPickup->WhoIAm ] = 0.0F;
					break;

				case PICKUP_Trojax: 
				case PICKUP_Pyrolite:
				case PICKUP_Transpulse:
				case PICKUP_SussGun:
				case PICKUP_Laser:
					Host_PrimaryWeaponsGot[ Player ][ ( lpVeryShortDropPickup->PickupInfo.Type - PICKUP_Trojax ) + 1 ] = 0;
					break;

				case PICKUP_Mugs:
				case PICKUP_HeatseakerPickup:
				case PICKUP_Thief:
				case PICKUP_Scatter:
				case PICKUP_Gravgon:
				case PICKUP_Launcher:
				case PICKUP_TitanStar:
				case PICKUP_PurgePickup:
				case PICKUP_PinePickup:
				case PICKUP_QuantumPickup:
				case PICKUP_SpiderPod:
				case PICKUP_Parasite:
					for( Count = 0; Count < MAXSECONDARYWEAPONS; Count++ )
					{
						if( SecondaryFromPickupTab[ ( Count * 2 ) ] == lpVeryShortDropPickup->PickupInfo.Type )
						{
							Weapon = (BYTE) Count;
							break;
						}
					}

					if( Host_SecondaryAmmo[ Player ][ Weapon ] > SecondaryFromPickupTab[ ( Weapon * 2 ) + 1 ] )
					{
						Host_SecondaryAmmo[ Player ][ Weapon ] -= SecondaryFromPickupTab[ ( Weapon * 2 ) + 1 ];
					}
					else
					{
						Host_SecondaryAmmo[ Player ][ Weapon ] = 0;
						Host_SecondaryWeaponsGot[ Player ][ Weapon ] = 0;
					}
					break;

				default:
					break;
			}
			break;


		case MSG_VERYSHORTFUPDATE:
			lpVeryShortFUpdate = (LPVERYSHORTFUPDATEMSG)MsgPnt;

			if( ( Ships[ Player ].SuperNashramTimer == 0.0F ) )
			{
				if( ( Ships[lpVeryShortFUpdate->WhoIAm].Object.Flags & SHIP_Multiple ) )
				{
					NumMultiples = (BYTE) ((lpVeryShortFUpdate->ShortGlobalShip.Flags >> SHIP_NumMultiples_Bit1 ) & 15);
				}
				else
				{
					NumMultiples = 0;
				}

				if( NumMultiples < Host_NumOfOrbitals[ Player ] )
				{
					if( Host_NumOfOrbitals[ Player ] > 1 )
					{
						for( Count = 0; Count < ( Host_NumOfOrbitals[ Player ] - 1 ); Count++ )
						{
							for( Count2 = ( Count + 1 ); Count2 < Host_NumOfOrbitals[ Player ]; Count2++ )
							{
								if( Host_OrbAmmo[ Player ][ Count ] < Host_OrbAmmo[ Player ][ Count2 ] )
								{
									TempFloat = Host_OrbAmmo[ Player ][ Count2 ];
									Host_OrbAmmo[ Player ][ Count2 ] = Host_OrbAmmo[ Player ][ Count ];
									Host_OrbAmmo[ Player ][ Count ] = TempFloat;
								}
							}
						}
					}

					NumOrbs += ( Host_NumOfOrbitals[ Player ] - NumMultiples );
					Host_NumOfOrbitals[ Player ] = NumMultiples;
				}
			}

			if( ( lpVeryShortFUpdate->ShortGlobalShip.Flags & SHIP_MulFire ) )
			{
				for( Count = 0; Count < Host_NumOfOrbitals[ Player ]; Count++ )
				{
//					if( !( Ships[ Player ].Object.Flags & SHIP_SuperNashram ) )
					if( Ships[ Player ].SuperNashramTimer == 0.0F )
					{
						Host_OrbAmmo[ Player ][ Count ] -= (float) PrimaryWeaponAttribs[ ORBITPULSAR ].AmmoUsage[ PowerLevel ];

						if( Host_OrbAmmo[ Player ][ Count ] < 0.0F )
						{
							if( Host_NumOfOrbitals[ Player ] )
							{
								Host_NumOfOrbitals[ Player ]--;
								Host_OrbAmmo[ Player ][ Count ] = Host_OrbAmmo[ Player ][ Host_NumOfOrbitals[ Player ] ]; 
								NumOrbs++;
							}
						}
					}
				}
			}

			if( ( lpVeryShortFUpdate->ShortGlobalShip.Flags & SHIP_SecFire ) )
			{
				Weapon = lpVeryShortFUpdate->ShortGlobalShip.Secondary;
				TempWeapon = ConvSecToNormWeapon( Weapon );

//				AddMessageToQue( "Update:FireSecondary : %s, Got = %d, Ammo = %d", SecondaryDescription[ TempWeapon ],
//																			Host_SecondaryWeaponsGot[ Player ][ TempWeapon ],
//																			Host_SecondaryAmmo[ Player ][ TempWeapon ] );

				if( !Host_SecondaryWeaponsGot[ Player ][ TempWeapon ] ) return( FALSE );
				if( !Host_SecondaryAmmo[ Player ][ TempWeapon ] ) return( FALSE );

				switch( Weapon )
				{
					case MUGMISSILE:
					case SOLARISMISSILE:
					case THIEFMISSILE:
					case SCATTERMISSILE:
					case GRAVGONMISSILE:
					case MULTIPLEMISSILE:
					case TITANSTARMISSILE:
					case PINEMISSILE:
					case TITANSTARSHRAPNEL:
					case ENEMYSPIRALMISSILE:
					case ENEMYHOMINGMISSILE:
					case ENEMYBLUEHOMINGMISSILE:
					case ENEMYFIREBALL:
					case ENEMYTENTACLE:
					case ENEMYDEPTHCHARGE:
						Host_SecAmmoUsed[ TempWeapon ]++;
						if( Host_SecondaryAmmo[ Player ][ TempWeapon ] ) Host_SecondaryAmmo[ Player ][ TempWeapon ]--;
						if( !Host_SecondaryAmmo[ Player ][ TempWeapon ] ) Host_SecondaryWeaponsGot[ Player ][ TempWeapon ] = 0;
						break;

					case PURGEMINE:
					case PINEMINE:
					case QUANTUMMINE:
					case SPIDERMINE:
						if( Host_SecondaryAmmo[ Player ][ TempWeapon ] ) Host_SecondaryAmmo[ Player ][ TempWeapon ]--;
						if( !Host_SecondaryAmmo[ Player ][ TempWeapon ] ) Host_SecondaryWeaponsGot[ Player ][ TempWeapon ] = 0;
						break;

					default:
						break;
				}
			}

			if( lpVeryShortFUpdate->ShortGlobalShip.Flags & SHIP_PrimFire )
			{
				Weapon = lpVeryShortFUpdate->ShortGlobalShip.Primary;

				if( Weapon >= MAXPRIMARYWEAPONS ) break;
				if( !Host_PrimaryWeaponsGot[ Player ][ Weapon ] ) return( FALSE );
				Ammo = GetPrimaryAmmoUsage( Player, Weapon );

				switch( Weapon )
				{
					case PULSAR:
					case TRANSPULSE_CANNON:
					case LASER:
					case NME_BULLET1:
					case NME_PULSAR:
					case NME_TRANSPULSE:
					case NME_LASER:
					case NME_LIGHTNING:
					case FLAMES:
					case NME_POWERLASER:
						if( Ships[ Player ].SuperNashramTimer == 0.0F )
						{
							if( Host_GeneralAmmo[ Player ] == 0.0F ) return( FALSE );
							Host_GeneralAmmoUsed += Ammo;
							Host_GeneralAmmo[ Player ] -= Ammo;
							if( Host_GeneralAmmo[ Player ] < 0.0F ) Host_GeneralAmmo[ Player ] = 0.0F;
						}

//						AddMessageToQue( "Update:FirePrimary : %s, Got = %hd, Ammo = %f", PrimaryDescription[ Weapon ],
//																	Host_PrimaryWeaponsGot[ Player ][ Weapon ],
//																	Host_GeneralAmmo[ Player ] );
						break;

					case TROJAX:
					case NME_TROJAX:
						if( Ships[ Player ].SuperNashramTimer == 0.0F )
						{
							if( Host_GeneralAmmo[ Player ] == 0.0F ) return( FALSE );
						}

//						AddMessageToQue( "Update:FirePrimary : %s, Got = %hd, Ammo = %f", PrimaryDescription[ Weapon ],
//																	Host_PrimaryWeaponsGot[ Player ][ Weapon ],
//																	Host_GeneralAmmo[ Player ] );
						break;

					case PYROLITE_RIFLE:
					case NME_PYROLITE:
						if( Ships[ Player ].SuperNashramTimer == 0.0F )
						{
							if( Host_PyroliteAmmo[ Player ] == 0.0F ) return( FALSE );
							Host_PyroliteAmmoUsed += Ammo;
							Host_PyroliteAmmo[ Player ] -= Ammo;
							if( Host_PyroliteAmmo[ Player ] < 0.0F ) Host_PyroliteAmmo[ Player ] = 0.0F;
						}

//						AddMessageToQue( "Update:FirePrimary : %s, Got = %hd, Ammo = %f", PrimaryDescription[ Weapon ],
//																	Host_PrimaryWeaponsGot[ Player ][ Weapon ],
//																	Host_PyroliteAmmo[ Player ] );
						break;

					case SUSS_GUN:
					case NME_SUSS_GUN:
						if( Ships[ Player ].SuperNashramTimer == 0.0F )
						{
							if( Host_SussGunAmmo[ Player ] == 0.0F ) return( FALSE );
							Host_SussGunAmmoUsed += Ammo;
							Host_SussGunAmmo[ Player ] -= Ammo;
							if( Host_SussGunAmmo[ Player ] < 0.0F ) Host_SussGunAmmo[ Player ] = 0.0F;
						}

//						AddMessageToQue( "Update:FirePrimary : %s, Got = %hd, Ammo = %f", PrimaryDescription[ Weapon ],
//																	Host_PrimaryWeaponsGot[ Player ][ Weapon ],
//																	Host_SussGunAmmo[ Player ] );
						break;

					case ORBITPULSAR:
						break;

					default:
						break;
				}
			}
			break;

		case MSG_GROUPONLY_VERYSHORTFUPDATE:
			lpGroupOnly_VeryShortFUpdate = (LPGROUPONLY_VERYSHORTFUPDATEMSG)MsgPnt;

			if( ( Ships[ Player ].SuperNashramTimer == 0.0F ) )
			{
				if( ( Ships[lpGroupOnly_VeryShortFUpdate->WhoIAm].Object.Flags & SHIP_Multiple ) )
				{
					NumMultiples = (BYTE) ((lpGroupOnly_VeryShortFUpdate->ShortGlobalShip.Flags >> SHIP_NumMultiples_Bit1 ) & 15);
				}
				else
				{
					NumMultiples = 0;
				}

				if( NumMultiples < Host_NumOfOrbitals[ Player ] )
				{
					if( Host_NumOfOrbitals[ Player ] > 1 )
					{
						for( Count = 0; Count < ( Host_NumOfOrbitals[ Player ] - 1 ); Count++ )
						{
							for( Count2 = ( Count + 1 ); Count2 < Host_NumOfOrbitals[ Player ]; Count2++ )
							{
								if( Host_OrbAmmo[ Player ][ Count ] < Host_OrbAmmo[ Player ][ Count2 ] )
								{
									TempFloat = Host_OrbAmmo[ Player ][ Count2 ];
									Host_OrbAmmo[ Player ][ Count2 ] = Host_OrbAmmo[ Player ][ Count ];
									Host_OrbAmmo[ Player ][ Count ] = TempFloat;
								}
							}
						}
					}

					NumOrbs += ( Host_NumOfOrbitals[ Player ] - NumMultiples );
					Host_NumOfOrbitals[ Player ] = NumMultiples;
				}
			}

			if( ( lpGroupOnly_VeryShortFUpdate->ShortGlobalShip.Flags & SHIP_MulFire ) )
			{
				for( Count = 0; Count < Host_NumOfOrbitals[ Player ]; Count++ )
				{
//					if( !( Ships[ Player ].Object.Flags & SHIP_SuperNashram ) )
					if( Ships[ Player ].SuperNashramTimer == 0.0F )
					{
						Host_OrbAmmo[ Player ][ Count ] -= (float) PrimaryWeaponAttribs[ ORBITPULSAR ].AmmoUsage[ PowerLevel ];

						if( Host_OrbAmmo[ Player ][ Count ] < 0.0F )
						{
							if( Host_NumOfOrbitals[ Player ] )
							{
								Host_NumOfOrbitals[ Player ]--;
								Host_OrbAmmo[ Player ][ Count ] = Host_OrbAmmo[ Player ][ Host_NumOfOrbitals[ Player ] ]; 
								NumOrbs++;
							}
						}
					}
				}
			}

			if( ( lpGroupOnly_VeryShortFUpdate->ShortGlobalShip.Flags & SHIP_SecFire ) )
			{
				Weapon = lpGroupOnly_VeryShortFUpdate->ShortGlobalShip.Secondary;
				TempWeapon = ConvSecToNormWeapon( Weapon );

//				AddMessageToQue( "Update:FireSecondary : %s, Got = %d, Ammo = %d", SecondaryDescription[ TempWeapon ],
//																			Host_SecondaryWeaponsGot[ Player ][ TempWeapon ],
//																			Host_SecondaryAmmo[ Player ][ TempWeapon ] );

				if( !Host_SecondaryWeaponsGot[ Player ][ TempWeapon ] ) return( FALSE );
				if( !Host_SecondaryAmmo[ Player ][ TempWeapon ] ) return( FALSE );

				switch( Weapon )
				{
					case MUGMISSILE:
					case SOLARISMISSILE:
					case THIEFMISSILE:
					case SCATTERMISSILE:
					case GRAVGONMISSILE:
					case MULTIPLEMISSILE:
					case TITANSTARMISSILE:
					case PINEMISSILE:
					case TITANSTARSHRAPNEL:
					case ENEMYSPIRALMISSILE:
					case ENEMYHOMINGMISSILE:
					case ENEMYBLUEHOMINGMISSILE:
					case ENEMYFIREBALL:
					case ENEMYTENTACLE:
					case ENEMYDEPTHCHARGE:
						Host_SecAmmoUsed[ TempWeapon ]++;
						if( Host_SecondaryAmmo[ Player ][ TempWeapon ] ) Host_SecondaryAmmo[ Player ][ TempWeapon ]--;
						if( !Host_SecondaryAmmo[ Player ][ TempWeapon ] ) Host_SecondaryWeaponsGot[ Player ][ TempWeapon ] = 0;
						break;

					case PURGEMINE:
					case PINEMINE:
					case QUANTUMMINE:
					case SPIDERMINE:
						if( Host_SecondaryAmmo[ Player ][ TempWeapon ] ) Host_SecondaryAmmo[ Player ][ TempWeapon ]--;
						if( !Host_SecondaryAmmo[ Player ][ TempWeapon ] ) Host_SecondaryWeaponsGot[ Player ][ TempWeapon ] = 0;
						break;

					default:
						break;
				}
			}

			if( lpGroupOnly_VeryShortFUpdate->ShortGlobalShip.Flags & SHIP_PrimFire )
			{
				Weapon = lpGroupOnly_VeryShortFUpdate->ShortGlobalShip.Primary;

				if( Weapon >= MAXPRIMARYWEAPONS ) break;
				if( !Host_PrimaryWeaponsGot[ Player ][ Weapon ] ) return( FALSE );
				Ammo = GetPrimaryAmmoUsage( Player, Weapon );

				switch( Weapon )
				{
					case PULSAR:
					case TRANSPULSE_CANNON:
					case LASER:
					case NME_BULLET1:
					case NME_PULSAR:
					case NME_TRANSPULSE:
					case NME_LASER:
					case NME_LIGHTNING:
					case FLAMES:
					case NME_POWERLASER:
						if( Ships[ Player ].SuperNashramTimer == 0.0F )
						{
							if( Host_GeneralAmmo[ Player ] == 0.0F ) return( FALSE );
							Host_GeneralAmmoUsed += Ammo;
							Host_GeneralAmmo[ Player ] -= Ammo;
							if( Host_GeneralAmmo[ Player ] < 0.0F ) Host_GeneralAmmo[ Player ] = 0.0F;
						}

//						AddMessageToQue( "Update:FirePrimary : %s, Got = %hd, Ammo = %f", PrimaryDescription[ Weapon ],
//																	Host_PrimaryWeaponsGot[ Player ][ Weapon ],
//																	Host_GeneralAmmo[ Player ] );
						break;

					case TROJAX:
					case NME_TROJAX:
						if( Ships[ Player ].SuperNashramTimer == 0.0F )
						{
							if( Host_GeneralAmmo[ Player ] == 0.0F ) return( FALSE );
						}

//						AddMessageToQue( "Update:FirePrimary : %s, Got = %hd, Ammo = %f", PrimaryDescription[ Weapon ],
//																	Host_PrimaryWeaponsGot[ Player ][ Weapon ],
//																	Host_GeneralAmmo[ Player ] );
						break;

					case PYROLITE_RIFLE:
					case NME_PYROLITE:
						if( Ships[ Player ].SuperNashramTimer == 0.0F )
						{
							if( Host_PyroliteAmmo[ Player ] == 0.0F ) return( FALSE );
							Host_PyroliteAmmoUsed += Ammo;
							Host_PyroliteAmmo[ Player ] -= Ammo;
							if( Host_PyroliteAmmo[ Player ] < 0.0F ) Host_PyroliteAmmo[ Player ] = 0.0F;
						}

//						AddMessageToQue( "Update:FirePrimary : %s, Got = %hd, Ammo = %f", PrimaryDescription[ Weapon ],
//																	Host_PrimaryWeaponsGot[ Player ][ Weapon ],
//																	Host_PyroliteAmmo[ Player ] );
						break;

					case SUSS_GUN:
					case NME_SUSS_GUN:
						if( Ships[ Player ].SuperNashramTimer == 0.0F )
						{
							if( Host_SussGunAmmo[ Player ] == 0.0F ) return( FALSE );
							Host_SussGunAmmoUsed += Ammo;
							Host_SussGunAmmo[ Player ] -= Ammo;
							if( Host_SussGunAmmo[ Player ] < 0.0F ) Host_SussGunAmmo[ Player ] = 0.0F;
						}

//						AddMessageToQue( "Update:FirePrimary : %s, Got = %hd, Ammo = %f", PrimaryDescription[ Weapon ],
//																	Host_PrimaryWeaponsGot[ Player ][ Weapon ],
//																	Host_SussGunAmmo[ Player ] );
						break;

					case ORBITPULSAR:
						break;

					default:
						break;
				}
			}
			break;

		case MSG_FUPDATE:
			lpFUpdate = (LPFUPDATEMSG)MsgPnt;

			if( ( Ships[ Player ].SuperNashramTimer == 0.0F ) )
			{
				if( ( Ships[lpFUpdate->WhoIAm].Object.Flags & SHIP_Multiple ) )
				{
					NumMultiples = (BYTE) ((lpFUpdate->ShortGlobalShip.Flags >> SHIP_NumMultiples_Bit1 ) & 15);
				}
				else
				{
					NumMultiples = 0;
				}

				if( NumMultiples < Host_NumOfOrbitals[ Player ] )
				{
					if( Host_NumOfOrbitals[ Player ] > 1 )
					{
						for( Count = 0; Count < ( Host_NumOfOrbitals[ Player ] - 1 ); Count++ )
						{
							for( Count2 = ( Count + 1 ); Count2 < Host_NumOfOrbitals[ Player ]; Count2++ )
							{
								if( Host_OrbAmmo[ Player ][ Count ] < Host_OrbAmmo[ Player ][ Count2 ] )
								{
									TempFloat = Host_OrbAmmo[ Player ][ Count2 ];
									Host_OrbAmmo[ Player ][ Count2 ] = Host_OrbAmmo[ Player ][ Count ];
									Host_OrbAmmo[ Player ][ Count ] = TempFloat;
								}
							}
						}
					}

					NumOrbs += ( Host_NumOfOrbitals[ Player ] - NumMultiples );
					Host_NumOfOrbitals[ Player ] = NumMultiples;
				}
			}

			if( lpFUpdate->ShortGlobalShip.Flags & SHIP_MulFire )
			{
				for( Count = 0; Count < Host_NumOfOrbitals[ Player ]; Count++ )
				{
//					if( !( Ships[ Player ].Object.Flags & SHIP_SuperNashram ) )
					if( ( Ships[ Player ].SuperNashramTimer == 0.0F ) )
					{
						Host_OrbAmmo[ Player ][ Count ] -= (float) PrimaryWeaponAttribs[ ORBITPULSAR ].AmmoUsage[ PowerLevel ];

						if( Host_OrbAmmo[ Player ][ Count ] < 0.0F )
						{
							if( Host_NumOfOrbitals[ Player ] )
							{
								Host_NumOfOrbitals[ Player ]--;
								Host_OrbAmmo[ Player ][ Count ] = Host_OrbAmmo[ Player ][ Host_NumOfOrbitals[ Player ] ]; 
								NumOrbs++;
							}
						}
					}
				}
			}

			if( lpFUpdate->ShortGlobalShip.Flags & SHIP_SecFire )
			{
				Weapon		= lpFUpdate->ShortGlobalShip.Secondary;
				TempWeapon	= ConvSecToNormWeapon( Weapon );

				if( !Host_SecondaryWeaponsGot[ Player ][ TempWeapon ] ) return( FALSE );
				if( !Host_SecondaryAmmo[ Player ][ TempWeapon ] ) return( FALSE );

//				AddMessageToQue( "Update:FireSecondary : %s, Got = %d, Ammo = %d", SecondaryDescription[ TempWeapon ],
//																			Host_SecondaryWeaponsGot[ Player ][ TempWeapon ],
//																			Host_SecondaryAmmo[ Player ][ TempWeapon ] );

				switch( Weapon )
				{
					case MUGMISSILE:
					case SOLARISMISSILE:
					case THIEFMISSILE:
					case SCATTERMISSILE:
					case GRAVGONMISSILE:
					case MULTIPLEMISSILE:
					case TITANSTARMISSILE:
					case PINEMISSILE:
					case TITANSTARSHRAPNEL:
					case ENEMYSPIRALMISSILE:
					case ENEMYHOMINGMISSILE:
					case ENEMYBLUEHOMINGMISSILE:
					case ENEMYFIREBALL:
					case ENEMYTENTACLE:
					case ENEMYDEPTHCHARGE:
						Host_SecAmmoUsed[ TempWeapon ]++;
						Host_SecondaryAmmo[ Player ][ TempWeapon ]--;
						if( !Host_SecondaryAmmo[ Player ][ TempWeapon ] ) Host_SecondaryWeaponsGot[ Player ][ TempWeapon ] = 0;
						break;

					case PURGEMINE:
					case PINEMINE:
					case QUANTUMMINE:
					case SPIDERMINE:
						Host_SecondaryAmmo[ Player ][ TempWeapon ]--;
						if( !Host_SecondaryAmmo[ Player ][ TempWeapon ] ) Host_SecondaryWeaponsGot[ Player ][ TempWeapon ] = 0;
						break;

					default:
						break;
				}
			}

			if( lpFUpdate->ShortGlobalShip.Flags & SHIP_PrimFire )
			{
				Weapon = lpFUpdate->ShortGlobalShip.Primary;

				if( Weapon >= MAXPRIMARYWEAPONS ) break;
				if( !Host_PrimaryWeaponsGot[ Player ][ Weapon ] ) return( FALSE );

				Ammo = GetPrimaryAmmoUsage( Player, Weapon );

				switch( Weapon )
				{
					case PULSAR:
					case TRANSPULSE_CANNON:
					case LASER:
					case NME_BULLET1:
					case NME_PULSAR:
					case NME_TRANSPULSE:
					case NME_LASER:
					case NME_LIGHTNING:
					case FLAMES:
					case NME_POWERLASER:
						if( Ships[ Player ].SuperNashramTimer == 0.0F )
						{
							if( Host_GeneralAmmo[ Player ] == 0.0F ) return( FALSE );
							Host_GeneralAmmoUsed += Ammo;
							Host_GeneralAmmo[ Player ] -= Ammo;
							if( Host_GeneralAmmo[ Player ] < 0.0F ) Host_GeneralAmmo[ Player ] = 0.0F;
						}

//						AddMessageToQue( "Update:FirePrimary : %s, Got = %hd, Ammo = %f", PrimaryDescription[ Weapon ],
//																	Host_PrimaryWeaponsGot[ Player ][ Weapon ],
//																	Host_GeneralAmmo[ Player ] );

						break;

					case TROJAX:
					case NME_TROJAX:
//						AddMessageToQue( "Update:FirePrimary : %s, Got = %hd, Ammo = %f", PrimaryDescription[ Weapon ],
//																	Host_PrimaryWeaponsGot[ Player ][ Weapon ],
//																	Host_GeneralAmmo[ Player ] );
						break;

					case PYROLITE_RIFLE:
					case NME_PYROLITE:
						if( Ships[ Player ].SuperNashramTimer == 0.0F )
						{
							if( Host_PyroliteAmmo[ Player ] == 0.0F ) return( FALSE );
							Host_PyroliteAmmoUsed += Ammo;
							Host_PyroliteAmmo[ Player ] -= Ammo;
							if( Host_PyroliteAmmo[ Player ] < 0.0F ) Host_PyroliteAmmo[ Player ] = 0.0F;
						}
		
//						AddMessageToQue( "Update:FirePrimary : %s, Got = %hd, Ammo = %f", PrimaryDescription[ Weapon ],
//																	Host_PrimaryWeaponsGot[ Player ][ Weapon ],
//																	Host_PyroliteAmmo[ Player ] );
						break;

					case SUSS_GUN:
					case NME_SUSS_GUN:
						if( Ships[ Player ].SuperNashramTimer == 0.0F )
						{
							if( Host_SussGunAmmo[ Player ] == 0.0F ) return( FALSE );
							Host_SussGunAmmoUsed += Ammo;
							Host_SussGunAmmo[ Player ] -= Ammo;
							if( Host_SussGunAmmo[ Player ] < 0.0F ) Host_SussGunAmmo[ Player ] = 0.0F;
						}

//						AddMessageToQue( "Update:FirePrimary : %s, Got = %hd, Ammo = %f", PrimaryDescription[ Weapon ],
//																	Host_PrimaryWeaponsGot[ Player ][ Weapon ],
//																	Host_SussGunAmmo[ Player ] );
						break;

					case ORBITPULSAR:
						break;

					default:
						break;
				}
			}
			break;

		case MSG_GUARANTEEDMSG:
			lpGuaranteedMsg = (LPGUARANTEEDMSG)MsgPnt;

			MsgPnt = &lpGuaranteedMsg->StartOfMessage;
			return( UpdateAmmoAndValidateMessage( MsgPnt ) );
			break;


		case MSG_PRIMBULLPOSDIR:
		    lpPrimBullPosDir = (LPPRIMBULLPOSDIRMSG)MsgPnt;
		
			if( lpPrimBullPosDir->PrimBullPosDir.OwnerType != OWNER_SHIP ) break;

			Weapon = lpPrimBullPosDir->PrimBullPosDir.Weapon; 

//			AddMessageToQue( "PrimBullPosDir : %s, Got = %d, Ammo = %d", PrimaryDescription[ Weapon ],
//																	Host_PrimaryWeaponsGot[ Player ][ Weapon ],
//																	Host_GeneralAmmo[ Player ] );

			switch( Weapon )
			{
				case PULSAR:
				case TRANSPULSE_CANNON:
				case LASER:
				case NME_BULLET1:
				case NME_PULSAR:
				case NME_TRANSPULSE:
				case NME_LASER:
				case NME_LIGHTNING:
				case FLAMES:
				case NME_POWERLASER:
						if( Ships[ Player ].SuperNashramTimer == 0.0F )
					{
						if( Host_GeneralAmmo[ Player ] == 0.0F ) return( FALSE );
						Host_GeneralAmmoUsed += PrimaryWeaponAttribs[ Weapon ].AmmoUsage[ PowerLevel ];
						Host_GeneralAmmo[ Player ] -= PrimaryWeaponAttribs[ Weapon ].AmmoUsage[ PowerLevel ];
						if( Host_GeneralAmmo[ Player ] < 0.0F ) Host_GeneralAmmo[ Player ] = 0.0F;
					}
					break;

				case TROJAX:
				case NME_TROJAX:
					break;

				case PYROLITE_RIFLE:
				case NME_PYROLITE:
					if( Ships[ Player ].SuperNashramTimer == 0.0F )
					{
						if( Host_PyroliteAmmo[ Player ] == 0.0F ) return( FALSE );
						Host_PyroliteAmmoUsed += PrimaryWeaponAttribs[ Weapon ].AmmoUsage[ PowerLevel ];
						Host_PyroliteAmmo[ Player ] -= PrimaryWeaponAttribs[ Weapon ].AmmoUsage[ PowerLevel ];
						if( Host_PyroliteAmmo[ Player ] < 0.0F ) Host_PyroliteAmmo[ Player ] = 0.0F;
					}
					break;

				case SUSS_GUN:
				case NME_SUSS_GUN:
					if( Ships[ Player ].SuperNashramTimer == 0.0F )
					{
						if( Host_SussGunAmmo[ Player ] == 0.0F ) return( FALSE );
						Host_SussGunAmmoUsed += PrimaryWeaponAttribs[ Weapon ].AmmoUsage[ PowerLevel ];
						Host_SussGunAmmo[ Player ] -= PrimaryWeaponAttribs[ Weapon ].AmmoUsage[ PowerLevel ];
						if( Host_SussGunAmmo[ Player ] < 0.0F ) Host_SussGunAmmo[ Player ] = 0.0F;
					}
					break;

				case ORBITPULSAR:
					break;

				default:
					break;
			}
			break;

		case MSG_SECBULLPOSDIR:
   			lpSecBullPosDir = (LPSECBULLPOSDIRMSG)MsgPnt;

			if( lpSecBullPosDir->SecBullPosDir.OwnerType != OWNER_SHIP ) break;

			Weapon		= lpSecBullPosDir->SecBullPosDir.Weapon;
			TempWeapon	= ConvSecToNormWeapon( Weapon );

//			AddMessageToQue( "SecBullPosDir : %s, Got = %d, Ammo = %d", SecondaryDescription[ TempWeapon ],
//																		Host_SecondaryWeaponsGot[ Player ][ TempWeapon ],
//																		Host_SecondaryAmmo[ Player ][ TempWeapon ] );

			switch( Weapon )
			{
				case MUGMISSILE:
				case SOLARISMISSILE:
				case THIEFMISSILE:
				case SCATTERMISSILE:
				case GRAVGONMISSILE:
				case MULTIPLEMISSILE:
				case TITANSTARMISSILE:
				case PINEMISSILE:
				case TITANSTARSHRAPNEL:
				case ENEMYSPIRALMISSILE:
				case ENEMYHOMINGMISSILE:
				case ENEMYBLUEHOMINGMISSILE:
				case ENEMYFIREBALL:
				case ENEMYTENTACLE:
				case ENEMYDEPTHCHARGE:
					break;

				case PURGEMINE:
				case PINEMINE:
				case QUANTUMMINE:
				case SPIDERMINE:
					if( !Host_SecondaryWeaponsGot[ Player ][ TempWeapon ] ) return( FALSE );
					if( !Host_SecondaryAmmo[ Player ][ TempWeapon ] ) return( FALSE );

					Host_SecondaryAmmo[ Player ][ TempWeapon ]--;
					if( !Host_SecondaryAmmo[ Player ][ TempWeapon ] ) Host_SecondaryWeaponsGot[ Player ][ TempWeapon ] = 0;
					break;

				default:
					break;
			}
			break;

		case MSG_TITANBITS:
			break;

		default:
			break;
	}

	return( TRUE );
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	Procedure	:		Check If A Player Is cheating...If he is Dont let him hurt anyone..
	Input		:		BYTE Player
	Output		:		BOOL TRUE/FALSE
ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

BOOL	CheckIfPlayerCheats( BYTE player )
{
	if( ( ( Ships[player].Object.Flags & SHIP_SuperNashram ) && !Ships[player].SuperNashramTimer ) ||
		( ( Ships[player].Object.Flags & SHIP_Invul ) && !Ships[player].InvulTimer ) ||
		( ( Ships[player].Object.Flags & SHIP_Stealth ) && !Ships[player].StealthTime ) )
		return TRUE;
	return FALSE;
}

BOOL AutoJoinSession( void )
{
	DWORD size;
	char session_guidtext[ 256 ];

	if ( !SessionGuidExists )
	{
		size = sizeof( session_guidtext );
		if ( RegGet( "SessionGuid", (LPBYTE)&session_guidtext, &size ) != ERROR_SUCCESS)
		{
			DebugPrintf("unable to open reg key 'SessionGuid' ");
			return FALSE;
		}
		if ( GUIDFromString( session_guidtext, &autojoin_session_guid ) != S_OK )
		{
			DebugPrintf("unable to convert session guid from string\n");
			return FALSE;
		}
	}

	MyGameStatus = STATUS_TryingToFindSession;
	FindSessionTimeout = FIND_SESSION_TIMEOUT;

	return TRUE;
}

#ifdef OPT_ON
#pragma optimize( "", off )
#endif

