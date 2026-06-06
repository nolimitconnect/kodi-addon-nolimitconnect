//============================================================================
// Copyright (C) 2012 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "FileShareSettings.h"
#include "EngineSettings.h"
#include "EngineSettingsDefaultValues.h"

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxFileUtil.h>
#include <CoreLib/VxGlobals.h>

namespace
{
	FileShareSettings g_oFileShareSettings; 
}

//============================================================================
FileShareSettings::FileShareSettings()
	: m_u32DownloadBandwidth(0)
	, m_u32MaxDownloadingFiles(10)
	, m_u32UploadBandwidth(0)
	, m_u32MaxUploadingFiles(5)
	, m_bStartServerOnStartup(true)
{ 
}

//============================================================================
FileShareSettings& FileShareSettings::operator=(const FileShareSettings& rhs) 
{	
	if( this != &rhs )
	{
		m_u32DownloadBandwidth		= rhs.m_u32DownloadBandwidth;
		m_u32MaxDownloadingFiles	= rhs.m_u32MaxDownloadingFiles;
		m_u32UploadBandwidth		= rhs.m_u32UploadBandwidth;
		m_u32UploadBandwidth		= rhs.m_u32UploadBandwidth;
		m_bStartServerOnStartup		= rhs.m_bStartServerOnStartup;
	}

	return *this;
}

//============================================================================
void FileShareSettings::loadSettings( EngineSettings& engineSettings )
{
	LogMsg( LOG_INFO, "FileShareSettings::loadSettings\n" );
	engineSettings.getDnldsBandwidth( m_u32DownloadBandwidth );
	engineSettings.getMaxDownloadingFiles( m_u32MaxDownloadingFiles );

	engineSettings.getUpldsBandwidth( m_u32UploadBandwidth );
	engineSettings.getMaxUploadingFiles( m_u32MaxUploadingFiles );


	if( 0 == m_u32MaxDownloadingFiles )
	{
		m_u32MaxDownloadingFiles = DEFAULT_MAX_DOWNLOADING_FILES;
		engineSettings.setMaxDownloadingFiles( m_u32MaxDownloadingFiles );
	}

	if( 0 == m_u32MaxUploadingFiles )
	{
		m_u32MaxUploadingFiles = DEFAULT_MAX_UPLOADING_FILES;
		engineSettings.setMaxUploadingFiles( m_u32MaxUploadingFiles );
	}
}

//============================================================================
void FileShareSettings::saveSettings( EngineSettings& engineSettings )
{
	engineSettings.setDnldsBandwidth( m_u32DownloadBandwidth );
	engineSettings.setMaxDownloadingFiles( m_u32MaxDownloadingFiles );

	engineSettings.setUpldsBandwidth( m_u32UploadBandwidth );
	engineSettings.setMaxUploadingFiles( m_u32MaxUploadingFiles );
}
