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

#ifndef __IM_RAW_ESSENCE_READER_H__
#define __IM_RAW_ESSENCE_READER_H__


#include <cstdio>

#include <im/essence_parser/EssenceParser.h>
#include <im/ByteArray.h>



namespace im
{


class RawEssenceReader
{
public:
    RawEssenceReader(FILE *raw_input);
    virtual ~RawEssenceReader();

    void SeekToOffset(int64_t offset);
    void SetMaxReadLength(int64_t len);

    void SetFixedSampleSize(uint32_t size);

    virtual void SetEssenceParser(EssenceParser *essence_parser);
    void SetCheckMaxSampleSize(uint32_t size);

    uint32_t GetFixedSampleSize() const { return mFixedSampleSize; }
    EssenceParser* GetEssenceParser() const { return mEssenceParser; }

public:
    virtual uint32_t ReadSamples(uint32_t num_samples);

    virtual const unsigned char* GetSampleData() const { return mSampleData.GetBytes(); }
    uint32_t GetSampleDataSize() const { return mSampleDataSize; }
    uint32_t GetNumSamples() const { return mNumSamples; }
    uint32_t GetSampleSize() const;
    int64_t GetSampleDataOffset() const { return mSampleDataOffset; }

    int64_t GetNumRemFixedSizeSamples() const;

    virtual void Reset();

protected:
    bool ReadAndParseSample();
    bool ReadBytes(uint32_t size, uint32_t *num_read_out);
    void ShiftSampleData(uint32_t to_offset, uint32_t from_offset, uint32_t size);

protected:
    FILE *mRawInput;

    int64_t mStartOffset;
    int64_t mMaxReadLength;
    int64_t mTotalReadLength;
    uint32_t mMaxSampleSize;

    uint32_t mFixedSampleSize;
    EssenceParser *mEssenceParser;

    ByteArray mSampleData;
    uint32_t mSampleDataSize;
    uint32_t mNumSamples;
    bool mReadFirstSample;
    bool mLastSampleRead;
    int64_t mSampleDataOffset;
};


};



#endif
