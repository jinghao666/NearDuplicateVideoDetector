// NearVideoDuplicateDetector.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <iostream>
#include <opencv2/imgproc.hpp>
#include <list>
//#include "FeatureDetection.h"
#include "opencv2/xfeatures2d.hpp"
//#include "VideoSignature.h"
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
#ifdef NEW_IMPLEMENTATION
template<bool compareResponse=true,bool compareAngle=true,bool compareSize=true,bool compareOctave=true,bool compareClassId=true>
void testKeyPoints(const size_t cols,const size_t rows,const lyonste::matrix::Mat2D<uchar>& gray,lyonste::featureDetection::FeatureDetectorFactory* customDetectorFactory,const std::string name="no-name")
{

	//std::vector<cv::KeyPoint> stockKeypoints;
	std::vector<cv::KeyPoint> customKeypoints;
	//stockDetector->detect(frame, stockKeypoints);
	lyonste::featureDetection::FeatureDetector* customDetector=customDetectorFactory->getFeatureDetector(cols,rows);
	customDetector->detect(gray,customKeypoints);
	delete customDetector;
	//const size_t stockSize = stockKeypoints.size();
	const size_t customSize=customKeypoints.size();
	std::vector<cv::KeyPoint>& smaller=customSize < customSize?customKeypoints:customKeypoints;
	std::vector<cv::KeyPoint>& bigger=customSize < customSize?customKeypoints:customKeypoints;
	for(const cv::KeyPoint& kp1:smaller)
	{
		auto kpIter2=bigger.begin();
		bool foundMatch=false;
		for(; kpIter2!=bigger.end(); ++kpIter2)
		{
			if(kpEqual<compareResponse,compareAngle,compareSize,compareOctave,compareClassId>(kp1,*kpIter2))
			{
				bigger.erase(kpIter2);
				foundMatch=true;
				break;
			}
		}
	}
	if(bigger.size()==0&&customSize==customSize)
	{
		MUT_COUT(name<<" -->    match numKeyPoints="<<customSize);
	}
	else
	{
		{
			std::lock_guard<std::mutex> lock(coutMut);
			std::cout<<name<<" --> no match stockSize="<<customSize<<"; customSize="<<customSize<<"; bigger.size()="<<bigger.size()<<std::endl;
		}
		//MUT_COUT(name << " --> no match stockSize=" << stockSize << "; customSize=" << customSize << "; bigger.size()=" << bigger.size());
		throw std::exception("keypoint mismatch");
	}
}

template<bool compareResponse=true,bool compareAngle=true,bool compareSize=true,bool compareOctave=true,bool compareClassId=true>
void testKeyPoints(const cv::Mat frame,const size_t cols,const size_t rows,const lyonste::matrix::Mat2D<uchar>& gray,cv::Ptr<cv::Feature2D>& stockDetector,lyonste::featureDetection::FeatureDetectorFactory* customDetectorFactory,const std::string name="no-name")
{

	std::vector<cv::KeyPoint> stockKeypoints;
	std::vector<cv::KeyPoint> customKeypoints;
	stockDetector->detect(frame,stockKeypoints);
	lyonste::featureDetection::FeatureDetector* customDetector=customDetectorFactory->getFeatureDetector(cols,rows);
	customDetector->detect(gray,customKeypoints);
	delete customDetector;
	const size_t stockSize=stockKeypoints.size();
	const size_t customSize=customKeypoints.size();
	std::vector<cv::KeyPoint>& smaller=stockSize < customSize?stockKeypoints:customKeypoints;
	std::vector<cv::KeyPoint>& bigger=stockSize < customSize?customKeypoints:stockKeypoints;
	for(const cv::KeyPoint& kp1:smaller)
	{
		auto kpIter2=bigger.begin();
		bool foundMatch=false;
		for(; kpIter2!=bigger.end(); ++kpIter2)
		{
			if(kpEqual<compareResponse,compareAngle,compareSize,compareOctave,compareClassId>(kp1,*kpIter2))
			{
				bigger.erase(kpIter2);
				foundMatch=true;
				break;
			}
		}
	}
	if(bigger.size()==0&&stockSize==customSize)
	{
		MUT_COUT(name<<" -->    match numKeyPoints="<<stockSize);
	}
	else
	{
		{
			std::lock_guard<std::mutex> lock(coutMut);
			std::cout<<name<<" --> no match stockSize="<<stockSize<<"; customSize="<<customSize<<"; bigger.size()="<<bigger.size()<<std::endl;
		}
		//MUT_COUT(name << " --> no match stockSize=" << stockSize << "; customSize=" << customSize << "; bigger.size()=" << bigger.size());
		throw std::exception("keypoint mismatch");
	}
}





