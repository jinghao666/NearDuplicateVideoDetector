
#include "featureDescription.h"
#include "../../hal.h"
#include "../../matrix/integral.h"
#include <bitset>
#include <opencv2/xfeatures2d.hpp>
#ifdef _DEBUG
#include <iostream>
#endif

namespace lyonste
{
	namespace feature2D
	{
		namespace featureDescription
		{
			using namespace lyonste::hal;
			namespace freak
			{
				uchar meanIntensity(const float xf,const float yf,const matrix::Ptr2D<uchar>& frame)
				{
					const size_t x=size_t(xf);
					const size_t y=size_t(yf);
					const size_t rX0=static_cast<size_t>((xf-x)*1024);
					const size_t rY0=static_cast<size_t>((yf-y)*1024);
					const size_t rX1=(1024-rX0);
					const size_t rY1=(1024-rY0);
					return static_cast<uchar>(((rX1*rY1*int(frame.val(x,y))+rX0*rY1*int(frame.val(x+1,y))+rX1*rY0*int(frame.val(x,y+1))+rX0*rY0*int(frame.val(x+1,y+1)))+(2*1024*1024))/(4*1024*1024));
				}

				struct PatternPoint
				{
					float x;
					float y;
					float sigma;


					template<HAL_FLAG hal>
					uchar extractPointValue(const matrix::Ptr2D<uchar>& frame,const matrix::integral::IntegralMat<hal>* integralMat,const cv::Point2f& pt)
					{
						const float xf=this->x+pt.x;
						const float yf=this->y+pt.y;
						if(sigma<0.5)
						{
							return meanIntensity(xf,yf,frame);
						}
						return integralMat->meanIntensity(xf,yf,sigma);

					}

					friend std::ostream& operator<<(std::ostream& stream,const PatternPoint& pp);

				};

				std::ostream& operator<<(std::ostream & stream,const PatternPoint & pp)
				{
					return stream<<"PatternPoint{x="<<pp.x<<"; y="<<pp.y<<"; sigma="<<pp.sigma<<"}";
				}

				struct DescriptionPair
				{
					uchar i;
					uchar j;

					friend std::ostream& operator<<(std::ostream& stream,const DescriptionPair& descPair);
				};

				std::ostream& operator<<(std::ostream & stream,const DescriptionPair & descPair)
				{
					return stream<<"DescriptionPair{i="<<((int)descPair.i)<<"; j="<<((int)descPair.j)<<"}";
				}
				
				struct OrientationPair
				{
					uchar i;
					uchar j;
					int weightX;
					int weightY;

					friend std::ostream& operator<<(std::ostream& stream,const OrientationPair& oPair);
				};

				std::ostream& operator<<(std::ostream & stream,const OrientationPair & oPair)
				{
					return stream<<"OrientationPair{i="<<((int)oPair.i)<<"; j="<<((int)oPair.j)<<"; weightX="<<oPair.weightX<<"; weightY="<<oPair.weightY<<"}";
				}

				static const double LOG2=0.693147180559945;
				

				constexpr uchar NUM_POINTS_IN_CIRCLE[8]=
				{
					6,6,6,6,6,6,6,1
				};

				constexpr double BIG_R=(2.0/3.0);
				
				constexpr double SMALL_R=(2.0/24.0);
				
				constexpr double UNIT_SPACE=((BIG_R-SMALL_R)/21.0);
				
				constexpr double RADIUS[8]=
				{
					BIG_R, BIG_R-6*UNIT_SPACE, BIG_R-11*UNIT_SPACE, BIG_R-15*UNIT_SPACE, BIG_R-18*UNIT_SPACE, BIG_R-20*UNIT_SPACE, SMALL_R, 0.0
				};
				
				constexpr double SIGMA[8]=
				{
					RADIUS[0]/2.0, RADIUS[1]/2.0, RADIUS[2]/2.0, RADIUS[3]/2.0, RADIUS[4]/2.0, RADIUS[5]/2.0, RADIUS[6]/2.0, RADIUS[6]/2.0
				};

