// NearVideoDuplicateDetector.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <iostream>
#include <opencv2/imgproc.hpp>
#include <list>
#include "opencv2/xfeatures2d.hpp"
#include <set>
#include <boost/property_tree/xml_parser.hpp>
#include "lyonste/fileManagement.h"
#include <opencv2/core/ocl.hpp>
#include "lyonste/feature2D/featureDetection/featureDetection.h"
#include "lyonste/feature2D/featureDescription/featureDescription.h"
#include "lyonste/matrix/resizer.h"
#include "lyonste/matrix/integral.h"
#include "lyonste/videoSigGen.h"
#include "lyonste/videoSigMatch.h"
#include <queue>
#include <chrono>
#ifdef _WIN32
#include <Windows.h>
#endif


#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#ifdef _DEBUG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

std::mutex coutMut;

#define PRINT_MUT_COUT
#ifdef PRINT_MUT_COUT

#define MUT_COUT(str)\
{\
	std::lock_guard<std::mutex> lock(coutMut);\
	std::cout << str<<std::endl;\
}
#else
#define MUT_COUT(str)
#endif


bool sortKPPosition(const cv::KeyPoint& kp1,const cv::KeyPoint kp2)
{
	if(kp1.pt.y < kp2.pt.y)
	{
		return true;
	}
	else if(kp1.pt.y==kp2.pt.y)
	{
		if(kp1.pt.x < kp2.pt.x)
		{
			return true;
		}
	}
	return false;
}


bool kpEqual(const cv::KeyPoint& kp1,const cv::KeyPoint kp2,bool compareResponse=true,bool compareAngle=true,bool compareSize=true,bool compareOctave=true,bool compareClassId=true)
{
	bool equal=true;

	if(kp1.pt.y!=kp2.pt.y)
	{
		return false;
		//cout << "index=" << index << "; kp1.pt.y    =" << kp1.pt.y << "; kp2.pt.y    =" << kp2.pt.y << endl;
	}
	if(kp1.pt.x!=kp2.pt.x)
	{
		return false;
		//cout << "index=" << index << "; kp1.pt.x    =" << kp1.pt.x << "; kp2.pt.x    =" << kp2.pt.x << endl;
	}
	if(compareResponse)
	{
		if(kp1.response!=kp2.response)
		{
			return false;
			//cout << "index=" << index << "; kp1.response=" << kp1.response << "; kp2.response=" << kp2.response << endl;
		}
	}
	if(compareAngle)
	{
		if(kp1.angle!=kp2.angle)
		{
			return false;
			//cout << "index=" << index << "; kp1.angle   =" << kp1.angle << "; kp2.angle   =" << kp2.angle << endl;
		}
	}
	if(compareSize)
	{
		if(kp1.size!=kp2.size)
		{
			return false;
			//cout << "index=" << index << "; kp1.size    =" << kp1.size << "; kp2.size    =" << kp2.size << endl;
		}
	}
	if(compareOctave)
	{
		if(kp1.octave!=kp2.octave)
		{
			return false;
			//cout << "index=" << index << "; kp1.octave  =" << kp1.octave << "; kp2.octave  =" << kp2.octave << endl;
		}
	}
	if(compareClassId)
	{
		if(kp1.class_id!=kp2.class_id)
		{
			return false;
			//cout << "index=" << index << "; kp1.class_id=" << kp1.class_id << "; kp2.class_id=" << kp2.class_id << endl;
		}
	}

	return true;

}

void printKp(const cv::KeyPoint& kp)
{
	using namespace std;
	cout<<"x="<<kp.pt.x<<"; y="<<kp.pt.y<<"; score="<<kp.response<<"; angle="<<kp.angle<<"; size="<<kp.size<<"; octave="<<kp.octave<<"; classID="<<kp.class_id;
}
void compareKeyPoints(std::vector<cv::KeyPoint>& keyPoints1,std::vector<cv::KeyPoint>& keyPoints2)
{
	using namespace std;
	std::sort(keyPoints1.begin(),keyPoints1.end(),sortKPPosition);
	size_t bound=min(keyPoints1.size(),keyPoints2.size());
	for(size_t i=0; i < bound; ++i)
	{
		float responseRatio=keyPoints2[i].response/keyPoints1[i].response;
		cout<<"{"<<keyPoints1[i].pt.x<<","<<keyPoints1[i].pt.y<<"} {"<<keyPoints2[i].pt.x<<","<<keyPoints2[i].pt.y<<"} score "<<keyPoints1[i].response<<"; "<<keyPoints2[i].response<<"; responseRatio="<<responseRatio<<endl;
	}
}

namespace lyonste
{
	namespace feature2D
	{
		namespace featureDetection
		{
			using namespace lyonste::hal;

			void printOutKeyPoints(const std::vector<cv::KeyPoint>& keyPoints)
			{
				for(const cv::KeyPoint& kp:keyPoints)
				{
					std::cout<<"keyPoints[";
					printKp(kp);
					std::cout<<"]"<<std::endl;
				}
			}


			struct KeyPointSorterPosition
			{
				bool operator()(const cv::KeyPoint& kp1,const cv::KeyPoint& kp2) const
				{
					const int octave1=kp1.octave;
					const int octave2=kp2.octave;
					if(octave1<octave2)
					{
						return true;
					}
					if(octave1==octave2)
					{
						const cv::Point2f& pt1=kp1.pt;
						const cv::Point2f& pt2=kp2.pt;
						const float kpY1=pt1.y;
						const float kpY2=pt2.y;
						if(kpY1<kpY2)
						{
							return true;
						}
						if(kpY1==kpY2)
						{
							const float kpX1=pt1.x;
							const float kpX2=pt2.x;
							if(kpX1<kpX2)
							{
								return true;
							}
						}
					}
					return false;
				}
			};

