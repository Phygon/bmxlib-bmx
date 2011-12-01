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

#ifndef __IM_CLIP_WRITER_TRACK_H__
#define __IM_CLIP_WRITER_TRACK_H__


#include <im/as02/AS02Track.h>
#include <im/as11/AS11Track.h>
#include <im/mxf_op1a/OP1ATrack.h>
#include <im/avid_mxf/AvidTrack.h>
#include <im/d10_mxf/D10Track.h>



namespace im
{


typedef enum
{
    CW_UNKNOWN_ESSENCE = 0,
    CW_IEC_DV25,
    CW_DVBASED_DV25,
    CW_DV50,
    CW_DV100_1080I,
    CW_DV100_1080P,
    CW_DV100_720P,
    CW_D10_30,
    CW_D10_40,
    CW_D10_50,
    CW_AVCI100_1080I,
    CW_AVCI100_1080P,
    CW_AVCI100_720P,
    CW_AVCI50_1080I,
    CW_AVCI50_1080P,
    CW_AVCI50_720P,
    CW_UNC_SD,
    CW_UNC_HD_1080I,
    CW_UNC_HD_1080P,
    CW_UNC_HD_720P,
    CW_AVID_10BIT_UNC_SD,
    CW_AVID_10BIT_UNC_HD_1080I,
    CW_AVID_10BIT_UNC_HD_1080P,
    CW_AVID_10BIT_UNC_HD_720P,
    CW_MPEG2LG_422P_HL_1080I,
    CW_MPEG2LG_422P_HL_1080P,
    CW_MPEG2LG_422P_HL_720P,
    CW_MPEG2LG_MP_HL_1080I,
    CW_MPEG2LG_MP_HL_1080P,
    CW_MPEG2LG_MP_HL_720P,
    CW_MPEG2LG_MP_H14_1080I,
    CW_MPEG2LG_MP_H14_1080P,
    CW_MJPEG_2_1,
    CW_MJPEG_3_1,
    CW_MJPEG_10_1,
    CW_MJPEG_20_1,
    CW_MJPEG_4_1M,
    CW_MJPEG_10_1M,
    CW_MJPEG_15_1S,
    CW_VC3_1080P_1235,
    CW_VC3_1080P_1237,
    CW_VC3_1080P_1238,
    CW_VC3_1080I_1241,
    CW_VC3_1080I_1242,
    CW_VC3_1080I_1243,
    CW_VC3_720P_1250,
    CW_VC3_720P_1251,
    CW_VC3_720P_1252,
    CW_VC3_1080P_1253,
    CW_PCM
} ClipWriterEssenceType;


typedef enum
{
    CW_UNKNOWN_CLIP_TYPE = 0,
    CW_AS02_CLIP_TYPE,
    CW_AS11_OP1A_CLIP_TYPE,
    CW_AS11_D10_CLIP_TYPE,
    CW_OP1A_CLIP_TYPE,
    CW_AVID_CLIP_TYPE,
    CW_D10_CLIP_TYPE,
} ClipWriterType;



class ClipWriterTrack
{
public:
    static bool IsSupported(ClipWriterType clip_type, ClipWriterEssenceType essence_type, bool is_mpeg2lg_720p,
                            Rational sample_rate);

    static int ConvertEssenceType(ClipWriterType clip_type, ClipWriterEssenceType essence_type);

    static std::string EssenceTypeToString(ClipWriterEssenceType essence_type);

public:
    ClipWriterTrack(ClipWriterEssenceType essence_type, AS02Track *track);
    ClipWriterTrack(ClipWriterEssenceType essence_type, AS11Track *track);
    ClipWriterTrack(ClipWriterEssenceType essence_type, OP1ATrack *track);
    ClipWriterTrack(ClipWriterEssenceType essence_type, AvidTrack *track);
    ClipWriterTrack(ClipWriterEssenceType essence_type, D10Track *track);
    virtual ~ClipWriterTrack();

public:
    // Picture properties
    void SetAspectRatio(Rational aspect_ratio);     // default 16/9
    void SetComponentDepth(uint32_t depth);         // default 8; alternative is 10
    void SetSampleSize(uint32_t size);              // D10 sample size
    void SetAVCIMode(AVCIMode mode);                // default depends on track type
    void SetAFD(uint8_t afd);                       // default not set
    void SetInputHeight(uint32_t height);           // uncompressed; default 0

    // Sound properties
    void SetSamplingRate(Rational sampling_rate);   // default 48000/1
    void SetQuantizationBits(uint32_t bits);        // default 16
    void SetChannelCount(uint32_t count);           // default 1
    void SetLocked(bool locked);                    // default not set
    void SetAudioRefLevel(int8_t level);            // default not set
    void SetDialNorm(int8_t dial_norm);             // default not set
    void SetSequenceOffset(uint8_t offset);         // default D10 determined from input or not set

public:
    void WriteSamples(const unsigned char *data, uint32_t size, uint32_t num_samples);

public:
    bool IsPicture() const;

    uint32_t GetSampleSize() const;
    uint32_t GetInputSampleSize() const;
    uint32_t GetAVCISampleWithoutHeaderSize() const;
    bool IsSingleField() const;

    std::vector<uint32_t> GetShiftedSampleSequence() const;

public:
    ClipWriterType GetClipType() const            { return mClipType; }
    ClipWriterEssenceType GetEssenceType() const  { return mEssenceType; }

    AS02Track* GetAS02Track() const { return mAS02Track; }
    AS11Track* GetAS11Track() const { return mAS11Track; }
    OP1ATrack* GetOP1ATrack() const { return mOP1ATrack; }
    AvidTrack* GetAvidTrack() const { return mAvidTrack; }
    D10Track* GetD10Track()   const { return mD10Track; }

private:
    ClipWriterType mClipType;
    ClipWriterEssenceType mEssenceType;
    AS02Track *mAS02Track;
    AS11Track *mAS11Track;
    OP1ATrack *mOP1ATrack;
    AvidTrack *mAvidTrack;
    D10Track *mD10Track;
};


};



#endif