				constexpr DescriptionPair ALL_PAIRS[903]=
				{
					DescriptionPair{1,0},
					DescriptionPair{2,0},DescriptionPair{2,1},
					DescriptionPair{3,0},DescriptionPair{3,1},DescriptionPair{3,2},
					DescriptionPair{4,0},DescriptionPair{4,1},DescriptionPair{4,2},DescriptionPair{4,3},
					DescriptionPair{5,0},DescriptionPair{5,1},DescriptionPair{5,2},DescriptionPair{5,3},DescriptionPair{5,4},
					DescriptionPair{6,0},DescriptionPair{6,1},DescriptionPair{6,2},DescriptionPair{6,3},DescriptionPair{6,4},DescriptionPair{6,5},
					DescriptionPair{7,0},DescriptionPair{7,1},DescriptionPair{7,2},DescriptionPair{7,3},DescriptionPair{7,4},DescriptionPair{7,5},DescriptionPair{7,6},
					DescriptionPair{8,0},DescriptionPair{8,1},DescriptionPair{8,2},DescriptionPair{8,3},DescriptionPair{8,4},DescriptionPair{8,5},DescriptionPair{8,6},DescriptionPair{8,7},
					DescriptionPair{9,0},DescriptionPair{9,1},DescriptionPair{9,2},DescriptionPair{9,3},DescriptionPair{9,4},DescriptionPair{9,5},DescriptionPair{9,6},DescriptionPair{9,7},DescriptionPair{9,8},
					DescriptionPair{10,0},DescriptionPair{10,1},DescriptionPair{10,2},DescriptionPair{10,3},DescriptionPair{10,4},DescriptionPair{10,5},DescriptionPair{10,6},DescriptionPair{10,7},DescriptionPair{10,8},DescriptionPair{10,9},
					DescriptionPair{11,0},DescriptionPair{11,1},DescriptionPair{11,2},DescriptionPair{11,3},DescriptionPair{11,4},DescriptionPair{11,5},DescriptionPair{11,6},DescriptionPair{11,7},DescriptionPair{11,8},DescriptionPair{11,9},DescriptionPair{11,10},
					DescriptionPair{12,0},DescriptionPair{12,1},DescriptionPair{12,2},DescriptionPair{12,3},DescriptionPair{12,4},DescriptionPair{12,5},DescriptionPair{12,6},DescriptionPair{12,7},DescriptionPair{12,8},DescriptionPair{12,9},DescriptionPair{12,10},DescriptionPair{12,11},
					DescriptionPair{13,0},DescriptionPair{13,1},DescriptionPair{13,2},DescriptionPair{13,3},DescriptionPair{13,4},DescriptionPair{13,5},DescriptionPair{13,6},DescriptionPair{13,7},DescriptionPair{13,8},DescriptionPair{13,9},DescriptionPair{13,10},DescriptionPair{13,11},DescriptionPair{13,12},
					DescriptionPair{14,0},DescriptionPair{14,1},DescriptionPair{14,2},DescriptionPair{14,3},DescriptionPair{14,4},DescriptionPair{14,5},DescriptionPair{14,6},DescriptionPair{14,7},DescriptionPair{14,8},DescriptionPair{14,9},DescriptionPair{14,10},DescriptionPair{14,11},DescriptionPair{14,12},DescriptionPair{14,13},
					DescriptionPair{15,0},DescriptionPair{15,1},DescriptionPair{15,2},DescriptionPair{15,3},DescriptionPair{15,4},DescriptionPair{15,5},DescriptionPair{15,6},DescriptionPair{15,7},DescriptionPair{15,8},DescriptionPair{15,9},DescriptionPair{15,10},DescriptionPair{15,11},DescriptionPair{15,12},DescriptionPair{15,13},DescriptionPair{15,14},
					DescriptionPair{16,0},DescriptionPair{16,1},DescriptionPair{16,2},DescriptionPair{16,3},DescriptionPair{16,4},DescriptionPair{16,5},DescriptionPair{16,6},DescriptionPair{16,7},DescriptionPair{16,8},DescriptionPair{16,9},DescriptionPair{16,10},DescriptionPair{16,11},DescriptionPair{16,12},DescriptionPair{16,13},DescriptionPair{16,14},DescriptionPair{16,15},
					DescriptionPair{17,0},DescriptionPair{17,1},DescriptionPair{17,2},DescriptionPair{17,3},DescriptionPair{17,4},DescriptionPair{17,5},DescriptionPair{17,6},DescriptionPair{17,7},DescriptionPair{17,8},DescriptionPair{17,9},DescriptionPair{17,10},DescriptionPair{17,11},DescriptionPair{17,12},DescriptionPair{17,13},DescriptionPair{17,14},DescriptionPair{17,15},DescriptionPair{17,16},
					DescriptionPair{18,0},DescriptionPair{18,1},DescriptionPair{18,2},DescriptionPair{18,3},DescriptionPair{18,4},DescriptionPair{18,5},DescriptionPair{18,6},DescriptionPair{18,7},DescriptionPair{18,8},DescriptionPair{18,9},DescriptionPair{18,10},DescriptionPair{18,11},DescriptionPair{18,12},DescriptionPair{18,13},DescriptionPair{18,14},DescriptionPair{18,15},DescriptionPair{18,16},DescriptionPair{18,17},
					DescriptionPair{19,0},DescriptionPair{19,1},DescriptionPair{19,2},DescriptionPair{19,3},DescriptionPair{19,4},DescriptionPair{19,5},DescriptionPair{19,6},DescriptionPair{19,7},DescriptionPair{19,8},DescriptionPair{19,9},DescriptionPair{19,10},DescriptionPair{19,11},DescriptionPair{19,12},DescriptionPair{19,13},DescriptionPair{19,14},DescriptionPair{19,15},DescriptionPair{19,16},DescriptionPair{19,17},DescriptionPair{19,18},
					DescriptionPair{20,0},DescriptionPair{20,1},DescriptionPair{20,2},DescriptionPair{20,3},DescriptionPair{20,4},DescriptionPair{20,5},DescriptionPair{20,6},DescriptionPair{20,7},DescriptionPair{20,8},DescriptionPair{20,9},DescriptionPair{20,10},DescriptionPair{20,11},DescriptionPair{20,12},DescriptionPair{20,13},DescriptionPair{20,14},DescriptionPair{20,15},DescriptionPair{20,16},DescriptionPair{20,17},DescriptionPair{20,18},DescriptionPair{20,19},
					DescriptionPair{21,0},DescriptionPair{21,1},DescriptionPair{21,2},DescriptionPair{21,3},DescriptionPair{21,4},DescriptionPair{21,5},DescriptionPair{21,6},DescriptionPair{21,7},DescriptionPair{21,8},DescriptionPair{21,9},DescriptionPair{21,10},DescriptionPair{21,11},DescriptionPair{21,12},DescriptionPair{21,13},DescriptionPair{21,14},DescriptionPair{21,15},DescriptionPair{21,16},DescriptionPair{21,17},DescriptionPair{21,18},DescriptionPair{21,19},DescriptionPair{21,20},
					DescriptionPair{22,0},DescriptionPair{22,1},DescriptionPair{22,2},DescriptionPair{22,3},DescriptionPair{22,4},DescriptionPair{22,5},DescriptionPair{22,6},DescriptionPair{22,7},DescriptionPair{22,8},DescriptionPair{22,9},DescriptionPair{22,10},DescriptionPair{22,11},DescriptionPair{22,12},DescriptionPair{22,13},DescriptionPair{22,14},DescriptionPair{22,15},DescriptionPair{22,16},DescriptionPair{22,17},DescriptionPair{22,18},DescriptionPair{22,19},DescriptionPair{22,20},DescriptionPair{22,21},
					DescriptionPair{23,0},DescriptionPair{23,1},DescriptionPair{23,2},DescriptionPair{23,3},DescriptionPair{23,4},DescriptionPair{23,5},DescriptionPair{23,6},DescriptionPair{23,7},DescriptionPair{23,8},DescriptionPair{23,9},DescriptionPair{23,10},DescriptionPair{23,11},DescriptionPair{23,12},DescriptionPair{23,13},DescriptionPair{23,14},DescriptionPair{23,15},DescriptionPair{23,16},DescriptionPair{23,17},DescriptionPair{23,18},DescriptionPair{23,19},DescriptionPair{23,20},DescriptionPair{23,21},DescriptionPair{23,22},
					DescriptionPair{24,0},DescriptionPair{24,1},DescriptionPair{24,2},DescriptionPair{24,3},DescriptionPair{24,4},DescriptionPair{24,5},DescriptionPair{24,6},DescriptionPair{24,7},DescriptionPair{24,8},DescriptionPair{24,9},DescriptionPair{24,10},DescriptionPair{24,11},DescriptionPair{24,12},DescriptionPair{24,13},DescriptionPair{24,14},DescriptionPair{24,15},DescriptionPair{24,16},DescriptionPair{24,17},DescriptionPair{24,18},DescriptionPair{24,19},DescriptionPair{24,20},DescriptionPair{24,21},DescriptionPair{24,22},DescriptionPair{24,23},
					DescriptionPair{25,0},DescriptionPair{25,1},DescriptionPair{25,2},DescriptionPair{25,3},DescriptionPair{25,4},DescriptionPair{25,5},DescriptionPair{25,6},DescriptionPair{25,7},DescriptionPair{25,8},DescriptionPair{25,9},DescriptionPair{25,10},DescriptionPair{25,11},DescriptionPair{25,12},DescriptionPair{25,13},DescriptionPair{25,14},DescriptionPair{25,15},DescriptionPair{25,16},DescriptionPair{25,17},DescriptionPair{25,18},DescriptionPair{25,19},DescriptionPair{25,20},DescriptionPair{25,21},DescriptionPair{25,22},DescriptionPair{25,23},DescriptionPair{25,24},
					DescriptionPair{26,0},DescriptionPair{26,1},DescriptionPair{26,2},DescriptionPair{26,3},DescriptionPair{26,4},DescriptionPair{26,5},DescriptionPair{26,6},DescriptionPair{26,7},DescriptionPair{26,8},DescriptionPair{26,9},DescriptionPair{26,10},DescriptionPair{26,11},DescriptionPair{26,12},DescriptionPair{26,13},DescriptionPair{26,14},DescriptionPair{26,15},DescriptionPair{26,16},DescriptionPair{26,17},DescriptionPair{26,18},DescriptionPair{26,19},DescriptionPair{26,20},DescriptionPair{26,21},DescriptionPair{26,22},DescriptionPair{26,23},DescriptionPair{26,24},DescriptionPair{26,25},
					DescriptionPair{27,0},DescriptionPair{27,1},DescriptionPair{27,2},DescriptionPair{27,3},DescriptionPair{27,4},DescriptionPair{27,5},DescriptionPair{27,6},DescriptionPair{27,7},DescriptionPair{27,8},DescriptionPair{27,9},DescriptionPair{27,10},DescriptionPair{27,11},DescriptionPair{27,12},DescriptionPair{27,13},DescriptionPair{27,14},DescriptionPair{27,15},DescriptionPair{27,16},DescriptionPair{27,17},DescriptionPair{27,18},DescriptionPair{27,19},DescriptionPair{27,20},DescriptionPair{27,21},DescriptionPair{27,22},DescriptionPair{27,23},DescriptionPair{27,24},DescriptionPair{27,25},DescriptionPair{27,26},
					DescriptionPair{28,0},DescriptionPair{28,1},DescriptionPair{28,2},DescriptionPair{28,3},DescriptionPair{28,4},DescriptionPair{28,5},DescriptionPair{28,6},DescriptionPair{28,7},DescriptionPair{28,8},DescriptionPair{28,9},DescriptionPair{28,10},DescriptionPair{28,11},DescriptionPair{28,12},DescriptionPair{28,13},DescriptionPair{28,14},DescriptionPair{28,15},DescriptionPair{28,16},DescriptionPair{28,17},DescriptionPair{28,18},DescriptionPair{28,19},DescriptionPair{28,20},DescriptionPair{28,21},DescriptionPair{28,22},DescriptionPair{28,23},DescriptionPair{28,24},DescriptionPair{28,25},DescriptionPair{28,26},DescriptionPair{28,27},
					DescriptionPair{29,0},DescriptionPair{29,1},DescriptionPair{29,2},DescriptionPair{29,3},DescriptionPair{29,4},DescriptionPair{29,5},DescriptionPair{29,6},DescriptionPair{29,7},DescriptionPair{29,8},DescriptionPair{29,9},DescriptionPair{29,10},DescriptionPair{29,11},DescriptionPair{29,12},DescriptionPair{29,13},DescriptionPair{29,14},DescriptionPair{29,15},DescriptionPair{29,16},DescriptionPair{29,17},DescriptionPair{29,18},DescriptionPair{29,19},DescriptionPair{29,20},DescriptionPair{29,21},DescriptionPair{29,22},DescriptionPair{29,23},DescriptionPair{29,24},DescriptionPair{29,25},DescriptionPair{29,26},DescriptionPair{29,27},DescriptionPair{29,28},
					DescriptionPair{30,0},DescriptionPair{30,1},DescriptionPair{30,2},DescriptionPair{30,3},DescriptionPair{30,4},DescriptionPair{30,5},DescriptionPair{30,6},DescriptionPair{30,7},DescriptionPair{30,8},DescriptionPair{30,9},DescriptionPair{30,10},DescriptionPair{30,11},DescriptionPair{30,12},DescriptionPair{30,13},DescriptionPair{30,14},DescriptionPair{30,15},DescriptionPair{30,16},DescriptionPair{30,17},DescriptionPair{30,18},DescriptionPair{30,19},DescriptionPair{30,20},DescriptionPair{30,21},DescriptionPair{30,22},DescriptionPair{30,23},DescriptionPair{30,24},DescriptionPair{30,25},DescriptionPair{30,26},DescriptionPair{30,27},DescriptionPair{30,28},DescriptionPair{30,29},
					DescriptionPair{31,0},DescriptionPair{31,1},DescriptionPair{31,2},DescriptionPair{31,3},DescriptionPair{31,4},DescriptionPair{31,5},DescriptionPair{31,6},DescriptionPair{31,7},DescriptionPair{31,8},DescriptionPair{31,9},DescriptionPair{31,10},DescriptionPair{31,11},DescriptionPair{31,12},DescriptionPair{31,13},DescriptionPair{31,14},DescriptionPair{31,15},DescriptionPair{31,16},DescriptionPair{31,17},DescriptionPair{31,18},DescriptionPair{31,19},DescriptionPair{31,20},DescriptionPair{31,21},DescriptionPair{31,22},DescriptionPair{31,23},DescriptionPair{31,24},DescriptionPair{31,25},DescriptionPair{31,26},DescriptionPair{31,27},DescriptionPair{31,28},DescriptionPair{31,29},DescriptionPair{31,30},
					DescriptionPair{32,0},DescriptionPair{32,1},DescriptionPair{32,2},DescriptionPair{32,3},DescriptionPair{32,4},DescriptionPair{32,5},DescriptionPair{32,6},DescriptionPair{32,7},DescriptionPair{32,8},DescriptionPair{32,9},DescriptionPair{32,10},DescriptionPair{32,11},DescriptionPair{32,12},DescriptionPair{32,13},DescriptionPair{32,14},DescriptionPair{32,15},DescriptionPair{32,16},DescriptionPair{32,17},DescriptionPair{32,18},DescriptionPair{32,19},DescriptionPair{32,20},DescriptionPair{32,21},DescriptionPair{32,22},DescriptionPair{32,23},DescriptionPair{32,24},DescriptionPair{32,25},DescriptionPair{32,26},DescriptionPair{32,27},DescriptionPair{32,28},DescriptionPair{32,29},DescriptionPair{32,30},DescriptionPair{32,31},
					DescriptionPair{33,0},DescriptionPair{33,1},DescriptionPair{33,2},DescriptionPair{33,3},DescriptionPair{33,4},DescriptionPair{33,5},DescriptionPair{33,6},DescriptionPair{33,7},DescriptionPair{33,8},DescriptionPair{33,9},DescriptionPair{33,10},DescriptionPair{33,11},DescriptionPair{33,12},DescriptionPair{33,13},DescriptionPair{33,14},DescriptionPair{33,15},DescriptionPair{33,16},DescriptionPair{33,17},DescriptionPair{33,18},DescriptionPair{33,19},DescriptionPair{33,20},DescriptionPair{33,21},DescriptionPair{33,22},DescriptionPair{33,23},DescriptionPair{33,24},DescriptionPair{33,25},DescriptionPair{33,26},DescriptionPair{33,27},DescriptionPair{33,28},DescriptionPair{33,29},DescriptionPair{33,30},DescriptionPair{33,31},DescriptionPair{33,32},
					DescriptionPair{34,0},DescriptionPair{34,1},DescriptionPair{34,2},DescriptionPair{34,3},DescriptionPair{34,4},DescriptionPair{34,5},DescriptionPair{34,6},DescriptionPair{34,7},DescriptionPair{34,8},DescriptionPair{34,9},DescriptionPair{34,10},DescriptionPair{34,11},DescriptionPair{34,12},DescriptionPair{34,13},DescriptionPair{34,14},DescriptionPair{34,15},DescriptionPair{34,16},DescriptionPair{34,17},DescriptionPair{34,18},DescriptionPair{34,19},DescriptionPair{34,20},DescriptionPair{34,21},DescriptionPair{34,22},DescriptionPair{34,23},DescriptionPair{34,24},DescriptionPair{34,25},DescriptionPair{34,26},DescriptionPair{34,27},DescriptionPair{34,28},DescriptionPair{34,29},DescriptionPair{34,30},DescriptionPair{34,31},DescriptionPair{34,32},DescriptionPair{34,33},
					DescriptionPair{35,0},DescriptionPair{35,1},DescriptionPair{35,2},DescriptionPair{35,3},DescriptionPair{35,4},DescriptionPair{35,5},DescriptionPair{35,6},DescriptionPair{35,7},DescriptionPair{35,8},DescriptionPair{35,9},DescriptionPair{35,10},DescriptionPair{35,11},DescriptionPair{35,12},DescriptionPair{35,13},DescriptionPair{35,14},DescriptionPair{35,15},DescriptionPair{35,16},DescriptionPair{35,17},DescriptionPair{35,18},DescriptionPair{35,19},DescriptionPair{35,20},DescriptionPair{35,21},DescriptionPair{35,22},DescriptionPair{35,23},DescriptionPair{35,24},DescriptionPair{35,25},DescriptionPair{35,26},DescriptionPair{35,27},DescriptionPair{35,28},DescriptionPair{35,29},DescriptionPair{35,30},DescriptionPair{35,31},DescriptionPair{35,32},DescriptionPair{35,33},DescriptionPair{35,34},
					DescriptionPair{36,0},DescriptionPair{36,1},DescriptionPair{36,2},DescriptionPair{36,3},DescriptionPair{36,4},DescriptionPair{36,5},DescriptionPair{36,6},DescriptionPair{36,7},DescriptionPair{36,8},DescriptionPair{36,9},DescriptionPair{36,10},DescriptionPair{36,11},DescriptionPair{36,12},DescriptionPair{36,13},DescriptionPair{36,14},DescriptionPair{36,15},DescriptionPair{36,16},DescriptionPair{36,17},DescriptionPair{36,18},DescriptionPair{36,19},DescriptionPair{36,20},DescriptionPair{36,21},DescriptionPair{36,22},DescriptionPair{36,23},DescriptionPair{36,24},DescriptionPair{36,25},DescriptionPair{36,26},DescriptionPair{36,27},DescriptionPair{36,28},DescriptionPair{36,29},DescriptionPair{36,30},DescriptionPair{36,31},DescriptionPair{36,32},DescriptionPair{36,33},DescriptionPair{36,34},DescriptionPair{36,35},
					DescriptionPair{37,0},DescriptionPair{37,1},DescriptionPair{37,2},DescriptionPair{37,3},DescriptionPair{37,4},DescriptionPair{37,5},DescriptionPair{37,6},DescriptionPair{37,7},DescriptionPair{37,8},DescriptionPair{37,9},DescriptionPair{37,10},DescriptionPair{37,11},DescriptionPair{37,12},DescriptionPair{37,13},DescriptionPair{37,14},DescriptionPair{37,15},DescriptionPair{37,16},DescriptionPair{37,17},DescriptionPair{37,18},DescriptionPair{37,19},DescriptionPair{37,20},DescriptionPair{37,21},DescriptionPair{37,22},DescriptionPair{37,23},DescriptionPair{37,24},DescriptionPair{37,25},DescriptionPair{37,26},DescriptionPair{37,27},DescriptionPair{37,28},DescriptionPair{37,29},DescriptionPair{37,30},DescriptionPair{37,31},DescriptionPair{37,32},DescriptionPair{37,33},DescriptionPair{37,34},DescriptionPair{37,35},DescriptionPair{37,36},
					DescriptionPair{38,0},DescriptionPair{38,1},DescriptionPair{38,2},DescriptionPair{38,3},DescriptionPair{38,4},DescriptionPair{38,5},DescriptionPair{38,6},DescriptionPair{38,7},DescriptionPair{38,8},DescriptionPair{38,9},DescriptionPair{38,10},DescriptionPair{38,11},DescriptionPair{38,12},DescriptionPair{38,13},DescriptionPair{38,14},DescriptionPair{38,15},DescriptionPair{38,16},DescriptionPair{38,17},DescriptionPair{38,18},DescriptionPair{38,19},DescriptionPair{38,20},DescriptionPair{38,21},DescriptionPair{38,22},DescriptionPair{38,23},DescriptionPair{38,24},DescriptionPair{38,25},DescriptionPair{38,26},DescriptionPair{38,27},DescriptionPair{38,28},DescriptionPair{38,29},DescriptionPair{38,30},DescriptionPair{38,31},DescriptionPair{38,32},DescriptionPair{38,33},DescriptionPair{38,34},DescriptionPair{38,35},DescriptionPair{38,36},DescriptionPair{38,37},
					DescriptionPair{39,0},DescriptionPair{39,1},DescriptionPair{39,2},DescriptionPair{39,3},DescriptionPair{39,4},DescriptionPair{39,5},DescriptionPair{39,6},DescriptionPair{39,7},DescriptionPair{39,8},DescriptionPair{39,9},DescriptionPair{39,10},DescriptionPair{39,11},DescriptionPair{39,12},DescriptionPair{39,13},DescriptionPair{39,14},DescriptionPair{39,15},DescriptionPair{39,16},DescriptionPair{39,17},DescriptionPair{39,18},DescriptionPair{39,19},DescriptionPair{39,20},DescriptionPair{39,21},DescriptionPair{39,22},DescriptionPair{39,23},DescriptionPair{39,24},DescriptionPair{39,25},DescriptionPair{39,26},DescriptionPair{39,27},DescriptionPair{39,28},DescriptionPair{39,29},DescriptionPair{39,30},DescriptionPair{39,31},DescriptionPair{39,32},DescriptionPair{39,33},DescriptionPair{39,34},DescriptionPair{39,35},DescriptionPair{39,36},DescriptionPair{39,37},DescriptionPair{39,38},
					DescriptionPair{40,0},DescriptionPair{40,1},DescriptionPair{40,2},DescriptionPair{40,3},DescriptionPair{40,4},DescriptionPair{40,5},DescriptionPair{40,6},DescriptionPair{40,7},DescriptionPair{40,8},DescriptionPair{40,9},DescriptionPair{40,10},DescriptionPair{40,11},DescriptionPair{40,12},DescriptionPair{40,13},DescriptionPair{40,14},DescriptionPair{40,15},DescriptionPair{40,16},DescriptionPair{40,17},DescriptionPair{40,18},DescriptionPair{40,19},DescriptionPair{40,20},DescriptionPair{40,21},DescriptionPair{40,22},DescriptionPair{40,23},DescriptionPair{40,24},DescriptionPair{40,25},DescriptionPair{40,26},DescriptionPair{40,27},DescriptionPair{40,28},DescriptionPair{40,29},DescriptionPair{40,30},DescriptionPair{40,31},DescriptionPair{40,32},DescriptionPair{40,33},DescriptionPair{40,34},DescriptionPair{40,35},DescriptionPair{40,36},DescriptionPair{40,37},DescriptionPair{40,38},DescriptionPair{40,39},
					DescriptionPair{41,0},DescriptionPair{41,1},DescriptionPair{41,2},DescriptionPair{41,3},DescriptionPair{41,4},DescriptionPair{41,5},DescriptionPair{41,6},DescriptionPair{41,7},DescriptionPair{41,8},DescriptionPair{41,9},DescriptionPair{41,10},DescriptionPair{41,11},DescriptionPair{41,12},DescriptionPair{41,13},DescriptionPair{41,14},DescriptionPair{41,15},DescriptionPair{41,16},DescriptionPair{41,17},DescriptionPair{41,18},DescriptionPair{41,19},DescriptionPair{41,20},DescriptionPair{41,21},DescriptionPair{41,22},DescriptionPair{41,23},DescriptionPair{41,24},DescriptionPair{41,25},DescriptionPair{41,26},DescriptionPair{41,27},DescriptionPair{41,28},DescriptionPair{41,29},DescriptionPair{41,30},DescriptionPair{41,31},DescriptionPair{41,32},DescriptionPair{41,33},DescriptionPair{41,34},DescriptionPair{41,35},DescriptionPair{41,36},DescriptionPair{41,37},DescriptionPair{41,38},DescriptionPair{41,39},DescriptionPair{41,40},
					DescriptionPair{42,0},DescriptionPair{42,1},DescriptionPair{42,2},DescriptionPair{42,3},DescriptionPair{42,4},DescriptionPair{42,5},DescriptionPair{42,6},DescriptionPair{42,7},DescriptionPair{42,8},DescriptionPair{42,9},DescriptionPair{42,10},DescriptionPair{42,11},DescriptionPair{42,12},DescriptionPair{42,13},DescriptionPair{42,14},DescriptionPair{42,15},DescriptionPair{42,16},DescriptionPair{42,17},DescriptionPair{42,18},DescriptionPair{42,19},DescriptionPair{42,20},DescriptionPair{42,21},DescriptionPair{42,22},DescriptionPair{42,23},DescriptionPair{42,24},DescriptionPair{42,25},DescriptionPair{42,26},DescriptionPair{42,27},DescriptionPair{42,28},DescriptionPair{42,29},DescriptionPair{42,30},DescriptionPair{42,31},DescriptionPair{42,32},DescriptionPair{42,33},DescriptionPair{42,34},DescriptionPair{42,35},DescriptionPair{42,36},DescriptionPair{42,37},DescriptionPair{42,38},DescriptionPair{42,39},DescriptionPair{42,40},DescriptionPair{42,41}

				};

