#include "featureDetection.h"
#include "../../lyonste.h"
//#include <iostream>

namespace lyonste
{
	namespace feature2D
	{
		namespace featureDetection
		{

			namespace segmenttest
			{
				namespace fast
				{
					using namespace lyonste::hal;

					template<size_t patternSize=16,PatternShape shape=Circle,KPRetentionPolicy retentionPolicy=NON_MAX_SUPPRESSION,bool compatibilityMode=true,HAL_FLAG hal=HAL_NONE,typename ENABLE=void>
					class FastKeyPointGenerator;

					template<size_t patternSize=16,PatternShape shape=Circle,KPRetentionPolicy retentionPolicy=NON_MAX_SUPPRESSION,bool compatibilityMode=true,HAL_FLAG hal=HAL_NONE>
					class FastKeyPointGeneratorFactory: public KeyPointGeneratorFactory
					{
					public:
						const size_t border;
						const short fastThreshold;
						const uchar thresholdTable[512];

						std::string toString() const noexcept override
						{
							return std::string(kpTypeStr<Fast>)+"{border="+std::to_string(border)+"; fastThreshold="+std::to_string(fastThreshold)+"; patternSize="+std::to_string(patternSize)
								+"; shape="+patternShapeStr<shape>+"; retentionPolicy="+kpRetentionPolicyStr<retentionPolicy>+"; compatibilityMode="+(compatibilityMode?"true":"false")+"; hal="+std::to_string(hal)+"}";
						}