template<bool harrisScore,bool compareResponse=true,bool compareAngles=true,bool compareSize=true,bool compareOctave=true,bool compareClassId=true>
void testORB(const cv::Mat frame,const size_t cols,const size_t rows,const lyonste::matrix::Mat2D<uchar>& gray,const size_t numLevels,const uchar fastThreshold,const size_t numFeatures,const size_t patchSize,const size_t edgeThreshold,const float scaleFactor)
{
	using namespace lyonste;
	using namespace lyonste::featureDetection;
	using namespace lyonste::featureDetection::orb;
	using namespace lyonste::featureDetection::fast;
	std::string name="{numLevels="+std::to_string(numLevels)+
		"; fastThreshold="+std::to_string(fastThreshold)+
		"; numFeatures="+std::to_string(numFeatures)+
		"; patchSize="+std::to_string(patchSize)+
		"; edgeThreshold="+std::to_string(edgeThreshold)+
		"; scaleFactor="+std::to_string(scaleFactor)+
		"; harrisScore="+(harrisScore?"true  }":"false }");
	if(false)
	{
		FeatureDetectorFactoryImpl < FastFeatureDetector<PATTERN_SIZE_16,NON_MAX_SUPPRESSION_FAST,HAL_SSE2,true>>* fastFactory=new FeatureDetectorFactoryImpl<FastFeatureDetector<PATTERN_SIZE_16,NON_MAX_SUPPRESSION_FAST,HAL_SSE2,true>>(edgeThreshold,fastThreshold);
		FeatureDetectorFactoryImpl<OrbFeatureDetector<FastFeatureDetector<PATTERN_SIZE_16,NON_MAX_SUPPRESSION_FAST,HAL_SSE2,true>,harrisScore,compareAngles>>* orbFactory=new FeatureDetectorFactoryImpl<OrbFeatureDetector<FastFeatureDetector<PATTERN_SIZE_16,NON_MAX_SUPPRESSION_FAST,HAL_SSE2,true>,harrisScore,compareAngles>>(fastFactory,numFeatures,numLevels,patchSize,scaleFactor);
		try
		{
			testKeyPoints<compareResponse,compareAngles,compareSize,compareOctave,compareClassId>(cols,rows,gray,orbFactory,name);
			delete orbFactory;
		}
		catch(...)
		{
			{
				std::lock_guard<std::mutex> lock(coutMut);
				std::cout<<"Critical failure on "<<name<<std::endl;
			}
			delete orbFactory;
			throw std::exception("critical failure");
		}
	}
	else
	{
		cv::Ptr<cv::Feature2D> stockDetector=cv::ORB::create(int(numFeatures),scaleFactor,int(numLevels),int(edgeThreshold),int(0),int(2),int(harrisScore?cv::ORB::HARRIS_SCORE:cv::ORB::FAST_SCORE),int(patchSize),int(fastThreshold));
		FeatureDetectorFactoryImpl < FastFeatureDetector<PATTERN_SIZE_16,NON_MAX_SUPPRESSION_FAST,HAL_SSE2,true>>* fastFactory=new FeatureDetectorFactoryImpl<FastFeatureDetector<PATTERN_SIZE_16,NON_MAX_SUPPRESSION_FAST,HAL_SSE2,true>>(edgeThreshold,fastThreshold);
		FeatureDetectorFactoryImpl<OrbFeatureDetector<FastFeatureDetector<PATTERN_SIZE_16,NON_MAX_SUPPRESSION_FAST,HAL_SSE2,true>,harrisScore,compareAngles>>* orbFactory=new FeatureDetectorFactoryImpl<OrbFeatureDetector<FastFeatureDetector<PATTERN_SIZE_16,NON_MAX_SUPPRESSION_FAST,HAL_SSE2,true>,harrisScore,compareAngles>>(fastFactory,numFeatures,numLevels,patchSize,scaleFactor);
		try
		{
			testKeyPoints<compareResponse,compareAngles,compareSize,compareOctave,compareClassId>(frame,cols,rows,gray,stockDetector,orbFactory,name);
			delete orbFactory;
			stockDetector.release();
		}
		catch(...)
		{
			{
				std::lock_guard<std::mutex> lock(coutMut);
				std::cout<<"Critical failure on "<<name<<std::endl;
			}
			delete orbFactory;
			stockDetector.release();
			throw std::exception("critical failure");
		}
	}


}

void testORB(const float scaleFactor)
{
	cv::Mat frame=cv::imread("F:\\Downloads\\DAYvUl_XYAA2i87.jpg");
	size_t cols=size_t(frame.cols);
	size_t rows=size_t(frame.rows);
	lyonste::matrix::Mat2D<uchar> gray(cols,rows);
	gray.overwrite(frame);
	using namespace lyonste::featureDetection;
	//const float scaleFactors[]= { 1.1,1.2,1.3,1.4,1.5,1.6,1.7,1.8,1.9 };
	ushort fastThreshold=0;
	//for (ushort fastThreshold = 256; fastThreshold--;)
	{
		size_t numLevels=12;
		//for (size_t numLevels = 13; numLevels--;)
		{

			size_t numFeatures=500000;
			//for (; numFeatures >= 1; --numFeatures)
			{
				//size_t patchSize = 2;
				size_t maxPatchSize;
				if(fastThreshold==0)
				{
					maxPatchSize=102;
				}
				else
				{
					maxPatchSize=2;
				}
				for(size_t patchSize=maxPatchSize; patchSize>=2; --patchSize)
				{
					//{
					//	std::lock_guard<std::mutex> lock(coutMut);
					//	std::cout << "scaleFactor=" << scaleFactor << "; numLevels= " << numLevels << "; fastThreshold=" << fastThreshold << "; patchSize="<<patchSize<<std::endl;
					//}
					size_t edgeMin=std::max(cvCeil((patchSize/2)*sqrt(2.0)),4)+1;;
					size_t edgeMax;
					if(fastThreshold==0)
					{

						edgeMax=edgeMin+100;
					}
					else
					{
						edgeMax=edgeMin;
					}
					//size_t max = min;
					for(size_t edgeThreshold=edgeMin; edgeThreshold<=edgeMax; ++edgeThreshold)
					{
						//std::thread(testORB, numLevels, fastThreshold, numFeatures, patchSize, edgeThreshold, 1.1);
						//for (size_t scaleFactorIdx = 0; scaleFactorIdx < 9; ++scaleFactorIdx)
						{
							//const float scaleFactor = scaleFactors[scaleFactorIdx];
							if(fastThreshold==0)
							{
								testORB<true,true,true,true,true,true>(frame,cols,rows,gray,numLevels,uchar(fastThreshold),numFeatures,patchSize,edgeThreshold,scaleFactor);
								testORB<false,true,true,true,true,true>(frame,cols,rows,gray,numLevels,uchar(fastThreshold),numFeatures,patchSize,edgeThreshold,scaleFactor);
							}
							else
							{
								testORB<true,true,false,false,false,false>(frame,cols,rows,gray,numLevels,uchar(fastThreshold),numFeatures,patchSize,edgeThreshold,scaleFactor);
								testORB<false,true,false,false,false,false>(frame,cols,rows,gray,numLevels,uchar(fastThreshold),numFeatures,patchSize,edgeThreshold,scaleFactor);
							}

						}
					}
				}
			}

		}
	}
}

