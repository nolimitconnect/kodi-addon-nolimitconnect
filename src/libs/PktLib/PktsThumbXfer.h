#pragma once
//============================================================================
// Copyright (C) 2021 Brett R. Jones 
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

class PktThumbGetReq : public PktBaseGetReq
{
public:
    PktThumbGetReq();
};

class PktThumbGetReply : public PktBaseGetReply
{
public:
    PktThumbGetReply();
};

class  PktThumbSendReq : public PktBaseSendReq
{
public:
     PktThumbSendReq();
};

class  PktThumbSendReply : public PktBaseSendReply
{
public:
     PktThumbSendReply();
};

//============================================================================
// Asset chunk packets
//============================================================================
class  PktThumbChunkReq : public PktBaseChunkReq
{
public:
     PktThumbChunkReq();
};

class  PktThumbChunkReply : public PktBaseChunkReply
{
public:
     PktThumbChunkReply();
};

//============================================================================
// PktAssetGetComplete
//============================================================================
class PktThumbGetCompleteReq : public PktBaseGetCompleteReq
{
public:
    PktThumbGetCompleteReq();
};

class PktThumbGetCompleteReply : public PktBaseGetCompleteReply
{
public:
    PktThumbGetCompleteReply();
};

//============================================================================
//  PktThumbSendComplete
//============================================================================
class  PktThumbSendCompleteReq : public PktBaseSendCompleteReq
{
public:
     PktThumbSendCompleteReq();
};

class  PktThumbSendCompleteReply : public PktBaseSendCompleteReply
{
public:
     PktThumbSendCompleteReply();
};

//============================================================================
//  PktThumbXferErr
//============================================================================
class  PktThumbXferErr : public PktBaseXferErr
{
public:
    PktThumbXferErr();
};

#pragma pack(pop)

