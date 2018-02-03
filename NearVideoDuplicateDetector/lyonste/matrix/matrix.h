#pragma once

#include "../hal.h"

namespace lyonste
{
	namespace matrix
	{


		const uchar NUM_CHANNELS=3;
		const uchar YUV_SHIFT=14;
		const uchar B_SRC_OFFSET=0;
		const uchar G_SRC_OFFSET=1;
		const uchar R_SRC_OFFSET=2;
		const ushort G_TAB_OFFSET=256;
		const ushort R_TAB_OFFSET=512;
		constexpr int BGR_TO_GRAY_TAB[]=
		{
			//b
			0, 1868, 3736, 5604, 7472, 9340, 11208, 13076, 14944, 16812, 18680, 20548, 22416, 24284, 26152, 28020,
			29888, 31756, 33624, 35492, 37360, 39228, 41096, 42964, 44832, 46700, 48568, 50436, 52304, 54172, 56040, 57908,
			59776, 61644, 63512, 65380, 67248, 69116, 70984, 72852, 74720, 76588, 78456, 80324, 82192, 84060, 85928, 87796,
			89664, 91532, 93400, 95268, 97136, 99004, 100872, 102740, 104608, 106476, 108344, 110212, 112080, 113948, 115816, 117684,
			119552, 121420, 123288, 125156, 127024, 128892, 130760, 132628, 134496, 136364, 138232, 140100, 141968, 143836, 145704, 147572,
			149440, 151308, 153176, 155044, 156912, 158780, 160648, 162516, 164384, 166252, 168120, 169988, 171856, 173724, 175592, 177460,
			179328, 181196, 183064, 184932, 186800, 188668, 190536, 192404, 194272, 196140, 198008, 199876, 201744, 203612, 205480, 207348,
			209216, 211084, 212952, 214820, 216688, 218556, 220424, 222292, 224160, 226028, 227896, 229764, 231632, 233500, 235368, 237236,
			239104, 240972, 242840, 244708, 246576, 248444, 250312, 252180, 254048, 255916, 257784, 259652, 261520, 263388, 265256, 267124,
			268992, 270860, 272728, 274596, 276464, 278332, 280200, 282068, 283936, 285804, 287672, 289540, 291408, 293276, 295144, 297012,
			298880, 300748, 302616, 304484, 306352, 308220, 310088, 311956, 313824, 315692, 317560, 319428, 321296, 323164, 325032, 326900,
			328768, 330636, 332504, 334372, 336240, 338108, 339976, 341844, 343712, 345580, 347448, 349316, 351184, 353052, 354920, 356788,
			358656, 360524, 362392, 364260, 366128, 367996, 369864, 371732, 373600, 375468, 377336, 379204, 381072, 382940, 384808, 386676,
			388544, 390412, 392280, 394148, 396016, 397884, 399752, 401620, 403488, 405356, 407224, 409092, 410960, 412828, 414696, 416564,
			418432, 420300, 422168, 424036, 425904, 427772, 429640, 431508, 433376, 435244, 437112, 438980, 440848, 442716, 444584, 446452,
			448320, 450188, 452056, 453924, 455792, 457660, 459528, 461396, 463264, 465132, 467000, 468868, 470736, 472604, 474472, 476340,
			//g
			0, 9617, 19234, 28851, 38468, 48085, 57702, 67319, 76936, 86553, 96170, 105787, 115404, 125021, 134638, 144255,
			153872, 163489, 173106, 182723, 192340, 201957, 211574, 221191, 230808, 240425, 250042, 259659, 269276, 278893, 288510, 298127,
			307744, 317361, 326978, 336595, 346212, 355829, 365446, 375063, 384680, 394297, 403914, 413531, 423148, 432765, 442382, 451999,
			461616, 471233, 480850, 490467, 500084, 509701, 519318, 528935, 538552, 548169, 557786, 567403, 577020, 586637, 596254, 605871,
			615488, 625105, 634722, 644339, 653956, 663573, 673190, 682807, 692424, 702041, 711658, 721275, 730892, 740509, 750126, 759743,
			769360, 778977, 788594, 798211, 807828, 817445, 827062, 836679, 846296, 855913, 865530, 875147, 884764, 894381, 903998, 913615,
			923232, 932849, 942466, 952083, 961700, 971317, 980934, 990551, 1000168, 1009785, 1019402, 1029019, 1038636, 1048253, 1057870, 1067487,
			1077104, 1086721, 1096338, 1105955, 1115572, 1125189, 1134806, 1144423, 1154040, 1163657, 1173274, 1182891, 1192508, 1202125, 1211742, 1221359,
			1230976, 1240593, 1250210, 1259827, 1269444, 1279061, 1288678, 1298295, 1307912, 1317529, 1327146, 1336763, 1346380, 1355997, 1365614, 1375231,
			1384848, 1394465, 1404082, 1413699, 1423316, 1432933, 1442550, 1452167, 1461784, 1471401, 1481018, 1490635, 1500252, 1509869, 1519486, 1529103,
			1538720, 1548337, 1557954, 1567571, 1577188, 1586805, 1596422, 1606039, 1615656, 1625273, 1634890, 1644507, 1654124, 1663741, 1673358, 1682975,
			1692592, 1702209, 1711826, 1721443, 1731060, 1740677, 1750294, 1759911, 1769528, 1779145, 1788762, 1798379, 1807996, 1817613, 1827230, 1836847,
			1846464, 1856081, 1865698, 1875315, 1884932, 1894549, 1904166, 1913783, 1923400, 1933017, 1942634, 1952251, 1961868, 1971485, 1981102, 1990719,
			2000336, 2009953, 2019570, 2029187, 2038804, 2048421, 2058038, 2067655, 2077272, 2086889, 2096506, 2106123, 2115740, 2125357, 2134974, 2144591,
			2154208, 2163825, 2173442, 2183059, 2192676, 2202293, 2211910, 2221527, 2231144, 2240761, 2250378, 2259995, 2269612, 2279229, 2288846, 2298463,
			2308080, 2317697, 2327314, 2336931, 2346548, 2356165, 2365782, 2375399, 2385016, 2394633, 2404250, 2413867, 2423484, 2433101, 2442718, 2452335,
			//r
			8192, 13091, 17990, 22889, 27788, 32687, 37586, 42485, 47384, 52283, 57182, 62081, 66980, 71879, 76778, 81677,
			86576, 91475, 96374, 101273, 106172, 111071, 115970, 120869, 125768, 130667, 135566, 140465, 145364, 150263, 155162, 160061,
			164960, 169859, 174758, 179657, 184556, 189455, 194354, 199253, 204152, 209051, 213950, 218849, 223748, 228647, 233546, 238445,
			243344, 248243, 253142, 258041, 262940, 267839, 272738, 277637, 282536, 287435, 292334, 297233, 302132, 307031, 311930, 316829,
			321728, 326627, 331526, 336425, 341324, 346223, 351122, 356021, 360920, 365819, 370718, 375617, 380516, 385415, 390314, 395213,
			400112, 405011, 409910, 414809, 419708, 424607, 429506, 434405, 439304, 444203, 449102, 454001, 458900, 463799, 468698, 473597,
			478496, 483395, 488294, 493193, 498092, 502991, 507890, 512789, 517688, 522587, 527486, 532385, 537284, 542183, 547082, 551981,
			556880, 561779, 566678, 571577, 576476, 581375, 586274, 591173, 596072, 600971, 605870, 610769, 615668, 620567, 625466, 630365,
			635264, 640163, 645062, 649961, 654860, 659759, 664658, 669557, 674456, 679355, 684254, 689153, 694052, 698951, 703850, 708749,
			713648, 718547, 723446, 728345, 733244, 738143, 743042, 747941, 752840, 757739, 762638, 767537, 772436, 777335, 782234, 787133,
			792032, 796931, 801830, 806729, 811628, 816527, 821426, 826325, 831224, 836123, 841022, 845921, 850820, 855719, 860618, 865517,
			870416, 875315, 880214, 885113, 890012, 894911, 899810, 904709, 909608, 914507, 919406, 924305, 929204, 934103, 939002, 943901,
			948800, 953699, 958598, 963497, 968396, 973295, 978194, 983093, 987992, 992891, 997790, 1002689, 1007588, 1012487, 1017386, 1022285,
			1027184, 1032083, 1036982, 1041881, 1046780, 1051679, 1056578, 1061477, 1066376, 1071275, 1076174, 1081073, 1085972, 1090871, 1095770, 1100669,
			1105568, 1110467, 1115366, 1120265, 1125164, 1130063, 1134962, 1139861, 1144760, 1149659, 1154558, 1159457, 1164356, 1169255, 1174154, 1179053,
			1183952, 1188851, 1193750, 1198649, 1203548, 1208447, 1213346, 1218245, 1223144, 1228043, 1232942, 1237841, 1242740, 1247639, 1252538, 1257437
		};
		constexpr void convertBGRBufferToGray(const uchar* src,uchar* dst,const size_t area)
		{
			for(size_t i=0; i!=area; ++i,src+=NUM_CHANNELS)
			{
				dst[i]=(uchar)((BGR_TO_GRAY_TAB[src[B_SRC_OFFSET]]+BGR_TO_GRAY_TAB[src[G_SRC_OFFSET]+G_TAB_OFFSET]+BGR_TO_GRAY_TAB[src[R_SRC_OFFSET]+R_TAB_OFFSET])>>YUV_SHIFT);
			}
		}
		
#define CHECK_BOUND(axis)CV_DbgAssert(axis<this->axis)

