#pragma once
#include "../../matrix/matrix.h"
#include <boost/property_tree/ptree.hpp>
#include <opencv2/features2d.hpp>
//#include "../../lyonste.h"
#include "../../ToStringUtilities.h"

namespace lyonste
{
	namespace feature2D
	{
		namespace featureDetection
		{

			//An enumeration of keypoint detector types
			enum KeyPointType
			{
				Fast,
				Orb,
				MotionDetecting,
				Agast
			};

			template<KeyPointType kpType>
			constexpr const char* const kpTypeStr=("<Unknown KeyPointType "+std::to_string((size_t)kpType)+">").c_str();


			ENUM_STR(kpTypeStr,Fast);
			ENUM_STR(kpTypeStr,Orb);
			ENUM_STR(kpTypeStr,MotionDetecting);
			ENUM_STR(kpTypeStr,Agast);

			inline std::string kpTypeToStr(const KeyPointType kpType) noexcept
			{
				switch(kpType)
				{
					case Fast:
						return kpTypeStr<Fast>;
					case Orb:
						return kpTypeStr<Orb>;
					case MotionDetecting:
						return kpTypeStr<MotionDetecting>;
					case Agast:
						return kpTypeStr<Agast>;
					default:
						return "<Unknown KeyPointType "+std::to_string((size_t)kpType)+">";
				}
			}

			inline KeyPointType kpTypeFromStr(const std::string& str)
			{
				if(str==kpTypeStr<Fast>)
				{
					return Fast;
				}
				if(str==kpTypeStr<Orb>)
				{
					return Orb;
				}
				if(str==kpTypeStr<MotionDetecting>)
				{
					return MotionDetecting;
				}
				if(str==kpTypeStr<Agast>)
				{
					return Agast;
				}
				CV_Error(1,"Unknown KeyPointType "+str);
				return (KeyPointType)0;
			}
			
			
			
			enum KPRetentionPolicy
			{
				NON_MAX_SUPPRESSION,
				RETAIN_ALL,
				RETAIN_ALL_NO_SCORE
			};

			template<KPRetentionPolicy retentionPolicy>
			constexpr const char* const kpRetentionPolicyStr=("<Unknown KPRetentionPolicy "+std::to_string((size_t)retentionPolicy)+">").c_str();

			ENUM_STR(kpRetentionPolicyStr,NON_MAX_SUPPRESSION);
			ENUM_STR(kpRetentionPolicyStr,RETAIN_ALL); 
			ENUM_STR(kpRetentionPolicyStr,RETAIN_ALL_NO_SCORE);

			inline std::string kpRetentionPolicyToStr(const KPRetentionPolicy retentionPolicy) noexcept
			{
				switch(retentionPolicy)
				{
					case NON_MAX_SUPPRESSION:
						return kpRetentionPolicyStr<NON_MAX_SUPPRESSION>;
					case RETAIN_ALL:
						return kpRetentionPolicyStr<RETAIN_ALL>;
					case RETAIN_ALL_NO_SCORE:
						return kpRetentionPolicyStr<RETAIN_ALL_NO_SCORE>;
					default:
						return "<Unknown KPRetentionPolicy "+std::to_string((size_t)retentionPolicy)+">";
				}
			}

			inline KPRetentionPolicy kpRetentionPolicyFromStr(const std::string& str)
			{
				if(str==kpRetentionPolicyStr<NON_MAX_SUPPRESSION>)
				{
					return NON_MAX_SUPPRESSION;
				}
				if(str==kpRetentionPolicyStr<RETAIN_ALL>)
				{
					return RETAIN_ALL;
				}
				if(str==kpRetentionPolicyStr<RETAIN_ALL_NO_SCORE>)
				{
					return RETAIN_ALL_NO_SCORE;
				}
				CV_Error(1,"Unknown KPRetentionPolicy "+str);
				return (KPRetentionPolicy)0;
			}


			namespace segmenttest
			{
				enum PatternShape
				{
					Circle,
					Diamond
				};

				template<PatternShape patternShape>
				constexpr const char* const patternShapeStr=("<Unknown PatternShape "+std::to_string((size_t)patternShape)+">").c_str();

				ENUM_STR(patternShapeStr,Circle);
				ENUM_STR(patternShapeStr,Diamond);

