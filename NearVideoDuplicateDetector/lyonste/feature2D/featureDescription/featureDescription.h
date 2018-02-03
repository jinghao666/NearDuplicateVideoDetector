#pragma once

#include "../../matrix/matrix.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/serialization/split_member.hpp>
namespace lyonste
{
	namespace feature2D
	{
		namespace featureDescription
		{
			enum DescriptorType
			{
				Freak
			};

			template<DescriptorType descType>
			class DescTypeInfo;

			template<DescriptorType descType>
			class DescriptorPropertyGenerator;

			template<DescriptorType descType>
			class Descriptors
			{
			private:
				size_t numDescriptors;
				
				typename DescTypeInfo<descType>::Elem* data;

				friend class boost::serialization::access;

				static constexpr typename DescTypeInfo<descType>::Elem* copyData(size_t numDescriptors,typename DescTypeInfo<descType>::Elem* thatData)
				{
					if(numDescriptors)
					{
						typename DescTypeInfo<descType>::Elem* data=new typename DescTypeInfo<descType>::Elem[numDescriptors*DescTypeInfo<descType>::descriptorCols];
						std::memcpy(data,thatData,DescTypeInfo<descType>::descriptorSize*numDescriptors);
						return data;
					}
					return NULL;
				}

				template<class Archive>
				constexpr void save(Archive& archive,const unsigned int version) const
				{
					archive & numDescriptors;
					if(numDescriptors)
					{
						archive & boost::serialization::make_array(data,numDescriptors*DescTypeInfo<descType>::descriptorCols);
					}
				}
				
				template<class Archive>
				constexpr void load(Archive& archive,const unsigned int version)
				{
					delete[] data;
					archive & numDescriptors;
					if(numDescriptors)
					{
						data=new typename DescTypeInfo<descType>::Elem[numDescriptors*DescTypeInfo<descType>::descriptorCols];
						archive & boost::serialization::make_array(data,numDescriptors*DescTypeInfo<descType>::descriptorCols);
					}
					else
					{
						data=NULL;
					}
				}
				
				template<class Archive>
				constexpr void serialize(Archive &archive,const unsigned int version)
				{
					boost::serialization::split_member(archive,*this,version);
				}

			public:

				constexpr Descriptors() noexcept
					:numDescriptors(0),
					data(NULL)
				{

				}

				constexpr bool empty() const noexcept
				{
					return numDescriptors==0;
				}

				constexpr size_t getNumDescriptors() const noexcept
				{
					return numDescriptors;
				}

				constexpr const typename DescTypeInfo<descType>::Elem* getData(const size_t row=0) const noexcept
				{
					return data+(DescTypeInfo<descType>::descriptorCols*row);
				}

				constexpr typename DescTypeInfo<descType>::Elem* getData(const size_t row=0) noexcept
				{
					return data+(DescTypeInfo<descType>::descriptorCols*row);
				}

				constexpr Descriptors(const Descriptors<descType>& that) noexcept
					:numDescriptors(that.numDescriptors),
					data(copyData(numDescriptors,that.data))
				{
				}

				constexpr Descriptors(Descriptors<descType>&& that) noexcept
					:numDescriptors(that.numDescriptors),
					data(that.data)
				{
					that.data=NULL;
				}

				constexpr Descriptors<descType>& operator=(const Descriptors<descType>& that) noexcept
				{
					if(this!=&that)
					{
						const size_t oldSize=numDescriptors
						numDescriptors=that.numDescriptors;
						if(oldSize<numDescriptors)
						{
							delete[] data;
							data=new typename DescTypeInfo<descType>::Elem[numDescriptors*DescTypeInfo<descType>::descriptorCols];
						}
						if(numDescriptors)
						{
							std::memcpy(data,that.data,numDescriptors*DescTypeInfo<descType>::descriptorSize);
						}
						else
						{
							data=NULL;
						}

					}
					return *this;
				}

				constexpr Descriptors<descType>& operator=(Descriptors<descType>&& that) noexcept
				{
					if(this!=&that)
					{
						numDescriptors=that.numDescriptors;
						data=that.data;
						that.data=NULL;
					}
					return *this;
				}

