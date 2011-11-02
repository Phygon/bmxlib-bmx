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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <im/mxf_helper/AVCIMXFDescriptorHelper.h>
#include <im/MXFUtils.h>
#include <im/IMException.h>
#include <im/Logging.h>

#include <mxf/mxf_avid_labels_and_keys.h>

using namespace std;
using namespace im;
using namespace mxfpp;



typedef struct
{
    mxfUL pc_label;
    MXFDescriptorHelper::EssenceType essence_type;
    mxfRational sample_rate;
    uint32_t frame_size; // includes size of sequence and picture parameter sets (512 bytes)
    int32_t avid_resolution_id;
} SupportedEssence;

static const SupportedEssence SUPPORTED_ESSENCE[] =
{
    {MXF_CMDEF_L(AVCI_100_1080_50_I),   MXFDescriptorHelper::AVCI100_1080I,   {25, 1},        568832,  0x0d58},
    {MXF_CMDEF_L(AVCI_100_1080_60_I),   MXFDescriptorHelper::AVCI100_1080I,   {30000, 1001},  472576,  0x0},
    {MXF_CMDEF_L(AVCI_100_1080_25_P),   MXFDescriptorHelper::AVCI100_1080P,   {25, 1},        568832,  0x0d59},
    {MXF_CMDEF_L(AVCI_100_1080_30_P),   MXFDescriptorHelper::AVCI100_1080P,   {30000, 1001},  472576,  0x0},
    {MXF_CMDEF_L(AVCI_100_720_50_P),    MXFDescriptorHelper::AVCI100_720P,    {50, 1},        284672,  0x0d52},
    {MXF_CMDEF_L(AVCI_100_720_50_P),    MXFDescriptorHelper::AVCI100_720P,    {25, 1},        284672,  0x0d54},
    {MXF_CMDEF_L(AVCI_100_720_60_P),    MXFDescriptorHelper::AVCI100_720P,    {60000, 1001},  236544,  0x0},
    {MXF_CMDEF_L(AVCI_100_720_60_P),    MXFDescriptorHelper::AVCI100_720P,    {30000, 1001},  236544,  0x0},
    {MXF_CMDEF_L(AVCI_50_1080_50_I),    MXFDescriptorHelper::AVCI50_1080I,    {25, 1},        281088,  0x0d4e},
    {MXF_CMDEF_L(AVCI_50_1080_60_I),    MXFDescriptorHelper::AVCI50_1080I,    {30000, 1001},  232960,  0x0},
    {MXF_CMDEF_L(AVCI_50_1080_25_P),    MXFDescriptorHelper::AVCI50_1080P,    {25, 1},        281088,  0x0d4f},
    {MXF_CMDEF_L(AVCI_50_1080_30_P),    MXFDescriptorHelper::AVCI50_1080P,    {30000, 1001},  232960,  0x0},
    {MXF_CMDEF_L(AVCI_50_720_50_P),     MXFDescriptorHelper::AVCI50_720P,     {50, 1},        140800,  0x0d48},
    {MXF_CMDEF_L(AVCI_50_720_50_P),     MXFDescriptorHelper::AVCI50_720P,     {25, 1},        140800,  0x0d4a},
    {MXF_CMDEF_L(AVCI_50_720_60_P),     MXFDescriptorHelper::AVCI50_720P,     {60000, 1001},  116736,  0x0},
    {MXF_CMDEF_L(AVCI_50_720_60_P),     MXFDescriptorHelper::AVCI50_720P,     {30000, 1001},  116736,  0x0},
};

#define SUPPORTED_ESSENCE_SIZE  (sizeof(SUPPORTED_ESSENCE) / sizeof(SupportedEssence))



