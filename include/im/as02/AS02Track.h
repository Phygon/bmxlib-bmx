/*
 * Copyright (C) 2010, British Broadcasting Corporation
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

#ifndef __IM_AS02_TRACK_H__
#define __IM_AS02_TRACK_H__


#include <im/as02/AS02Bundle.h>
#include <im/mxf_helper/MXFDescriptorHelper.h>
#include <im/MD5.h>



namespace im
{


typedef enum
{
    AS02_UNKNOWN_ESSENCE = 0,
    AS02_IEC_DV25,
    AS02_DVBASED_DV25,
    AS02_DV50,
    AS02_DV100_1080I,
    AS02_DV100_720P,
    AS02_D10_30,
    AS02_D10_40,
    AS02_D10_50,
    AS02_AVCI100_1080I,
    AS02_AVCI100_1080P,
    AS02_AVCI100_720P,
    AS02_AVCI50_1080I,
    AS02_AVCI50_1080P,
    AS02_AVCI50_720P,
    AS02_UNC_SD,
    AS02_UNC_HD_1080I,
    AS02_UNC_HD_1080P,
    AS02_UNC_HD_720P,
    AS02_MPEG2LG_422P_HL,
    AS02_MPEG2LG_MP_HL,
    AS02_MPEG2LG_MP_H14,
    AS02_PCM
} AS02EssenceType;


class AS02Clip;

class AS02Track
{
public:
    static bool IsSupported(AS02EssenceType essence_type, bool is_mpeg2lg_720p, mxfRational sample_rate);

    static MXFDescriptorHelper::EssenceType ConvertEssenceType(AS02EssenceType as02_essence_type);
    static AS02EssenceType ConvertEssenceType(MXFDescriptorHelper::EssenceType mh_essence_type);

    static AS02Track* OpenNew(AS02Clip *clip, std::string filepath, std::string rel_uri, uint32_t track_index,
                              AS02EssenceType essence_type);

public:
    virtual ~AS02Track();

    void SetFileSourcePackageUID(mxfUMID package_uid);
    void SetMaterialTrackNumber(uint32_t track_number);
    void SetMICType(MICType type);
    void SetMICScope(MICScope scope);

    void SetLowerLevelSourcePackage(mxfpp::SourcePackage *package, uint32_t track_id, std::string uri);
    void SetLowerLevelSourcePackage(mxfUMID package_uid, uint32_t track_id);

    void SetOutputStartOffset(int64_t offset);
    void SetOutputEndOffset(int64_t offset);

public:
    void PrepareWrite();
    virtual void WriteSamples(const unsigned char *data, uint32_t size, uint32_t num_samples) = 0;
    void CompleteWrite();

    void UpdatePackageMetadata(mxfpp::GenericPackage *package);

public:
    uint32_t GetTrackIndex() const { return mTrackIndex; }
    uint32_t GetClipTrackNumber() const { return mClipTrackNumber; }
    std::string GetRelativeURL() const { return mRelativeURL; }
    AS02EssenceType GetEssenceType() const { return mEssenceType; }
    virtual mxfUL GetEssenceContainerUL() const;
    mxfpp::SourcePackage* GetFileSourcePackage() const { return mFileSourcePackage; }
    bool IsPicture() const { return mIsPicture; }
    mxfRational GetSampleRate() const;
    std::pair<mxfUMID, uint32_t> GetSourceReference() const;

    uint32_t GetSampleSize();

    int64_t GetOutputDuration(bool clip_frame_rate) const;
    int64_t GetDuration() const;
    int64_t GetContainerDuration() const;

protected:
    AS02Track(AS02Clip *clip, uint32_t track_index, AS02EssenceType essence_type, mxfpp::File *mxf_file,
              std::string rel_uri);

    virtual bool HaveCBEIndexTable() { return mSampleSize > 0; }
    virtual void WriteCBEIndexTable(mxfpp::Partition *partition);
    virtual bool HaveVBEIndexEntries() { return true; }
    virtual void WriteVBEIndexTable(mxfpp::Partition *partition) { (void)partition; };
    virtual void PreSampleWriting() {}
    virtual void PostSampleWriting(mxfpp::Partition *partition) { (void)partition; }

protected:
    int64_t ContainerDurationToClipFrameRate(int64_t length) const;
    mxfRational& GetVideoFrameRate() const;

    void WriteCBEIndexTable(mxfpp::Partition *partition, uint32_t edit_unit_size, mxfpp::IndexTableSegment *&mIndexSegment);

    void UpdateEssenceOnlyChecksum(const unsigned char *data, uint32_t size);

protected:
    bool mIsPicture;
    uint32_t mTrackNumber;
    uint32_t mIndexSID;
    uint32_t mBodySID;
    uint8_t mLLen;
    uint32_t mKAGSize;
    mxfKey mEssenceElementKey;

    AS02EssenceType mEssenceType;
    MXFDescriptorHelper *mDescriptorHelper;

    uint32_t mSampleSize;

    mxfUMID mMaterialPackageUID;
    mxfUMID mFileSourcePackageUID;

    int64_t mContainerDuration;
    int64_t mContainerSize;
    int64_t mOutputStartOffset;
    int64_t mOutputEndOffset;

    mxfpp::File *mMXFFile;

private:
    void CreateHeaderMetadata();
    void CreateFile();

private:
    AS02ManifestFile *mManifestFile;
    AS02Clip *mClip;
    uint32_t mTrackIndex;
    uint32_t mClipTrackNumber;
    std::string mRelativeURL;

    mxfpp::DataModel *mDataModel;
    mxfpp::HeaderMetadata *mHeaderMetadata;
    int64_t mHeaderMetadataStartPos;
    int64_t mHeaderMetadataEndPos;
    mxfpp::IndexTableSegment *mCBEIndexSegment;
    int64_t mIndexTableStartPos;

    mxfpp::MaterialPackage* mMaterialPackage;
    mxfpp::SourcePackage* mFileSourcePackage;

    bool mHaveLowerLevelSourcePackage;
    mxfpp::SourcePackage *mLowerLevelSourcePackage;
    mxfUMID mLowerLevelSourcePackageUID;
    uint32_t mLowerLevelTrackId;
    std::string mLowerLevelURI;

    MD5Context mEssenceOnlyMD5Context;
};


};



#endif

