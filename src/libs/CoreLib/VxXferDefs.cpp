//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxXferDefs.h"


namespace
{
    const char* ENUM_BAD_PARM = "ENUM BAD PARAM ";

    const char* EXferDirectionEnumStrings[] =
    {
        "Xfer Direction Unknown ",
        "Xfer Direction Rx ",
        "Xfer Direction Tx ",

        "Max Xfer Direction "
    };

    const char* EXferErrorEnumStrings[] =
    {
        "No Error ",
        "Disconnected ",
        "Permission ",
        "File Not Found ",
        "Canceled ",
        "Bad Param ",
        "At Src ",
        "Busy ",
        "Already Downloading ",
        "Already Downloaded ",
        "Already Uploading ",
        "File Create Error ",
        "File Open Append Error ",
        "File Open Error ",
        "File Seek Error ",
        "File Read Error ",
        "File Write Error ",
        "File Move Error ",

        "Max Xfer Error "
    };

    const char* EXferStateEnumStrings[] =
    {
        "None ",
        "Upload Not Started ",
        "Waiting Offer Response ",
        "In Upload Que ",
        "Begin Upload ",
        "In Upload Xfer ",
        "Complete dUpload ",
        "User Canceled Upload ",
        "Upload Offer Rejected ",
        "Upload Error ",
        "Download Not Started ",
        "In Download Que ",
        "Begin Download ",
        "In Download Xfer ",
        "Completed Download ",
        "User Canceled Download ",
        "Download Error ",

        "Streaming ",
        "Stream Stopped ",

        "Max Xfer State "
    };
};

//============================================================================
const char* DescribeXferDirection( EXferDirection xferDir )
{
    if( xferDir < 0 || eMaxXferDirection <= xferDir )
    {
        return ENUM_BAD_PARM;
    }

    return EXferDirectionEnumStrings[ xferDir ];
}

//============================================================================
const char* DescribeXferError( EXferError xferErr )
{
    if( xferErr < 0 || eMaxXferError <= xferErr )
    {
        return ENUM_BAD_PARM;
    }

    return EXferErrorEnumStrings[ xferErr ];
}

//============================================================================
const char* DescribeXferState( EXferState xferState )
{
    if( xferState < 0 || eMaxXferState <= xferState )
    {
        return ENUM_BAD_PARM;
    }

    return EXferStateEnumStrings[ xferState ];
}