			void printOutKeyPoints( std::vector<cv::KeyPoint>& customKeyPoints1, std::vector<cv::KeyPoint>& customKeyPoints2,bool compareResponse=true,bool compareAngle=true,bool compareSize=true,bool compareOctave=true,bool compareClassId=true)
			{
				bool printNonMatching=true;
				std::sort(customKeyPoints1.begin(),customKeyPoints1.end(),KeyPointSorterPosition());
				std::sort(customKeyPoints2.begin(),customKeyPoints2.end(),KeyPointSorterPosition());
				const size_t size1=customKeyPoints1.size();
				const size_t size2=customKeyPoints2.size();
				std::cout<<"customKeyPoints1.size()="<<size1<<std::endl;
				std::cout<<"customKeyPoints2.size()="<<size2<<std::endl;
				for(size_t i=0;;++i)
				{
					
					if(i<size1)
					{
						
						if(i<size2)
						{
							
							if(printNonMatching)
							{
								const cv::KeyPoint& kp1=customKeyPoints1[i];
								const cv::KeyPoint& kp2=customKeyPoints2[i];
								if(!kpEqual(kp1,kp2,compareResponse,compareAngle,compareSize,compareOctave,compareClassId))
								{
									std::cout<<"customKeyPoints1[";
									printKp(kp1);
									std::cout<<"]"<<std::endl;
									std::cout<<"customKeyPoints2[";
									printKp(kp2);
									std::cout<<"]"<<std::endl;

									std::cout<<"xDiff="<<(kp1.pt.x-kp2.pt.x)<<"; yDiff="<<(kp1.pt.y-kp2.pt.y)<<std::endl;
								}
							}
							else
							{
								std::cout<<"customKeyPoints1[";
								printKp(customKeyPoints1[i]);
								std::cout<<"]"<<std::endl;
								std::cout<<"customKeyPoints2[";
								printKp(customKeyPoints2[i]);
								std::cout<<"]"<<std::endl;
							}
						}
						else
						{
							if(!printNonMatching)
							{
								std::cout<<"customKeyPoints1[";
								printKp(customKeyPoints1[i]);
								std::cout<<"]"<<std::endl;
							}
							
						}
					}
					else
					{
						if(i<size2)
						{
							if(!printNonMatching)
							{
								std::cout<<"customKeyPoints2[";
								printKp(customKeyPoints2[i]);
								std::cout<<"]"<<std::endl;
							}
							
						}
						else
						{
							break;
						}
					}
				}
			}


			//void compareKeyPoints(std::vector<cv::KeyPoint>& keyPoints1,std::vector<cv::KeyPoint>& keyPoints2,const std::string& name="no-name",bool compareResponse=true,bool compareAngle=true,bool compareSize=true,bool compareOctave=true,bool compareClassId=true)
			//{
			//	const size_t size1=keyPoints1.size();
			//	const size_t size2=keyPoints2.size();
			//	std::vector<cv::KeyPoint> smaller=size1<size2?keyPoints1:keyPoints2;
			//	std::vector<cv::KeyPoint> bigger=size1<size2?keyPoints2:keyPoints1;;
			//	for(const cv::KeyPoint& kp1:smaller)
			//	{
			//		auto kpIter2=bigger.begin();
			//		bool foundMatch=false;
			//		for(; kpIter2!=bigger.end(); ++kpIter2)
			//		{
			//			if(kpEqual(kp1,*kpIter2,compareResponse,compareAngle,compareSize,compareOctave,compareClassId))
			//			{
			//				bigger.erase(kpIter2);
			//				foundMatch=true;
			//				break;
			//			}
			//		}
			//	}
			//	if(bigger.size()==0&&size1==size2)
			//	{
			//		//MUT_COUT(name<<" -->    match numKeyPoints="<<size1);
			//	}
			//	else
			//	{
			//		{
			//			std::lock_guard<std::mutex> lock(coutMut);
			//			std::cout<<name<<" --> no match size1="<<size1<<"; size2="<<size2<<"; bigger.size()="<<bigger.size()<<std::endl;
			//			printOutKeyPoints(keyPoints1,keyPoints2,compareResponse,compareAngle,compareSize,compareOctave,compareClassId);
			//		}
			//		throw std::exception("keypoint mismatch");
			//	}
			//}

//			double getCPUThreadTime()
//			{
//#ifdef _WIN32
//				FILETIME a,b,c,d;
//				//if(GetThreadTimes(GetCurrentThread(),&a,&b,&c,&d)!=0)
//				//{
//				//	return
//				//	(double)(d.dwLowDateTime|
//				//		((unsigned long long)d.dwHighDateTime<<32)) * 0.0000001;
//				//}
//				//else
//				//{
//
//				//	return 0;
//				//}
//				if(GetProcessTimes(GetCurrentProcess(),&a,&b,&c,&d)!=0)
//				{
//					//  Returns total user time.
//					//  Can be tweaked to include kernel times as well.
//					return
//						(double)(d.dwLowDateTime|
//						((unsigned long long)d.dwHighDateTime<<32)) * 0.0000001;
//				}
//				else
//				{
//					//  Handle error
//					return 0;
//				}
//#else
//				return 0;
//#endif
//			}

