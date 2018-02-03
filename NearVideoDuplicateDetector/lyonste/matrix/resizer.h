#pragma once
#include "matrix.h"
#include "../lyonste.h"

namespace lyonste
{
	namespace matrix
	{
		namespace resizer
		{

			template<lyonste::hal::HAL_FLAG hal=lyonste::hal::HAL_NONE>
			class GrayMat2DResizer
			{
			public:
				virtual void resize(const Ptr2D<uchar>& src,Ptr2D<uchar>& dst)=0;
				virtual ~GrayMat2DResizer();
				static constexpr GrayMat2DResizer<hal>* getResizer(size_t srcX,size_t srcY,size_t dstX,size_t dstY);
			};

			struct GrayMat2DResizer2
			{
				size_t srcX;
				size_t srcY;
				size_t dstX;
				size_t dstY;

				constexpr GrayMat2DResizer2(const size_t srcX,const size_t srcY,const size_t dstX,const size_t dstY):
					srcX(srcX),srcY(srcY),dstX(dstX),dstY(dstY)
				{}
				template<lyonste::hal::HAL_FLAG hal=lyonste::hal::HAL_NONE> constexpr static GrayMat2DResizer2* getResizer(const size_t srcX,const size_t srcY,const size_t dstX,const size_t dstY);
				virtual void resize(const Ptr2D<uchar>& src,Ptr2D<uchar>& dst)=0;
				virtual ~GrayMat2DResizer2()
				{}
			};
			struct NoOpResizer2: public GrayMat2DResizer2
			{
			public:
				constexpr NoOpResizer2(const size_t srcX,const size_t srcY,const size_t dstX,const size_t dstY): GrayMat2DResizer2(srcX,srcY,dstX,dstY)
				{}
				void resize(const Ptr2D<uchar>& src,Ptr2D<uchar>& dst) override
				{}
			};
			struct SameSizeResizer2: public GrayMat2DResizer2
			{
			public:
				constexpr SameSizeResizer2(const size_t srcX,const size_t srcY,const size_t dstX,const size_t dstY):
					GrayMat2DResizer2(srcX,srcY,dstX,dstY)
				{}
				void resize(const Ptr2D<uchar>& src,Ptr2D<uchar>& dst) override
				{
					CV_DbgAssert(src.x==dst.x);
					CV_DbgAssert(src.y==dst.y);
					if(&src!=&dst)
					{
						std::memmove(dst.data,src.data,src.x*src.y);
					}
				}
			};
			class LinearShrinker2: public GrayMat2DResizer2,public Mat1D<uchar>
			{
			protected:
				size_t buffStep;
				int* tmpRow0;
				int* tmpRow1;
				size_t* xOffsets;
				size_t* yOffsets;
				short* xBlend;
				short* yBlend;
				constexpr LinearShrinker2(const size_t srcX,const size_t srcY,const size_t dstX,const size_t dstY,const double scaleX,const double scaleY,const size_t buffStep):
					GrayMat2DResizer2(srcX,srcY,dstX,dstY),
					Mat1D<uchar>(((dstX+dstY)*(sizeof(size_t)+(sizeof(short)<<1)))+((buffStep<<1)*sizeof(int))),
					buffStep(buffStep),
					tmpRow0((int*)data),
					tmpRow1(tmpRow0+buffStep),
					xOffsets((size_t*)(tmpRow1+buffStep)),
					yOffsets(xOffsets+dstX),
					xBlend((short*)(yOffsets+dstY)),
					yBlend(xBlend+(dstX<<1))
				{
					const size_t srcXMin1=srcX-1;
					for(size_t x=0,interpolationOffset=-1; x < dstX; ++x)
					{
						float fx=(float)((x+0.5)*scaleX-0.5);
						size_t sx=size_t(fx);
						fx-=sx;
						if(sx+1>=srcX)
						{
							CV_DbgAssert(fx==0);
							CV_DbgAssert(sx!=srcXMin1);
							if(sx > srcXMin1)
							{
								sx=srcXMin1;
							}
						}
						xOffsets[x]=sx;
						xBlend[++interpolationOffset]=cv::saturate_cast<short>((1.f-fx)*2048);
						xBlend[++interpolationOffset]=cv::saturate_cast<short>(fx*2048);
					}

					for(size_t y=0,interpolationOffset=-1; y < dstY; ++y)
					{
						float fy=(float)((y+0.5)*scaleY-0.5);
						size_t sy=size_t(fy);
						fy-=sy;
						yOffsets[y]=sy;
						yBlend[++interpolationOffset]=cv::saturate_cast<short>((1.f-fy)*2048);
						yBlend[++interpolationOffset]=cv::saturate_cast<short>(fy*2048);
					}
				}
				virtual void vResize(uchar* dst,const size_t yBlendIndex)=0;
			public:
				void resize(const Ptr2D<uchar>& src,Ptr2D<uchar>& dst) override
				{
					CV_DbgAssert(src.x==srcX);
					CV_DbgAssert(src.y==srcY);
					CV_DbgAssert(dst.x==dstX);
					CV_DbgAssert(dst.y==dstY);
					const uchar* srcPtr=src.data;
					uchar* dstPtr=dst.data;
					size_t prevSrcY0=-1;
					size_t prevSrcY1=-1;
					size_t betaOffset=0;
					for(size_t y=0; y < dstY; ++y,dstPtr+=dstX,betaOffset+=2)
					{
						size_t yOff=yOffsets[y];
						size_t sRow=clip(yOff,srcY);
						if(sRow==prevSrcY0)
						{
							const uchar* srcRow1Ptr=srcPtr+((prevSrcY1=clip(yOff+1,srcY))*srcX);
							size_t alphaOffset=-1;
							for(size_t x=0; x < dstX; ++x)
							{
								size_t xOff;
								tmpRow1[x]=(srcRow1Ptr[xOff=xOffsets[x]]*(int)xBlend[++alphaOffset])+(srcRow1Ptr[xOff+1]*(int)xBlend[++alphaOffset]);
							}
						}
						else
						{
							if(sRow==prevSrcY1)
							{
								std::memcpy(tmpRow0,tmpRow1,buffStep*sizeof(int));
							}
							const uchar* srcRow0Ptr=srcPtr+((prevSrcY0=sRow)*srcX);
							const uchar* srcRow1Ptr=srcPtr+((prevSrcY1=clip(yOff+1,srcY))*srcX);
							size_t alphaOffset=-1;
							for(size_t x=0; x < dstX; ++x)
							{
								size_t xOff=xOffsets[x];
								int a0=int(xBlend[++alphaOffset]);
								int a1=int(xBlend[++alphaOffset]);
								tmpRow0[x]=(srcRow0Ptr[xOff]*a0)+(srcRow0Ptr[xOff+1]*a1);
								tmpRow1[x]=(srcRow1Ptr[xOff]*a0)+(srcRow1Ptr[xOff+1]*a1);
							}
						}
						vResize(dstPtr,betaOffset);
					}
				}

			};
			class NonHALLinearShrinker2: public LinearShrinker2
			{
			protected:
				void vResize(uchar* dst,const size_t yBlendIndex) override
				{
					const short beta0=yBlend[yBlendIndex];
					const short beta1=yBlend[yBlendIndex+1];
					for(size_t x=0; x < dstX; ++x)
					{
						dst[x]=uchar((((beta0 * (tmpRow0[x]>>4))>>16)+((beta1 * (tmpRow1[x]>>4))>>16)+2)>>2);
					}
				}
			public:
				constexpr NonHALLinearShrinker2(const size_t srcX,const size_t srcY,const size_t dstX,const size_t dstY,const double scaleX,const double scaleY):
					LinearShrinker2(srcX,srcY,dstX,dstY,scaleX,scaleY,dstX)
				{

				}
			};
			class NonHALHalfSizeShrinker2: public GrayMat2DResizer2
			{
			public:
				constexpr NonHALHalfSizeShrinker2(const size_t srcX,const size_t srcY,const size_t dstX,const size_t dstY):
					GrayMat2DResizer2(srcX,srcY,dstX,dstY)
				{}
				void resize(const Ptr2D<uchar>& src,Ptr2D<uchar>& dst) override
				{
					CV_DbgAssert(src.x==srcX);
					CV_DbgAssert(src.y==srcY);
					CV_DbgAssert(dst.x==dstX);
					CV_DbgAssert(dst.y==dstY);
					for(size_t y=0; y < dstY; ++y)
					{
						uchar* dstRow=dst.ptr(y);
						const uchar* srcRow0=src.ptr(y<<1);
						const uchar* srcRow1=srcRow0+srcX;
						for(size_t dx=0; dx < dstX; ++dx)
						{
							size_t index=dx<<1;
							dstRow[dx]=(uchar)((srcRow0[index]+srcRow0[index+1]+srcRow1[index]+srcRow1[index+1]+2)>>2);
						}
					}
				}
			};
			class SSE2LinearShrinker2: public LinearShrinker2
			{
			protected:
				void vResize(uchar* dst,const size_t yBlendIndex) override
				{
					const short beta0=yBlend[yBlendIndex];
					const short beta1=yBlend[yBlendIndex+1];
					size_t x=0;
					if(dstX>=16)
					{

						__m128i beta0_128=_mm_set1_epi16(beta0);
						__m128i beta1_128=_mm_set1_epi16(beta1);
						__m128i delta_128=_mm_set1_epi16(2);
						size_t xBound=dstX-16;
						if((((size_t)tmpRow0|(size_t)tmpRow1)&15)==0)
						{
							for(; x<=xBound; x+=16)
							{
								size_t offset=x;
								__m128i x0=_mm_loadu_si128((const __m128i*)(tmpRow0+offset));
								__m128i y0=_mm_loadu_si128((const __m128i*)(tmpRow1+offset));
								offset+=4;
								__m128i x1=_mm_loadu_si128((const __m128i*)(tmpRow0+offset));
								__m128i y1=_mm_loadu_si128((const __m128i*)(tmpRow1+offset));
								offset+=4;
								x0=_mm_packs_epi32(_mm_srai_epi32(x0,4),_mm_srai_epi32(x1,4));
								y0=_mm_packs_epi32(_mm_srai_epi32(y0,4),_mm_srai_epi32(y1,4));
								x1=_mm_loadu_si128((const __m128i*)(tmpRow0+offset));
								y1=_mm_loadu_si128((const __m128i*)(tmpRow1+offset));
								offset+=4;
								__m128i x2=_mm_loadu_si128((const __m128i*)(tmpRow0+offset));
								__m128i y2=_mm_loadu_si128((const __m128i*)(tmpRow1+offset));
								x1=_mm_packs_epi32(_mm_srai_epi32(x1,4),_mm_srai_epi32(x2,4));
								y1=_mm_packs_epi32(_mm_srai_epi32(y1,4),_mm_srai_epi32(y2,4));
								x0=_mm_adds_epi16(_mm_mulhi_epi16(x0,beta0_128),_mm_mulhi_epi16(y0,beta1_128));
								x1=_mm_adds_epi16(_mm_mulhi_epi16(x1,beta0_128),_mm_mulhi_epi16(y1,beta1_128));
								x0=_mm_srai_epi16(_mm_adds_epi16(x0,delta_128),2);
								x1=_mm_srai_epi16(_mm_adds_epi16(x1,delta_128),2);
								_mm_storeu_si128((__m128i*)(dst+x),_mm_packus_epi16(x0,x1));
							}
						}
						else
						{
							for(; x<=xBound; x+=16)
							{
								size_t offset=x;
								__m128i x0=_mm_load_si128((const __m128i*)(tmpRow0+offset));
								__m128i y0=_mm_load_si128((const __m128i*)(tmpRow1+offset));
								offset+=4;
								__m128i x1=_mm_load_si128((const __m128i*)(tmpRow0+offset));
								__m128i y1=_mm_load_si128((const __m128i*)(tmpRow1+offset));
								offset+=4;
								x0=_mm_packs_epi32(_mm_srai_epi32(x0,4),_mm_srai_epi32(x1,4));
								y0=_mm_packs_epi32(_mm_srai_epi32(y0,4),_mm_srai_epi32(y1,4));
								x1=_mm_load_si128((const __m128i*)(tmpRow0+offset));
								y1=_mm_load_si128((const __m128i*)(tmpRow1+offset));
								offset+=4;
								__m128i x2=_mm_load_si128((const __m128i*)(tmpRow0+offset));
								__m128i y2=_mm_load_si128((const __m128i*)(tmpRow1+offset));
								x1=_mm_packs_epi32(_mm_srai_epi32(x1,4),_mm_srai_epi32(x2,4));
								y1=_mm_packs_epi32(_mm_srai_epi32(y1,4),_mm_srai_epi32(y2,4));
								x0=_mm_adds_epi16(_mm_mulhi_epi16(x0,beta0_128),_mm_mulhi_epi16(y0,beta1_128));
								x1=_mm_adds_epi16(_mm_mulhi_epi16(x1,beta0_128),_mm_mulhi_epi16(y1,beta1_128));
								x0=_mm_srai_epi16(_mm_adds_epi16(x0,delta_128),2);
								x1=_mm_srai_epi16(_mm_adds_epi16(x1,delta_128),2);
								_mm_storeu_si128((__m128i*)(dst+x),_mm_packus_epi16(x0,x1));
							}
						}
						xBound=dstX-4;
						for(; x < xBound; x+=4)
						{
							__m128i x0=_mm_srai_epi32(_mm_loadu_si128((const __m128i*)(tmpRow0+x)),4);
							__m128i y0=_mm_srai_epi32(_mm_loadu_si128((const __m128i*)(tmpRow1+x)),4);
							x0=_mm_packs_epi32(x0,x0);
							y0=_mm_packs_epi32(y0,y0);
							x0=_mm_adds_epi16(_mm_mulhi_epi16(x0,beta0_128),_mm_mulhi_epi16(y0,beta1_128));
							x0=_mm_srai_epi16(_mm_adds_epi16(x0,delta_128),2);
							x0=_mm_packus_epi16(x0,x0);
							*(int*)(dst+x)=_mm_cvtsi128_si32(x0);
						}
					}
					for(; x < dstX; ++x)
					{
						dst[x]=uchar((((beta0 * (tmpRow0[x]>>4))>>16)+((beta1 * (tmpRow1[x]>>4))>>16)+2)>>2);
					}
				}
			public:
				constexpr SSE2LinearShrinker2(const size_t srcX,const size_t srcY,const size_t dstX,const size_t dstY,const double scaleX,const double scaleY):
					LinearShrinker2(srcX,srcY,dstX,dstY,scaleX,scaleY,(dstX+15)&-16)
				{

				}
			};
			class SSE2HalfSizeShrinker2: public GrayMat2DResizer2
			{
			public:
				constexpr SSE2HalfSizeShrinker2(const size_t srcX,const size_t srcY,const size_t dstX,const size_t dstY):
					GrayMat2DResizer2(srcX,srcY,dstX,dstY)
				{

				}