		template<typename T> struct Mat1D;
		template<typename T> struct Mat2D;
		template<typename T> struct Mat3D;
		template<typename T> struct Ptr1D
		{
			size_t x;
			T* data;
			constexpr Ptr1D(const size_t x,T* const data);
			constexpr const T* ptr(const size_t x) const;
			T* ptr(const size_t x);
			constexpr const T& ref(const size_t x) const;
			T& ref(const size_t x);
			constexpr const T& operator[](const size_t x) const;
			T& operator[](const size_t x);
			constexpr T val(const size_t x) const;
			constexpr Mat1D<T> val() const;
			constexpr void zeros();
			constexpr void ones();
			constexpr void fill(const T& val);
			virtual ~Ptr1D();
		};
		template<typename T> struct Ptr2D
		{
			size_t x,y;
			T* data;
			constexpr Ptr2D(const size_t x,const size_t y,T* const data);
			constexpr size_t area() const;
			constexpr const T*ptr(const size_t x,const size_t y) const;
			constexpr T* ptr(const size_t x,const size_t y);
			constexpr const T& ref(const size_t x,const size_t y) const;
			constexpr T& ref(const size_t x,const size_t y);
			constexpr T val(const size_t x,const size_t y) const;
			constexpr const T* ptr(const size_t y) const;
			constexpr T* ptr(const size_t y);
			constexpr const Ptr1D<T> ref(const size_t y) const;
			constexpr Ptr1D<T> ref(const size_t y);
			constexpr const Ptr1D<T> operator[](const size_t y) const;
			constexpr Ptr1D<T> operator[](const size_t y);
			constexpr Mat1D<T> val(const size_t y) const;
			constexpr void zeros(const size_t y);
			constexpr void ones(const size_t y);
			constexpr void fill(const size_t y,const T& val);
			constexpr Mat2D<T> val() const;
			constexpr void zeros();
			constexpr void ones();
			constexpr void fill(const T& val);
			/*constexpr*/ inline const cv::Mat getCVMat() const;
			/*constexpr*/ inline cv::Mat getCVMat();
			constexpr void overwrite(const cv::Mat& that);
			virtual ~Ptr2D();
		};
		template<typename T> struct Ptr3D
		{
			size_t x,y,z;
			T* data;
			constexpr Ptr3D(const size_t x,const size_t y,const size_t z,T* const data);
			constexpr size_t volume() const;
			constexpr const T* ptr(const size_t x,const size_t y,const size_t z) const;
			constexpr T* ptr(const size_t x,const size_t y,const size_t z);
			constexpr const T& ref(const size_t x,const size_t y,const size_t z) const;
			constexpr T& ref(const size_t x,const size_t y,const size_t z);
			constexpr T val(const size_t x,const size_t y,const size_t z) const;
			constexpr const T* ptr(const size_t y,const size_t z) const;
			constexpr T* ptr(const size_t y,const size_t z);
			constexpr const Ptr1D<T> ref(const size_t y,const size_t z) const;
			constexpr Ptr1D<T> ref(const size_t y,const size_t z);
			constexpr Mat1D<T> val(const size_t y,const size_t z) const;
			constexpr void zeros(const size_t y,const size_t z);
			constexpr void ones(const size_t y,const size_t z);
			constexpr void fill(const size_t y,const size_t z,const T& val);
			constexpr const T* ptr(const size_t z) const;
			constexpr T* ptr(const size_t z);
			constexpr const Ptr2D<T> ref(const size_t z) const;
			constexpr Ptr2D<T> ref(const size_t z);
			constexpr const Ptr2D<T> operator[](const size_t z) const;
			constexpr Ptr2D<T> operator[](const size_t z);
			constexpr Mat2D<T> val(const size_t z) const;
			constexpr void zeros(const size_t z);
			constexpr void ones(const size_t z);
			constexpr void fill(const size_t z,const T& val);
			constexpr Mat3D<T> val() const;
			constexpr void zeros();
			constexpr void ones();
			constexpr void fill(const T& val);
			/*constexpr*/ inline const cv::Mat getCVMat(const size_t z) const;
			/*constexpr*/ inline cv::Mat getCVMat(const size_t z);
			constexpr void overwrite(const cv::Mat& that,const size_t z);
			virtual ~Ptr3D();
		};
		template<typename T> struct Mat1D: public Ptr1D<T>
		{
			size_t capacity;
			constexpr Mat1D();
			constexpr Mat1D(const size_t x);
			constexpr Mat1D(const size_t x,T* const data,const size_t capacity);
			constexpr Mat1D(const Ptr1D<T>& that);
			constexpr Mat1D(const Mat1D<T>& that);
			constexpr Mat1D(Mat1D<T>&& that);
			constexpr Mat1D<T>& operator=(const Ptr1D<T>& that);
			constexpr Mat1D<T>& operator=(const Mat1D<T>& that);
			constexpr Mat1D<T>& operator=(Mat1D<T>&& that);
			constexpr void resize(size_t newSize);
			~Mat1D();
		};
		template<typename T> struct Mat2D: public Ptr2D<T>
		{
			size_t capacity;
			constexpr Mat2D();
			constexpr Mat2D(const size_t x,const size_t y,T* const data,const size_t capacity);
			constexpr Mat2D(const size_t x,const size_t y,const size_t capacity);
			constexpr Mat2D(const size_t x,const size_t y);
			constexpr Mat2D(const cv::Mat&  that);
			constexpr Mat2D(const Ptr2D<T>& that);
			constexpr Mat2D(const Mat2D<T>& that);
			constexpr Mat2D(Mat2D<T>&& that);
			constexpr Mat2D<T>& operator=(const Ptr2D<T>& that);
			constexpr Mat2D<T>& operator=(const Mat2D<T>& that);
			constexpr Mat2D<T>& operator=(Mat2D<T>&& that);
			~Mat2D();
		};
		template<typename T> struct Mat3D: public Ptr3D<T>
		{
			size_t capacity;
			constexpr Mat3D();
			constexpr Mat3D(const size_t x,const size_t y,const size_t z);
			constexpr Mat3D(const size_t x,const size_t y,const size_t z,const size_t capacity);
			constexpr Mat3D(const size_t x,const size_t y,const size_t z,T* const data,const size_t capacity);
			constexpr Mat3D(const cv::Mat& that,const size_t z);
			constexpr Mat3D(const Ptr3D<T>&  that);
			constexpr Mat3D(const Mat3D<T>&  that);
			constexpr Mat3D(Mat3D<T>&& that);
			constexpr Mat3D<T>& operator=(const Ptr3D<T> &  that);
			constexpr Mat3D<T>& operator=(const Mat3D<T> &  that);
			constexpr Mat3D<T>& operator=(Mat3D<T> && that);
			~Mat3D();
		};

