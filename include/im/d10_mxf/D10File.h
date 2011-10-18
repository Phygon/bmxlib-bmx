/*
 * Copyright (C) 2011, British Broadcasting Corporation
 * All Rights Reserved.
 *
 * Author: Philip de Nier
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the British Broadcasting Corporation nor the names
 *       of its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __IM_D10_FILE_H__
#define __IM_D10_FILE_H__

#include <vector>

#include <libMXF++/MXF.h>

#include <im/d10_mxf/D10Track.h>
#include <im/d10_mxf/D10MPEGTrack.h>
#include <im/d10_mxf/D10PCMTrack.h>
#include <im/IMTypes.h>



namespace im
{


class D10File
{
public:
    friend class D10Track;

public:
    static D10File* OpenNew(std::string filename, mxfRational frame_rate);

public:
    virtual ~D10File();

    void SetClipName(std::string name);                                 // default ""
    void SetStartTimecode(Timecode start_timecode);                     // default 00:00:00:00, non-drop frame
    void SetHaveInputUserTimecode(bool enable);                         // default false (generated)
    void SetSoundSequenceOffset(uint8_t offset);                        // default determined from input
    void SetProductInfo(std::string company_name, std::string product_name, mxfProductVersion product_version,
                        std::string version, mxfUUID product_uid);
    void SetCreationDate(mxfTimestamp creation_date);                   // default generated ('now')
    void SetGenerationUID(mxfUUID generation_uid);                      // default generated
    void SetMaterialPackageUID(mxfUMID package_uid);                    // default generated
    void SetFileSourcePackageUID(mxfUMID package_uid);                  // default generated
    void ReserveHeaderMetadataSpace(uint32_t min_bytes);                // default 8192

public:
    D10Track* CreateTrack(D10EssenceType essence_type);

public:
    void PrepareHeaderMetadata();
    void PrepareWrite();
    void WriteUserTimecode(Timecode user_timecode);
    void WriteSamples(uint32_t track_index, const unsigned char *data, uint32_t size, uint32_t num_samples);
    void CompleteWrite();

public:
    mxfpp::HeaderMetadata* GetHeaderMetadata() const { return mHeaderMetadata; }
    mxfpp::DataModel* GetDataModel() const { return mDataModel; }

    mxfRational GetFrameRate() const { return mFrameRate; }

    Timecode GetStartTimecode() const { return mStartTimecode; }

    int64_t GetDuration() const { return mCPManager->GetDuration(); }

    uint32_t GetNumTracks() const { return mTracks.size(); }
    D10Track* GetTrack(uint32_t track_index);

private:
    D10File(mxfpp::File *mxf_file, mxfRational frame_rate);

    D10ContentPackageManager* GetContentPackageManager() const { return mCPManager; }

    void CreateHeaderMetadata();
    void CreateFile();

    void UpdatePackageMetadata();
    void UpdateTrackMetadata(mxfpp::GenericPackage *package, int64_t duration);

private:
    mxfpp::File *mMXFFile;

    std::string mClipName;
    mxfRational mFrameRate;
    Timecode mStartTimecode;
    std::string mCompanyName;
    std::string mProductName;
    mxfProductVersion mProductVersion;
    std::string mVersionString;
    mxfUUID mProductUID;
    uint32_t mReserveMinBytes;
    mxfTimestamp mCreationDate;
    mxfUUID mGenerationUID;
    mxfUMID mMaterialPackageUID;
    mxfUMID mFileSourcePackageUID;

    std::vector<D10Track*> mTracks;
    std::map<uint32_t, D10Track*> mTrackMap;
    D10MPEGTrack *mPictureTrack;
    D10PCMTrack *mFirstSoundTrack;
    uint8_t mSoundTrackCount;
    uint32_t mMaxSoundOutputChannelIndex;

    mxfUL mEssenceContainerUL;

    mxfpp::DataModel *mDataModel;
    mxfpp::HeaderMetadata *mHeaderMetadata;
    int64_t mHeaderMetadataStartPos;
    int64_t mHeaderMetadataEndPos;
    int64_t mCBEIndexTableStartPos;

    mxfpp::MaterialPackage *mMaterialPackage;
    mxfpp::SourcePackage *mFileSourcePackage;

    D10ContentPackageManager *mCPManager;
    mxfpp::IndexTableSegment *mIndexSegment;
};


};



#endif

