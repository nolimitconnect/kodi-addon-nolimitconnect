

#include "VirtualStreamFile.h"

#include <sys/stat.h>
#include <errno.h>
#include <limits.h>

using namespace XFILE;

//*********************************************************************************************
CVirtualStreamFile::~CVirtualStreamFile()
{
	if( m_VFile )
	{
		Close();
	}
}

//*********************************************************************************************
bool CVirtualStreamFile::Open( const NlcUrl& url )
{
	m_VFile = OpenVFile( url.GetFileName().c_str() );
	if( !m_VFile )
	{
		return false;
	}

	m_LastUrl = url;
	return true;
}

//*********************************************************************************************
VFile* CVirtualStreamFile::OpenVFile( const char * fileName )
{
	return VFileOpen( fileName, "rbv" );
}

//*********************************************************************************************
int64_t CVirtualStreamFile::Read( void* lpBuf, size_t uiBufSize )
{
	if( uiBufSize > SSIZE_MAX )
		uiBufSize = SSIZE_MAX;
	if( uiBufSize > LONG_MAX )
		uiBufSize = LONG_MAX;

	if( !m_VFile )
	{
		return -1;
	}

	return VFileRead( lpBuf, 1, uiBufSize,  m_VFile  );
}

//*********************************************************************************************
void CVirtualStreamFile::Close()
{
	if( !m_VFile )
	{
		return;
	}

	VFileClose( m_VFile  );
	m_VFile = nullptr;
}

//*********************************************************************************************
int64_t CVirtualStreamFile::Seek( int64_t iFilePosition, int iWhence )
{
	if( !m_VFile )
	{
		return -1;
	}

	return VFileSeek( m_VFile, iFilePosition, iWhence );
}

//*********************************************************************************************
int64_t CVirtualStreamFile::GetLength()
{
	if( !m_VFile )
	{
		return -1;
	}

	return m_VFile->m_FileLen;
}

//*********************************************************************************************
int64_t CVirtualStreamFile::GetPosition()
{
	if( !m_VFile )
	{
		return -1;
	}

	return m_VFile->m_FileOffs;
}

//*********************************************************************************************
bool CVirtualStreamFile::Exists( const NlcUrl& url )
{
	if( m_LastUrl == url )
	{
		if( m_VFile )
		{
			return true;
		}
	}

	m_VFile = OpenVFile( url.GetFileName().c_str()  );
	if( !m_VFile )
	{
		return false;
	}

	m_LastUrl = url;
	return true;
}

//*********************************************************************************************
int CVirtualStreamFile::Stat( const NlcUrl& url, struct __stat64* buffer )
{
	if( url.GetFileName().empty() )
	{
		buffer->st_mode = _S_IFDIR;
		return 0;
	}

	if( m_LastUrl == url )
	{
		if( m_VFile )
		{
			buffer->st_mode = _S_IFREG;
			buffer->st_size = m_VFile->m_FileLen;
			return 0;
		}
	}

	VFile* tmpFile = OpenVFile( url.GetFileName().c_str() );
	if( tmpFile )
	{
		buffer->st_mode = _S_IFREG;
		buffer->st_size = tmpFile->m_FileLen;

		VFileClose( tmpFile );
		return 0;
	}

	errno = ENOENT;
	return -1;
}