				constexpr Descriptors(const size_t numDescriptors) noexcept
					:numDescriptors(numDescriptors),
					data(numDescriptors?new typename DescTypeInfo<descType>::Elem[numDescriptors*DescTypeInfo<descType>::descriptorCols]:NULL)
				{
				}

				constexpr const cv::Mat toCVMat() const noexcept
				{
					return cv::Mat((int)numDescriptors,(int)DescTypeInfo<descType>::descriptorCols,(int)DescTypeInfo<descType>::openCVDescElemType,data,DescTypeInfo<descType>::descriptorSize);
				}

				constexpr const typename DescTypeInfo<descType>::Elem* ptr(const size_t row=0,const size_t col=0) const noexcept
				{
					return data+row*DescTypeInfo<descType>::descriptorCols+col;
				}

				~Descriptors() noexcept
				{
					delete[] data;
				}
			};

			template<DescriptorType descType>
			class DescriptorGenerator
			{
			public:

				virtual typename DescTypeInfo<descType>::DescriptorMat compute(const matrix::Ptr2D<uchar>& frame,std::vector<cv::KeyPoint>& keyPoints)=0;

				virtual ~DescriptorGenerator()
				{
				}
			};
			
			template<DescriptorType descType>
			class DescriptorGeneratorFactory
			{
			public:
				virtual DescriptorGenerator<descType>* getDescriptorGenerator(size_t cols,size_t rows) const=0;
				virtual void writeProperties(boost::property_tree::ptree& properties) const=0;
				virtual ~DescriptorGeneratorFactory()
				{

				}
				
				static DescriptorGeneratorFactory<descType>* getDescriptorGeneratorFactory(const boost::property_tree::ptree& properties,const lyonste::hal::HAL_FLAG hal=lyonste::hal::halFlag);
			};

			template<DescriptorType descType>
			class DescriptorMatcher
			{
			public:
				virtual double getFrameSimilarity(const Descriptors<descType>& desc1,const Descriptors<descType>& desc2) const noexcept=0;
				virtual ~DescriptorMatcher() noexcept
				{
				}

				static constexpr DescriptorMatcher<descType>* getDescriptorMatcher(const boost::property_tree::ptree& properties,lyonste::hal::HAL_FLAG halFlag=lyonste::hal::halFlag);
			};

			namespace hal
			{
				const uchar popCountTable[256]=
				{
					0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
					1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
					1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
					2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
					1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
					2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
					2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
					3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
				};
			}

			template<DescriptorType descType,cv::NormTypes norm,lyonste::hal::HAL_FLAG hal=HAL_NONE>
			class NearestNeighborBFDescriptorMatcher: public DescriptorMatcher<descType>
			{
			private:

				template<cv::NormTypes normImpl=norm,lyonste::hal::HAL_FLAG halImpl=hal> static constexpr
				std::enable_if_t<(normImpl==cv::NormTypes::NORM_HAMMING)&&(!lyonste::hal::HalFlagInfo<halImpl>::hasSSE2),int> distanceFunc(const uchar* query,const uchar* train,size_t i=0,int result=0)
				{
					for(;i<=DescTypeInfo<descType>::descriptorCols-4;i+=4)
					{
						result+=hal::popCountTable[query[i]^train[i]]+hal::popCountTable[query[i+1]^train[i+1]]+hal::popCountTable[query[i+2]^train[i+2]]+hal::popCountTable[query[i+3]^train[i+3]];
					}
					for(;i<DescTypeInfo<descType>::descriptorCols;++i)
					{
						result+=hal::popCountTable[query[i]^train[i]];
					}
					return result;
				}