		class HistoryMask: public Mat3D<uchar>
		{
		private:
			float weightFactor;
			short fgDiffThreshold;
		protected:
			virtual size_t getCombinedPtrIndex()=0;
		public:
#ifdef NDEBUG
			constexpr
#else
			inline
#endif
				HistoryMask(const float weightFactor,const short fgDiffThreshold,const size_t x,const size_t y,const size_t z):
				Mat3D<uchar>(x,y,z)
				,weightFactor(weightFactor)
				,fgDiffThreshold(fgDiffThreshold)
			{
				CV_DbgAssert(weightFactor > 1);
				CV_DbgAssert(fgDiffThreshold > 0);
				CV_DbgAssert(fgDiffThreshold < 255);
				CV_DbgAssert(x);
				CV_DbgAssert(y);
				CV_DbgAssert(z > 1);
				CV_DbgAssert(data);
				std::memset(data,0,x*y*z);
			}
			template<lyonste::hal::HAL_FLAG hal=lyonste::hal::HAL_NONE>constexpr void iterate(const Ptr2D<uchar>& frame)
			{
				CV_DbgAssert(frame.data);
				CV_DbgAssert(frame.x==x);
				CV_DbgAssert(frame.y==y);
				CV_DbgAssert(data);
				size_t i=0;
				const size_t area=x*y;
				const uchar* currFramePtr=frame.data;
				uchar* prevFramePtr=data+area*(z-1);
				uchar* combinedPtr=data+area*getCombinedPtrIndex();
				if((hal&lyonste::hal::HAL_SSSE3)!=0)
				{
					if(area>=16)
					{
						const size_t bound128=area-16;
						__m128i fgDiffThreshold128Pos=_mm_set1_epi16(fgDiffThreshold);
						__m128i fgDiffThreshold128Neg=_mm_set1_epi16(-fgDiffThreshold);
						if((((size_t)currFramePtr|(size_t)prevFramePtr)&15)==0)
						{
							//aligned
							for(; i < bound128; i+=16)
							{
								__m128i currFrame128lo=_mm_load_si128((const __m128i*)(currFramePtr+i));
								__m128i prevFrame128lo=_mm_load_si128((const __m128i*)(prevFramePtr+i));
								_mm_storeu_si128((__m128i*)(combinedPtr+i),_mm_packus_epi16(_mm_cmpgt_epi16(_mm_abs_epi16(_mm_sub_epi16(_mm_unpacklo_epi8(_mm_setzero_si128(),currFrame128lo),_mm_unpacklo_epi8(_mm_setzero_si128(),prevFrame128lo))),fgDiffThreshold128Pos),_mm_cmpgt_epi16(_mm_abs_epi16(_mm_sub_epi16(_mm_unpackhi_epi8(_mm_setzero_si128(),currFrame128lo),_mm_unpackhi_epi8(_mm_setzero_si128(),prevFrame128lo))),fgDiffThreshold128Pos)));
							}
						}
						else
						{
							//unaligned
							for(; i < bound128; i+=16)
							{
								__m128i currFrame128lo=_mm_loadu_si128((const __m128i*)(currFramePtr+i));
								__m128i prevFrame128lo=_mm_loadu_si128((const __m128i*)(prevFramePtr+i));
								_mm_storeu_si128((__m128i*)(combinedPtr+i),_mm_packus_epi16(_mm_cmpgt_epi16(_mm_abs_epi16(_mm_sub_epi16(_mm_unpacklo_epi8(_mm_setzero_si128(),currFrame128lo),_mm_unpacklo_epi8(_mm_setzero_si128(),prevFrame128lo))),fgDiffThreshold128Pos),_mm_cmpgt_epi16(_mm_abs_epi16(_mm_sub_epi16(_mm_unpackhi_epi8(_mm_setzero_si128(),currFrame128lo),_mm_unpackhi_epi8(_mm_setzero_si128(),prevFrame128lo))),fgDiffThreshold128Pos)));
							}
						}
					}
				}
				else if((hal&lyonste::hal::HAL_SSE2)!=0)
				{
					if(area>=16)
					{
						const size_t bound128=area-16;
						__m128i fgDiffThreshold128Pos=_mm_set1_epi16(fgDiffThreshold);
						__m128i fgDiffThreshold128Neg=_mm_set1_epi16(-fgDiffThreshold);
						if((((size_t)currFramePtr|(size_t)prevFramePtr)&15)==0)
						{
							//aligned
							for(; i < bound128; i+=16)
							{
								__m128i currFrame128lo=_mm_load_si128((const __m128i*)(currFramePtr+i));
								__m128i prevFrame128lo=_mm_load_si128((const __m128i*)(prevFramePtr+i));
								__m128i diff128lo=_mm_sub_epi16(_mm_unpacklo_epi8(_mm_setzero_si128(),currFrame128lo),_mm_unpacklo_epi8(_mm_setzero_si128(),prevFrame128lo));
								__m128i diff128hi=_mm_sub_epi16(_mm_unpackhi_epi8(_mm_setzero_si128(),currFrame128lo),_mm_unpackhi_epi8(_mm_setzero_si128(),prevFrame128lo));
								_mm_storeu_si128((__m128i*)(combinedPtr+i),_mm_packus_epi16(_mm_or_si128(_mm_cmpgt_epi16(diff128lo,fgDiffThreshold128Pos),_mm_cmplt_epi16(diff128lo,fgDiffThreshold128Neg)),_mm_or_si128(_mm_cmpgt_epi16(diff128hi,fgDiffThreshold128Pos),_mm_cmplt_epi16(diff128hi,fgDiffThreshold128Neg))));
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
								_mm_storeu_si128((__m128i*)(combinedPtr+i),_mm_packus_epi16(_mm_or_si128(_mm_cmpgt_epi16(diff128lo,fgDiffThreshold128Pos),_mm_cmplt_epi16(diff128lo,fgDiffThreshold128Neg)),_mm_or_si128(_mm_cmpgt_epi16(diff128hi,fgDiffThreshold128Pos),_mm_cmplt_epi16(diff128hi,fgDiffThreshold128Neg))));
							}
						}
					}
				}
				for(; i!=area; ++i)
				{
					combinedPtr[i]=std::abs(short(currFramePtr[i])-short(prevFramePtr[i])) > fgDiffThreshold?255:0;
				}
				std::memcpy(prevFramePtr,currFramePtr,area);
			}
			constexpr float weightResponse(const float response,const size_t x,const size_t y) const
			{
				CV_DbgAssert(x<this->x);
				CV_DbgAssert(y<this->y);
				CV_DbgAssert(data);
				for(size_t i=z-1; i--;)
				{
					if(data[this->x*this->y*i+this->x*y+x])
					{
						return response*weightFactor;
					}
				}
				return response;
			}
		};
		class SingleFrameHistoryMask: public HistoryMask
		{
		protected:
			size_t getCombinedPtrIndex() override
			{
				return 0;
			}
		public:
			constexpr SingleFrameHistoryMask(const float weightFactor,const short fgDiffThreshold,const size_t x,const size_t y):
				HistoryMask(weightFactor,fgDiffThreshold,x,y,2)
			{}
		};
		class MultiFrameHistoryMask: public HistoryMask
		{
		private:
			size_t frameIndex;
		protected:
			size_t getCombinedPtrIndex() override
			{
				return (++frameIndex)%(z-1);
			}
		public:
			constexpr MultiFrameHistoryMask(const float weightFactor,const short fgDiffThreshold,const size_t x,const size_t y,const size_t historyLength):
				HistoryMask(weightFactor,fgDiffThreshold,x,y,historyLength+1)
				,frameIndex(-1)
			{}
		};


