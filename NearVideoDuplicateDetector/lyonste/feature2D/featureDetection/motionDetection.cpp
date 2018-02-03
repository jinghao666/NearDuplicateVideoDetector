#include "featureDetection.h"

namespace lyonste
{
	namespace feature2D
	{
		namespace featureDetection
		{
			namespace motionDetection
			{
				using namespace lyonste::hal;

				template<HAL_FLAG hal=HAL_NONE>
				class SingleFrameMotionDetectingKeyPointGenerator;

				template<HAL_FLAG hal=HAL_NONE>
				class MultiFrameMotionDetectingKeyPointGenerator;

				template<bool multiFrame=true,HAL_FLAG hal=HAL_NONE>
				class MotionDetectingKeyPointGeneratorFactory;

				template<bool multiFrame,HAL_FLAG hal>
				class MotionDetectingKeyPointGenerator;

				template<HAL_FLAG hal>
				class MotionDetectingKeyPointGeneratorFactory<false,hal>: public KeyPointGeneratorFactory
				{
				private:
					friend class MotionDetectingKeyPointGenerator<false,hal>;
					friend class SingleFrameMotionDetectingKeyPointGenerator<hal>;
					friend KeyPointGeneratorFactory* KeyPointGeneratorFactory::configureKeyPointGeneratorFactory<MotionDetecting>(const boost::property_tree::ptree& properties,const lyonste::hal::HAL_FLAG hal_flag);

					const KeyPointGeneratorFactory* const kpGenFactory;
					const float weightFactor;
					const short diffThreshold;

					constexpr MotionDetectingKeyPointGeneratorFactory(const KeyPointGeneratorFactory* kpGenFactory,float weightFactor,short diffThreshold):
						kpGenFactory(kpGenFactory),
						weightFactor(weightFactor),
						diffThreshold(diffThreshold)
					{}

				public:

					bool doResponse() const override
					{
						return kpGenFactory->doResponse();
					}
					bool doAngles() const override
					{
						return kpGenFactory->doAngles();
					}

					size_t getBorder() const override
					{
						return kpGenFactory->getBorder();
					}

					std::string toString() const noexcept override
					{
						return std::string(kpTypeStr<MotionDetecting>)+"{historyLength=1; weightFactor="+std::to_string(weightFactor)+"; diffThreshold="+std::to_string(diffThreshold)+"; hal="+std::to_string(hal)+"; kpGenFactory="+(kpGenFactory->operator std::string())+"}";
					}

					SingleFrameMotionDetectingKeyPointGenerator<hal>* getKeyPointGenerator(size_t cols,size_t rows,float kpSize=7.f,int octave=0,float angle=-1,float response=0,int classID=-1) const override;

					void writeProperties(boost::property_tree::ptree& properties) const override
					{
						properties.put<std::string>("keyPointType",kpTypeStr<MotionDetecting>);
						boost::property_tree::ptree kpGenFactoryProperties;
						kpGenFactory->writeProperties(kpGenFactoryProperties);
						properties.add_child("kpGenFactoryProperties",kpGenFactoryProperties);
						properties.put<size_t>("historyLength",1);
						properties.put<float>("weightFactor",weightFactor);
						properties.put<uchar>("diffThreshold",(uchar)diffThreshold);
					}

					~MotionDetectingKeyPointGeneratorFactory()
					{
						delete kpGenFactory;
					}

				};

				template<HAL_FLAG hal>
				class MotionDetectingKeyPointGeneratorFactory<true,hal>: public KeyPointGeneratorFactory
				{
				private:
					friend class MotionDetectingKeyPointGenerator<true,hal>;
					friend class MultiFrameMotionDetectingKeyPointGenerator<hal>;
					friend KeyPointGeneratorFactory* KeyPointGeneratorFactory::configureKeyPointGeneratorFactory<MotionDetecting>(const boost::property_tree::ptree& properties,const lyonste::hal::HAL_FLAG hal_flag);

					const KeyPointGeneratorFactory* const kpGenFactory;
					const size_t historyLength;
					const float weightFactor;
					const short diffThreshold;

					constexpr MotionDetectingKeyPointGeneratorFactory(const KeyPointGeneratorFactory* kpGenFactory,size_t historyLength,float weightFactor,short diffThreshold):
						kpGenFactory(kpGenFactory),
						historyLength(historyLength),
						weightFactor(weightFactor),
						diffThreshold(diffThreshold)
					{}

