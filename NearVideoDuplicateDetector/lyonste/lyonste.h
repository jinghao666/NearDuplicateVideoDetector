#pragma once

#include <opencv2/core.hpp>
#include <boost/property_tree/ptree.hpp>

namespace lyonste
{



	constexpr size_t getNumThreadsProperty(const boost::property_tree::ptree& properties,const std::string& propertyName="numThreads") noexcept
	{
#ifdef NDEBUG
		return std::max<size_t>(1,std::min<size_t>(cv::getNumberOfCPUs(),properties.get<size_t>(propertyName,cv::getNumberOfCPUs())));
#else
		return std::max<size_t>(1,std::min<size_t>(cv::getNumberOfCPUs(),properties.get<size_t>(propertyName,cv::getNumberOfCPUs())));
		//return 1;
#endif
		
	}


	template<typename dimensionType>
	constexpr dimensionType clipHiBorder(const dimensionType dimension,const dimensionType border)
	{
		return dimension<border?0:dimension-border;
	}

	template<typename dimensionType>
	constexpr dimensionType clip(dimensionType val,dimensionType exclusiveMax)
	{
		return val<exclusiveMax?val:exclusiveMax-1;
	}

	template<typename dimensionType>
	constexpr dimensionType clip(dimensionType min,dimensionType val,dimensionType exclusiveMax)
	{
		return std::max<dimensionType>(min,std::min<dimensionType>(val,exclusiveMax-1));
	}


	constexpr size_t ceiling(const float val)
	{
		size_t i=(size_t)val;
		return i+(i<val);
	}
	constexpr size_t ceiling(const double val)
	{
		size_t i=(size_t)val;
		return i+(i<val);
	}

	template<typename T>
	struct DerefPtrHash
	{
		constexpr size_t operator()(const T* ptr) const
		{
			if(ptr)
			{
				return std::hash<T>()(*ptr);
			}
			return 0;
		}
	};
	template<typename T>
	struct DerefPtrEqualTo
	{
		constexpr bool operator()(const T* ptr1,const T* ptr2) const
		{
			return ptr1==ptr2||(ptr1 && ptr2 && std::equal_to<T>()(*ptr1,*ptr2));
		}
	};

	template<typename T>
	struct DerefPtrLess
	{
		constexpr bool operator()(const T* ptr1,const T* ptr2) const
		{
			if(ptr1)
			{
				if(ptr2)
				{
					return std::less<T>()(*ptr1,*ptr2);
				}
				return false;
			}
			if(ptr2)
			{
				return true;
			}
			return false;;
		}
	};




	//enum ExceptionCause
	//{
	//	UnspecifiedCause,
	//	VideoFileDoesNotExist,
	//	UnableToOpenVideoFile,
	//	UnableToGrabVideoFrame,
	//	InvalidVideoDimensions,
	//	UnableToOpenKeyFrameFileForWriting
	//};

	//template<ExceptionCause cause>
	//std::exception createException(const std::string& message)
	//{
	//	return std::exception(message.c_str(),(int)cause);
	//}
}