		template<typename T> constexpr Ptr1D<T>::Ptr1D(const size_t x,T* const data): x(x),data(data)
		{}
		template<typename T> constexpr const T* Ptr1D<T>::ptr(const size_t x) const
		{
			CHECK_BOUND(x); return data+x;
		}
		template<typename T> T* Ptr1D<T>::ptr(const size_t x)
		{
			CHECK_BOUND(x); return data+x;
		}
		template<typename T> constexpr const T& Ptr1D<T>::ref(const size_t x) const
		{
			CHECK_BOUND(x); CV_DbgAssert(data); return data[x];
		}
		template<typename T> T& Ptr1D<T>::ref(const size_t x)
		{
			CHECK_BOUND(x); CV_DbgAssert(data); return data[x];
		}
		template<typename T> constexpr const T& Ptr1D<T>::operator[](const size_t x) const
		{
			CHECK_BOUND(x); CV_DbgAssert(data); return data[x];
		}
		template<typename T> T& Ptr1D<T>::operator[](const size_t x)
		{
			CHECK_BOUND(x); CV_DbgAssert(data); return data[x];
		}
		template<typename T> constexpr T Ptr1D<T>::val(const size_t x) const
		{
			CHECK_BOUND(x); CV_DbgAssert(data); return data[x];
		}
		template<typename T> constexpr Mat1D<T> Ptr1D<T>::val() const
		{
			if(x)
			{
				CV_DbgAssert(data);
				T* tmpData=new T[x];
				std::memcpy(tmpData,data,x*sizeof(T));
				return Mat1D<T>(x,tmpData,x);
			}
			return Mat1D<T>(0,NULL,0);
		}
		template<typename T> constexpr void Ptr1D<T>::zeros()
		{
			if(x)
			{
				CV_DbgAssert(data);
				std::memset(data,0,sizeof(T)*x);
			}
		}
		template<typename T> constexpr void Ptr1D<T>::ones()
		{
			if(x)
			{
				CV_DbgAssert(data);
				std::memset(data,0b11111111,sizeof(T)*x);
			}
		}
		template<typename T> constexpr void Ptr1D<T>::fill(const T& val)
		{
			if(x)
			{
				CV_DbgAssert(data);
				std::fill(data,data+x,val);
			}
		}
		template<typename T> Ptr1D<T>::~Ptr1D()
		{}