				public:
					MultiFrameMotionDetectingKeyPointGenerator<hal>* getKeyPointGenerator(size_t cols,size_t rows,float kpSize=7.f,int octave=0,float angle=-1,float response=0,int classID=-1) const override;

					std::string toString() const noexcept override
					{
						return std::string(kpTypeStr<MotionDetecting>)+"{historyLength="+std::to_string(historyLength)+"; weightFactor="+std::to_string(weightFactor)+"; diffThreshold="+std::to_string(diffThreshold)+"; hal="+std::to_string(hal)+"; kpGenFactory="+(kpGenFactory->operator std::string())+"}";
					}

					bool doResponse() const override
					{
						return kpGenFactory->doResponse();
					}
					bool doAngles() const override
					{
						return kpGenFactory->doAngles();
					}

					size_t getBorder() const override
					{
						return kpGenFactory->getBorder();
					}

					void writeProperties(boost::property_tree::ptree& properties) const override
					{
						properties.put<std::string>("keyPointType",kpTypeStr<MotionDetecting>);
						boost::property_tree::ptree kpGenFactoryProperties;
						kpGenFactory->writeProperties(kpGenFactoryProperties);
						properties.add_child("kpGenFactoryProperties",kpGenFactoryProperties);
						properties.put<size_t>("historyLength",historyLength);
						properties.put<float>("weightFactor",weightFactor);
						properties.put<uchar>("diffThreshold",(uchar)diffThreshold);
					}

					~MotionDetectingKeyPointGeneratorFactory()
					{
						delete kpGenFactory;
					}

				};

				template<bool multiFrame,HAL_FLAG hal>
				class MotionDetectingKeyPointGenerator: protected matrix::Mat3D<uchar>
				{
				protected:
					KeyPointGenerator* const kpGen;
					const MotionDetectingKeyPointGeneratorFactory<multiFrame,hal>* const factory;


					constexpr void weightResponse(cv::KeyPoint& keyPoint) const
					{
						const cv::Point2f& pt=keyPoint.pt;
						const size_t kpX=size_t(pt.x);
						const size_t kpY=size_t(pt.y);
						CV_DbgAssert(cvRound(pt.x)==kpX && kpX<x);
						CV_DbgAssert(cvRound(pt.y)==kpY && kpY<y);
						CV_DbgAssert(data);
						const float weightFactor=factory->weightFactor;
						for(size_t i=z-1;i--;)
						{
							if(data[x*y*i+x*kpY+kpX])
							{
								keyPoint.response*=weightFactor;
								return;
							}
						}
					}


					template<HAL_FLAG iterateHal=hal> static constexpr
						std::enable_if_t<!HalFlagInfo<iterateHal>::hasSSSE3&&!HalFlagInfo<iterateHal>::hasSSE2>
						iterate(size_t i,const uchar* const currFramePtr,uchar* const prevFramePtr,uchar* const combinedPtr,const size_t area,const short diffThreshold)
					{
						for(;i!=area;++i)
						{
							combinedPtr[i]=std::abs(short(currFramePtr[i])-short(prevFramePtr[i])) > diffThreshold?255:0;
						}
						std::memcpy(prevFramePtr,currFramePtr,area);
					}