			void testKeyPoints(const size_t cols,const size_t rows,const matrix::Mat2D<uchar>& gray,const boost::property_tree::ptree& properties,std::atomic<size_t>& finishIndex,size_t totalNumTests)
			{
				cv::Ptr<cv::Feature2D> stockDetector=KeyPointGeneratorFactory::getOpenCVKeyPointGenerator(properties,properties.get<bool>("ignoreIncompatibleProperties",false));
				KeyPointGeneratorFactory* customDetectorFactory=KeyPointGeneratorFactory::getKeyPointGeneratorFactory(properties,properties.get<lyonste::hal::HAL_FLAG>("hal",lyonste::hal::halFlag));
				const std::string name=*customDetectorFactory;
				const bool compareResponse=properties.get<bool>("compareResponse",true);
				const bool compareAngle=properties.get<bool>("compareAngle",true);
				const bool compareSize=properties.get<bool>("compareSize",true);
				const bool compareOctave=properties.get<bool>("compareOctave",true);
				const bool compareClassId=properties.get<bool>("compareClassId",true);
				std::vector<cv::KeyPoint> stockKeyPoints;
				std::vector<cv::KeyPoint> customKeyPoints;
				KeyPointGenerator* customDetector=customDetectorFactory->getKeyPointGenerator(cols,rows);
				const cv::Mat& cvMat=gray.getCVMat();
				stockDetector->detect(cvMat,stockKeyPoints);
				customDetector->detect(gray,customKeyPoints);
				
				delete customDetector;
				delete customDetectorFactory;
				const size_t size1=stockKeyPoints.size();
				const size_t size2=customKeyPoints.size();

				std::vector<cv::KeyPoint> smaller=size1<size2?stockKeyPoints:customKeyPoints;
				std::vector<cv::KeyPoint> bigger=size1<size2?customKeyPoints:stockKeyPoints;;

				for(const cv::KeyPoint& kp1:smaller)
				{
					auto kpIter2=bigger.begin();
					bool foundMatch=false;
					for(; kpIter2!=bigger.end(); ++kpIter2)
					{
						if(kpEqual(kp1,*kpIter2,compareResponse,compareAngle,compareSize,compareOctave,compareClassId))
						{
							bigger.erase(kpIter2);
							foundMatch=true;
							break;
						}
					}
				}
				if(bigger.size()==0&&size1==size2)
				{
					MUT_COUT((++finishIndex)<<"/"<<totalNumTests<<" : "<<name<<" --> numKeyPoints="<<size1);
				}
				else
				{
					{
						std::lock_guard<std::mutex> lock(coutMut);
						std::cout<<(++finishIndex)<<"/"<<totalNumTests<<" : "<<name<<" --> no match size1="<<size1<<"; size2="<<size2<<"; bigger.size()="<<bigger.size()<<std::endl;
						printOutKeyPoints(stockKeyPoints,customKeyPoints,compareResponse,compareAngle,compareSize,compareOctave,compareClassId);
					}
					throw std::exception("keypoint mismatch");
				}
			}

			void testKeyPointsThread(const size_t cols,const size_t rows,const lyonste::matrix::Mat2D<uchar>& gray,std::queue<boost::property_tree::ptree>& params,std::mutex& mut,std::atomic<size_t>& finishIndex,size_t totalNumTests)
			{
				for(;;)
				{
					std::unique_lock<std::mutex> lock(mut);
					if(params.empty())
					{
						break;
					}
					const boost::property_tree::ptree properties=params.front();
					params.pop();
					lock.unlock();
					testKeyPoints(cols,rows,gray,properties,finishIndex,totalNumTests);
				}
			}

			void testKeyPoints(std::queue<boost::property_tree::ptree>& params)
			{
				cv::setNumThreads(0);
				cv::Mat frame=cv::imread("F:\\Downloads\\DAYvUl_XYAA2i87.jpg");
				size_t cols=size_t(frame.cols);
				size_t rows=size_t(frame.rows);
				matrix::Mat2D<uchar> gray(cols,rows);
				gray.overwrite(frame);
				std::mutex mut;
				std::atomic<size_t> finishIndex=0;
				size_t totalNumTests=params.size();
#ifdef NDEBUG
				
				std::vector<std::thread> threads;
				for(size_t i=0;i<11;++i)
				{
					threads.emplace_back(&testKeyPointsThread,cols,rows,std::ref(gray),std::ref(params),std::ref(mut),std::ref(finishIndex),totalNumTests);
				}
#endif
				testKeyPointsThread(cols,rows,gray,params,mut,finishIndex,totalNumTests);
#ifdef NDEBUG

				for(std::thread& t:threads)
				{
					t.join();
				}
#endif
			}

			namespace segmenttest
			{
				namespace fast
				{
					//const  KeyPointPropertyGenerator<Fast> propGen;
					template<size_t patternSize,KPRetentionPolicy retentionPolicy,lyonste::hal::HAL_FLAG hal>
					boost::property_tree::ptree generateProperties(uchar fastThreshold)
					{
						boost::property_tree::ptree properties=KeyPointPropertyGenerator<Fast>(0,fastThreshold,patternSize,retentionPolicy,Circle,true).getProperties();
						
						properties.put<lyonste::hal::HAL_FLAG>("hal",hal);
						properties.put<bool>("compareResponse",retentionPolicy!=RETAIN_ALL);
						properties.put<bool>("ignoreIncompatibleProperties",retentionPolicy==RETAIN_ALL);
						return properties;
					}
					
