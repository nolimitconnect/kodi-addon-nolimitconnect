/*
 *  Copyright (C) 2016-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "Bookmark.h"
#include "VideoInfoTag.h"

#include "utils/SortUtils.h"
#include "utils/UrlOptions.h"

#include <memory>
#include <set>
#include <utility>
#include <vector>

class CFileItem;
class CFileItemList;
class CVideoSettings;
class CGUIDialogProgress;
class CGUIDialogProgressBarHandle;

namespace dbiplus
{
  class field_value;
  typedef std::vector<field_value> sql_record;
}

#ifndef my_offsetof
#ifndef TARGET_POSIX
#define my_offsetof(TYPE, MEMBER) offsetof(TYPE, MEMBER)
#else
/*
   Custom version of standard offsetof() macro which can be used to get
   offsets of members in class for non-POD types (according to the current
   version of C++ standard offsetof() macro can't be used in such cases and
   attempt to do so causes warnings to be emitted, OTOH in many cases it is
   still OK to assume that all instances of the class has the same offsets
   for the same members).
 */
#define my_offsetof(TYPE, MEMBER) \
               ((size_t)((char *)&(((TYPE *)0x10)->MEMBER) - (char*)0x10))
#endif
#endif

typedef std::vector<CVideoInfoTag> VECMOVIES;

namespace VIDEO
{
  class IVideoInfoScannerObserver;
  struct SScanSettings;
}

enum VideoDbDetails
{
  VideoDbDetailsNone     = 0x00,
  VideoDbDetailsRating   = 0x01,
  VideoDbDetailsTag      = 0x02,
  VideoDbDetailsShowLink = 0x04,
  VideoDbDetailsStream   = 0x08,
  VideoDbDetailsCast     = 0x10,
  VideoDbDetailsBookmark = 0x20,
  VideoDbDetailsUniqueID = 0x40,
  VideoDbDetailsAll      = 0xFF
} ;

// these defines are based on how many columns we have and which column certain data is going to be in
// when we do GetDetailsForMovie()
#define VIDEODB_MAX_COLUMNS 24
#define VIDEODB_DETAILS_FILEID      1

#define VIDEODB_DETAILS_MOVIE_SET_ID            VIDEODB_MAX_COLUMNS + 2
#define VIDEODB_DETAILS_MOVIE_USER_RATING       VIDEODB_MAX_COLUMNS + 3
#define VIDEODB_DETAILS_MOVIE_PREMIERED         VIDEODB_MAX_COLUMNS + 4
#define VIDEODB_DETAILS_MOVIE_SET_NAME          VIDEODB_MAX_COLUMNS + 5
#define VIDEODB_DETAILS_MOVIE_SET_OVERVIEW      VIDEODB_MAX_COLUMNS + 6
#define VIDEODB_DETAILS_MOVIE_FILE              VIDEODB_MAX_COLUMNS + 7
#define VIDEODB_DETAILS_MOVIE_PATH              VIDEODB_MAX_COLUMNS + 8
#define VIDEODB_DETAILS_MOVIE_PLAYCOUNT         VIDEODB_MAX_COLUMNS + 9
#define VIDEODB_DETAILS_MOVIE_LASTPLAYED        VIDEODB_MAX_COLUMNS + 10
#define VIDEODB_DETAILS_MOVIE_DATEADDED         VIDEODB_MAX_COLUMNS + 11
#define VIDEODB_DETAILS_MOVIE_RESUME_TIME       VIDEODB_MAX_COLUMNS + 12
#define VIDEODB_DETAILS_MOVIE_TOTAL_TIME        VIDEODB_MAX_COLUMNS + 13
#define VIDEODB_DETAILS_MOVIE_PLAYER_STATE      VIDEODB_MAX_COLUMNS + 14
#define VIDEODB_DETAILS_MOVIE_RATING            VIDEODB_MAX_COLUMNS + 15
#define VIDEODB_DETAILS_MOVIE_VOTES             VIDEODB_MAX_COLUMNS + 16
#define VIDEODB_DETAILS_MOVIE_RATING_TYPE       VIDEODB_MAX_COLUMNS + 17
#define VIDEODB_DETAILS_MOVIE_UNIQUEID_VALUE    VIDEODB_MAX_COLUMNS + 18
#define VIDEODB_DETAILS_MOVIE_UNIQUEID_TYPE     VIDEODB_MAX_COLUMNS + 19