					template<HAL_FLAG iterateHal=hal> static constexpr
						std::enable_if_t<HalFlagInfo<iterateHal>::hasSSSE3>
						iterate(size_t i,const uchar* const currFramePtr,uchar* const prevFramePtr,uchar* const combinedPtr,const size_t area,const short diffThreshold)
					{
						if(area>=16)
						{
							const size_t bound128=area-16;
							__m128i diffThreshold128Pos=_mm_set1_epi16(diffThreshold);
							__m128i diffThreshold128Neg=_mm_set1_epi16(-diffThreshold);
							if((((size_t)currFramePtr|(size_t)prevFramePtr)&15)==0)
							{
								//aligned
								for(; i < bound128; i+=16)
								{
									__m128i currFrame128lo=_mm_load_si128((const __m128i*)(currFramePtr+i));
									__m128i prevFrame128lo=_mm_load_si128((const __m128i*)(prevFramePtr+i));
									_mm_storeu_si128((__m128i*)(combinedPtr+i),_mm_packus_epi16(_mm_cmpgt_epi16(_mm_abs_epi16(_mm_sub_epi16(_mm_unpacklo_epi8(_mm_setzero_si128(),currFrame128lo),_mm_unpacklo_epi8(_mm_setzero_si128(),prevFrame128lo))),diffThreshold128Pos),_mm_cmpgt_epi16(_mm_abs_epi16(_mm_sub_epi16(_mm_unpackhi_epi8(_mm_setzero_si128(),currFrame128lo),_mm_unpackhi_epi8(_mm_setzero_si128(),prevFrame128lo))),diffThreshold128Pos)));
								}
							}
							else
							{
								//unaligned
								for(; i < bound128; i+=16)
								{
									__m128i currFrame128lo=_mm_loadu_si128((const __m128i*)(currFramePtr+i));
									__m128i prevFrame128lo=_mm_loadu_si128((const __m128i*)(prevFramePtr+i));
									_mm_storeu_si128((__m128i*)(combinedPtr+i),_mm_packus_epi16(_mm_cmpgt_epi16(_mm_abs_epi16(_mm_sub_epi16(_mm_unpacklo_epi8(_mm_setzero_si128(),currFrame128lo),_mm_unpacklo_epi8(_mm_setzero_si128(),prevFrame128lo))),diffThreshold128Pos),_mm_cmpgt_epi16(_mm_abs_epi16(_mm_sub_epi16(_mm_unpackhi_epi8(_mm_setzero_si128(),currFrame128lo),_mm_unpackhi_epi8(_mm_setzero_si128(),prevFrame128lo))),diffThreshold128Pos)));
								}
							}
						}
						iterate<HAL_NONE>(i,currFramePtr,prevFramePtr,combinedPtr,area,diffThreshold);
					}

					template<HAL_FLAG iterateHal=hal> static constexpr
						std::enable_if_t<!HalFlagInfo<iterateHal>::hasSSSE3 && HalFlagInfo<iterateHal>::hasSSE2>
						iterate(size_t i,const uchar* const currFramePtr,uchar* const prevFramePtr,uchar* const combinedPtr,const size_t area,const short diffThreshold)
					{
						if(area>=16)
						{
							const size_t bound128=area-16;
							__m128i diffThreshold128Pos=_mm_set1_epi16(diffThreshold);
							__m128i diffThreshold128Neg=_mm_set1_epi16(-diffThreshold);
							if((((size_t)currFramePtr|(size_t)prevFramePtr)&15)==0)
							{
								//aligned
								for(; i < bound128; i+=16)
								{
									__m128i currFrame128lo=_mm_load_si128((const __m128i*)(currFramePtr+i));
									__m128i prevFrame128lo=_mm_load_si128((const __m128i*)(prevFramePtr+i));
									__m128i diff128lo=_mm_sub_epi16(_mm_unpacklo_epi8(_mm_setzero_si128(),currFrame128lo),_mm_unpacklo_epi8(_mm_setzero_si128(),prevFrame128lo));
									__m128i diff128hi=_mm_sub_epi16(_mm_unpackhi_epi8(_mm_setzero_si128(),currFrame128lo),_mm_unpackhi_epi8(_mm_setzero_si128(),prevFrame128lo));
									_mm_storeu_si128((__m128i*)(combinedPtr+i),_mm_packus_epi16(_mm_or_si128(_mm_cmpgt_epi16(diff128lo,diffThreshold128Pos),_mm_cmplt_epi16(diff128lo,diffThreshold128Neg)),_mm_or_si128(_mm_cmpgt_epi16(diff128hi,diffThreshold128Pos),_mm_cmplt_epi16(diff128hi,diffThreshold128Neg))));
								}
							}
							else
							{
								//unaligned
								for(; i < bound128; i+=16)
								{
									__m128i currFrame128lo=_mm_loadu_si128((const __m128i*)(currFramePtr+i));
									__m128i prevFrame128lo=_mm_loadu_si128((const __m128i*)(prevFramePtr+i));
									__m128i diff128lo=_mm_sub_epi16(_mm_unpacklo_epi8(_mm_setzero_si128(),currFrame128lo),_mm_unpacklo_epi8(_mm_setzero_si128(),prevFrame128lo));
									__m128i diff128hi=_mm_sub_epi16(_mm_unpackhi_epi8(_mm_setzero_si128(),currFrame128lo),_mm_unpackhi_epi8(_mm_setzero_si128(),prevFrame128lo));
									_mm_storeu_si128((__m128i*)(combinedPtr+i),_mm_packus_epi16(_mm_or_si128(_mm_cmpgt_epi16(diff128lo,diffThreshold128Pos),_mm_cmplt_epi16(diff128lo,diffThreshold128Neg)),_mm_or_si128(_mm_cmpgt_epi16(diff128hi,diffThreshold128Pos),_mm_cmplt_epi16(diff128hi,diffThreshold128Neg))));
								}
							}
						}
						iterate<HAL_NONE>(i,currFramePtr,prevFramePtr,combinedPtr,area,diffThreshold);
					}