				template<cv::NormTypes normImpl=norm,lyonste::hal::HAL_FLAG halImpl=hal> static constexpr
				std::enable_if_t<(normImpl==cv::NormTypes::NORM_HAMMING)&&(lyonste::hal::HalFlagInfo<halImpl>::hasSSE2),int> distanceFunc(const uchar* query,const uchar* train,size_t i=0,int result=0)
				{
					//using namespace cv;
					//v_uint32x4 t=v_setzero_u32();
					__m128i t=_mm_setzero_si128();
					for(; i<=DescTypeInfo<descType>::descriptorCols-16; i+=16)
					{
						//std::cout<<"i="<<i<<std::endl;
						//std::cout<<"query="<<((size_t)(query+i))<<std::endl;
						//std::cout<<"train="<<((size_t)(train+i))<<std::endl;
						__m128i query128=_mm_loadu_si128((const __m128i*)(query+i));
						__m128i train128=_mm_loadu_si128((const __m128i*)(train+i));
						__m128i p=_mm_xor_si128(query128,train128);
						__m128i m1=_mm_set1_epi32(0x55555555);
						__m128i m2=_mm_set1_epi32(0x33333333);
						__m128i m4=_mm_set1_epi32(0x0f0f0f0f);
						p=_mm_add_epi32(_mm_and_si128(_mm_srli_epi32(p,1),m1),_mm_and_si128(p,m1));
						p=_mm_add_epi32(_mm_and_si128(_mm_srli_epi32(p,2),m2),_mm_and_si128(p,m2));
						p=_mm_add_epi32(_mm_and_si128(_mm_srli_epi32(p,4),m4),_mm_and_si128(p,m4));
						p=_mm_adds_epi8(p,_mm_srli_si128(p,1));
						p=_mm_adds_epi8(p,_mm_srli_si128(p,2));
						t=_mm_add_epi32(t,_mm_and_si128(p,_mm_set1_epi32(0x000000ff)));
					}
					t=_mm_add_epi32(t,_mm_srli_si128(t,8));
					t=_mm_add_epi32(t,_mm_srli_si128(t, 4));
					return distanceFunc<normImpl,lyonste::hal::HAL_NONE>(query,train,i,result+_mm_cvtsi128_si32(t));
				}

			public:
				constexpr double getFrameSimilarity(const Descriptors<descType>& desc1,const Descriptors<descType>& desc2) const noexcept override
				{
					size_t numQueryDescriptors=desc1.getNumDescriptors();
					if(numQueryDescriptors)
					{
						size_t numTrainDescriptors=desc2.getNumDescriptors();
						if(numTrainDescriptors>1)
						{
							double avg=0.0;
							const typename DescTypeInfo<descType>::Elem* queryData=desc1.getData();
							const typename DescTypeInfo<descType>::Elem* const trainData=desc2.getData();
							for(size_t i=0;i!=numQueryDescriptors;++i,queryData+=DescTypeInfo<descType>::descriptorCols)
							{
								double dist1=DBL_MAX;
								double dist2=DBL_MAX;
								for(size_t j=0;j!=numTrainDescriptors;++j)
								{
									const double d=static_cast<double>(distanceFunc(queryData,trainData+DescTypeInfo<descType>::descriptorSize*j));
									if(d<dist2)
									{
										if(d<dist1)
										{
											dist2=dist1;
											dist1=d;
										}
										else
										{
											dist2=d;
										}
									}
								}
								avg+=(dist1/dist2);
							}
							return avg/numQueryDescriptors;
						}
					}
					return 1.0;
				}
			};
			
			template<DescriptorType descType>
			constexpr DescriptorMatcher<descType>* DescriptorMatcher<descType>::getDescriptorMatcher(const boost::property_tree::ptree & properties,lyonste::hal::HAL_FLAG halFlag)
			{
				const bool bruteForce=properties.get<bool>("bruteForce",true);
				const std::string matchMethod=properties.get<std::string>("matchMethod","knn2Ratio");
				const std::string normType=properties.get<std::string>("normType","hamming");
				if(bruteForce)
				{
					if(matchMethod=="knn2Ratio")
					{
						if(normType=="hamming")
						{
							if(lyonste::hal::supports<lyonste::hal::HAL_SSE2>(halFlag))
							{
								return new NearestNeighborBFDescriptorMatcher<descType,cv::NormTypes::NORM_HAMMING,lyonste::hal::HAL_SSE2>();
							}
							return new NearestNeighborBFDescriptorMatcher<descType,cv::NormTypes::NORM_HAMMING,lyonste::hal::HAL_NONE>();
						}
						CV_Error(1,"normType "+normType+" not supported");
					}
					CV_Error(1,"match method "+matchMethod+" not supported");
				}
				CV_Error(1,"non-brute-force match not supported");
				return NULL;
				
			}