				inline std::string patternShapeToStr(const PatternShape patternShape) noexcept
				{
					switch(patternShape)
					{
						case Circle:
							return patternShapeStr<Circle>;
						case Diamond:
							return patternShapeStr<Diamond>;
						default:
							return "<Unknown PatternShape "+std::to_string((size_t)patternShape)+">";
					}
				}


				inline PatternShape patternShapeFromStr(const std::string& str)
				{
					if(str==patternShapeStr<Circle>)
					{
						return Circle;
					}
					if(str==patternShapeStr<Diamond>)
					{
						return Diamond;
					}
					CV_Error(1,"Unknown PatternShape "+str);
					return (PatternShape)0;
				}



				//template<bool compatibilityMode,size_t patternSize,PatternShape shape>
				//const std::enable_if_t<!(patternSize==16&&shape==Diamond)&&(compatibilityMode||patternSize==16||(patternSize==12&&shape==Diamond)),size_t> patternBorder=3;

				//template<>
				//static const std::enable_if_t<true,size_t> patternBorder<false,8,Circle> =1;

				//template<bool compatibilityMode>
				//const std::enable_if_t<true,size_t> patternBorder<compatibilityMode,16,Diamond> =4;

				//template<size_t patternSize,PatternShape shape>
				//const std::enable_if_t<((patternSize==12&&shape==Circle)||(patternSize==8&&shape==Diamond)),size_t> patternBorder<false,patternSize,shape> =2;

				//


				template<bool compatibilityMode,size_t patternSize,PatternShape shape,typename ENABLE=std::enable_if_t<!(patternSize==16&&shape==Diamond)&&(compatibilityMode||patternSize==16||(patternSize==12&&shape==Diamond))>>
				const size_t patternBorder=3;

				template<bool compatibilityMode>
				const size_t patternBorder<compatibilityMode,16,Diamond,void> =4;

				template<size_t patternSize,PatternShape shape>
				const size_t patternBorder<false,patternSize,shape,std::enable_if_t<((patternSize==12&&shape==Circle)||(patternSize==8&&shape==Diamond))>> =2;

				template<>
				static const size_t patternBorder<false,8,Circle,void> =1;

				template<size_t patternSize,PatternShape shape>
				class Pattern
				{
				public:

					enum
					{
						halfPatternSize=patternSize/2,
						quarterPatternSize=patternSize/4,
					};

					const int pixel[patternSize];

					

					constexpr Pattern(const int cols) noexcept;
				};

				template<> constexpr Pattern<16,Circle>::Pattern(const int cols) noexcept
					:pixel
				{
					+0+(cols*3),/////0
					+1+(cols*3),/////1
					+2+(cols*2),/////2
					+3+(cols*1),/////3
					+3-(cols*0),/////4
					+3-(cols*1),/////5
					+2-(cols*2),/////6
					+1-(cols*3),/////7
					-0-(cols*3),/////8
					-1-(cols*3),/////9         { 9}{ 8}{ 7}
					-2-(cols*2),////10     {10}            { 6}
					-3-(cols*1),////11 {11}                    { 5}
					-3+(cols*0),////12 {12}        {  }        { 4}
					-3+(cols*1),////13 {13}                    { 3}
					-2+(cols*2),////14     {14}            { 2}
					-1+(cols*3)/////15         {15}{ 0}{ 1}
				}
				{}

				template<> constexpr Pattern<16,Diamond>::Pattern(const int cols) noexcept
					:pixel
				{
					+0+(cols*4),/////0
					+1+(cols*3),/////1
					+2+(cols*2),/////2
					+3+(cols*1),/////3
					+4-(cols*0),/////4
					+3-(cols*1),/////5
					+2-(cols*2),/////6
					+1-(cols*3),/////7                 { 8}
					-0-(cols*4),/////8             { 9}    { 7}
					-1-(cols*3),/////9         {10}            { 6}
					-2-(cols*2),////10     {11}                    { 5}
					-3-(cols*1),////11 {12}            {  }            { 4}
					-4+(cols*0),////12     {13}                    { 3}
					-3+(cols*1),////13         {14}            { 2}
					-2+(cols*2),////14             {15}    { 1}
					-1+(cols*3)/////15                 { 0}
				}
				{}