		template<typename T> constexpr Ptr2D<T>::Ptr2D(const size_t x,const size_t y,T* const data): x(x),y(y),data(data)
		{}
		template<typename T> constexpr size_t Ptr2D<T>::area() const
		{
			return x*y;
		}
		template<typename T> constexpr const T* Ptr2D<T>::ptr(const size_t x,const size_t y) const
		{
			CHECK_BOUND(x); CHECK_BOUND(y); return data+this->x*y+x;
		}
		template<typename T> constexpr T* Ptr2D<T>::ptr(const size_t x,const size_t y)
		{
			CHECK_BOUND(x); CHECK_BOUND(y); return data+this->x*y+x;
		}
		template<typename T> constexpr const T& Ptr2D<T>::ref(const size_t x,const size_t y) const
		{
			CHECK_BOUND(x); CHECK_BOUND(y); CV_DbgAssert(data); return data[this->x*y+x];
		}
		template<typename T> constexpr T& Ptr2D<T>::ref(const size_t x,const size_t y)
		{
			CHECK_BOUND(x); CHECK_BOUND(y); CV_DbgAssert(data); return data[this->x*y+x];
		}
		template<typename T> constexpr T Ptr2D<T>::val(const size_t x,const size_t y) const
		{
			CHECK_BOUND(x); CHECK_BOUND(y); CV_DbgAssert(data); return data[this->x*y+x];
		}
		template<typename T> constexpr const T* Ptr2D<T>::ptr(const size_t y) const
		{
			CHECK_BOUND(y); return data+x*y;
		}
		template<typename T> constexpr T* Ptr2D<T>::ptr(const size_t y)
		{
			CHECK_BOUND(y); return data+x*y;
		}
		template<typename T> constexpr const Ptr1D<T> Ptr2D<T>::ref(const size_t y) const
		{
			CHECK_BOUND(y); return Ptr1D<T>(x,data+x*y);
		}
		template<typename T> constexpr Ptr1D<T> Ptr2D<T>::ref(const size_t y)
		{
			CHECK_BOUND(y); return Ptr1D<T>(x,data+x*y);
		}
		template<typename T> constexpr const Ptr1D<T> Ptr2D<T>::operator[](const size_t y) const
		{
			CHECK_BOUND(y); return Ptr1D<T>(x,data+x*y);
		}
		template<typename T> constexpr Ptr1D<T> Ptr2D<T>::operator[](const size_t y)
		{
			CHECK_BOUND(y); return Ptr1D<T>(x,data+x*y);
		}
		template<typename T> constexpr Mat1D<T> Ptr2D<T>::val(const size_t y) const
		{
			CHECK_BOUND(y);
			if(x)
			{
				CV_DbgAssert(data);
				T* tmpData=new T[x];
				std::memcpy(tmpData,data+x*y,x*sizeof(T));
				return Mat1D<T>(x,tmpData,x);
			}
			return Mat1D<T>(0,NULL,0);
		}
		
		template<typename T> constexpr void  Ptr2D<T>::zeros(const size_t y)
		{
			CHECK_BOUND(y);
			if(x)
			{
				CV_DbgAssert(data);
				std::memset(data+x*y,0,sizeof(T)*x);
			}
		}
		template<typename T> constexpr void  Ptr2D<T>::ones(const size_t y)
		{
			CHECK_BOUND(y);
			if(x)
			{
				CV_DbgAssert(data);
				std::memset(data+x*y,0b11111111,sizeof(T)*x);
			}
		}
		template<typename T> constexpr void  Ptr2D<T>::fill(const size_t y,const T& val)
		{
			CHECK_BOUND(y);
			if(x)
			{
				CV_DbgAssert(data);
				std::fill(data+x*y,data+x*(y+1),val);
			}
		}
		template<typename T> constexpr Mat2D<T> Ptr2D<T>::val() const
		{
			size_t area=x*y;
			if(area)
			{
				CV_DbgAssert(data);
				T* tmpData=new T[area];
				std::memcpy(tmpData,data,area*sizeof(T));
				return Mat2D<T>(x,y,data,area);
			}
			return Mat2D<T>(x,y,NULL,0);
		}
		template<typename T> constexpr void  Ptr2D<T>::zeros()
		{
			size_t area=x*y;
			if(area)
			{
				CV_DbgAssert(data);
				std::memset(data,0,sizeof(T)*area);
			}
		}
		template<typename T> constexpr void  Ptr2D<T>::ones()
		{
			size_t area=x*y;
			if(area)
			{
				CV_DbgAssert(data);
				std::memset(data,0b11111111,sizeof(T)*area);
			}
		}
		template<typename T> constexpr void  Ptr2D<T>::fill(const T& val)
		{
			size_t area=x*y;
			if(area)
			{
				CV_DbgAssert(data);
				std::fill(data,data+area,val);
			}
		}
		template<typename T> inline const cv::Mat Ptr2D<T>::getCVMat() const
		{
			CV_DbgAssert(x < size_t(std::numeric_limits<int>::max()));
			CV_DbgAssert(y < size_t(std::numeric_limits<int>::max()));
			return cv::Mat(int(y),int(x),cv::DataType<T>::type,data,sizeof(T)*x);
		}
		template<typename T> inline cv::Mat Ptr2D<T>::getCVMat()
		{
			CV_DbgAssert(x < size_t(std::numeric_limits<int>::max()));
			CV_DbgAssert(y < size_t(std::numeric_limits<int>::max()));
			return cv::Mat(int(y),int(x),cv::DataType<T>::type,data,sizeof(T)*x);
		}
		template<> constexpr void Ptr2D<uchar>::overwrite(const cv::Mat& that)
		{
			CV_DbgAssert(that.channels()==3);
			CV_DbgAssert(that.depth()==0);
			CV_DbgAssert(size_t(that.cols)==x);
			CV_DbgAssert(size_t(that.rows)==y);
			size_t area=x*y;
			CV_DbgAssert(area?data!=NULL:true);
			convertBGRBufferToGray(that.data,data,area);
		}
		template<typename T> Ptr2D<T>::~Ptr2D()
		{}