bool compareImage(const cv::Mat& customFrame,const cv::Mat& standardFrame,bool displayImage=true,int windowFlags=cv::WINDOW_AUTOSIZE)
{

	int customCols=customFrame.cols;
	int customRows=customFrame.rows;
	int standardCols=standardFrame.cols;
	int standardRows=standardFrame.rows;
	if(customCols!=standardCols||customRows!=standardRows)
	{
		std::cout<<"customCols="<<customCols<<"; customRows="<<customRows<<"; standardCols="<<standardCols<<"; standardRows="<<standardRows<<std::endl;
		return false;
	}
	size_t area=customCols*customRows;
	const uchar* customData=customFrame.data;
	const uchar* standardData=standardFrame.data;
	for(size_t i=0; i < area; ++i)
	{
		if(customData[i]!=standardData[i])
		{
			std::cout<<"pixel mismatch at index "<<i<<"; for {x="<<customCols<<", y="<<customRows<<"}; custom val="<<customData[i]<<"; standard val="<<standardData[i]<<std::endl;
			return false;
		}
	}
	std::cout<<"match for {x="<<customCols<<", y="<<customRows<<"}"<<std::endl;
	return true;
}

void testResizer(const float scaleFactor,const size_t numLevels)
{
	cv::Mat frame=cv::imread("F:\\Downloads\\DAYvUl_XYAA2i87.jpg");
	size_t cols=size_t(frame.cols);
	size_t rows=size_t(frame.rows);
	lyonste::matrix::Mat2D<uchar> gray(cols,rows);
	gray.overwrite(frame);
	std::vector<cv::Mat> standardFrames;
	std::vector<lyonste::matrix::Mat2D<uchar>> customFrames;
	for(size_t i=1; i!=numLevels; ++i)
	{
		float layerScale=(float)(std::pow(scaleFactor,(double)i));
		const size_t levelCols=size_t(cvRound(cols/layerScale));
		const size_t levelRows=size_t(cvRound(rows/layerScale));
		standardFrames.push_back(cv::Mat(int(levelCols),int(levelRows),CV_8U));
		customFrames.push_back(lyonste::matrix::Mat2D<uchar>(levelCols,levelRows));
	}
	size_t i=0;
	cv::resize(gray.getCVMat(),standardFrames[i],cv::Size(standardFrames[i].cols,standardFrames[i].rows));
	lyonste::matrix::resizer::GrayMat2DResizer* resizer=lyonste::matrix::resizer::GrayMat2DResizer::getResizer<lyonste::HAL_SSE2>(cols,rows,customFrames[i].x,customFrames[i].y);
	resizer->resize(gray,customFrames[i]);
	delete resizer;
	compareImage(customFrames[i].getCVMat(),standardFrames[i]);
	if(true)
	{

		cv::Mat customOut;
		cv::resize(customFrames[i].getCVMat(),customOut,cv::Size(568,568),cv::INTER_NEAREST);
		cvNamedWindow("custom image",cv::WINDOW_AUTOSIZE);
		cv::imshow("custom image",customOut);
		//cv::resizeWindow("custom image", 568, 568);
		cv::Mat standardOut;
		cv::resize(standardFrames[i],standardOut,cv::Size(568,568),cv::INTER_NEAREST);
		cvNamedWindow("standard image",cv::WINDOW_AUTOSIZE);
		cv::imshow("standard image",standardOut);
		//cv::resizeWindow("standard image", 568, 568);

		cv::waitKey();
	}
	for(size_t i=1; i!=numLevels-1; ++i)
	{
		cv::resize(standardFrames[i-1],standardFrames[i],cv::Size(standardFrames[i].cols,standardFrames[i].rows));
		resizer=lyonste::matrix::resizer::GrayMat2DResizer::getResizer<lyonste::HAL_SSE2>(customFrames[i-1].x,customFrames[i-1].y,customFrames[i].x,customFrames[i].y);
		resizer->resize(customFrames[i-1],customFrames[i]);
		delete resizer;
		compareImage(customFrames[i].getCVMat(),standardFrames[i]);
		if(true)
		{

			cv::Mat customOut;
			cv::resize(customFrames[i].getCVMat(),customOut,cv::Size(568,568),cv::INTER_NEAREST);
			cvNamedWindow("custom image",cv::WINDOW_AUTOSIZE);
			cv::imshow("custom image",customOut);
			//cv::resizeWindow("custom image", 568, 568);
			cv::Mat standardOut;
			cv::resize(standardFrames[i],standardOut,cv::Size(568,568),cv::INTER_NEAREST);
			cvNamedWindow("standard image",cv::WINDOW_AUTOSIZE);
			cv::imshow("standard image",standardOut);
			//cv::resizeWindow("standard image", 568, 568);

			cv::waitKey();
		}
	}
}