						constexpr
							FastKeyPointGeneratorFactory(size_t border,short fastThreshold):
							border(std::max<size_t>(border,patternBorder<compatibilityMode,patternSize,shape,void>)),
							//border(std::max<size_t>(border,Pattern<patternSize,shape>::getBorder<compatibilityMode>())),
							fastThreshold(fastThreshold),
							thresholdTable
						{
							uchar(-255 < -fastThreshold?1:-255 > fastThreshold?2:0),
							uchar(-254 < -fastThreshold?1:-254 > fastThreshold?2:0),
							uchar(-253 < -fastThreshold?1:-253 > fastThreshold?2:0),
							uchar(-252 < -fastThreshold?1:-252 > fastThreshold?2:0),
							uchar(-251 < -fastThreshold?1:-251 > fastThreshold?2:0),
							uchar(-250 < -fastThreshold?1:-250 > fastThreshold?2:0),
							uchar(-249 < -fastThreshold?1:-249 > fastThreshold?2:0),
							uchar(-248 < -fastThreshold?1:-248 > fastThreshold?2:0),
							uchar(-247 < -fastThreshold?1:-247 > fastThreshold?2:0),
							uchar(-246 < -fastThreshold?1:-246 > fastThreshold?2:0),
							uchar(-245 < -fastThreshold?1:-245 > fastThreshold?2:0),
							uchar(-244 < -fastThreshold?1:-244 > fastThreshold?2:0),
							uchar(-243 < -fastThreshold?1:-243 > fastThreshold?2:0),
							uchar(-242 < -fastThreshold?1:-242 > fastThreshold?2:0),
							uchar(-241 < -fastThreshold?1:-241 > fastThreshold?2:0),
							uchar(-240 < -fastThreshold?1:-240 > fastThreshold?2:0),
							uchar(-239 < -fastThreshold?1:-239 > fastThreshold?2:0),
							uchar(-238 < -fastThreshold?1:-238 > fastThreshold?2:0),
							uchar(-237 < -fastThreshold?1:-237 > fastThreshold?2:0),
							uchar(-236 < -fastThreshold?1:-236 > fastThreshold?2:0),
							uchar(-235 < -fastThreshold?1:-235 > fastThreshold?2:0),
							uchar(-234 < -fastThreshold?1:-234 > fastThreshold?2:0),
							uchar(-233 < -fastThreshold?1:-233 > fastThreshold?2:0),
							uchar(-232 < -fastThreshold?1:-232 > fastThreshold?2:0),
							uchar(-231 < -fastThreshold?1:-231 > fastThreshold?2:0),
							uchar(-230 < -fastThreshold?1:-230 > fastThreshold?2:0),
							uchar(-229 < -fastThreshold?1:-229 > fastThreshold?2:0),
							uchar(-228 < -fastThreshold?1:-228 > fastThreshold?2:0),
							uchar(-227 < -fastThreshold?1:-227 > fastThreshold?2:0),
							uchar(-226 < -fastThreshold?1:-226 > fastThreshold?2:0),
							uchar(-225 < -fastThreshold?1:-225 > fastThreshold?2:0),
							uchar(-224 < -fastThreshold?1:-224 > fastThreshold?2:0),
							uchar(-223 < -fastThreshold?1:-223 > fastThreshold?2:0),
							uchar(-222 < -fastThreshold?1:-222 > fastThreshold?2:0),
							uchar(-221 < -fastThreshold?1:-221 > fastThreshold?2:0),
							uchar(-220 < -fastThreshold?1:-220 > fastThreshold?2:0),
							uchar(-219 < -fastThreshold?1:-219 > fastThreshold?2:0),
							uchar(-218 < -fastThreshold?1:-218 > fastThreshold?2:0),
							uchar(-217 < -fastThreshold?1:-217 > fastThreshold?2:0),
							uchar(-216 < -fastThreshold?1:-216 > fastThreshold?2:0),
							uchar(-215 < -fastThreshold?1:-215 > fastThreshold?2:0),
							uchar(-214 < -fastThreshold?1:-214 > fastThreshold?2:0),
							uchar(-213 < -fastThreshold?1:-213 > fastThreshold?2:0),
							uchar(-212 < -fastThreshold?1:-212 > fastThreshold?2:0),
							uchar(-211 < -fastThreshold?1:-211 > fastThreshold?2:0),
							uchar(-210 < -fastThreshold?1:-210 > fastThreshold?2:0),
							uchar(-209 < -fastThreshold?1:-209 > fastThreshold?2:0),
							uchar(-208 < -fastThreshold?1:-208 > fastThreshold?2:0),
							uchar(-207 < -fastThreshold?1:-207 > fastThreshold?2:0),
							uchar(-206 < -fastThreshold?1:-206 > fastThreshold?2:0),
							uchar(-205 < -fastThreshold?1:-205 > fastThreshold?2:0),
							uchar(-204 < -fastThreshold?1:-204 > fastThreshold?2:0),
							uchar(-203 < -fastThreshold?1:-203 > fastThreshold?2:0),
							uchar(-202 < -fastThreshold?1:-202 > fastThreshold?2:0),
							uchar(-201 < -fastThreshold?1:-201 > fastThreshold?2:0),
							uchar(-200 < -fastThreshold?1:-200 > fastThreshold?2:0),
							uchar(-199 < -fastThreshold?1:-199 > fastThreshold?2:0),
							uchar(-198 < -fastThreshold?1:-198 > fastThreshold?2:0),
							uchar(-197 < -fastThreshold?1:-197 > fastThreshold?2:0),
							uchar(-196 < -fastThreshold?1:-196 > fastThreshold?2:0),
							uchar(-195 < -fastThreshold?1:-195 > fastThreshold?2:0),
							uchar(-194 < -fastThreshold?1:-194 > fastThreshold?2:0),
							uchar(-193 < -fastThreshold?1:-193 > fastThreshold?2:0),
							uchar(-192 < -fastThreshold?1:-192 > fastThreshold?2:0),
							uchar(-191 < -fastThreshold?1:-191 > fastThreshold?2:0),
							uchar(-190 < -fastThreshold?1:-190 > fastThreshold?2:0),
							uchar(-189 < -fastThreshold?1:-189 > fastThreshold?2:0),
							uchar(-188 < -fastThreshold?1:-188 > fastThreshold?2:0),
							uchar(-187 < -fastThreshold?1:-187 > fastThreshold?2:0),
							uchar(-186 < -fastThreshold?1:-186 > fastThreshold?2:0),
							uchar(-185 < -fastThreshold?1:-185 > fastThreshold?2:0),
							uchar(-184 < -fastThreshold?1:-184 > fastThreshold?2:0),
							uchar(-183 < -fastThreshold?1:-183 > fastThreshold?2:0),
							uchar(-182 < -fastThreshold?1:-182 > fastThreshold?2:0),
							uchar(-181 < -fastThreshold?1:-181 > fastThreshold?2:0),
							uchar(-180 < -fastThreshold?1:-180 > fastThreshold?2:0),
							uchar(-179 < -fastThreshold?1:-179 > fastThreshold?2:0),
							uchar(-178 < -fastThreshold?1:-178 > fastThreshold?2:0),
							uchar(-177 < -fastThreshold?1:-177 > fastThreshold?2:0),
							uchar(-176 < -fastThreshold?1:-176 > fastThreshold?2:0),
							uchar(-175 < -fastThreshold?1:-175 > fastThreshold?2:0),
							uchar(-174 < -fastThreshold?1:-174 > fastThreshold?2:0),
							uchar(-173 < -fastThreshold?1:-173 > fastThreshold?2:0),
							uchar(-172 < -fastThreshold?1:-172 > fastThreshold?2:0),
							uchar(-171 < -fastThreshold?1:-171 > fastThreshold?2:0),
							uchar(-170 < -fastThreshold?1:-170 > fastThreshold?2:0),
							uchar(-169 < -fastThreshold?1:-169 > fastThreshold?2:0),
							uchar(-168 < -fastThreshold?1:-168 > fastThreshold?2:0),
							uchar(-167 < -fastThreshold?1:-167 > fastThreshold?2:0),
							uchar(-166 < -fastThreshold?1:-166 > fastThreshold?2:0),
							uchar(-165 < -fastThreshold?1:-165 > fastThreshold?2:0),
							uchar(-164 < -fastThreshold?1:-164 > fastThreshold?2:0),
							uchar(-163 < -fastThreshold?1:-163 > fastThreshold?2:0),
							uchar(-162 < -fastThreshold?1:-162 > fastThreshold?2:0),
							uchar(-161 < -fastThreshold?1:-161 > fastThreshold?2:0),
							uchar(-160 < -fastThreshold?1:-160 > fastThreshold?2:0),
							uchar(-159 < -fastThreshold?1:-159 > fastThreshold?2:0),
							uchar(-158 < -fastThreshold?1:-158 > fastThreshold?2:0),
							uchar(-157 < -fastThreshold?1:-157 > fastThreshold?2:0),
							uchar(-156 < -fastThreshold?1:-156 > fastThreshold?2:0),
							uchar(-155 < -fastThreshold?1:-155 > fastThreshold?2:0),
							uchar(-154 < -fastThreshold?1:-154 > fastThreshold?2:0),
							uchar(-153 < -fastThreshold?1:-153 > fastThreshold?2:0),
							uchar(-152 < -fastThreshold?1:-152 > fastThreshold?2:0),
							uchar(-151 < -fastThreshold?1:-151 > fastThreshold?2:0),
							uchar(-150 < -fastThreshold?1:-150 > fastThreshold?2:0),
							uchar(-149 < -fastThreshold?1:-149 > fastThreshold?2:0),
							uchar(-148 < -fastThreshold?1:-148 > fastThreshold?2:0),
							uchar(-147 < -fastThreshold?1:-147 > fastThreshold?2:0),
							uchar(-146 < -fastThreshold?1:-146 > fastThreshold?2:0),
							uchar(-145 < -fastThreshold?1:-145 > fastThreshold?2:0),
							uchar(-144 < -fastThreshold?1:-144 > fastThreshold?2:0),
							uchar(-143 < -fastThreshold?1:-143 > fastThreshold?2:0),
							uchar(-142 < -fastThreshold?1:-142 > fastThreshold?2:0),
							uchar(-141 < -fastThreshold?1:-141 > fastThreshold?2:0),
							uchar(-140 < -fastThreshold?1:-140 > fastThreshold?2:0),
							uchar(-139 < -fastThreshold?1:-139 > fastThreshold?2:0),
							uchar(-138 < -fastThreshold?1:-138 > fastThreshold?2:0),
							uchar(-137 < -fastThreshold?1:-137 > fastThreshold?2:0),
							uchar(-136 < -fastThreshold?1:-136 > fastThreshold?2:0),
							uchar(-135 < -fastThreshold?1:-135 > fastThreshold?2:0),
							uchar(-134 < -fastThreshold?1:-134 > fastThreshold?2:0),
							uchar(-133 < -fastThreshold?1:-133 > fastThreshold?2:0),
							uchar(-132 < -fastThreshold?1:-132 > fastThreshold?2:0),
							uchar(-131 < -fastThreshold?1:-131 > fastThreshold?2:0),
							uchar(-130 < -fastThreshold?1:-130 > fastThreshold?2:0),
							uchar(-129 < -fastThreshold?1:-129 > fastThreshold?2:0),
							uchar(-128 < -fastThreshold?1:-128 > fastThreshold?2:0),
							uchar(-127 < -fastThreshold?1:-127 > fastThreshold?2:0),
							uchar(-126 < -fastThreshold?1:-126 > fastThreshold?2:0),
							uchar(-125 < -fastThreshold?1:-125 > fastThreshold?2:0),
							uchar(-124 < -fastThreshold?1:-124 > fastThreshold?2:0),
							uchar(-123 < -fastThreshold?1:-123 > fastThreshold?2:0),
							uchar(-122 < -fastThreshold?1:-122 > fastThreshold?2:0),
							uchar(-121 < -fastThreshold?1:-121 > fastThreshold?2:0),
							uchar(-120 < -fastThreshold?1:-120 > fastThreshold?2:0),
							uchar(-119 < -fastThreshold?1:-119 > fastThreshold?2:0),
							uchar(-118 < -fastThreshold?1:-118 > fastThreshold?2:0),
							uchar(-117 < -fastThreshold?1:-117 > fastThreshold?2:0),
							uchar(-116 < -fastThreshold?1:-116 > fastThreshold?2:0),
							uchar(-115 < -fastThreshold?1:-115 > fastThreshold?2:0),
							uchar(-114 < -fastThreshold?1:-114 > fastThreshold?2:0),
							uchar(-113 < -fastThreshold?1:-113 > fastThreshold?2:0),
							uchar(-112 < -fastThreshold?1:-112 > fastThreshold?2:0),
							uchar(-111 < -fastThreshold?1:-111 > fastThreshold?2:0),
							uchar(-110 < -fastThreshold?1:-110 > fastThreshold?2:0),
							uchar(-109 < -fastThreshold?1:-109 > fastThreshold?2:0),
							uchar(-108 < -fastThreshold?1:-108 > fastThreshold?2:0),
							uchar(-107 < -fastThreshold?1:-107 > fastThreshold?2:0),
							uchar(-106 < -fastThreshold?1:-106 > fastThreshold?2:0),
							uchar(-105 < -fastThreshold?1:-105 > fastThreshold?2:0),
							uchar(-104 < -fastThreshold?1:-104 > fastThreshold?2:0),
							uchar(-103 < -fastThreshold?1:-103 > fastThreshold?2:0),
							uchar(-102 < -fastThreshold?1:-102 > fastThreshold?2:0),
							uchar(-101 < -fastThreshold?1:-101 > fastThreshold?2:0),
							uchar(-100 < -fastThreshold?1:-100 > fastThreshold?2:0),
							uchar(-99 < -fastThreshold?1:-99 > fastThreshold?2:0),
							uchar(-98 < -fastThreshold?1:-98 > fastThreshold?2:0),
							uchar(-97 < -fastThreshold?1:-97 > fastThreshold?2:0),
							uchar(-96 < -fastThreshold?1:-96 > fastThreshold?2:0),
							uchar(-95 < -fastThreshold?1:-95 > fastThreshold?2:0),
							uchar(-94 < -fastThreshold?1:-94 > fastThreshold?2:0),
							uchar(-93 < -fastThreshold?1:-93 > fastThreshold?2:0),
							uchar(-92 < -fastThreshold?1:-92 > fastThreshold?2:0),
							uchar(-91 < -fastThreshold?1:-91 > fastThreshold?2:0),
							uchar(-90 < -fastThreshold?1:-90 > fastThreshold?2:0),
							uchar(-89 < -fastThreshold?1:-89 > fastThreshold?2:0),
							uchar(-88 < -fastThreshold?1:-88 > fastThreshold?2:0),
							uchar(-87 < -fastThreshold?1:-87 > fastThreshold?2:0),
							uchar(-86 < -fastThreshold?1:-86 > fastThreshold?2:0),
							uchar(-85 < -fastThreshold?1:-85 > fastThreshold?2:0),
							uchar(-84 < -fastThreshold?1:-84 > fastThreshold?2:0),
							uchar(-83 < -fastThreshold?1:-83 > fastThreshold?2:0),
							uchar(-82 < -fastThreshold?1:-82 > fastThreshold?2:0),
							uchar(-81 < -fastThreshold?1:-81 > fastThreshold?2:0),
							uchar(-80 < -fastThreshold?1:-80 > fastThreshold?2:0),
							uchar(-79 < -fastThreshold?1:-79 > fastThreshold?2:0),
							uchar(-78 < -fastThreshold?1:-78 > fastThreshold?2:0),
							uchar(-77 < -fastThreshold?1:-77 > fastThreshold?2:0),
							uchar(-76 < -fastThreshold?1:-76 > fastThreshold?2:0),
							uchar(-75 < -fastThreshold?1:-75 > fastThreshold?2:0),
							uchar(-74 < -fastThreshold?1:-74 > fastThreshold?2:0),
							uchar(-73 < -fastThreshold?1:-73 > fastThreshold?2:0),
							uchar(-72 < -fastThreshold?1:-72 > fastThreshold?2:0),
							uchar(-71 < -fastThreshold?1:-71 > fastThreshold?2:0),
							uchar(-70 < -fastThreshold?1:-70 > fastThreshold?2:0),
							uchar(-69 < -fastThreshold?1:-69 > fastThreshold?2:0),
							uchar(-68 < -fastThreshold?1:-68 > fastThreshold?2:0),
							uchar(-67 < -fastThreshold?1:-67 > fastThreshold?2:0),
							uchar(-66 < -fastThreshold?1:-66 > fastThreshold?2:0),
							uchar(-65 < -fastThreshold?1:-65 > fastThreshold?2:0),
							uchar(-64 < -fastThreshold?1:-64 > fastThreshold?2:0),
							uchar(-63 < -fastThreshold?1:-63 > fastThreshold?2:0),
							uchar(-62 < -fastThreshold?1:-62 > fastThreshold?2:0),
							uchar(-61 < -fastThreshold?1:-61 > fastThreshold?2:0),
							uchar(-60 < -fastThreshold?1:-60 > fastThreshold?2:0),
							uchar(-59 < -fastThreshold?1:-59 > fastThreshold?2:0),
							uchar(-58 < -fastThreshold?1:-58 > fastThreshold?2:0),
							uchar(-57 < -fastThreshold?1:-57 > fastThreshold?2:0),
							uchar(-56 < -fastThreshold?1:-56 > fastThreshold?2:0),
							uchar(-55 < -fastThreshold?1:-55 > fastThreshold?2:0),
							uchar(-54 < -fastThreshold?1:-54 > fastThreshold?2:0),
							uchar(-53 < -fastThreshold?1:-53 > fastThreshold?2:0),
							uchar(-52 < -fastThreshold?1:-52 > fastThreshold?2:0),
							uchar(-51 < -fastThreshold?1:-51 > fastThreshold?2:0),
							uchar(-50 < -fastThreshold?1:-50 > fastThreshold?2:0),
							uchar(-49 < -fastThreshold?1:-49 > fastThreshold?2:0),
							uchar(-48 < -fastThreshold?1:-48 > fastThreshold?2:0),
							uchar(-47 < -fastThreshold?1:-47 > fastThreshold?2:0),
							uchar(-46 < -fastThreshold?1:-46 > fastThreshold?2:0),
							uchar(-45 < -fastThreshold?1:-45 > fastThreshold?2:0),
							uchar(-44 < -fastThreshold?1:-44 > fastThreshold?2:0),
							uchar(-43 < -fastThreshold?1:-43 > fastThreshold?2:0),
							uchar(-42 < -fastThreshold?1:-42 > fastThreshold?2:0),
							uchar(-41 < -fastThreshold?1:-41 > fastThreshold?2:0),
							uchar(-40 < -fastThreshold?1:-40 > fastThreshold?2:0),
							uchar(-39 < -fastThreshold?1:-39 > fastThreshold?2:0),
							uchar(-38 < -fastThreshold?1:-38 > fastThreshold?2:0),
							uchar(-37 < -fastThreshold?1:-37 > fastThreshold?2:0),
							uchar(-36 < -fastThreshold?1:-36 > fastThreshold?2:0),
							uchar(-35 < -fastThreshold?1:-35 > fastThreshold?2:0),
							uchar(-34 < -fastThreshold?1:-34 > fastThreshold?2:0),
							uchar(-33 < -fastThreshold?1:-33 > fastThreshold?2:0),
							uchar(-32 < -fastThreshold?1:-32 > fastThreshold?2:0),
							uchar(-31 < -fastThreshold?1:-31 > fastThreshold?2:0),
							uchar(-30 < -fastThreshold?1:-30 > fastThreshold?2:0),
							uchar(-29 < -fastThreshold?1:-29 > fastThreshold?2:0),
							uchar(-28 < -fastThreshold?1:-28 > fastThreshold?2:0),
							uchar(-27 < -fastThreshold?1:-27 > fastThreshold?2:0),
							uchar(-26 < -fastThreshold?1:-26 > fastThreshold?2:0),
							uchar(-25 < -fastThreshold?1:-25 > fastThreshold?2:0),
							uchar(-24 < -fastThreshold?1:-24 > fastThreshold?2:0),
							uchar(-23 < -fastThreshold?1:-23 > fastThreshold?2:0),
							uchar(-22 < -fastThreshold?1:-22 > fastThreshold?2:0),
							uchar(-21 < -fastThreshold?1:-21 > fastThreshold?2:0),
							uchar(-20 < -fastThreshold?1:-20 > fastThreshold?2:0),
							uchar(-19 < -fastThreshold?1:-19 > fastThreshold?2:0),
							uchar(-18 < -fastThreshold?1:-18 > fastThreshold?2:0),
							uchar(-17 < -fastThreshold?1:-17 > fastThreshold?2:0),
							uchar(-16 < -fastThreshold?1:-16 > fastThreshold?2:0),
							uchar(-15 < -fastThreshold?1:-15 > fastThreshold?2:0),
							uchar(-14 < -fastThreshold?1:-14 > fastThreshold?2:0),
							uchar(-13 < -fastThreshold?1:-13 > fastThreshold?2:0),
							uchar(-12 < -fastThreshold?1:-12 > fastThreshold?2:0),
							uchar(-11 < -fastThreshold?1:-11 > fastThreshold?2:0),
							uchar(-10 < -fastThreshold?1:-10 > fastThreshold?2:0),
							uchar(-9 < -fastThreshold?1:-9 > fastThreshold?2:0),
							uchar(-8 < -fastThreshold?1:-8 > fastThreshold?2:0),
							uchar(-7 < -fastThreshold?1:-7 > fastThreshold?2:0),
							uchar(-6 < -fastThreshold?1:-6 > fastThreshold?2:0),
							uchar(-5 < -fastThreshold?1:-5 > fastThreshold?2:0),
							uchar(-4 < -fastThreshold?1:-4 > fastThreshold?2:0),
							uchar(-3 < -fastThreshold?1:-3 > fastThreshold?2:0),
							uchar(-2 < -fastThreshold?1:-2 > fastThreshold?2:0),
							uchar(-1 < -fastThreshold?1:-1 > fastThreshold?2:0),
							uchar(0 < -fastThreshold?1:0 > fastThreshold?2:0),
							uchar(1 < -fastThreshold?1:1 > fastThreshold?2:0),
							uchar(2 < -fastThreshold?1:2 > fastThreshold?2:0),
							uchar(3 < -fastThreshold?1:3 > fastThreshold?2:0),
							uchar(4 < -fastThreshold?1:4 > fastThreshold?2:0),
							uchar(5 < -fastThreshold?1:5 > fastThreshold?2:0),
							uchar(6 < -fastThreshold?1:6 > fastThreshold?2:0),
							uchar(7 < -fastThreshold?1:7 > fastThreshold?2:0),
							uchar(8 < -fastThreshold?1:8 > fastThreshold?2:0),
							uchar(9 < -fastThreshold?1:9 > fastThreshold?2:0),
							uchar(10 < -fastThreshold?1:10 > fastThreshold?2:0),
							uchar(11 < -fastThreshold?1:11 > fastThreshold?2:0),
							uchar(12 < -fastThreshold?1:12 > fastThreshold?2:0),
							uchar(13 < -fastThreshold?1:13 > fastThreshold?2:0),
							uchar(14 < -fastThreshold?1:14 > fastThreshold?2:0),
							uchar(15 < -fastThreshold?1:15 > fastThreshold?2:0),
							uchar(16 < -fastThreshold?1:16 > fastThreshold?2:0),
							uchar(17 < -fastThreshold?1:17 > fastThreshold?2:0),
							uchar(18 < -fastThreshold?1:18 > fastThreshold?2:0),
							uchar(19 < -fastThreshold?1:19 > fastThreshold?2:0),
							uchar(20 < -fastThreshold?1:20 > fastThreshold?2:0),
							uchar(21 < -fastThreshold?1:21 > fastThreshold?2:0),
							uchar(22 < -fastThreshold?1:22 > fastThreshold?2:0),
							uchar(23 < -fastThreshold?1:23 > fastThreshold?2:0),
							uchar(24 < -fastThreshold?1:24 > fastThreshold?2:0),
							uchar(25 < -fastThreshold?1:25 > fastThreshold?2:0),
							uchar(26 < -fastThreshold?1:26 > fastThreshold?2:0),
							uchar(27 < -fastThreshold?1:27 > fastThreshold?2:0),
							uchar(28 < -fastThreshold?1:28 > fastThreshold?2:0),
							uchar(29 < -fastThreshold?1:29 > fastThreshold?2:0),
							uchar(30 < -fastThreshold?1:30 > fastThreshold?2:0),
							uchar(31 < -fastThreshold?1:31 > fastThreshold?2:0),
							uchar(32 < -fastThreshold?1:32 > fastThreshold?2:0),
							uchar(33 < -fastThreshold?1:33 > fastThreshold?2:0),
							uchar(34 < -fastThreshold?1:34 > fastThreshold?2:0),
							uchar(35 < -fastThreshold?1:35 > fastThreshold?2:0),
							uchar(36 < -fastThreshold?1:36 > fastThreshold?2:0),
							uchar(37 < -fastThreshold?1:37 > fastThreshold?2:0),
							uchar(38 < -fastThreshold?1:38 > fastThreshold?2:0),
							uchar(39 < -fastThreshold?1:39 > fastThreshold?2:0),
							uchar(40 < -fastThreshold?1:40 > fastThreshold?2:0),
							uchar(41 < -fastThreshold?1:41 > fastThreshold?2:0),
							uchar(42 < -fastThreshold?1:42 > fastThreshold?2:0),
							uchar(43 < -fastThreshold?1:43 > fastThreshold?2:0),
							uchar(44 < -fastThreshold?1:44 > fastThreshold?2:0),
							uchar(45 < -fastThreshold?1:45 > fastThreshold?2:0),
							uchar(46 < -fastThreshold?1:46 > fastThreshold?2:0),
							uchar(47 < -fastThreshold?1:47 > fastThreshold?2:0),
							uchar(48 < -fastThreshold?1:48 > fastThreshold?2:0),
							uchar(49 < -fastThreshold?1:49 > fastThreshold?2:0),
							uchar(50 < -fastThreshold?1:50 > fastThreshold?2:0),
							uchar(51 < -fastThreshold?1:51 > fastThreshold?2:0),
							uchar(52 < -fastThreshold?1:52 > fastThreshold?2:0),
							uchar(53 < -fastThreshold?1:53 > fastThreshold?2:0),
							uchar(54 < -fastThreshold?1:54 > fastThreshold?2:0),
							uchar(55 < -fastThreshold?1:55 > fastThreshold?2:0),
							uchar(56 < -fastThreshold?1:56 > fastThreshold?2:0),
							uchar(57 < -fastThreshold?1:57 > fastThreshold?2:0),
							uchar(58 < -fastThreshold?1:58 > fastThreshold?2:0),
							uchar(59 < -fastThreshold?1:59 > fastThreshold?2:0),
							uchar(60 < -fastThreshold?1:60 > fastThreshold?2:0),
							uchar(61 < -fastThreshold?1:61 > fastThreshold?2:0),
							uchar(62 < -fastThreshold?1:62 > fastThreshold?2:0),
							uchar(63 < -fastThreshold?1:63 > fastThreshold?2:0),
							uchar(64 < -fastThreshold?1:64 > fastThreshold?2:0),
							uchar(65 < -fastThreshold?1:65 > fastThreshold?2:0),
							uchar(66 < -fastThreshold?1:66 > fastThreshold?2:0),
							uchar(67 < -fastThreshold?1:67 > fastThreshold?2:0),
							uchar(68 < -fastThreshold?1:68 > fastThreshold?2:0),
							uchar(69 < -fastThreshold?1:69 > fastThreshold?2:0),
							uchar(70 < -fastThreshold?1:70 > fastThreshold?2:0),
							uchar(71 < -fastThreshold?1:71 > fastThreshold?2:0),
							uchar(72 < -fastThreshold?1:72 > fastThreshold?2:0),
							uchar(73 < -fastThreshold?1:73 > fastThreshold?2:0),
							uchar(74 < -fastThreshold?1:74 > fastThreshold?2:0),
							uchar(75 < -fastThreshold?1:75 > fastThreshold?2:0),
							uchar(76 < -fastThreshold?1:76 > fastThreshold?2:0),
							uchar(77 < -fastThreshold?1:77 > fastThreshold?2:0),
							uchar(78 < -fastThreshold?1:78 > fastThreshold?2:0),
							uchar(79 < -fastThreshold?1:79 > fastThreshold?2:0),
							uchar(80 < -fastThreshold?1:80 > fastThreshold?2:0),
							uchar(81 < -fastThreshold?1:81 > fastThreshold?2:0),
							uchar(82 < -fastThreshold?1:82 > fastThreshold?2:0),
							uchar(83 < -fastThreshold?1:83 > fastThreshold?2:0),
							uchar(84 < -fastThreshold?1:84 > fastThreshold?2:0),
							uchar(85 < -fastThreshold?1:85 > fastThreshold?2:0),
							uchar(86 < -fastThreshold?1:86 > fastThreshold?2:0),
							uchar(87 < -fastThreshold?1:87 > fastThreshold?2:0),
							uchar(88 < -fastThreshold?1:88 > fastThreshold?2:0),
							uchar(89 < -fastThreshold?1:89 > fastThreshold?2:0),
							uchar(90 < -fastThreshold?1:90 > fastThreshold?2:0),
							uchar(91 < -fastThreshold?1:91 > fastThreshold?2:0),
							uchar(92 < -fastThreshold?1:92 > fastThreshold?2:0),
							uchar(93 < -fastThreshold?1:93 > fastThreshold?2:0),
							uchar(94 < -fastThreshold?1:94 > fastThreshold?2:0),
							uchar(95 < -fastThreshold?1:95 > fastThreshold?2:0),
							uchar(96 < -fastThreshold?1:96 > fastThreshold?2:0),
							uchar(97 < -fastThreshold?1:97 > fastThreshold?2:0),
							uchar(98 < -fastThreshold?1:98 > fastThreshold?2:0),
							uchar(99 < -fastThreshold?1:99 > fastThreshold?2:0),
							uchar(100 < -fastThreshold?1:100 > fastThreshold?2:0),
							uchar(101 < -fastThreshold?1:101 > fastThreshold?2:0),
							uchar(102 < -fastThreshold?1:102 > fastThreshold?2:0),
							uchar(103 < -fastThreshold?1:103 > fastThreshold?2:0),
							uchar(104 < -fastThreshold?1:104 > fastThreshold?2:0),
							uchar(105 < -fastThreshold?1:105 > fastThreshold?2:0),
							uchar(106 < -fastThreshold?1:106 > fastThreshold?2:0),
							uchar(107 < -fastThreshold?1:107 > fastThreshold?2:0),
							uchar(108 < -fastThreshold?1:108 > fastThreshold?2:0),
							uchar(109 < -fastThreshold?1:109 > fastThreshold?2:0),
							uchar(110 < -fastThreshold?1:110 > fastThreshold?2:0),
							uchar(111 < -fastThreshold?1:111 > fastThreshold?2:0),
							uchar(112 < -fastThreshold?1:112 > fastThreshold?2:0),
							uchar(113 < -fastThreshold?1:113 > fastThreshold?2:0),
							uchar(114 < -fastThreshold?1:114 > fastThreshold?2:0),
							uchar(115 < -fastThreshold?1:115 > fastThreshold?2:0),
							uchar(116 < -fastThreshold?1:116 > fastThreshold?2:0),
							uchar(117 < -fastThreshold?1:117 > fastThreshold?2:0),
							uchar(118 < -fastThreshold?1:118 > fastThreshold?2:0),
							uchar(119 < -fastThreshold?1:119 > fastThreshold?2:0),
							uchar(120 < -fastThreshold?1:120 > fastThreshold?2:0),
							uchar(121 < -fastThreshold?1:121 > fastThreshold?2:0),
							uchar(122 < -fastThreshold?1:122 > fastThreshold?2:0),
							uchar(123 < -fastThreshold?1:123 > fastThreshold?2:0),
							uchar(124 < -fastThreshold?1:124 > fastThreshold?2:0),
							uchar(125 < -fastThreshold?1:125 > fastThreshold?2:0),
							uchar(126 < -fastThreshold?1:126 > fastThreshold?2:0),
							uchar(127 < -fastThreshold?1:127 > fastThreshold?2:0),
							uchar(128 < -fastThreshold?1:128 > fastThreshold?2:0),
							uchar(129 < -fastThreshold?1:129 > fastThreshold?2:0),
							uchar(130 < -fastThreshold?1:130 > fastThreshold?2:0),
							uchar(131 < -fastThreshold?1:131 > fastThreshold?2:0),
							uchar(132 < -fastThreshold?1:132 > fastThreshold?2:0),
							uchar(133 < -fastThreshold?1:133 > fastThreshold?2:0),
							uchar(134 < -fastThreshold?1:134 > fastThreshold?2:0),
							uchar(135 < -fastThreshold?1:135 > fastThreshold?2:0),
							uchar(136 < -fastThreshold?1:136 > fastThreshold?2:0),
							uchar(137 < -fastThreshold?1:137 > fastThreshold?2:0),
							uchar(138 < -fastThreshold?1:138 > fastThreshold?2:0),
							uchar(139 < -fastThreshold?1:139 > fastThreshold?2:0),
							uchar(140 < -fastThreshold?1:140 > fastThreshold?2:0),
							uchar(141 < -fastThreshold?1:141 > fastThreshold?2:0),
							uchar(142 < -fastThreshold?1:142 > fastThreshold?2:0),
							uchar(143 < -fastThreshold?1:143 > fastThreshold?2:0),
							uchar(144 < -fastThreshold?1:144 > fastThreshold?2:0),
							uchar(145 < -fastThreshold?1:145 > fastThreshold?2:0),
							uchar(146 < -fastThreshold?1:146 > fastThreshold?2:0),
							uchar(147 < -fastThreshold?1:147 > fastThreshold?2:0),
							uchar(148 < -fastThreshold?1:148 > fastThreshold?2:0),
							uchar(149 < -fastThreshold?1:149 > fastThreshold?2:0),
							uchar(150 < -fastThreshold?1:150 > fastThreshold?2:0),
							uchar(151 < -fastThreshold?1:151 > fastThreshold?2:0),
							uchar(152 < -fastThreshold?1:152 > fastThreshold?2:0),
							uchar(153 < -fastThreshold?1:153 > fastThreshold?2:0),
							uchar(154 < -fastThreshold?1:154 > fastThreshold?2:0),
							uchar(155 < -fastThreshold?1:155 > fastThreshold?2:0),
							uchar(156 < -fastThreshold?1:156 > fastThreshold?2:0),
							uchar(157 < -fastThreshold?1:157 > fastThreshold?2:0),
							uchar(158 < -fastThreshold?1:158 > fastThreshold?2:0),
							uchar(159 < -fastThreshold?1:159 > fastThreshold?2:0),
							uchar(160 < -fastThreshold?1:160 > fastThreshold?2:0),
							uchar(161 < -fastThreshold?1:161 > fastThreshold?2:0),
							uchar(162 < -fastThreshold?1:162 > fastThreshold?2:0),
							uchar(163 < -fastThreshold?1:163 > fastThreshold?2:0),
							uchar(164 < -fastThreshold?1:164 > fastThreshold?2:0),
							uchar(165 < -fastThreshold?1:165 > fastThreshold?2:0),
							uchar(166 < -fastThreshold?1:166 > fastThreshold?2:0),
							uchar(167 < -fastThreshold?1:167 > fastThreshold?2:0),
							uchar(168 < -fastThreshold?1:168 > fastThreshold?2:0),
							uchar(169 < -fastThreshold?1:169 > fastThreshold?2:0),
							uchar(170 < -fastThreshold?1:170 > fastThreshold?2:0),
							uchar(171 < -fastThreshold?1:171 > fastThreshold?2:0),
							uchar(172 < -fastThreshold?1:172 > fastThreshold?2:0),
							uchar(173 < -fastThreshold?1:173 > fastThreshold?2:0),
							uchar(174 < -fastThreshold?1:174 > fastThreshold?2:0),
							uchar(175 < -fastThreshold?1:175 > fastThreshold?2:0),
							uchar(176 < -fastThreshold?1:176 > fastThreshold?2:0),
							uchar(177 < -fastThreshold?1:177 > fastThreshold?2:0),
							uchar(178 < -fastThreshold?1:178 > fastThreshold?2:0),
							uchar(179 < -fastThreshold?1:179 > fastThreshold?2:0),
							uchar(180 < -fastThreshold?1:180 > fastThreshold?2:0),
							uchar(181 < -fastThreshold?1:181 > fastThreshold?2:0),
							uchar(182 < -fastThreshold?1:182 > fastThreshold?2:0),
							uchar(183 < -fastThreshold?1:183 > fastThreshold?2:0),
							uchar(184 < -fastThreshold?1:184 > fastThreshold?2:0),
							uchar(185 < -fastThreshold?1:185 > fastThreshold?2:0),
							uchar(186 < -fastThreshold?1:186 > fastThreshold?2:0),
							uchar(187 < -fastThreshold?1:187 > fastThreshold?2:0),
							uchar(188 < -fastThreshold?1:188 > fastThreshold?2:0),
							uchar(189 < -fastThreshold?1:189 > fastThreshold?2:0),
							uchar(190 < -fastThreshold?1:190 > fastThreshold?2:0),
							uchar(191 < -fastThreshold?1:191 > fastThreshold?2:0),
							uchar(192 < -fastThreshold?1:192 > fastThreshold?2:0),
							uchar(193 < -fastThreshold?1:193 > fastThreshold?2:0),
							uchar(194 < -fastThreshold?1:194 > fastThreshold?2:0),
							uchar(195 < -fastThreshold?1:195 > fastThreshold?2:0),
							uchar(196 < -fastThreshold?1:196 > fastThreshold?2:0),
							uchar(197 < -fastThreshold?1:197 > fastThreshold?2:0),
							uchar(198 < -fastThreshold?1:198 > fastThreshold?2:0),
							uchar(199 < -fastThreshold?1:199 > fastThreshold?2:0),
							uchar(200 < -fastThreshold?1:200 > fastThreshold?2:0),
							uchar(201 < -fastThreshold?1:201 > fastThreshold?2:0),
							uchar(202 < -fastThreshold?1:202 > fastThreshold?2:0),
							uchar(203 < -fastThreshold?1:203 > fastThreshold?2:0),
							uchar(204 < -fastThreshold?1:204 > fastThreshold?2:0),
							uchar(205 < -fastThreshold?1:205 > fastThreshold?2:0),
							uchar(206 < -fastThreshold?1:206 > fastThreshold?2:0),
							uchar(207 < -fastThreshold?1:207 > fastThreshold?2:0),
							uchar(208 < -fastThreshold?1:208 > fastThreshold?2:0),
							uchar(209 < -fastThreshold?1:209 > fastThreshold?2:0),
							uchar(210 < -fastThreshold?1:210 > fastThreshold?2:0),
							uchar(211 < -fastThreshold?1:211 > fastThreshold?2:0),
							uchar(212 < -fastThreshold?1:212 > fastThreshold?2:0),
							uchar(213 < -fastThreshold?1:213 > fastThreshold?2:0),
							uchar(214 < -fastThreshold?1:214 > fastThreshold?2:0),
							uchar(215 < -fastThreshold?1:215 > fastThreshold?2:0),
							uchar(216 < -fastThreshold?1:216 > fastThreshold?2:0),
							uchar(217 < -fastThreshold?1:217 > fastThreshold?2:0),
							uchar(218 < -fastThreshold?1:218 > fastThreshold?2:0),
							uchar(219 < -fastThreshold?1:219 > fastThreshold?2:0),
							uchar(220 < -fastThreshold?1:220 > fastThreshold?2:0),
							uchar(221 < -fastThreshold?1:221 > fastThreshold?2:0),
							uchar(222 < -fastThreshold?1:222 > fastThreshold?2:0),
							uchar(223 < -fastThreshold?1:223 > fastThreshold?2:0),
							uchar(224 < -fastThreshold?1:224 > fastThreshold?2:0),
							uchar(225 < -fastThreshold?1:225 > fastThreshold?2:0),
							uchar(226 < -fastThreshold?1:226 > fastThreshold?2:0),
							uchar(227 < -fastThreshold?1:227 > fastThreshold?2:0),
							uchar(228 < -fastThreshold?1:228 > fastThreshold?2:0),
							uchar(229 < -fastThreshold?1:229 > fastThreshold?2:0),
							uchar(230 < -fastThreshold?1:230 > fastThreshold?2:0),
							uchar(231 < -fastThreshold?1:231 > fastThreshold?2:0),
							uchar(232 < -fastThreshold?1:232 > fastThreshold?2:0),
							uchar(233 < -fastThreshold?1:233 > fastThreshold?2:0),
							uchar(234 < -fastThreshold?1:234 > fastThreshold?2:0),
							uchar(235 < -fastThreshold?1:235 > fastThreshold?2:0),
							uchar(236 < -fastThreshold?1:236 > fastThreshold?2:0),
							uchar(237 < -fastThreshold?1:237 > fastThreshold?2:0),
							uchar(238 < -fastThreshold?1:238 > fastThreshold?2:0),
							uchar(239 < -fastThreshold?1:239 > fastThreshold?2:0),
							uchar(240 < -fastThreshold?1:240 > fastThreshold?2:0),
							uchar(241 < -fastThreshold?1:241 > fastThreshold?2:0),
							uchar(242 < -fastThreshold?1:242 > fastThreshold?2:0),
							uchar(243 < -fastThreshold?1:243 > fastThreshold?2:0),
							uchar(244 < -fastThreshold?1:244 > fastThreshold?2:0),
							uchar(245 < -fastThreshold?1:245 > fastThreshold?2:0),
							uchar(246 < -fastThreshold?1:246 > fastThreshold?2:0),
							uchar(247 < -fastThreshold?1:247 > fastThreshold?2:0),
							uchar(248 < -fastThreshold?1:248 > fastThreshold?2:0),
							uchar(249 < -fastThreshold?1:249 > fastThreshold?2:0),
							uchar(250 < -fastThreshold?1:250 > fastThreshold?2:0),
							uchar(251 < -fastThreshold?1:251 > fastThreshold?2:0),
							uchar(252 < -fastThreshold?1:252 > fastThreshold?2:0),
							uchar(253 < -fastThreshold?1:253 > fastThreshold?2:0),
							uchar(254 < -fastThreshold?1:254 > fastThreshold?2:0),
							uchar(255 < -fastThreshold?1:255 > fastThreshold?2:0)
						}
						{
						}

						bool doResponse() const override
						{
							return retentionPolicy!=RETAIN_ALL_NO_SCORE;
						}
						bool doAngles() const override
						{
							return false;
						}

						size_t getBorder() const override
						{
							return border;
						}

						void writeProperties(boost::property_tree::ptree& properties) const override
						{
							properties.put<std::string>("keyPointType",kpTypeStr<Fast>);
							properties.put<std::string>("shape",patternShapeStr<shape>);
							properties.put<std::string>("retentionPolicy",kpRetentionPolicyStr<retentionPolicy>);
							properties.put<size_t>("patternSize",patternSize);
							properties.put<bool>("compatibilityMode",compatibilityMode);
							properties.put<uchar>("fastThreshold",uchar(fastThreshold));
							properties.put<size_t>("border",border);
						}

