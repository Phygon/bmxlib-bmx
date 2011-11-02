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

#include <cstring>

#include <im/mxf_helper/VC3MXFDescriptorHelper.h>
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
    int32_t resolution_id;
    uint32_t component_depth;
    uint32_t frame_size;
    uint32_t stored_width;
    uint32_t stored_height;
    int32_t video_line_map[2];
    uint8_t frame_layout;
    uint8_t signal_standard;
    mxfUL avid_pc_label;
    mxfUL avid_ec_label;
} SupportedEssence;

static const SupportedEssence SUPPORTED_ESSENCE[] =
{
    {MXF_CMDEF_L(VC3_1080P_1235),  MXFDescriptorHelper::VC3_1080P_1235, 1235,   10, 917504, 1920,   1080,   {42, 0},    MXF_FULL_FRAME,       MXF_SIGNAL_STANDARD_SMPTE274M,    MXF_CMDEF_L(DNxHD), MXF_EC_L(DNxHD1080p1235ClipWrapped)},
    {MXF_CMDEF_L(VC3_1080P_1237),  MXFDescriptorHelper::VC3_1080P_1237, 1237,   8,  606208, 1920,   1080,   {42, 0},    MXF_FULL_FRAME,       MXF_SIGNAL_STANDARD_SMPTE274M,    MXF_CMDEF_L(DNxHD), MXF_EC_L(DNxHD1080p1237ClipWrapped)},
    {MXF_CMDEF_L(VC3_1080P_1238),  MXFDescriptorHelper::VC3_1080P_1238, 1238,   8,  917504, 1920,   1080,   {42, 0},    MXF_FULL_FRAME,       MXF_SIGNAL_STANDARD_SMPTE274M,    MXF_CMDEF_L(DNxHD), MXF_EC_L(DNxHD1080p1238ClipWrapped)},
    {MXF_CMDEF_L(VC3_1080I_1241),  MXFDescriptorHelper::VC3_1080I_1241, 1241,   10, 917504, 1920,   540,    {21, 584},  MXF_SEPARATE_FIELDS,  MXF_SIGNAL_STANDARD_SMPTE274M,    MXF_CMDEF_L(DNxHD), MXF_EC_L(DNxHD1080i1241ClipWrapped)},
    {MXF_CMDEF_L(VC3_1080I_1242),  MXFDescriptorHelper::VC3_1080I_1242, 1242,   8,  606208, 1920,   540,    {21, 584},  MXF_SEPARATE_FIELDS,  MXF_SIGNAL_STANDARD_SMPTE274M,    MXF_CMDEF_L(DNxHD), MXF_EC_L(DNxHD1080i1242ClipWrapped)},
    {MXF_CMDEF_L(VC3_1080I_1243),  MXFDescriptorHelper::VC3_1080I_1243, 1243,   8,  917504, 1920,   540,    {21, 584},  MXF_SEPARATE_FIELDS,  MXF_SIGNAL_STANDARD_SMPTE274M,    MXF_CMDEF_L(DNxHD), MXF_EC_L(DNxHD1080i1243ClipWrapped)},
    {MXF_CMDEF_L(VC3_720P_1250),   MXFDescriptorHelper::VC3_720P_1250,  1250,   10, 458752, 1280,   720,    {26, 0},    MXF_FULL_FRAME,       MXF_SIGNAL_STANDARD_SMPTE296M,    MXF_CMDEF_L(DNxHD), MXF_EC_L(DNxHD720p1250ClipWrapped)},
    {MXF_CMDEF_L(VC3_720P_1251),   MXFDescriptorHelper::VC3_720P_1251,  1251,   8,  458752, 1280,   720,    {26, 0},    MXF_FULL_FRAME,       MXF_SIGNAL_STANDARD_SMPTE296M,    MXF_CMDEF_L(DNxHD), MXF_EC_L(DNxHD720p1251ClipWrapped)},
    {MXF_CMDEF_L(VC3_720P_1252),   MXFDescriptorHelper::VC3_720P_1252,  1252,   8,  303104, 1280,   720,    {26, 0},    MXF_FULL_FRAME,       MXF_SIGNAL_STANDARD_SMPTE296M,    MXF_CMDEF_L(DNxHD), MXF_EC_L(DNxHD720p1252ClipWrapped)},
    {MXF_CMDEF_L(VC3_1080P_1253),  MXFDescriptorHelper::VC3_1080P_1253, 1253,   8,  188416, 1920,   1080,   {42, 0},    MXF_FULL_FRAME,       MXF_SIGNAL_STANDARD_SMPTE274M,    MXF_CMDEF_L(DNxHD), MXF_EC_L(DNxHD1080p1253ClipWrapped)},
};