int main(int argc,char* argv[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF);
	if(false)
	{
		testResizer(2.0,12);
	}
	else
	{
		float scaleFactor=2.1f;
		if(false)
		{

			for(size_t i=0; i < 10; ++i)
			{
				testORB(scaleFactor-=0.1f);
			}
		}
		else
		{
			cv::setNumThreads(3);
			std::thread threads[9];
			for(size_t i=0; i < 9; ++i)
			{
				threads[i]=std::thread(&testORB,scaleFactor-=0.1f);
			}
			testORB(1.1f);
			for(size_t i=0; i < 9; ++i)
			{
				threads[i].join();
			}
		}
	}




	return 1;
}
#else






#ifdef NEW_IMPLEMENTATION

void testFREAKVideo()
{
	using namespace std;
	using namespace lyonste;
	cout<<"here1"<<endl;
	//std::string filename = "F:\\Downloads\\MST3k Season 11 New Door Sequence (BUBBLES!).mp4";
	//std::string filename = "F:\\Downloads\\MST3k s08 e20   Space Mutiny 480p.mp4";
	//cv::VideoCapture cap(filename);
	cv::VideoCapture cap(0);
	cv::Mat capMat,videoOut,descMat;
	cv::namedWindow("video",cv::WINDOW_AUTOSIZE);
	cv::namedWindow("descriptors",cv::WINDOW_AUTOSIZE);
	size_t cols=size_t(cap.get(cv::CAP_PROP_FRAME_WIDTH));
	size_t rows=size_t(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
	lyonste::matrix::Mat2D<uchar> gray(cols,rows);
	std::vector<cv::KeyPoint> keyPoints;
	FeatureDetectorFactory* kpGenFactory;
	FeatureDetector* kpGen;
	cout<<"here2"<<endl;

	if(true)
	{
		ORBParameters orbParameters;
		//orbParameters.fgMaskHistory = 5;

		orbParameters.setNumFeatures(1000);
		kpGenFactory=FeatureDetectorFactory::getFeatureDetectorFactory(orbParameters);
		kpGen=kpGenFactory->getFeatureDetector(cols,rows);

	}
	else
	{
		FASTParameters fastParameters;
		fastParameters.setBorder(3);
		fastParameters.setFASTThreshold(10);
		//fastParameters.fgMaskHistory = 30;
		//fastParameters.fgMaskThreshold = 25;
		kpGenFactory=FeatureDetectorFactory::getFeatureDetectorFactory(fastParameters);
		kpGen=kpGenFactory->getFeatureDetector(cols,rows);

	}
	cout<<"here3"<<endl;

	FREAKParameters freakParameters;
	FeatureComputerFactory* descGenFactory=FeatureComputerFactory::getFeatureComputerFactory(freakParameters);
	FeatureComputer* descGen=descGenFactory->getFeatureComputer(cols,rows);
	//int flags = cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS;
	int flags=cv::DrawMatchesFlags::DEFAULT|cv::DrawMatchesFlags::DRAW_OVER_OUTIMG;
	cv::waitKey();


	while(cap.read(capMat))
	{
		cout<<"here4"<<endl;

		gray.overwrite(capMat);
		kpGen->detect(gray,keyPoints);
		descGen->compute(gray,keyPoints,descMat);
		//cv::Mat out(cols, rows,CV_8U);
		//videoOut.create(capMat.cols, capMat.rows,capMat.type());
		capMat.setTo(cv::Scalar(255,255,255));

		cv::drawKeypoints(capMat,keyPoints,capMat,cv::Scalar(0,0,255),flags);
		cv::imshow("video",capMat);
		if(descMat.cols > 0&&descMat.rows > 0)
		{
			//cv::Mat resized;
			//cv::resize(descMat, resized, cv::Size(), 2, 2,cv::INTER_AREA);
			//cv::imshow("descriptors", resized);
			cv::imshow("descriptors",descMat);
		}

		cv::waitKey(1);
	}
	delete kpGenFactory;
	delete descGenFactory;
	delete kpGen;
	delete descGen;
	cap.release();
	cv::waitKey();


}


void compareMat(const cv::Mat& mat1,const cv::Mat& mat2)
{
	using namespace std;

	size_t area1=size_t(mat1.cols)*size_t(mat1.rows);
	size_t area2=size_t(mat2.cols)*size_t(mat2.rows);
	size_t min;
	if(area1 < area2)
	{
		min=area1;
	}
	else
	{
		min=area2;
	}
	//size_t min = std::min(area1, area2);
	const uchar* data1=mat1.data;
	const uchar* data2=mat2.data;
	for(size_t i=0; i < min; ++i)
	{
		if(data1[i]!=data2[i])
		{
			cout<<"data1["<<i<<"]="<<data1[i]<<"; data2["<<i<<"]="<<data2[i]<<endl;
		}
	}
	if(mat1.rows!=mat2.rows)
	{
		cout<<"mat1.rows="<<mat1.rows<<"; mat2.rows="<<mat2.rows<<endl;
	}
	if(mat1.cols!=mat2.cols)
	{
		cout<<"mat1.cols="<<mat1.cols<<"; mat2.cols="<<mat2.cols<<endl;
	}



}

void testFREAK()
{
	using namespace lyonste;
	cv::Mat frame=cv::imread("F:\\Downloads\\DAYvUl_XYAA2i87.jpg");
	std::vector<cv::KeyPoint> keyPoints1;
	std::vector<cv::KeyPoint> keyPoints2;
	lyonste::matrix::Mat2D<uchar> gray(frame);
	ORBParameters parameters;

	FeatureDetectorFactory* factory=FeatureDetectorFactory::getFeatureDetectorFactory(parameters);
	FeatureDetector* detector=factory->getFeatureDetector(gray.x,gray.y);
	detector->detect(gray,keyPoints1);

	cv::Ptr<cv::ORB> orb=cv::ORB::create();
	orb->detect(frame,keyPoints2);
	FREAKParameters freakParameters;
	freakParameters.orientationNormalized=true;
	freakParameters.scaleNormalized=true;
	FeatureComputerFactory* freakFactory=FeatureComputerFactory::getFeatureComputerFactory(freakParameters);
	FeatureComputer* freakComputer=freakFactory->getFeatureComputer(gray.x,gray.y);
	cv::Mat customDescriptors=freakComputer->compute(gray,keyPoints1);
	cv::Mat standardDescriptors;
	if(false)
	{
		FeatureComputer* freakComputer2=freakFactory->getFeatureComputer(gray.x,gray.y);
		standardDescriptors=freakComputer2->compute(gray,keyPoints1);
		delete freakComputer2;
	}
	else
	{
		cv::Ptr<cv::xfeatures2d::FREAK> freakPtr=cv::xfeatures2d::FREAK::create(true,true);
		freakPtr->compute(gray.getCVMat(),keyPoints2,standardDescriptors);
	}
	delete freakComputer;
	delete freakFactory;


	compareMat(customDescriptors,standardDescriptors);

	cv::namedWindow("mat1",cv::WINDOW_AUTOSIZE);
	cv::imshow("mat1",standardDescriptors);
	cv::namedWindow("mat2",cv::WINDOW_AUTOSIZE);
	cv::imshow("mat2",customDescriptors);

	cv::waitKey();
}
#endif





//
//void testVideoSigGen()
//{
//	using namespace lyonste;
//	cv::setNumThreads(0);
//	std::ifstream inStream("F:\\duplicateVideoSearchConfig.xml");
//	boost::property_tree::ptree pt;
//	boost::property_tree::read_xml(inStream,pt);
//	inStream.close();
//	lyonste::videosig::cleanUpSigs(pt);
//	std::vector<boost::filesystem::path> files;
//	lyonste::videosig::gatherFiles(files,pt);
//	std::vector<lyonste::videosig::VideoSignature> signatures;
//	lyonste::videosig::generateSigs(files,signatures,pt);
//	std::multiset<lyonste::matching::DefaultMatchPair> matches;
//	lyonste::matching::findMatches(signatures,matches,pt);
//
//}
//
//void writeProperties()
//{
//	std::string sigRoot="F:\\sigs";
//
//	boost::property_tree::ptree duplicateVideoSearch;
//
//	//videoGathering
//	boost::property_tree::ptree videoGathering;
//	videoGathering.put<std::string>("root","F:\\uncategorized");
//	videoGathering.add<std::string>("includeRegex","(.*)\\.(mp4|m4v|mkv|avi|divx|flv|mov|mpg|wmv)");
//	videoGathering.add<std::string>("excludeRegex","F:\\\\uncategorized\\\\other(.*)");
//	duplicateVideoSearch.add_child("videoGathering",videoGathering);
//
//
//	boost::property_tree::ptree fastProperties;
//	fastProperties.put<uchar>("patternSize",16);
//	fastProperties.put<std::string>("scoreType","nonMaxSuppressionFast");
//	fastProperties.put<bool>("compatibilityMode",true);
//	fastProperties.put<size_t>("border",22);
//	fastProperties.put<uchar>("fastThreshold",20);
//
//
//	boost::property_tree::ptree layerProperties;
//	layerProperties.put<std::string>("featureDetectorType","fast");
//	layerProperties.add_child("fastFeatureDetector",fastProperties);
//
//	boost::property_tree::ptree orbProperties;
//	orbProperties.put<bool>("harrisScore",true);
//	orbProperties.put<bool>("calculateAngles",false);
//	orbProperties.put<size_t>("numFeatures",1000);
//	orbProperties.put<size_t>("numLevels",16);
//	orbProperties.put<size_t>("patchSize",31);
//	orbProperties.put<double>("scaleFactor",1.2f);
//	orbProperties.put<size_t>("historyLength",5);
//	orbProperties.put<short>("fgDiffThreshold",75);
//	orbProperties.put<float>("weightFactor",1000);
//
//
//	orbProperties.add_child("layerProperties",layerProperties);
//
//
//
//	boost::property_tree::ptree featureDetection;
//	featureDetection.put<std::string>("featureDetectorType","orb");
//	featureDetection.add_child("orbFeatureDetector",orbProperties);
//
//
//	boost::property_tree::ptree freakProperties;
//	freakProperties.put<float>("patternScale",11.f);
//	freakProperties.put<size_t>("numOctaves",8);
//	freakProperties.put<bool>("orientationNormalized",false);
//	freakProperties.put<bool>("scaleNormalized",true);
//
//
//	boost::property_tree::ptree featureDescription;
//	featureDescription.put<std::string >("featureComputerType","freak");
//	featureDescription.add_child("freakFeatureComputer",freakProperties);
//
//
//
//	//signatureCleaning
//	boost::property_tree::ptree sigGeneration;
//	sigGeneration.put<size_t>("version",0);
//	sigGeneration.put<std::string>("root",sigRoot);
//	sigGeneration.add<std::string>("includeRegex","(.*)\\.sig");
//	sigGeneration.add<double>("keyFrameIntervalMS",2000.0);
//	sigGeneration.add_child("featureDetectionProperties",featureDetection);
//	sigGeneration.add_child("featureDescriptionProperties",featureDescription);
//
//
//
//
//
//
//
//	duplicateVideoSearch.add_child("sigGeneration",sigGeneration);
//
//
//
//	boost::property_tree::ptree matching;
//	matching.put<size_t>("version",0);
//	matching.put<std::string>("blackListFile","F:\\blacklist.filemap");
//	//matching.put<std::string>("blackListFile", "F:\\blacklist.filemap.tmp");
//	matching.put<std::string>("whiteListFile","F:\\whitelist.filemap");
//	matching.put<double>("matchTimeDiffRatio",.0025);
//	//matching.put<double>("minDuration", 600000);
//	//matching.put<double>("maxDuration", 60000);
//#ifdef _DEBUG
//	matching.put<size_t>("numThreads",cv::getNumberOfCPUs());
//	//matching.put<size_t>("numThreads", 1);
//#else
//	//matching.put<size_t>("numThreads", 1);
//	matching.put<size_t>("numThreads",cv::getNumberOfCPUs());
//#endif
//
//	matching.put<size_t>("maxMemoryFootprint",8589934592);
//	matching.put<double>("skipRatio",.05);
//	matching.put<bool>("matchLongestSigsFirst",false);
//	matching.put<double>("keyFrameIntervalRatio",.01);
//	matching.put<double>("multiphasicNarrowingFactor",.025);
//	matching.put<size_t>("multiphasicInitialTargetNumMatches",100);
//	matching.put<bool>("multiphasicMatching",false);
//	matching.put<bool>("printNonMatches",true);
//
//	switch(0)
//	{
//		case 0:
//			matching.put<std::string>("matchingType","drifted");
//			matching.put<double>("maxDriftRatio",.0025);
//			matching.put<double>("driftRatioThreshold",.30);
//			break;
//		default:
//			matching.put<std::string>("matchingType","default");
//	}
//
//	matching.put<std::string>("matchesFile","F:\\matches.txt");
//	matching.put<size_t>("targetNumMatches",100);
//	matching.put<double>("maxSimilarity",1.0);
//	duplicateVideoSearch.add_child("matching",matching);
//
//
//	//write the file
//	std::ofstream outStream("F:\\duplicateVideoSearchConfig.xml");
//	boost::property_tree::write_xml(outStream,duplicateVideoSearch);
//	outStream.close();
//
//}
//
//

//void updateFileMap(lyonste::FileMap& fileMap,const boost::filesystem::path& rootPath)
//{
//	boost::filesystem::directory_iterator endItr;
//	for(boost::filesystem::directory_iterator itr1(rootPath); itr1!=endItr; ++itr1)
//	{
//		boost::filesystem::path folder=itr1->path();
//		if(boost::filesystem::is_directory(folder))
//		{
//			for(boost::filesystem::directory_iterator itr2(folder); itr2!=endItr; ++itr2)
//			{
//				boost::filesystem::path file1=itr2->path();
//				if(boost::filesystem::is_regular_file(file1))
//				{
//					for(boost::filesystem::directory_iterator itr3(folder); itr3!=endItr; ++itr3)
//					{
//						boost::filesystem::path file2=itr3->path();
//						if(boost::filesystem::is_regular_file(file2))
//						{
//							if(file2!=file1)
//							{
//								fileMap.addMapping(file1,file2);
//							}
//						}
//
//					}
//				}
//			}
//		}
//	}
//}
//

//void printFileMap(const lyonste::FileMap& fileMap,const std::string& mapName="")
//{
//	for(auto mapItr=fileMap.begin(); mapItr!=fileMap.end(); ++mapItr)
//	{
//		const lyonste::FileInfo* info1=mapItr->first;
//		const std::unordered_set<lyonste::FileInfo*>& fileSet=mapItr->second;
//		for(auto setItr=fileSet.begin(); setItr!=fileSet.end(); ++setItr)
//		{
//			const lyonste::FileInfo* info2=*setItr;
//			std::cout<<mapName<<" "<<info1->getPath()<<" <--> "<<info2->getPath()<<std::endl;
//		}
//		std::cout<<std::endl;
//	}
//}
//

//void updateFileMaps()
//{
//	boost::filesystem::path whiteListFile("F:\\whitelist.filemap");
//	boost::filesystem::path blackListFile("F:\\blacklist.filemap");
//	lyonste::FileMap whiteList(whiteListFile);
//	lyonste::FileMap blackList(blackListFile);
//
//
//
//	updateFileMap(whiteList,boost::filesystem::path("F:\\PotentialDuplicates"));
//	updateFileMap(blackList,boost::filesystem::path("F:\\BlackList"));
//
//
//	whiteList.removeFileWithRegex("(.*)desktop\\.ini");
//	blackList.removeFileWithRegex("(.*)desktop\\.ini");
//
//	whiteList.writeToFile(whiteListFile);
//	blackList.writeToFile(blackListFile);
//
//
//
//	//printFileMap(whiteList, "whiteList");
//	//std::cout << std::endl;
//	//printFileMap(blackList, "blackList");
//
//
//}
//

//void writeFileMap()
//{
//	boost::filesystem::path rootPath("F:\\PotentialDuplicates");
//	boost::filesystem::directory_iterator endItr;
//	lyonste::FileMap fileMap;
//	for(boost::filesystem::directory_iterator itr1(rootPath); itr1!=endItr; ++itr1)
//	{
//		boost::filesystem::path folder=itr1->path();
//		if(boost::filesystem::is_directory(folder))
//		{
//			for(boost::filesystem::directory_iterator itr2(folder); itr2!=endItr;++itr2)
//			{
//				boost::filesystem::path file1=itr2->path();
//				if(boost::filesystem::is_regular_file(file1))
//				{
//					for(boost::filesystem::directory_iterator itr3(folder); itr3!=endItr; ++itr3)
//					{
//						boost::filesystem::path file2=itr3->path();
//						if(boost::filesystem::is_regular_file(file2))
//						{
//							if(file2!=file1)
//							{
//								fileMap.addMapping(file1,file2);
//							}
//						}
//
//					}
//				}
//			}
//		}
//	}
//
//	fileMap.writeToFile(boost::filesystem::path("F:\\whitelist.filemap"));
//}
//

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
	return lyonste::feature2D::featureDescription::DescriptorPropertyGenerator<lyonste::feature2D::featureDescription::Freak>()(11.f,8,false,true);
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
	matchProperties.put<double>("maxSimilarity",1.0);
	matchProperties.put<std::string>("whiteListFile","F:\\whiteList2.whiteList");
	matchProperties.put<std::string>("blackListFile","F:\\blackList2.blackList");
	matchProperties.put<std::string>("potentialDupDir","F:\\PotentialDuplicates");
	matchProperties.put<std::string>("blackListDir","F:\\Blacklist");
	matchProperties.put<double>("minDuration",0);
	//matchProperties.put<double>("minDuration",1200000);
	matchProperties.put<double>("maxDuration",std::numeric_limits<double>::infinity());
	matchProperties.put<double>("maxAbsoluteDurationDiff",std::numeric_limits<double>::infinity());
	matchProperties.put<double>("maxDurationRatio",.005);
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
	std::ifstream inStream("F:\\duplicateVideoSearchConfig2.xml");
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
	lyonste::fileManagement::FileGatherer<FileInfo> fileGatherer(rootDir,std::numeric_limits<size_t>::max(),false,std::vector<boost::regex>{boost::regex("(.*)\\.(.*)")},std::vector<boost::regex>{});
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
	boost::property_tree::ptree& cacheProperties=masterProperties.get_child("cacheProperties");
	boost::property_tree::ptree& sigMatchProperties=cacheProperties.get_child("sigMatchProperties");
	boost::filesystem::path whiteListFile=boost::filesystem::path(sigMatchProperties.get<std::string>("whiteListFile"));
	boost::filesystem::path blackListFile=boost::filesystem::path(sigMatchProperties.get<std::string>("blackListFile"));
	boost::filesystem::path whiteListDir=boost::filesystem::path(sigMatchProperties.get<std::string>("potentialDupDir"));
	boost::filesystem::path blackListDir=boost::filesystem::path(sigMatchProperties.get<std::string>("blackListDir"));
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
				djSet.makeUnion(representative,*itr);
			}
		}
	}
	djSet.writeToFile(whiteListFile);

	const std::unordered_map<FileInfo,std::unordered_set<FileInfo>> mapView=djSet.getMapView();

	lyonste::MutualMap<FileInfo> mutualMap;
	mutualMap.readFromFile(blackListFile);
	DirectoryMap& blackListDirMap=getDirectoryMap(blackListDir);
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
									mutualMap.add(set1Val,set2Val);
								}
							}
						}
						else
						{
							for(const auto set1Val:file1SetItr->second)
							{
								mutualMap.add(set1Val,file2);
							}
						}
					}
					else
					{
						if(file2SetItr!=mapView.end())
						{
							for(const auto set2Val:file2SetItr->second)
							{
								mutualMap.add(file1,set2Val);
							}
						}
						else
						{
							mutualMap.add(file1,file2);
						}
					}
				}
			}
		}
	}
	mutualMap.writeToFile(blackListFile);
}