				void resize(const Ptr2D<uchar>& src,Ptr2D<uchar>& dst) override
				{
					CV_DbgAssert(src.x==srcX);
					CV_DbgAssert(src.y==srcY);
					CV_DbgAssert(dst.x==dstX);
					CV_DbgAssert(dst.y==dstY);
					for(size_t y=0; y < dstY; ++y)
					{
						uchar* dstRow=dst.ptr(y);
						const uchar* srcRow0=src.ptr(y<<1);
						const uchar* srcRow1=srcRow0+srcX;
						size_t dx=0;

						if(dstX>=8)
						{
							__m128i zero_128=_mm_setzero_si128();
							__m128i delta_128=_mm_set1_epi16(2);
							__m128i maskLow_128=_mm_set1_epi16(0x00ff);
							size_t xBound=dstX-8;
							uchar* dstRow_simd=dstRow;
							const uchar* srcRow0_simd=srcRow0;
							const uchar* srcRow1_simd=srcRow1;
							for(; dx<=xBound; dx+=8,srcRow0_simd+=16,srcRow1_simd+=16,dstRow_simd+=8)
							{
								__m128i r0=_mm_loadu_si128((const __m128i*)srcRow0_simd);
								__m128i r1=_mm_loadu_si128((const __m128i*)srcRow1_simd);
								__m128i s0=_mm_add_epi16(_mm_srli_epi16(r0,8),_mm_and_si128(r0,maskLow_128));
								__m128i s1=_mm_add_epi16(_mm_srli_epi16(r1,8),_mm_and_si128(r1,maskLow_128));
								s0=_mm_add_epi16(_mm_add_epi16(s0,s1),delta_128);
								s0=_mm_packus_epi16(_mm_srli_epi16(s0,2),zero_128);
								_mm_storel_epi64((__m128i*)dstRow_simd,s0);
							}
						}
						for(; dx < dstX; ++dx)
						{
							size_t index=dx<<1;
							dstRow[dx]=(uchar)((srcRow0[index]+srcRow0[index+1]+srcRow1[index]+srcRow1[index+1]+2)>>2);
						}
					}
				}
			};
			template<lyonste::hal::HAL_FLAG hal> constexpr  GrayMat2DResizer2* GrayMat2DResizer2::getResizer(const size_t srcX,const size_t srcY,const size_t dstX,const size_t dstY)
			{
				//increasing dimensions is not supported
				CV_DbgAssert(dstX<=srcX);
				CV_DbgAssert(dstY<=srcY);
				if(dstX==0||dstY==0||srcX==0||srcY==0)
				{
					return new NoOpResizer2(srcX,srcY,dstX,dstY);
				}
				if(dstX==srcX && dstY==srcY)
				{
					return new SameSizeResizer2(srcX,srcY,dstX,dstY);
				}
				double scaleX=1./((double)dstX/(double)srcX);
				double scaleY=1./((double)dstY/(double)srcY);
				long iScaleX=cv::saturate_cast<long>(scaleX);
				long iScaleY=cv::saturate_cast<long>(scaleY);
				if(std::abs(scaleX-iScaleX) < DBL_EPSILON && std::abs(scaleY-iScaleY)<DBL_EPSILON && iScaleX==2&&iScaleY==2)
				{
					if(hal & lyonste::hal::HAL_SSE2)
					{
						return new SSE2HalfSizeShrinker2(srcX,srcY,dstX,dstY);
					}
					return new NonHALHalfSizeShrinker2(srcX,srcY,dstX,dstY);
				}
				if(hal&lyonste::hal::HAL_SSE2)
				{
					return new SSE2LinearShrinker2(srcX,srcY,dstX,dstY,scaleX,scaleY);
				}
				return new  NonHALLinearShrinker2(srcX,srcY,dstX,dstY,scaleX,scaleY);
			}
		}

	}
}