					constexpr MotionDetectingKeyPointGenerator(const size_t x,const size_t y,const size_t z,KeyPointGenerator* kpGen,const MotionDetectingKeyPointGeneratorFactory<multiFrame,hal>* factory):
						matrix::Mat3D<uchar>(x,y,z),
						kpGen(kpGen),
						factory(factory)
					{
						std::memset(data,0,x*y*z);
					}

				public:

					~MotionDetectingKeyPointGenerator()
					{
						delete kpGen;
					}
				};

				template<HAL_FLAG hal>
				class SingleFrameMotionDetectingKeyPointGenerator: public KeyPointGenerator,private MotionDetectingKeyPointGenerator<false,hal>
				{


				private:
					friend class MotionDetectingKeyPointGeneratorFactory<false,hal>;

					constexpr SingleFrameMotionDetectingKeyPointGenerator(const size_t x,const size_t y,KeyPointGenerator* kpGen,const MotionDetectingKeyPointGeneratorFactory<false,hal>* factory):
						MotionDetectingKeyPointGenerator<false,hal>(x,y,2,kpGen,factory)
					{

					}

				public:

					const MotionDetectingKeyPointGeneratorFactory<false,hal>* getFactory() const noexcept
					{
						return factory;
					}

					void detect(const matrix::Ptr2D<uchar>& frame,std::vector<cv::KeyPoint>& keyPoints) override
					{
						const size_t area=x*y;
						iterate(0,frame.data,data+area,data,area,factory->diffThreshold);
						kpGen->detect(frame,keyPoints);
						for(cv::KeyPoint& kp:keyPoints)
						{
							weightResponse(kp);
						}
					}
				};

				template<HAL_FLAG hal>
				class MultiFrameMotionDetectingKeyPointGenerator: public KeyPointGenerator,private MotionDetectingKeyPointGenerator<true,hal>
				{
				private:
					friend class MotionDetectingKeyPointGeneratorFactory<true,hal>;

					size_t frameIndex;

					constexpr MultiFrameMotionDetectingKeyPointGenerator(const size_t x,const size_t y,KeyPointGenerator* kpGen,const MotionDetectingKeyPointGeneratorFactory<true,hal>* factory):
						MotionDetectingKeyPointGenerator<true,hal>(x,y,factory->historyLength+1,kpGen,factory),
						frameIndex(-1)
					{

					}
				public:

					const MotionDetectingKeyPointGeneratorFactory<true,hal>* getFactory() const noexcept
					{
						return factory;
					}

					void detect(const matrix::Ptr2D<uchar>& frame,std::vector<cv::KeyPoint>& keyPoints) override
					{
						const size_t area=x*y;
						iterate(0,frame.data,data+area*(z-1),data+area*((++frameIndex)%(z-1)),area,factory->diffThreshold);
						kpGen->detect(frame,keyPoints);
						for(cv::KeyPoint& kp:keyPoints)
						{
							weightResponse(kp);
						}
					}
				};

				template<HAL_FLAG hal>
				inline SingleFrameMotionDetectingKeyPointGenerator<hal>* MotionDetectingKeyPointGeneratorFactory<false,hal>::getKeyPointGenerator(size_t cols,size_t rows,float kpSize,int octave,float angle,float response,int classID) const
				{
					KeyPointGenerator* kpGen=kpGenFactory->getKeyPointGenerator(cols,rows,kpSize,octave,angle,response,classID);
					return new SingleFrameMotionDetectingKeyPointGenerator<hal>(cols,rows,kpGen,this);
				}