void moveMatches()
{
	boost::property_tree::ptree& masterProperties=getProperties();
	boost::property_tree::ptree& cacheProperties=masterProperties.get_child("cacheProperties");
	boost::property_tree::ptree& sigMatchProperties=cacheProperties.get_child("sigMatchProperties");
	boost::property_tree::ptree& vidGatheringProperties=masterProperties.get_child("videoGatheringProperties");
	lyonste::fileManagement::FileGatherer<FileInfo> vidFileGatherer(vidGatheringProperties);
	FileSet vidFiles;
	vidFileGatherer.gatherFiles(std::inserter(vidFiles,vidFiles.end()));
	const std::string& descriptorType=cacheProperties.get<std::string>("descriptorType");
	if(descriptorType=="freak")
	{

		lyonste::videoSig::VideoSigCache<lyonste::feature2D::featureDescription::Freak> cache(cacheProperties);
		std::unique_ptr<lyonste::videoSig::videoSigMatch::SigMatchSession<lyonste::feature2D::featureDescription::Freak>> sigMatchSession(lyonste::videoSig::videoSigMatch::getSigMatchSession<lyonste::feature2D::featureDescription::Freak>(cache,sigMatchProperties));
		sigMatchSession->readFromFile();
		sigMatchSession->moveMatches(vidFiles,boost::filesystem::path(sigMatchProperties.get<std::string>("potentialDupDir")));
	}
	else
	{
		CV_Error(1,"Unknown descriptorType "+descriptorType);
	}
}

