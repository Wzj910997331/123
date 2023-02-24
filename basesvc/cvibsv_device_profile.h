/**
 * File Name: cvi_device_profile.h
 *
 * Version: V1.0
 *
 * Brief: device profile params&options
 *
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * Description:
 *
 * History			:
 * =====================================================================================
   1.Date 			:	2020/07/16
   Author 			:	garry.xin
   Modification		:	Created file
 * ====================================================================================*/
#ifndef _CVI_DEVICE_PROFILE_H_
#define _CVI_DEVICE_PROFILE_H_
#include <iostream>
#include <vector>
using namespace std;

#include "cvi_common_type.h"
#include "cvi_base_type.h"
#include "cvi_platform.h"
#include "cvi_const.h"

#ifdef __cplusplus
	extern "C" {
#endif

#include "cvibsv_device.h"

//////////////////////////////////////////////////////////////////
/// PROFILE(ENCODER) PARAMS
//////////////////////////////////////////////////////////////////
class CVI_VideoResolution_C
{
public:
	cvi_bool m_bIsSupport;

	cvi_uint16 m_u16Width;
	cvi_uint16 m_u16Height;

    CVI_VideoResolution_C(){}

    CVI_VideoResolution_C(cvi_uint16 width, cvi_uint16 height):m_u16Width(width),m_u16Height(height)
    {
        m_bIsSupport = true;
    }
};
class CVI_VideoRateControl_C
{
public:
	cvi_bool m_bIsSupport;

    cvi_bool m_bIsConstantBitRateSupport;

	cvi_uint8 m_u8FrameRateLimit;
	cvi_uint8 m_u8EncodingInterval; //(A value of 1 means that every frame is encoded, a value of 2 means that every 2nd frame is encoded ...)  ?????
	cvi_uint32 m_u32BitrateLimit; // required, the maximum output bitrate in kbps
	cvi_bool m_bIsConstantBitRate; // optional, Enforce constant bitrate

    CVI_VideoRateControl_C(){}
};

class CVI_JpegOptions_C
{
public:
	cvi_bool m_bIsSupport;

	vector<CVI_VideoResolution_C> m_olResolutionsAvailableList;// required, List of supported image sizes

	CVI_IntRange_C m_oFrameRateRange;// required, Supported frame rate in fps (frames per second)
	CVI_IntRange_C m_oEncodingIntervalRange;// required, Supported encoding interval range. The encoding interval corresponds to the number of frames devided by the encoded frames. An encoding interval value of "1" means that all frames are encoded

    CVI_IntRange_C m_oBitrateRange;
};
class CVI_Mpeg4Options_C
{
public:
	cvi_bool m_bIsSupport;

	cvi_bool m_bIsMpeg4ProfileSPSupport;
	cvi_bool m_bIsMpeg4ProfileASPSupport;

	vector<CVI_VideoResolution_C> m_olResolutionsAvailableList;

	CVI_IntRange_C m_oGovLengthRange;					    // required, Supported group of Video frames length. This value typically corresponds to the I-Frame distance
	CVI_IntRange_C m_oFrameRateRange;					    // required, Supported frame rate in fps (frames per second)
	CVI_IntRange_C m_oEncodingIntervalRange;			    // required, Supported encoding interval range. The encoding interval corresponds to the number of frames devided by the encoded frames. An encoding interval value of "1" means that all frames are encoded

    CVI_IntRange_C m_oBitrateRange;

    CVI_Mpeg4Options_C()
    {
        m_bIsSupport = false;
        m_bIsMpeg4ProfileSPSupport = false;
        m_bIsMpeg4ProfileASPSupport = false;
    }
};

class CVI_H264Options_C
{
public:
	cvi_bool m_bIsSupport;

	cvi_bool m_bIsH264ProfileBaselineSupport;
	cvi_bool m_bIsH264ProfileMainSupport;
	cvi_bool m_bIsH264ProfileExtendedSupport;
	cvi_bool m_bIsH264ProfileHighSupport;

	vector<CVI_VideoResolution_C> m_olResolutionsAvailableList;	// required, List of supported image sizes

	CVI_IntRange_C m_oGovLengthRange;					    // required, Supported group of Video frames length. This value typically corresponds to the I-Frame distance
	CVI_IntRange_C m_oFrameRateRange;					    // required, Supported frame rate in fps (frames per second)
	CVI_IntRange_C m_oEncodingIntervalRange;			    // required, Supported encoding interval range. The encoding interval corresponds to the number of frames devided by the encoded frames. An encoding interval value of "1" means that all frames are encoded

    CVI_IntRange_C m_oBitrateRange;

    CVI_H264Options_C()
    {
        m_bIsSupport = false;

        m_bIsH264ProfileBaselineSupport = false;
        m_bIsH264ProfileMainSupport = false;
        m_bIsH264ProfileExtendedSupport = false;
        m_bIsH264ProfileHighSupport = false;
    }
};

class CVI_H265Options_C
{
public:
	cvi_bool m_bIsSupport;

	cvi_bool m_bIsH265ProfileMainSupport;
	cvi_bool m_bIsH265ProfileExtendedSupport;
	cvi_bool m_bIsH265ProfileHighSupport;

	vector<CVI_VideoResolution_C> m_olResolutionsAvailableList;	// required, List of supported image sizes

	CVI_IntRange_C m_oGovLengthRange;					    // required, Supported group of Video frames length. This value typically corresponds to the I-Frame distance
	CVI_IntRange_C m_oFrameRateRange;					    // required, Supported frame rate in fps (frames per second)
	CVI_IntRange_C m_oEncodingIntervalRange;			    // required, Supported encoding interval range. The encoding interval corresponds to the number of frames devided by the encoded frames. An encoding interval value of "1" means that all frames are encoded

    CVI_IntRange_C m_oBitrateRange;

    CVI_H265Options_C()
    {
        m_bIsSupport = false;

        m_bIsH265ProfileMainSupport = false;
        m_bIsH265ProfileExtendedSupport = false;
        m_bIsH265ProfileHighSupport = false;
    }
};

class CVI_DevVideoEncoderOptions_C
{
public:
	cvi_bool m_bIsSupport;

	cvi_bool m_bIsJPEGSupport;
	cvi_bool m_bIsMPEG4Support;
	cvi_bool m_bIsH264Support;
	cvi_bool m_bIsH265Support;
	//cvi_bool m_bIsExtensionSupport;

	CVI_IntRange_C m_oQualityRange;

	CVI_JpegOptions_C m_oJPEG;
	CVI_Mpeg4Options_C m_oMPEG4;// optional, Optional MPEG-4 encoder settings ranges
	CVI_H264Options_C m_oH264;// optional, Optional H.264 encoder settings ranges
	CVI_H265Options_C m_oH265; // optional, Optional H.265 encoder settings ranges

	//onvif_VideoEncoderOptionsExtension  Extension; // optional

    CVI_DevVideoEncoderOptions_C()
    {
        m_bIsSupport = false;
        m_bIsJPEGSupport = false;
        m_bIsMPEG4Support = false;
        m_bIsH264Support = false;
        m_bIsH265Support = false;
    }
};
class CVI_AudioEncoderOptionsParams_C
{
public:
	cvi_bool m_bIsSupport;

	CVI_AUDIO_ENCODE_TYPE_E m_eType;
	vector<cvi_uint32> m_u32lBitRateList;
	vector<cvi_uint16> m_u16lSampleRateList;
};
class CVI_DevAudioEncoderOptions_C
{
public:
	cvi_bool m_bIsSupport;

	vector<CVI_AudioEncoderOptionsParams_C> m_olList;
};
class CVI_DevVideoEncoderParams_C
{
public:
	cvi_bool m_bIsSupport;

	cvi_bool m_bRateControlSupport;

    string m_strToken;
    string m_strName;

	CVI_VIDEO_ENCODE_TYPE_E m_eEncoding;
	CVI_VideoResolution_C m_oResolution;

	cvi_uint16 m_u16Quality;
	cvi_uint16 m_u16GopLength;
	//cvi_uint32 m_u32Profile; // h.264 profile(BASELINE/...)/h.265 profile(MAIN10,...)/mpeg4 profile(SP/ASP)
	string m_strProfile; // h.264 profile(BASELINE/...)/h.265 profile(MAIN10,...)/mpeg4 profile(SP/ASP)

	CVI_VideoRateControl_C m_oRateControl;

	//onvif_MulticastConfiguration    Multicast;

	//cvi_uint16 m_u16SessionTimeout;	// unit in secs

    CVI_DevVideoEncoderParams_C()
    {
        m_bIsSupport = false;
    }
};

//tt__Vector
class CVI_Vector
{
public:
    float m_fX;
    float m_fY;
};


class CVI_OSDPosConfiguration
{
public:
    string m_strType;
    CVI_Vector m_oPos;

};

class CVI_Color
{
public:
    float X;
    float Y;
    float Z;
    string m_strColorspace;
};

class CVI_OSDColor
{
public:
    CVI_Color m_oColor;
    cvi_uint16 m_u16Transparent;
};

class CVI_OSDTextConfiguration
{
public:
    cvi_bool m_bIsSupport;
    string m_strType;
    string m_strDateFormat;
    string m_strTimeFormat;
    cvi_uint16 m_u16FontSize;
    CVI_OSDColor m_oOSDColor;
    CVI_OSDColor m_oBackgroundColor;
    string m_strPlainText;
    string m_strTextExtension;
    bool m_IsPersistentText;
    CVI_OSDTextConfiguration()
    {
        m_bIsSupport = false;
    }
};

class CVI_OSDImgConfiguration
{
public:
    cvi_bool m_bIsSupport;
    string m_strImgPath;
    string m_strImageExtension;
    CVI_OSDImgConfiguration()
    {
        m_bIsSupport = false;
    }
};

//tt__OSDConfiguration
class CVI_DevImageOSDParams_C
{
public:
    cvi_bool m_bIsSupport;
    CVI_OSDPosConfiguration m_oPosition;
    CVI_OSDTextConfiguration m_oTextString;
    CVI_OSDImgConfiguration m_oImage;
    string m_strOSDConfigurationExtension;
    CVI_DevImageOSDParams_C()
    {
        m_bIsSupport = false;
    }
};

/* tt:Polygon */
class CVI_Polygon
{
public:
	vector<CVI_Vector * > m_listPoint;	/* required element of type tt:Vector */
};

//tr2__Mask
class CVI_DevImageMaskParams_C
{
public:
    cvi_bool m_bIsSupport;
	CVI_Polygon *m_oPolygon;	/* required element of type tt:Polygon */
	string m_strType;	/* required element of type xsd:string */
    CVI_Color m_oColor;
	bool m_bEnabled;	/* required element of type xsd:boolean */
	vector<char * >__any;
    CVI_DevImageMaskParams_C()
    {
        m_bIsSupport = false;
    }
};

class CVI_DevAudioEncoderParams_C
{
public:
	cvi_bool m_bIsSupport;

    string m_strToken;
	string m_strName;	// required, User readable name. Length up to 64 characters

	CVI_AUDIO_ENCODE_TYPE_E m_eEncoding;

	cvi_uint32 m_u32Bitrate;// required, The output bitrate in kbps
	cvi_uint16 m_u16SampleRate;	// required, The output sample rate in kHz

	//onvif_MulticastConfiguration    Multicast;// required, Defines the multicast settings that could be used for video streaming

	cvi_uint32 m_u32SessionTimeout;// required, The rtsp session timeout for the related audio stream, unit is second
};

class CVI_DevAudioSourceParams_C
{
public:
	cvi_bool m_bIsSupport;

    string m_strToken;
    string m_strName;

	cvi_uint8 m_u8ChannelNum; //  ?????

    CVI_DevAudioSourceParams_C()
    {
        m_bIsSupport = false;

        m_u8ChannelNum = 0;
    }
};

class CVI_DevVideoSourceParams_C
{
public:
	cvi_bool m_bIsSupport;

    string m_strToken;
    string m_strName;

	cvi_bool m_bIsRotateSupport;
	cvi_bool m_bIsDegreeSupport;
	CVI_IntRectangleRange_C m_oBounds;
	CVI_ROTATE_MODE_E m_eRotateMode;
	cvi_int16 m_i16Degree;

    CVI_DevVideoSourceParams_C()
    {
        m_bIsSupport = false;
        m_bIsRotateSupport = false;
        m_bIsDegreeSupport = false;
    }
};


class CVI_DevVideoSourceOptions_C
{
public:
	cvi_bool m_bIsSupport;

	CVI_IntRectangleRange_C m_oBounds;

	cvi_bool m_bIsRotateSupport;
    cvi_bool m_bIsRotateModeOffSupport;
    cvi_bool m_bIsRotateModeOnSupport;
    cvi_bool m_bIsRotateModeAutoSupport;

    cvi_bool m_bIsDegreeListSupport;
	vector<cvi_int16> m_i16lDegreeList;

    CVI_DevVideoSourceOptions_C()
    {
	    m_bIsSupport = false;

	    m_bIsRotateSupport = false;
        m_bIsRotateModeOffSupport = false;
        m_bIsRotateModeOnSupport = false;
        m_bIsRotateModeAutoSupport = false;
        m_bIsDegreeListSupport = false;
    }
};

class CVI_DevStreamInfo_C
{
public:
    string m_strStreamUri;
};

class CVIBSV_DevProfileParams_C
{
public:

    cvi_bool m_bIsValid;

    string m_strToken;
    string m_strName;

	// video source:skip
	CVI_DevVideoSourceParams_C m_oVideoSource;

	// audio source:skip
	CVI_DevAudioSourceParams_C m_oAudioSource;

	// video encoder
	CVI_DevVideoEncoderParams_C m_oVideoEncoder;
	// audio encoder
	CVI_DevAudioEncoderParams_C m_oAudioEncoder;

	// audio decoder:skip

    // TODO

    CVI_DevStreamInfo_C m_oStreamInfo;

    CVIBSV_DevProfileParams_C()
    {
        m_bIsValid = false;
    }

    string toString()
    {
        char buff[1024] = {0};
        snprintf(buff, sizeof(buff), "\r\n\r\nm_oVideoEncoder:\r\n"\
                                             "    m_strName:        %s\r\n"\
                                             "    m_eEncoding:      %s\r\n"\
                                             "    m_oResolution:    %d x %d\r\n"\
                                             "    m_u16Quality:     %d\r\n"\
                                             "    m_u16GopLength:   %d\r\n"\
                                             "    m_strProfile:     %s\r\n"\
                                             "m_oAudioEncoder:\r\n"\
                                             "    m_strName:        %s\r\n"\
                                             "    m_eEncoding:      %s\r\n"\
                                             "    m_u32Bitrate:     %d\r\n"\
                                             "    m_u16SampleRate:  %d\r\n"\
                                             "m_oStreamInfo:    %s\r\n\r\n",\
                                             m_oVideoEncoder.m_strName.c_str(),\
                                             videoEncodeTypeTransform2String(m_oVideoEncoder.m_eEncoding).c_str(),\
                                             m_oVideoEncoder.m_oResolution.m_u16Width, m_oVideoEncoder.m_oResolution.m_u16Height,\
                                             m_oVideoEncoder.m_u16Quality,\
                                             m_oVideoEncoder.m_u16GopLength,\
                                             m_oVideoEncoder.m_strProfile.c_str(),\
                                             m_oAudioEncoder.m_strName.c_str(),\
                                             audioEncodeTypeTransform2String(m_oAudioEncoder.m_eEncoding).c_str(),\
                                             m_oAudioEncoder.m_u32Bitrate,\
                                             m_oAudioEncoder.m_u16SampleRate,\
                                             m_oStreamInfo.m_strStreamUri.c_str());
        return string(buff);
    }
};

class CVIBSV_DevProfileOptions_C
{
public:
	CVI_DevVideoEncoderOptions_C m_oVideoEncoder;
	CVI_DevAudioEncoderOptions_C m_oAudioEncoder;

	CVI_DevVideoSourceOptions_C m_oVideoSource;

    CVIBSV_DevProfileOptions_C(){}

private:

    string resolutionsList2String(vector<CVI_VideoResolution_C> &list)
    {
        string str = "";

        char buff[16] = {0};

        for(auto r : list)
        {
            snprintf(buff, sizeof(buff), "%d x %d; ", r.m_u16Width, r.m_u16Height);

            str += string(buff);
        }

        return str;
    }

    string audioEncoderOptionsList2String()
    {
        string str = "";

        for(auto a : m_oAudioEncoder.m_olList)
        {
            str += "encodeType: " + audioEncodeTypeTransform2String(a.m_eType) + ", ";

            str += "bitRateList: ";

            for(auto bit : a.m_u32lBitRateList)
            {
                str += std::to_string(bit) + ", ";
            }

            str += "sampleRateList: ";

            for(auto sample : a.m_u16lSampleRateList)
            {
                str += std::to_string(sample) + ", ";
            }
        }

        return str;
    }

public:

    string toString()
    {
        char buff[1024] = {0};
        snprintf(buff, sizeof(buff), "\r\n\r\nm_oVideoEncoderOptions:\r\n"\
                                             "    m_bIsH264Support:        %d\r\n"\
                                             "        m_olResolutionsAvailableList:      %s\r\n"\
                                             "        m_oGovLengthRange:                 %d - %d\r\n"\
                                             "        m_oFrameRateRange:                 %d - %d\r\n"\
                                             "    m_bIsH265Support:        %d\r\n"\
                                             "        m_olResolutionsAvailableList:      %s\r\n"\
                                             "        m_oGovLengthRange:                 %d - %d\r\n"\
                                             "        m_oFrameRateRange:                 %d - %d\r\n"\
                                             "m_oAudioEncoderOptions:\r\n"\
                                             "    m_olList:        %s\r\n\r\n",\
                                             m_oVideoEncoder.m_bIsH264Support,\
                                             resolutionsList2String(m_oVideoEncoder.m_oH264.m_olResolutionsAvailableList).c_str(),\
                                             m_oVideoEncoder.m_oH264.m_oGovLengthRange.m_i32Min, m_oVideoEncoder.m_oH264.m_oGovLengthRange.m_i32Max,\
                                             m_oVideoEncoder.m_oH264.m_oFrameRateRange.m_i32Min, m_oVideoEncoder.m_oH264.m_oFrameRateRange.m_i32Max,\
                                             m_oVideoEncoder.m_bIsH265Support,\
                                             resolutionsList2String(m_oVideoEncoder.m_oH265.m_olResolutionsAvailableList).c_str(),\
                                             m_oVideoEncoder.m_oH265.m_oGovLengthRange.m_i32Min, m_oVideoEncoder.m_oH265.m_oGovLengthRange.m_i32Max,\
                                             m_oVideoEncoder.m_oH265.m_oFrameRateRange.m_i32Min, m_oVideoEncoder.m_oH265.m_oFrameRateRange.m_i32Max,\
                                             audioEncoderOptionsList2String().c_str());
        return string(buff);
    }
};

#ifdef __cplusplus
	}
#endif
#endif//_CVI_DEVICE_PROFILE_H_