						FastKeyPointGenerator<patternSize,shape,retentionPolicy,compatibilityMode,hal>* getKeyPointGenerator(size_t cols,size_t rows,float kpSize,int octave,float,float,int) const override;
					};

					template<KPRetentionPolicy scoreType=NON_MAX_SUPPRESSION>
					class FastKeyPointPostProcessor
					{
					private:
						size_t* const posBuffer;
						uchar* const scoreBuffer;
					public:
						constexpr
							FastKeyPointPostProcessor(size_t* posBuffer,uchar* scoreBuffer):
							posBuffer(posBuffer),
							scoreBuffer(scoreBuffer)
						{}

						constexpr
							void acceptScore(size_t x,uchar score)
						{
							scoreBuffer[posBuffer[posBuffer[-1]++]=x]=score;
						}
					};

					template<>
					class FastKeyPointPostProcessor<RETAIN_ALL>
					{
					private:
						std::vector<cv::KeyPoint>& keyPoints;
						const float y;
						const float kpSize;
						const int octave;
					public:
						constexpr
							FastKeyPointPostProcessor(std::vector<cv::KeyPoint>& keyPoints,float y,float kpSize,int octave):
							keyPoints(keyPoints),
							y(y),
							kpSize(kpSize),
							octave(octave)
						{}

						constexpr
							void acceptScore(size_t x,uchar score)
						{
							keyPoints.emplace_back(float(x),y,kpSize,-1.f,score,octave,-1);
						}
					};

					template<>
					class FastKeyPointPostProcessor<RETAIN_ALL_NO_SCORE>
					{
					private:
						std::vector<cv::KeyPoint>& keyPoints;
						const float y;
						const float kpSize;
						const int octave;
					public:

						constexpr
							FastKeyPointPostProcessor(std::vector<cv::KeyPoint>& keyPoints,float y,float kpSize,int octave):
							keyPoints(keyPoints),
							y(y),
							kpSize(kpSize),
							octave(octave)
						{}

						constexpr
							void acceptScore(size_t x,uchar)
						{
							keyPoints.emplace_back(float(x),y,kpSize,-1.f,0.f,octave,-1);
						}
					};

					template<size_t patternSize=16,PatternShape shape=Circle,KPRetentionPolicy retentionPolicy=NON_MAX_SUPPRESSION,bool compatibilityMode=true,HAL_FLAG hal=HAL_NONE>
					class FastPattern: public Pattern<patternSize,shape>
					{

					private:
						enum
						{
							bufferSize=patternSize+Pattern<patternSize,shape>::halfPatternSize+1
						};
						constexpr void initializeDiffArr(short(&diffArr)[bufferSize],const uchar* srcPtr) const noexcept
						{
							short v=srcPtr[0];
							uchar i=0;
							for(; i!=patternSize; ++i)
							{
								diffArr[i]=v-srcPtr[pixel[i]];
							}
							for(; i!=bufferSize; ++i)
							{
								diffArr[i]=diffArr[i-patternSize];
							}
						}
						constexpr void initializeDiffArr(short(&diffArr)[bufferSize+4],const uchar* srcPtr) const noexcept
						{
							short v=srcPtr[0];
							uchar i=0;
							for(; i!=patternSize; ++i)
							{
								diffArr[i]=v-srcPtr[pixel[i]];
							}
							for(; i!=bufferSize; ++i)
							{
								diffArr[i]=diffArr[i-patternSize];
							}
							diffArr[i]=diffArr[0];
							diffArr[++i]=diffArr[1];
							diffArr[++i]=diffArr[2];
							diffArr[++i]=diffArr[3];
						}

					public:
						const FastKeyPointGeneratorFactory<patternSize,shape,retentionPolicy,compatibilityMode,hal>* const factory;

						constexpr FastPattern(const int cols,const FastKeyPointGeneratorFactory<patternSize,shape,retentionPolicy,compatibilityMode,hal>* const factory) noexcept
							:Pattern<patternSize,shape>(cols)
							,factory(factory)
						{
						}

						template<size_t funcPatternSize=patternSize,KPRetentionPolicy funcRetentionPolicy=retentionPolicy,HAL_FLAG funcHal=hal> constexpr
							std::enable_if_t<funcPatternSize==8&&((funcRetentionPolicy==NON_MAX_SUPPRESSION||funcRetentionPolicy==RETAIN_ALL)&&!HalFlagInfo<funcHal>::hasSSE2),uchar>
							calculateScore(const uchar* srcPtr) const noexcept
						{
							short diffArr[bufferSize];
							initializeDiffArr(diffArr,srcPtr);
							short a0=factory->fastThreshold;
							for(uchar k=0; k!=funcPatternSize; k+=2)
							{
								short a=std::min(diffArr[k+1],diffArr[k+2]);
								if(a<=a0)
									continue;
								a=std::min(a,diffArr[k+3]);
								a=std::min(a,diffArr[k+4]);
								a0=std::max(a0,std::min(a,diffArr[k]));
								a0=std::max(a0,std::min(a,diffArr[k+5]));
							}
							short b0=-a0;
							for(uchar k=0; k < funcPatternSize; k+=2)
							{
								short b=std::max(diffArr[k+1],diffArr[k+2]);
								b=std::max(b,diffArr[k+3]);
								if(b>=b0)
									continue;
								b=std::max(b,diffArr[k+4]);
								b0=std::min(b0,std::max(b,diffArr[k]));
								b0=std::min(b0,std::max(b,diffArr[k+5]));
							}
							return uchar(-b0-1);
						}
						template<size_t funcPatternSize=patternSize,KPRetentionPolicy funcRetentionPolicy=retentionPolicy,HAL_FLAG funcHal=hal> constexpr
							std::enable_if_t<funcPatternSize==12&&((funcRetentionPolicy==NON_MAX_SUPPRESSION||funcRetentionPolicy==RETAIN_ALL)&&!HalFlagInfo<funcHal>::hasSSE2),uchar>
							calculateScore(const uchar* srcPtr) const noexcept
						{
							short diffArr[bufferSize];
							initializeDiffArr(diffArr,srcPtr);
							short a0=factory->fastThreshold;
							for(uchar k=0; k !=funcPatternSize; k+=2)
							{
								short a=std::min(diffArr[k+1],diffArr[k+2]);
								if(a<=a0)
									continue;
								a=std::min(a,diffArr[k+3]);
								a=std::min(a,diffArr[k+4]);
								a=std::min(a,diffArr[k+5]);
								a=std::min(a,diffArr[k+6]);
								a0=std::max(a0,std::min(a,diffArr[k]));
								a0=std::max(a0,std::min(a,diffArr[k+7]));
							}
							short b0=-a0;
							for(uchar k=0; k !=funcPatternSize; k+=2)
							{
								short b=std::max(diffArr[k+1],diffArr[k+2]);
								b=std::max(b,diffArr[k+3]);
								b=std::max(b,diffArr[k+4]);
								if(b>=b0)
									continue;
								b=std::max(b,diffArr[k+5]);
								b=std::max(b,diffArr[k+6]);
								b0=std::min(b0,std::max(b,diffArr[k]));
								b0=std::min(b0,std::max(b,diffArr[k+7]));
							}
							return uchar(-b0-1);
						}
						template<size_t funcPatternSize=patternSize,KPRetentionPolicy funcRetentionPolicy=retentionPolicy,HAL_FLAG funcHal=hal> constexpr
							std::enable_if_t<funcPatternSize==16&&((funcRetentionPolicy==NON_MAX_SUPPRESSION||funcRetentionPolicy==RETAIN_ALL)&&!HalFlagInfo<funcHal>::hasSSE2),uchar>
							calculateScore(const uchar* srcPtr) const noexcept
						{
							short diffArr[bufferSize];
							initializeDiffArr(diffArr,srcPtr);
							short a0=factory->fastThreshold;
							for(uchar k=0; k != funcPatternSize; k+=2)
							{
								short a=std::min(diffArr[k+1],diffArr[k+2]);
								a=std::min(a,diffArr[k+3]);
								if(a > a0)
								{
									a=std::min(a,diffArr[k+4]);
									a=std::min(a,diffArr[k+5]);
									a=std::min(a,diffArr[k+6]);
									a=std::min(a,diffArr[k+7]);
									a=std::min(a,diffArr[k+8]);
									a0=std::max(a0,std::min(a,diffArr[k]));
									a0=std::max(a0,std::min(a,diffArr[k+9]));
								}
							}
							short b0=-a0;
							for(uchar k=0; k != funcPatternSize; k+=2)
							{
								short b=std::max(diffArr[k+1],diffArr[k+2]);
								b=std::max(b,diffArr[k+3]);
								b=std::max(b,diffArr[k+4]);
								b=std::max(b,diffArr[k+5]);
								if(b < b0)
								{
									b=std::max(b,diffArr[k+6]);
									b=std::max(b,diffArr[k+7]);
									b=std::max(b,diffArr[k+8]);
									b0=std::min(b0,std::max(b,diffArr[k]));
									b0=std::min(b0,std::max(b,diffArr[k+9]));
								}
							}
							return uchar(-b0-1);
						}
						template<size_t funcPatternSize=patternSize,KPRetentionPolicy funcRetentionPolicy=retentionPolicy,HAL_FLAG funcHal=hal> constexpr
							std::enable_if_t<funcPatternSize==8&&((funcRetentionPolicy==NON_MAX_SUPPRESSION||funcRetentionPolicy==RETAIN_ALL)&&HalFlagInfo<funcHal>::hasSSE2),uchar>
							calculateScore(const uchar* srcPtr) const noexcept
						{
							short diffArr[bufferSize];
							initializeDiffArr(diffArr,srcPtr);
							__m128i v0=_mm_loadu_si128((__m128i*)(diffArr+1));
							__m128i v1=_mm_loadu_si128((__m128i*)(diffArr+2));
							__m128i a=_mm_min_epi16(v0,v1);
							__m128i b=_mm_max_epi16(v0,v1);
							v0=_mm_loadu_si128((__m128i*)(diffArr+3));
							a=_mm_min_epi16(a,v0);
							b=_mm_max_epi16(b,v0);
							v0=_mm_loadu_si128((__m128i*)(diffArr+4));
							a=_mm_min_epi16(a,v0);
							b=_mm_max_epi16(b,v0);
							v0=_mm_loadu_si128((__m128i*)(diffArr));
							__m128i q0=_mm_min_epi16(a,v0);
							__m128i q1=_mm_max_epi16(b,v0);
							v0=_mm_loadu_si128((__m128i*)(diffArr+5));
							q0=_mm_max_epi16(q0,_mm_min_epi16(a,v0));
							q1=_mm_min_epi16(q1,_mm_max_epi16(b,v0));
							q0=_mm_max_epi16(q0,_mm_sub_epi16(_mm_setzero_si128(),q1));
							q0=_mm_max_epi16(q0,_mm_unpackhi_epi64(q0,q0));
							q0=_mm_max_epi16(q0,_mm_srli_si128(q0,4));
							q0=_mm_max_epi16(q0,_mm_srli_si128(q0,2));
							return uchar((short)_mm_cvtsi128_si32(q0)-1);
						}
						template<size_t funcPatternSize=patternSize,KPRetentionPolicy funcRetentionPolicy=retentionPolicy,HAL_FLAG funcHal=hal> constexpr
							std::enable_if_t<funcPatternSize==12&&((funcRetentionPolicy==NON_MAX_SUPPRESSION||funcRetentionPolicy==RETAIN_ALL)&&HalFlagInfo<funcHal>::hasSSE2),uchar>
							calculateScore(const uchar* srcPtr) const noexcept
						{
							short diffArr[bufferSize+4];
							initializeDiffArr(diffArr,srcPtr);
							__m128i q0=_mm_set1_epi16(-1000),q1=_mm_set1_epi16(1000);
							for(uchar k=0; k != 16; k+=8)
							{
								__m128i v0=_mm_loadu_si128((__m128i*)(diffArr+k+1));
								__m128i v1=_mm_loadu_si128((__m128i*)(diffArr+k+2));
								__m128i a=_mm_min_epi16(v0,v1);
								__m128i b=_mm_max_epi16(v0,v1);
								v0=_mm_loadu_si128((__m128i*)(diffArr+k+3));
								a=_mm_min_epi16(a,v0);
								b=_mm_max_epi16(b,v0);
								v0=_mm_loadu_si128((__m128i*)(diffArr+k+4));
								a=_mm_min_epi16(a,v0);
								b=_mm_max_epi16(b,v0);
								v0=_mm_loadu_si128((__m128i*)(diffArr+k+5));
								a=_mm_min_epi16(a,v0);
								b=_mm_max_epi16(b,v0);
								v0=_mm_loadu_si128((__m128i*)(diffArr+k+6));
								a=_mm_min_epi16(a,v0);
								b=_mm_max_epi16(b,v0);
								v0=_mm_loadu_si128((__m128i*)(diffArr+k));
								q0=_mm_max_epi16(q0,_mm_min_epi16(a,v0));
								q1=_mm_min_epi16(q1,_mm_max_epi16(b,v0));
								v0=_mm_loadu_si128((__m128i*)(diffArr+k+7));
								q0=_mm_max_epi16(q0,_mm_min_epi16(a,v0));
								q1=_mm_min_epi16(q1,_mm_max_epi16(b,v0));
							}
							q0=_mm_max_epi16(q0,_mm_sub_epi16(_mm_setzero_si128(),q1));
							q0=_mm_max_epi16(q0,_mm_unpackhi_epi64(q0,q0));
							q0=_mm_max_epi16(q0,_mm_srli_si128(q0,4));
							q0=_mm_max_epi16(q0,_mm_srli_si128(q0,2));
							return uchar((short)_mm_cvtsi128_si32(q0)-1);
						}
						template<size_t funcPatternSize=patternSize,KPRetentionPolicy funcRetentionPolicy=retentionPolicy,HAL_FLAG funcHal=hal> constexpr
							std::enable_if_t<funcPatternSize==16&&((funcRetentionPolicy==NON_MAX_SUPPRESSION||funcRetentionPolicy==RETAIN_ALL)&&HalFlagInfo<funcHal>::hasSSE2),uchar>
							calculateScore(const uchar* srcPtr) const noexcept
						{
							short diffArr[bufferSize];
							initializeDiffArr(diffArr,srcPtr);
							__m128i q0=_mm_set1_epi16(-1000);
							__m128i q1=_mm_set1_epi16(1000);
							for(uchar i=0; i!=funcPatternSize; i+=8)
							{
								__m128i v0=_mm_loadu_si128((__m128i*)(diffArr+i+1));
								__m128i v1=_mm_loadu_si128((__m128i*)(diffArr+i+2));
								__m128i a=_mm_min_epi16(v0,v1);
								__m128i b=_mm_max_epi16(v0,v1);
								v0=_mm_loadu_si128((__m128i*)(diffArr+i+3));
								a=_mm_min_epi16(a,v0);
								b=_mm_max_epi16(b,v0);
								v0=_mm_loadu_si128((__m128i*)(diffArr+i+4));
								a=_mm_min_epi16(a,v0);
								b=_mm_max_epi16(b,v0);
								v0=_mm_loadu_si128((__m128i*)(diffArr+i+5));
								a=_mm_min_epi16(a,v0);
								b=_mm_max_epi16(b,v0);
								v0=_mm_loadu_si128((__m128i*)(diffArr+i+6));
								a=_mm_min_epi16(a,v0);
								b=_mm_max_epi16(b,v0);
								v0=_mm_loadu_si128((__m128i*)(diffArr+i+7));
								a=_mm_min_epi16(a,v0);
								b=_mm_max_epi16(b,v0);
								v0=_mm_loadu_si128((__m128i*)(diffArr+i+8));
								a=_mm_min_epi16(a,v0);
								b=_mm_max_epi16(b,v0);
								v0=_mm_loadu_si128((__m128i*)(diffArr+i));
								q0=_mm_max_epi16(q0,_mm_min_epi16(a,v0));
								q1=_mm_min_epi16(q1,_mm_max_epi16(b,v0));
								v0=_mm_loadu_si128((__m128i*)(diffArr+i+9));
								q0=_mm_max_epi16(q0,_mm_min_epi16(a,v0));
								q1=_mm_min_epi16(q1,_mm_max_epi16(b,v0));
							}
							q0=_mm_max_epi16(q0,_mm_sub_epi16(_mm_setzero_si128(),q1));
							q0=_mm_max_epi16(q0,_mm_unpackhi_epi64(q0,q0));
							q0=_mm_max_epi16(q0,_mm_srli_si128(q0,4));
							q0=_mm_max_epi16(q0,_mm_srli_si128(q0,2));
							return uchar((short)_mm_cvtsi128_si32(q0)-1);
						}
						template<size_t funcPatternSize=patternSize,KPRetentionPolicy funcRetentionPolicy=retentionPolicy,HAL_FLAG funcHal=hal> constexpr
							std::enable_if_t<funcRetentionPolicy==RETAIN_ALL_NO_SCORE,uchar>
							calculateScore(const uchar* srcPtr) const noexcept
						{
							return 0;
						}

						constexpr void countDarkerPixels(const int v,const uchar* const srcPtr,FastKeyPointPostProcessor<retentionPolicy>& keyPointPostProcessor,const size_t x) const
						{
							//const int vt=v-factory->fastThreshold;
							//int numLeft=bufferSize-1;
							//int numNeeded=Pattern<patternSize,shape>::halfPatternSize+1;
							//for(;;)
							//{
							//	if(srcPtr[pixel[numLeft-patternSize]]<vt)
							//	{
							//		if(!(--numNeeded))
							//		{
							//			keyPointPostProcessor.acceptScore(x,calculateScore(srcPtr));
							//			return;
							//		}
							//	}
							//	else
							//	{
							//		numNeeded=halfPatternSize+1;
							//	}
							//	if(numNeeded>=numLeft)
							//	{
							//		return;
							//	}
							//	if(numLeft--==patternSize)
							//	{
							//		numLeft=patternSize-1;
							//	}
							//}
							const int vt=v-factory->fastThreshold;
							for(uchar numLeft=bufferSize,numNeeded=Pattern<patternSize,shape>::halfPatternSize+1;numNeeded<(numLeft--);)
							{
								if(srcPtr[pixel[numLeft%patternSize]] < vt)
								{
									if(!(--numNeeded))
									{
										keyPointPostProcessor.acceptScore(x,calculateScore(srcPtr));
										break;
									}
								}
								else
								{
									numNeeded=halfPatternSize+1;
								}
							}
						}

						constexpr void countLighterPixels(const int v,const uchar* const srcPtr,FastKeyPointPostProcessor<retentionPolicy>& keyPointPostProcessor,const size_t x) const
						{
							//const int vt=v+factory->fastThreshold;
							//int numLeft=bufferSize-1;
							//int numNeeded=Pattern<patternSize,shape>::halfPatternSize+1;
							//for(;;)
							//{
							//	if(srcPtr[pixel[numLeft-patternSize]]>vt)
							//	{
							//		if(!(--numNeeded))
							//		{
							//			keyPointPostProcessor.acceptScore(x,calculateScore(srcPtr));
							//			return;
							//		}
							//	}
							//	else
							//	{
							//		numNeeded=halfPatternSize+1;
							//	}
							//	if(numNeeded>=numLeft)
							//	{
							//		return;
							//	}
							//	if(numLeft--==patternSize)
							//	{
							//		numLeft=patternSize-1;
							//	}
							//}
							const int vt=v+factory->fastThreshold;
							for(uchar numLeft=bufferSize,numNeeded=Pattern<patternSize,shape>::halfPatternSize+1;numNeeded<(numLeft--);)
							{
								if(srcPtr[pixel[numLeft%patternSize]] > vt)
								{
									if(!(--numNeeded))
									{
										keyPointPostProcessor.acceptScore(x,calculateScore(srcPtr));
										break;
									}
								}
								else
								{
									numNeeded=halfPatternSize+1;
								}
							}
						}