//BEGIN DEFINITIONS
namespace lyonste
{
	namespace matrix
	{
		namespace resizer
		{
			using namespace lyonste::hal;


			enum ResizerType
			{
				NoOp,
				SameSize,
				HalfSizeShrink,
				LinearShrink
			};

			//TODO implement interpolation option
			//enum InterpolationType
			//{
			//	Nearest,
			//	Linear,
			//	Cubic,
			//	Area,
			//	Lanczos4
			//};

			template<HAL_FLAG hal> GrayMat2DResizer<hal>::~GrayMat2DResizer()
			{}



			template<ResizerType resizerType,HAL_FLAG hal> class GrayMat2DResizerImpl;

			template<HAL_FLAG hal> class GrayMat2DResizerImpl<NoOp,hal>: public GrayMat2DResizer<hal>
			{
			private:
				friend class GrayMat2DResizer<hal>;
				constexpr GrayMat2DResizerImpl()
				{}

			public:
				void resize(const Ptr2D<uchar>& src,Ptr2D<uchar>& dst) override
				{
					//do nothing
				}
			};
			template<HAL_FLAG hal> class GrayMat2DResizerImpl<SameSize,hal>: public GrayMat2DResizer<hal>
			{
			private:
				friend class GrayMat2DResizer<hal>;
				constexpr GrayMat2DResizerImpl()
				{

				}
			public:
				void resize(const Ptr2D<uchar>& src,Ptr2D<uchar>& dst) override
				{
					CV_DbgAssert(src.x==dst.x);
					CV_DbgAssert(src.y==dst.y);
					CV_DbgAssert(src.x);
					CV_DbgAssert(src.y);
					CV_DbgAssert(src.data);
					std::memmove(dst.data,src.data,src.x*src.y);
				}
			};
			template<HAL_FLAG hal> class GrayMat2DResizerImpl<HalfSizeShrink,hal>: public GrayMat2DResizer<hal>
			{
			private:
				friend class GrayMat2DResizer<hal>;
				constexpr GrayMat2DResizerImpl()
				{}

				template<HAL_FLAG implHal=hal> static constexpr
					std::enable_if_t<!HalFlagInfo<implHal>::hasSSE2> resizeImpl(size_t dx,const size_t dstX,uchar* const dstRow,const uchar* const srcRow0,const uchar* const srcRow1)
				{
					for(;dx<dstX;++dx)
					{
						const size_t index=dx<<1;
						dstRow[dx]=(uchar)((srcRow0[index]+srcRow0[index+1]+srcRow1[index]+srcRow1[index+1]+2)>>2);
					}
				}
				template<HAL_FLAG implHal=hal> static constexpr
					std::enable_if_t<HalFlagInfo<implHal>::hasSSE2> resizeImpl(size_t dx,const size_t dstX,uchar* const dstRow,const uchar* const srcRow0,const uchar* const srcRow1)
				{
					if(dstX>=8)
					{
						const __m128i zero_128=_mm_setzero_si128();
						const __m128i delta_128=_mm_set1_epi16(2);
						const __m128i maskLow_128=_mm_set1_epi16(0x00ff);
						const size_t xBound=dstX-8;
						uchar* dstRow_simd=dstRow;
						const uchar* srcRow0_simd=srcRow0;
						const uchar* srcRow1_simd=srcRow1;
						for(; dx<=xBound; dx+=8,srcRow0_simd+=16,srcRow1_simd+=16,dstRow_simd+=8)
						{
							const __m128i r0=_mm_loadu_si128((const __m128i*)srcRow0_simd);
							const __m128i r1=_mm_loadu_si128((const __m128i*)srcRow1_simd);
							__m128i s0=_mm_add_epi16(_mm_srli_epi16(r0,8),_mm_and_si128(r0,maskLow_128));
							const __m128i s1=_mm_add_epi16(_mm_srli_epi16(r1,8),_mm_and_si128(r1,maskLow_128));
							s0=_mm_add_epi16(_mm_add_epi16(s0,s1),delta_128);
							s0=_mm_packus_epi16(_mm_srli_epi16(s0,2),zero_128);
							_mm_storel_epi64((__m128i*)dstRow_simd,s0);
						}
					}
					resizeImpl<HAL_NONE>(dx,dstX,dstRow,srcRow0,srcRow1);
				}

			public:
				void resize(const Ptr2D<uchar>& src,Ptr2D<uchar>& dst) override
				{
					const size_t dstX=dst.x;
					const size_t dstY=dst.y;
					const size_t srcX=src.x;
					CV_DbgAssert(dstX);
					CV_DbgAssert(dstY);
					CV_DbgAssert(srcX==dstX<<1);
					CV_DbgAssert(src.y==dstY<<1);
					CV_DbgAssert(src.data);
					for(size_t y=0;y<dstY;++y)
					{
						const uchar* const srcRow0=src.ptr(y<<1);
						resizeImpl(0,dstX,dst.ptr(y),srcRow0,srcRow0+srcX);
					}
				}
			};





