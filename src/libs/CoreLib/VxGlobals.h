#pragma once
//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxTime.h" // time stamp and other time functions

class VxGUID;

enum EAppDir
{
	eAppDirUnknown = 0,

	// standard paths set from qt paths
	eAppData,

	eAppDirRootDataStorage,
	eAppDirAppTempData,
	eAppDirAppLogs,

	eAppDirAppNoLimitData,
    eAppDirPlayerNlcData,

	eAppDirRootUserData,

	eAppDirUserSpecific,
	eAppDirSettings,
	eAppDirAboutMePageServer,
	eAppDirAboutMePageClient,
	eAppDirStoryboardPageServer,
	eAppDirStoryBoardPageClient,
	eAppDirRootXfer,
	eAppDirUserXfer,
	eAppDirDownloads,
	eAppDirUploads,
	eAppDirIncomplete,
	eAppDirPersonalRecords,

    eAppDirThumbs,
    eAppDirCamRecord,

	eAppDirFonts,

	eAppDirTranslations,

	eMaxAppDir

};

//============================================================================
void							VxSetAppIsShuttingDown( bool bIsShuttingDown );
bool							VxIsAppShuttingDown( void );

const char*						VxGetCompanyDomain( void );
const char*						VxGetOrginizationName( void );
const char*						VxGetCompanyWebsite( void );

const char*						VxGetApplicationTitle( void );
const char*						VxGetApplicationNameNoSpaces( void );
const char*						VxGetApplicationNameNoSpacesLowerCase( void );

uint16_t						VxGetAppVersionShort( void );
uint32_t						VxGetAppVersionFull( void );
const char*						VxGetAppVersionString( void );

//============================================================================
void                            VxSetAppDirectory( enum EAppDir appDir, std::string setDir );
std::string& 					VxGetAppDirectory( enum EAppDir appDir );

// user writable directories	
void							VxSetRootDataStorageDirectory( const char* rootDataDir );
std::string&					VxGetRootDataStorageDirectory( void );

std::string&					VxGetAppTempDirectory( void );
std::string&					VxGetAppLogsDirectory( void );
std::string&					VxGetAppNoLimitDataDirectory( void );
std::string&					VxGetAppThumbnailDirectory( void );

void							VxSetRootUserDataDirectory( const char* rootUserDataDir );
std::string&					VxGetRootUserDataDirectory( void );

void							VxSetUserSpecificDataDirectory( const char* userDataDir  );
std::string&					VxGetUserSpecificDataDirectory( void  );
std::string&					VxGetSettingsDirectory( void );

std::string&                    VxGetFontDirectory( void );

std::string&					VxGetAboutMePageServerDirectory( void ); 
std::string                     VxGetAboutMePageClientDirectory( VxGUID& onlineId );
std::string&					VxGetStoryBoardPageServerDirectory( void );
std::string                     VxGetStoryBoardPageClientDirectory( VxGUID& onlineId );

std::string&                    VxGetTranslationsDirectory( void );

void							VxSetRootXferDirectory( const char* rootXferDir  );
std::string&					VxGetRootXferDirectory( void ) ;

void							VxSetUserXferDirectory( std::string userXferDir  );
std::string&					VxGetUserXferDirectory( void  );
std::string&					VxGetDownloadsDirectory( void );
std::string&					VxGetUploadsDirectory( void );
std::string&					VxGetIncompleteDirectory( void );
std::string&					VxGetPersonalRecordDirectory( void );

//============================================================================
//=== miscellaneous ===//
//============================================================================
void							VxSetNetworkLoopbackAllowed( bool bIsLoopbackAllowed );
bool							VxIsNetworkLoopbackAllowed( void );

int 							VxGlobalAccessLock( void );
int 							VxGlobalAccessUnlock( void );

// set time format to military 24hr or AM/PM
void							SetUseMilitaryTime( bool useMilitaryTime );
bool							GetUseMilitaryTime( void );

void							VxSetMaxMessageHistory( int16_t maxHistory );
int32_t							VxGetMaxMessageHistory( void );

void							VxSetShowMyselfInLists( bool showMyself ); // NOT recommended.. (for debug only)
bool							VxGetShowMyselfInLists( void );

void							VxSetFastHostAnnounce( bool fastAnnounce ); // NOT recommended.. (for debug only)
bool							VxGetFastHostAnnounce( void );

void							VxSetCanDeleteUserFromDb( bool canDeleteFromDb ); // NOT recommended.. (for debug only)
bool							VxGetCanDeleteUserFromDb( void );