						template<HAL_FLAG detectHal=hal,size_t funcPatternSize=patternSize> constexpr
							std::enable_if_t<funcPatternSize==16&&!HalFlagInfo<detectHal>::hasSSE2>
							detectAndScore(const uchar* srcPtr,size_t x,const size_t xEnd,FastKeyPointPostProcessor<retentionPolicy>& keyPointPostProcessor) const
						{
							for(;x<xEnd;++x,++srcPtr)
							{
								const int v=srcPtr[0];
								const uchar* tab=factory->thresholdTable-v+255;
								int d;
								if((d=tab[srcPtr[pixel[0]]]|tab[srcPtr[pixel[8]]])
									&&((d=d
										&(tab[srcPtr[pixel[2]]]|tab[srcPtr[pixel[10]]])
										&(tab[srcPtr[pixel[4]]]|tab[srcPtr[pixel[12]]])
										&(tab[srcPtr[pixel[6]]]|tab[srcPtr[pixel[14]]])
										))
									)
								{
									if((d=d
										&(tab[srcPtr[pixel[1]]]|tab[srcPtr[pixel[9]]])
										&(tab[srcPtr[pixel[3]]]|tab[srcPtr[pixel[11]]])
										&(tab[srcPtr[pixel[5]]]|tab[srcPtr[pixel[13]]])
										&(tab[srcPtr[pixel[7]]]|tab[srcPtr[pixel[15]]]))&1)
									{
										countDarkerPixels(v,srcPtr,keyPointPostProcessor,x);
									}
									if(d&2)
									{
										countLighterPixels(v,srcPtr,keyPointPostProcessor,x);
									}
								}
							}
						}
						template<HAL_FLAG detectHal=hal,size_t funcPatternSize=patternSize> constexpr
							std::enable_if_t<funcPatternSize==16&&HalFlagInfo<detectHal>::hasSSE2>
							detectAndScore(const uchar* srcPtr,size_t x,const size_t xEnd,FastKeyPointPostProcessor<retentionPolicy>& keyPointPostProcessor) const
						{
							const size_t xBound=clipHiBorder(xEnd,size_t(16));
							__m128i delta=_mm_set1_epi8(-128),t=_mm_set1_epi8((char)factory->fastThreshold),K16=_mm_set1_epi8((char)8);
							for(;x<xBound;x+=16,srcPtr+=16)
							{
								__m128i m0,m1;
								__m128i v0=_mm_loadu_si128((const __m128i*)srcPtr);
								__m128i v1=_mm_xor_si128(_mm_subs_epu8(v0,t),delta);
								v0=_mm_xor_si128(_mm_adds_epu8(v0,t),delta);
								__m128i x0=_mm_sub_epi8(_mm_loadu_si128((const __m128i*)(srcPtr+pixel[0])),delta);
								__m128i x1=_mm_sub_epi8(_mm_loadu_si128((const __m128i*)(srcPtr+pixel[Pattern<funcPatternSize,shape>::quarterPatternSize])),delta);
								__m128i x2=_mm_sub_epi8(_mm_loadu_si128((const __m128i*)(srcPtr+pixel[2*Pattern<funcPatternSize,shape>::quarterPatternSize])),delta);
								__m128i x3=_mm_sub_epi8(_mm_loadu_si128((const __m128i*)(srcPtr+pixel[3*Pattern<funcPatternSize,shape>::quarterPatternSize])),delta);
								m0=_mm_and_si128(_mm_cmpgt_epi8(x0,v0),_mm_cmpgt_epi8(x1,v0));
								m1=_mm_and_si128(_mm_cmpgt_epi8(v1,x0),_mm_cmpgt_epi8(v1,x1));
								m0=_mm_or_si128(m0,_mm_and_si128(_mm_cmpgt_epi8(x1,v0),_mm_cmpgt_epi8(x2,v0)));
								m1=_mm_or_si128(m1,_mm_and_si128(_mm_cmpgt_epi8(v1,x1),_mm_cmpgt_epi8(v1,x2)));
								m0=_mm_or_si128(m0,_mm_and_si128(_mm_cmpgt_epi8(x2,v0),_mm_cmpgt_epi8(x3,v0)));
								m1=_mm_or_si128(m1,_mm_and_si128(_mm_cmpgt_epi8(v1,x2),_mm_cmpgt_epi8(v1,x3)));
								m0=_mm_or_si128(m0,_mm_and_si128(_mm_cmpgt_epi8(x3,v0),_mm_cmpgt_epi8(x0,v0)));
								m1=_mm_or_si128(m1,_mm_and_si128(_mm_cmpgt_epi8(v1,x3),_mm_cmpgt_epi8(v1,x0)));
								m0=_mm_or_si128(m0,m1);
								int mask=_mm_movemask_epi8(m0);
								if(mask)
								{
									if((mask&255)==0)
									{
										x-=8;
										srcPtr-=8;
										continue;
									}
									__m128i c0=_mm_setzero_si128(),c1=c0,max0=c0,max1=c0;
									uchar k=0;
									for(; k!=funcPatternSize; ++k)
									{
										__m128i xor=_mm_xor_si128(_mm_loadu_si128((const __m128i*)(srcPtr+pixel[k])),delta);
										m0=_mm_cmpgt_epi8(xor,v0);
										m1=_mm_cmpgt_epi8(v1,xor);
										c0=_mm_and_si128(_mm_sub_epi8(c0,m0),m0);
										c1=_mm_and_si128(_mm_sub_epi8(c1,m1),m1);
										max0=_mm_max_epu8(max0,c0);
										max1=_mm_max_epu8(max1,c1);
									}
									for(; k!=bufferSize; ++k)
									{
										__m128i xor=_mm_xor_si128(_mm_loadu_si128((const __m128i*)(srcPtr+pixel[k-funcPatternSize])),delta);
										m0=_mm_cmpgt_epi8(xor,v0);
										m1=_mm_cmpgt_epi8(v1,xor);
										c0=_mm_and_si128(_mm_sub_epi8(c0,m0),m0);
										c1=_mm_and_si128(_mm_sub_epi8(c1,m1),m1);
										max0=_mm_max_epu8(max0,c0);
										max1=_mm_max_epu8(max1,c1);
									}
									max0=_mm_max_epu8(max0,max1);
									int m=_mm_movemask_epi8(_mm_cmpgt_epi8(max0,K16));
									for(uchar k=0; m > 0&&k!=funcPatternSize; ++k,m>>=1)
									{
										if(m&1)
										{
											keyPointPostProcessor.acceptScore(x+k,calculateScore(srcPtr+k));
										}
									}
								}
							}
							detectAndScore<HAL_NONE>(srcPtr,x,xEnd,keyPointPostProcessor);
						}
						template<HAL_FLAG detectHal=hal,size_t funcPatternSize=patternSize> constexpr
							std::enable_if_t<funcPatternSize==12>
							detectAndScore(const uchar* srcPtr,size_t x,const size_t xEnd,FastKeyPointPostProcessor<retentionPolicy>& keyPointPostProcessor) const
						{
							for(;x<xEnd;++x,++srcPtr)
							{
								const int v=srcPtr[0];
								const uchar* tab=factory->thresholdTable-v+255;
								int d;
								if((d=tab[srcPtr[pixel[0]]]|tab[srcPtr[pixel[8]]])
								   &&((d=d
									   &(tab[srcPtr[pixel[2]]]|tab[srcPtr[pixel[10]]])
									   &(tab[srcPtr[pixel[4]]]|tab[srcPtr[pixel[0]]])
									   &(tab[srcPtr[pixel[6]]]|tab[srcPtr[pixel[2]]])
									   ))
								   )
								{
									if((d=d
										&(tab[srcPtr[pixel[1]]]|tab[srcPtr[pixel[9]]])
										&(tab[srcPtr[pixel[3]]]|tab[srcPtr[pixel[11]]])
										&(tab[srcPtr[pixel[5]]]|tab[srcPtr[pixel[1]]])
										&(tab[srcPtr[pixel[7]]]|tab[srcPtr[pixel[3]]]))&1)
									{
										countDarkerPixels(v,srcPtr,keyPointPostProcessor,x);
									}
									if(d&2)
									{
										countLighterPixels(v,srcPtr,keyPointPostProcessor,x);
									}
								}
							}
						}
						template<HAL_FLAG detectHal=hal,size_t funcPatternSize=patternSize> constexpr
							std::enable_if_t<funcPatternSize==8>
							detectAndScore(const uchar* srcPtr,size_t x,const size_t xEnd,FastKeyPointPostProcessor<retentionPolicy>& keyPointPostProcessor) const
						{
							for(; x<xEnd; ++x,++srcPtr)
							{
								const int v=srcPtr[0];
								const uchar* tab=factory->thresholdTable-v+255;
								int d;
								if((d=tab[srcPtr[pixel[0]]])
								   &&((d=d
									   &(tab[srcPtr[pixel[2]]])
									   &(tab[srcPtr[pixel[4]]])
									   &(tab[srcPtr[pixel[6]]]))))
								{
									if((d=d&(tab[srcPtr[pixel[1]]])&(tab[srcPtr[pixel[3]]])&(tab[srcPtr[pixel[5]]])&(tab[srcPtr[pixel[7]]]))&1)
									{
										countDarkerPixels(v,srcPtr,keyPointPostProcessor,x);
									}
									if(d&2)
									{
										countLighterPixels(v,srcPtr,keyPointPostProcessor,x);
									}
								}
							}
						}

					};

					template<size_t patternSize,PatternShape shape,KPRetentionPolicy retentionPolicy,bool compatibilityMode,HAL_FLAG hal>
					class FastKeyPointGenerator<patternSize,shape,retentionPolicy,compatibilityMode,hal,std::enable_if_t<retentionPolicy==NON_MAX_SUPPRESSION>>: public KeyPointGenerator,private matrix::Mat1D<uchar>
					{
					private:
						const size_t cols;
						const int octave;
						const float kpSize;
						const size_t xMax;
						const size_t yMax;
						const size_t xEnd;
						const size_t yEnd;
						const size_t start;
						size_t* const positionBuffer[2];
						uchar* const scoreBuffer[3];
						FastPattern<patternSize,shape,retentionPolicy,compatibilityMode,hal> pattern;
					public:

						const FastKeyPointGeneratorFactory<patternSize,shape,retentionPolicy,compatibilityMode,hal>* getFactory() const noexcept
						{
							return pattern.factory;
						}

						constexpr FastKeyPointGenerator(size_t cols,size_t rows,int octave,float kpSize,const FastKeyPointGeneratorFactory<patternSize,shape,retentionPolicy,compatibilityMode,hal>* factory) noexcept
							:matrix::Mat1D<uchar>((sizeof(size_t)*(cols+1)*2)+(sizeof(uchar)*cols*3))
							,cols(cols)
							,octave(octave)
							,kpSize(kpSize)
							,xMax(clipHiBorder(cols,factory->border))
							,yMax(clipHiBorder(rows,factory->border))
							,xEnd(std::min<size_t>(cols-patternBorder<compatibilityMode,patternSize,shape,void>,xMax+1))
							//,xEnd(std::min<size_t>(cols-Pattern<patternSize,shape>::getBorder<compatibilityMode>(),xMax+1))
							,yEnd(std::min<size_t>(rows-patternBorder<compatibilityMode,patternSize,shape,void>,yMax+1))
							//,yEnd(std::min<size_t>(rows-Pattern<patternSize,shape>::getBorder<compatibilityMode>(),yMax+1))
							,start(std::max<size_t>(factory->border-1,patternBorder<compatibilityMode,patternSize,shape,void>))
							//,start(std::max<size_t>(factory->border-1,Pattern<patternSize,shape>::getBorder<compatibilityMode>()))
							,positionBuffer{((size_t*)data)+1,this->positionBuffer[0]+cols+1}
							,scoreBuffer{(uchar*)(positionBuffer[1]+cols),scoreBuffer[0]+cols,scoreBuffer[1]+cols}
							,pattern(int(cols),factory)
						{}

						void detect(const matrix::Ptr2D<uchar>& frame,std::vector<cv::KeyPoint>& keyPoints) noexcept override
						{
							keyPoints.clear();
							std::memset(scoreBuffer[0],0,cols*3);
							const size_t border=pattern.factory->border;
							for(size_t y=start;y<=yEnd;++y)
							{
								uchar* const nextScoreBuffer=scoreBuffer[y%3];
								size_t* const nextPositionBuffer=positionBuffer[y%2];
								std::memset(nextScoreBuffer,0,cols);
								nextPositionBuffer[-1]=0;
								if(y!=yEnd)
								{
									pattern.detectAndScore(frame.ptr(start,y),start,xEnd,FastKeyPointPostProcessor<retentionPolicy>(nextPositionBuffer,nextScoreBuffer));
								}
								if(y>border && y<=yMax)
								{
									const size_t* const currPositionBuffer=positionBuffer[(y-1)%2];
									const size_t numCorners=currPositionBuffer[-1];
									const uchar* const currScoreBuffer=scoreBuffer[(y-1)%3];
									const uchar* const prevScoreBuffer=scoreBuffer[(y-2)%3];
									for(size_t i=0; i!=numCorners; ++i)
									{
										const size_t x=currPositionBuffer[i];
										if(x>=border && x < xMax)
										{
											const int score=currScoreBuffer[x];
											if
												(
													score > prevScoreBuffer[x-1]&&
													score > prevScoreBuffer[x]&&
													score > prevScoreBuffer[x+1]&&
													score > currScoreBuffer[x-1]&&
													score > currScoreBuffer[x+1]&&
													score > nextScoreBuffer[x-1]&&
													score > nextScoreBuffer[x]&&
													score > nextScoreBuffer[x+1]
													)
											{
												keyPoints.emplace_back(float(x),float(y-1),kpSize,-1.f,float(score),octave,-1);
											}
										}
									}
								}
							}
						}
					};

					template<size_t patternSize,PatternShape shape,KPRetentionPolicy retentionPolicy,bool compatibilityMode,HAL_FLAG hal>
					class FastKeyPointGenerator<patternSize,shape,retentionPolicy,compatibilityMode,hal,std::enable_if_t<retentionPolicy==RETAIN_ALL||retentionPolicy==RETAIN_ALL_NO_SCORE>>: public KeyPointGenerator
					{
					private:
						const size_t cols;
						const int octave;
						const float kpSize;
						const size_t xMax;
						const size_t yMax;
						const size_t start;
						FastPattern<patternSize,shape,retentionPolicy,compatibilityMode,hal> pattern;
					public:

						const FastKeyPointGeneratorFactory<patternSize,shape,retentionPolicy,compatibilityMode,hal>* getFactory() const noexcept
						{
							return pattern.factory;
						}

						constexpr
							FastKeyPointGenerator(size_t cols,size_t rows,int octave,float kpSize,const FastKeyPointGeneratorFactory<patternSize,shape,retentionPolicy,compatibilityMode,hal>* factory) noexcept
							:cols(cols)
							,octave(octave)
							,kpSize(kpSize)
							,xMax(clipHiBorder(cols,factory->border))
							,yMax(clipHiBorder(rows,factory->border))
							,start(std::max<size_t>(factory->border,patternBorder<compatibilityMode,patternSize,shape,void>))
							//,start(std::max<size_t>(factory->border,Pattern<patternSize,shape>::getBorder<compatibilityMode>()))
							,pattern(int(cols),factory)
						{}

						void detect(const matrix::Ptr2D<uchar>& frame,std::vector<cv::KeyPoint>& keyPoints) override
						{
							keyPoints.clear();
							for(size_t y=start;y<yMax;++y)
							{
								pattern.detectAndScore(frame.ptr(start,y),start,xMax,FastKeyPointPostProcessor<retentionPolicy>(keyPoints,float(y),kpSize,octave));
							}
						}
					};

					template<size_t patternSize,PatternShape shape,KPRetentionPolicy retentionPolicy,bool compatibilityMode,HAL_FLAG hal>
					inline FastKeyPointGenerator<patternSize,shape,retentionPolicy,compatibilityMode,hal>* FastKeyPointGeneratorFactory<patternSize,shape,retentionPolicy,compatibilityMode,hal>::getKeyPointGenerator(size_t cols,size_t rows,float kpSize,int octave,float,float,int) const
					{
						return new FastKeyPointGenerator<patternSize,shape,retentionPolicy,compatibilityMode,hal>(cols,rows,octave,kpSize,this);
					}

				}
			}