void repairBlackAndWhiteLists()
{
	boost::property_tree::ptree& masterProperties=getProperties();
	boost::property_tree::ptree& cacheProperties=masterProperties.get_child("cacheProperties");
	boost::property_tree::ptree& sigMatchProperties=cacheProperties.get_child("sigMatchProperties");
	boost::filesystem::path whiteListFile=boost::filesystem::path(sigMatchProperties.get<std::string>("whiteListFile"));
	boost::filesystem::path blackListFile=boost::filesystem::path(sigMatchProperties.get<std::string>("blackListFile"));
	boost::filesystem::path whiteListDir=boost::filesystem::path(sigMatchProperties.get<std::string>("potentialDupDir"));
	boost::filesystem::path blackListDir=boost::filesystem::path(sigMatchProperties.get<std::string>("blackListDir"));
	lyonste::MutualMap<FileInfo> blackList;
	blackList.readFromFile(blackListFile);
	lyonste::DisjointSet<FileInfo> whiteList;
	whiteList.readFromFile(whiteListFile);

	for(auto itr1=blackList.begin();itr1!=blackList.end();++itr1)
	{
		const FileInfo& blFile1=*(itr1->first);
		auto& blFileSet=itr1->second;
		for(auto itr2=blFileSet.begin();itr2!=blFileSet.end();++itr2)
		{
			const FileInfo& blFile2=**itr2;
			if(whiteList.areJoined(blFile1,blFile2))
			{
				std::cout<<"removing "<<blFile1.getFilePathStr()<<" and "<<blFile2.getFilePathStr()<<std::endl;
				whiteList.remove(blFile1);
				whiteList.remove(blFile2);
				blackList.remove(blFile1,blFile2);
			}
		}
	}
	blackList.writeToFile(blackListFile);
	whiteList.writeToFile(whiteListFile);
}