					void testFast()
					{
						std::queue<boost::property_tree::ptree> params;
						for(int i=0;i<=255;++i)
						{
							params.push(generateProperties<16,RETAIN_ALL,lyonste::hal::HAL_NONE>((uchar)i));
							params.push(generateProperties<16,RETAIN_ALL,lyonste::hal::HAL_SSE2>((uchar)i));
							params.push(generateProperties<16,NON_MAX_SUPPRESSION,lyonste::hal::HAL_NONE>((uchar)i));
							params.push(generateProperties<16,NON_MAX_SUPPRESSION,lyonste::hal::HAL_SSE2>((uchar)i));
							params.push(generateProperties<16,RETAIN_ALL_NO_SCORE,lyonste::hal::HAL_NONE>((uchar)i));
							params.push(generateProperties<16,RETAIN_ALL_NO_SCORE,lyonste::hal::HAL_SSE2>((uchar)i));
							params.push(generateProperties<12,RETAIN_ALL,lyonste::hal::HAL_NONE>((uchar)i));
							params.push(generateProperties<12,RETAIN_ALL,lyonste::hal::HAL_SSE2>((uchar)i));
							params.push(generateProperties<12,NON_MAX_SUPPRESSION,lyonste::hal::HAL_NONE>((uchar)i));
							params.push(generateProperties<12,NON_MAX_SUPPRESSION,lyonste::hal::HAL_SSE2>((uchar)i));
							params.push(generateProperties<12,RETAIN_ALL_NO_SCORE,lyonste::hal::HAL_NONE>((uchar)i));
							params.push(generateProperties<12,RETAIN_ALL_NO_SCORE,lyonste::hal::HAL_SSE2>((uchar)i));
							params.push(generateProperties<8,RETAIN_ALL,lyonste::hal::HAL_NONE>((uchar)i));
							params.push(generateProperties<8,RETAIN_ALL,lyonste::hal::HAL_SSE2>((uchar)i));
							params.push(generateProperties<8,NON_MAX_SUPPRESSION,lyonste::hal::HAL_NONE>((uchar)i));
							params.push(generateProperties<8,NON_MAX_SUPPRESSION,lyonste::hal::HAL_SSE2>((uchar)i));
							params.push(generateProperties<8,RETAIN_ALL_NO_SCORE,lyonste::hal::HAL_NONE>((uchar)i));
							params.push(generateProperties<8,RETAIN_ALL_NO_SCORE,lyonste::hal::HAL_SSE2>((uchar)i));
						}
						testKeyPoints(params);
					}
				}
			}
			namespace orb
			{
				////const  KeyPointPropertyGenerator<Orb> propGen;
				////template<size_t patternSize,KPRetentionPolicy retentionPolicy,lyonste::hal::HAL_FLAG hal>
				//template<lyonste::hal::HAL_FLAG hal,bool harrisScore>
				//boost::property_tree::ptree generateProperties(size_t numFeatures, size_t numLevels, size_t patchSize, float scaleFactor,size_t border, uchar fastThreshold)
				//{
				//	boost::property_tree::ptree layerProperties=KeyPointPropertyGenerator<Fast>(border,fastThreshold,16,NON_MAX_SUPPRESSION,segmenttest::Circle,true).getProperties();
				//	boost::property_tree::ptree properties=KeyPointPropertyGenerator<Orb>(layerProperties,numFeatures,numLevels,patchSize,scaleFactor,harrisScore,true),getProperties();
				//	//boost::property_tree::ptree properties=propGen(0,fastThreshold,patternSize,retentionPolicy,Circle,true);
				//	//properties.put<std::string>("testName","fast{threshold="+std::to_string((int)fastThreshold)+"; patternSize="+std::to_string(patternSize)+"; retentionPolicy="+std::to_string(retentionPolicy)+"; hal="+std::to_string(hal)+"}");
				//	//properties.put<lyonste::hal::HAL_FLAG>("hal",hal);
				//	//properties.put<bool>("compareResponse",retentionPolicy!=RETAIN_ALL);
				//	//properties.put<bool>("ignoreIncompatibleProperties",retentionPolicy==RETAIN_ALL);
				//	//return properties;
				//}
				//void testOrb()
				//{
				//	std::queue<boost::property_tree::ptree> params;

				//}
			}
		}
		

	}


}

boost::property_tree::ptree getKPGenProperties()
{
	boost::property_tree::ptree fastProperties;
	fastProperties.put<std::string>("keyPointType",lyonste::feature2D::featureDetection::kpTypeStr<lyonste::feature2D::featureDetection::Fast>);
	fastProperties.put<size_t>("patternSize",16);
	fastProperties.put<std::string>("shape",lyonste::feature2D::featureDetection::segmenttest::patternShapeStr<lyonste::feature2D::featureDetection::segmenttest::Circle>);
	fastProperties.put<std::string>("retentionPolicy",lyonste::feature2D::featureDetection::kpRetentionPolicyStr<lyonste::feature2D::featureDetection::NON_MAX_SUPPRESSION>);//std::to_string(lyonste::feature2D::featureDetection::NON_MAX_SUPPRESSION));
	fastProperties.put<bool>("compatibilityMode",false);
	fastProperties.put<size_t>("border",4);
	fastProperties.put<uchar>("fastThreshold",20);

	boost::property_tree::ptree motionDetectingProperties;
	motionDetectingProperties.add_child("kpGenFactoryProperties",fastProperties);
	motionDetectingProperties.put<std::string>("keyPointType",lyonste::feature2D::featureDetection::kpTypeStr<lyonste::feature2D::featureDetection::MotionDetecting>);
	motionDetectingProperties.put<size_t>("historyLength",5);
	motionDetectingProperties.put<float>("weightFactor",1000.f);
	motionDetectingProperties.put<uchar>("diffThreshold",75);

	boost::property_tree::ptree orbProperties;
	orbProperties.add_child("layerFactoryProperties",motionDetectingProperties);
	orbProperties.put<std::string>("keyPointType",lyonste::feature2D::featureDetection::kpTypeStr<lyonste::feature2D::featureDetection::Orb>);
	orbProperties.put<bool>("harrisScore",true);
	orbProperties.put<bool>("calculateAngles",false);
	orbProperties.put<size_t>("numFeatures",1000);
	orbProperties.put<size_t>("numLevels",16);
	orbProperties.put<size_t>("patchSize",31);
	orbProperties.put<float>("scaleFactor",1.2f);

	return orbProperties;
}

boost::property_tree::ptree getDescGenProperties()
{
	return lyonste::feature2D::featureDescription::DescriptorPropertyGeneratorImpl<lyonste::feature2D::featureDescription::Freak>()(11.f,8,false,true);
}

