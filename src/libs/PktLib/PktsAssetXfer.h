#pragma once
//============================================================================
// Copyright (C) 2010 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PktsBaseXfer.h"

#pragma pack(push)
#pragma pack(1)

class PktAssetGetReq : public PktBaseGetReq
{
public:
    PktAssetGetReq();
};

class PktAssetGetReply : public PktBaseGetReply
{
public:
    PktAssetGetReply();
};

class PktAssetSendReq : public PktBaseSendReq
{
public:
	PktAssetSendReq();
};

class PktAssetSendReply : public PktBaseSendReply
{
public:
	PktAssetSendReply();
};

//============================================================================
// Asset chunk packets
//============================================================================
class PktAssetChunkReq : public PktBaseChunkReq
{
public:
	PktAssetChunkReq();
};

class PktAssetChunkReply : public PktBaseChunkReply
{
public:
	PktAssetChunkReply();
};

//============================================================================
// PktAssetGetComplete
//============================================================================
class PktAssetGetCompleteReq : public PktBaseGetCompleteReq
{
public:
    PktAssetGetCompleteReq();
};

class PktAssetGetCompleteReply : public PktBaseGetCompleteReply
{
public:
    PktAssetGetCompleteReply();
};

//============================================================================
// PktAssetSendComplete
//============================================================================
class PktAssetSendCompleteReq : public PktBaseSendCompleteReq
{
public:
	PktAssetSendCompleteReq();
};

class PktAssetSendCompleteReply : public PktBaseSendCompleteReply
{
public:
	PktAssetSendCompleteReply();
};

//============================================================================
// PktAssetXferErr
//============================================================================
class PktAssetXferErr : public PktBaseXferErr
{
public:
	PktAssetXferErr();
};

#pragma pack(pop) 