void mainDupDetectionRoutine()
{
	cv::setNumThreads(0);
	boost::property_tree::ptree& masterProperties=getProperties();
	boost::property_tree::ptree& cacheProperties=masterProperties.get_child("cacheProperties");
	boost::property_tree::ptree& vidGatheringProperties=masterProperties.get_child("videoGatheringProperties");
	boost::property_tree::ptree& sigGenProperties=cacheProperties.get_child("sigGenProperties");
	boost::property_tree::ptree& sigMatchProperties=cacheProperties.get_child("sigMatchProperties");
	lyonste::fileManagement::FileGatherer<FileInfo> vidFileGatherer(vidGatheringProperties);
	FileSet vidFiles;
	vidFileGatherer.gatherFiles(std::inserter(vidFiles,vidFiles.end()));
	const std::string& descriptorType=cacheProperties.get<std::string>("descriptorType");
	if(descriptorType=="freak")
	{
		lyonste::videoSig::VideoSigCache<lyonste::feature2D::featureDescription::Freak> cache(cacheProperties);
		{
			std::unique_ptr<lyonste::videoSig::videoSigGen::SigGenSession<lyonste::feature2D::featureDescription::Freak,decltype(vidFiles)>> sigGenSession(lyonste::videoSig::videoSigGen::getSigGenSession(vidFiles,cache,sigGenProperties));
			sigGenSession->generateSigs();
		}
		{
			std::unique_ptr<lyonste::videoSig::videoSigMatch::SigMatchSession<lyonste::feature2D::featureDescription::Freak>> sigMatchSession(lyonste::videoSig::videoSigMatch::getSigMatchSession<lyonste::feature2D::featureDescription::Freak>(cache,sigMatchProperties));
			sigMatchSession->generateMatches();
			const auto& vecView=sigMatchSession->getMatchDisjointSet().getVectorView();
			size_t matchGroupCount=0;
			for(const auto& set:vecView)
			{
				std::cout<<"Match group #"<<(++matchGroupCount)<<std::endl;
				for(const auto& vid:set)
				{
					std::cout<<vid.getVidFile().getFilePathStr()<<std::endl;
				}
				std::cout<<std::endl;
			}
		}
	}
	else
	{
		CV_Error(1,"Unknown descriptorType "+descriptorType);
	}

}



