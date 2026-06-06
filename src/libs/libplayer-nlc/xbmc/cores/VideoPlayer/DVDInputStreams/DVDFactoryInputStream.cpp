/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "config_components_kodi.h"
#include "DVDFactoryInputStream.h"

#include "DVDInputStream.h"

#include "DVDInputStreamFFmpeg.h"
#include "DVDInputStreamFile.h"
#include "DVDInputStreamNavigator.h"
#include "DVDInputStreamStack.h"
#include "FileItem.h"
#include "InputStreamAddon.h"
#include "InputStreamMultiSource.h"
#include "InputStreamPVRChannel.h"
#include "InputStreamPVRRecording.h"
#include "ServiceBroker.h"

#include "filesystem/IFileTypes.h"
#include "storage/MediaManager.h"
#include "utils/FileUtils.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"

#include "NlcCoreUtil.h"
#include "NlcUrl.h"

std::shared_ptr<CDVDInputStream> CDVDFactoryInputStream::CreateInputStream(IVideoPlayer* pPlayer, const CFileItem &fileitem, bool scanforextaudio)
{
  using namespace ADDON;

  const std::string& file = fileitem.GetDynPath();
  if (scanforextaudio)
  {
    // find any available external audio tracks
    std::vector<std::string> filenames;
    filenames.push_back(file);
    CUtil::ScanForExternalAudio(file, filenames);
    if (filenames.size() >= 2)
    {
      return CreateInputStream(pPlayer, fileitem, filenames);
    }
  }
  else if(StringUtils::StartsWithNoCase(file, "rtp://") ||
          StringUtils::StartsWithNoCase(file, "rtsp://") ||
          StringUtils::StartsWithNoCase(file, "rtsps://") ||
           StringUtils::StartsWithNoCase(file, "satip://") ||
          StringUtils::StartsWithNoCase(file, "sdp://") ||
          StringUtils::StartsWithNoCase(file, "udp://") ||
          StringUtils::StartsWithNoCase(file, "tcp://") ||
          StringUtils::StartsWithNoCase(file, "mms://") ||
          StringUtils::StartsWithNoCase(file, "mmst://") ||
          StringUtils::StartsWithNoCase(file, "mmsh://") ||
          StringUtils::StartsWithNoCase(file, "rtmp://") ||
          StringUtils::StartsWithNoCase(file, "rtmpt://") ||
          StringUtils::StartsWithNoCase(file, "rtmpe://") ||
          StringUtils::StartsWithNoCase(file, "rtmpte://") ||
          StringUtils::StartsWithNoCase(file, "rtmps://"))
  {
    return std::shared_ptr<CDVDInputStreamFFmpeg>(new CDVDInputStreamFFmpeg(fileitem));
  }
  else if(StringUtils::StartsWithNoCase(file, "stack://"))
    return std::shared_ptr<CDVDInputStreamStack>(new CDVDInputStreamStack(fileitem));

  CFileItem finalFileitem(fileitem);

  if (finalFileitem.IsInternetStream())
  {
    if (finalFileitem.ContentLookup())
    {

    }

    if (finalFileitem.IsType(".m3u8"))
      return std::shared_ptr<CDVDInputStreamFFmpeg>(new CDVDInputStreamFFmpeg(finalFileitem));

    // mime type for m3u8/hls streams
    if (finalFileitem.GetMimeType() == "application/vnd.apple.mpegurl" ||
        finalFileitem.GetMimeType() == "application/x-mpegURL")
      return std::shared_ptr<CDVDInputStreamFFmpeg>(new CDVDInputStreamFFmpeg(finalFileitem));

    if (URIUtils::IsProtocol(finalFileitem.GetPath(), "udp"))
      return std::shared_ptr<CDVDInputStreamFFmpeg>(new CDVDInputStreamFFmpeg(finalFileitem));
  }

  // our file interface handles all these types of streams
  return std::shared_ptr<CDVDInputStreamFile>(new CDVDInputStreamFile(finalFileitem,
                                                                      XFILE::READ_TRUNCATED |
                                                                      XFILE::READ_BITRATE |
                                                                      XFILE::READ_CHUNKED));
}

std::shared_ptr<CDVDInputStream> CDVDFactoryInputStream::CreateInputStream(IVideoPlayer* pPlayer, const CFileItem &fileitem, const std::vector<std::string>& filenames)
{
  return std::shared_ptr<CInputStreamMultiSource>(new CInputStreamMultiSource(pPlayer, fileitem, filenames));
}