				template<> constexpr Pattern<12,Circle>::Pattern(const int cols) noexcept
					:pixel
				{
					+0+(cols*2),/////0
					+1+(cols*2),/////1
					+2+(cols*1),/////2
					+2+(cols*0),/////3
					+2-(cols*1),/////4
					+1-(cols*2),/////5
					-0-(cols*2),/////6
					-1-(cols*2),/////7     { 7}{ 6}{ 5}
					-2-(cols*1),/////8 { 8}            { 4}
					-2-(cols*0),/////9 { 9}    {  }    { 3}
					-2+(cols*1),////10 {10}            { 2}
					-1+(cols*2)/////11     {11}{ 0}{ 1}
				}
				{}

				template<> constexpr Pattern<12,Diamond>::Pattern(const int cols) noexcept
					:pixel
				{
					+0+(cols*3),/////0
					+1+(cols*2),/////1
					+2+(cols*1),/////2
					+3+(cols*0),/////3
					+2-(cols*1),/////4
					+1-(cols*2),/////5             { 6}
					-0-(cols*3),/////6         { 7}    { 5}
					-1-(cols*2),/////7     { 8}            { 4}
					-2-(cols*1),/////8 { 9}        {  }        { 3}
					-3-(cols*0),/////9     {10}            { 2}
					-2+(cols*1),////10         {11}    { 1}
					-1+(cols*2)/////11             { 0}
				}
				{}

				template<> constexpr Pattern<8,Circle>::Pattern(const int cols) noexcept
					:pixel
				{
					+0+(cols*1),////0
					+1+(cols*1),////1
					+1+(cols*0),////2
					+1-(cols*1),////3
					-0-(cols*1),////4
					-1-(cols*1),////5 { 5}{ 4}{ 3}
					-1-(cols*0),////6 { 6}{  }{ 2}
					-1+(cols*1)/////7 { 7}{ 0}{ 1}
				}
				{}

				template<> constexpr Pattern<8,Diamond>::Pattern(const int cols) noexcept
					:pixel
				{
					+0+(cols*2),////0
					+1+(cols*1),////1
					+2+(cols*0),////2
					+1-(cols*1),////3         { 4}
					-0-(cols*2),////4     { 5}    { 3}
					-1-(cols*1),////5 { 6}    {  }    { 2}
					-2-(cols*0),////6     { 7}    { 1}
					-1+(cols*1)/////7         { 0}
				}
				{}
			}

			//This class represents a function that generates boost property_trees
			//for use in keypoint generator factory construction
			template<KeyPointType kpType> class KeyPointPropertyGenerator;

			template<> class KeyPointPropertyGenerator<Fast>
			{
			public:
				size_t border;
				uchar fastThreshold;
				size_t patternSize;
				KPRetentionPolicy retentionPolicy;
				segmenttest::PatternShape shape;
				bool compatibilityMode;
				KeyPointPropertyGenerator(size_t border=0,uchar fastThreshold=10,size_t patternSize=16,KPRetentionPolicy retentionPolicy=NON_MAX_SUPPRESSION,segmenttest::PatternShape shape=segmenttest::PatternShape::Circle,bool compatibilityMode=true) noexcept
					:border(border)
					,fastThreshold(fastThreshold)
					,patternSize(patternSize)
					,retentionPolicy(retentionPolicy)
					,shape(shape)
					,compatibilityMode(compatibilityMode)
				{
				}
				boost::property_tree::ptree getProperties();

			};

			template<> class KeyPointPropertyGenerator<Orb>
			{
			public:
				boost::property_tree::ptree layerProperties;
				size_t numFeatures;
				size_t numLevels;
				size_t patchSize;
				float scaleFactor;
				bool harrisScore;
				bool calculateAngles;
				KeyPointPropertyGenerator(const boost::property_tree::ptree& layerProperties,size_t numFeatures=500,size_t numLevels=8,size_t patchSize=31,float scaleFactor=1.2f,bool harrisScore=true,bool calculateAngles=true) noexcept
					:layerProperties(layerProperties)
					,numFeatures(numFeatures)
					,numLevels(numLevels)
					,patchSize(patchSize)
					,scaleFactor(scaleFactor)
					,harrisScore(harrisScore)
					,calculateAngles(calculateAngles)
				{}

				boost::property_tree::ptree getProperties();
				//boost::property_tree::ptree operator()(const boost::property_tree::ptree& layerProperties,size_t numFeatures=500,size_t numLevels=8,size_t patchSize=31,float scaleFactor=1.2f,bool harrisScore=true,bool calculateAngles=true) const;
			};