boost::property_tree::ptree getSigGenProperties()
{
	const boost::property_tree::ptree& kpGenProperties=getKPGenProperties();
	const boost::property_tree::ptree& descGenProperties=getDescGenProperties();
	boost::property_tree::ptree sigGenProperties;
	sigGenProperties.add_child("keyPointGeneratorProperties",kpGenProperties);
	sigGenProperties.add_child("descriptorGeneratorProperities",descGenProperties);
	sigGenProperties.put<double>("keyFrameInterval",2000.0);
	sigGenProperties.put<std::string>("root","F:\\sigs2");
	sigGenProperties.put<size_t>("numThreads",cv::getNumberOfCPUs());
	sigGenProperties.put<bool>("verbose",true);
	sigGenProperties.put<size_t>("reportIntervalMillis",5000);
	sigGenProperties.add<std::string>("includePattern","(.*)\\.sig");
	sigGenProperties.put<bool>("followSymlink",false);
	sigGenProperties.put<size_t>("maxDepth",std::numeric_limits<size_t>::max());
	return sigGenProperties;
}

boost::property_tree::ptree getSigMatchProperties()
{
	boost::property_tree::ptree matchProperties;
	matchProperties.put<double>("keyFrameIntervalRatio",.01);
	matchProperties.put<double>("skipRatio",.05);
	matchProperties.put<std::string>("sigMatchFunction","drifted");
	matchProperties.put<double>("maxDriftRatio",.0025);
	matchProperties.put<double>("driftRatioThreshold",.3);
	matchProperties.put<size_t>("numThreads",cv::getNumberOfCPUs());
	matchProperties.put<std::string>("matchFile","F:\\matches.match");
	matchProperties.put<size_t>("writeIntervalMillis",5000);
	matchProperties.put<size_t>("maxNumMatches",100);
	matchProperties.put<size_t>("maxNumMatchesToMove", 100);
	matchProperties.put<double>("maxSimilarity",1.0);
	matchProperties.put<std::string>("whiteListFile","F:\\whiteList2.whiteList");
	matchProperties.put<std::string>("blackListFile","F:\\blackList2.blackList");
	matchProperties.put<std::string>("potentialDupDir","F:\\PotentialDuplicates");
	matchProperties.put<std::string>("blackListDir","F:\\Blacklist");
	matchProperties.put<double>("minDuration",0);
	matchProperties.put<double>("maxDuration",std::numeric_limits<double>::infinity());
	matchProperties.put<double>("maxAbsoluteDurationDiff",std::numeric_limits<double>::infinity());
	matchProperties.put<double>("maxDurationRatio",.005);
	matchProperties.put<size_t>("minFileSize", 0);
	matchProperties.put<size_t>("maxFileSize", std::numeric_limits<size_t>::max());
	matchProperties.put<size_t>("maxAbsoluteFileSizeDiff", std::numeric_limits<size_t>::max());
	matchProperties.put<double>("maxFileSizeRatio", std::numeric_limits<double>::infinity());
	matchProperties.put<bool>("bruteForce",true);
	matchProperties.put<std::string>("matchMethod","knn2Ratio");
	matchProperties.put<std::string>("normType","hamming");
	matchProperties.put<std::string>("matchCandidatePredicateType", "default");
	return matchProperties;
}

boost::property_tree::ptree getCacheProperties()
{
	boost::property_tree::ptree cacheProperties;
	cacheProperties.put<std::string>("descriptorType","freak");
	cacheProperties.put<size_t>("maxMemoryFootprint",8589934592);
	cacheProperties.add_child("sigGenProperties",getSigGenProperties());
	cacheProperties.add_child("sigMatchProperties",getSigMatchProperties());
	cacheProperties.put<std::string>("cachePath","F:\\sigs2\\videosigscache.cache");
	cacheProperties.put<size_t>("writeIntervalMillis",0);
	return cacheProperties;
}

boost::property_tree::ptree getVideoGatheringProperties()
{
	boost::property_tree::ptree videoGatheringProperties;
	videoGatheringProperties.put<std::string>("root","F:\\uncategorized");
	videoGatheringProperties.put<size_t>("maxDepth",std::numeric_limits<size_t>::max());
	videoGatheringProperties.put<bool>("followSymlink",false);
	videoGatheringProperties.add<std::string>("includePattern","(.*)\\.(mp4|m4v|mkv|avi|divx|flv|mov|mpg|wmv)");
	videoGatheringProperties.add<std::string>("excludePattern","F:\\\\uncategorized\\\\other(.*)");
	return videoGatheringProperties;
}

boost::property_tree::ptree getProperties()
{
	std::ifstream inStream("F:\\duplicateVideoSearchConfig.xml");
	boost::property_tree::ptree masterProperties;
	boost::property_tree::read_xml(inStream,masterProperties);
	return masterProperties;
}

boost::property_tree::ptree writeProperties()
{
	boost::property_tree::ptree masterProperties;
	masterProperties.add_child("cacheProperties",getCacheProperties());
	masterProperties.add_child("videoGatheringProperties",getVideoGatheringProperties());
	std::ofstream outStream("F:\\duplicateVideoSearchConfig2.xml");
	boost::property_tree::write_xml(outStream,masterProperties);
	outStream.close();
	return masterProperties;
}

struct SortFilesBySize
{
	constexpr bool operator()(const lyonste::fileManagement::FileInfo& file1,const lyonste::fileManagement::FileInfo& file2) const
	{
		return file1.getFileSize()<file2.getFileSize();
	}
};

using FileInfo=lyonste::fileManagement::FileInfo;
using FileSet=std::set<FileInfo>;
using DirectoryMap=std::unordered_map<FileInfo,FileSet>;