			//namespace fast
			//{
			//
			//	using namespace lyonste::hal;
			//	template<PatternSize patternSize=Fast16,KPRetentionPolicy scoreType=NON_MAX_SUPPRESSION,bool compatibilityMode=true,HAL_FLAG hal=HAL_NONE,typename ENABLE=void>
			//	class FastKeyPointGenerator;
			//
			//	template<PatternSize patternSize=Fast16,KPRetentionPolicy scoreType=NON_MAX_SUPPRESSION,bool compatibilityMode=true,HAL_FLAG hal=HAL_NONE>
			//	class FastKeyPointGeneratorFactory;
			//
			//	template<KPRetentionPolicy scoreType=NON_MAX_SUPPRESSION>
			//	class FastKeyPointPostProcessor
			//	{
			//	private:
			//		size_t* const posBuffer;
			//		uchar* const scoreBuffer;
			//	public:
			//		constexpr
			//			FastKeyPointPostProcessor(size_t* posBuffer,uchar* scoreBuffer):
			//			posBuffer(posBuffer),
			//			scoreBuffer(scoreBuffer)
			//		{}
			//
			//		constexpr
			//			void acceptScore(size_t x,uchar score)
			//		{
			//			scoreBuffer[posBuffer[posBuffer[-1]++]=x]=score;
			//		}
			//	};
			//
			//	template<>
			//	class FastKeyPointPostProcessor<RETAIN_ALL>
			//	{
			//	private:
			//		std::vector<cv::KeyPoint>& keyPoints;
			//		const float y;
			//		const float kpSize;
			//		const int octave;
			//	public:
			//		constexpr
			//			FastKeyPointPostProcessor(std::vector<cv::KeyPoint>& keyPoints,float y,float kpSize,int octave):
			//			keyPoints(keyPoints),
			//			y(y),
			//			kpSize(kpSize),
			//			octave(octave)
			//		{}
			//
			//		constexpr
			//			void acceptScore(size_t x,uchar score)
			//		{
			//			keyPoints.emplace_back(float(x),y,kpSize,-1.f,score,octave,-1);
			//		}
			//	};
			//
			//	template<>
			//	class FastKeyPointPostProcessor<RETAIN_ALL_NO_SCORE>
			//	{
			//	private:
			//		std::vector<cv::KeyPoint>& keyPoints;
			//		const float y;
			//		const float kpSize;
			//		const int octave;
			//	public:
			//
			//		constexpr
			//			FastKeyPointPostProcessor(std::vector<cv::KeyPoint>& keyPoints,float y,float kpSize,int octave):
			//			keyPoints(keyPoints),
			//			y(y),
			//			kpSize(kpSize),
			//			octave(octave)
			//		{}
			//
			//		constexpr
			//			void acceptScore(size_t x,uchar)
			//		{
			//			keyPoints.emplace_back(float(x),y,kpSize,-1.f,0.f,octave,-1);
			//		}
			//	};
			//
			//	template<PatternSize patternSize>
			//	class PatternCircle
			//	{
			//	public:
			//		template<bool compatibilityMode> constexpr static
			//			size_t getRadius()
			//		{
			//			if(!compatibilityMode)
			//			{
			//				switch(patternSize)
			//				{
			//					case Fast8:
			//						return 1;
			//					case Fast12:
			//						return 2;
			//					default:
			//						break;
			//				}
			//			}
			//			return 3;
			//		}
			//
			//		enum
			//		{
			//			halfPatternSize=patternSize>>1,
			//			quarterPatternSize=halfPatternSize>>1,
			//			bufferSize=patternSize+halfPatternSize+1
			//		};
			//
			//	private:
			//		const int circle[bufferSize];
			//
			//		template<KPRetentionPolicy scoreType,HAL_FLAG hal> constexpr
			//			std::enable_if_t<(scoreType==NON_MAX_SUPPRESSION||scoreType==RETAIN_ALL)&&HalFlagInfo<hal>::hasSSE2,uchar>
			//			calculateScore(const uchar* srcPtr,short fastThreshold=10) const;
			//
			//		template<KPRetentionPolicy scoreType,HAL_FLAG hal> constexpr
			//			std::enable_if_t<(scoreType==NON_MAX_SUPPRESSION||scoreType==RETAIN_ALL)&&!HalFlagInfo<hal>::hasSSE2,uchar>
			//			calculateScore(const uchar* srcPtr,short fastThreshold=10) const;
			//
			//		template<KPRetentionPolicy scoreType,HAL_FLAG hal> constexpr
			//			std::enable_if_t<(scoreType==RETAIN_ALL_NO_SCORE),uchar>
			//			calculateScore(const uchar* srcPtr,short fastThreshold=10) const
			//		{
			//			return 0;
			//		};
			//
			//		template<KPRetentionPolicy scoreType,HAL_FLAG scoreHal> constexpr
			//			void countDarkerPixels(const int v,const short fastThreshold,const uchar* const srcPtr,FastKeyPointPostProcessor<scoreType>& keyPointPostProcessor,const size_t x) const
			//		{
			//			const int vt=v-fastThreshold;
			//			for(uchar numLeft=bufferSize,numNeeded=halfPatternSize+1;numNeeded<(numLeft--);)
			//			{
			//				if(srcPtr[circle[numLeft]] < vt)
			//				{
			//					if(!(--numNeeded))
			//					{
			//						keyPointPostProcessor.acceptScore(x,calculateScore<scoreType,scoreHal>(srcPtr,fastThreshold));
			//						break;
			//					}
			//				}
			//				else
			//				{
			//					numNeeded=halfPatternSize+1;
			//				}
			//			}
			//		}
			//
			//		template<KPRetentionPolicy scoreType,HAL_FLAG scoreHal> constexpr
			//			void countLighterPixels(const int v,const short fastThreshold,const uchar* const srcPtr,FastKeyPointPostProcessor<scoreType>& keyPointPostProcessor,const size_t x) const
			//		{
			//			const int vt=v+fastThreshold;
			//			for(uchar numLeft=bufferSize,numNeeded=halfPatternSize+1;numNeeded<(numLeft--);)
			//			{
			//				if(srcPtr[circle[numLeft]] > vt)
			//				{
			//					if(!(--numNeeded))
			//					{
			//						keyPointPostProcessor.acceptScore(x,calculateScore<scoreType,scoreHal>(srcPtr,fastThreshold));
			//						break;
			//					}
			//				}
			//				else
			//				{
			//					numNeeded=halfPatternSize+1;
			//				}
			//			}
			//		}
			//
			//	public:
			//		constexpr PatternCircle(const int cols);
			//
			//		template<KPRetentionPolicy scoreType,bool compatibilityMode,HAL_FLAG detectHal,HAL_FLAG scoreHal,PatternSize pSize=patternSize> constexpr
			//			std::enable_if_t<(pSize!=Fast8&&!HalFlagInfo<detectHal>::hasSSE2)||pSize==Fast12>
			//			detectAndScore(const uchar* srcPtr,size_t x,const size_t xEnd,FastKeyPointPostProcessor<scoreType>& keyPointPostProcessor,const FastKeyPointGeneratorFactory<pSize,scoreType,compatibilityMode,scoreHal>* const factory) const
			//		{
			//			for(;x<xEnd;++x,++srcPtr)
			//			{
			//				const int v=srcPtr[0];
			//				const uchar* tab=factory->thresholdTable-v+255;
			//				int d;
			//				if((d=tab[srcPtr[circle[0]]]|tab[srcPtr[circle[8]]])
			//				   &&((d=d
			//					   &(tab[srcPtr[circle[2]]]|tab[srcPtr[circle[10]]])
			//					   &(tab[srcPtr[circle[4]]]|tab[srcPtr[circle[12]]])
			//					   &(tab[srcPtr[circle[6]]]|tab[srcPtr[circle[14]]])
			//					   ))
			//				   )
			//				{
			//					if((d=d
			//						&(tab[srcPtr[circle[1]]]|tab[srcPtr[circle[9]]])
			//						&(tab[srcPtr[circle[3]]]|tab[srcPtr[circle[11]]])
			//						&(tab[srcPtr[circle[5]]]|tab[srcPtr[circle[13]]])
			//						&(tab[srcPtr[circle[7]]]|tab[srcPtr[circle[15]]]))&1)
			//					{
			//						countDarkerPixels<scoreType,scoreHal>(v,factory->fastThreshold,srcPtr,keyPointPostProcessor,x);
			//					}
			//					if(d&2)
			//					{
			//						countLighterPixels<scoreType,scoreHal>(v,factory->fastThreshold,srcPtr,keyPointPostProcessor,x);
			//					}
			//				}
			//			}
			//		}
			//
			//		template<KPRetentionPolicy scoreType,bool compatibilityMode,HAL_FLAG detectHal,HAL_FLAG scoreHal,PatternSize pSize=patternSize> constexpr
			//			std::enable_if_t<HalFlagInfo<detectHal>::hasSSE2 && pSize==Fast16>
			//			detectAndScore(const uchar* srcPtr,size_t x,const size_t xEnd,FastKeyPointPostProcessor<scoreType>& keyPointPostProcessor,const FastKeyPointGeneratorFactory<pSize,scoreType,compatibilityMode,scoreHal>* const factory) const
			//		{
			//			const short fastThreshold=factory->fastThreshold;
			//			const size_t xBound=clipHiBorder(xEnd,size_t(16));
			//			__m128i delta=_mm_set1_epi8(-128),t=_mm_set1_epi8((char)fastThreshold),K16=_mm_set1_epi8((char)8);
			//			for(;x<xBound;x+=16,srcPtr+=16)
			//			{
			//				__m128i m0,m1;
			//				__m128i v0=_mm_loadu_si128((const __m128i*)srcPtr);
			//				__m128i v1=_mm_xor_si128(_mm_subs_epu8(v0,t),delta);
			//				v0=_mm_xor_si128(_mm_adds_epu8(v0,t),delta);
			//				__m128i x0=_mm_sub_epi8(_mm_loadu_si128((const __m128i*)(srcPtr+circle[0])),delta);
			//				__m128i x1=_mm_sub_epi8(_mm_loadu_si128((const __m128i*)(srcPtr+circle[PatternCircle<pSize>::quarterPatternSize])),delta);
			//				__m128i x2=_mm_sub_epi8(_mm_loadu_si128((const __m128i*)(srcPtr+circle[2*PatternCircle<pSize>::quarterPatternSize])),delta);
			//				__m128i x3=_mm_sub_epi8(_mm_loadu_si128((const __m128i*)(srcPtr+circle[3*PatternCircle<pSize>::quarterPatternSize])),delta);
			//				m0=_mm_and_si128(_mm_cmpgt_epi8(x0,v0),_mm_cmpgt_epi8(x1,v0));
			//				m1=_mm_and_si128(_mm_cmpgt_epi8(v1,x0),_mm_cmpgt_epi8(v1,x1));
			//				m0=_mm_or_si128(m0,_mm_and_si128(_mm_cmpgt_epi8(x1,v0),_mm_cmpgt_epi8(x2,v0)));
			//				m1=_mm_or_si128(m1,_mm_and_si128(_mm_cmpgt_epi8(v1,x1),_mm_cmpgt_epi8(v1,x2)));
			//				m0=_mm_or_si128(m0,_mm_and_si128(_mm_cmpgt_epi8(x2,v0),_mm_cmpgt_epi8(x3,v0)));
			//				m1=_mm_or_si128(m1,_mm_and_si128(_mm_cmpgt_epi8(v1,x2),_mm_cmpgt_epi8(v1,x3)));
			//				m0=_mm_or_si128(m0,_mm_and_si128(_mm_cmpgt_epi8(x3,v0),_mm_cmpgt_epi8(x0,v0)));
			//				m1=_mm_or_si128(m1,_mm_and_si128(_mm_cmpgt_epi8(v1,x3),_mm_cmpgt_epi8(v1,x0)));
			//				m0=_mm_or_si128(m0,m1);
			//				int mask=_mm_movemask_epi8(m0);
			//				if(mask)
			//				{
			//					if((mask&255)==0)
			//					{
			//						x-=8;
			//						srcPtr-=8;
			//						continue;
			//					}
			//					__m128i c0=_mm_setzero_si128(),c1=c0,max0=c0,max1=c0;
			//					for(uchar k=0; k!=PatternCircle<pSize>::bufferSize; k++)
			//					{
			//						__m128i xor=_mm_xor_si128(_mm_loadu_si128((const __m128i*)(srcPtr+circle[k])),delta);
			//						m0=_mm_cmpgt_epi8(xor,v0);
			//						m1=_mm_cmpgt_epi8(v1,xor);
			//						c0=_mm_and_si128(_mm_sub_epi8(c0,m0),m0);
			//						c1=_mm_and_si128(_mm_sub_epi8(c1,m1),m1);
			//						max0=_mm_max_epu8(max0,c0);
			//						max1=_mm_max_epu8(max1,c1);
			//					}
			//					max0=_mm_max_epu8(max0,max1);
			//					int m=_mm_movemask_epi8(_mm_cmpgt_epi8(max0,K16));
			//					for(uchar k=0; m > 0&&k!=pSize; k++,m>>=1)
			//					{
			//						if(m&1)
			//						{
			//							keyPointPostProcessor.acceptScore(x+k,calculateScore<scoreType,scoreHal>(srcPtr+k,fastThreshold));
			//						}
			//					}
			//				}
			//			}
			//			detectAndScore<scoreType,compatibilityMode,HAL_NONE,scoreHal>(srcPtr,x,xEnd,keyPointPostProcessor,factory);
			//		}
			//
			//		template<KPRetentionPolicy scoreType,bool compatibilityMode,HAL_FLAG detectHal,HAL_FLAG scoreHal,PatternSize pSize=patternSize> constexpr
			//			std::enable_if_t<pSize==Fast8>
			//			detectAndScore(const uchar* srcPtr,size_t x,const size_t xEnd,FastKeyPointPostProcessor<scoreType>& keyPointPostProcessor,const FastKeyPointGeneratorFactory<pSize,scoreType,compatibilityMode,scoreHal>* const factory) const
			//		{
			//			for(; x<xEnd; ++x,++srcPtr)
			//			{
			//				const int v=srcPtr[0];
			//				const uchar* tab=factory->thresholdTable-v+255;
			//				int d;
			//				if((d=tab[srcPtr[circle[0]]])
			//				   &&((d=d
			//					   &(tab[srcPtr[circle[2]]])
			//					   &(tab[srcPtr[circle[4]]])
			//					   &(tab[srcPtr[circle[6]]]))))
			//				{
			//					if((d=d&(tab[srcPtr[circle[1]]])&(tab[srcPtr[circle[3]]])&(tab[srcPtr[circle[5]]])&(tab[srcPtr[circle[7]]]))&1)
			//					{
			//						countDarkerPixels<scoreType,scoreHal>(v,factory->fastThreshold,srcPtr,keyPointPostProcessor,x);
			//					}
			//					if(d&2)
			//					{
			//						countLighterPixels<scoreType,scoreHal>(v,factory->fastThreshold,srcPtr,keyPointPostProcessor,x);
			//					}
			//				}
			//			}
			//		}
			//	};
			//	template<> constexpr
			//		PatternCircle<Fast8>::PatternCircle(const int cols):
			//		circle
			//	{
			//		+cols,/////////0
			//		+1+cols,///////1
			//		+1,////////////2
			//		+1-cols,///////3
			//		-cols,/////////4
			//		-1-cols,///////5 { 5}{ 4}{ 3}   {12}{11}
			//		-1,////////////6 { 6}{  }{ 2}       {10}
			//		-1+cols,///////7 { 7}{ 0}{ 1}   { 8}{ 9}
			//		circle[0],/////8
			//		circle[1],/////9
			//		circle[2],////10
			//		circle[3],////11
			//		circle[4]/////12
			//	}
			//	{
			//
			//	}
			//
			//	template<> constexpr
			//		PatternCircle<Fast12>::PatternCircle(const int cols):
			//		circle
			//	{
			//		+(cols<<1),///////0
			//		+1+(cols<<1),/////1
			//		+2+cols,//////////2
			//		+2,///////////////3
			//		+2-cols,//////////4
			//		+1-(cols<<1),/////5
			//		-(cols<<1),///////6
			//		-1-(cols<<1),/////7     { 7}{ 6}{ 5}       {18}{17}
			//		-2-cols,//////////8 { 8}            { 4}           {16}
			//		-2,///////////////9 { 9}    {  }    { 3}           {15}
			//		-2+cols,/////////10 {10}            { 2}           {14}
			//		-1+(cols<<1),////11     {11}{ 0}{ 1}       {12}{13}
			//		circle[0],///////12
			//		circle[1],///////13
			//		circle[2],///////14
			//		circle[3],///////15
			//		circle[4],///////16
			//		circle[5],///////17
			//		circle[6]////////18
			//	}
			//	{
			//
			//	}
			//
			//	template<> constexpr
			//		PatternCircle<Fast16>::PatternCircle(const int cols):
			//		circle
			//	{
			//		+(cols*3),////////0
			//		+1+(cols*3),//////1
			//		+2+(cols<<1),/////2
			//		+3+cols,//////////3
			//		+3,///////////////4
			//		+3-cols,//////////5
			//		+2-(cols<<1),/////6
			//		+1-(cols*3),//////7
			//		-(cols*3),////////8
			//		-1-(cols*3),//////9         { 9}{ 8}{ 7}           {24}{23}
			//		-2-(cols<<1),////10     {10}            { 6}               {22}
			//		-3-cols,/////////11 {11}                    { 5}               {21}
			//		-3,//////////////12 {12}        {  }        { 4}               {20}
			//		-3+cols,/////////13 {13}                    { 3}               {19}
			//		-2+(cols<<1),////14     {14}            { 2}               {18}
			//		-1+(cols*3),/////15         {15}{ 0}{ 1}           {16}{17}
			//		circle[0],///////16
			//		circle[1],///////17
			//		circle[2],///////18
			//		circle[3],///////19
			//		circle[4],///////20
			//		circle[5],///////21
			//		circle[6],///////22
			//		circle[7],///////23
			//		circle[8]////////24
			//	}
			//	{
			//
			//	}
			//
			//	template<> template<KPRetentionPolicy scoreType,HAL_FLAG hal> constexpr
			//		std::enable_if_t<(scoreType==NON_MAX_SUPPRESSION||scoreType==RETAIN_ALL)&&!HalFlagInfo<hal>::hasSSE2,uchar>
			//		PatternCircle<Fast8>::calculateScore(const uchar* srcPtr,short fastThreshold) const
			//	{
			//		short diffArr[PatternCircle<Fast8>::bufferSize];
			//		short v=srcPtr[0];
			//		for(uchar i=0; i!=PatternCircle<Fast8>::bufferSize; ++i)
			//		{
			//			diffArr[i]=v-srcPtr[circle[i]];
			//		}
			//		for(uchar k=0; k < Fast8; k+=2)
			//		{
			//			short a=std::min(diffArr[k+1],diffArr[k+2]);
			//			if(a<=fastThreshold)
			//				continue;
			//			a=std::min(a,diffArr[k+3]);
			//			a=std::min(a,diffArr[k+4]);
			//			fastThreshold=std::max(fastThreshold,std::min(a,diffArr[k]));
			//			fastThreshold=std::max(fastThreshold,std::min(a,diffArr[k+5]));
			//		}
			//		short b0=-fastThreshold;
			//		for(uchar k=0; k < Fast8; k+=2)
			//		{
			//			short b=std::max(diffArr[k+1],diffArr[k+2]);
			//			b=std::max(b,diffArr[k+3]);
			//			if(b>=b0)
			//				continue;
			//			b=std::max(b,diffArr[k+4]);
			//			b0=std::min(b0,std::max(b,diffArr[k]));
			//			b0=std::min(b0,std::max(b,diffArr[k+5]));
			//		}
			//		return uchar(-b0-1);
			//	}
			//	template<> template<KPRetentionPolicy scoreType,HAL_FLAG hal> constexpr
			//		std::enable_if_t<(scoreType==NON_MAX_SUPPRESSION||scoreType==RETAIN_ALL)&&!HalFlagInfo<hal>::hasSSE2,uchar>
			//		PatternCircle<Fast12>::calculateScore(const uchar* srcPtr,short fastThreshold) const
			//	{
			//		short diffArr[PatternCircle<Fast12>::bufferSize];
			//		short v=srcPtr[0];
			//		for(uchar i=0; i!=PatternCircle<Fast12>::bufferSize; ++i)
			//		{
			//			diffArr[i]=v-srcPtr[circle[i]];
			//		}
			//		for(uchar k=0; k < Fast12; k+=2)
			//		{
			//			short a=std::min(diffArr[k+1],diffArr[k+2]);
			//			if(a<=fastThreshold)
			//				continue;
			//			a=std::min(a,diffArr[k+3]);
			//			a=std::min(a,diffArr[k+4]);
			//			a=std::min(a,diffArr[k+5]);
			//			a=std::min(a,diffArr[k+6]);
			//			fastThreshold=std::max(fastThreshold,std::min(a,diffArr[k]));
			//			fastThreshold=std::max(fastThreshold,std::min(a,diffArr[k+7]));
			//		}
			//		short b0=-fastThreshold;
			//		for(uchar k=0; k < Fast12; k+=2)
			//		{
			//			short b=std::max(diffArr[k+1],diffArr[k+2]);
			//			b=std::max(b,diffArr[k+3]);
			//			b=std::max(b,diffArr[k+4]);
			//			if(b>=b0)
			//				continue;
			//			b=std::max(b,diffArr[k+5]);
			//			b=std::max(b,diffArr[k+6]);
			//			b0=std::min(b0,std::max(b,diffArr[k]));
			//			b0=std::min(b0,std::max(b,diffArr[k+7]));
			//		}
			//		return uchar(-b0-1);
			//	}
			//	template<> template<KPRetentionPolicy scoreType,HAL_FLAG hal> constexpr
			//		std::enable_if_t<(scoreType==NON_MAX_SUPPRESSION||scoreType==RETAIN_ALL)&&!HalFlagInfo<hal>::hasSSE2,uchar>
			//		PatternCircle<Fast16>::calculateScore(const uchar* srcPtr,short fastThreshold) const
			//	{
			//		short diffArr[PatternCircle<Fast16>::bufferSize];
			//		short v=srcPtr[0];
			//		for(uchar i=0; i!=PatternCircle<Fast16>::bufferSize; ++i)
			//		{
			//			diffArr[i]=v-srcPtr[circle[i]];
			//		}
			//		for(uchar k=0; k < Fast16; k+=2)
			//		{
			//			short a=std::min(diffArr[k+1],diffArr[k+2]);
			//			a=std::min(a,diffArr[k+3]);
			//			if(a > fastThreshold)
			//			{
			//				a=std::min(a,diffArr[k+4]);
			//				a=std::min(a,diffArr[k+5]);
			//				a=std::min(a,diffArr[k+6]);
			//				a=std::min(a,diffArr[k+7]);
			//				a=std::min(a,diffArr[k+8]);
			//				fastThreshold=std::max(fastThreshold,std::min(a,diffArr[k]));
			//				fastThreshold=std::max(fastThreshold,std::min(a,diffArr[k+9]));
			//			}
			//		}
			//		short b0=-fastThreshold;
			//		for(uchar k=0; k < Fast16; k+=2)
			//		{
			//			short b=std::max(diffArr[k+1],diffArr[k+2]);
			//			b=std::max(b,diffArr[k+3]);
			//			b=std::max(b,diffArr[k+4]);
			//			b=std::max(b,diffArr[k+5]);
			//			if(b < b0)
			//			{
			//				b=std::max(b,diffArr[k+6]);
			//				b=std::max(b,diffArr[k+7]);
			//				b=std::max(b,diffArr[k+8]);
			//				b0=std::min(b0,std::max(b,diffArr[k]));
			//				b0=std::min(b0,std::max(b,diffArr[k+9]));
			//			}
			//		}
			//		return uchar(-b0-1);
			//	}
			//	template<> template<KPRetentionPolicy scoreType,HAL_FLAG hal> constexpr
			//		std::enable_if_t<(scoreType==NON_MAX_SUPPRESSION||scoreType==RETAIN_ALL)&&HalFlagInfo<hal>::hasSSE2,uchar>
			//		PatternCircle<Fast8>::calculateScore(const uchar* srcPtr,short) const
			//	{
			//		short diffArr[PatternCircle<Fast8>::bufferSize];
			//		short v=srcPtr[0];
			//		for(uchar i=0; i!=PatternCircle<Fast8>::bufferSize; ++i)
			//		{
			//			diffArr[i]=v-srcPtr[circle[i]];
			//		}
			//		__m128i v0=_mm_loadu_si128((__m128i*)(diffArr+1));
			//		__m128i v1=_mm_loadu_si128((__m128i*)(diffArr+2));
			//		__m128i a=_mm_min_epi16(v0,v1);
			//		__m128i b=_mm_max_epi16(v0,v1);
			//		v0=_mm_loadu_si128((__m128i*)(diffArr+3));
			//		a=_mm_min_epi16(a,v0);
			//		b=_mm_max_epi16(b,v0);
			//		v0=_mm_loadu_si128((__m128i*)(diffArr+4));
			//		a=_mm_min_epi16(a,v0);
			//		b=_mm_max_epi16(b,v0);
			//		v0=_mm_loadu_si128((__m128i*)(diffArr));
			//		__m128i q0=_mm_min_epi16(a,v0);
			//		__m128i q1=_mm_max_epi16(b,v0);
			//		v0=_mm_loadu_si128((__m128i*)(diffArr+5));
			//		q0=_mm_max_epi16(q0,_mm_min_epi16(a,v0));
			//		q1=_mm_min_epi16(q1,_mm_max_epi16(b,v0));
			//		q0=_mm_max_epi16(q0,_mm_sub_epi16(_mm_setzero_si128(),q1));
			//		q0=_mm_max_epi16(q0,_mm_unpackhi_epi64(q0,q0));
			//		q0=_mm_max_epi16(q0,_mm_srli_si128(q0,4));
			//		q0=_mm_max_epi16(q0,_mm_srli_si128(q0,2));
			//		return uchar((short)_mm_cvtsi128_si32(q0)-1);
			//	}
			//	template<> template<KPRetentionPolicy scoreType,HAL_FLAG hal> constexpr
			//		std::enable_if_t<(scoreType==NON_MAX_SUPPRESSION||scoreType==RETAIN_ALL)&&HalFlagInfo<hal>::hasSSE2,uchar>
			//		PatternCircle<Fast12>::calculateScore(const uchar* srcPtr,short) const
			//	{
			//		short diffArr[PatternCircle<Fast12>::bufferSize+4];
			//		int v=srcPtr[0];
			//		for(uchar i=0; i!=PatternCircle<Fast12>::bufferSize; ++i)
			//		{
			//			diffArr[i]=short(v-srcPtr[circle[i]]);
			//		}
			//		diffArr[19]=diffArr[0];
			//		diffArr[20]=diffArr[1];
			//		diffArr[21]=diffArr[2];
			//		diffArr[22]=diffArr[3];
			//		__m128i q0=_mm_set1_epi16(-1000),q1=_mm_set1_epi16(1000);
			//		for(uchar k=0; k < 16; k+=8)
			//		{
			//			__m128i v0=_mm_loadu_si128((__m128i*)(diffArr+k+1));
			//			__m128i v1=_mm_loadu_si128((__m128i*)(diffArr+k+2));
			//			__m128i a=_mm_min_epi16(v0,v1);
			//			__m128i b=_mm_max_epi16(v0,v1);
			//			v0=_mm_loadu_si128((__m128i*)(diffArr+k+3));
			//			a=_mm_min_epi16(a,v0);
			//			b=_mm_max_epi16(b,v0);
			//			v0=_mm_loadu_si128((__m128i*)(diffArr+k+4));
			//			a=_mm_min_epi16(a,v0);
			//			b=_mm_max_epi16(b,v0);
			//			v0=_mm_loadu_si128((__m128i*)(diffArr+k+5));
			//			a=_mm_min_epi16(a,v0);
			//			b=_mm_max_epi16(b,v0);
			//			v0=_mm_loadu_si128((__m128i*)(diffArr+k+6));
			//			a=_mm_min_epi16(a,v0);
			//			b=_mm_max_epi16(b,v0);
			//			v0=_mm_loadu_si128((__m128i*)(diffArr+k));
			//			q0=_mm_max_epi16(q0,_mm_min_epi16(a,v0));
			//			q1=_mm_min_epi16(q1,_mm_max_epi16(b,v0));
			//			v0=_mm_loadu_si128((__m128i*)(diffArr+k+7));
			//			q0=_mm_max_epi16(q0,_mm_min_epi16(a,v0));
			//			q1=_mm_min_epi16(q1,_mm_max_epi16(b,v0));
			//		}
			//		q0=_mm_max_epi16(q0,_mm_sub_epi16(_mm_setzero_si128(),q1));
			//		q0=_mm_max_epi16(q0,_mm_unpackhi_epi64(q0,q0));
			//		q0=_mm_max_epi16(q0,_mm_srli_si128(q0,4));
			//		q0=_mm_max_epi16(q0,_mm_srli_si128(q0,2));
			//		return uchar((short)_mm_cvtsi128_si32(q0)-1);
			//	}
			//	template<> template<KPRetentionPolicy scoreType,HAL_FLAG hal> constexpr
			//		std::enable_if_t<(scoreType==NON_MAX_SUPPRESSION||scoreType==RETAIN_ALL)&&HalFlagInfo<hal>::hasSSE2,uchar>
			//		PatternCircle<Fast16>::calculateScore(const uchar* srcPtr,short) const
			//	{
			//		short diffArr[PatternCircle<Fast16>::bufferSize];
			//		short v=srcPtr[0];
			//		for(uchar i=0; i!=PatternCircle<Fast16>::bufferSize; ++i)
			//		{
			//			diffArr[i]=v-srcPtr[circle[i]];
			//		}
			//		__m128i q0=_mm_set1_epi16(-1000);
			//		__m128i q1=_mm_set1_epi16(1000);
			//		for(uchar i=0; i!=16; i+=8)
			//		{
			//			__m128i v0=_mm_loadu_si128((__m128i*)(diffArr+i+1));
			//			__m128i v1=_mm_loadu_si128((__m128i*)(diffArr+i+2));
			//			__m128i a=_mm_min_epi16(v0,v1);
			//			__m128i b=_mm_max_epi16(v0,v1);
			//			v0=_mm_loadu_si128((__m128i*)(diffArr+i+3));
			//			a=_mm_min_epi16(a,v0);
			//			b=_mm_max_epi16(b,v0);
			//			v0=_mm_loadu_si128((__m128i*)(diffArr+i+4));
			//			a=_mm_min_epi16(a,v0);
			//			b=_mm_max_epi16(b,v0);
			//			v0=_mm_loadu_si128((__m128i*)(diffArr+i+5));
			//			a=_mm_min_epi16(a,v0);
			//			b=_mm_max_epi16(b,v0);
			//			v0=_mm_loadu_si128((__m128i*)(diffArr+i+6));
			//			a=_mm_min_epi16(a,v0);
			//			b=_mm_max_epi16(b,v0);
			//			v0=_mm_loadu_si128((__m128i*)(diffArr+i+7));
			//			a=_mm_min_epi16(a,v0);
			//			b=_mm_max_epi16(b,v0);
			//			v0=_mm_loadu_si128((__m128i*)(diffArr+i+8));
			//			a=_mm_min_epi16(a,v0);
			//			b=_mm_max_epi16(b,v0);
			//			v0=_mm_loadu_si128((__m128i*)(diffArr+i));
			//			q0=_mm_max_epi16(q0,_mm_min_epi16(a,v0));
			//			q1=_mm_min_epi16(q1,_mm_max_epi16(b,v0));
			//			v0=_mm_loadu_si128((__m128i*)(diffArr+i+9));
			//			q0=_mm_max_epi16(q0,_mm_min_epi16(a,v0));
			//			q1=_mm_min_epi16(q1,_mm_max_epi16(b,v0));
			//		}
			//		q0=_mm_max_epi16(q0,_mm_sub_epi16(_mm_setzero_si128(),q1));
			//		q0=_mm_max_epi16(q0,_mm_unpackhi_epi64(q0,q0));
			//		q0=_mm_max_epi16(q0,_mm_srli_si128(q0,4));
			//		q0=_mm_max_epi16(q0,_mm_srli_si128(q0,2));
			//		return uchar((short)_mm_cvtsi128_si32(q0)-1);
			//	}
			//
			//
			//
			//	template<PatternSize patternSize,KPRetentionPolicy scoreType,bool compatibilityMode,HAL_FLAG hal>
			//	class FastKeyPointGeneratorFactory: public KeyPointGeneratorFactory
			//	{
			//	private:
			//
			//
			//		friend class PatternCircle<patternSize>;
			//		friend class FastKeyPointGenerator<patternSize,scoreType,compatibilityMode,hal>;
			//
			//		friend KeyPointGeneratorFactory* KeyPointGeneratorFactory::configureKeyPointGeneratorFactory<Fast>(const boost::property_tree::ptree& properties,const lyonste::hal::HAL_FLAG hal_flag);
			//
			//		const size_t border;
			//		const short fastThreshold;
			//		const uchar thresholdTable[512];
			//
			//
			//		constexpr
			//			FastKeyPointGeneratorFactory(size_t border,short fastThreshold):
			//			border(std::max<size_t>(border,PatternCircle<patternSize>::getRadius<compatibilityMode>())),
			//			fastThreshold(fastThreshold),
			//			thresholdTable
			//		{
			//			uchar(-255 < -fastThreshold?1:-255 > fastThreshold?2:0),
			//			uchar(-254 < -fastThreshold?1:-254 > fastThreshold?2:0),
			//			uchar(-253 < -fastThreshold?1:-253 > fastThreshold?2:0),
			//			uchar(-252 < -fastThreshold?1:-252 > fastThreshold?2:0),
			//			uchar(-251 < -fastThreshold?1:-251 > fastThreshold?2:0),
			//			uchar(-250 < -fastThreshold?1:-250 > fastThreshold?2:0),
			//			uchar(-249 < -fastThreshold?1:-249 > fastThreshold?2:0),
			//			uchar(-248 < -fastThreshold?1:-248 > fastThreshold?2:0),
			//			uchar(-247 < -fastThreshold?1:-247 > fastThreshold?2:0),
			//			uchar(-246 < -fastThreshold?1:-246 > fastThreshold?2:0),
			//			uchar(-245 < -fastThreshold?1:-245 > fastThreshold?2:0),
			//			uchar(-244 < -fastThreshold?1:-244 > fastThreshold?2:0),
			//			uchar(-243 < -fastThreshold?1:-243 > fastThreshold?2:0),
			//			uchar(-242 < -fastThreshold?1:-242 > fastThreshold?2:0),
			//			uchar(-241 < -fastThreshold?1:-241 > fastThreshold?2:0),
			//			uchar(-240 < -fastThreshold?1:-240 > fastThreshold?2:0),
			//			uchar(-239 < -fastThreshold?1:-239 > fastThreshold?2:0),
			//			uchar(-238 < -fastThreshold?1:-238 > fastThreshold?2:0),
			//			uchar(-237 < -fastThreshold?1:-237 > fastThreshold?2:0),
			//			uchar(-236 < -fastThreshold?1:-236 > fastThreshold?2:0),
			//			uchar(-235 < -fastThreshold?1:-235 > fastThreshold?2:0),
			//			uchar(-234 < -fastThreshold?1:-234 > fastThreshold?2:0),
			//			uchar(-233 < -fastThreshold?1:-233 > fastThreshold?2:0),
			//			uchar(-232 < -fastThreshold?1:-232 > fastThreshold?2:0),
			//			uchar(-231 < -fastThreshold?1:-231 > fastThreshold?2:0),
			//			uchar(-230 < -fastThreshold?1:-230 > fastThreshold?2:0),
			//			uchar(-229 < -fastThreshold?1:-229 > fastThreshold?2:0),
			//			uchar(-228 < -fastThreshold?1:-228 > fastThreshold?2:0),
			//			uchar(-227 < -fastThreshold?1:-227 > fastThreshold?2:0),
			//			uchar(-226 < -fastThreshold?1:-226 > fastThreshold?2:0),
			//			uchar(-225 < -fastThreshold?1:-225 > fastThreshold?2:0),
			//			uchar(-224 < -fastThreshold?1:-224 > fastThreshold?2:0),
			//			uchar(-223 < -fastThreshold?1:-223 > fastThreshold?2:0),
			//			uchar(-222 < -fastThreshold?1:-222 > fastThreshold?2:0),
			//			uchar(-221 < -fastThreshold?1:-221 > fastThreshold?2:0),
			//			uchar(-220 < -fastThreshold?1:-220 > fastThreshold?2:0),
			//			uchar(-219 < -fastThreshold?1:-219 > fastThreshold?2:0),
			//			uchar(-218 < -fastThreshold?1:-218 > fastThreshold?2:0),
			//			uchar(-217 < -fastThreshold?1:-217 > fastThreshold?2:0),
			//			uchar(-216 < -fastThreshold?1:-216 > fastThreshold?2:0),
			//			uchar(-215 < -fastThreshold?1:-215 > fastThreshold?2:0),
			//			uchar(-214 < -fastThreshold?1:-214 > fastThreshold?2:0),
			//			uchar(-213 < -fastThreshold?1:-213 > fastThreshold?2:0),
			//			uchar(-212 < -fastThreshold?1:-212 > fastThreshold?2:0),
			//			uchar(-211 < -fastThreshold?1:-211 > fastThreshold?2:0),
			//			uchar(-210 < -fastThreshold?1:-210 > fastThreshold?2:0),
			//			uchar(-209 < -fastThreshold?1:-209 > fastThreshold?2:0),
			//			uchar(-208 < -fastThreshold?1:-208 > fastThreshold?2:0),
			//			uchar(-207 < -fastThreshold?1:-207 > fastThreshold?2:0),
			//			uchar(-206 < -fastThreshold?1:-206 > fastThreshold?2:0),
			//			uchar(-205 < -fastThreshold?1:-205 > fastThreshold?2:0),
			//			uchar(-204 < -fastThreshold?1:-204 > fastThreshold?2:0),
			//			uchar(-203 < -fastThreshold?1:-203 > fastThreshold?2:0),
			//			uchar(-202 < -fastThreshold?1:-202 > fastThreshold?2:0),
			//			uchar(-201 < -fastThreshold?1:-201 > fastThreshold?2:0),
			//			uchar(-200 < -fastThreshold?1:-200 > fastThreshold?2:0),
			//			uchar(-199 < -fastThreshold?1:-199 > fastThreshold?2:0),
			//			uchar(-198 < -fastThreshold?1:-198 > fastThreshold?2:0),
			//			uchar(-197 < -fastThreshold?1:-197 > fastThreshold?2:0),
			//			uchar(-196 < -fastThreshold?1:-196 > fastThreshold?2:0),
			//			uchar(-195 < -fastThreshold?1:-195 > fastThreshold?2:0),
			//			uchar(-194 < -fastThreshold?1:-194 > fastThreshold?2:0),
			//			uchar(-193 < -fastThreshold?1:-193 > fastThreshold?2:0),
			//			uchar(-192 < -fastThreshold?1:-192 > fastThreshold?2:0),
			//			uchar(-191 < -fastThreshold?1:-191 > fastThreshold?2:0),
			//			uchar(-190 < -fastThreshold?1:-190 > fastThreshold?2:0),
			//			uchar(-189 < -fastThreshold?1:-189 > fastThreshold?2:0),
			//			uchar(-188 < -fastThreshold?1:-188 > fastThreshold?2:0),
			//			uchar(-187 < -fastThreshold?1:-187 > fastThreshold?2:0),
			//			uchar(-186 < -fastThreshold?1:-186 > fastThreshold?2:0),
			//			uchar(-185 < -fastThreshold?1:-185 > fastThreshold?2:0),
			//			uchar(-184 < -fastThreshold?1:-184 > fastThreshold?2:0),
			//			uchar(-183 < -fastThreshold?1:-183 > fastThreshold?2:0),
			//			uchar(-182 < -fastThreshold?1:-182 > fastThreshold?2:0),
			//			uchar(-181 < -fastThreshold?1:-181 > fastThreshold?2:0),
			//			uchar(-180 < -fastThreshold?1:-180 > fastThreshold?2:0),
			//			uchar(-179 < -fastThreshold?1:-179 > fastThreshold?2:0),
			//			uchar(-178 < -fastThreshold?1:-178 > fastThreshold?2:0),
			//			uchar(-177 < -fastThreshold?1:-177 > fastThreshold?2:0),
			//			uchar(-176 < -fastThreshold?1:-176 > fastThreshold?2:0),
			//			uchar(-175 < -fastThreshold?1:-175 > fastThreshold?2:0),
			//			uchar(-174 < -fastThreshold?1:-174 > fastThreshold?2:0),
			//			uchar(-173 < -fastThreshold?1:-173 > fastThreshold?2:0),
			//			uchar(-172 < -fastThreshold?1:-172 > fastThreshold?2:0),
			//			uchar(-171 < -fastThreshold?1:-171 > fastThreshold?2:0),
			//			uchar(-170 < -fastThreshold?1:-170 > fastThreshold?2:0),
			//			uchar(-169 < -fastThreshold?1:-169 > fastThreshold?2:0),
			//			uchar(-168 < -fastThreshold?1:-168 > fastThreshold?2:0),
			//			uchar(-167 < -fastThreshold?1:-167 > fastThreshold?2:0),
			//			uchar(-166 < -fastThreshold?1:-166 > fastThreshold?2:0),
			//			uchar(-165 < -fastThreshold?1:-165 > fastThreshold?2:0),
			//			uchar(-164 < -fastThreshold?1:-164 > fastThreshold?2:0),
			//			uchar(-163 < -fastThreshold?1:-163 > fastThreshold?2:0),
			//			uchar(-162 < -fastThreshold?1:-162 > fastThreshold?2:0),
			//			uchar(-161 < -fastThreshold?1:-161 > fastThreshold?2:0),
			//			uchar(-160 < -fastThreshold?1:-160 > fastThreshold?2:0),
			//			uchar(-159 < -fastThreshold?1:-159 > fastThreshold?2:0),
			//			uchar(-158 < -fastThreshold?1:-158 > fastThreshold?2:0),
			//			uchar(-157 < -fastThreshold?1:-157 > fastThreshold?2:0),
			//			uchar(-156 < -fastThreshold?1:-156 > fastThreshold?2:0),
			//			uchar(-155 < -fastThreshold?1:-155 > fastThreshold?2:0),
			//			uchar(-154 < -fastThreshold?1:-154 > fastThreshold?2:0),
			//			uchar(-153 < -fastThreshold?1:-153 > fastThreshold?2:0),
			//			uchar(-152 < -fastThreshold?1:-152 > fastThreshold?2:0),
			//			uchar(-151 < -fastThreshold?1:-151 > fastThreshold?2:0),
			//			uchar(-150 < -fastThreshold?1:-150 > fastThreshold?2:0),
			//			uchar(-149 < -fastThreshold?1:-149 > fastThreshold?2:0),
			//			uchar(-148 < -fastThreshold?1:-148 > fastThreshold?2:0),
			//			uchar(-147 < -fastThreshold?1:-147 > fastThreshold?2:0),
			//			uchar(-146 < -fastThreshold?1:-146 > fastThreshold?2:0),
			//			uchar(-145 < -fastThreshold?1:-145 > fastThreshold?2:0),
			//			uchar(-144 < -fastThreshold?1:-144 > fastThreshold?2:0),
			//			uchar(-143 < -fastThreshold?1:-143 > fastThreshold?2:0),
			//			uchar(-142 < -fastThreshold?1:-142 > fastThreshold?2:0),
			//			uchar(-141 < -fastThreshold?1:-141 > fastThreshold?2:0),
			//			uchar(-140 < -fastThreshold?1:-140 > fastThreshold?2:0),
			//			uchar(-139 < -fastThreshold?1:-139 > fastThreshold?2:0),
			//			uchar(-138 < -fastThreshold?1:-138 > fastThreshold?2:0),
			//			uchar(-137 < -fastThreshold?1:-137 > fastThreshold?2:0),
			//			uchar(-136 < -fastThreshold?1:-136 > fastThreshold?2:0),
			//			uchar(-135 < -fastThreshold?1:-135 > fastThreshold?2:0),
			//			uchar(-134 < -fastThreshold?1:-134 > fastThreshold?2:0),
			//			uchar(-133 < -fastThreshold?1:-133 > fastThreshold?2:0),
			//			uchar(-132 < -fastThreshold?1:-132 > fastThreshold?2:0),
			//			uchar(-131 < -fastThreshold?1:-131 > fastThreshold?2:0),
			//			uchar(-130 < -fastThreshold?1:-130 > fastThreshold?2:0),
			//			uchar(-129 < -fastThreshold?1:-129 > fastThreshold?2:0),
			//			uchar(-128 < -fastThreshold?1:-128 > fastThreshold?2:0),
			//			uchar(-127 < -fastThreshold?1:-127 > fastThreshold?2:0),
			//			uchar(-126 < -fastThreshold?1:-126 > fastThreshold?2:0),
			//			uchar(-125 < -fastThreshold?1:-125 > fastThreshold?2:0),
			//			uchar(-124 < -fastThreshold?1:-124 > fastThreshold?2:0),
			//			uchar(-123 < -fastThreshold?1:-123 > fastThreshold?2:0),
			//			uchar(-122 < -fastThreshold?1:-122 > fastThreshold?2:0),
			//			uchar(-121 < -fastThreshold?1:-121 > fastThreshold?2:0),
			//			uchar(-120 < -fastThreshold?1:-120 > fastThreshold?2:0),
			//			uchar(-119 < -fastThreshold?1:-119 > fastThreshold?2:0),
			//			uchar(-118 < -fastThreshold?1:-118 > fastThreshold?2:0),
			//			uchar(-117 < -fastThreshold?1:-117 > fastThreshold?2:0),
			//			uchar(-116 < -fastThreshold?1:-116 > fastThreshold?2:0),
			//			uchar(-115 < -fastThreshold?1:-115 > fastThreshold?2:0),
			//			uchar(-114 < -fastThreshold?1:-114 > fastThreshold?2:0),
			//			uchar(-113 < -fastThreshold?1:-113 > fastThreshold?2:0),
			//			uchar(-112 < -fastThreshold?1:-112 > fastThreshold?2:0),
			//			uchar(-111 < -fastThreshold?1:-111 > fastThreshold?2:0),
			//			uchar(-110 < -fastThreshold?1:-110 > fastThreshold?2:0),
			//			uchar(-109 < -fastThreshold?1:-109 > fastThreshold?2:0),
			//			uchar(-108 < -fastThreshold?1:-108 > fastThreshold?2:0),
			//			uchar(-107 < -fastThreshold?1:-107 > fastThreshold?2:0),
			//			uchar(-106 < -fastThreshold?1:-106 > fastThreshold?2:0),
			//			uchar(-105 < -fastThreshold?1:-105 > fastThreshold?2:0),
			//			uchar(-104 < -fastThreshold?1:-104 > fastThreshold?2:0),
			//			uchar(-103 < -fastThreshold?1:-103 > fastThreshold?2:0),
			//			uchar(-102 < -fastThreshold?1:-102 > fastThreshold?2:0),
			//			uchar(-101 < -fastThreshold?1:-101 > fastThreshold?2:0),
			//			uchar(-100 < -fastThreshold?1:-100 > fastThreshold?2:0),
			//			uchar(-99 < -fastThreshold?1:-99 > fastThreshold?2:0),
			//			uchar(-98 < -fastThreshold?1:-98 > fastThreshold?2:0),
			//			uchar(-97 < -fastThreshold?1:-97 > fastThreshold?2:0),
			//			uchar(-96 < -fastThreshold?1:-96 > fastThreshold?2:0),
			//			uchar(-95 < -fastThreshold?1:-95 > fastThreshold?2:0),
			//			uchar(-94 < -fastThreshold?1:-94 > fastThreshold?2:0),
			//			uchar(-93 < -fastThreshold?1:-93 > fastThreshold?2:0),
			//			uchar(-92 < -fastThreshold?1:-92 > fastThreshold?2:0),
			//			uchar(-91 < -fastThreshold?1:-91 > fastThreshold?2:0),
			//			uchar(-90 < -fastThreshold?1:-90 > fastThreshold?2:0),
			//			uchar(-89 < -fastThreshold?1:-89 > fastThreshold?2:0),
			//			uchar(-88 < -fastThreshold?1:-88 > fastThreshold?2:0),
			//			uchar(-87 < -fastThreshold?1:-87 > fastThreshold?2:0),
			//			uchar(-86 < -fastThreshold?1:-86 > fastThreshold?2:0),
			//			uchar(-85 < -fastThreshold?1:-85 > fastThreshold?2:0),
			//			uchar(-84 < -fastThreshold?1:-84 > fastThreshold?2:0),
			//			uchar(-83 < -fastThreshold?1:-83 > fastThreshold?2:0),
			//			uchar(-82 < -fastThreshold?1:-82 > fastThreshold?2:0),
			//			uchar(-81 < -fastThreshold?1:-81 > fastThreshold?2:0),
			//			uchar(-80 < -fastThreshold?1:-80 > fastThreshold?2:0),
			//			uchar(-79 < -fastThreshold?1:-79 > fastThreshold?2:0),
			//			uchar(-78 < -fastThreshold?1:-78 > fastThreshold?2:0),
			//			uchar(-77 < -fastThreshold?1:-77 > fastThreshold?2:0),
			//			uchar(-76 < -fastThreshold?1:-76 > fastThreshold?2:0),
			//			uchar(-75 < -fastThreshold?1:-75 > fastThreshold?2:0),
			//			uchar(-74 < -fastThreshold?1:-74 > fastThreshold?2:0),
			//			uchar(-73 < -fastThreshold?1:-73 > fastThreshold?2:0),
			//			uchar(-72 < -fastThreshold?1:-72 > fastThreshold?2:0),
			//			uchar(-71 < -fastThreshold?1:-71 > fastThreshold?2:0),
			//			uchar(-70 < -fastThreshold?1:-70 > fastThreshold?2:0),
			//			uchar(-69 < -fastThreshold?1:-69 > fastThreshold?2:0),
			//			uchar(-68 < -fastThreshold?1:-68 > fastThreshold?2:0),
			//			uchar(-67 < -fastThreshold?1:-67 > fastThreshold?2:0),
			//			uchar(-66 < -fastThreshold?1:-66 > fastThreshold?2:0),
			//			uchar(-65 < -fastThreshold?1:-65 > fastThreshold?2:0),
			//			uchar(-64 < -fastThreshold?1:-64 > fastThreshold?2:0),
			//			uchar(-63 < -fastThreshold?1:-63 > fastThreshold?2:0),
			//			uchar(-62 < -fastThreshold?1:-62 > fastThreshold?2:0),
			//			uchar(-61 < -fastThreshold?1:-61 > fastThreshold?2:0),
			//			uchar(-60 < -fastThreshold?1:-60 > fastThreshold?2:0),
			//			uchar(-59 < -fastThreshold?1:-59 > fastThreshold?2:0),
			//			uchar(-58 < -fastThreshold?1:-58 > fastThreshold?2:0),
			//			uchar(-57 < -fastThreshold?1:-57 > fastThreshold?2:0),
			//			uchar(-56 < -fastThreshold?1:-56 > fastThreshold?2:0),
			//			uchar(-55 < -fastThreshold?1:-55 > fastThreshold?2:0),
			//			uchar(-54 < -fastThreshold?1:-54 > fastThreshold?2:0),
			//			uchar(-53 < -fastThreshold?1:-53 > fastThreshold?2:0),
			//			uchar(-52 < -fastThreshold?1:-52 > fastThreshold?2:0),
			//			uchar(-51 < -fastThreshold?1:-51 > fastThreshold?2:0),
			//			uchar(-50 < -fastThreshold?1:-50 > fastThreshold?2:0),
			//			uchar(-49 < -fastThreshold?1:-49 > fastThreshold?2:0),
			//			uchar(-48 < -fastThreshold?1:-48 > fastThreshold?2:0),
			//			uchar(-47 < -fastThreshold?1:-47 > fastThreshold?2:0),
			//			uchar(-46 < -fastThreshold?1:-46 > fastThreshold?2:0),
			//			uchar(-45 < -fastThreshold?1:-45 > fastThreshold?2:0),
			//			uchar(-44 < -fastThreshold?1:-44 > fastThreshold?2:0),
			//			uchar(-43 < -fastThreshold?1:-43 > fastThreshold?2:0),
			//			uchar(-42 < -fastThreshold?1:-42 > fastThreshold?2:0),
			//			uchar(-41 < -fastThreshold?1:-41 > fastThreshold?2:0),
			//			uchar(-40 < -fastThreshold?1:-40 > fastThreshold?2:0),
			//			uchar(-39 < -fastThreshold?1:-39 > fastThreshold?2:0),
			//			uchar(-38 < -fastThreshold?1:-38 > fastThreshold?2:0),
			//			uchar(-37 < -fastThreshold?1:-37 > fastThreshold?2:0),
			//			uchar(-36 < -fastThreshold?1:-36 > fastThreshold?2:0),
			//			uchar(-35 < -fastThreshold?1:-35 > fastThreshold?2:0),
			//			uchar(-34 < -fastThreshold?1:-34 > fastThreshold?2:0),
			//			uchar(-33 < -fastThreshold?1:-33 > fastThreshold?2:0),
			//			uchar(-32 < -fastThreshold?1:-32 > fastThreshold?2:0),
			//			uchar(-31 < -fastThreshold?1:-31 > fastThreshold?2:0),
			//			uchar(-30 < -fastThreshold?1:-30 > fastThreshold?2:0),
			//			uchar(-29 < -fastThreshold?1:-29 > fastThreshold?2:0),
			//			uchar(-28 < -fastThreshold?1:-28 > fastThreshold?2:0),
			//			uchar(-27 < -fastThreshold?1:-27 > fastThreshold?2:0),
			//			uchar(-26 < -fastThreshold?1:-26 > fastThreshold?2:0),
			//			uchar(-25 < -fastThreshold?1:-25 > fastThreshold?2:0),
			//			uchar(-24 < -fastThreshold?1:-24 > fastThreshold?2:0),
			//			uchar(-23 < -fastThreshold?1:-23 > fastThreshold?2:0),
			//			uchar(-22 < -fastThreshold?1:-22 > fastThreshold?2:0),
			//			uchar(-21 < -fastThreshold?1:-21 > fastThreshold?2:0),
			//			uchar(-20 < -fastThreshold?1:-20 > fastThreshold?2:0),
			//			uchar(-19 < -fastThreshold?1:-19 > fastThreshold?2:0),
			//			uchar(-18 < -fastThreshold?1:-18 > fastThreshold?2:0),
			//			uchar(-17 < -fastThreshold?1:-17 > fastThreshold?2:0),
			//			uchar(-16 < -fastThreshold?1:-16 > fastThreshold?2:0),
			//			uchar(-15 < -fastThreshold?1:-15 > fastThreshold?2:0),
			//			uchar(-14 < -fastThreshold?1:-14 > fastThreshold?2:0),
			//			uchar(-13 < -fastThreshold?1:-13 > fastThreshold?2:0),
			//			uchar(-12 < -fastThreshold?1:-12 > fastThreshold?2:0),
			//			uchar(-11 < -fastThreshold?1:-11 > fastThreshold?2:0),
			//			uchar(-10 < -fastThreshold?1:-10 > fastThreshold?2:0),
			//			uchar(-9 < -fastThreshold?1:-9 > fastThreshold?2:0),
			//			uchar(-8 < -fastThreshold?1:-8 > fastThreshold?2:0),
			//			uchar(-7 < -fastThreshold?1:-7 > fastThreshold?2:0),
			//			uchar(-6 < -fastThreshold?1:-6 > fastThreshold?2:0),
			//			uchar(-5 < -fastThreshold?1:-5 > fastThreshold?2:0),
			//			uchar(-4 < -fastThreshold?1:-4 > fastThreshold?2:0),
			//			uchar(-3 < -fastThreshold?1:-3 > fastThreshold?2:0),
			//			uchar(-2 < -fastThreshold?1:-2 > fastThreshold?2:0),
			//			uchar(-1 < -fastThreshold?1:-1 > fastThreshold?2:0),
			//			uchar(0 < -fastThreshold?1:0 > fastThreshold?2:0),
			//			uchar(1 < -fastThreshold?1:1 > fastThreshold?2:0),
			//			uchar(2 < -fastThreshold?1:2 > fastThreshold?2:0),
			//			uchar(3 < -fastThreshold?1:3 > fastThreshold?2:0),
			//			uchar(4 < -fastThreshold?1:4 > fastThreshold?2:0),
			//			uchar(5 < -fastThreshold?1:5 > fastThreshold?2:0),
			//			uchar(6 < -fastThreshold?1:6 > fastThreshold?2:0),
			//			uchar(7 < -fastThreshold?1:7 > fastThreshold?2:0),
			//			uchar(8 < -fastThreshold?1:8 > fastThreshold?2:0),
			//			uchar(9 < -fastThreshold?1:9 > fastThreshold?2:0),
			//			uchar(10 < -fastThreshold?1:10 > fastThreshold?2:0),
			//			uchar(11 < -fastThreshold?1:11 > fastThreshold?2:0),
			//			uchar(12 < -fastThreshold?1:12 > fastThreshold?2:0),
			//			uchar(13 < -fastThreshold?1:13 > fastThreshold?2:0),
			//			uchar(14 < -fastThreshold?1:14 > fastThreshold?2:0),
			//			uchar(15 < -fastThreshold?1:15 > fastThreshold?2:0),
			//			uchar(16 < -fastThreshold?1:16 > fastThreshold?2:0),
			//			uchar(17 < -fastThreshold?1:17 > fastThreshold?2:0),
			//			uchar(18 < -fastThreshold?1:18 > fastThreshold?2:0),
			//			uchar(19 < -fastThreshold?1:19 > fastThreshold?2:0),
			//			uchar(20 < -fastThreshold?1:20 > fastThreshold?2:0),
			//			uchar(21 < -fastThreshold?1:21 > fastThreshold?2:0),
			//			uchar(22 < -fastThreshold?1:22 > fastThreshold?2:0),
			//			uchar(23 < -fastThreshold?1:23 > fastThreshold?2:0),
			//			uchar(24 < -fastThreshold?1:24 > fastThreshold?2:0),
			//			uchar(25 < -fastThreshold?1:25 > fastThreshold?2:0),
			//			uchar(26 < -fastThreshold?1:26 > fastThreshold?2:0),
			//			uchar(27 < -fastThreshold?1:27 > fastThreshold?2:0),
			//			uchar(28 < -fastThreshold?1:28 > fastThreshold?2:0),
			//			uchar(29 < -fastThreshold?1:29 > fastThreshold?2:0),
			//			uchar(30 < -fastThreshold?1:30 > fastThreshold?2:0),
			//			uchar(31 < -fastThreshold?1:31 > fastThreshold?2:0),
			//			uchar(32 < -fastThreshold?1:32 > fastThreshold?2:0),
			//			uchar(33 < -fastThreshold?1:33 > fastThreshold?2:0),
			//			uchar(34 < -fastThreshold?1:34 > fastThreshold?2:0),
			//			uchar(35 < -fastThreshold?1:35 > fastThreshold?2:0),
			//			uchar(36 < -fastThreshold?1:36 > fastThreshold?2:0),
			//			uchar(37 < -fastThreshold?1:37 > fastThreshold?2:0),
			//			uchar(38 < -fastThreshold?1:38 > fastThreshold?2:0),
			//			uchar(39 < -fastThreshold?1:39 > fastThreshold?2:0),
			//			uchar(40 < -fastThreshold?1:40 > fastThreshold?2:0),
			//			uchar(41 < -fastThreshold?1:41 > fastThreshold?2:0),
			//			uchar(42 < -fastThreshold?1:42 > fastThreshold?2:0),
			//			uchar(43 < -fastThreshold?1:43 > fastThreshold?2:0),
			//			uchar(44 < -fastThreshold?1:44 > fastThreshold?2:0),
			//			uchar(45 < -fastThreshold?1:45 > fastThreshold?2:0),
			//			uchar(46 < -fastThreshold?1:46 > fastThreshold?2:0),
			//			uchar(47 < -fastThreshold?1:47 > fastThreshold?2:0),
			//			uchar(48 < -fastThreshold?1:48 > fastThreshold?2:0),
			//			uchar(49 < -fastThreshold?1:49 > fastThreshold?2:0),
			//			uchar(50 < -fastThreshold?1:50 > fastThreshold?2:0),
			//			uchar(51 < -fastThreshold?1:51 > fastThreshold?2:0),
			//			uchar(52 < -fastThreshold?1:52 > fastThreshold?2:0),
			//			uchar(53 < -fastThreshold?1:53 > fastThreshold?2:0),
			//			uchar(54 < -fastThreshold?1:54 > fastThreshold?2:0),
			//			uchar(55 < -fastThreshold?1:55 > fastThreshold?2:0),
			//			uchar(56 < -fastThreshold?1:56 > fastThreshold?2:0),
			//			uchar(57 < -fastThreshold?1:57 > fastThreshold?2:0),
			//			uchar(58 < -fastThreshold?1:58 > fastThreshold?2:0),
			//			uchar(59 < -fastThreshold?1:59 > fastThreshold?2:0),
			//			uchar(60 < -fastThreshold?1:60 > fastThreshold?2:0),
			//			uchar(61 < -fastThreshold?1:61 > fastThreshold?2:0),
			//			uchar(62 < -fastThreshold?1:62 > fastThreshold?2:0),
			//			uchar(63 < -fastThreshold?1:63 > fastThreshold?2:0),
			//			uchar(64 < -fastThreshold?1:64 > fastThreshold?2:0),
			//			uchar(65 < -fastThreshold?1:65 > fastThreshold?2:0),
			//			uchar(66 < -fastThreshold?1:66 > fastThreshold?2:0),
			//			uchar(67 < -fastThreshold?1:67 > fastThreshold?2:0),
			//			uchar(68 < -fastThreshold?1:68 > fastThreshold?2:0),
			//			uchar(69 < -fastThreshold?1:69 > fastThreshold?2:0),
			//			uchar(70 < -fastThreshold?1:70 > fastThreshold?2:0),
			//			uchar(71 < -fastThreshold?1:71 > fastThreshold?2:0),
			//			uchar(72 < -fastThreshold?1:72 > fastThreshold?2:0),
			//			uchar(73 < -fastThreshold?1:73 > fastThreshold?2:0),
			//			uchar(74 < -fastThreshold?1:74 > fastThreshold?2:0),
			//			uchar(75 < -fastThreshold?1:75 > fastThreshold?2:0),
			//			uchar(76 < -fastThreshold?1:76 > fastThreshold?2:0),
			//			uchar(77 < -fastThreshold?1:77 > fastThreshold?2:0),
			//			uchar(78 < -fastThreshold?1:78 > fastThreshold?2:0),
			//			uchar(79 < -fastThreshold?1:79 > fastThreshold?2:0),
			//			uchar(80 < -fastThreshold?1:80 > fastThreshold?2:0),
			//			uchar(81 < -fastThreshold?1:81 > fastThreshold?2:0),
			//			uchar(82 < -fastThreshold?1:82 > fastThreshold?2:0),
			//			uchar(83 < -fastThreshold?1:83 > fastThreshold?2:0),
			//			uchar(84 < -fastThreshold?1:84 > fastThreshold?2:0),
			//			uchar(85 < -fastThreshold?1:85 > fastThreshold?2:0),
			//			uchar(86 < -fastThreshold?1:86 > fastThreshold?2:0),
			//			uchar(87 < -fastThreshold?1:87 > fastThreshold?2:0),
			//			uchar(88 < -fastThreshold?1:88 > fastThreshold?2:0),
			//			uchar(89 < -fastThreshold?1:89 > fastThreshold?2:0),
			//			uchar(90 < -fastThreshold?1:90 > fastThreshold?2:0),
			//			uchar(91 < -fastThreshold?1:91 > fastThreshold?2:0),
			//			uchar(92 < -fastThreshold?1:92 > fastThreshold?2:0),
			//			uchar(93 < -fastThreshold?1:93 > fastThreshold?2:0),
			//			uchar(94 < -fastThreshold?1:94 > fastThreshold?2:0),
			//			uchar(95 < -fastThreshold?1:95 > fastThreshold?2:0),
			//			uchar(96 < -fastThreshold?1:96 > fastThreshold?2:0),
			//			uchar(97 < -fastThreshold?1:97 > fastThreshold?2:0),
			//			uchar(98 < -fastThreshold?1:98 > fastThreshold?2:0),
			//			uchar(99 < -fastThreshold?1:99 > fastThreshold?2:0),
			//			uchar(100 < -fastThreshold?1:100 > fastThreshold?2:0),
			//			uchar(101 < -fastThreshold?1:101 > fastThreshold?2:0),
			//			uchar(102 < -fastThreshold?1:102 > fastThreshold?2:0),
			//			uchar(103 < -fastThreshold?1:103 > fastThreshold?2:0),
			//			uchar(104 < -fastThreshold?1:104 > fastThreshold?2:0),
			//			uchar(105 < -fastThreshold?1:105 > fastThreshold?2:0),
			//			uchar(106 < -fastThreshold?1:106 > fastThreshold?2:0),
			//			uchar(107 < -fastThreshold?1:107 > fastThreshold?2:0),
			//			uchar(108 < -fastThreshold?1:108 > fastThreshold?2:0),
			//			uchar(109 < -fastThreshold?1:109 > fastThreshold?2:0),
			//			uchar(110 < -fastThreshold?1:110 > fastThreshold?2:0),
			//			uchar(111 < -fastThreshold?1:111 > fastThreshold?2:0),
			//			uchar(112 < -fastThreshold?1:112 > fastThreshold?2:0),
			//			uchar(113 < -fastThreshold?1:113 > fastThreshold?2:0),
			//			uchar(114 < -fastThreshold?1:114 > fastThreshold?2:0),
			//			uchar(115 < -fastThreshold?1:115 > fastThreshold?2:0),
			//			uchar(116 < -fastThreshold?1:116 > fastThreshold?2:0),
			//			uchar(117 < -fastThreshold?1:117 > fastThreshold?2:0),
			//			uchar(118 < -fastThreshold?1:118 > fastThreshold?2:0),
			//			uchar(119 < -fastThreshold?1:119 > fastThreshold?2:0),
			//			uchar(120 < -fastThreshold?1:120 > fastThreshold?2:0),
			//			uchar(121 < -fastThreshold?1:121 > fastThreshold?2:0),
			//			uchar(122 < -fastThreshold?1:122 > fastThreshold?2:0),
			//			uchar(123 < -fastThreshold?1:123 > fastThreshold?2:0),
			//			uchar(124 < -fastThreshold?1:124 > fastThreshold?2:0),
			//			uchar(125 < -fastThreshold?1:125 > fastThreshold?2:0),
			//			uchar(126 < -fastThreshold?1:126 > fastThreshold?2:0),
			//			uchar(127 < -fastThreshold?1:127 > fastThreshold?2:0),
			//			uchar(128 < -fastThreshold?1:128 > fastThreshold?2:0),
			//			uchar(129 < -fastThreshold?1:129 > fastThreshold?2:0),
			//			uchar(130 < -fastThreshold?1:130 > fastThreshold?2:0),
			//			uchar(131 < -fastThreshold?1:131 > fastThreshold?2:0),
			//			uchar(132 < -fastThreshold?1:132 > fastThreshold?2:0),
			//			uchar(133 < -fastThreshold?1:133 > fastThreshold?2:0),
			//			uchar(134 < -fastThreshold?1:134 > fastThreshold?2:0),
			//			uchar(135 < -fastThreshold?1:135 > fastThreshold?2:0),
			//			uchar(136 < -fastThreshold?1:136 > fastThreshold?2:0),
			//			uchar(137 < -fastThreshold?1:137 > fastThreshold?2:0),
			//			uchar(138 < -fastThreshold?1:138 > fastThreshold?2:0),
			//			uchar(139 < -fastThreshold?1:139 > fastThreshold?2:0),
			//			uchar(140 < -fastThreshold?1:140 > fastThreshold?2:0),
			//			uchar(141 < -fastThreshold?1:141 > fastThreshold?2:0),
			//			uchar(142 < -fastThreshold?1:142 > fastThreshold?2:0),
			//			uchar(143 < -fastThreshold?1:143 > fastThreshold?2:0),
			//			uchar(144 < -fastThreshold?1:144 > fastThreshold?2:0),
			//			uchar(145 < -fastThreshold?1:145 > fastThreshold?2:0),
			//			uchar(146 < -fastThreshold?1:146 > fastThreshold?2:0),
			//			uchar(147 < -fastThreshold?1:147 > fastThreshold?2:0),
			//			uchar(148 < -fastThreshold?1:148 > fastThreshold?2:0),
			//			uchar(149 < -fastThreshold?1:149 > fastThreshold?2:0),
			//			uchar(150 < -fastThreshold?1:150 > fastThreshold?2:0),
			//			uchar(151 < -fastThreshold?1:151 > fastThreshold?2:0),
			//			uchar(152 < -fastThreshold?1:152 > fastThreshold?2:0),
			//			uchar(153 < -fastThreshold?1:153 > fastThreshold?2:0),
			//			uchar(154 < -fastThreshold?1:154 > fastThreshold?2:0),
			//			uchar(155 < -fastThreshold?1:155 > fastThreshold?2:0),
			//			uchar(156 < -fastThreshold?1:156 > fastThreshold?2:0),
			//			uchar(157 < -fastThreshold?1:157 > fastThreshold?2:0),
			//			uchar(158 < -fastThreshold?1:158 > fastThreshold?2:0),
			//			uchar(159 < -fastThreshold?1:159 > fastThreshold?2:0),
			//			uchar(160 < -fastThreshold?1:160 > fastThreshold?2:0),
			//			uchar(161 < -fastThreshold?1:161 > fastThreshold?2:0),
			//			uchar(162 < -fastThreshold?1:162 > fastThreshold?2:0),
			//			uchar(163 < -fastThreshold?1:163 > fastThreshold?2:0),
			//			uchar(164 < -fastThreshold?1:164 > fastThreshold?2:0),
			//			uchar(165 < -fastThreshold?1:165 > fastThreshold?2:0),
			//			uchar(166 < -fastThreshold?1:166 > fastThreshold?2:0),
			//			uchar(167 < -fastThreshold?1:167 > fastThreshold?2:0),
			//			uchar(168 < -fastThreshold?1:168 > fastThreshold?2:0),
			//			uchar(169 < -fastThreshold?1:169 > fastThreshold?2:0),
			//			uchar(170 < -fastThreshold?1:170 > fastThreshold?2:0),
			//			uchar(171 < -fastThreshold?1:171 > fastThreshold?2:0),
			//			uchar(172 < -fastThreshold?1:172 > fastThreshold?2:0),
			//			uchar(173 < -fastThreshold?1:173 > fastThreshold?2:0),
			//			uchar(174 < -fastThreshold?1:174 > fastThreshold?2:0),
			//			uchar(175 < -fastThreshold?1:175 > fastThreshold?2:0),
			//			uchar(176 < -fastThreshold?1:176 > fastThreshold?2:0),
			//			uchar(177 < -fastThreshold?1:177 > fastThreshold?2:0),
			//			uchar(178 < -fastThreshold?1:178 > fastThreshold?2:0),
			//			uchar(179 < -fastThreshold?1:179 > fastThreshold?2:0),
			//			uchar(180 < -fastThreshold?1:180 > fastThreshold?2:0),
			//			uchar(181 < -fastThreshold?1:181 > fastThreshold?2:0),
			//			uchar(182 < -fastThreshold?1:182 > fastThreshold?2:0),
			//			uchar(183 < -fastThreshold?1:183 > fastThreshold?2:0),
			//			uchar(184 < -fastThreshold?1:184 > fastThreshold?2:0),
			//			uchar(185 < -fastThreshold?1:185 > fastThreshold?2:0),
			//			uchar(186 < -fastThreshold?1:186 > fastThreshold?2:0),
			//			uchar(187 < -fastThreshold?1:187 > fastThreshold?2:0),
			//			uchar(188 < -fastThreshold?1:188 > fastThreshold?2:0),
			//			uchar(189 < -fastThreshold?1:189 > fastThreshold?2:0),
			//			uchar(190 < -fastThreshold?1:190 > fastThreshold?2:0),
			//			uchar(191 < -fastThreshold?1:191 > fastThreshold?2:0),
			//			uchar(192 < -fastThreshold?1:192 > fastThreshold?2:0),
			//			uchar(193 < -fastThreshold?1:193 > fastThreshold?2:0),
			//			uchar(194 < -fastThreshold?1:194 > fastThreshold?2:0),
			//			uchar(195 < -fastThreshold?1:195 > fastThreshold?2:0),
			//			uchar(196 < -fastThreshold?1:196 > fastThreshold?2:0),
			//			uchar(197 < -fastThreshold?1:197 > fastThreshold?2:0),
			//			uchar(198 < -fastThreshold?1:198 > fastThreshold?2:0),
			//			uchar(199 < -fastThreshold?1:199 > fastThreshold?2:0),
			//			uchar(200 < -fastThreshold?1:200 > fastThreshold?2:0),
			//			uchar(201 < -fastThreshold?1:201 > fastThreshold?2:0),
			//			uchar(202 < -fastThreshold?1:202 > fastThreshold?2:0),
			//			uchar(203 < -fastThreshold?1:203 > fastThreshold?2:0),
			//			uchar(204 < -fastThreshold?1:204 > fastThreshold?2:0),
			//			uchar(205 < -fastThreshold?1:205 > fastThreshold?2:0),
			//			uchar(206 < -fastThreshold?1:206 > fastThreshold?2:0),
			//			uchar(207 < -fastThreshold?1:207 > fastThreshold?2:0),
			//			uchar(208 < -fastThreshold?1:208 > fastThreshold?2:0),
			//			uchar(209 < -fastThreshold?1:209 > fastThreshold?2:0),
			//			uchar(210 < -fastThreshold?1:210 > fastThreshold?2:0),
			//			uchar(211 < -fastThreshold?1:211 > fastThreshold?2:0),
			//			uchar(212 < -fastThreshold?1:212 > fastThreshold?2:0),
			//			uchar(213 < -fastThreshold?1:213 > fastThreshold?2:0),
			//			uchar(214 < -fastThreshold?1:214 > fastThreshold?2:0),
			//			uchar(215 < -fastThreshold?1:215 > fastThreshold?2:0),
			//			uchar(216 < -fastThreshold?1:216 > fastThreshold?2:0),
			//			uchar(217 < -fastThreshold?1:217 > fastThreshold?2:0),
			//			uchar(218 < -fastThreshold?1:218 > fastThreshold?2:0),
			//			uchar(219 < -fastThreshold?1:219 > fastThreshold?2:0),
			//			uchar(220 < -fastThreshold?1:220 > fastThreshold?2:0),
			//			uchar(221 < -fastThreshold?1:221 > fastThreshold?2:0),
			//			uchar(222 < -fastThreshold?1:222 > fastThreshold?2:0),
			//			uchar(223 < -fastThreshold?1:223 > fastThreshold?2:0),
			//			uchar(224 < -fastThreshold?1:224 > fastThreshold?2:0),
			//			uchar(225 < -fastThreshold?1:225 > fastThreshold?2:0),
			//			uchar(226 < -fastThreshold?1:226 > fastThreshold?2:0),
			//			uchar(227 < -fastThreshold?1:227 > fastThreshold?2:0),
			//			uchar(228 < -fastThreshold?1:228 > fastThreshold?2:0),
			//			uchar(229 < -fastThreshold?1:229 > fastThreshold?2:0),
			//			uchar(230 < -fastThreshold?1:230 > fastThreshold?2:0),
			//			uchar(231 < -fastThreshold?1:231 > fastThreshold?2:0),
			//			uchar(232 < -fastThreshold?1:232 > fastThreshold?2:0),
			//			uchar(233 < -fastThreshold?1:233 > fastThreshold?2:0),
			//			uchar(234 < -fastThreshold?1:234 > fastThreshold?2:0),
			//			uchar(235 < -fastThreshold?1:235 > fastThreshold?2:0),
			//			uchar(236 < -fastThreshold?1:236 > fastThreshold?2:0),
			//			uchar(237 < -fastThreshold?1:237 > fastThreshold?2:0),
			//			uchar(238 < -fastThreshold?1:238 > fastThreshold?2:0),
			//			uchar(239 < -fastThreshold?1:239 > fastThreshold?2:0),
			//			uchar(240 < -fastThreshold?1:240 > fastThreshold?2:0),
			//			uchar(241 < -fastThreshold?1:241 > fastThreshold?2:0),
			//			uchar(242 < -fastThreshold?1:242 > fastThreshold?2:0),
			//			uchar(243 < -fastThreshold?1:243 > fastThreshold?2:0),
			//			uchar(244 < -fastThreshold?1:244 > fastThreshold?2:0),
			//			uchar(245 < -fastThreshold?1:245 > fastThreshold?2:0),
			//			uchar(246 < -fastThreshold?1:246 > fastThreshold?2:0),
			//			uchar(247 < -fastThreshold?1:247 > fastThreshold?2:0),
			//			uchar(248 < -fastThreshold?1:248 > fastThreshold?2:0),
			//			uchar(249 < -fastThreshold?1:249 > fastThreshold?2:0),
			//			uchar(250 < -fastThreshold?1:250 > fastThreshold?2:0),
			//			uchar(251 < -fastThreshold?1:251 > fastThreshold?2:0),
			//			uchar(252 < -fastThreshold?1:252 > fastThreshold?2:0),
			//			uchar(253 < -fastThreshold?1:253 > fastThreshold?2:0),
			//			uchar(254 < -fastThreshold?1:254 > fastThreshold?2:0),
			//			uchar(255 < -fastThreshold?1:255 > fastThreshold?2:0)
			//		}
			//		{
			//		}
			//
			//	public:
			//
			//
			//		bool doResponse() const override
			//		{
			//			return scoreType!=RETAIN_ALL_NO_SCORE;
			//		}
			//		bool doAngles() const override
			//		{
			//			return false;
			//		}
			//
			//		size_t getBorder() const override
			//		{
			//			return border;
			//		}
			//
			//		void writeProperties(boost::property_tree::ptree& properties) const override
			//		{
			//			properties.put<std::string>("keyPointType","fast");
			//			properties.put<size_t>("patternSize",(size_t)patternSize);
			//			switch(scoreType)
			//			{
			//				case NON_MAX_SUPPRESSION:
			//					properties.put<std::string>("scoreType","nonMaxSuppression");
			//					break;
			//				case RETAIN_ALL:
			//					properties.put<std::string>("scoreType","retainAll");
			//					break;
			//				case RETAIN_ALL_NO_SCORE:
			//					properties.put<std::string>("scoreType","none");
			//					break;
			//				default:
			//					CV_Error(1,"unknown scoreType "+std::to_string(scoreType));
			//			}
			//			properties.put<bool>("compatibilityMode",compatibilityMode);
			//			properties.put<size_t>("border",border);
			//			properties.put<uchar>("fastThreshold",uchar(fastThreshold));
			//		}
			//
			//		FastKeyPointGenerator<patternSize,scoreType,compatibilityMode,hal>* getKeyPointGenerator(size_t cols,size_t rows,float kpSize,int octave,float,float,int) const override;
			//
			//	};
			//
			//
			//	template<PatternSize patternSize,KPRetentionPolicy scoreType,bool compatibilityMode,HAL_FLAG hal>
			//	class FastKeyPointGenerator<patternSize,scoreType,compatibilityMode,hal,std::enable_if_t<scoreType==NON_MAX_SUPPRESSION>>: public KeyPointGenerator,private matrix::Mat1D<uchar>
			//	{
			//	private:
			//		friend class FastKeyPointGeneratorFactory<patternSize,scoreType,compatibilityMode,hal>;
			//
			//		const size_t cols;
			//		const int octave;
			//		const float kpSize;
			//		const FastKeyPointGeneratorFactory<patternSize,scoreType,compatibilityMode,hal>* const factory;
			//		const size_t xMax;
			//		const size_t yMax;
			//		const size_t xEnd;
			//		const size_t yEnd;
			//		const size_t start;
			//		size_t* const positionBuffer[2];
			//		uchar* const scoreBuffer[3];
			//		PatternCircle<patternSize> circle;
			//
			//		constexpr
			//			FastKeyPointGenerator(size_t cols,size_t rows,int octave,float kpSize,const FastKeyPointGeneratorFactory<patternSize,scoreType,compatibilityMode,hal>* factory):
			//			matrix::Mat1D<uchar>((sizeof(size_t)*(cols+1)*3)+(cols*3)),
			//			cols(cols),
			//			octave(octave),
			//			kpSize(kpSize),
			//			factory(factory),
			//			xMax(clipHiBorder(cols,factory->border)),
			//			yMax(clipHiBorder(rows,factory->border)),
			//			xEnd(std::min<size_t>(cols-PatternCircle<patternSize>::getRadius<compatibilityMode>(),xMax+1)),
			//			yEnd(std::min<size_t>(rows-PatternCircle<patternSize>::getRadius<compatibilityMode>(),yMax+1)),
			//			start(std::max<size_t>(PatternCircle<patternSize>::getRadius<compatibilityMode>(),factory->border-1)),
			//			positionBuffer{((size_t*)data)+1,this->positionBuffer[0]+cols+1},
			//			scoreBuffer{(uchar*)(positionBuffer[1]+cols),scoreBuffer[0]+cols,scoreBuffer[1]+cols},
			//			circle(int(cols))
			//		{
			//		}
			//
			//	public:
			//
			//		void detect(const matrix::Ptr2D<uchar>& frame,std::vector<cv::KeyPoint>& keyPoints) override
			//		{
			//			keyPoints.clear();
			//			std::memset(scoreBuffer[0],0,cols*3);
			//			const size_t border=factory->border;
			//			for(size_t y=start;y<=yEnd;++y)
			//			{
			//				uchar* const nextScoreBuffer=scoreBuffer[y%3];
			//				size_t* const nextPositionBuffer=positionBuffer[y%2];
			//				std::memset(nextScoreBuffer,0,cols);
			//				nextPositionBuffer[-1]=0;
			//				if(y!=yEnd)
			//				{
			//					circle.detectAndScore<scoreType,compatibilityMode,hal,hal>(frame.ptr(start,y),start,xEnd,FastKeyPointPostProcessor<scoreType>(nextPositionBuffer,nextScoreBuffer),factory);
			//				}
			//				if(y>border && y<=yMax)
			//				{
			//					const size_t* const currPositionBuffer=positionBuffer[(y-1)%2];
			//					const size_t numCorners=currPositionBuffer[-1];
			//					const uchar* const currScoreBuffer=scoreBuffer[(y-1)%3];
			//					const uchar* const prevScoreBuffer=scoreBuffer[(y-2)%3];
			//					for(size_t i=0; i!=numCorners; ++i)
			//					{
			//						const size_t x=currPositionBuffer[i];
			//						if(x>=border && x < xMax)
			//						{
			//							const int score=currScoreBuffer[x];
			//							if
			//								(
			//									score > prevScoreBuffer[x-1]&&
			//									score > prevScoreBuffer[x]&&
			//									score > prevScoreBuffer[x+1]&&
			//									score > currScoreBuffer[x-1]&&
			//									score > currScoreBuffer[x+1]&&
			//									score > nextScoreBuffer[x-1]&&
			//									score > nextScoreBuffer[x]&&
			//									score > nextScoreBuffer[x+1]
			//									)
			//							{
			//								keyPoints.emplace_back(float(x),float(y-1),kpSize,-1.f,float(score),octave,-1);
			//							}
			//						}
			//					}
			//				}
			//			}
			//		}
			//	};
			//
			//	template<PatternSize patternSize,KPRetentionPolicy scoreType,bool compatibilityMode,HAL_FLAG hal>
			//	class FastKeyPointGenerator<patternSize,scoreType,compatibilityMode,hal,std::enable_if_t<scoreType==RETAIN_ALL||scoreType==RETAIN_ALL_NO_SCORE>>: public KeyPointGenerator
			//	{
			//	private:
			//		friend class FastKeyPointGeneratorFactory<patternSize,scoreType,compatibilityMode,hal>;
			//
			//		const size_t cols;
			//		const int octave;
			//		const float kpSize;
			//		const FastKeyPointGeneratorFactory<patternSize,scoreType,compatibilityMode,hal>* const factory;
			//		const size_t xMax;
			//		const size_t yMax;
			//		const size_t start;
			//		PatternCircle<patternSize> circle;
			//
			//		constexpr
			//			FastKeyPointGenerator(size_t cols,size_t rows,int octave,float kpSize,const FastKeyPointGeneratorFactory<patternSize,scoreType,compatibilityMode,hal>* factory):
			//			cols(cols),
			//			octave(octave),
			//			kpSize(kpSize),
			//			factory(factory),
			//			xMax(clipHiBorder(cols,factory->border)),
			//			yMax(clipHiBorder(rows,factory->border)),
			//			start(std::max<size_t>(PatternCircle<patternSize>::getRadius<compatibilityMode>(),factory->border)),
			//			circle(int(cols))
			//		{}
			//
			//	public:
			//
			//		void detect(const matrix::Ptr2D<uchar>& frame,std::vector<cv::KeyPoint>& keyPoints) override
			//		{
			//			keyPoints.clear();
			//			for(size_t y=start;y<yMax;++y)
			//			{
			//				circle.detectAndScore<scoreType,compatibilityMode,hal,hal>(frame.ptr(start,y),start,xMax,FastKeyPointPostProcessor<scoreType>(keyPoints,float(y),kpSize,octave),factory);
			//			}
			//		}
			//	};
			//
			//	template<PatternSize patternSize,KPRetentionPolicy scoreType,bool compatibilityMode,HAL_FLAG hal>
			//	inline FastKeyPointGenerator<patternSize,scoreType,compatibilityMode,hal>* FastKeyPointGeneratorFactory<patternSize,scoreType,compatibilityMode,hal>::getKeyPointGenerator(size_t cols,size_t rows,float kpSize,int octave,float,float,int) const
			//	{
			//		return new FastKeyPointGenerator<patternSize,scoreType,compatibilityMode,hal>(cols,rows,octave,kpSize,this);
			//	}
			//}
			//
			//boost::property_tree::ptree KeyPointPropertyGenerator<Fast>::operator()(size_t border,uchar fastThreshold,fast::PatternSize patternSize,KPRetentionPolicy scoreType,bool compatibilityMode) const
			//{
			//	boost::property_tree::ptree properties;
			//	properties.put<std::string>("keyPointType","fast");
			//	switch(patternSize)
			//	{
			//		case fast::Fast12:
			//			if(compatibilityMode)
			//			{
			//				border=std::max<size_t>(border,fast::PatternCircle<fast::Fast12>::getRadius<true>());
			//			}
			//			else
			//			{
			//				border=std::max<size_t>(border,fast::PatternCircle<fast::Fast12>::getRadius<false>());
			//			}
			//			properties.put<size_t>("patternSize",12);
			//			break;
			//		case fast::Fast8:
			//			if(compatibilityMode)
			//			{
			//				border=std::max<size_t>(border,fast::PatternCircle<fast::Fast8>::getRadius<true>());
			//			}
			//			else
			//			{
			//				border=std::max<size_t>(border,fast::PatternCircle<fast::Fast8>::getRadius<false>());
			//			}
			//			properties.put<size_t>("patternSize",8);
			//			break;
			//		default:
			//			if(compatibilityMode)
			//			{
			//				border=std::max<size_t>(border,fast::PatternCircle<fast::Fast16>::getRadius<true>());
			//			}
			//			else
			//			{
			//				border=std::max<size_t>(border,fast::PatternCircle<fast::Fast16>::getRadius<false>());
			//			}
			//			properties.put<size_t>("patternSize",16);
			//			break;
			//	}
			//	switch(scoreType)
			//	{
			//		case RETAIN_ALL:
			//			properties.put<std::string>("scoreType","retainAll");
			//			break;
			//		case RETAIN_ALL_NO_SCORE:
			//			properties.put<std::string>("scoreType","none");
			//			break;
			//		default:
			//			properties.put<std::string>("scoreType","nonMaxSuppression");
			//			break;
			//	}
			//	properties.put<bool>("compatibilityMode",compatibilityMode);
			//	properties.put<uchar>("fastThreshold",fastThreshold);
			//	properties.put<size_t>("border",border);
			//	return properties;
			//}