#define VIDEODB_DETAILS_EPISODE_TVSHOW_ID       VIDEODB_MAX_COLUMNS + 2
#define VIDEODB_DETAILS_EPISODE_USER_RATING     VIDEODB_MAX_COLUMNS + 3
#define VIDEODB_DETAILS_EPISODE_SEASON_ID       VIDEODB_MAX_COLUMNS + 4
#define VIDEODB_DETAILS_EPISODE_FILE            VIDEODB_MAX_COLUMNS + 5
#define VIDEODB_DETAILS_EPISODE_PATH            VIDEODB_MAX_COLUMNS + 6
#define VIDEODB_DETAILS_EPISODE_PLAYCOUNT       VIDEODB_MAX_COLUMNS + 7
#define VIDEODB_DETAILS_EPISODE_LASTPLAYED      VIDEODB_MAX_COLUMNS + 8
#define VIDEODB_DETAILS_EPISODE_DATEADDED       VIDEODB_MAX_COLUMNS + 9
#define VIDEODB_DETAILS_EPISODE_TVSHOW_NAME     VIDEODB_MAX_COLUMNS + 10
#define VIDEODB_DETAILS_EPISODE_TVSHOW_GENRE    VIDEODB_MAX_COLUMNS + 11
#define VIDEODB_DETAILS_EPISODE_TVSHOW_STUDIO   VIDEODB_MAX_COLUMNS + 12
#define VIDEODB_DETAILS_EPISODE_TVSHOW_AIRED    VIDEODB_MAX_COLUMNS + 13
#define VIDEODB_DETAILS_EPISODE_TVSHOW_MPAA     VIDEODB_MAX_COLUMNS + 14
#define VIDEODB_DETAILS_EPISODE_RESUME_TIME     VIDEODB_MAX_COLUMNS + 15
#define VIDEODB_DETAILS_EPISODE_TOTAL_TIME      VIDEODB_MAX_COLUMNS + 16
#define VIDEODB_DETAILS_EPISODE_PLAYER_STATE    VIDEODB_MAX_COLUMNS + 17
#define VIDEODB_DETAILS_EPISODE_RATING          VIDEODB_MAX_COLUMNS + 18
#define VIDEODB_DETAILS_EPISODE_VOTES           VIDEODB_MAX_COLUMNS + 19
#define VIDEODB_DETAILS_EPISODE_RATING_TYPE     VIDEODB_MAX_COLUMNS + 20
#define VIDEODB_DETAILS_EPISODE_UNIQUEID_VALUE  VIDEODB_MAX_COLUMNS + 21
#define VIDEODB_DETAILS_EPISODE_UNIQUEID_TYPE   VIDEODB_MAX_COLUMNS + 22

#define VIDEODB_DETAILS_TVSHOW_USER_RATING      VIDEODB_MAX_COLUMNS + 1
#define VIDEODB_DETAILS_TVSHOW_DURATION         VIDEODB_MAX_COLUMNS + 2
#define VIDEODB_DETAILS_TVSHOW_PARENTPATHID     VIDEODB_MAX_COLUMNS + 3
#define VIDEODB_DETAILS_TVSHOW_PATH             VIDEODB_MAX_COLUMNS + 4
#define VIDEODB_DETAILS_TVSHOW_DATEADDED        VIDEODB_MAX_COLUMNS + 5
#define VIDEODB_DETAILS_TVSHOW_LASTPLAYED       VIDEODB_MAX_COLUMNS + 6
#define VIDEODB_DETAILS_TVSHOW_NUM_EPISODES     VIDEODB_MAX_COLUMNS + 7
#define VIDEODB_DETAILS_TVSHOW_NUM_WATCHED      VIDEODB_MAX_COLUMNS + 8
#define VIDEODB_DETAILS_TVSHOW_NUM_SEASONS      VIDEODB_MAX_COLUMNS + 9
#define VIDEODB_DETAILS_TVSHOW_RATING           VIDEODB_MAX_COLUMNS + 10
#define VIDEODB_DETAILS_TVSHOW_VOTES            VIDEODB_MAX_COLUMNS + 11
#define VIDEODB_DETAILS_TVSHOW_RATING_TYPE      VIDEODB_MAX_COLUMNS + 12
#define VIDEODB_DETAILS_TVSHOW_UNIQUEID_VALUE   VIDEODB_MAX_COLUMNS + 13
#define VIDEODB_DETAILS_TVSHOW_UNIQUEID_TYPE    VIDEODB_MAX_COLUMNS + 14