		template<typename T> constexpr Ptr3D<T>::Ptr3D(const size_t x,const size_t y,const size_t z,T* const data): x(x),y(y),z(z),data(data)
		{}
		template<typename T> constexpr size_t Ptr3D<T>::volume() const
		{
			return x*y*z;
		}
		template<typename T> constexpr const T* Ptr3D<T>::ptr(const size_t x,const size_t y,const size_t z) const
		{
			CHECK_BOUND(x); CHECK_BOUND(y); CHECK_BOUND(z); return data+this->x * this->y*z+this->x*y+x;
		}
		template<typename T> constexpr T* Ptr3D<T>::ptr(const size_t x,const size_t y,const size_t z)
		{
			CHECK_BOUND(x); CHECK_BOUND(y); CHECK_BOUND(z); return data+this->x * this->y*z+this->x*y+x;
		}
		template<typename T> constexpr const T& Ptr3D<T>::ref(const size_t x,const size_t y,const size_t z) const
		{
			CHECK_BOUND(x); CHECK_BOUND(y); CHECK_BOUND(z); CV_DbgAssert(data); return data[this->x+this->y*z+this->x*y+x];
		}
		template<typename T> constexpr T& Ptr3D<T>::ref(const size_t x,const size_t y,const size_t z)
		{
			CHECK_BOUND(x); CHECK_BOUND(y); CHECK_BOUND(z); CV_DbgAssert(data); return data[this->x+this->y*z+this->x*y+x];
		}
		template<typename T> constexpr T Ptr3D<T>::val(const size_t x,const size_t y,const size_t z) const
		{
			CHECK_BOUND(x); CHECK_BOUND(y); CHECK_BOUND(z); CV_DbgAssert(data); return data[this->x+this->y*z+this->x*y+x];
		}
		template<typename T> constexpr const T* Ptr3D<T>::ptr(const size_t y,const size_t z) const
		{
			CHECK_BOUND(y); CHECK_BOUND(z); return data+x*this->y*z+x*y;
		}
		template<typename T> constexpr T* Ptr3D<T>::ptr(const size_t y,const size_t z)
		{
			CHECK_BOUND(y); CHECK_BOUND(z); return data+x*this->y*z+x*y;
		}
		template<typename T> constexpr const Ptr1D<T> Ptr3D<T>::ref(const size_t y,const size_t z) const
		{
			CHECK_BOUND(y); CHECK_BOUND(z); return Ptr1D<T>(x,data+x*this->y*z+x*y);
		}
		template<typename T> constexpr Ptr1D<T> Ptr3D<T>::ref(const size_t y,const size_t z)
		{
			CHECK_BOUND(y); CHECK_BOUND(z); return Ptr1D<T>(x,data+x*this->y*z+x*y);
		}
		template<typename T> constexpr Mat1D<T> Ptr3D<T>::val(const size_t y,const size_t z) const
		{
			CHECK_BOUND(y);
			CHECK_BOUND(z);
			if(x)
			{
				CV_DbgAssert(data);
				T* tmpData=new T[x];
				std::memcpy(tmpData,data+x*this->y*z+x*y,x*sizeof(T));
				return Mat1D<T>(x,tmpData,x);
			}
			return Mat1D<T>(0,NULL,0);
		}
		template<typename T> constexpr void Ptr3D<T>::zeros(const size_t y,const size_t z)
		{
			CHECK_BOUND(y);
			CHECK_BOUND(z);
			if(x)
			{
				CV_DbgAssert(data);
				std::memset(data+x*this->y*z+x*y,0,x*sizeof(T));
			}
		}
		template<typename T> constexpr void Ptr3D<T>::ones(const size_t y,const size_t z)
		{
			CHECK_BOUND(y);
			CHECK_BOUND(z);
			if(x)
			{
				CV_DbgAssert(data);
				std::memset(data+x*this->y*z+x*y,0b11111111,x*sizeof(T));
			}
		}
		template<typename T> constexpr void Ptr3D<T>::fill(const size_t y,const size_t z,const T& val)
		{
			CHECK_BOUND(y);
			CHECK_BOUND(z);
			if(x)
			{
				CV_DbgAssert(data);
				std::fill(data+(x*this->y*z+x*y),data+(x*this->y*z+x*(y+1)),val);
			}

		}
		template<typename T> constexpr const T* Ptr3D<T>::ptr(const size_t z) const
		{
			CHECK_BOUND(z); return data+x*y*z;
		}
		template<typename T> constexpr T* Ptr3D<T>::ptr(const size_t z)
		{
			CHECK_BOUND(z); return data+x*y*z;
		}
		template<typename T> constexpr const Ptr2D<T> Ptr3D<T>::ref(const size_t z) const
		{
			CHECK_BOUND(z); return Ptr2D<T>(x,y,data+x*y*z);
		}
		template<typename T> constexpr Ptr2D<T> Ptr3D<T>::ref(const size_t z)
		{
			CHECK_BOUND(z); return Ptr2D<T>(x,y,data+x*y*z);
		}
		template<typename T> constexpr const Ptr2D<T> Ptr3D<T>::operator[](const size_t z) const
		{
			CHECK_BOUND(z); return Ptr2D<T>(x,y,data+x*y*z);
		}
		template<typename T> constexpr Ptr2D<T> Ptr3D<T>::operator[](const size_t z)
		{
			CHECK_BOUND(z); return Ptr2D<T>(x,y,data+x*y*z);
		}
		template<typename T> constexpr Mat2D<T> Ptr3D<T>::val(const size_t z) const
		{
			CHECK_BOUND(z);
			size_t area=x*y;
			if(area)
			{
				CV_DbgAssert(data);
				T* tmpData=new T[area];
				std::memcpy(tmpData,data+area*z,area*sizeof(T));
				return Mat2D<T>(x,y,data,area);
			}
			return Mat2D<T>(x,y,NULL,0);
		}
		template<typename T> constexpr void Ptr3D<T>::zeros(const size_t z)
		{
			CHECK_BOUND(z);
			size_t area=x*y;
			if(area)
			{
				CV_DbgAssert(data);
				std::memset(data+area*z,0,area);
			}
		}
		template<typename T> constexpr void Ptr3D<T>::ones(const size_t z)
		{
			CHECK_BOUND(z);
			size_t area=x*y;
			if(area)
			{
				CV_DbgAssert(data);
				std::memset(data+area*z,0,area);
			}
		}
		template<typename T> constexpr void Ptr3D<T>::fill(const size_t z,const T& val)
		{
			CHECK_BOUND(z);
			size_t area=x*y;
			if(area)
			{
				CV_DbgAssert(data);
				std::fill(data+(area*z),data+(area*(z+1)),val);
			}
		}
		template<typename T> constexpr Mat3D<T> Ptr3D<T>::val() const
		{
			size_t volume=x*y*z;
			if(volume)
			{
				CV_DbgAssert(data);
				T* tmpData=new T[volume];
				std::memcpy(tmpData,data,volume);
				return Mat3D<T>(x,y,z,data,volume);
			}
			return Mat3D<T>(x,y,z,NULL,volume);
		}
		template<typename T> constexpr void Ptr3D<T>::zeros()
		{
			size_t volume=x*y*z;
			if(volume)
			{
				CV_DbgAssert(data);
				std::memset(data,0,volume);
			}
		}
		template<typename T> constexpr void Ptr3D<T>::ones()
		{
			size_t volume=x*y*z;
			if(volume)
			{
				CV_DbgAssert(data);
				std::memset(data,0b11111111,volume);
			}
		}
		template<typename T> constexpr void Ptr3D<T>::fill(const T& val)
		{
			size_t volume=x*y*z;
			if(volume)
			{
				CV_DbgAssert(data);
				std::fill(data,data+volume,val);
			}
		}
		template<typename T> inline const cv::Mat Ptr3D<T>::getCVMat(const size_t z) const
		{
			CHECK_BOUND(z);
			CV_DbgAssert(x < size_t(std::numeric_limits<int>::max()));
			CV_DbgAssert(y < size_t(std::numeric_limits<int>::max()));
			return cv::Mat(int(y),int(x),cv::DataType<T>::type,data+x*y*z,sizeof(T)*x);
		}
		template<typename T>  inline cv::Mat Ptr3D<T>::getCVMat(const size_t z)
		{
			CHECK_BOUND(z);
			CV_DbgAssert(x < size_t(std::numeric_limits<int>::max()));
			CV_DbgAssert(y < size_t(std::numeric_limits<int>::max()));
			return cv::Mat(int(y),int(x),cv::DataType<T>::type,data+x*y*z,sizeof(T)*x);
		}
		template<> constexpr void Ptr3D<uchar>::overwrite(const cv::Mat& that,const size_t z)
		{
			CHECK_BOUND(z);
			CV_DbgAssert(that.channels()==3);
			CV_DbgAssert(that.depth()==0);
			CV_DbgAssert(size_t(that.cols)==x);
			CV_DbgAssert(size_t(that.rows)==y);
			size_t area=x*y;
			CV_DbgAssert(area?data!=NULL:true);
			convertBGRBufferToGray(that.data,data+area*z,area);
		}
		template<typename T> Ptr3D<T>::~Ptr3D()
		{}