				template<bool orientationNormalized=true,bool scaleNormalized=true,HAL_FLAG hal=HAL_NONE>
				class FreakDescriptorGenerator;

				template<bool orientationNormalized=true,bool scaleNormalized=true,HAL_FLAG hal=HAL_NONE>
				class FreakDescriptorGeneratorFactory;

				template<bool orientationNormalized=true,bool scaleNormalized=true>
				struct PatternLookup
				{

					const float sizeCst;
					PatternPoint patternPoints[NUM_ORIENTATIONS*NUM_SCALES*NUM_POINTS];
					size_t patternSizes[NUM_SCALES];

					PatternLookup(size_t numOctaves,double patternScale):
						sizeCst(static_cast<float>(NUM_SCALES/(LOG2*numOctaves)))
					{
						const double scaleStep=std::pow(2.0,(double)(numOctaves)/NUM_SCALES);
						for(uchar scaleIndex=0;scaleIndex!=NUM_SCALES;++scaleIndex)
						{
							int pSize=0;
							const double scalingFactor=std::pow(scaleStep,scaleIndex);
							for(ushort orientationIndex=0;orientationIndex!=NUM_ORIENTATIONS;++orientationIndex)
							{
								const double theta=double(orientationIndex)*2*CV_PI/double(NUM_ORIENTATIONS);
								uchar pointIndex=0;
								for(uchar i=0;i!=8;++i)
								{
									const double radius=RADIUS[i];
									const double sigma=SIGMA[i];
									const float scaledSigma=static_cast<float>(sigma*scalingFactor*patternScale);
									const uchar numPointsInCircle=NUM_POINTS_IN_CIRCLE[i];
									const double orientationOffset=CV_PI/double(numPointsInCircle);
									const double beta=orientationOffset*(i%2)+theta;
									const int sizeMax=static_cast<int>(ceil((radius+sigma)*scalingFactor*patternScale))+1;
									if(pSize<sizeMax)
									{
										pSize=sizeMax;
									}
									for(uchar j=0;j!=numPointsInCircle;++j,++pointIndex)
									{
										const double alpha=double(j)*2*orientationOffset+beta;
										PatternPoint& point=patternPoints[scaleIndex*NUM_ORIENTATIONS*NUM_POINTS+orientationIndex*NUM_POINTS+pointIndex];
										point.x=static_cast<float>(radius*cos(alpha)*scalingFactor*patternScale);
										point.y=static_cast<float>(radius*sin(alpha)*scalingFactor*patternScale);
										point.sigma=scaledSigma;
									}
								}
							}
							patternSizes[scaleIndex]=(size_t)pSize;
						}
//#ifdef _DEBUG
//
//						for(size_t i=0;i<NUM_POINTS*2;++i)
//						{
//							std::cout<<"patternPoints["<<i<<"]="<<patternPoints[i]<<std::endl;
//						}
//						for(size_t i=0;i<NUM_SCALES;++i)
//						{
//							std::cout<<"patternSize["<<i<<"] = "<<patternSizes[i]<<std::endl;
//						}
//						std::cout<<"sizeCst="<<sizeCst<<std::endl;
//#endif

					}

