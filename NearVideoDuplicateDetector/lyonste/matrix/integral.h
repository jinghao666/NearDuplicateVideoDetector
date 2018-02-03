#pragma once

#include "matrix.h"
#include "../hal.h"

namespace lyonste
{
	namespace matrix
	{
		namespace integral
		{
			template<lyonste::hal::HAL_FLAG hal>
			struct IntegralMat
			{
			public:
				virtual void calculateIntegral(const matrix::Ptr2D<uchar>& frame)=0;
				virtual uchar meanIntensity(float xf,float yf,float radius) const=0;
				virtual ~IntegralMat()
				{}
			};

			template<lyonste::hal::HAL_FLAG hal,typename SUM_TYPE>
			class IntegralMatImpl: public matrix::Ptr2D<SUM_TYPE>,public IntegralMat<hal>
			{
			private:
				IntegralMatImpl(const size_t x,const size_t y,const size_t capacity):
					matrix::Ptr2D<SUM_TYPE>(x,y,capacity?new SUM_TYPE[capacity]:NULL)
				{
					SUM_TYPE* sumPtr=data;
					std::memset(sumPtr,0,x*sizeof(SUM_TYPE));
					for(size_t dy=1;dy<y;++dy)
					{
						sumPtr[dy*x]=0;
					}
				}

				template<lyonste::hal::HAL_FLAG implHal=hal,typename IMPL_SUM_TYPE=SUM_TYPE>
				std::enable_if_t<lyonste::hal::HalFlagInfo<implHal>::hasSSE2 && std::is_same<IMPL_SUM_TYPE,int>::value>
					integralImpl(const size_t frameX,const size_t frameY,const uchar* src_row)
				{
					__m128i v_zero=_mm_setzero_si128();
					IMPL_SUM_TYPE* sum_row=data+x+1;
					for(size_t dy=0;dy<frameY;++dy,src_row+=frameX,sum_row+=x)
					{
						IMPL_SUM_TYPE* prev_sum_row=sum_row-x;
						__m128i prev=v_zero;
						size_t dx=0;
						for(;dx+7<frameX;dx+=8)
						{
							__m128i vsuml=_mm_loadu_si128((const __m128i *)(prev_sum_row+dx));
							__m128i vsumh=_mm_loadu_si128((const __m128i *)(prev_sum_row+dx+4));

							__m128i el8shr0=_mm_loadl_epi64((const __m128i *)(src_row+dx));
							__m128i el8shr1=_mm_slli_si128(el8shr0,1);
							__m128i el8shr2=_mm_slli_si128(el8shr0,2);
							__m128i el8shr3=_mm_slli_si128(el8shr0,3);

							vsuml=_mm_add_epi32(vsuml,prev);
							vsumh=_mm_add_epi32(vsumh,prev);

							__m128i el8shr12=_mm_add_epi16(_mm_unpacklo_epi8(el8shr1,v_zero),
														   _mm_unpacklo_epi8(el8shr2,v_zero));
							__m128i el8shr03=_mm_add_epi16(_mm_unpacklo_epi8(el8shr0,v_zero),
														   _mm_unpacklo_epi8(el8shr3,v_zero));
							__m128i el8=_mm_add_epi16(el8shr12,el8shr03);

							__m128i el4h=_mm_add_epi16(_mm_unpackhi_epi16(el8,v_zero),
													   _mm_unpacklo_epi16(el8,v_zero));

							vsuml=_mm_add_epi32(vsuml,_mm_unpacklo_epi16(el8,v_zero));
							vsumh=_mm_add_epi32(vsumh,el4h);

							_mm_storeu_si128((__m128i *)(sum_row+dx),vsuml);
							_mm_storeu_si128((__m128i *)(sum_row+dx+4),vsumh);

							prev=_mm_add_epi32(prev,_mm_shuffle_epi32(el4h,_MM_SHUFFLE(3,3,3,3)));
						}
						for(IMPL_SUM_TYPE v=sum_row[dx-1]-prev_sum_row[dx-1]; dx < frameX; ++dx)
							sum_row[dx]=(v+=src_row[dx])+prev_sum_row[dx];
					}
				}

				template<lyonste::hal::HAL_FLAG implHal=hal,typename IMPL_SUM_TYPE=SUM_TYPE>
				std::enable_if_t<!lyonste::hal::HalFlagInfo<implHal>::hasSSE2||!std::is_same<IMPL_SUM_TYPE,int>::value>
					integralImpl(const size_t frameX,const size_t frameY,const uchar*  src_row)
				{
					SUM_TYPE* sum_row=data+x+1;
					for(size_t dy=0;dy<frameY;++dy,src_row+=frameX,sum_row+=x)
					{
						IMPL_SUM_TYPE s=0;
						for(size_t dx=0;dx<frameX;++dx)
						{
							s+=src_row[dx];
							sum_row[dx]=sum_row[dx-x]+s;
						}
					}

				}

			public:
				IntegralMatImpl(const size_t x,const size_t y):
					IntegralMatImpl(x+1,y+1,(x+1)*(y+1))
				{
					
				}

				void calculateIntegral(const matrix::Ptr2D<uchar>& frame) override
				{
					CV_DbgAssert(frame.x==x-1);
					CV_DbgAssert(frame.y==y-1);
					CV_DbgAssert(frame.data);
					integralImpl(frame.x,frame.y,frame.data);
				}


				uchar meanIntensity(float xf,float yf,float radius) const override
				{
					const size_t xL=size_t(xf-radius+0.5);
					const size_t yT=size_t(yf-radius+0.5);
					const size_t xR=size_t(xf+radius+1.5);
					const size_t yB=size_t(yf+radius+1.5);
					const size_t area=(xR-xL)*(yB-yT);
					return static_cast<uchar>(((val(xR,yB)-val(xL,yB)+val(xL,yT)-val(xR,yT))+(area>>1))/area);
				}

				~IntegralMatImpl()
				{
					delete[] data;
				}

			};

		}
	}
}