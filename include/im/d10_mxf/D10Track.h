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

#ifndef __IM_D10_TRACK_H__
#define __IM_D10_TRACK_H__


#include <libMXF++/MXF.h>

#include <im/mxf_helper/MXFDescriptorHelper.h>
#include <im/d10_mxf/D10ContentPackage.h>



namespace im
{


typedef enum
{
    D10_UNKNOWN_ESSENCE = 0,
    D10_MPEG_30,
    D10_MPEG_40,
    D10_MPEG_50,
    D10_PCM
} D10EssenceType;


class D10File;

class D10Track
{
public:
    friend class D10File;

public:
    static bool IsSupported(D10EssenceType essence_type, mxfRational sample_rate);

    static MXFDescriptorHelper::EssenceType ConvertEssenceType(D10EssenceType d10_essence_type);
    static D10EssenceType ConvertEssenceType(MXFDescriptorHelper::EssenceType mh_essence_type);

    static D10Track* Create(D10File *file, uint32_t track_index, mxfRational frame_rate, D10EssenceType essence_type);

public:
    virtual ~D10Track();

public:
    void WriteSamples(const unsigned char *data, uint32_t size, uint32_t num_samples);

public:
    uint32_t GetTrackIndex() const { return mTrackIndex; }

    bool IsPicture() const { return mIsPicture; }
    D10EssenceType GetEssenceType() const { return mEssenceType; }

    uint32_t GetSampleSize();

protected:
    D10Track(D10File *file, uint32_t track_index, mxfRational frame_rate, D10EssenceType essence_type);

protected:
    virtual void PrepareWrite() = 0;
    virtual void WriteSamplesInt(const unsigned char *data, uint32_t size, uint32_t num_samples);

protected:
    D10File *mD10File;
    uint32_t mTrackIndex;
    mxfRational mFrameRate;
    D10ContentPackageManager *mCPManager;
    bool mIsPicture;

    D10EssenceType mEssenceType;
    MXFDescriptorHelper *mDescriptorHelper;
};


};



#endif

