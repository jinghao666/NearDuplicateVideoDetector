#include "featureDetection.h"
#include "../../lyonste.h"

namespace lyonste
{
	namespace feature2D
	{
		namespace featureDetection
		{
			namespace agast
			{
				//using namespace lyonste::hal;
				//
				//template<PatternSize patternSize=Agast16, KPRetentionPolicy scoreType=NON_MAX_SUPPRESSION, HAL_FLAG hal=HAL_NONE,typename ENABLE=void>
				//class AgastKeyPointGenerator;
				//
				//template<PatternSize patternSize=Agast16,KPRetentionPolicy scoreType=NON_MAX_SUPPRESSION,HAL_FLAG hal=HAL_NONE>
				//class AgastKeyPointGeneratorFactory;
				//
				//template<PatternSize patternSize>
				//class PatternCircle;
				//
				//template<>
				//class PatternCircle<Agast8>
				//{
				//private:
				//	const int circle[8];
				//public:
				//	constexpr PatternCircle(const int cols):
				//		circle
				//	{
				//		    -1,/////////0
				//			-1-cols,////1
				//			-cols,//////2 [1][2][3]
				//			+1-cols,////3 [0]   [4]
				//			+1,/////////4 [7][6][5]
				//			+1+cols,////5
				//			+cols,//////6
				//			-1+cols/////7
				//	}
				//	{}
				//	
				//};
				//
				//template<>
				//class PatternCircle<Agast12d>
				//{
				//private:
				//	const int circle[12];
				//public:
				//	constexpr PatternCircle(const int cols):
				//		circle
				//	{
				//		-3,///////////////0
				//		-2-cols,//////////1
				//		-1-(cols<<1),/////2             [ 3]
				//		-(cols*3),////////3         [ 2]    [ 4]
				//		+1-(cols<<1),/////4     [ 1]            [ 5]
				//		+2-cols,//////////5 [ 0]                    [ 6]
				//		+3,///////////////6     [11]            [ 7]
				//		+2+cols,//////////7         [10]    [ 8]
				//		+1+(cols<<1),/////8             [ 9]
				//		+(cols*3),////////9
				//		-1+(cols<<1),////10
				//		-2+cols//////////11
				//	}
				//	{}
				//
				//};
				//template<>
				//class PatternCircle<Agast12s>
				//{
				//private:
				//	const int circle[12];
				//public:
				//	constexpr PatternCircle(const int cols):
				//		circle
				//	{
				//		-2,///////////////0
				//		-2-cols,//////////1
				//		-1-(cols<<1),/////2
				//		-(cols<<1),///////3     [ 2][ 3][ 4]
				//		+1-(cols<<1),/////4 [ 1]            [ 5]
				//		+2-cols,//////////5 [ 0]            [ 6]
				//		+2,///////////////6 [11]            [ 7]
				//		+2+cols,//////////7     [10][ 9][ 8]
				//		+1+(cols<<1),/////8
				//		+(cols<<1),///////9
				//		-1+(cols<<1),////10
				//		-2+cols//////////11
				//	}
				//	{}
				//
				//};
				//template<>
				//class PatternCircle<Agast16>
				//{
				//private:
				//	const int circle[16];
				//public:
				//	constexpr PatternCircle(const int cols):
				//		circle
				//	{
				//		-3,///////////////0
				//		-3-cols,//////////1
				//		-2-(cols<<1),/////2
				//		-1-(cols*3),//////3
				//		-(cols*3),////////4         [ 3][ 4][ 5]
				//		+1-(cols*3),//////5     [ 2]            [ 6]
				//		+2-(cols<<1),/////6 [ 1]                    [ 7]
				//		+3-cols,//////////7 [ 0]                    [ 8]
				//		+3,///////////////8 [15]                    [ 9]
				//		+3+cols,//////////9     [14]            [10]
				//		+2+(cols<<1),////10         [13][12][11]
				//		+1+(cols*3),/////11
				//		+(cols*3),///////12
				//		-1+(cols*3),/////13
				//		-2+(cols<<1),////14
				//		-3+cols//////////15
				//	}
				//	{}
				//
				//};
			}
		}
	}
}