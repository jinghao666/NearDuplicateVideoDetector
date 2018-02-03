
#include "featureDetection.h"

namespace lyonste
{
	namespace feature2D
	{
		namespace featureDetection
		{


			KeyPointGeneratorFactory * KeyPointGeneratorFactory::getKeyPointGeneratorFactory(const boost::property_tree::ptree & properties,const lyonste::hal::HAL_FLAG hal)
			{
				const std::string keyPointType=properties.get<std::string>("keyPointType");
				if(keyPointType==kpTypeStr<Fast>)
				{
					return configureKeyPointGeneratorFactory<Fast>(properties,hal);
				}
				else if(keyPointType==kpTypeStr<Orb>)
				{
					return configureKeyPointGeneratorFactory<Orb>(properties,hal);
				}
				else if(keyPointType==kpTypeStr<MotionDetecting>)
				{
					return configureKeyPointGeneratorFactory<MotionDetecting>(properties,hal);
				}
				else
				{
					CV_Error(1,"Unknown keyPointType: "+keyPointType);
				}
				return NULL;
			}

			cv::Ptr<cv::Feature2D> KeyPointGeneratorFactory::getOpenCVKeyPointGenerator(const boost::property_tree::ptree & properties,bool ignoreIncompatibleProperties)
			{
				const std::string keyPointType=properties.get<std::string>("keyPointType");
				if(keyPointType==kpTypeStr<Fast>)
				{
					return configureOpenCVKeyPointGenerator<Fast>(properties,ignoreIncompatibleProperties);
				}
				else if(keyPointType==kpTypeStr<Orb>)
				{
					return configureOpenCVKeyPointGenerator<Orb>(properties,ignoreIncompatibleProperties);
				}
				else if(keyPointType==kpTypeStr<MotionDetecting>)
				{
					return configureOpenCVKeyPointGenerator<MotionDetecting>(properties,ignoreIncompatibleProperties);
				}
				else
				{
					CV_Error(1,"Unknown keyPointType: "+keyPointType);
				}
				return cv::Ptr<cv::Feature2D>();
			}
		}
	}
}