#define VIDEODB_DETAILS_MUSICVIDEO_USER_RATING  VIDEODB_MAX_COLUMNS + 2
#define VIDEODB_DETAILS_MUSICVIDEO_PREMIERED    VIDEODB_MAX_COLUMNS + 3
#define VIDEODB_DETAILS_MUSICVIDEO_FILE         VIDEODB_MAX_COLUMNS + 4
#define VIDEODB_DETAILS_MUSICVIDEO_PATH         VIDEODB_MAX_COLUMNS + 5
#define VIDEODB_DETAILS_MUSICVIDEO_PLAYCOUNT    VIDEODB_MAX_COLUMNS + 6
#define VIDEODB_DETAILS_MUSICVIDEO_LASTPLAYED   VIDEODB_MAX_COLUMNS + 7
#define VIDEODB_DETAILS_MUSICVIDEO_DATEADDED    VIDEODB_MAX_COLUMNS + 8
#define VIDEODB_DETAILS_MUSICVIDEO_RESUME_TIME  VIDEODB_MAX_COLUMNS + 9
#define VIDEODB_DETAILS_MUSICVIDEO_TOTAL_TIME   VIDEODB_MAX_COLUMNS + 10
#define VIDEODB_DETAILS_MUSICVIDEO_PLAYER_STATE VIDEODB_MAX_COLUMNS + 11
#define VIDEODB_DETAILS_MUSICVIDEO_UNIQUEID_VALUE VIDEODB_MAX_COLUMNS + 12
#define VIDEODB_DETAILS_MUSICVIDEO_UNIQUEID_TYPE VIDEODB_MAX_COLUMNS + 13

#define VIDEODB_TYPE_UNUSED 0
#define VIDEODB_TYPE_STRING 1
#define VIDEODB_TYPE_INT 2
#define VIDEODB_TYPE_FLOAT 3
#define VIDEODB_TYPE_BOOL 4
#define VIDEODB_TYPE_COUNT 5
#define VIDEODB_TYPE_STRINGARRAY 6
#define VIDEODB_TYPE_DATE 7
#define VIDEODB_TYPE_DATETIME 8

enum class VideoDbContentType
{
  UNKNOWN = -1,
  MOVIES = 1,
  TVSHOWS = 2,
  MUSICVIDEOS = 3,
  EPISODES = 4,
  MOVIE_SETS = 5,
  MUSICALBUMS = 6
};

typedef enum // this enum MUST match the offset struct further down!! and make sure to keep min and max at -1 and sizeof(offsets)
{
  VIDEODB_ID_MIN = -1,
  VIDEODB_ID_TITLE = 0,
  VIDEODB_ID_PLOT = 1,
  VIDEODB_ID_PLOTOUTLINE = 2,
  VIDEODB_ID_TAGLINE = 3,
  VIDEODB_ID_VOTES = 4, // unused
  VIDEODB_ID_RATING_ID = 5,
  VIDEODB_ID_CREDITS = 6,
  VIDEODB_ID_YEAR = 7, // unused
  VIDEODB_ID_THUMBURL = 8,
  VIDEODB_ID_IDENT_ID = 9,
  VIDEODB_ID_SORTTITLE = 10,
  VIDEODB_ID_RUNTIME = 11,
  VIDEODB_ID_MPAA = 12,
  VIDEODB_ID_TOP250 = 13,
  VIDEODB_ID_GENRE = 14,
  VIDEODB_ID_DIRECTOR = 15,
  VIDEODB_ID_ORIGINALTITLE = 16,
  VIDEODB_ID_THUMBURL_SPOOF = 17,
  VIDEODB_ID_STUDIOS = 18,
  VIDEODB_ID_TRAILER = 19,
  VIDEODB_ID_FANART = 20,
  VIDEODB_ID_COUNTRY = 21,
  VIDEODB_ID_BASEPATH = 22,
  VIDEODB_ID_PARENTPATHID = 23,
  VIDEODB_ID_MAX
} VIDEODB_IDS;