#define SUPPORTED_ESSENCE_SIZE  (sizeof(SUPPORTED_ESSENCE) / sizeof(SupportedEssence))



MXFDescriptorHelper::EssenceType VC3MXFDescriptorHelper::IsSupported(FileDescriptor *file_descriptor,
                                                                     mxfUL alternative_ec_label)
{
    mxfUL ec_label = file_descriptor->getEssenceContainer();
    if (!mxf_equals_ul_mod_regver(&ec_label, &MXF_EC_L(VC3FrameWrapped)) &&
        !mxf_equals_ul_mod_regver(&ec_label, &MXF_EC_L(VC3ClipWrapped)) &&
        !(mxf_equals_ul_mod_regver(&ec_label, &MXF_EC_L(AvidAAFKLVEssenceContainer)) &&
            mxf_equals_ul_mod_regver(&alternative_ec_label, &MXF_EC_L(VC3ClipWrapped))))
    {
        return MXFDescriptorHelper::UNKNOWN_ESSENCE;
    }
    
    GenericPictureEssenceDescriptor *pic_descriptor = dynamic_cast<GenericPictureEssenceDescriptor*>(file_descriptor);
    if (!pic_descriptor || !pic_descriptor->havePictureEssenceCoding())
        return MXFDescriptorHelper::UNKNOWN_ESSENCE;

    mxfUL pc_label = pic_descriptor->getPictureEssenceCoding();
    size_t i;
    for (i = 0; i < SUPPORTED_ESSENCE_SIZE; i++) {
        if (mxf_equals_ul_mod_regver(&pc_label, &SUPPORTED_ESSENCE[i].pc_label))
            return SUPPORTED_ESSENCE[i].essence_type;
    }

    return MXFDescriptorHelper::UNKNOWN_ESSENCE;
}

bool VC3MXFDescriptorHelper::IsSupported(EssenceType essence_type)
{
    size_t i;
    for (i = 0; i < SUPPORTED_ESSENCE_SIZE; i++) {
        if (essence_type == SUPPORTED_ESSENCE[i].essence_type)
            return true;
    }

    return false;
}

VC3MXFDescriptorHelper::VC3MXFDescriptorHelper()
: PictureMXFDescriptorHelper()
{
    mEssenceIndex = 0;
    mEssenceType = SUPPORTED_ESSENCE[0].essence_type;
}

VC3MXFDescriptorHelper::~VC3MXFDescriptorHelper()
{
}

void VC3MXFDescriptorHelper::Initialize(FileDescriptor *file_descriptor, mxfUL alternative_ec_label)
{
    IM_ASSERT(IsSupported(file_descriptor, alternative_ec_label));

    PictureMXFDescriptorHelper::Initialize(file_descriptor, alternative_ec_label);

    mxfUL ec_label = file_descriptor->getEssenceContainer();
    mFrameWrapped = mxf_equals_ul_mod_regver(&ec_label, &MXF_EC_L(VC3FrameWrapped));

    GenericPictureEssenceDescriptor *pic_descriptor = dynamic_cast<GenericPictureEssenceDescriptor*>(file_descriptor);
    mxfUL pc_label = pic_descriptor->getPictureEssenceCoding();
    size_t i;
    for (i = 0; i < SUPPORTED_ESSENCE_SIZE; i++) {
        if (mxf_equals_ul_mod_regver(&pc_label, &SUPPORTED_ESSENCE[i].pc_label)) {
            mEssenceIndex = i;
            mEssenceType = SUPPORTED_ESSENCE[i].essence_type;
            mAvidResolutionId = SUPPORTED_ESSENCE[i].resolution_id;
            break;
        }
    }
}