					template<HAL_FLAG hal>
					uchar extractPointValue(const matrix::Ptr2D<uchar>& frame,const matrix::integral::IntegralMat<hal>* integralMat,const cv::Point2f& pt,const size_t pointIndex,const size_t scaleIndex,const size_t orientationIndex=0) const
					{
						const PatternPoint& patternPoint=patternPoints[(scaleIndex*NUM_ORIENTATIONS*NUM_POINTS)+(orientationIndex*NUM_POINTS)+pointIndex];
						const float xf=patternPoint.x+pt.x;
						const float yf=patternPoint.y+pt.y;
						const float radius=patternPoint.sigma;
						if(radius<0.5)
						{
							return meanIntensity(xf,yf,frame);
						}
						return integralMat->meanIntensity(xf,yf,radius);

					}
				};
				
				template<> struct PatternLookup<true,false>
				{
					PatternPoint patternPoints[NUM_ORIENTATIONS*NUM_POINTS];
					size_t patternSize;

					PatternLookup(size_t numOctaves,double patternScale)
					{
						const size_t scaleIndex=std::min<size_t>(NUM_SCALES-1,(size_t)(1.0986122886681*static_cast<float>(NUM_SCALES/(LOG2*numOctaves))+0.5));
						int pSize=0;
						const double scalingFactor=std::pow(std::pow(2.0,(double)(numOctaves)/NUM_SCALES),scaleIndex);
						for(ushort orientationIndex=0;orientationIndex!=NUM_ORIENTATIONS;++orientationIndex)
						{
							const double theta=double(orientationIndex)*2*CV_PI/double(NUM_ORIENTATIONS);
							uchar pointIndex=0;
							for(uchar i=0;i!=8;++i)
							{
								const double radius=RADIUS[i];
								const double sigma=SIGMA[i];
								const float scaledSigma=static_cast<float>(sigma*scalingFactor*patternScale);
								const uchar numPointsInCircle=NUM_POINTS_IN_CIRCLE[i];
								const double orientationOffset=CV_PI/double(numPointsInCircle);
								const double beta=orientationOffset*(i%2)+theta;
								const int sizeMax=static_cast<int>(ceil((radius+sigma)*scalingFactor*patternScale))+1;
								if(pSize<sizeMax)
								{
									pSize=sizeMax;
								}
								for(uchar j=0;j!=numPointsInCircle;++j,++pointIndex)
								{
									const double alpha=double(j)*2*orientationOffset+beta;
									PatternPoint& point=patternPoints[orientationIndex*NUM_POINTS+pointIndex];
									point.x=static_cast<float>(radius*cos(alpha)*scalingFactor*patternScale);
									point.y=static_cast<float>(radius*sin(alpha)*scalingFactor*patternScale);
									point.sigma=scaledSigma;
								}
							}
						}
						patternSize=(size_t)pSize;
					}
				
					template<HAL_FLAG hal>
					uchar extractPointValue(const matrix::Ptr2D<uchar>& frame,const matrix::integral::IntegralMat<hal>* integralMat,const cv::Point2f& pt,const size_t pointIndex,const size_t orientationIndex=0) const
					{
						const PatternPoint& patternPoint=patternPoints[(orientationIndex*NUM_POINTS)+pointIndex];
						const float xf=patternPoint.x+pt.x;
						const float yf=patternPoint.y+pt.y;
						const float radius=patternPoint.sigma;
						if(radius<0.5)
						{
							return meanIntensity(xf,yf,frame);
						}
						return integralMat->meanIntensity(xf,yf,radius);

					}

				};
				