const struct SDbTableOffsets
{
  int type;
  size_t offset;
} DbMovieOffsets[] =
{
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_strTitle) },
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_strPlot) },
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_strPlotOutline) },
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_strTagLine) },
  { VIDEODB_TYPE_UNUSED, 0 }, // unused
  { VIDEODB_TYPE_INT, my_offsetof(CVideoInfoTag,m_iIdRating) },
  { VIDEODB_TYPE_STRINGARRAY, my_offsetof(CVideoInfoTag,m_writingCredits) },
  { VIDEODB_TYPE_UNUSED, 0 }, // unused
#if HAVE_LIB_CURL
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_strPictureURL.m_data) },
#else
  { VIDEODB_TYPE_UNUSED, 0 }, // unused
#endif // HAVE_LIB_CURL
  { VIDEODB_TYPE_INT, my_offsetof(CVideoInfoTag,m_iIdUniqueID) },
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_strSortTitle) },
  { VIDEODB_TYPE_INT, my_offsetof(CVideoInfoTag,m_duration) },
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_strMPAARating) },
  { VIDEODB_TYPE_INT, my_offsetof(CVideoInfoTag,m_iTop250) },
  { VIDEODB_TYPE_STRINGARRAY, my_offsetof(CVideoInfoTag,m_genre) },
  { VIDEODB_TYPE_STRINGARRAY, my_offsetof(CVideoInfoTag,m_director) },
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_strOriginalTitle) },
  { VIDEODB_TYPE_UNUSED, 0 }, // unused
  { VIDEODB_TYPE_STRINGARRAY, my_offsetof(CVideoInfoTag,m_studio) },
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_strTrailer) },
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_fanart.m_xml) },
  { VIDEODB_TYPE_STRINGARRAY, my_offsetof(CVideoInfoTag,m_country) },
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_basePath) },
  { VIDEODB_TYPE_INT, my_offsetof(CVideoInfoTag,m_parentPathID) }
};

typedef enum // this enum MUST match the offset struct further down!! and make sure to keep min and max at -1 and sizeof(offsets)
{
  VIDEODB_ID_TV_MIN = -1,
  VIDEODB_ID_TV_TITLE = 0,
  VIDEODB_ID_TV_PLOT = 1,
  VIDEODB_ID_TV_STATUS = 2,
  VIDEODB_ID_TV_VOTES = 3, // unused
  VIDEODB_ID_TV_RATING_ID = 4,
  VIDEODB_ID_TV_PREMIERED = 5,
  VIDEODB_ID_TV_THUMBURL = 6,
  VIDEODB_ID_TV_THUMBURL_SPOOF = 7,
  VIDEODB_ID_TV_GENRE = 8,
  VIDEODB_ID_TV_ORIGINALTITLE = 9,
  VIDEODB_ID_TV_EPISODEGUIDE = 10,
  VIDEODB_ID_TV_FANART = 11,
  VIDEODB_ID_TV_IDENT_ID = 12,
  VIDEODB_ID_TV_MPAA = 13,
  VIDEODB_ID_TV_STUDIOS = 14,
  VIDEODB_ID_TV_SORTTITLE = 15,
  VIDEODB_ID_TV_TRAILER = 16,
  VIDEODB_ID_TV_MAX
} VIDEODB_TV_IDS;

const struct SDbTableOffsets DbTvShowOffsets[] =
{
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_strTitle) },
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_strPlot) },
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_strStatus) },
  { VIDEODB_TYPE_UNUSED, 0 }, //unused
  { VIDEODB_TYPE_INT, my_offsetof(CVideoInfoTag,m_iIdRating) },
  { VIDEODB_TYPE_DATE, my_offsetof(CVideoInfoTag,m_premiered) },
#if HAVE_LIB_CURL
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_strPictureURL.m_data) },
#else
  { VIDEODB_TYPE_UNUSED, 0 }, // unused
#endif // HAVE_LIB_CURL
  { VIDEODB_TYPE_UNUSED, 0 }, // unused
  { VIDEODB_TYPE_STRINGARRAY, my_offsetof(CVideoInfoTag,m_genre) },
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_strOriginalTitle)},
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_strEpisodeGuide)},
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_fanart.m_xml)},
  { VIDEODB_TYPE_INT, my_offsetof(CVideoInfoTag,m_iIdUniqueID)},
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_strMPAARating)},
  { VIDEODB_TYPE_STRINGARRAY, my_offsetof(CVideoInfoTag,m_studio)},
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_strSortTitle)},
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_strTrailer)}
};

