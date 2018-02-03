#pragma once
#include <opencv2/core/utility.hpp>

namespace lyonste
{
	namespace hal
	{
		typedef size_t HAL_FLAG;

		const HAL_FLAG HAL_NONE=0;
		const HAL_FLAG HAL_MMX=1<<0;
		const HAL_FLAG HAL_SSE=1<<1;
		const HAL_FLAG HAL_SSE2=1<<2;
		const HAL_FLAG HAL_SSE3=1<<3;
		const HAL_FLAG HAL_SSSE3=1<<4;
		const HAL_FLAG HAL_SSE4_1=1<<5;
		const HAL_FLAG HAL_SSE4_2=1<<6;
		const HAL_FLAG HAL_POPCNT=1<<7;
		const HAL_FLAG HAL_FP16=1<<8;
		const HAL_FLAG HAL_AVX=1<<9;
		const HAL_FLAG HAL_AVX2=1<<10;
		const HAL_FLAG HAL_FMA3=1<<11;
		const HAL_FLAG HAL_AVX_512F=1<<12;
		const HAL_FLAG HAL_AVX_512BW=1<<13;
		const HAL_FLAG HAL_AVX_512CD=1<<14;
		const HAL_FLAG HAL_AVX_512DQ=1<<15;
		const HAL_FLAG HAL_AVX_512ER=1<<16;
		const HAL_FLAG HAL_AVX_512IFMA512=1<<17;
		const HAL_FLAG HAL_AVX_512PF=1<<18;
		const HAL_FLAG HAL_AVX_512VBMI=1<<19;
		const HAL_FLAG HAL_AVX_512VL=1<<20;

		const HAL_FLAG HAL_NEON=1<<21;

		const HAL_FLAG HAL_VSX1=1<<22;

		const HAL_FLAG HAL_OCL=1<<23;

		const inline HAL_FLAG initializeHalFlag()
		{
			//TODO: eventually, add support for other types
			HAL_FLAG flag=HAL_NONE;
			if(cv::checkHardwareSupport(CV_CPU_MMX))
			{
				flag|=HAL_MMX;
			}
			if(cv::checkHardwareSupport(CV_CPU_SSE))
			{
				flag|=HAL_SSE;
			}
			if(cv::checkHardwareSupport(CV_CPU_SSE2))
			{
				flag|=HAL_SSE2;
			}
			if(cv::checkHardwareSupport(CV_CPU_SSE3))
			{
				flag|=HAL_SSE3;
			}
			if(cv::checkHardwareSupport(CV_CPU_SSSE3))
			{
				flag|=HAL_SSSE3;
			}
			if(cv::checkHardwareSupport(CV_CPU_SSE4_1))
			{
				flag|=HAL_SSE4_1;
			}
			if(cv::checkHardwareSupport(CV_CPU_SSE4_1))
			{
				flag|=HAL_SSE4_1;
			}
			if(cv::checkHardwareSupport(CV_CPU_AVX))
			{
				flag|=HAL_AVX;
			}
			return flag;
		}

		const HAL_FLAG halFlag=initializeHalFlag();

		template<HAL_FLAG hal>
		struct HalFlagInfo
		{
			constexpr static bool hasMMX=(hal&HAL_MMX)!=0;
			constexpr static bool hasSSE=(hal&HAL_SSE)!=0;
			constexpr static bool hasSSE2=(hal&HAL_SSE2)!=0;
			constexpr static bool hasSSE3=(hal&HAL_SSE3)!=0;
			constexpr static bool hasSSSE3=(hal&HAL_SSSE3)!=0;
			constexpr static bool hasSSE4_1=(hal&HAL_SSE4_1)!=0;
			constexpr static bool hasSSE4_2=(hal&HAL_SSE4_2)!=0;
			constexpr static bool hasAVX=(hal&HAL_AVX)!=0;
		
		};

		template<HAL_FLAG hal>
		constexpr bool supports(const HAL_FLAG flag=halFlag)
		{
			return (flag&hal)==hal;
		}
	}
}
//namespace std
//{
//	//inline std::string toString(lyonste::hal::HAL_FLAG hal) noexcept
//	//{
//	//	if(hal==lyonste::hal::HAL_NONE)
//	//	{
//	//		return "NONE";
//	//	}
//	//	std::stringstream ss;
//	//	size_t i=0;
//	//	bool foundOne=false;
//	//	for(;i<=23;++i)
//	//	{
//	//		if(hal&(1ull<<i))
//	//		{
//	//			if(foundOne)
//	//			{
//	//				ss<<"|";
//	//			}
//	//			else
//	//			{
//	//				foundOne=true;
//	//			}
//	//			switch(i)
//	//			{
//	//				case 0:
//	//					ss<<"MMX";
//	//					break;
//	//				case 1:
//	//					ss<<"SSE";
//	//					break;
//	//				case 2:
//	//					ss<<"SSE2";
//	//					break;
//	//				case 3:
//	//					ss<<"SSE3";
//	//					break;
//	//				case 4:
//	//					ss<<"SSSE3";
//	//					break;
//	//				case 5:
//	//					ss<<"SSE4.1";
//	//					break;
//	//				case 6:
//	//					ss<<"SSE4.2";
//	//					break;
//	//				case 7:
//	//					ss<<"POPCNT";
//	//					break;
//	//				case 8:
//	//					ss<<"FP16";
//	//					break;
//	//				case 9:
//	//					ss<<"AVX";
//	//					break;
//	//				case 10:
//	//					ss<<"AVX2";
//	//					break;
//	//				case 11:
//	//					ss<<"FMA3";
//	//					break;
//	//				case 12:
//	//					ss<<"AVX512F";
//	//					break;
//	//				case 13:
//	//					ss<<"AVX512BW";
//	//					break;
//	//				case 14:
//	//					ss<<"AVX512CD";
//	//					break;
//	//				case 15:
//	//					ss<<"AVX512DQ";
//	//					break;
//	//				case 16:
//	//					ss<<"AVX512ER";
//	//					break;
//	//				case 17:
//	//					ss<<"AVX512IFMA512";
//	//					break;
//	//				case 18:
//	//					ss<<"AVX512PF";
//	//					break;
//	//				case 19:
//	//					ss<<"AVX512VBMI";
//	//					break;
//	//				case 20:
//	//					ss<<"AVX512VL";
//	//					break;
//	//				case 21:
//	//					ss<<"NEON";
//	//					break;
//	//				case 22:
//	//					ss<<"VSX1";
//	//					break;
//	//				case 23:
//	//					ss<<"OCL";
//	//					break;
//	//				default:
//	//					ss<<"<Unknown>";
//	//			}
//	//		}
//	//	}
//	//	return ss.str();
//	//}
//}