			template<HAL_FLAG hal> class GrayMat2DResizerImpl<LinearShrink,hal>: public GrayMat2DResizer<hal>,private matrix::Mat1D<uchar>
			{
			private:
				static constexpr const size_t * const initializeLinearXOffsetsAndBlend(size_t dstX,double scaleX,size_t* xOffsets)
				{
					short* const xBlend=(short*)(xOffsets+dstX);
					for(size_t dx=0,interpolationOffset=-1;dx<dstX;++dx)
					{
						float fx=(float)((dx+0.5)*scaleX-0.5);
						size_t sx=size_t(fx);
						fx-=sx;
						CV_DbgAssert(sx>=0);
						xOffsets[dx]=sx;
						xBlend[++interpolationOffset]=cv::saturate_cast<short>((1.f-fx)*2048);
						xBlend[++interpolationOffset]=cv::saturate_cast<short>(fx*2048);
					}
					return xOffsets;
				}


				static constexpr const size_t* const initializeLinearYOffsetsAndBlend(size_t dstY,double scaleY,size_t* yOffsets)
				{
					short* const yBlend=(short*)(yOffsets+dstY);
					for(size_t dy=0,interpolationOffset=-1;dy<dstY;++dy)
					{
						float fy=(float)((dy+0.5)*scaleY-0.5);
						size_t sy=size_t(fy);
						fy-=sy;
						yOffsets[dy]=sy;
						yBlend[++interpolationOffset]=cv::saturate_cast<short>((1.f-fy)*2048);
						yBlend[++interpolationOffset]=cv::saturate_cast<short>(fy*2048);
					}
					return yOffsets;
				}

				template<HAL_FLAG implHal=hal> static constexpr
					std::enable_if_t<!HalFlagInfo<implHal>::hasSSE2,size_t> getBuffStep(const size_t dstX)
				{
					return dstX;
				}
				template<HAL_FLAG implHal=hal> static constexpr
					std::enable_if_t<HalFlagInfo<implHal>::hasSSE2,size_t> getBuffStep(const size_t dstX)
				{
					return (dstX+15)&-16;
				}



				template<HAL_FLAG implHal=hal> constexpr
					std::enable_if_t<!HalFlagInfo<implHal>::hasSSE2> vResizeImpl(size_t dx,uchar* const dst,const short beta0,const short beta1)
				{
					for(; dx < dstX; ++dx)
					{
						dst[dx]=uchar((((beta0 * (tmpRow0[dx]>>4))>>16)+((beta1 * (tmpRow1[dx]>>4))>>16)+2)>>2);
					}
				}

				template<HAL_FLAG implHal=hal> constexpr
					std::enable_if_t<HalFlagInfo<implHal>::hasSSE2> vResizeImpl(size_t dx,uchar* const dst,const short beta0,const short beta1)
				{
					if(dstX>=16)
					{
						__m128i beta0_128=_mm_set1_epi16(beta0);
						__m128i beta1_128=_mm_set1_epi16(beta1);
						__m128i delta_128=_mm_set1_epi16(2);
						size_t xBound=dstX-16;
						if((((size_t)tmpRow0|(size_t)tmpRow1)&15)==0)
						{
							for(; dx<=xBound; dx+=16)
							{
								size_t offset=dx;
								__m128i x0=_mm_loadu_si128((const __m128i*)(tmpRow0+offset));
								__m128i y0=_mm_loadu_si128((const __m128i*)(tmpRow1+offset));
								offset+=4;
								__m128i x1=_mm_loadu_si128((const __m128i*)(tmpRow0+offset));
								__m128i y1=_mm_loadu_si128((const __m128i*)(tmpRow1+offset));
								offset+=4;
								x0=_mm_packs_epi32(_mm_srai_epi32(x0,4),_mm_srai_epi32(x1,4));
								y0=_mm_packs_epi32(_mm_srai_epi32(y0,4),_mm_srai_epi32(y1,4));
								x1=_mm_loadu_si128((const __m128i*)(tmpRow0+offset));
								y1=_mm_loadu_si128((const __m128i*)(tmpRow1+offset));
								offset+=4;
								__m128i x2=_mm_loadu_si128((const __m128i*)(tmpRow0+offset));
								__m128i y2=_mm_loadu_si128((const __m128i*)(tmpRow1+offset));
								x1=_mm_packs_epi32(_mm_srai_epi32(x1,4),_mm_srai_epi32(x2,4));
								y1=_mm_packs_epi32(_mm_srai_epi32(y1,4),_mm_srai_epi32(y2,4));
								x0=_mm_adds_epi16(_mm_mulhi_epi16(x0,beta0_128),_mm_mulhi_epi16(y0,beta1_128));
								x1=_mm_adds_epi16(_mm_mulhi_epi16(x1,beta0_128),_mm_mulhi_epi16(y1,beta1_128));
								x0=_mm_srai_epi16(_mm_adds_epi16(x0,delta_128),2);
								x1=_mm_srai_epi16(_mm_adds_epi16(x1,delta_128),2);
								_mm_storeu_si128((__m128i*)(dst+dx),_mm_packus_epi16(x0,x1));
							}
						}
						else
						{
							for(; dx<=xBound; dx+=16)
							{
								size_t offset=dx;
								__m128i x0=_mm_load_si128((const __m128i*)(tmpRow0+offset));
								__m128i y0=_mm_load_si128((const __m128i*)(tmpRow1+offset));
								offset+=4;
								__m128i x1=_mm_load_si128((const __m128i*)(tmpRow0+offset));
								__m128i y1=_mm_load_si128((const __m128i*)(tmpRow1+offset));
								offset+=4;
								x0=_mm_packs_epi32(_mm_srai_epi32(x0,4),_mm_srai_epi32(x1,4));
								y0=_mm_packs_epi32(_mm_srai_epi32(y0,4),_mm_srai_epi32(y1,4));
								x1=_mm_load_si128((const __m128i*)(tmpRow0+offset));
								y1=_mm_load_si128((const __m128i*)(tmpRow1+offset));
								offset+=4;
								__m128i x2=_mm_load_si128((const __m128i*)(tmpRow0+offset));
								__m128i y2=_mm_load_si128((const __m128i*)(tmpRow1+offset));
								x1=_mm_packs_epi32(_mm_srai_epi32(x1,4),_mm_srai_epi32(x2,4));
								y1=_mm_packs_epi32(_mm_srai_epi32(y1,4),_mm_srai_epi32(y2,4));
								x0=_mm_adds_epi16(_mm_mulhi_epi16(x0,beta0_128),_mm_mulhi_epi16(y0,beta1_128));
								x1=_mm_adds_epi16(_mm_mulhi_epi16(x1,beta0_128),_mm_mulhi_epi16(y1,beta1_128));
								x0=_mm_srai_epi16(_mm_adds_epi16(x0,delta_128),2);
								x1=_mm_srai_epi16(_mm_adds_epi16(x1,delta_128),2);
								_mm_storeu_si128((__m128i*)(dst+dx),_mm_packus_epi16(x0,x1));
							}
						}
						xBound=dstX-4;
						for(; dx < xBound; dx+=4)
						{
							__m128i x0=_mm_srai_epi32(_mm_loadu_si128((const __m128i*)(tmpRow0+dx)),4);
							__m128i y0=_mm_srai_epi32(_mm_loadu_si128((const __m128i*)(tmpRow1+dx)),4);
							x0=_mm_packs_epi32(x0,x0);
							y0=_mm_packs_epi32(y0,y0);
							x0=_mm_adds_epi16(_mm_mulhi_epi16(x0,beta0_128),_mm_mulhi_epi16(y0,beta1_128));
							x0=_mm_srai_epi16(_mm_adds_epi16(x0,delta_128),2);
							x0=_mm_packus_epi16(x0,x0);
							*(int*)(dst+dx)=_mm_cvtsi128_si32(x0);
						}
					}
					vResizeImpl<HAL_NONE>(dx,dst,beta0,beta1);
				}



				friend class GrayMat2DResizer<hal>;

				const size_t srcX;
				const size_t srcY;
				const size_t dstX;
				const size_t dstY;
				int* const tmpRow0;
				int* const tmpRow1;
				const size_t* const xOffsets;
				const short* const xBlend;
				const size_t* const yOffsets;
				const short* const yBlend;
				const size_t buffStep;

				//delegate constructor
				constexpr GrayMat2DResizerImpl(size_t srcX,size_t srcY,size_t dstX,size_t dstY,double scaleX,double scaleY,const size_t buffStep):
					Mat1D<uchar>(((dstX+dstY)*(sizeof(size_t)+(sizeof(short)<<1)))+((buffStep<<1)*sizeof(int))),
					srcX(srcX),
					srcY(srcY),
					dstX(dstX),
					dstY(dstY),
					tmpRow0((int*)data),
					tmpRow1(tmpRow0+buffStep),
					xOffsets(initializeLinearXOffsetsAndBlend(dstX,scaleX,(size_t*)(tmpRow1+buffStep))),
					xBlend((short*)(xOffsets+dstX)),
					yOffsets(initializeLinearYOffsetsAndBlend(dstY,scaleY,(size_t*)(xBlend+(dstX<<1)))),
					yBlend((short*)(yOffsets+dstY)),
					buffStep(buffStep)
				{

				}


				constexpr GrayMat2DResizerImpl(size_t srcX,size_t srcY,size_t dstX,size_t dstY,double scaleX,double scaleY):
					GrayMat2DResizerImpl<LinearShrink,hal>(srcX,srcY,dstX,dstY,scaleX,scaleY,getBuffStep(dstX))
				{

				}

			public:



				void resize(const Ptr2D<uchar>& src,Ptr2D<uchar>& dst) override
				{
					CV_DbgAssert(src.x==srcX);
					CV_DbgAssert(src.y==srcY);
					CV_DbgAssert(dst.x==dstX);
					CV_DbgAssert(dst.y==dstY);
					uchar* dstPtr=dst.data;
					const short* betaPtr=yBlend;
					for(size_t dy=0,prevSY0=-1,prevSY1=-1; dy < dstY; ++dy,dstPtr+=dstX,betaPtr+=2)
					{
						const size_t sy0=yOffsets[dy];
						CV_DbgAssert(sy0<srcY);
						if(sy0==prevSY0)
						{
							CV_DbgAssert(sy0+1<srcY);
							const uchar* const srcRow1=src.ptr(prevSY1=sy0+1);
							for(size_t dx=0,alphaOffset=-1;dx<dstX;++dx)
							{
								const size_t sx=xOffsets[dx];
								tmpRow1[dx]=srcRow1[sx]*xBlend[++alphaOffset]+srcRow1[sx+1]*xBlend[++alphaOffset];
							}
						}
						else
						{
							if(sy0==prevSY1)
							{
								std::memcpy(tmpRow0,tmpRow1,buffStep*sizeof(int));
							}
							const uchar* const srcRow0=src.ptr(prevSY0=sy0);
							const uchar* const srcRow1=src.ptr(prevSY1=clip(sy0+1,srcY));
							for(size_t dx=0,alphaOffset=-1;dx<dstX;++dx)
							{
								const size_t sx=xOffsets[dx];
								const int alpha0=xBlend[++alphaOffset];
								const int alpha1=xBlend[++alphaOffset];
								tmpRow0[dx]=srcRow0[sx]*alpha0+srcRow0[sx+1]*alpha1;
								tmpRow1[dx]=srcRow1[sx]*alpha0+srcRow1[sx+1]*alpha1;
							}
						}
						vResizeImpl(0,dstPtr,betaPtr[0],betaPtr[1]);
					}
				}


			};