//! @todo is this comment valid for seasons? There is no offset structure or am I wrong?
typedef enum // this enum MUST match the offset struct further down!! and make sure to keep min and max at -1 and sizeof(offsets)
{
  VIDEODB_ID_SEASON_MIN = -1,
  VIDEODB_ID_SEASON_ID = 0,
  VIDEODB_ID_SEASON_TVSHOW_ID = 1,
  VIDEODB_ID_SEASON_NUMBER = 2,
  VIDEODB_ID_SEASON_NAME = 3,
  VIDEODB_ID_SEASON_USER_RATING = 4,
  VIDEODB_ID_SEASON_TVSHOW_PATH = 5,
  VIDEODB_ID_SEASON_TVSHOW_TITLE = 6,
  VIDEODB_ID_SEASON_TVSHOW_PLOT = 7,
  VIDEODB_ID_SEASON_TVSHOW_PREMIERED = 8,
  VIDEODB_ID_SEASON_TVSHOW_GENRE = 9,
  VIDEODB_ID_SEASON_TVSHOW_STUDIO = 10,
  VIDEODB_ID_SEASON_TVSHOW_MPAA = 11,
  VIDEODB_ID_SEASON_EPISODES_TOTAL = 12,
  VIDEODB_ID_SEASON_EPISODES_WATCHED = 13,
  VIDEODB_ID_SEASON_PREMIERED = 14,
  VIDEODB_ID_SEASON_MAX
} VIDEODB_SEASON_IDS;

typedef enum // this enum MUST match the offset struct further down!! and make sure to keep min and max at -1 and sizeof(offsets)
{
  VIDEODB_ID_EPISODE_MIN = -1,
  VIDEODB_ID_EPISODE_TITLE = 0,
  VIDEODB_ID_EPISODE_PLOT = 1,
  VIDEODB_ID_EPISODE_VOTES = 2, // unused
  VIDEODB_ID_EPISODE_RATING_ID = 3,
  VIDEODB_ID_EPISODE_CREDITS = 4,
  VIDEODB_ID_EPISODE_AIRED = 5,
  VIDEODB_ID_EPISODE_THUMBURL = 6,
  VIDEODB_ID_EPISODE_THUMBURL_SPOOF = 7,
  VIDEODB_ID_EPISODE_PLAYCOUNT = 8, // unused - feel free to repurpose
  VIDEODB_ID_EPISODE_RUNTIME = 9,
  VIDEODB_ID_EPISODE_DIRECTOR = 10,
  VIDEODB_ID_EPISODE_PRODUCTIONCODE = 11,
  VIDEODB_ID_EPISODE_SEASON = 12,
  VIDEODB_ID_EPISODE_EPISODE = 13,
  VIDEODB_ID_EPISODE_ORIGINALTITLE = 14,
  VIDEODB_ID_EPISODE_SORTSEASON = 15,
  VIDEODB_ID_EPISODE_SORTEPISODE = 16,
  VIDEODB_ID_EPISODE_BOOKMARK = 17,
  VIDEODB_ID_EPISODE_BASEPATH = 18,
  VIDEODB_ID_EPISODE_PARENTPATHID = 19,
  VIDEODB_ID_EPISODE_IDENT_ID = 20,
  VIDEODB_ID_EPISODE_MAX
} VIDEODB_EPISODE_IDS;

const struct SDbTableOffsets DbEpisodeOffsets[] =
{
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_strTitle) },
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_strPlot) },
  { VIDEODB_TYPE_UNUSED, 0 }, // unused
  { VIDEODB_TYPE_INT, my_offsetof(CVideoInfoTag,m_iIdRating) },
  { VIDEODB_TYPE_STRINGARRAY, my_offsetof(CVideoInfoTag,m_writingCredits) },
  { VIDEODB_TYPE_DATE, my_offsetof(CVideoInfoTag,m_firstAired) },
#if HAVE_LIB_CURL
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_strPictureURL.m_data) },
#else
  { VIDEODB_TYPE_UNUSED, 0 }, // unused
