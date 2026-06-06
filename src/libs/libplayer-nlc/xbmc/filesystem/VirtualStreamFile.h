
#pragma once

#include "IFile.h"
#include "NlcUrl.h"

#include <CoreLib/VFile.h>

namespace XFILE
{

	class CVirtualStreamFile : public IFile
	{
	public:
		CVirtualStreamFile() = default;
		~CVirtualStreamFile() override;

		int64_t		GetPosition() override;
		int64_t		GetLength() override;
		bool		Open( const NlcUrl& url ) override;
		bool		Exists( const NlcUrl& url ) override;
		int			Stat( const NlcUrl& url, struct __stat64* buffer ) override;
        int64_t		Read( void* lpBuf, size_t uiBufSize ) override;
		int64_t		Seek( int64_t iFilePosition, int iWhence = SEEK_SET ) override;
		void		Close() override;

	protected:
		VFile*		OpenVFile( const char * fileName );
		
		NlcUrl		m_LastUrl;
		VFile*		m_VFile{ nullptr };
	};

}