				template<> struct PatternLookup<false,true>
				{
					const float sizeCst;
					PatternPoint patternPoints[NUM_SCALES*NUM_POINTS];
					size_t patternSizes[NUM_SCALES];

					PatternLookup(size_t numOctaves,double patternScale):
						sizeCst(static_cast<float>(NUM_SCALES/(LOG2*numOctaves)))
					{
						const double scaleStep=std::pow(2.0,(double)(numOctaves)/NUM_SCALES);
						for(uchar scaleIndex=0;scaleIndex!=NUM_SCALES;++scaleIndex)
						{
							int pSize=0;
							const double scalingFactor=std::pow(scaleStep,scaleIndex);
							uchar pointIndex=0;
							for(uchar i=0;i!=8;++i)
							{
								const double radius=RADIUS[i];
								const double sigma=SIGMA[i];
								const float scaledSigma=static_cast<float>(sigma*scalingFactor*patternScale);
								const uchar numPointsInCircle=NUM_POINTS_IN_CIRCLE[i];
								const double orientationOffset=CV_PI/double(numPointsInCircle);
								const double beta=orientationOffset*(i%2);
								const int sizeMax=static_cast<int>(ceil((radius+sigma)*scalingFactor*patternScale))+1;
								if(pSize<sizeMax)
								{
									pSize=sizeMax;
								}
								for(uchar j=0;j!=numPointsInCircle;++j,++pointIndex)
								{
									const double alpha=double(j)*2*orientationOffset+beta;
									PatternPoint& point=patternPoints[scaleIndex*NUM_POINTS+pointIndex];
									point.x=static_cast<float>(radius*cos(alpha)*scalingFactor*patternScale);
									point.y=static_cast<float>(radius*sin(alpha)*scalingFactor*patternScale);
									point.sigma=scaledSigma;
								}
							}
							patternSizes[scaleIndex]=(size_t)pSize;
						}


					}
					
					template<HAL_FLAG hal>
					uchar extractPointValue(const matrix::Ptr2D<uchar>& frame,const matrix::integral::IntegralMat<hal>* integralMat,const cv::Point2f& pt,const size_t pointIndex,const size_t scaleIndex) const
					{
						const PatternPoint& patternPoint=patternPoints[(scaleIndex*NUM_POINTS)+pointIndex];
						const float xf=patternPoint.x+pt.x;
						const float yf=patternPoint.y+pt.y;
						const float radius=patternPoint.sigma;
						if(radius<0.5)
						{
							return meanIntensity(xf,yf,frame);
						}
						return integralMat->meanIntensity(xf,yf,radius);

					}
				};
				
				template<> struct PatternLookup<false,false>
				{
					PatternPoint patternPoints[NUM_POINTS];
					size_t patternSize;

					PatternLookup(size_t numOctaves,double patternScale)
					{
						const size_t scaleIndex=std::min<size_t>(NUM_SCALES-1,(size_t)(1.0986122886681*static_cast<float>(NUM_SCALES/(LOG2*numOctaves))+0.5));
						int pSize=0;
						const double scalingFactor=std::pow(std::pow(2.0,(double)(numOctaves)/NUM_SCALES),scaleIndex);
						
						uchar pointIndex=0;
						for(uchar i=0;i!=8;++i)
						{
							const double radius=RADIUS[i];
							const double sigma=SIGMA[i];
							const float scaledSigma=static_cast<float>(sigma*scalingFactor*patternScale);
							const uchar numPointsInCircle=NUM_POINTS_IN_CIRCLE[i];
							const double orientationOffset=CV_PI/double(numPointsInCircle);
							const double beta=orientationOffset*(i%2);
							const int sizeMax=static_cast<int>(ceil((radius+sigma)*scalingFactor*patternScale))+1;
							if(pSize<sizeMax)
							{
								pSize=sizeMax;
							}
							for(uchar j=0;j!=numPointsInCircle;++j,++pointIndex)
							{
								const double alpha=double(j)*2*orientationOffset+beta;
								PatternPoint& point=patternPoints[pointIndex];
								point.x=static_cast<float>(radius*cos(alpha)*scalingFactor*patternScale);
								point.y=static_cast<float>(radius*sin(alpha)*scalingFactor*patternScale);
								point.sigma=scaledSigma;
							}
						}
						
						patternSize=(size_t)pSize;
					}
				
					template<HAL_FLAG hal>
					uchar extractPointValue(const matrix::Ptr2D<uchar>& frame,const matrix::integral::IntegralMat<hal>* integralMat,const cv::Point2f& pt,const size_t pointIndex) const
					{
						const PatternPoint& patternPoint=patternPoints[pointIndex];
						const float xf=patternPoint.x+pt.x;
						const float yf=patternPoint.y+pt.y;
						const float radius=patternPoint.sigma;
						if(radius<0.5)
						{
							return meanIntensity(xf,yf,frame);
						}
						return integralMat->meanIntensity(xf,yf,radius);

					}
				
				};

				struct OrientationLookup
				{
					OrientationPair orientationPairs[NUM_ORIENTATION_PAIRS];

					constexpr void initializePairs()
					{
						orientationPairs[0].i=0; orientationPairs[0].j=3; orientationPairs[1].i=1; orientationPairs[1].j=4; orientationPairs[2].i=2; orientationPairs[2].j=5;
						orientationPairs[3].i=0; orientationPairs[3].j=2; orientationPairs[4].i=1; orientationPairs[4].j=3; orientationPairs[5].i=2; orientationPairs[5].j=4;
						orientationPairs[6].i=3; orientationPairs[6].j=5; orientationPairs[7].i=4; orientationPairs[7].j=0; orientationPairs[8].i=5; orientationPairs[8].j=1;

						orientationPairs[9].i=6; orientationPairs[9].j=9; orientationPairs[10].i=7; orientationPairs[10].j=10; orientationPairs[11].i=8; orientationPairs[11].j=11;
						orientationPairs[12].i=6; orientationPairs[12].j=8; orientationPairs[13].i=7; orientationPairs[13].j=9; orientationPairs[14].i=8; orientationPairs[14].j=10;
						orientationPairs[15].i=9; orientationPairs[15].j=11; orientationPairs[16].i=10; orientationPairs[16].j=6; orientationPairs[17].i=11; orientationPairs[17].j=7;

						orientationPairs[18].i=12; orientationPairs[18].j=15; orientationPairs[19].i=13; orientationPairs[19].j=16; orientationPairs[20].i=14; orientationPairs[20].j=17;
						orientationPairs[21].i=12; orientationPairs[21].j=14; orientationPairs[22].i=13; orientationPairs[22].j=15; orientationPairs[23].i=14; orientationPairs[23].j=16;
						orientationPairs[24].i=15; orientationPairs[24].j=17; orientationPairs[25].i=16; orientationPairs[25].j=12; orientationPairs[26].i=17; orientationPairs[26].j=13;

						orientationPairs[27].i=18; orientationPairs[27].j=21; orientationPairs[28].i=19; orientationPairs[28].j=22; orientationPairs[29].i=20; orientationPairs[29].j=23;
						orientationPairs[30].i=18; orientationPairs[30].j=20; orientationPairs[31].i=19; orientationPairs[31].j=21; orientationPairs[32].i=20; orientationPairs[32].j=22;
						orientationPairs[33].i=21; orientationPairs[33].j=23; orientationPairs[34].i=22; orientationPairs[34].j=18; orientationPairs[35].i=23; orientationPairs[35].j=19;

						orientationPairs[36].i=24; orientationPairs[36].j=27; orientationPairs[37].i=25; orientationPairs[37].j=28; orientationPairs[38].i=26; orientationPairs[38].j=29;
						orientationPairs[39].i=30; orientationPairs[39].j=33; orientationPairs[40].i=31; orientationPairs[40].j=34; orientationPairs[41].i=32; orientationPairs[41].j=35;
						orientationPairs[42].i=36; orientationPairs[42].j=39; orientationPairs[43].i=37; orientationPairs[43].j=40; orientationPairs[44].i=38; orientationPairs[44].j=41;
					}

					static std::pair<float,float> getUnscaledPatternPoint(const double patternScale,const uchar i)
					{
						const uchar nCircle=i/6;
						const uchar nPointsInCircle=NUM_POINTS_IN_CIRCLE[nCircle];
						const double radius=RADIUS[nCircle];
						const double alpha=double(i%6)*2*CV_PI/double(nPointsInCircle)+((CV_PI/nPointsInCircle)*(nCircle%2));
						return std::pair<float,float>(static_cast<float>(radius*cos(alpha)*patternScale),static_cast<float>(radius*sin(alpha)*patternScale));
					}