MXFDescriptorHelper::EssenceType AVCIMXFDescriptorHelper::IsSupported(FileDescriptor *file_descriptor,
                                                                      mxfUL alternative_ec_label)
{
    mxfUL ec_label = file_descriptor->getEssenceContainer();
    if (!mxf_equals_ul_mod_regver(&ec_label, &MXF_EC_L(AVCIFrameWrapped)) &&
        !mxf_equals_ul_mod_regver(&ec_label, &MXF_EC_L(AVCIClipWrapped)) &&
        !(mxf_equals_ul_mod_regver(&ec_label, &MXF_EC_L(AvidAAFKLVEssenceContainer)) &&
            mxf_equals_ul_mod_regver(&alternative_ec_label, &MXF_EC_L(AVCIClipWrapped))))
    {
        return MXFDescriptorHelper::UNKNOWN_ESSENCE;
    }

    mxfRational sample_rate = file_descriptor->getSampleRate();

    GenericPictureEssenceDescriptor *pic_descriptor = dynamic_cast<GenericPictureEssenceDescriptor*>(file_descriptor);
    if (!pic_descriptor || !pic_descriptor->havePictureEssenceCoding())
        return MXFDescriptorHelper::UNKNOWN_ESSENCE;

    mxfUL pc_label = pic_descriptor->getPictureEssenceCoding();
    size_t i;
    for (i = 0; i < SUPPORTED_ESSENCE_SIZE; i++) {
        if (mxf_equals_ul_mod_regver(&pc_label, &SUPPORTED_ESSENCE[i].pc_label) &&
            SUPPORTED_ESSENCE[i].sample_rate == sample_rate)
        {
            return SUPPORTED_ESSENCE[i].essence_type;
        }
    }

    return MXFDescriptorHelper::UNKNOWN_ESSENCE;
}

bool AVCIMXFDescriptorHelper::IsSupported(EssenceType essence_type)
{
    size_t i;
    for (i = 0; i < SUPPORTED_ESSENCE_SIZE; i++) {
        if (essence_type == SUPPORTED_ESSENCE[i].essence_type)
            return true;
    }

    return false;
}

AVCIMXFDescriptorHelper::AVCIMXFDescriptorHelper()
: PictureMXFDescriptorHelper()
{
    mEssenceIndex = 0;
    mEssenceType = SUPPORTED_ESSENCE[0].essence_type;
}

AVCIMXFDescriptorHelper::~AVCIMXFDescriptorHelper()
{
}

void AVCIMXFDescriptorHelper::Initialize(FileDescriptor *file_descriptor, mxfUL alternative_ec_label)
{
    IM_ASSERT(IsSupported(file_descriptor, alternative_ec_label));

    PictureMXFDescriptorHelper::Initialize(file_descriptor, alternative_ec_label);

    mxfRational sample_rate = file_descriptor->getSampleRate();

    mxfUL ec_label = file_descriptor->getEssenceContainer();
    mFrameWrapped = mxf_equals_ul_mod_regver(&ec_label, &MXF_EC_L(AVCIFrameWrapped));

    GenericPictureEssenceDescriptor *pic_descriptor = dynamic_cast<GenericPictureEssenceDescriptor*>(file_descriptor);
    mxfUL pc_label = pic_descriptor->getPictureEssenceCoding();
    size_t i;
    for (i = 0; i < SUPPORTED_ESSENCE_SIZE; i++) {
        if (mxf_equals_ul_mod_regver(&pc_label, &SUPPORTED_ESSENCE[i].pc_label) &&
            SUPPORTED_ESSENCE[i].sample_rate == sample_rate)
        {
            mEssenceIndex = i;
            mEssenceType = SUPPORTED_ESSENCE[i].essence_type;
            mAvidResolutionId = SUPPORTED_ESSENCE[i].avid_resolution_id;
            break;
        }
    }
}

void AVCIMXFDescriptorHelper::SetEssenceType(EssenceType essence_type)
{
    IM_ASSERT(!mFileDescriptor);

    PictureMXFDescriptorHelper::SetEssenceType(essence_type);

    UpdateEssenceIndex();
}

void AVCIMXFDescriptorHelper::SetSampleRate(mxfRational sample_rate)
{
    IM_ASSERT(!mFileDescriptor);

    PictureMXFDescriptorHelper::SetSampleRate(sample_rate);

    UpdateEssenceIndex();
}

FileDescriptor* AVCIMXFDescriptorHelper::CreateFileDescriptor(mxfpp::HeaderMetadata *header_metadata)
{
    UpdateEssenceIndex();

    if (mFlavour == AVID_FLAVOUR)
        mFileDescriptor = new CDCIEssenceDescriptor(header_metadata);
    else
        mFileDescriptor = new MPEGVideoDescriptor(header_metadata);
    UpdateFileDescriptor();
    return mFileDescriptor;
}