void VC3MXFDescriptorHelper::SetEssenceType(EssenceType essence_type)
{
    IM_ASSERT(!mFileDescriptor);

    size_t i;
    for (i = 0; i < SUPPORTED_ESSENCE_SIZE; i++) {
        if (SUPPORTED_ESSENCE[i].essence_type == essence_type) {
            mEssenceIndex = i;
            mAvidResolutionId = SUPPORTED_ESSENCE[i].resolution_id;
            break;
        }
    }
    IM_CHECK(i < SUPPORTED_ESSENCE_SIZE);

    PictureMXFDescriptorHelper::SetEssenceType(essence_type);
}

FileDescriptor* VC3MXFDescriptorHelper::CreateFileDescriptor(mxfpp::HeaderMetadata *header_metadata)
{
    mFileDescriptor = new CDCIEssenceDescriptor(header_metadata);
    UpdateFileDescriptor();
    return mFileDescriptor;
}

void VC3MXFDescriptorHelper::UpdateFileDescriptor()
{
    PictureMXFDescriptorHelper::UpdateFileDescriptor();

    CDCIEssenceDescriptor *cdci_descriptor = dynamic_cast<CDCIEssenceDescriptor*>(mFileDescriptor);
    IM_ASSERT(cdci_descriptor);

    if (mFlavour == AVID_FLAVOUR)
        cdci_descriptor->setPictureEssenceCoding(SUPPORTED_ESSENCE[mEssenceIndex].avid_pc_label);
    else
        cdci_descriptor->setPictureEssenceCoding(SUPPORTED_ESSENCE[mEssenceIndex].pc_label);
    cdci_descriptor->setSignalStandard(SUPPORTED_ESSENCE[mEssenceIndex].signal_standard);
    cdci_descriptor->setFrameLayout(SUPPORTED_ESSENCE[mEssenceIndex].frame_layout);
    SetColorSiting(MXF_COLOR_SITING_REC601);
    cdci_descriptor->setComponentDepth(SUPPORTED_ESSENCE[mEssenceIndex].component_depth);
    if (SUPPORTED_ESSENCE[mEssenceIndex].component_depth == 10) {
        cdci_descriptor->setBlackRefLevel(64);
        cdci_descriptor->setWhiteReflevel(940);
        cdci_descriptor->setColorRange(897);
    } else {
        cdci_descriptor->setBlackRefLevel(16);
        cdci_descriptor->setWhiteReflevel(235);
        cdci_descriptor->setColorRange(225);
    }
    SetCodingEquations(ITUR_BT709_CODING_EQ);
    cdci_descriptor->setStoredWidth(SUPPORTED_ESSENCE[mEssenceIndex].stored_width);
    cdci_descriptor->setStoredHeight(SUPPORTED_ESSENCE[mEssenceIndex].stored_height);
    cdci_descriptor->setDisplayWidth(cdci_descriptor->getStoredWidth());
    cdci_descriptor->setDisplayHeight(cdci_descriptor->getStoredHeight());
    cdci_descriptor->setSampledWidth(cdci_descriptor->getStoredWidth());
    cdci_descriptor->setSampledHeight(cdci_descriptor->getStoredHeight());
    cdci_descriptor->appendVideoLineMap(SUPPORTED_ESSENCE[mEssenceIndex].video_line_map[0]);
    cdci_descriptor->appendVideoLineMap(SUPPORTED_ESSENCE[mEssenceIndex].video_line_map[1]);
    cdci_descriptor->setHorizontalSubsampling(2);
    cdci_descriptor->setVerticalSubsampling(1);
    if (mFlavour == AVID_FLAVOUR)
        cdci_descriptor->setImageAlignmentOffset(8192);
}

uint32_t VC3MXFDescriptorHelper::GetSampleSize()
{
    return SUPPORTED_ESSENCE[mEssenceIndex].frame_size;
}

mxfUL VC3MXFDescriptorHelper::ChooseEssenceContainerUL() const
{
    if (mFlavour == AVID_FLAVOUR) {
        IM_ASSERT(!mFrameWrapped);
        return SUPPORTED_ESSENCE[mEssenceIndex].avid_ec_label;
    } else {
        if (mFrameWrapped)
            return MXF_EC_L(VC3FrameWrapped);
        else
            return MXF_EC_L(VC3ClipWrapped);
    }
}