DirectoryMap getDirectoryMap(const boost::filesystem::path& rootDir)
{
	lyonste::fileManagement::FileGatherer fileGatherer(rootDir,std::numeric_limits<size_t>::max(),false,std::vector<boost::regex>{boost::regex("(.*)\\.(.*)")},std::vector<boost::regex>{});
	FileSet files;
	fileGatherer.gatherFiles(std::inserter(files,files.begin()));
	DirectoryMap directoryMap;
	for(const auto& file:files)
	{
		auto itr=directoryMap.find(file.getPath().parent_path());
		if(itr==directoryMap.end())
		{
			directoryMap.emplace(file.getPath().parent_path(),FileSet{file});
		}
		else
		{
			itr->second.emplace(file);
		}
	}
	return directoryMap;
}

void updateFileMaps()
{
	boost::property_tree::ptree& masterProperties=getProperties();
	std::cout << "Updating black and white lists" << std::endl;
	boost::filesystem::path whiteListFile=boost::filesystem::path(masterProperties.get<std::string>("cacheProperties.sigMatchProperties.whiteListFile"));
	boost::filesystem::path blackListFile=boost::filesystem::path(masterProperties.get<std::string>("cacheProperties.sigMatchProperties.blackListFile"));
	boost::filesystem::path whiteListDir=boost::filesystem::path(masterProperties.get<std::string>("cacheProperties.sigMatchProperties.potentialDupDir"));
	boost::filesystem::path blackListDir=boost::filesystem::path(masterProperties.get<std::string>("cacheProperties.sigMatchProperties.blackListDir"));
	if(!boost::filesystem::exists(blackListDir))
	{
		CV_Error(1,"blackListDir "+blackListDir.string()+" does not exist");
	}
	if(!boost::filesystem::exists(whiteListDir))
	{
		CV_Error(1,"whiteListDir "+whiteListDir.string()+" does not exist");
	}
	lyonste::DisjointSet<FileInfo> djSet;
	djSet.readFromFile(whiteListFile);
	DirectoryMap& whiteListDirMap=getDirectoryMap(whiteListDir);
	size_t numAddedWhiteList = 0;
	for(const auto& directory:whiteListDirMap)
	{
		const FileSet& files=directory.second;
		auto itr=files.cbegin();
		auto end=files.cend();
		if(itr!=end)
		{
			const FileInfo& representative=*itr;
			for(;++itr!=end;)
			{
				const FileInfo& file = *itr;
				if (!djSet.areJoined(representative, file))
				{
					++numAddedWhiteList;
					djSet.join(representative, file);
				}
			}
		}
	}
	djSet.writeToFile(whiteListFile);
	const std::unordered_map<FileInfo,std::unordered_set<FileInfo>> mapView=djSet.getMapView();
	lyonste::MutualMap<FileInfo> mutualMap;
	mutualMap.readFromFile(blackListFile);
	DirectoryMap& blackListDirMap=getDirectoryMap(blackListDir);
	size_t numAddedBlackList = 0;
	for(const auto& directory:blackListDirMap)
	{
		const FileSet& files=directory.second;
		auto end=files.cend();
		for(auto itr1=files.cbegin();itr1!=end;++itr1)
		{
			const FileInfo& file1=*itr1;
			for(auto itr2=files.cbegin();itr2!=end;++itr2)
			{
				if(itr1!=itr2)
				{
					const FileInfo& file2=*itr2;
					const auto file1SetItr=mapView.find(file1);
					const auto file2SetItr=mapView.find(file2);
					if(file1SetItr!=mapView.end())
					{
						if(file2SetItr!=mapView.end())
						{
							for(const auto& set1Val:file1SetItr->second)
							{
								for(const auto& set2Val:file2SetItr->second)
								{
									if (mutualMap.add(set1Val, set2Val))
									{
										++numAddedBlackList;
									}
								}
							}
						}
						else
						{
							for(const auto set1Val:file1SetItr->second)
							{
								if (mutualMap.add(set1Val, file2))
								{
									++numAddedBlackList;

								}
							}
						}
					}
					else
					{
						if(file2SetItr!=mapView.end())
						{
							for(const auto set2Val:file2SetItr->second)
							{
								if (mutualMap.add(file1, set2Val))
								{
									++numAddedBlackList;
								}
							}
						}
						else
						{
							if (mutualMap.add(file1, file2))
							{
								++numAddedBlackList;
							}
						}
					}
				}
			}
		}
	}
	mutualMap.writeToFile(blackListFile);
	std::cout << "Added " << numAddedWhiteList << " matches to the whitelist" << std::endl;
	std::cout << "Added " << numAddedBlackList << " matches to the blacklist" << std::endl;
}

FileSet scanForVideoFiles(const  boost::property_tree::ptree& masterProperties)
{
	lyonste::fileManagement::FileGatherer vidFileGatherer(masterProperties.get_child("videoGatheringProperties"));
	FileSet vidFiles;
	std::cout << "Scanning for video files" << std::endl;
	vidFileGatherer.gatherFiles(std::inserter(vidFiles, vidFiles.end()));
	std::cout << "Discovered " << vidFiles.size() << " video files" << std::endl;
	return vidFiles;
}

lyonste::MutualMap<lyonste::fileManagement::FileInfo> readBlackList(const  boost::property_tree::ptree& masterProperties)
{
	lyonste::MutualMap<lyonste::fileManagement::FileInfo> blackList;
	const boost::optional<std::string>& optionalBlackListPath = masterProperties.get_optional<std::string>("cacheProperties.sigMatchProperties.blackListFile");
	if (optionalBlackListPath.is_initialized())
	{
		boost::filesystem::path blackListFilePath = boost::filesystem::path(optionalBlackListPath.get());
		if (boost::filesystem::exists(blackListFilePath))
		{
			std::cout << "Reading in blacklisted matches" << std::endl;
			blackList.readFromFile(blackListFilePath);
			std::cout << "Found " << blackList.size() << " blacklisted matches" << std::endl;
		}
	}
	return blackList;
}