void AVCIMXFDescriptorHelper::UpdateFileDescriptor()
{
    PictureMXFDescriptorHelper::UpdateFileDescriptor();

    CDCIEssenceDescriptor *cdci_descriptor = dynamic_cast<CDCIEssenceDescriptor*>(mFileDescriptor);
    IM_ASSERT(cdci_descriptor);
    MPEGVideoDescriptor *mpeg_descriptor = dynamic_cast<MPEGVideoDescriptor*>(mFileDescriptor);
    IM_ASSERT(mFlavour == AVID_FLAVOUR || mpeg_descriptor);

    cdci_descriptor->setPictureEssenceCoding(SUPPORTED_ESSENCE[mEssenceIndex].pc_label);
    switch (mEssenceType)
    {
        case AVCI100_1080I:
        case AVCI100_1080P:
        case AVCI50_1080I:
        case AVCI50_1080P:
            cdci_descriptor->setSignalStandard(MXF_SIGNAL_STANDARD_SMPTE274M);
            break;
        case AVCI100_720P:
        case AVCI50_720P:
            cdci_descriptor->setSignalStandard(MXF_SIGNAL_STANDARD_SMPTE296M);
            break;
        default:
            IM_ASSERT(false);
    }
    switch (mEssenceType)
    {
        case AVCI100_1080I:
        case AVCI50_1080I:
            cdci_descriptor->setFrameLayout(MXF_SEPARATE_FIELDS);
            break;
        case AVCI100_1080P:
        case AVCI100_720P:
        case AVCI50_1080P:
        case AVCI50_720P:
            cdci_descriptor->setFrameLayout(MXF_FULL_FRAME);
            break;
        default:
            IM_ASSERT(false);
    }
    SetColorSiting(0x00); // coSiting
    cdci_descriptor->setComponentDepth(10);
    cdci_descriptor->setBlackRefLevel(64);
    cdci_descriptor->setWhiteReflevel(940);
    cdci_descriptor->setColorRange(897);
    SetCodingEquations(ITUR_BT709_CODING_EQ);
    switch (mEssenceType)
    {
        case AVCI100_1080I:
        case AVCI50_1080I:
            cdci_descriptor->setStoredWidth(1920);
            cdci_descriptor->setStoredHeight(540);
            cdci_descriptor->appendVideoLineMap(21);
            cdci_descriptor->appendVideoLineMap(584);
            break;
        case AVCI100_1080P:
        case AVCI50_1080P:
            cdci_descriptor->setStoredWidth(1920);
            cdci_descriptor->setStoredHeight(1080);
            cdci_descriptor->appendVideoLineMap(42);
            cdci_descriptor->appendVideoLineMap(0);
            break;
        case AVCI100_720P:
        case AVCI50_720P:
            cdci_descriptor->setStoredWidth(1280);
            cdci_descriptor->setStoredHeight(720);
            cdci_descriptor->appendVideoLineMap(26);
            cdci_descriptor->appendVideoLineMap(0);
            break;
        default:
            IM_ASSERT(false);
    }
    cdci_descriptor->setDisplayWidth(cdci_descriptor->getStoredWidth());
    cdci_descriptor->setDisplayHeight(cdci_descriptor->getStoredHeight());
    cdci_descriptor->setSampledWidth(cdci_descriptor->getStoredWidth());
    cdci_descriptor->setSampledHeight(cdci_descriptor->getStoredHeight());
    switch (mEssenceType)
    {
        case AVCI100_1080I:
        case AVCI100_1080P:
        case AVCI100_720P:
            cdci_descriptor->setHorizontalSubsampling(2);
            cdci_descriptor->setVerticalSubsampling(1);
            break;
        case AVCI50_1080I:
        case AVCI50_1080P:
        case AVCI50_720P:
            cdci_descriptor->setHorizontalSubsampling(2);
            cdci_descriptor->setVerticalSubsampling(2);
            break;
        default:
            IM_ASSERT(false);
    }
}

uint32_t AVCIMXFDescriptorHelper::GetSampleSize()
{
    return SUPPORTED_ESSENCE[mEssenceIndex].frame_size;
}

uint32_t AVCIMXFDescriptorHelper::GetSampleWithoutHeaderSize()
{
    return SUPPORTED_ESSENCE[mEssenceIndex].frame_size - 512;
}

mxfUL AVCIMXFDescriptorHelper::ChooseEssenceContainerUL() const
{
    if (mFrameWrapped)
        return MXF_EC_L(AVCIFrameWrapped);
    else
        return MXF_EC_L(AVCIClipWrapped);
}

void AVCIMXFDescriptorHelper::UpdateEssenceIndex()
{
    size_t i;
    for (i = 0; i < SUPPORTED_ESSENCE_SIZE; i++) {
        if (SUPPORTED_ESSENCE[i].essence_type == mEssenceType &&
            SUPPORTED_ESSENCE[i].sample_rate == mSampleRate)
        {
            mEssenceIndex = i;
            mAvidResolutionId = SUPPORTED_ESSENCE[i].avid_resolution_id;
            break;
        }
    }
    IM_CHECK(i < SUPPORTED_ESSENCE_SIZE);
}