			template<HAL_FLAG hal> constexpr
				GrayMat2DResizer<hal>* GrayMat2DResizer<hal>::getResizer(size_t srcX,size_t srcY,size_t dstX,size_t dstY)
			{
				if(dstX>srcX||dstY>srcY)
				{
					CV_Error(1,"frame growing not currently supported");
				}
				if(!dstX||!dstY||!srcX||!srcY)
				{
					return new GrayMat2DResizerImpl<NoOp,hal>();
				}
				if(dstX==srcX && dstY==srcY)
				{
					return new GrayMat2DResizerImpl<SameSize,hal>();
				}
				double scaleX=1./((double)dstX/(double)srcX);
				double scaleY=1./((double)dstY/(double)srcY);
				long iScaleX=cv::saturate_cast<long>(scaleX);
				long iScaleY=cv::saturate_cast<long>(scaleY);
				if(std::abs(scaleX-iScaleX)<DBL_EPSILON && std::abs(scaleY-iScaleY)<DBL_EPSILON && iScaleX==2&&iScaleY==2)
				{
					return new GrayMat2DResizerImpl<HalfSizeShrink,hal>();
				}
				return new GrayMat2DResizerImpl<LinearShrink,hal>(srcX,srcY,dstX,dstY,scaleX,scaleY);
			}












		}
	}
}
//END DEFINITIONS