		template<typename T> constexpr Mat1D<T>::Mat1D(): Ptr1D<T>(0,NULL),capacity(0)
		{}
		template<typename T> constexpr Mat1D<T>::Mat1D(const size_t x):Ptr1D<T>(x,x?new T[x]:NULL),capacity(x)
		{}
		template<typename T> constexpr Mat1D<T>::Mat1D(const size_t x,T* const data,const size_t capacity):Ptr1D<T>(x,data),capacity(capacity)
		{}
		template<typename T> constexpr Mat1D<T>::Mat1D(const Ptr1D<T>& that):Ptr1D<T>(that.x,NULL),capacity(x)
		{
			if(capacity)
			{
				CV_DbgAssert(that.data);
				std::memcpy(data=new T[capacity],that.data,sizeof(T)*capacity);
			}
		}
		template<typename T> constexpr Mat1D<T>::Mat1D(const Mat1D<T>& that): Ptr1D<T>(that.x,NULL),capacity(that.capacity)
		{
			if(capacity)
			{
				data=new T[capacity];
				if(x)
				{
					CV_DbgAssert(that.data);
					std::memcpy(data,that.data,sizeof(T)*x);
				}
			}
		}
		template<typename T> constexpr Mat1D<T>::Mat1D(Mat1D<T>&& that): Ptr1D<T>(that.x,that.data),capacity(that.capacity)
		{
			that.data=NULL;
		}
		template<typename T> constexpr Mat1D<T>& Mat1D<T>::operator=(const Ptr1D<T>& that)
		{
			if(this!=&that)
			{
				if(x=that.x)
				{
					CV_DbgAssert(that.data);
					if(x > capacity)
					{
						T* tmpData=new T[capacity=x];
						std::memcpy(tmpData,that.data,sizeof(T)*x);
						delete[] data;
						data=tmpData;
					}
					else
					{
						std::memmove(data,that.data,sizeof(T)*x);
					}
				}
			}
			return *this;
		}
		template<typename T> constexpr Mat1D<T>& Mat1D<T>::operator=(const Mat1D<T>& that)
		{
			if(this!=&that)
			{
				if(x=that.x)
				{
					CV_DbgAssert(that.data);
					if(x > capacity)
					{
						T* tmpData=new T[capacity=x];
						std::memcpy(tmpData,that.data,sizeof(T)*x);
						delete[] data;
						data=tmpData;
					}
					else
					{
						std::memmove(data,that.data,sizeof(T)*x);
					}
				}
			}
			return *this;
		}
		template<typename T> constexpr Mat1D<T>& Mat1D<T>::operator=(Mat1D<T>&& that)
		{
			if(this!=&that)
			{
				x=that.x;
				capacity=that.capacity;
				delete[] data;
				data=that.data;
				that.data=NULL;
			}
			return *this;
		}
		template<typename T> constexpr void Mat1D<T>::resize(size_t newSize)
		{
			if(newSize>capacity)
			{
				T* tmpData=new T[newSize];
				if(capacity)
				{
					std::memcpy(tmpData,data,sizeof(T)*capacity);
				}
				delete[] data;
				data=tmpData;
				capacity=newSize;
			}
			x=newSize;
		}
		template<typename T> Mat1D<T>::~Mat1D()
		{
			delete[] data;
		}