int main(int argc,char* argv[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF);
	

	for(;;)
	{
		std::cout<<"   Enter command option"<<std::endl;
		std::cout<<"1. Exit."<<std::endl;
		std::cout<<"2. Run video duplication detection routine."<<std::endl;
		std::cout<<"3. Move matches."<<std::endl;
		std::cout<<"4. Update black and white lists."<<std::endl;
		std::cout<<"5. Repair black and white lists."<<std::endl;
		std::cout<<"6. Write default properties."<<std::endl;
		std::cout<<"7. Test FAST keypoint generator."<<std::endl;
		
		int option;
		std::cin>>option;
		switch(option)
		{
			case 1:
				return 0;
			case 2:
				mainDupDetectionRoutine();
				continue;
			case 3:
				moveMatches();
				continue;
			case 4:
				updateFileMaps();
				continue;
			case 5:
				repairBlackAndWhiteLists();
				continue;
			case 6:
				writeProperties();
				continue;
			case 7:
				lyonste::feature2D::featureDetection::segmenttest::fast::testFast();
				continue;
			default:
				std::cout<<"Unknown command option"<<std::endl;
		}
	}
	
	

}

#endif

#ifdef _DEBUG
#ifdef DEBUG_NEW
#undef DEBUG_NEW
#endif
#ifdef new
#undef new
#endif
#endif
