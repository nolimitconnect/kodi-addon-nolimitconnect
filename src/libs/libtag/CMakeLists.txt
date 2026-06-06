
cmake_minimum_required (VERSION 3.22.1)

project(tag, LANGUAGES CXX)

set(tag_SOURCES)

list(APPEND tag_SOURCES 
    ape/apefile.cpp
    ape/apefooter.cpp
    ape/apeitem.cpp
    ape/apeproperties.cpp
    ape/apetag.cpp
    asf/asfattribute.cpp
    asf/asffile.cpp
    asf/asfpicture.cpp
    asf/asfproperties.cpp
    asf/asftag.cpp
    audioproperties.cpp
    fileref.cpp
    flac/flacfile.cpp
    flac/flacmetadatablock.cpp
    flac/flacpicture.cpp
    flac/flacproperties.cpp
    flac/flacunknownmetadatablock.cpp
    it/itfile.cpp
    it/itproperties.cpp
    mod/modfile.cpp
    mod/modfilebase.cpp
    mod/modproperties.cpp
    mod/modtag.cpp
    mp4/mp4atom.cpp
    mp4/mp4coverart.cpp
    mp4/mp4file.cpp
    mp4/mp4item.cpp
    mp4/mp4properties.cpp
    mp4/mp4tag.cpp
    mpc/mpcfile.cpp
    mpc/mpcproperties.cpp
    mpeg/id3v1/id3v1genres.cpp
    mpeg/id3v1/id3v1tag.cpp
    mpeg/id3v2/frames/attachedpictureframe.cpp
    mpeg/id3v2/frames/chapterframe.cpp
    mpeg/id3v2/frames/commentsframe.cpp
    mpeg/id3v2/frames/eventtimingcodesframe.cpp
    mpeg/id3v2/frames/generalencapsulatedobjectframe.cpp
    mpeg/id3v2/frames/ownershipframe.cpp
    mpeg/id3v2/frames/podcastframe.cpp
    mpeg/id3v2/frames/popularimeterframe.cpp
    mpeg/id3v2/frames/privateframe.cpp
    mpeg/id3v2/frames/relativevolumeframe.cpp
    mpeg/id3v2/frames/synchronizedlyricsframe.cpp
    mpeg/id3v2/frames/tableofcontentsframe.cpp
    mpeg/id3v2/frames/textidentificationframe.cpp
    mpeg/id3v2/frames/uniquefileidentifierframe.cpp
    mpeg/id3v2/frames/unknownframe.cpp
    mpeg/id3v2/frames/unsynchronizedlyricsframe.cpp
    mpeg/id3v2/frames/urllinkframe.cpp
    mpeg/id3v2/id3v2extendedheader.cpp
    mpeg/id3v2/id3v2footer.cpp
    mpeg/id3v2/id3v2frame.cpp
    mpeg/id3v2/id3v2framefactory.cpp
    mpeg/id3v2/id3v2header.cpp
    mpeg/id3v2/id3v2synchdata.cpp
    mpeg/id3v2/id3v2tag.cpp
    mpeg/mpegfile.cpp
    mpeg/mpegheader.cpp
    mpeg/mpegproperties.cpp
    mpeg/xingheader.cpp
    ogg/flac/oggflacfile.cpp
    ogg/oggfile.cpp
    ogg/oggpage.cpp
    ogg/oggpageheader.cpp
    ogg/opus/opusfile.cpp
    ogg/opus/opusproperties.cpp
    ogg/speex/speexfile.cpp
    ogg/speex/speexproperties.cpp
    ogg/vorbis/vorbisfile.cpp
    ogg/vorbis/vorbisproperties.cpp
    ogg/xiphcomment.cpp
    riff/aiff/aifffile.cpp
    riff/aiff/aiffproperties.cpp
    riff/rifffile.cpp
    riff/wav/infotag.cpp
    riff/wav/wavfile.cpp
    riff/wav/wavproperties.cpp
    s3m/s3mfile.cpp
    s3m/s3mproperties.cpp
    tag.cpp
    tagunion.cpp
    tagutils.cpp
    toolkit/tbytevector.cpp
    toolkit/tbytevectorlist.cpp
    toolkit/tbytevectorstream.cpp
    toolkit/tdebug.cpp
    toolkit/tdebuglistener.cpp
    toolkit/tfile.cpp
    toolkit/tfilestream.cpp
    toolkit/tiostream.cpp
    toolkit/tpropertymap.cpp
    toolkit/trefcounter.cpp
    toolkit/tstring.cpp
    toolkit/tstringlist.cpp
    toolkit/tzlib.cpp
    toolkit/unicode.cpp
    trueaudio/trueaudiofile.cpp
    trueaudio/trueaudioproperties.cpp
    wavpack/wavpackfile.cpp
    wavpack/wavpackproperties.cpp
    xm/xmfile.cpp
    xm/xmproperties.cpp
)

add_library(tag STATIC ${tag_SOURCES})

nlc_target_add_supported_warning_options(tag CXX
    -Wno-delete-abstract-non-virtual-dtor
)