			boost::property_tree::ptree KeyPointPropertyGenerator<Fast>::getProperties()
			{
				boost::property_tree::ptree properties;
				properties.put<std::string>("keyPointType",kpTypeStr<Fast>);
				properties.put<std::string>("shape",segmenttest::patternShapeToStr(shape));
				properties.put<std::string>("retentionPolicy",kpRetentionPolicyToStr(retentionPolicy));
				properties.put<size_t>("patternSize",patternSize);
				properties.put<bool>("compatibilityMode",compatibilityMode);
				properties.put<uchar>("fastThreshold",fastThreshold);
				properties.put<size_t>("border",border);
				return properties;
			}


//			template<>
//			KeyPointGeneratorFactory* KeyPointGeneratorFactory::configureKeyPointGeneratorFactory<Fast>(const boost::property_tree::ptree & properties,const lyonste::hal::HAL_FLAG hal_flag)
//			{
//#define GET_FAST_04(patternSize,scoreType,compatibilityMode,hal)\
//return new fast::FastKeyPointGeneratorFactory<patternSize,scoreType,compatibilityMode,hal>(border,short(fastThreshold));
//
//#define GET_FAST_03(patternSize,scoreType,compatibilityMode)\
//if(lyonste::hal::supports<lyonste::hal::HAL_SSE2>(hal_flag))\
//{\
//	GET_FAST_04(patternSize,scoreType,compatibilityMode,lyonste::hal::HAL_SSE2)\
//}\
//else\
//{\
//	GET_FAST_04(patternSize,scoreType,compatibilityMode,lyonste::hal::HAL_NONE)\
//}
//
//#define GET_FAST_02(patternSize,scoreType)\
//if(compatibilityMode)\
//{\
//	GET_FAST_03(patternSize,scoreType,true)\
//}\
//else\
//{\
//	GET_FAST_03(patternSize,scoreType,false)\
//}
//
//#define GET_FAST_01(patternSize)\
//if(scoreType=="nonMaxSuppression")\
//{\
//	GET_FAST_02(patternSize,NON_MAX_SUPPRESSION)\
//}\
//else if(scoreType=="retainAll")\
//{\
//	GET_FAST_02(patternSize,RETAIN_ALL)}\
//else if(scoreType=="none")\
//{\
//	GET_FAST_02(patternSize,RETAIN_ALL_NO_SCORE)\
//}\
//else\
//{\
//	CV_Error(1,"Unknown scoreType"+scoreType);\
//}
//
//
//
//				const size_t patternSize=properties.get<size_t>("patternSize",16);
//				const std::string scoreType=properties.get<std::string>("scoreType","nonMaxSuppression");
//				const bool compatibilityMode=properties.get<bool>("compatibilityMode",true);
//				const size_t border=properties.get<size_t>("border",0);
//				const uchar fastThreshold=properties.get<uchar>("fastThreshold",10);
//				switch(patternSize)
//				{
//					case 16:
//						GET_FAST_01(fast::Fast16)
//					case 12:
//						GET_FAST_01(fast::Fast12)
//					case 8:
//						GET_FAST_01(fast::Fast8)
//					default:
//						CV_Error(1,"unknown fast patternSize: "+std::to_string(patternSize));
//				}
//				return NULL;
//			}