#endif // HAVE_LIB_CURL
  { VIDEODB_TYPE_UNUSED, 0 }, // unused
  { VIDEODB_TYPE_UNUSED, 0 }, // unused
  { VIDEODB_TYPE_INT, my_offsetof(CVideoInfoTag,m_duration) },
  { VIDEODB_TYPE_STRINGARRAY, my_offsetof(CVideoInfoTag,m_director) },
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_strProductionCode) },
  { VIDEODB_TYPE_INT, my_offsetof(CVideoInfoTag,m_iSeason) },
  { VIDEODB_TYPE_INT, my_offsetof(CVideoInfoTag,m_iEpisode) },
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_strOriginalTitle)},
  { VIDEODB_TYPE_INT, my_offsetof(CVideoInfoTag,m_iSpecialSortSeason) },
  { VIDEODB_TYPE_INT, my_offsetof(CVideoInfoTag,m_iSpecialSortEpisode) },
  { VIDEODB_TYPE_INT, my_offsetof(CVideoInfoTag,m_iBookmarkId) },
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_basePath) },
  { VIDEODB_TYPE_INT, my_offsetof(CVideoInfoTag,m_parentPathID) },
  { VIDEODB_TYPE_INT, my_offsetof(CVideoInfoTag,m_iIdUniqueID) }
};

typedef enum // this enum MUST match the offset struct further down!! and make sure to keep min and max at -1 and sizeof(offsets)
{
  VIDEODB_ID_MUSICVIDEO_MIN = -1,
  VIDEODB_ID_MUSICVIDEO_TITLE = 0,
  VIDEODB_ID_MUSICVIDEO_THUMBURL = 1,
  VIDEODB_ID_MUSICVIDEO_THUMBURL_SPOOF = 2,
  VIDEODB_ID_MUSICVIDEO_PLAYCOUNT = 3, // unused - feel free to repurpose
  VIDEODB_ID_MUSICVIDEO_RUNTIME = 4,
  VIDEODB_ID_MUSICVIDEO_DIRECTOR = 5,
  VIDEODB_ID_MUSICVIDEO_STUDIOS = 6,
  VIDEODB_ID_MUSICVIDEO_YEAR = 7, // unused
  VIDEODB_ID_MUSICVIDEO_PLOT = 8,
  VIDEODB_ID_MUSICVIDEO_ALBUM = 9,
  VIDEODB_ID_MUSICVIDEO_ARTIST = 10,
  VIDEODB_ID_MUSICVIDEO_GENRE = 11,
  VIDEODB_ID_MUSICVIDEO_TRACK = 12,
  VIDEODB_ID_MUSICVIDEO_BASEPATH = 13,
  VIDEODB_ID_MUSICVIDEO_PARENTPATHID = 14,
  VIDEODB_ID_MUSICVIDEO_IDENT_ID = 15,
  VIDEODB_ID_MUSICVIDEO_MAX
} VIDEODB_MUSICVIDEO_IDS;

const struct SDbTableOffsets DbMusicVideoOffsets[] =
{
  { VIDEODB_TYPE_STRING, my_offsetof(class CVideoInfoTag,m_strTitle) },
#if HAVE_LIB_CURL
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_strPictureURL.m_data) },
#else
  { VIDEODB_TYPE_UNUSED, 0 }, // unused
#endif // HAVE_LIB_CURL
  { VIDEODB_TYPE_UNUSED, 0 }, // unused
  { VIDEODB_TYPE_UNUSED, 0 }, // unused
  { VIDEODB_TYPE_INT, my_offsetof(CVideoInfoTag,m_duration) },
  { VIDEODB_TYPE_STRINGARRAY, my_offsetof(CVideoInfoTag,m_director) },
  { VIDEODB_TYPE_STRINGARRAY, my_offsetof(CVideoInfoTag,m_studio) },
  { VIDEODB_TYPE_UNUSED, 0 }, // unused
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_strPlot) },
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_strAlbum) },
  { VIDEODB_TYPE_STRINGARRAY, my_offsetof(CVideoInfoTag,m_artist) },
  { VIDEODB_TYPE_STRINGARRAY, my_offsetof(CVideoInfoTag,m_genre) },
  { VIDEODB_TYPE_INT, my_offsetof(CVideoInfoTag,m_iTrack) },
  { VIDEODB_TYPE_STRING, my_offsetof(CVideoInfoTag,m_basePath) },
  { VIDEODB_TYPE_INT, my_offsetof(CVideoInfoTag,m_parentPathID) },
  { VIDEODB_TYPE_INT, my_offsetof(CVideoInfoTag,m_iIdUniqueID)}
};

#define COMPARE_PERCENTAGE     0.90f // 90%
#define COMPARE_PERCENTAGE_MIN 0.50f // 50%