					OrientationLookup(const double patternScale)
					{
						initializePairs();
						for(unsigned m=NUM_ORIENTATION_PAIRS; m--; )
						{
							OrientationPair& pair=orientationPairs[m];
							const std::pair<float,float> pp1=getUnscaledPatternPoint(patternScale,pair.i);
							const std::pair<float,float> pp2=getUnscaledPatternPoint(patternScale,pair.j);
							const float dx=pp1.first-pp2.first;
							const float dy=pp1.second-pp2.second;
							const float norm_sq=(dx*dx+dy*dy);
							pair.weightX=int((dx/(norm_sq))*4096.0+0.5);
							pair.weightY=int((dy/(norm_sq))*4096.0+0.5);
						}
//#ifdef _DEBUG
//
//						for(size_t i=0;i<NUM_ORIENTATION_PAIRS;++i)
//						{
//							std::cout<<"orientationPairs["<<i<<"]="<<orientationPairs[i]<<std::endl;
//						}
//#endif
					}

					OrientationLookup(const PatternLookup<true,true>& patternLookup)
					{
						initializePairs();
						for(unsigned m=NUM_ORIENTATION_PAIRS; m--; )
						{
							OrientationPair& pair=orientationPairs[m];
							const PatternPoint& pp1=patternLookup.patternPoints[pair.i];
							const PatternPoint& pp2=patternLookup.patternPoints[pair.j];
							const float dx=pp1.x-pp2.x;
							const float dy=pp1.y-pp2.y;
							const float norm_sq=(dx*dx+dy*dy);
							pair.weightX=int((dx/(norm_sq))*4096.0+0.5);
							pair.weightY=int((dy/(norm_sq))*4096.0+0.5);
						}

//#ifdef _DEBUG
//
//						for(size_t i=0;i<NUM_ORIENTATION_PAIRS;++i)
//						{
//							std::cout<<"orientationPairs["<<i<<"]="<<orientationPairs[i]<<std::endl;
//						}
//#endif
					}

					template<HAL_FLAG hal>
					size_t extractPointValues(const matrix::Ptr2D<uchar>& frame,const matrix::integral::IntegralMat<hal>* integralMat,const cv::Point2f& pt,const PatternLookup<true,true>& patternLookup,const size_t scaleIndex) const
					{
						uchar pointsValue[NUM_POINTS];
						for(uchar i=NUM_POINTS;i--;)
						{
							pointsValue[i]=patternLookup.extractPointValue(frame,integralMat,pt,i,scaleIndex);
						}
						int direction0=0;
						int direction1=0;
						for(ushort m=NUM_ORIENTATION_PAIRS;m--;)
						{
							const OrientationPair& pair=orientationPairs[m];
							const int delta=(pointsValue[pair.i]-pointsValue[pair.j]);
							direction0+=delta*(pair.weightX)/2048;
							direction1+=delta*(pair.weightY)/2048;
						}
						const float angle=static_cast<float>(atan2((float)direction1,(float)direction0)*(180.0/CV_PI));
						int thetaIndex;
						if(angle<0.f)
						{
							thetaIndex=int(NUM_ORIENTATIONS*angle*(1/360.0)-0.5);
						}
						else
						{
							thetaIndex=int(NUM_ORIENTATIONS*angle*(1/360.0)+0.5);
						}
						if(thetaIndex<0)
						{
							return size_t(thetaIndex+NUM_ORIENTATIONS);
						}
						if(thetaIndex>=NUM_ORIENTATIONS)
						{
							return size_t(thetaIndex-NUM_ORIENTATIONS);
						}
						return size_t(thetaIndex);
					}


					template<HAL_FLAG hal>
					size_t extractPointValues(const matrix::Ptr2D<uchar>& frame,const matrix::integral::IntegralMat<hal>* integralMat,const cv::Point2f& pt,const PatternLookup<true,false>& patternLookup) const
					{
						uchar pointsValue[NUM_POINTS];
						for(uchar i=NUM_POINTS;i--;)
						{
							pointsValue[i]=patternLookup.extractPointValue(frame,integralMat,pt,i);
						}
						int direction0=0;
						int direction1=0;
						for(ushort m=NUM_ORIENTATION_PAIRS;m--;)
						{
							const OrientationPair& pair=orientationPairs[m];
							const int delta=(pointsValue[pair.i]-pointsValue[pair.j]);
							direction0+=delta*(pair.weightX)/2048;
							direction1+=delta*(pair.weightY)/2048;
						}
						const float angle=static_cast<float>(atan2((float)direction1,(float)direction0)*(180.0/CV_PI));
						int thetaIndex;
						if(angle<0.f)
						{
							thetaIndex=int(NUM_ORIENTATIONS*angle*(1/360.0)-0.5);
						}
						else
						{
							thetaIndex=int(NUM_ORIENTATIONS*angle*(1/360.0)+0.5);
						}
						if(thetaIndex<0)
						{
							return size_t(thetaIndex+NUM_ORIENTATIONS);
						}
						if(thetaIndex>=NUM_ORIENTATIONS)
						{
							return size_t(thetaIndex-NUM_ORIENTATIONS);
						}
						return size_t(thetaIndex);
					}
				};

				struct DescriptionLookup
				{
					ushort selectedPairs[NUM_PAIRS];
					DescriptionPair descriptionPairs[NUM_PAIRS];
					DescriptionLookup(const ushort(&selectedPairs)[NUM_PAIRS]=DEFAULT_PAIRS)
					{
						std::memcpy(this->selectedPairs,selectedPairs,sizeof(ushort)*NUM_PAIRS);
						for(ushort i=0;i!=NUM_PAIRS;++i)
						{
							descriptionPairs[i]=ALL_PAIRS[selectedPairs[i]];
						}
					}
				
					template<HAL_FLAG hal>
					std::enable_if_t<!HalFlagInfo<hal>::hasSSE2> extractDescriptor(const uchar(&pointsValue)[NUM_POINTS],uchar** ptr) const
					{
						std::bitset<NUM_PAIRS>** ptrScalar=(std::bitset<NUM_PAIRS>**)ptr;
						unsigned int count=0;
						for(int n=7;n<NUM_PAIRS;n+=128)
						{
							for(int m=8;m--;)
							{
								int nm=n-m;
								for(int kk=nm+15*8;kk>=nm;kk-=8,++count)
								{
									(*ptrScalar)->set(kk,pointsValue[descriptionPairs[count].i]>=pointsValue[descriptionPairs[count].j]);
								}
							}
						}
						--(*ptrScalar);
					}

					template<HAL_FLAG hal>
					std::enable_if_t<HalFlagInfo<hal>::hasSSE2> extractDescriptor(const uchar(&pointsValue)[NUM_POINTS],uchar** ptr) const
					{
						__m128i** ptrSSE=(__m128i**) ptr;

						unsigned int count=0;
						for(int n=NUM_PAIRS/128; n--; )
						{
							__m128i result128=_mm_setzero_si128();
							for(int m=128/16; m--; count+=16)
							{
								__m128i operand1=_mm_set_epi8(pointsValue[descriptionPairs[count+0].i],
															  pointsValue[descriptionPairs[count+1].i],
															  pointsValue[descriptionPairs[count+2].i],
															  pointsValue[descriptionPairs[count+3].i],
															  pointsValue[descriptionPairs[count+4].i],
															  pointsValue[descriptionPairs[count+5].i],
															  pointsValue[descriptionPairs[count+6].i],
															  pointsValue[descriptionPairs[count+7].i],
															  pointsValue[descriptionPairs[count+8].i],
															  pointsValue[descriptionPairs[count+9].i],
															  pointsValue[descriptionPairs[count+10].i],
															  pointsValue[descriptionPairs[count+11].i],
															  pointsValue[descriptionPairs[count+12].i],
															  pointsValue[descriptionPairs[count+13].i],
															  pointsValue[descriptionPairs[count+14].i],
															  pointsValue[descriptionPairs[count+15].i]);

								__m128i operand2=_mm_set_epi8(pointsValue[descriptionPairs[count+0].j],
															  pointsValue[descriptionPairs[count+1].j],
															  pointsValue[descriptionPairs[count+2].j],
															  pointsValue[descriptionPairs[count+3].j],
															  pointsValue[descriptionPairs[count+4].j],
															  pointsValue[descriptionPairs[count+5].j],
															  pointsValue[descriptionPairs[count+6].j],
															  pointsValue[descriptionPairs[count+7].j],
															  pointsValue[descriptionPairs[count+8].j],
															  pointsValue[descriptionPairs[count+9].j],
															  pointsValue[descriptionPairs[count+10].j],
															  pointsValue[descriptionPairs[count+11].j],
															  pointsValue[descriptionPairs[count+12].j],
															  pointsValue[descriptionPairs[count+13].j],
															  pointsValue[descriptionPairs[count+14].j],
															  pointsValue[descriptionPairs[count+15].j]);

								__m128i workReg=_mm_min_epu8(operand1,operand2); // emulated "not less than" for 8-bit UNSIGNED integers
								workReg=_mm_cmpeq_epi8(workReg,operand2);        // emulated "not less than" for 8-bit UNSIGNED integers

								workReg=_mm_and_si128(_mm_set1_epi16(short(0x8080>>m)),workReg); // merge the last 16 bits with the 128bits std::vector until full
								result128=_mm_or_si128(result128,workReg);
							}
							(**ptrSSE)=result128;
							++(*ptrSSE);
						}
						(*ptrSSE)-=8;
					}

				};