			namespace freak
			{
				static const ushort NUM_ORIENTATIONS=256;
				static const uchar NUM_POINTS=43;
				static const uchar SMALLEST_KP_SIZE=7;
				static const uchar NUM_SCALES=64;
				static const ushort NUM_PAIRS=512;
				static const ushort NUM_ORIENTATION_PAIRS=45;

				constexpr ushort DEFAULT_PAIRS[NUM_PAIRS]=
				{
					404,431,818,511,181,52,311,874,774,543,719,230,417,205,11,
					560,149,265,39,306,165,857,250,8,61,15,55,717,44,412,
					592,134,761,695,660,782,625,487,549,516,271,665,762,392,178,
					796,773,31,672,845,548,794,677,654,241,831,225,238,849,83,
					691,484,826,707,122,517,583,731,328,339,571,475,394,472,580,
					381,137,93,380,327,619,729,808,218,213,459,141,806,341,95,
					382,568,124,750,193,749,706,843,79,199,317,329,768,198,100,
					466,613,78,562,783,689,136,838,94,142,164,679,219,419,366,
					418,423,77,89,523,259,683,312,555,20,470,684,123,458,453,833,
					72,113,253,108,313,25,153,648,411,607,618,128,305,232,301,84,
					56,264,371,46,407,360,38,99,176,710,114,578,66,372,653,
					129,359,424,159,821,10,323,393,5,340,891,9,790,47,0,175,346,
					236,26,172,147,574,561,32,294,429,724,755,398,787,288,299,
					769,565,767,722,757,224,465,723,498,467,235,127,802,446,233,
					544,482,800,318,16,532,801,441,554,173,60,530,713,469,30,
					212,630,899,170,266,799,88,49,512,399,23,500,107,524,90,
					194,143,135,192,206,345,148,71,119,101,563,870,158,254,214,
					276,464,332,725,188,385,24,476,40,231,620,171,258,67,109,
					844,244,187,388,701,690,50,7,850,479,48,522,22,154,12,659,
					736,655,577,737,830,811,174,21,237,335,353,234,53,270,62,
					182,45,177,245,812,673,355,556,612,166,204,54,248,365,226,
					242,452,700,685,573,14,842,481,468,781,564,416,179,405,35,
					819,608,624,367,98,643,448,2,460,676,440,240,130,146,184,
					185,430,65,807,377,82,121,708,239,310,138,596,730,575,477,
					851,797,247,27,85,586,307,779,326,494,856,324,827,96,748,
					13,397,125,688,702,92,293,716,277,140,112,4,80,855,839,1,
					413,347,584,493,289,696,19,751,379,76,73,115,6,590,183,734,
					197,483,217,344,330,400,186,243,587,220,780,200,793,246,824,
					41,735,579,81,703,322,760,720,139,480,490,91,814,813,163,
					152,488,763,263,425,410,576,120,319,668,150,160,302,491,515,
					260,145,428,97,251,395,272,252,18,106,358,854,485,144,550,
					131,133,378,68,102,104,58,361,275,209,697,582,338,742,589,
					325,408,229,28,304,191,189,110,126,486,211,547,533,70,215,
					670,249,36,581,389,605,331,518,442,822
				};
			}
			
			template<>
			class DescriptorPropertyGenerator<Freak>
			{
			public:
				boost::property_tree::ptree operator()(float patternScale=22.f,size_t numOctaves=4,bool orientationNormalized=true,bool scaleNormalized=true,const ushort(&selectedPairs)[freak::NUM_PAIRS]=freak::DEFAULT_PAIRS);
			};

			template<>
			class DescTypeInfo<Freak>
			{
			public:
				typedef uchar Elem;

				enum
				{
					descriptorCols=64,
					openCVDescElemType=cv::DataType<Elem>::type,
					descriptorSize=descriptorCols*sizeof(Elem),
					defaultNorm=cv::NormTypes::NORM_HAMMING
				};
				typedef Descriptors<Freak> DescriptorMat;
			};
		}
	}
}


