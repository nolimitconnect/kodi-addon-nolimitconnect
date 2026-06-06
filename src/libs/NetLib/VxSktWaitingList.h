#ifndef VX_SKT_WAITING_LIST_H
#define VX_SKT_WAITING_LIST_H

//============================================================================
// Copyright (C) 2014 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <CoreLib/VxMutex.h>

#include <vector>
#include <memory>

class VxSktBase;
class VxPktHdr;

class IVxSktWaitingCallback
{
public:
	virtual	void					onSktWaitExpired(	std::shared_ptr<VxSktBase>&		sktBase, 
														uint32_t		u32WaitReason, 
														VxPktHdr*		poPkt,
														void *			pvWaitInstance,
														bool			bConnectionLost ) = 0;
};

class VxSktWaitReason
{
public:
	VxSktWaitReason() = default;
	VxSktWaitReason( std::shared_ptr<VxSktBase>& sktBase, uint32_t waitReason, uint64_t timeExpiresSysTimeMs, VxPktHdr* poPkt, void * pvWaitInstance );
	VxSktWaitReason( const VxSktWaitReason& rhs );
	virtual ~VxSktWaitReason();

	VxSktWaitReason& operator =( const VxSktWaitReason& rhs );

	std::shared_ptr<VxSktBase>	m_Skt;
	VxPktHdr*					m_Pkt{ nullptr };
	uint64_t					m_u64TimeExpires{ 0 };
	uint32_t					m_u32WaitReason{ 0 };
	void *						m_pvWaitInstance{ nullptr };
};

class VxSktWaitingList
{
public:
	VxSktWaitingList();
	virtual ~VxSktWaitingList();

	void						setSktWaitCallback( IVxSktWaitingCallback * sktWaitCallback );

	virtual void				onOncePerSecond( void );		
	virtual void				onConnectionLost( std::shared_ptr<VxSktBase>& sktBase );	

	virtual void				addWaiting(		std::shared_ptr<VxSktBase>&		sktBase, 
												uint32_t		u32WaitReason, 
												uint64_t		u64TimeExpiresSysTimeMs, 
												VxPktHdr*		poPkt = 0, 
												void *			pvWaitInstance = 0 );		
	virtual void				removeWaiting( std::shared_ptr<VxSktBase>& sktBase, uint32_t waitReason, void * pvWaitInstance = 0 );		
	virtual void				clearAllWaiting( void );

protected:
	std::vector<VxSktWaitReason *>	m_SktWaitList;
	VxMutex							m_SktWaitListMutex;
	IVxSktWaitingCallback *			m_SktWaitCallback;
};

#endif // VX_SKT_WAITING_LIST_H