				template<HAL_FLAG hal>
				inline MultiFrameMotionDetectingKeyPointGenerator<hal>* MotionDetectingKeyPointGeneratorFactory<true,hal>::getKeyPointGenerator(size_t cols,size_t rows,float kpSize,int octave,float angle,float response,int classID) const
				{
					KeyPointGenerator* kpGen=kpGenFactory->getKeyPointGenerator(cols,rows,kpSize,octave,angle,response,classID);
					return new MultiFrameMotionDetectingKeyPointGenerator<hal>(cols,rows,kpGen,this);
				}

			}

			boost::property_tree::ptree lyonste::feature2D::featureDetection::KeyPointPropertyGenerator<MotionDetecting>::getProperties()
			{
				CV_Assert(historyLength);
				CV_Assert(weightFactor==weightFactor&&weightFactor>1.0f);
				CV_Assert(diffThreshold<255&&diffThreshold>0);
				boost::property_tree::ptree properties;
				properties.put<std::string>("keyPointType","motionDetecting");
				properties.put_child("kpGenFactoryProperties",kpGenFactoryProperties);
				properties.put<size_t>("historyLength",historyLength);
				properties.put<float>("weightFactor",weightFactor);
				properties.put<uchar>("diffThreshold",diffThreshold);
				return properties;
			}

			template<>
			cv::Ptr<cv::Feature2D> KeyPointGeneratorFactory::configureOpenCVKeyPointGenerator<MotionDetecting>(const boost::property_tree::ptree & properties,bool)
			{
				CV_Error(1,"There is no OpenCV equivalent for a FeatureDetector with type \"MotionDetecting\"");
				return cv::Ptr<cv::Feature2D>();
			}


			template<>
			KeyPointGeneratorFactory* KeyPointGeneratorFactory::configureKeyPointGeneratorFactory<MotionDetecting>(const boost::property_tree::ptree& properties,const lyonste::hal::HAL_FLAG hal_flag)
			{
				const size_t historyLength=properties.get<size_t>("historyLength",1);
				CV_DbgAssert(historyLength);
				const float weightFactor=properties.get<float>("weightFactor",1000.f);
				CV_DbgAssert(weightFactor==weightFactor && weightFactor>1.0);
				const short diffThreshold=(short)properties.get<uchar>("diffThreshold",75);
				CV_DbgAssert(diffThreshold<255&&diffThreshold>0);
				const KeyPointGeneratorFactory* kpGenFactory=getKeyPointGeneratorFactory(properties.get_child("kpGenFactoryProperties"));
				CV_DbgAssert(kpGenFactory);
				if(historyLength>1)
				{
					if(lyonste::hal::supports<lyonste::hal::HAL_SSSE3>(hal_flag))
					{
						return new motionDetection::MotionDetectingKeyPointGeneratorFactory<true,lyonste::hal::HAL_SSSE3>(kpGenFactory,historyLength,weightFactor,diffThreshold);
					}
					else if(lyonste::hal::supports<lyonste::hal::HAL_SSE2>(hal_flag))
					{
						return new motionDetection::MotionDetectingKeyPointGeneratorFactory<true,lyonste::hal::HAL_SSE2>(kpGenFactory,historyLength,weightFactor,diffThreshold);
					}
					return new motionDetection::MotionDetectingKeyPointGeneratorFactory<true,lyonste::hal::HAL_NONE>(kpGenFactory,historyLength,weightFactor,diffThreshold);
				}
				if(lyonste::hal::supports<lyonste::hal::HAL_SSSE3>(hal_flag))
				{
					return new motionDetection::MotionDetectingKeyPointGeneratorFactory<false,lyonste::hal::HAL_SSSE3>(kpGenFactory,weightFactor,diffThreshold);
				}
				else if(lyonste::hal::supports<lyonste::hal::HAL_SSE2>(hal_flag))
				{
					return new motionDetection::MotionDetectingKeyPointGeneratorFactory<false,lyonste::hal::HAL_SSE2>(kpGenFactory,weightFactor,diffThreshold);
				}
				return new motionDetection::MotionDetectingKeyPointGeneratorFactory<false,lyonste::hal::HAL_NONE>(kpGenFactory,weightFactor,diffThreshold);
			}
		}
	}
}