std::unique_ptr<lyonste::videoSig::VideoSigCache> getVideoSigCache(const  boost::property_tree::ptree& masterProperties)
{
	return std::unique_ptr<lyonste::videoSig::VideoSigCache>(lyonste::videoSig::getVideoSigCache(masterProperties.get_child("cacheProperties")));
}

std::unique_ptr<lyonste::videoSig::videoSigMatch::SigMatchSessionImpl> readMatchSessionFromFile(const boost::property_tree::ptree& masterProperties, lyonste::MutualMap<lyonste::fileManagement::FileInfo>& blackList)
{
	FileSet vidFiles = scanForVideoFiles(masterProperties);
	std::unique_ptr<lyonste::videoSig::VideoSigCache> cache = getVideoSigCache(masterProperties);
	std::cout << "Pruning video signature cache. Before size=" << cache->size() << std::endl;
	cache->pruneCache(vidFiles);
	std::cout << "Finished pruning video signature cache. After size=" << cache->size() << std::endl;
	std::cout << "Writing videos signature cache to file" << std::endl;
	cache->writeToFile();
	std::unique_ptr<lyonste::videoSig::videoSigMatch::SigMatchSessionImpl> sigMatchSession(lyonste::videoSig::videoSigMatch::getSigMatchSession(cache.get(), masterProperties.get_child("cacheProperties.sigMatchProperties"), false));
	std::cout << "Reading in matches from file" << std::endl;
	sigMatchSession->readFromFile();
	std::cout << "Finished reading from file. Number of matches = " << sigMatchSession->getNumMatches() << std::endl;
	return sigMatchSession;
}

void printMatchGroups()
{
	const boost::property_tree::ptree& masterProperties = getProperties();
	lyonste::MutualMap<lyonste::fileManagement::FileInfo> blackList = readBlackList(masterProperties);
	std::unique_ptr<lyonste::videoSig::videoSigMatch::SigMatchSessionImpl> sigMatchSession= readMatchSessionFromFile(masterProperties,blackList);
	std::cout << "Partitioning matches into groups" << std::endl;
	auto& sortedMatchGroups=sigMatchSession->getSortedMatchGroups(std::numeric_limits<size_t>::max(), blackList);
	std::cout << "Sorting match groups by size" << std::endl;
	std::sort(sortedMatchGroups.begin(), sortedMatchGroups.end(), [](const auto& val1, const auto& val2) {return val1.size() > val2.size(); });
	size_t groupCounter = 0;
	for (const auto& matchGroup : sortedMatchGroups)
	{
		std::cout << "Match group #" << (++groupCounter) <<" -- Size="<<matchGroup.size()<<std::endl;
		for (const auto& vid : matchGroup)
		{
			std::cout <<'\t'<< vid.getVidFile().getFilePathStr() << std::endl;
		}
		std::cout << std::endl;
	}
	std::cout << "Total number of matches = " << sigMatchSession->getNumMatches() << std::endl;
}


void moveMatches()
{
	const boost::property_tree::ptree& masterProperties = getProperties();
	lyonste::MutualMap<lyonste::fileManagement::FileInfo> blackList = readBlackList(masterProperties);
	std::unique_ptr<lyonste::videoSig::videoSigMatch::SigMatchSessionImpl> sigMatchSession = readMatchSessionFromFile(masterProperties, blackList);
	std::cout << "Moving matches" << std::endl;
	sigMatchSession->moveMatches(boost::filesystem::path(masterProperties.get<std::string>("cacheProperties.sigMatchProperties.potentialDupDir"))
		, masterProperties.get<size_t>("cacheProperties.sigMatchProperties.maxNumMatchesToMove", masterProperties.get<size_t>("cacheProperties.sigMatchProperties.maxNumMatches"))
		, masterProperties.get<size_t>("cacheProperties.sigMatchProperties.maxMatchMoveGroupSize", std::numeric_limits<size_t>::max())
		,blackList);
}

//void repairBlackAndWhiteLists()
//{
//	boost::property_tree::ptree& masterProperties=getProperties();
//	boost::property_tree::ptree& cacheProperties=masterProperties.get_child("cacheProperties");
//	boost::property_tree::ptree& sigMatchProperties=cacheProperties.get_child("sigMatchProperties");
//	boost::filesystem::path whiteListFile=boost::filesystem::path(sigMatchProperties.get<std::string>("whiteListFile"));
//	boost::filesystem::path blackListFile=boost::filesystem::path(sigMatchProperties.get<std::string>("blackListFile"));
//	boost::filesystem::path whiteListDir=boost::filesystem::path(sigMatchProperties.get<std::string>("potentialDupDir"));
//	boost::filesystem::path blackListDir=boost::filesystem::path(sigMatchProperties.get<std::string>("blackListDir"));
//	lyonste::MutualMap<FileInfo> blackList;
//	blackList.readFromFile(blackListFile);
//	lyonste::DisjointSet<FileInfo> whiteList;
//	whiteList.readFromFile(whiteListFile);
//	std::unordered_set<FileInfo> whiteListToRemove;
//
//	for(auto itr1=blackList.begin();itr1!=blackList.end();++itr1)
//	{
//		const FileInfo& blFile1=*(itr1->first);
//		auto& blFileSet=itr1->second;
//		
//		for(auto itr2=blFileSet.begin();itr2!=blFileSet.end();++itr2)
//		{
//			const FileInfo& blFile2=**itr2;
//			if(whiteList.areJoined(blFile1,blFile2))
//			{
//				std::cout<<"removing "<<blFile1.getFilePathStr()<<" and "<<blFile2.getFilePathStr()<<std::endl;
//				whiteListToRemove.emplace(blFile1);
//				whiteListToRemove.emplace(blFile2);
//			}
//		}
//	}
//	//for (auto& pair : blackListToRemove)
//	//{
//	//	//blackList.remove(pair.first, pair.second);
//	//}
//	for (auto& file : whiteListToRemove)
//	{
//		whiteList.remove(file);
//	}
//	//blackList.writeToFile(blackListFile);
//	whiteList.writeToFile(whiteListFile);
//}