			template<> class KeyPointPropertyGenerator<MotionDetecting>
			{
			public:
				boost::property_tree::ptree kpGenFactoryProperties;
				size_t historyLength;
				float weightFactor;
				uchar diffThreshold;
				KeyPointPropertyGenerator(const boost::property_tree::ptree& kpGenFactoryProperties,size_t historyLength=1,float weightFactor=1000.f,uchar diffThreshold=75) noexcept
					:kpGenFactoryProperties(kpGenFactoryProperties)
					,historyLength(historyLength)
					,weightFactor(weightFactor)
					,diffThreshold(diffThreshold)
				{}

				boost::property_tree::ptree getProperties();

				//std::string getStr() const noexcept
				//{

				//}
				//boost::property_tree::ptree operator()(const boost::property_tree::ptree& kpGenFactoryProperties,size_t historyLength=1,float weightFactor=1000.f,uchar diffThreshold=75) const;
			};

			//This class represents a factory with, given image dimensions (and optional keypoint constants),
			//will construct a keypoint generator for those given image dimensions
			class KeyPointGeneratorFactory;

			//This class represents a function that takes a gray-scale image and detects keypoints
			//The keypoints are inserted into a vector
			//The images fed into this generator must be appropriatately sized, hence the call to the
			//KeyPointGeneratorFactory's getKeyPointGenerator method
			class KeyPointGenerator
			{
			public:
				virtual void detect(const matrix::Ptr2D<uchar>& frame,std::vector<cv::KeyPoint>& keyPoints)=0;
				virtual const KeyPointGeneratorFactory* getFactory() const noexcept=0;
				virtual ~KeyPointGenerator()
				{
				}
			};

			
			class KeyPointGeneratorFactory
			{
			protected:
				template<KeyPointType kpType>
				static KeyPointGeneratorFactory* configureKeyPointGeneratorFactory(const boost::property_tree::ptree& properties,const lyonste::hal::HAL_FLAG hal);
				template<KeyPointType kpType>
				static cv::Ptr<cv::Feature2D> configureOpenCVKeyPointGenerator(const boost::property_tree::ptree& properties,bool ignoreIncompatibleProperties);
			public:
				//If true, the keypoint generator will calculate a score for each keypoint
				//Otherwise, a default score is used
				virtual bool doResponse() const=0;

				//If true, the keypoint generator will calculate an angle for each keypoint
				//Otherwise, a default angle is used
				virtual bool doAngles() const=0;

				//Returns the pixel border. Keypoints within X pixels of the image edge will not show up as
				//keypoints
				virtual size_t getBorder() const=0;

				//The primary functional method. Given the dimensions of an image, will return a keypoint
				//generator pointer for those dimensions. Optional parameters may be ignored by the implementation,
			    //but otherwise will be used in the resulting keypoints
				virtual KeyPointGenerator* getKeyPointGenerator(size_t cols,size_t rows,float kpSize=7.f,int octave=0,float angle=-1,float response=0,int classID=-1) const=0;

				//Write the properties of this keypoint generator to a property tree.
				//This property tree can then be fed into getKeyPointGeneratorFactory
				//This effectively allows for the serialization and storage of properties
				virtual void writeProperties(boost::property_tree::ptree& properties) const=0;

				//Given a property tree, return a keypoint generator factory pointer.
				static KeyPointGeneratorFactory* getKeyPointGeneratorFactory(const boost::property_tree::ptree& properties,const lyonste::hal::HAL_FLAG hal=lyonste::hal::halFlag);
				
				static cv::Ptr<cv::Feature2D> getOpenCVKeyPointGenerator(const boost::property_tree::ptree& properties,bool ignoreIncompatibleProperties=true);

				virtual std::string toString() const noexcept=0;

				operator std::string() const noexcept
				{
					return toString();
				}



				virtual ~KeyPointGeneratorFactory()
				{
				}
			};

			inline std::ostream& operator<<(std::ostream& stream,const KeyPointGeneratorFactory* kpGenFactory)
			{
				if(kpGenFactory)
				{
					return stream<<(kpGenFactory->operator std::string());
				}
				else
				{
					return stream<<"<NULL>";
				}

			}