				template<bool scaleNormalized,HAL_FLAG hal>
				class FreakDescriptorGeneratorFactory<true,scaleNormalized,hal> : public DescriptorGeneratorFactory<Freak>
				{
				private:
					friend class FreakDescriptorGenerator<true,scaleNormalized,hal>;
					const size_t numOctaves;
					const double patternScale;
					const PatternLookup<true,scaleNormalized> patternLookup;
					const OrientationLookup orientationLookup;
					const DescriptionLookup descriptionLookup;

					template<bool implScaleNormalized=scaleNormalized>
					std::enable_if_t<implScaleNormalized>
						extractDescriptor(DescTypeInfo<Freak>::Elem** descriptorData,const cv::KeyPoint& kp,const matrix::Ptr2D<uchar>& frame,const matrix::integral::IntegralMat<hal>* integral,const size_t scaleIndex) const
					{
						const size_t thetaIndex=orientationLookup.extractPointValues(frame,integral,kp.pt,patternLookup,scaleIndex);
						uchar pointsValue[NUM_POINTS];
						for(size_t i=NUM_POINTS;i--;)
						{
							pointsValue[i]=patternLookup.extractPointValue(frame,integral,kp.pt,i,scaleIndex,thetaIndex);
						}

						descriptionLookup.extractDescriptor<hal>(pointsValue,descriptorData);
					}
					template<bool implScaleNormalized=scaleNormalized>
					std::enable_if_t<!implScaleNormalized>
						extractDescriptor(DescTypeInfo<Freak>::Elem** descriptorData,const cv::KeyPoint& kp,const matrix::Ptr2D<uchar>& frame,const matrix::integral::IntegralMat<hal>* integral) const
					{
						
						const size_t thetaIndex=orientationLookup.extractPointValues(frame,integral,kp.pt,patternLookup);
						uchar pointsValue[NUM_POINTS];
						for(size_t i=NUM_POINTS;i--;)
						{
							pointsValue[i]=patternLookup.extractPointValue(frame,integral,kp.pt,i,thetaIndex);
						}

						descriptionLookup.extractDescriptor<hal>(pointsValue,descriptorData);
					}
					template<bool implScaleNormalized=scaleNormalized>
					std::enable_if_t<implScaleNormalized,OrientationLookup> initializeOrientationLookup()
					{
						return OrientationLookup(patternLookup);
					}

					template<bool implScaleNormalized=scaleNormalized>
					std::enable_if_t<!implScaleNormalized,OrientationLookup> initializeOrientationLookup()
					{
						return OrientationLookup(patternScale);
					}

				public:
					FreakDescriptorGeneratorFactory(size_t numOctaves,float patternScale,const ushort(&selectedPairs)[NUM_PAIRS]=DEFAULT_PAIRS):
						numOctaves(numOctaves),
						patternScale(patternScale),
						patternLookup(numOctaves,patternScale),
						orientationLookup(initializeOrientationLookup()),
						descriptionLookup(selectedPairs) //TODO, eventually support custom pairs
					{

					}

					FreakDescriptorGenerator<true,scaleNormalized,hal>* getDescriptorGenerator(size_t cols,size_t rows) const override;
						
					void writeProperties(boost::property_tree::ptree& properties) const override
					{
						properties.put<std::string>("descriptorType","freak");
						properties.put<bool>("orientationNormalized",true);
						properties.put<bool>("scaleNormalized",scaleNormalized);
						properties.put<float>("patternScale",(float)patternScale);
						properties.put<size_t>("numOctaves",numOctaves);
						for(size_t i=0;i!=NUM_PAIRS;++i)
						{
							properties.add<ushort>("selectedPairs",descriptionLookup.selectedPairs[i]);
						}
					}
				};

				template<bool scaleNormalized,HAL_FLAG hal>
				class FreakDescriptorGeneratorFactory<false,scaleNormalized,hal>: public DescriptorGeneratorFactory<Freak>
				{
				private:
					friend class FreakDescriptorGenerator<false,scaleNormalized,hal>;
					const size_t numOctaves;
					const double patternScale;
					const PatternLookup<false,scaleNormalized> patternLookup;
					const DescriptionLookup descriptionLookup;

					template<bool implScaleNormalized=scaleNormalized>
					std::enable_if_t<implScaleNormalized> 
						extractDescriptor(DescTypeInfo<Freak>::Elem** descriptorData,const cv::KeyPoint& kp,const matrix::Ptr2D<uchar>& frame,const matrix::integral::IntegralMat<hal>* integral,const size_t scaleIndex) const
					{
						uchar pointsValue[NUM_POINTS];
						for(size_t i=NUM_POINTS;i--;)
						{
							pointsValue[i]=patternLookup.extractPointValue(frame,integral,kp.pt,i,scaleIndex);
						}
						descriptionLookup.extractDescriptor<hal>(pointsValue,descriptorData);
					}
					template<bool implScaleNormalized=scaleNormalized>
					std::enable_if_t<!implScaleNormalized>
						extractDescriptor(DescTypeInfo<Freak>::Elem** descriptorData,const cv::KeyPoint& kp,const matrix::Ptr2D<uchar>& frame,const matrix::integral::IntegralMat<hal>* integral) const
					{
						uchar pointsValue[NUM_POINTS];
						for(size_t i=NUM_POINTS;i--;)
						{
							pointsValue[i]=patternLookup.extractPointValue(frame,integral,kp.pt,i);
						}
						descriptionLookup.extractDescriptor<hal>(pointsValue,descriptorData);
					}

				public:
					FreakDescriptorGeneratorFactory(size_t numOctaves,float patternScale,const ushort(&selectedPairs)[NUM_PAIRS]=DEFAULT_PAIRS):
						numOctaves(numOctaves),
						patternScale(patternScale),
						patternLookup(numOctaves,patternScale),
						descriptionLookup(selectedPairs) //TODO, eventually support custom pairs
					{

					}

					FreakDescriptorGenerator<false,scaleNormalized,hal>* getDescriptorGenerator(size_t cols,size_t rows) const override;

					void writeProperties(boost::property_tree::ptree& properties) const override
					{
						properties.put<std::string>("descriptorType","freak");
						properties.put<bool>("orientationNormalized",false);
						properties.put<bool>("scaleNormalized",scaleNormalized);
						properties.put<float>("patternScale",(float)patternScale);
						properties.put<size_t>("numOctaves",numOctaves);
						for(size_t i=0;i!=NUM_PAIRS;++i)
						{
							properties.add<ushort>("selectedPairs",descriptionLookup.selectedPairs[i]);
						}
					}
				};

				template<HAL_FLAG hal>
				matrix::integral::IntegralMat<hal>* const initializeIntegral(const size_t cols,const size_t rows)
				{
					if(cols*rows<8388608)
					{
						return new matrix::integral::IntegralMatImpl<hal,int>(cols,rows);
					}
					return new matrix::integral::IntegralMatImpl<hal,double>(cols,rows);
				}

				std::ostream& operator<<(std::ostream& stream,const cv::KeyPoint& kp)
				{
					return stream<<"KeyPoint{x="<<kp.pt.x<<"; y="<<kp.pt.y<<"; response="<<kp.response<<"; angle="<<kp.angle<<"; size="<<kp.size<<"}";
				}

				template<bool orientationNormalized,HAL_FLAG hal>
				class FreakDescriptorGenerator<orientationNormalized,true,hal> : public DescriptorGenerator<Freak>
				{
				private:
					const size_t cols;
					const size_t rows;
					matrix::integral::IntegralMat<hal>* const integralMat;
					std::vector<size_t> kpScaleIndices;
					uchar pointsValue[NUM_POINTS];
					const FreakDescriptorGeneratorFactory<orientationNormalized,true,hal>* const factory;

					void removeBorderKeyPoints(std::vector<cv::KeyPoint>& keyPoints)
					{
						const PatternLookup<orientationNormalized,true>& patternLookup=factory->patternLookup;
						const size_t(&patternSizes)[NUM_SCALES]=patternLookup.patternSizes;
						const float sizeCst=patternLookup.sizeCst;
						const size_t numKeyPoints=keyPoints.size();
						kpScaleIndices.reserve(numKeyPoints);
						const std::vector<cv::KeyPoint>::iterator kpBegin=keyPoints.begin();
						for(size_t k=numKeyPoints;k--;)
						{
							const cv::KeyPoint& kp=keyPoints[k];
							const size_t kpScaleIndex=std::min((size_t)NUM_SCALES-1,(size_t)std::max((int)(std::log(kp.size/SMALLEST_KP_SIZE)*sizeCst+0.5),0));
							const size_t patternSize=patternSizes[kpScaleIndex];
							const cv::Point2f& pt=kp.pt;
							const float kpX=pt.x;
							const float kpY=pt.y;
							if(kpX<=patternSize||kpY<=patternSize||kpX>=(cols-patternSize)||kpY>=(rows-patternSize))
							{
								keyPoints.erase(kpBegin+k);
							}
							else
							{
								kpScaleIndices.push_back(kpScaleIndex);
							}
						}
					}