			template<>
			cv::Ptr<cv::Feature2D> KeyPointGeneratorFactory::configureOpenCVKeyPointGenerator<Fast>(const boost::property_tree::ptree& properties,bool ignoreIncompatibleProperties)
			{
				const std::string retentionPolicy=properties.get<std::string>("retentionPolicy",kpRetentionPolicyStr<NON_MAX_SUPPRESSION>);
				if(!ignoreIncompatibleProperties)
				{
					const std::string shape=properties.get<std::string>("shape",segmenttest::patternShapeStr<segmenttest::Circle>);
					if(shape!=segmenttest::patternShapeStr<segmenttest::Circle>)
					{
						CV_Error(1,"Cannot create cv::FastFeatureDetector with shape="+shape);
					}
					if(retentionPolicy!=kpRetentionPolicyStr<NON_MAX_SUPPRESSION> && retentionPolicy!=kpRetentionPolicyStr<RETAIN_ALL_NO_SCORE>)
					{
						CV_Error(1,"Cannot create cv::FastFeatureDetector with retentionPolicy="+retentionPolicy);
					}
					if(!properties.get<bool>("compatibilityMode",true))
					{
						CV_Error(1,"Cannot create cv::FastFeatureDetector with compatibilityMode=false");
					}
					const size_t border=properties.get<size_t>("border",0);
					if(border>3)
					{
						CV_Error(1,"Cannot create cv::FastFeatureDetector with border="+std::to_string(border));
					}
				}
				const size_t patternSize=properties.get<size_t>("patternSize",16);
				int type;
				switch(patternSize)
				{
					case 12:
						type=cv::FastFeatureDetector::TYPE_7_12;
						break;
					case 8:
						type=cv::FastFeatureDetector::TYPE_5_8;
						break;
					default:
						if(!ignoreIncompatibleProperties)
						{
							CV_Error(1,"Cannot create cv::FastFeatureDetector with patternSize="+std::to_string(patternSize));
						}
					case 16:
						type=cv::FastFeatureDetector::TYPE_9_16;
				}
				return cv::FastFeatureDetector::create((int)properties.get<uchar>("fastThreshold",10),retentionPolicy==kpRetentionPolicyStr<NON_MAX_SUPPRESSION>,type);
			}