		template<typename T> constexpr Mat2D<T>::Mat2D(): Ptr2D<T>(0,0,NULL),capacity(0)
		{}
		template<typename T> constexpr Mat2D<T>::Mat2D(const size_t x,const size_t y,T* const data,const size_t capacity):Ptr2D<T>(x,y,data),capacity(capacity)
		{}
		template<typename T> constexpr Mat2D<T>::Mat2D(const size_t x,const size_t y,const size_t capacity):Ptr2D<T>(x,y,capacity?new T[capacity]:NULL),capacity(capacity)
		{}
		template<typename T> constexpr Mat2D<T>::Mat2D(const size_t x,const size_t y):Mat2D<T>(x,y,x*y)
		{}
		template<> constexpr Mat2D<uchar>::Mat2D(const cv::Mat& that):Mat2D<uchar>(size_t(that.cols),size_t(that.rows))
		{
			if(capacity)
			{
				CV_DbgAssert(that.data);
				CV_DbgAssert(that.channels()==3);
				CV_DbgAssert(that.depth()==0);
				convertBGRBufferToGray(that.data,this->data,capacity);
			}
		}
		template<typename T> constexpr Mat2D<T>::Mat2D(const Ptr2D<T>& that):Ptr2D<T>(that.x,that.y,NULL),capacity(that.x*that.y)
		{
			if(capacity)
			{
				CV_DbgAssert(that.data);
				std::memcpy(data=new T[capacity],that.data,sizeof(T)*capacity);
			}
		}
		template<typename T> constexpr Mat2D<T>::Mat2D(const Mat2D<T>& that): Ptr2D<T>(that.x,that.y,NULL),capacity(that.capacity)
		{
			if(capacity)
			{
				data=new T[capacity];
				size_t area=x*y;
				if(area)
				{
					CV_DbgAssert(that.data);
					std::memcpy(data,that.data,sizeof(T)*area);
				}
			}
		}
		template<typename T> constexpr Mat2D<T>::Mat2D(Mat2D<T>&& that): Ptr2D<T>(that.x,that.y,that.data),capacity(that.capacity)
		{
			that.data=NULL;
		}
		template<typename T> constexpr Mat2D<T>& Mat2D<T>::operator=(const Ptr2D<T>& that)
		{
			if(this!=&that)
			{
				size_t area=(x=that.x)*(y=that.y);
				if(area)
				{
					CV_DbgAssert(that.data);
					if(area > capacity)
					{
						T* tmpData=new T[capacity=area];
						std::memcpy(tmpData,that.data,sizeof(T)*area);
						delete[] data;
						data=tmpData;
					}
					else
					{
						std::memmove(data,that.data,sizeof(T)*area);
					}
				}
			}
			return *this;
		}
		template<typename T> constexpr Mat2D<T>& Mat2D<T>::operator=(const Mat2D<T>& that)
		{
			if(this!=&that)
			{
				size_t area=(x=that.x)*(y=that.y);
				if(area)
				{
					CV_DbgAssert(that.data);
					if(area > capacity)
					{
						T* tmpData=new T[capacity=area];
						std::memcpy(tmpData,that.data,sizeof(T)*area);
						delete[] data;
						data=tmpData;
					}
					else
					{
						std::memmove(data,that.data,sizeof(T)*area);
					}
				}
			}
			return *this;
		}
		template<typename T> constexpr Mat2D<T>& Mat2D<T>::operator=(Mat2D<T>&& that)
		{
			if(this!=&that)
			{
				x=that.x;
				y=that.y;
				capacity=that.capacity;
				delete[] data;
				data=that.data;
				that.data=NULL;
			}
			return *this;
		}
		template<typename T> Mat2D<T>::~Mat2D()
		{
			delete[] data;
		}

		template<typename T> constexpr Mat3D<T>::Mat3D(): Ptr3D<T>(0,0,0,NULL),capacity(0)
		{}
		template<typename T> constexpr Mat3D<T>::Mat3D(const size_t x,const size_t y,const size_t z):Mat3D<T>(x,y,z,x*y*z)
		{}
		template<typename T> constexpr Mat3D<T>::Mat3D(const size_t x,const size_t y,const size_t z,const size_t capacity):Ptr3D<T>(x,y,z,capacity?new T[capacity]:NULL),capacity(capacity)
		{}
		template<typename T> constexpr Mat3D<T>::Mat3D(const size_t x,const size_t y,const size_t z,T* const data,const size_t capacity):Ptr3D<T>(x,y,z,data),capacity(capacity)
		{}
		template<> constexpr Mat3D<uchar>::Mat3D(const cv::Mat & that,const size_t z):Mat3D(size_t(that.cols),size_t(that.rows),z)
		{
			if(capacity)
			{
				CV_DbgAssert(that.data);
				CV_DbgAssert(that.channels()==3);
				CV_DbgAssert(that.depth()==0);
				size_t area=x*y;
				convertBGRBufferToGray(that.data,this->data+area*z,area);
			}
		}
		template<typename T> constexpr Mat3D<T>::Mat3D(const Ptr3D<T>&  that):Ptr3D<T>(that.x,that.y,that.z,NULL),capacity(that.x*that.y*that.z)
		{
			if(capacity)
			{
				CV_DbgAssert(that.data);
				std::memcpy(data=new T[capacity],that.data,sizeof(T)*capacity);
			}
		}
		template<typename T> constexpr Mat3D<T>::Mat3D(const Mat3D<T>&  that): Ptr3D<T>(that.x,that.y,that.z,NULL),capacity(that.x*that.y*that.z)
		{
			if(capacity)
			{
				data=new T[capacity];
				size_t volume=x*y*z;
				if(volume)
				{
					CV_DbgAssert(that.data);
					std::memcpy(data,that.data,sizeof(T)*volume);
				}
			}
		}
		template<typename T> constexpr Mat3D<T>::Mat3D(Mat3D<T>&& that): Ptr3D<T>(that.x,that.y,that.z,that.data),capacity(that.capacity)
		{
			that.data=NULL;
		}
		template<typename T> constexpr Mat3D<T>& Mat3D<T>::operator=(const Ptr3D<T> &  that)
		{
			if(this!=&that)
			{
				size_t volume=(x=that.x)*(y=that.y)*(z=that.z);
				if(volume)
				{
					CV_DbgAssert(that.data);
					if(volume > capacity)
					{
						T* tmpData=new T[capacity=volume];
						std::memcpy(tmpData,that.data,sizeof(T)*volume);
						delete[] data;
						data=tmpData;
					}
					else
					{
						std::memmove(data,that.data,sizeof(T)*volume);
					}
				}
			}
			return *this;
		}
		template<typename T> constexpr Mat3D<T>& Mat3D<T>::operator=(const Mat3D<T> &  that)
		{
			if(this!=&that)
			{
				size_t volume=(x=that.x)*(y=that.y)*(z=that.z);
				if(volume)
				{
					CV_DbgAssert(that.data);
					if(volume > capacity)
					{
						T* tmpData=new T[capacity=volume];
						std::memcpy(tmpData,that.data,sizeof(T)*volume);
						delete[] data;
						data=tmpData;
					}
					else
					{
						std::memmove(data,that.data,sizeof(T)*volume);
					}
				}
			}
			return *this;
		}
		template<typename T> constexpr Mat3D<T>& Mat3D<T>::operator=(Mat3D<T> && that)
		{
			if(this!=&that)
			{
				x=that.x;
				y=that.y;
				z=that.z;
				capacity=that.capacity;
				delete[] data;
				data=that.data;
				that.data=NULL;
			}
			return *this;
		}
		template<typename T> Mat3D<T>::~Mat3D()
		{
			delete[] data;
		}
	}
}