				public:
					FreakDescriptorGenerator(size_t cols,size_t rows,const FreakDescriptorGeneratorFactory<orientationNormalized,true,hal>*  factory):
						cols(cols),
						rows(rows),
						integralMat(initializeIntegral<hal>(cols,rows)),
						factory(factory)
					{

					}

					typename DescTypeInfo<Freak>::DescriptorMat compute(const matrix::Ptr2D<uchar>& frame,std::vector<cv::KeyPoint>& keyPoints) override
					{
						kpScaleIndices.clear();
						integralMat->calculateIntegral(frame);
						removeBorderKeyPoints(keyPoints);
						const size_t numKeyPoints=keyPoints.size();						
						if(numKeyPoints)
						{
							typename DescTypeInfo<Freak>::DescriptorMat descriptors(numKeyPoints);
							DescTypeInfo<Freak>::Elem* ptr=descriptors.getData(numKeyPoints-1);
							for(size_t k=numKeyPoints;k--;)
							{
								factory->extractDescriptor(&ptr,keyPoints[k],frame,integralMat,kpScaleIndices[numKeyPoints-k-1]);
							}
							return descriptors;
						}
						return typename DescTypeInfo<Freak>::DescriptorMat();
					}

					~FreakDescriptorGenerator()
					{
						delete integralMat;
					}
				};

				template<bool orientationNormalized,HAL_FLAG hal>
				class FreakDescriptorGenerator<orientationNormalized,false,hal>: public DescriptorGenerator<Freak>
				{
				private:
					const size_t cols;
					const size_t rows;
					matrix::integral::IntegralMat<hal>* const integralMat;
					uchar pointsValue[NUM_POINTS];
					const FreakDescriptorGeneratorFactory<orientationNormalized,false,hal>* const factory;

					void removeBorderKeyPoints(std::vector<cv::KeyPoint>& keyPoints)
					{

						const size_t numKeyPoints=keyPoints.size();
						const std::vector<cv::KeyPoint>::iterator kpBegin=keyPoints.begin();
						const size_t patternSize=factory->patternLookup.patternSize;
						for(size_t k=numKeyPoints;k--;)
						{
							const cv::KeyPoint& kp=keyPoints[k];
							const cv::Point2f& pt=kp.pt;
							const float kpX=pt.x;
							const float kpY=pt.y;
							if(kpX<=patternSize||kpY<=patternSize||kpX>=(cols-patternSize)||kpY>=(rows-patternSize))
							{
								keyPoints.erase(kpBegin+k);
							}
						}
						
					}

				public:
					FreakDescriptorGenerator(size_t cols,size_t rows,const FreakDescriptorGeneratorFactory<orientationNormalized,false,hal>*  factory):
						cols(cols),
						rows(rows),
						integralMat(initializeIntegral<hal>(cols,rows)),
						factory(factory)
					{

					}
					typename DescTypeInfo<Freak>::DescriptorMat compute(const matrix::Ptr2D<uchar>& frame,std::vector<cv::KeyPoint>& keyPoints) override
					{
						integralMat->calculateIntegral(frame);
						removeBorderKeyPoints(keyPoints);
						const size_t numKeyPoints=keyPoints.size();
						if(numKeyPoints)
						{
							typename DescTypeInfo<Freak>::DescriptorMat descriptors(numKeyPoints);
							DescTypeInfo<Freak>::Elem* ptr=descriptors.getData(numKeyPoints-1);
							for(size_t k=numKeyPoints;k--;)
							{
								factory->extractDescriptor(&ptr,keyPoints[k],frame,integralMat);
							}
							return descriptors;
						}
						return typename DescTypeInfo<Freak>::DescriptorMat();
					}

					~FreakDescriptorGenerator()
					{
						delete integralMat;
					}
				};
			
				template<bool scaleNormalized,HAL_FLAG hal>
				FreakDescriptorGenerator<true,scaleNormalized,hal>* FreakDescriptorGeneratorFactory<true,scaleNormalized,hal>::getDescriptorGenerator(size_t cols,size_t rows) const
				{
					return new FreakDescriptorGenerator<true,scaleNormalized,hal>(cols,rows,this);
				}

				template<bool scaleNormalized,HAL_FLAG hal>
				FreakDescriptorGenerator<false,scaleNormalized,hal>* FreakDescriptorGeneratorFactory<false,scaleNormalized,hal>::getDescriptorGenerator(size_t cols,size_t rows) const
				{
					return new FreakDescriptorGenerator<false,scaleNormalized,hal>(cols,rows,this);
				}
			}

			boost::property_tree::ptree DescriptorPropertyGenerator<Freak>::operator()(float patternScale,size_t numOctaves,bool orientationNormalized,bool scaleNormalized,const ushort(&selectedPairs)[freak::NUM_PAIRS])
			{
				CV_Assert(patternScale==patternScale&&patternScale>=1.0f);
				CV_Assert(numOctaves);
				boost::property_tree::ptree properties;
				properties.put<std::string>("descriptorType","freak");
				properties.put<float>("patternScale",patternScale);
				properties.put<size_t>("numOctaves",numOctaves);
				properties.put<bool>("orientationNormalized",orientationNormalized);
				properties.put<bool>("scaleNormalized",scaleNormalized);
				for(size_t i=0;i!=freak::NUM_PAIRS;++i)
				{
					if(selectedPairs[i]>902)
					{
						CV_Error(1,"selected pair "+std::to_string(selectedPairs[i])+" is out of range [0 - 902]");
					}
					properties.add<ushort>("selectedPairs",selectedPairs[i]);
				}
				return properties;
			}

			template<>
			DescriptorGeneratorFactory<Freak>* DescriptorGeneratorFactory<Freak>::getDescriptorGeneratorFactory(const boost::property_tree::ptree & properties,const lyonste::hal::HAL_FLAG hal)
			{
				const bool orientationNormalized=properties.get<bool>("orientationNormalized",true);
				const bool scaleNormalized=properties.get<bool>("scaleNormalized",true);
				const float patternScale=properties.get<float>("patternScale",22.f);
				const size_t numOctaves=properties.get<size_t>("numOctaves",4);
				auto itr=properties.find("selectedPairs");
				ushort selectedPairs[freak::NUM_PAIRS];
				if(itr!=properties.not_found()&&itr->first=="selectedPairs")
				{
					//ushort selectedPairs[512];
					size_t numPairs=0;
					for(;;)
					{
						ushort val=itr->second.get_value<ushort>();
						if(val>902)
						{
							CV_Error(1,"The value "+std::to_string(val)+" is out of range [0 - 902]");
						}
						selectedPairs[numPairs]=val;
						if(++numPairs==freak::NUM_PAIRS)
						{
							break;
						}
						if(++itr==properties.not_found()||itr->first!="selectedPairs")
						{
							CV_Error(1,"You must provide 512 selected pairs");
						}
					}
				}
				else
				{
					std::memcpy(selectedPairs,freak::DEFAULT_PAIRS,sizeof(ushort)*freak::NUM_PAIRS);
				}
				if(orientationNormalized)
				{
					if(scaleNormalized)
					{
						if(lyonste::hal::supports<lyonste::hal::HAL_SSE2>(hal))
						{
							return new freak::FreakDescriptorGeneratorFactory<true,true,lyonste::hal::HAL_SSE2>(numOctaves,patternScale,selectedPairs);
						}
						return new freak::FreakDescriptorGeneratorFactory<true,true,lyonste::hal::HAL_NONE>(numOctaves,patternScale,selectedPairs);
					}
					if(lyonste::hal::supports<lyonste::hal::HAL_SSE2>(hal))
					{
						return new freak::FreakDescriptorGeneratorFactory<true,false,lyonste::hal::HAL_SSE2>(numOctaves,patternScale,selectedPairs);
					}
					return new freak::FreakDescriptorGeneratorFactory<true,false,lyonste::hal::HAL_NONE>(numOctaves,patternScale,selectedPairs);
				}
				if(scaleNormalized)
				{
					if(lyonste::hal::supports<lyonste::hal::HAL_SSE2>(hal))
					{
						return new freak::FreakDescriptorGeneratorFactory<false,true,lyonste::hal::HAL_SSE2>(numOctaves,patternScale,selectedPairs);
					}
					return new freak::FreakDescriptorGeneratorFactory<false,true,lyonste::hal::HAL_NONE>(numOctaves,patternScale,selectedPairs);
				}
				if(lyonste::hal::supports<lyonste::hal::HAL_SSE2>(hal))
				{
					return new freak::FreakDescriptorGeneratorFactory<false,false,lyonste::hal::HAL_SSE2>(numOctaves,patternScale,selectedPairs);
				}
				return new freak::FreakDescriptorGeneratorFactory<false,false,lyonste::hal::HAL_NONE>(numOctaves,patternScale,selectedPairs);
			}
		}
	}
}