void generateSigs(FileSet& vidFiles ,const boost::property_tree::ptree& masterProperties, lyonste::videoSig::VideoSigCache* cache)
{
	std::unique_ptr<lyonste::videoSig::videoSigGen::SigGenSessionImpl<FileSet>> sigGenSession(lyonste::videoSig::videoSigGen::getSigGenSession<FileSet>(vidFiles, cache, masterProperties.get_child("cacheProperties.sigGenProperties")));
	std::cout << "Generating signatures for " << vidFiles.size() << " videos" << std::endl;
	sigGenSession->generateSigs();
	std::cout << "Finished generating signatures. Total number of sigs in cache = " << sigGenSession->getNumSigsInCache() << std::endl;
	
}

void getMatches(const boost::property_tree::ptree& masterProperties, lyonste::videoSig::VideoSigCache* cache,bool incremental=false)
{
	std::unique_ptr<lyonste::videoSig::videoSigMatch::SigMatchSessionImpl> sigMatchSession(lyonste::videoSig::videoSigMatch::getSigMatchSession(cache, masterProperties.get_child("cacheProperties.sigMatchProperties"),incremental));
	std::cout << "Generating matches for " << sigMatchSession->getNumCandidates() <<" candidates."<< std::endl;
	sigMatchSession->generateMatches();
	std::cout << "Finished generating matches. Total number of matches = " << sigMatchSession->getNumMatches() << std::endl;
}




void mainDupDetectionRoutine(bool incremental)
{
	boost::property_tree::ptree& masterProperties=getProperties();
	std::unique_ptr<lyonste::videoSig::VideoSigCache> cache(lyonste::videoSig::getVideoSigCache(masterProperties.get_child("cacheProperties")));
	FileSet vidFiles = scanForVideoFiles(masterProperties);
	cache->pruneCache(vidFiles);
	cache->writeToFile();
	generateSigs(vidFiles,masterProperties, cache.get());
	getMatches(masterProperties, cache.get(), incremental);
}


void playVideo()
{
	for (;;)
	{
		std::cout << "Enter full video path, or type exit to go back.";
		std::string videoPathStr;
		std::getline(std::cin, videoPathStr);
		lyonste::trim(videoPathStr);
		if (boost::algorithm::to_lower_copy(videoPathStr) == "exit")
		{
			break;
		}
		boost::filesystem::path videoPath(videoPathStr);
		if (!boost::filesystem::exists(videoPath))
		{
			std::cout << "The path " << videoPathStr << " does not exist." << std::endl;
			continue;
		}

		try
		{
			cv::VideoCapture cap(videoPathStr);
			if (!cap.isOpened())
			{
				std::cerr << "Unable to open video." << std::endl;
				continue;
			}
			cv::namedWindow("Video");
			cv::Mat capMat;
			double fps = cap.get(cv::CAP_PROP_FPS);
			if (fps <= 0 || fps != fps || fps>60)
			{
				fps = 30;
				
			}
			double interval = 1000.0 / fps;
			double nextPos = interval;
			for (;;)
			{
				if (!cap.grab())
				{
					break;
				}
				double currPos = cap.get(cv::CAP_PROP_POS_MSEC);
				if (currPos >= nextPos)
				{
					nextPos += interval;
					if (cap.retrieve(capMat))
					{
						cv::imshow("Video", capMat);
					}
					else
					{
						std::cerr << "Failed to decode frame at time " << currPos << std::endl;
					}
					if (cv::waitKey((int)interval))
					{
						break;
					}
				}
			}
			cv::destroyWindow("Video");
			return;
		}
		catch (...)
		{
			std::cerr << "Exception while reading video" << std::endl;
			std::cerr << boost::current_exception_diagnostic_information() << std::endl;
			cv::destroyWindow("Video");
		}
	}
}

int main(int argc,char* argv[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF);
	
#ifdef _WIN32

	SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
#endif


	for(;;)
	{
		std::cout<<"   Enter command option"<<std::endl;
		std::cout<<"1. Exit."<<std::endl;
		std::cout<<"2. Run video duplication detection routine."<<std::endl;
		std::cout<<"3. Run incremental video duplication detection routine." << std::endl;
		std::cout<<"4. Move matches."<<std::endl;
		std::cout<<"5. Update black and white lists."<<std::endl;
		//std::cout<<"6. Repair black and white lists."<<std::endl;
		std::cout<<"6. Write default properties."<<std::endl;
		std::cout<<"7. Print match groups."<<std::endl;
		//std::cout << "8. Play video." << std::endl;
		
		int option;
		std::cin>>option;
		switch(option)
		{
			case 1:
				return 0;
			case 2:
				mainDupDetectionRoutine(false);
				continue;
			case 3:
				mainDupDetectionRoutine(true);
				continue;
			case 4:
				moveMatches();
				continue;
			case 5:
				updateFileMaps();
				continue;
			//case 6:
				//repairBlackAndWhiteLists();
				//continue;
			case 6:
				writeProperties();
				continue;
			case 7:
				printMatchGroups();
				continue;
			//case 8:
			//	playVideo();
			//	continue;
			default:
				std::cout<<"Unknown command option"<<std::endl;
		}
	}
	
	

}



#ifdef _DEBUG
#ifdef DEBUG_NEW
#undef DEBUG_NEW
#endif
#ifdef new
#undef new
#endif
#endif