			template<size_t patternSize,segmenttest::PatternShape shape,bool compatibilityMode,lyonste::hal::HAL_FLAG hal>
			KeyPointGeneratorFactory* getKPGenFactoryHelper04(const boost::property_tree::ptree& properties)
			{
				const size_t border=properties.get<size_t>("border",segmenttest::patternBorder<compatibilityMode,patternSize,shape,void>);
				const short fastThreshold=short(properties.get<uchar>("fastThreshold",10));
				const std::string retentionPolicy=properties.get<std::string>("retentionPolicy",kpRetentionPolicyStr<NON_MAX_SUPPRESSION>);
				
				if(retentionPolicy==kpRetentionPolicyStr<NON_MAX_SUPPRESSION>)
				{
					return new segmenttest::fast::FastKeyPointGeneratorFactory<patternSize,shape,NON_MAX_SUPPRESSION,compatibilityMode,hal>(border,fastThreshold);
				}
				if(retentionPolicy==kpRetentionPolicyStr<RETAIN_ALL_NO_SCORE>)
				{
					return new segmenttest::fast::FastKeyPointGeneratorFactory<patternSize,shape,RETAIN_ALL_NO_SCORE,compatibilityMode,hal>(border,fastThreshold);
				}
				if(retentionPolicy==kpRetentionPolicyStr<RETAIN_ALL>)
				{
					return new segmenttest::fast::FastKeyPointGeneratorFactory<patternSize,shape,RETAIN_ALL,compatibilityMode,hal>(border,fastThreshold);
				}
				CV_Error(1,"Unknown KPRetentionPolicy "+retentionPolicy);
				return nullptr;
			}

			template<size_t patternSize,bool compatibilityMode,lyonste::hal::HAL_FLAG hal>
			KeyPointGeneratorFactory* getKPGenFactoryHelper03(const boost::property_tree::ptree& properties)
			{
				const std::string shape=properties.get<std::string>("shape",segmenttest::patternShapeStr<segmenttest::Circle>);
				if(shape==segmenttest::patternShapeStr<segmenttest::Circle>)
				{
					return getKPGenFactoryHelper04<patternSize,segmenttest::Circle,compatibilityMode,hal>(properties);
				}
				if(shape==segmenttest::patternShapeStr<segmenttest::Diamond>)
				{
					return getKPGenFactoryHelper04<patternSize,segmenttest::Diamond,compatibilityMode,hal>(properties);
				}
				CV_Error(1,"Unknown PatternShape "+shape);
				return nullptr;
			}

			template<size_t patternSize,lyonste::hal::HAL_FLAG hal>
			KeyPointGeneratorFactory* getKPGenFactoryHelper02(const boost::property_tree::ptree& properties)
			{
				if(properties.get<bool>("compatibilityMode",true))
				{
					return getKPGenFactoryHelper03<patternSize,true,hal>(properties);
				}
				else
				{
					return getKPGenFactoryHelper03<patternSize,false,hal>(properties);
				}
			}

			template<lyonste::hal::HAL_FLAG hal>
			KeyPointGeneratorFactory* getKPGenFactoryHelper01(const boost::property_tree::ptree& properties)
			{
				const size_t patternSize=properties.get<size_t>("patternSize",16);
				switch(patternSize)
				{
					case 16:
						return getKPGenFactoryHelper02<16,hal>(properties);
					case 12:
						return getKPGenFactoryHelper02<12,hal>(properties);
					case 8:
						return getKPGenFactoryHelper02<8,hal>(properties);
					default:
						CV_Error(1,"Unknown patternSize "+std::to_string(patternSize));
						return nullptr;
				}
			}

			template<>
			KeyPointGeneratorFactory* KeyPointGeneratorFactory::configureKeyPointGeneratorFactory<Fast>(const boost::property_tree::ptree & properties,const lyonste::hal::HAL_FLAG hal_flag)
			{
				if(lyonste::hal::supports<lyonste::hal::HAL_SSE2>(hal_flag))
				{
					return getKPGenFactoryHelper01<lyonste::hal::HAL_SSE2>(properties);
				}
				else
				{
					return getKPGenFactoryHelper01<lyonste::hal::HAL_NONE>(properties);
				}
			}

		}
	}
}