			class BestResponseKeyPointCuller: public KeyPointGenerator
			{
			private:
				KeyPointGenerator* const kpGen;
				const size_t numToRetain;

				class KeypointResponseGreater
				{
				public:
					constexpr bool operator()(const cv::KeyPoint& kp1,const cv::KeyPoint& kp2)const
					{
						return kp1.response>kp2.response;
					}
				};

				class KeypointResponseGreaterThanThreshold
				{
				private:
					const float value;
				public:
					constexpr KeypointResponseGreaterThanThreshold(float _value):
						value(_value)
					{}
					constexpr bool operator()(const cv::KeyPoint& kpt) const
					{
						return kpt.response>=value;
					}
				};

			public:

				const KeyPointGeneratorFactory* getFactory() const noexcept
				{
					return kpGen->getFactory();
				}

				static void retainBestKeyPoints(std::vector<cv::KeyPoint>& keyPoints,const size_t numToRetain)
				{
					if(numToRetain>=0&&keyPoints.size() > numToRetain)
					{
						if(numToRetain==0)
						{
							keyPoints.clear();
							return;
						}
						std::nth_element(keyPoints.begin(),keyPoints.begin()+numToRetain,keyPoints.end(),KeypointResponseGreater());
						float ambiguous_response=keyPoints[numToRetain-1].response;
						std::vector<cv::KeyPoint>::const_iterator new_end=std::partition(keyPoints.begin()+numToRetain,keyPoints.end(),KeypointResponseGreaterThanThreshold(ambiguous_response));
						keyPoints.resize(new_end-keyPoints.begin());
					}
				}

				constexpr BestResponseKeyPointCuller(KeyPointGenerator* kpGen,size_t numToRetain):
					kpGen(kpGen),
					numToRetain(numToRetain)
				{

				}
				
				void detect(const matrix::Ptr2D<uchar>& frame,std::vector<cv::KeyPoint>& keyPoints) override
				{
					kpGen->detect(frame,keyPoints);
					retainBestKeyPoints(keyPoints,numToRetain);
				}

				~BestResponseKeyPointCuller()
				{
					delete kpGen;
				}
			};
		}
	}
}

namespace std
{
	//template<typename type> type from_string(const std::string& str);


	//

	//template<>
	//inline lyonste::feature2D::featureDetection::KPRetentionPolicy from_string<lyonste::feature2D::featureDetection::KPRetentionPolicy>(const std::string& str)
	//{
	//	if(str=="NON_MAX_SUPPRESSION")
	//	{
	//		return lyonste::feature2D::featureDetection::KPRetentionPolicy::NON_MAX_SUPPRESSION;
	//	}
	//	if(str=="RETAIN_ALL")
	//	{
	//		return lyonste::feature2D::featureDetection::KPRetentionPolicy::RETAIN_ALL;
	//	}
	//	if(str=="RETAIN_ALL_NO_SCORE")
	//	{
	//		return lyonste::feature2D::featureDetection::KPRetentionPolicy::RETAIN_ALL_NO_SCORE;
	//	}
	//	CV_Error(1,"Unknown KPRetentionPolicy "+str);
	//	return (lyonste::feature2D::featureDetection::KPRetentionPolicy)0;
	//}
	//inline std::string to_string(lyonste::feature2D::featureDetection::segmenttest::PatternShape shape) noexcept
	//{
	//	switch(shape)
	//	{
	//		case lyonste::feature2D::featureDetection::segmenttest::PatternShape::Circle:
	//			return "Circle";
	//		case lyonste::feature2D::featureDetection::segmenttest::PatternShape::Diamond:
	//			return "Diamond";
	//		default:
	//			return "<Unknown PatternShape>";
	//	}
	//}
	//template<>
	//inline lyonste::feature2D::featureDetection::segmenttest::PatternShape from_string<lyonste::feature2D::featureDetection::segmenttest::PatternShape>(const std::string& str)
	//{
	//	if(str=="Circle")
	//	{
	//		return lyonste::feature2D::featureDetection::segmenttest::PatternShape::Circle;
	//	}
	//	if(str=="Diamond")
	//	{
	//		return lyonste::feature2D::featureDetection::segmenttest::PatternShape::Diamond;
	//	}
	//	CV_Error(1,"Unknown PatternShape "+str);
	//	return (lyonste::feature2D::featureDetection::segmenttest::PatternShape)0;
	//}
}

