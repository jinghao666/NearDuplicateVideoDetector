#include "featureDetection.h"
#include "../../matrix/resizer.h"

namespace lyonste
{
	namespace feature2D
	{
		namespace featureDetection
		{
			using namespace lyonste::hal;
			namespace orb
			{


				const double SQRT2=std::sqrt(2.0);
				const float SQRT_2_DIV_2=float(SQRT2)/2;

				constexpr const int* const initializeUMax(int* const uMax,const size_t patchSize)
				{
					const size_t halfPatchSize=patchSize>>1;
					const size_t vMax=size_t(halfPatchSize*SQRT_2_DIV_2+1);//cvFloor(halfPatchSize*SQRT_2_DIV_2+1);
					const size_t vMin=ceiling(halfPatchSize*SQRT_2_DIV_2);//cvCeil(halfPatchSize*SQRT_2_DIV_2);
					const double halfPatch2=(double)halfPatchSize*(double)halfPatchSize;
					for(size_t v=0; v<=vMax; ++v)
					{
						uMax[v]=int(cvRound(std::sqrt(halfPatch2-v*v)));
					}
					for(size_t v=halfPatchSize,v0=0; v>=vMin; --v)
					{
						while(uMax[v0]==uMax[v0+1])
						{
							++v0;
						}
						uMax[v]=int(v0);
						++v0;
					}
					return uMax;
				}
				template<bool harrisScore> constexpr const size_t* const initializeNumFeaturesPerLevel(size_t* const numFeaturesPerLevel,const double scaleFactor,const size_t numFeatures,const size_t numLevels)
				{
					const float factor=(float)(1.0/scaleFactor);
					float numDesiredFeaturesPerScale=numFeatures*(1-factor)/(1-(float)std::pow((double)factor,(double)numLevels));
					size_t sumFeatures=0;
					const size_t bound=numLevels-1;
					for(size_t i=0; i!=bound; ++i)
					{
						const size_t numFeaturesForLevel=cvRound(numDesiredFeaturesPerScale);
						numFeaturesPerLevel[i]=harrisScore?(numFeaturesForLevel<<1):(numFeaturesForLevel);
						sumFeatures+=numFeaturesForLevel;
						numDesiredFeaturesPerScale*=factor;
					}
					if(numFeatures<=sumFeatures)
					{
						numFeaturesPerLevel[bound]=0;
					}
					else
					{
						numFeaturesPerLevel[bound]=harrisScore?((numFeatures-sumFeatures)<<1):(numFeatures-sumFeatures);
					}
					return numFeaturesPerLevel;
				}
				constexpr const float* const initializeLayerScales(float* const layerScales,const size_t numLevels,const double scaleFactor)
				{
					for(size_t i=1; i!=numLevels; ++i)
					{
						layerScales[i-1]=(float)(std::pow(scaleFactor,(double)i));
					}
					return layerScales;
				}
				constexpr matrix::Mat2D<uchar>** const initializeResizedFrames(matrix::Mat2D<uchar>**const resizedFrames,const float* const layerScales,const size_t cols,const size_t rows,const size_t numLevelsMinus1)
				{

					for(size_t i=0;i!=numLevelsMinus1;++i)
					{
						const float layerScale=layerScales[i];
						const size_t levelCols=size_t(cvRound(cols/layerScale));
						const size_t levelRows=size_t(cvRound(rows/layerScale));
						resizedFrames[i]=new matrix::Mat2D<uchar>(levelCols,levelRows);
					}
					return resizedFrames;
				}
				template<HAL_FLAG hal> constexpr matrix::resizer::GrayMat2DResizer<hal>** const initializeResizers(matrix::resizer::GrayMat2DResizer<hal>** const resizers,matrix::Mat2D<uchar>** const resizedFrames,size_t prevCols,size_t prevRows,const size_t numLevelsMinus1)
				{
					for(size_t i=0;i!=numLevelsMinus1;++i)
					{
						const matrix::Mat2D<uchar>* levelResizedFrame=resizedFrames[i];
						const size_t levelCols=levelResizedFrame->x;
						const size_t levelRows=levelResizedFrame->y;
						resizers[i]=matrix::resizer::GrayMat2DResizer<hal>::getResizer(prevCols,prevRows,levelCols,levelRows);
						prevCols=levelCols;
						prevRows=levelRows;
					}
					return resizers;
				}
				BestResponseKeyPointCuller** const initializeKPGens(BestResponseKeyPointCuller** const kpGens,matrix::Mat2D<uchar>** const resizedFrames,const size_t cols,const size_t rows,const float* const layerScales,const size_t* const numFeaturesPerLevel,const float fPatchSize,const KeyPointGeneratorFactory* layerFactory,const size_t numLevels)
				{
					kpGens[0]=new BestResponseKeyPointCuller(layerFactory->getKeyPointGenerator(cols,rows,fPatchSize),numFeaturesPerLevel[0]);
					for(size_t i=1;i<numLevels;++i)
					{
						const  matrix::Mat2D<uchar>* levelResizedFrame=resizedFrames[i-1];
						kpGens[i]=new BestResponseKeyPointCuller(layerFactory->getKeyPointGenerator(levelResizedFrame->x,levelResizedFrame->y,fPatchSize*layerScales[i-1],int(i)),numFeaturesPerLevel[i]);
					}
					return kpGens;
				}

				constexpr const size_t* initializeHarrisBlocks(size_t* const harrisBlocks,const size_t numLevels,const size_t cols,matrix::Mat2D<uchar>** const resizedFrames)
				{
					for(size_t i=0; i!=7; ++i)
					{
						for(size_t j=0; j!=7; ++j)
						{
							harrisBlocks[i*7+j]=(i*cols+j);
						}
					}
					for(size_t level=1;level!=numLevels;++level)
					{
						const size_t levelCols=resizedFrames[level-1]->x;
						for(size_t i=0; i!=7; ++i)
						{
							for(size_t j=0; j!=7; ++j)
							{
								harrisBlocks[(7*7*level)+(i*7+j)]=(i*levelCols+j);
							}
						}
					}
					return harrisBlocks;
				}

				constexpr float calculateHarrisScore(const uchar* srcPtr,const size_t* harrisBlock,const int cols,const float harrisK=0.04f)
				{
					srcPtr=srcPtr-(cols*(7>>1))-(7>>1);
					const float scale=1.f/(4*7*255.f);
					const float scale4=scale*scale*scale*scale;
					int a=0;
					int b=0;
					int c=0;
					for(size_t k=0; k < 7*7; ++k)
					{
						const uchar* srcPtr0=srcPtr+harrisBlock[k];
						int iX=((srcPtr0[1]-srcPtr0[-1])<<1)+(srcPtr0[-cols+1]-srcPtr0[-cols-1])+(srcPtr0[cols+1]-srcPtr0[+cols-1]);
						int iY=((srcPtr0[cols]-srcPtr0[-cols])<<1)+(srcPtr0[+cols-1]-srcPtr0[-cols-1])+(srcPtr0[cols+1]-srcPtr0[-cols+1]);
						a+=iX*iX;
						b+=iY*iY;
						c+=iX*iY;
					}
					return ((float)a * b-(float)c * c-harrisK * ((float)a+b) * ((float)a+b))*scale4;
				}

				inline float calculateOrbAngle(const uchar* const srcPtr,const int* const uMax,const size_t cols,const int halfPatchSize)
				{
					int m01=0;
					int m10=0;
					for(int i=-halfPatchSize;i<=halfPatchSize;++i)
					{
						m10+=i*srcPtr[i];
					}
					for(int i=1;i<=halfPatchSize;++i)
					{
						int vSum=0;
						int d=uMax[i];
						const size_t verticalOffset=i*cols;
						for(int j=-d; j<=d; ++j)
						{
							int valP=srcPtr[j+verticalOffset];
							int valM=srcPtr[j-verticalOffset];
							vSum+=(valP-valM);
							m10+=j*(valP+valM);
						}
						m01+=i*vSum;
					}
					return cv::fastAtan2((float)m01,(float)m10);
				}


				template<bool harrisScore=true,bool calculateAngles=true,HAL_FLAG hal=HAL_NONE>
				class SingleLevelOrbKeyPointGenerator;

				template<bool harrisScore=true,bool calculateAngles=true,HAL_FLAG hal=HAL_NONE>
				class MultiLevelOrbKeyPointGenerator;

				template<bool multiLevel=true,bool harrisScore=true,bool calculateAngles=true,HAL_FLAG hal=HAL_NONE>
				class OrbKeyPointGeneratorFactory;

				template<bool harrisScore,HAL_FLAG hal>
				class OrbKeyPointGeneratorFactory<false,harrisScore,false,hal>: public KeyPointGeneratorFactory
				{
				private:
					friend class SingleLevelOrbKeyPointGenerator<harrisScore,false,hal>;
					friend KeyPointGeneratorFactory* KeyPointGeneratorFactory::configureKeyPointGeneratorFactory<Orb>(const boost::property_tree::ptree& properties,const lyonste::hal::HAL_FLAG hal_flag);
					const size_t numFeatures;
					const size_t patchSize;
					const KeyPointGeneratorFactory* const layerFactory;
					constexpr OrbKeyPointGeneratorFactory(const size_t numFeatures,const size_t patchSize,const KeyPointGeneratorFactory* const layerFactory):
						numFeatures(numFeatures),
						patchSize(patchSize),
						layerFactory(layerFactory)
					{}

					constexpr void finalizeKeyPoints(const matrix::Ptr2D<uchar>& frame,std::vector<cv::KeyPoint>& keyPoints) const
					{
						//nothing to do
					}

				public:

					std::string toString() const noexcept override
					{
						return std::string(kpTypeStr<Orb>)+
							+"{numFeatures="+std::to_string(numFeatures)
							+"; numLevels=1; scaleFactor=1.0; patchSize="+std::to_string(patchSize)
							+"; harrisScore="+(harrisScore?"true":"false")
							+"; calculateAngles=false; hal="+std::to_string(hal)
							+"; layerFactory="+(layerFactory->operator std::string())+"}";
					}

					bool doResponse() const override
					{
						return harrisScore||layerFactory->doResponse();
					}
					bool doAngles() const override
					{
						return false;
					}
					size_t getBorder() const override
					{
						return layerFactory->getBorder();
					}
					SingleLevelOrbKeyPointGenerator<harrisScore,false,hal>* getKeyPointGenerator(size_t cols,size_t rows,float kpSize=7.f,int octave=0,float angle=-1,float response=0,int classID=-1) const;
					void writeProperties(boost::property_tree::ptree& properties) const
					{
						properties.put<std::string>("keyPointType",kpTypeStr<Orb>);
						boost::property_tree::ptree layerFactoryProperties;
						layerFactory->writeProperties(layerFactoryProperties);
						properties.add_child("layerFactoryProperties",layerFactoryProperties);
						properties.put<size_t>("numLevels",1);
						properties.put<bool>("harrisScore",harrisScore);
						properties.put<bool>("calculateAngles",false);
						properties.put<size_t>("numFeatures",numFeatures);
						properties.put<size_t>("patchSize",patchSize);
					}
					~OrbKeyPointGeneratorFactory()
					{
						delete layerFactory;
					}
				};

				template<bool harrisScore,HAL_FLAG hal>
				class OrbKeyPointGeneratorFactory<false,harrisScore,true,hal>: public KeyPointGeneratorFactory,private matrix::Mat1D<int>
				{
				private:
					friend class SingleLevelOrbKeyPointGenerator<harrisScore,true,hal>;
					friend KeyPointGeneratorFactory* KeyPointGeneratorFactory::configureKeyPointGeneratorFactory<Orb>(const boost::property_tree::ptree& properties,const lyonste::hal::HAL_FLAG hal_flag);
					const size_t numFeatures;
					const size_t patchSize;
					const int* const uMax;
					const KeyPointGeneratorFactory* const layerFactory;

					constexpr OrbKeyPointGeneratorFactory(const size_t numFeatures,const size_t patchSize,const KeyPointGeneratorFactory* const layerFactory):
						matrix::Mat1D<int>((patchSize<<1)+2),
						numFeatures(numFeatures),
						patchSize(patchSize),
						uMax(initializeUMax(data,patchSize)),
						layerFactory(layerFactory)
					{}


					constexpr void finalizeKeyPoints(const matrix::Ptr2D<uchar>& frame,std::vector<cv::KeyPoint>& keyPoints) const
					{
						const size_t cols=frame.x;
						const int halfPatchSize=int(patchSize>>1);
						for(cv::KeyPoint& kp:keyPoints)
						{
							const cv::Point2f& pt=kp.pt;
							kp.angle=calculateOrbAngle(frame.ptr(size_t(pt.x),size_t(pt.y)),uMax,cols,halfPatchSize);
						}
					}

				public:

					std::string toString() const noexcept override
					{
						return std::string(kpTypeStr<Orb>)+
							+"{numFeatures="+std::to_string(numFeatures)
							+"; numLevels=1; scaleFactor=1.0; patchSize="+std::to_string(patchSize)
							+"; harrisScore="+(harrisScore?"true":"false")
							+"; calculateAngles=true; hal="+std::to_string(hal)
							+"; layerFactory="+(layerFactory->operator std::string())+"}";
					}

					bool doResponse() const override
					{
						return harrisScore||layerFactory->doResponse();
					}
					bool doAngles() const override
					{
						return true;
					}
					size_t getBorder() const override
					{
						return layerFactory->getBorder();
					}
					SingleLevelOrbKeyPointGenerator<harrisScore,true,hal>* getKeyPointGenerator(size_t cols,size_t rows,float kpSize=7.f,int octave=0,float angle=-1,float response=0,int classID=-1) const;
					void writeProperties(boost::property_tree::ptree& properties) const
					{
						properties.put<std::string>("keyPointType",kpTypeStr<Orb>);
						boost::property_tree::ptree layerFactoryProperties;
						layerFactory->writeProperties(layerFactoryProperties);
						properties.add_child("layerFactoryProperties",layerFactoryProperties);
						properties.put<size_t>("numLevels",1);
						properties.put<bool>("harrisScore",harrisScore);
						properties.put<bool>("calculateAngles",true);
						properties.put<size_t>("numFeatures",numFeatures);
						properties.put<size_t>("patchSize",patchSize);
					}

				};

				template<bool harrisScore,HAL_FLAG hal>
				class OrbKeyPointGeneratorFactory<true,harrisScore,false,hal>: public KeyPointGeneratorFactory,private matrix::Mat1D<uchar>
				{
				private:
					friend class MultiLevelOrbKeyPointGenerator<harrisScore,false,hal>;
					friend KeyPointGeneratorFactory* KeyPointGeneratorFactory::configureKeyPointGeneratorFactory<Orb>(const boost::property_tree::ptree& properties,const lyonste::hal::HAL_FLAG hal_flag);
					const size_t numFeatures;
					const size_t patchSize;
					const size_t numLevels;
					const double scaleFactor;
					const size_t* const numFeaturesPerLevel;
					const float* const layerScales;
					const KeyPointGeneratorFactory* const layerFactory;
					constexpr OrbKeyPointGeneratorFactory(const size_t numFeatures,const size_t patchSize,const size_t numLevels,const float scaleFactor,const KeyPointGeneratorFactory* const layerFactory):
						matrix::Mat1D<uchar>((sizeof(size_t)*numLevels)+(sizeof(float)*(numLevels-1))),
						numFeatures(numFeatures),
						patchSize(patchSize),
						numLevels(numLevels),
						scaleFactor(scaleFactor),
						numFeaturesPerLevel(initializeNumFeaturesPerLevel<harrisScore>((size_t*)data,scaleFactor,numFeatures,numLevels)),
						layerScales(initializeLayerScales((float*)(numFeaturesPerLevel+numLevels),numLevels,scaleFactor)),
						layerFactory(layerFactory)
					{}

					constexpr void finalizeKeyPoints(const matrix::Ptr2D<uchar>& frame,std::vector<cv::KeyPoint>& keyPoints,matrix::Mat2D<uchar>** const resizedFrames) const
					{
						for(cv::KeyPoint& kp:keyPoints)
						{
							const int octave=kp.octave;
							if(octave)
							{
								kp.pt*=layerScales[octave-1];
							}
						}
					}


				public:
					bool doResponse() const override
					{
						return harrisScore||layerFactory->doResponse();
					}
					bool doAngles() const override
					{
						return false;
					}
					size_t getBorder() const override
					{
						return layerFactory->getBorder();
					}

					std::string toString() const noexcept override
					{
						return std::string(kpTypeStr<Orb>)+
							+"{numFeatures="+std::to_string(numFeatures)
							+"; numLevels="+std::to_string(numLevels)
							+"; scaleFactor="+std::to_string(scaleFactor)
							+"; patchSize="+std::to_string(patchSize)
							+"; harrisScore="+(harrisScore?"true":"false")
							+"; calculateAngles=false; hal="+std::to_string(hal)
							+"; layerFactory="+(layerFactory->operator std::string())+"}";
					}

					MultiLevelOrbKeyPointGenerator<harrisScore,false,hal>* getKeyPointGenerator(size_t cols,size_t rows,float kpSize=7.f,int octave=0,float angle=-1,float response=0,int classID=-1) const;

					void writeProperties(boost::property_tree::ptree& properties) const
					{
						properties.put<std::string>("keyPointType",kpTypeStr<Orb>);
						boost::property_tree::ptree layerFactoryProperties;
						layerFactory->writeProperties(layerFactoryProperties);
						properties.add_child("layerFactoryProperties",layerFactoryProperties);
						properties.put<size_t>("numLevels",numLevels);
						properties.put<bool>("harrisScore",harrisScore);
						properties.put<bool>("calculateAngles",false);
						properties.put<size_t>("numFeatures",numFeatures);
						properties.put<size_t>("patchSize",patchSize);
						properties.put<double>("scaleFactor",scaleFactor);
					}
					~OrbKeyPointGeneratorFactory()
					{
						delete layerFactory;
					}
				};

				template<bool harrisScore,HAL_FLAG hal>
				class OrbKeyPointGeneratorFactory<true,harrisScore,true,hal>: public KeyPointGeneratorFactory,private matrix::Mat1D<uchar>
				{
				private:
					friend class MultiLevelOrbKeyPointGenerator<harrisScore,true,hal>;
					friend KeyPointGeneratorFactory* KeyPointGeneratorFactory::configureKeyPointGeneratorFactory<Orb>(const boost::property_tree::ptree& properties,const lyonste::hal::HAL_FLAG hal_flag);
					const size_t numFeatures;
					const size_t patchSize;
					const size_t numLevels;
					const double scaleFactor;
					const size_t* const numFeaturesPerLevel;
					const int* const uMax;
					const float* const layerScales;
					const KeyPointGeneratorFactory* const layerFactory;
					constexpr OrbKeyPointGeneratorFactory(const size_t numFeatures,const size_t patchSize,const size_t numLevels,const float scaleFactor,const KeyPointGeneratorFactory* const layerFactory):
						matrix::Mat1D<uchar>(sizeof(size_t)*numLevels+sizeof(int)*((patchSize<<1)+2)+(sizeof(float)*(numLevels-1))),
						numFeatures(numFeatures),
						patchSize(patchSize),
						numLevels(numLevels),
						scaleFactor(scaleFactor),
						numFeaturesPerLevel(initializeNumFeaturesPerLevel<harrisScore>((size_t*)data,scaleFactor,numFeatures,numLevels)),
						uMax(initializeUMax((int*)(numFeaturesPerLevel+numLevels),patchSize)),
						layerScales(initializeLayerScales((float*)(uMax+(patchSize<<1)+2),numLevels,scaleFactor)),
						layerFactory(layerFactory)
					{}

					constexpr void finalizeKeyPoints(const matrix::Ptr2D<uchar>& frame,std::vector<cv::KeyPoint>& keyPoints,matrix::Mat2D<uchar>** const resizedFrames) const
					{
						const int halfPatchSize=int(patchSize>>1);
						for(cv::KeyPoint& kp:keyPoints)
						{
							const int octave=kp.octave;
							size_t levelCols;
							const uchar* levelPtr;
							if(octave)
							{
								cv::Point2f& pt=kp.pt;
								const matrix::Mat2D<uchar>* levelFrame=resizedFrames[octave-1];
								levelCols=levelFrame->x;
								levelPtr=levelFrame->ptr(size_t(pt.x),size_t(pt.y));
								pt*=layerScales[octave-1];
							}
							else
							{
								const cv::Point2f& pt=kp.pt;
								levelCols=frame.x;
								levelPtr=frame.ptr(size_t(pt.x),size_t(pt.y));
							}
							kp.angle=calculateOrbAngle(levelPtr,uMax,levelCols,halfPatchSize);
						}
					}

				public:
					bool doResponse() const override
					{
						return harrisScore||layerFactory->doResponse();
					}
					bool doAngles() const override
					{
						return true;
					}
					size_t getBorder() const override
					{
						return layerFactory->getBorder();
					}

					std::string toString() const noexcept override
					{
						return std::string(kpTypeStr<Orb>)+
							+"{numFeatures="+std::to_string(numFeatures)
							+"; numLevels="+std::to_string(numLevels)
							+"; scaleFactor="+std::to_string(scaleFactor)
							+"; patchSize="+std::to_string(patchSize)
							+"; harrisScore="+(harrisScore?"true":"false")
							+"; calculateAngles=true; hal="+std::to_string(hal)
							+"; layerFactory="+(layerFactory->operator std::string())+"}";
					}

					MultiLevelOrbKeyPointGenerator<harrisScore,true,hal>* getKeyPointGenerator(size_t cols,size_t rows,float kpSize=7.f,int octave=0,float angle=-1,float response=0,int classID=-1) const;

					void writeProperties(boost::property_tree::ptree& properties) const
					{
						properties.put<std::string>("keyPointType",kpTypeStr<Orb>);
						boost::property_tree::ptree layerFactoryProperties;
						layerFactory->writeProperties(layerFactoryProperties);
						properties.add_child("layerFactoryProperties",layerFactoryProperties);
						properties.put<size_t>("numLevels",numLevels);
						properties.put<bool>("harrisScore",harrisScore);
						properties.put<bool>("calculateAngles",true);
						properties.put<size_t>("numFeatures",numFeatures);
						properties.put<size_t>("patchSize",patchSize);
						properties.put<double>("scaleFactor",scaleFactor);
					}
					~OrbKeyPointGeneratorFactory()
					{
						delete layerFactory;
					}
				};




				template<bool calculateAngles,HAL_FLAG hal>
				class SingleLevelOrbKeyPointGenerator<false,calculateAngles,hal>: public KeyPointGenerator
				{
				private:



					friend class OrbKeyPointGeneratorFactory<false,false,calculateAngles,hal>;
					const size_t cols;
					const size_t rows;
					const OrbKeyPointGeneratorFactory<false,false,calculateAngles,hal>* const factory;
					BestResponseKeyPointCuller kpGen;

					constexpr
						SingleLevelOrbKeyPointGenerator(const size_t cols,const size_t rows,const OrbKeyPointGeneratorFactory<false,false,calculateAngles,hal>* const factory):
						cols(cols),
						rows(rows),
						factory(factory),
						kpGen(factory->layerFactory->getKeyPointGenerator(cols,rows,float(factory->patchSize)),factory->numFeatures)
					{}
				public:
					const OrbKeyPointGeneratorFactory<false,false,calculateAngles,hal>* getFactory() const noexcept
					{
						return factory;
					}

					void detect(const matrix::Ptr2D<uchar>& frame,std::vector<cv::KeyPoint>& keyPoints) override
					{
						keyPoints.clear();
						keyPoints.reserve(factory->numFeatures);
						kpGen.detect(frame,keyPoints);
						factory->finalizeKeyPoints(frame,keyPoints);
					}
				};

				template<bool calculateAngles,HAL_FLAG hal>
				class SingleLevelOrbKeyPointGenerator<true,calculateAngles,hal>: public KeyPointGenerator
				{
				private:
					friend class OrbKeyPointGeneratorFactory<false,true,calculateAngles,hal>;
					const size_t cols;
					const size_t rows;
					const OrbKeyPointGeneratorFactory<false,true,calculateAngles,hal>* const factory;
					BestResponseKeyPointCuller kpGen;
					const size_t harrisBlock[49];

					constexpr
						SingleLevelOrbKeyPointGenerator(const size_t cols,const size_t rows,const OrbKeyPointGeneratorFactory<false,true,calculateAngles,hal>* const factory):
						cols(cols),
						rows(rows),
						factory(factory),
						kpGen(factory->layerFactory->getKeyPointGenerator(cols,rows,float(factory->patchSize)),factory->numFeatures<<1),
						harrisBlock
					{
						0,
						1,
						2,
						3,
						4,
						5,
						6,
						cols,
						cols+1,
						cols+2,
						cols+3,
						cols+4,
						cols+5,
						cols+6,
						(cols<<1),
						(cols<<1)+1,
						(cols<<1)+2,
						(cols<<1)+3,
						(cols<<1)+4,
						(cols<<1)+5,
						(cols<<1)+6,
						(cols*3),
						(cols*3)+1,
						(cols*3)+2,
						(cols*3)+3,
						(cols*3)+4,
						(cols*3)+5,
						(cols*3)+6,
						(cols<<2),
						(cols<<2)+1,
						(cols<<2)+2,
						(cols<<2)+3,
						(cols<<2)+4,
						(cols<<2)+5,
						(cols<<2)+6,
						(cols*5),
						(cols*5)+1,
						(cols*5)+2,
						(cols*5)+3,
						(cols*5)+4,
						(cols*5)+5,
						(cols*5)+6,
						(cols*6),
						(cols*6)+1,
						(cols*6)+2,
						(cols*6)+3,
						(cols*6)+4,
						(cols*6)+5,
						(cols*6)+6,
					}
					{}
				public:

					const OrbKeyPointGeneratorFactory<false,true,calculateAngles,hal>* getFactory() const noexcept
					{
						return factory;
					}
					void detect(const matrix::Ptr2D<uchar>& frame,std::vector<cv::KeyPoint>& keyPoints) override
					{
						keyPoints.clear();
						const size_t numFeatures=factory->numFeatures;
						keyPoints.reserve(numFeatures);
						kpGen.detect(frame,keyPoints);
						if(!keyPoints.empty())
						{
							const int cols=int(this->cols);
							for(cv::KeyPoint& kp:keyPoints)
							{
								const cv::Point2f& pt=kp.pt;
								kp.response=calculateHarrisScore(frame.ptr(size_t(pt.x),size_t(pt.y)),harrisBlock,cols);
							}
							BestResponseKeyPointCuller::retainBestKeyPoints(keyPoints,numFeatures);
							factory->finalizeKeyPoints(frame,keyPoints);
						}
					}
				};

				template<bool calculateAngles,HAL_FLAG hal>
				class MultiLevelOrbKeyPointGenerator<false,calculateAngles,hal>: public KeyPointGenerator,private matrix::Mat1D<uchar>
				{
				private:
					friend class OrbKeyPointGeneratorFactory<true,false,calculateAngles,hal>;
					const size_t cols;
					const size_t rows;
					const OrbKeyPointGeneratorFactory<true,false,calculateAngles,hal>* const factory;
					matrix::Mat2D<uchar>** const resizedFrames;
					matrix::resizer::GrayMat2DResizer<hal>** const resizers;
					BestResponseKeyPointCuller** const kpGens;
					std::vector<cv::KeyPoint> tmpKeyPoints0;

					//delegate constructor
					constexpr MultiLevelOrbKeyPointGenerator(const size_t numLevels,const float* const layerScales,const size_t cols,const size_t rows,const OrbKeyPointGeneratorFactory<true,false,calculateAngles,hal>* const factory):
						matrix::Mat1D<uchar>(numLevels*(sizeof(BestResponseKeyPointCuller*))+(numLevels-1)*(sizeof(matrix::resizer::GrayMat2DResizer<hal>*)+sizeof(matrix::Mat2D<uchar>*))),
						cols(cols),
						rows(rows),
						factory(factory),
						resizedFrames(initializeResizedFrames((matrix::Mat2D<uchar>**)data,layerScales,cols,rows,numLevels-1)),
						resizers(initializeResizers<hal>((matrix::resizer::GrayMat2DResizer<hal>**)(resizedFrames+numLevels-1),resizedFrames,cols,rows,numLevels-1)),
						kpGens(initializeKPGens((BestResponseKeyPointCuller**)(resizers+numLevels-1),resizedFrames,cols,rows,layerScales,factory->numFeaturesPerLevel,float(factory->patchSize),factory->layerFactory,numLevels))
					{

					}

					constexpr MultiLevelOrbKeyPointGenerator(const size_t cols,const size_t rows,const OrbKeyPointGeneratorFactory<true,false,calculateAngles,hal>* const factory):
						MultiLevelOrbKeyPointGenerator(factory->numLevels,factory->layerScales,cols,rows,factory)
					{

					}

				public:

					const OrbKeyPointGeneratorFactory<true,false,calculateAngles,hal>* getFactory() const noexcept
					{
						return factory;
					}

					void detect(const matrix::Ptr2D<uchar>& frame,std::vector<cv::KeyPoint>& keyPoints) override
					{
						keyPoints.clear();
						keyPoints.reserve(factory->numFeatures);
						kpGens[0]->detect(frame,keyPoints);
						const matrix::Ptr2D<uchar>* prevFrame=&frame;
						const size_t numLevels=factory->numLevels;
						for(size_t i=1;i!=numLevels;++i)
						{
							resizers[i-1]->resize(*prevFrame,*resizedFrames[i-1]);
							tmpKeyPoints0.clear();
							kpGens[i]->detect(*(prevFrame=resizedFrames[i-1]),tmpKeyPoints0);
							std::copy(tmpKeyPoints0.begin(),tmpKeyPoints0.end(),std::back_inserter(keyPoints));
						}
						factory->finalizeKeyPoints(frame,keyPoints,resizedFrames);
					}
					~MultiLevelOrbKeyPointGenerator()
					{
						size_t i=factory->numLevels-1;
						delete kpGens[i];
						for(;i--;)
						{
							delete resizers[i];
							delete kpGens[i];
							delete resizedFrames[i];
						}
					}
				};

				template<bool calculateAngles,HAL_FLAG hal>
				class MultiLevelOrbKeyPointGenerator<true,calculateAngles,hal>: public KeyPointGenerator,private matrix::Mat1D<uchar>
				{
				private:
					friend class OrbKeyPointGeneratorFactory<true,true,calculateAngles,hal>;
					const size_t cols;
					const size_t rows;
					const OrbKeyPointGeneratorFactory<true,true,calculateAngles,hal>* const factory;
					matrix::Mat2D<uchar>** const resizedFrames;
					matrix::resizer::GrayMat2DResizer<hal>** const resizers;
					BestResponseKeyPointCuller** const kpGens;
					const size_t* const harrisBlocks;
					size_t* const counters;
					std::vector<cv::KeyPoint> tmpKeyPoints0;
					std::vector<cv::KeyPoint> tmpKeyPoints1;

					//delegate constructor
					constexpr MultiLevelOrbKeyPointGenerator(const size_t numLevels,const float* const layerScales,const size_t cols,const size_t rows,const OrbKeyPointGeneratorFactory<true,true,calculateAngles,hal>* const factory):
						matrix::Mat1D<uchar>(numLevels*sizeof(BestResponseKeyPointCuller*)+sizeof(size_t)*(numLevels*7*7+numLevels)+(numLevels-1)*(sizeof(matrix::resizer::GrayMat2DResizer<hal>*)+sizeof(matrix::Mat2D<uchar>*))),
						cols(cols),
						rows(rows),
						factory(factory),
						resizedFrames(initializeResizedFrames((matrix::Mat2D<uchar>**)data,layerScales,cols,rows,numLevels-1)),
						resizers(initializeResizers<hal>((matrix::resizer::GrayMat2DResizer<hal>**)(resizedFrames+numLevels-1),resizedFrames,cols,rows,numLevels-1)),
						kpGens(initializeKPGens((BestResponseKeyPointCuller**)(resizers+numLevels-1),resizedFrames,cols,rows,layerScales,factory->numFeaturesPerLevel,float(factory->patchSize),factory->layerFactory,numLevels)),
						harrisBlocks(initializeHarrisBlocks((size_t*)(kpGens+numLevels),numLevels,cols,resizedFrames)),
						counters((size_t*)(harrisBlocks+(7*7*numLevels)))
					{

					}

					constexpr MultiLevelOrbKeyPointGenerator(const size_t cols,const size_t rows,const OrbKeyPointGeneratorFactory<true,true,calculateAngles,hal>* const factory):
						MultiLevelOrbKeyPointGenerator(factory->numLevels,factory->layerScales,cols,rows,factory)
					{

					}

				public:

					const OrbKeyPointGeneratorFactory<true,true,calculateAngles,hal>* getFactory() const noexcept
					{
						return factory;
					}
					void detect(const matrix::Ptr2D<uchar>& frame,std::vector<cv::KeyPoint>& keyPoints) override
					{
						keyPoints.clear();
						keyPoints.reserve(factory->numFeatures);
						kpGens[0]->detect(frame,keyPoints);
						counters[0]=keyPoints.size();
						const matrix::Ptr2D<uchar>* prevFrame=&frame;
						const size_t numLevels=factory->numLevels;
						for(size_t i=1;i!=numLevels;++i)
						{
							resizers[i-1]->resize(*prevFrame,*resizedFrames[i-1]);
							tmpKeyPoints0.clear();
							kpGens[i]->detect(*(prevFrame=resizedFrames[i-1]),tmpKeyPoints0);
							counters[i]=tmpKeyPoints0.size();
							std::copy(tmpKeyPoints0.begin(),tmpKeyPoints0.end(),std::back_inserter(keyPoints));
						}
						if(!keyPoints.empty())
						{
							for(cv::KeyPoint& kp:keyPoints)
							{
								const int octave=kp.octave;
								const cv::Point2f& pt=kp.pt;
								int levelCols;
								const uchar* levelPtr;
								if(octave)
								{
									const matrix::Mat2D<uchar>* levelFrame=resizedFrames[octave-1];
									levelCols=int(levelFrame->x);
									levelPtr=levelFrame->ptr(size_t(pt.x),size_t(pt.y));
								}
								else
								{
									levelCols=int(frame.x);
									levelPtr=frame.ptr(size_t(pt.x),size_t(pt.y));
								}
								kp.response=calculateHarrisScore(levelPtr,harrisBlocks+(octave*7*7),levelCols);
							}
							tmpKeyPoints0.clear();
							size_t kpOffset=0;
							auto kpBegin=keyPoints.cbegin();
							const size_t* numFeaturesPerLevel=factory->numFeaturesPerLevel;
							for(size_t i=0;i!=numLevels;++i)
							{
								const size_t count=counters[i];
								tmpKeyPoints1.resize(count);
								std::copy(kpBegin+kpOffset,kpBegin+kpOffset+count,tmpKeyPoints1.begin());
								kpOffset+=count;
								BestResponseKeyPointCuller::retainBestKeyPoints(tmpKeyPoints1,numFeaturesPerLevel[i]>>1);
								std::copy(tmpKeyPoints1.begin(),tmpKeyPoints1.end(),std::back_inserter(tmpKeyPoints0));
							}
							std::swap(keyPoints,tmpKeyPoints0);
							factory->finalizeKeyPoints(frame,keyPoints,resizedFrames);
						}
					}

					~MultiLevelOrbKeyPointGenerator()
					{
						size_t i=factory->numLevels-1;
						delete kpGens[i];
						for(;i--;)
						{
							delete resizers[i];
							delete kpGens[i];
							delete resizedFrames[i];
						}
					}
				};

				template<bool harrisScore,HAL_FLAG hal>
				SingleLevelOrbKeyPointGenerator<harrisScore,false,hal>* lyonste::feature2D::featureDetection::orb::OrbKeyPointGeneratorFactory<false,harrisScore,false,hal>::getKeyPointGenerator(size_t cols,size_t rows,float kpSize,int octave,float angle,float response,int classID) const
				{
					return new SingleLevelOrbKeyPointGenerator<harrisScore,false,hal>(cols,rows,this);
				}

				template<bool harrisScore,HAL_FLAG hal>
				SingleLevelOrbKeyPointGenerator<harrisScore,true,hal>* lyonste::feature2D::featureDetection::orb::OrbKeyPointGeneratorFactory<false,harrisScore,true,hal>::getKeyPointGenerator(size_t cols,size_t rows,float kpSize,int octave,float angle,float response,int classID) const
				{
					return new SingleLevelOrbKeyPointGenerator<harrisScore,true,hal>(cols,rows,this);
				}

				template<bool harrisScore,HAL_FLAG hal>
				MultiLevelOrbKeyPointGenerator<harrisScore,false,hal>* lyonste::feature2D::featureDetection::orb::OrbKeyPointGeneratorFactory<true,harrisScore,false,hal>::getKeyPointGenerator(size_t cols,size_t rows,float kpSize,int octave,float angle,float response,int classID) const
				{
					return new MultiLevelOrbKeyPointGenerator<harrisScore,false,hal>(cols,rows,this);
				}

				template<bool harrisScore,HAL_FLAG hal>
				MultiLevelOrbKeyPointGenerator<harrisScore,true,hal>* lyonste::feature2D::featureDetection::orb::OrbKeyPointGeneratorFactory<true,harrisScore,true,hal>::getKeyPointGenerator(size_t cols,size_t rows,float kpSize,int octave,float angle,float response,int classID) const
				{
					return new MultiLevelOrbKeyPointGenerator<harrisScore,true,hal>(cols,rows,this);
				}

			}

			boost::property_tree::ptree lyonste::feature2D::featureDetection::KeyPointPropertyGenerator<Orb>::getProperties()
			{
				CV_Assert(patchSize<2);
				CV_Assert(numLevels);
				CV_Assert(numFeatures);
				CV_Assert(numLevels<2||(scaleFactor==scaleFactor&&scaleFactor>1.0f));
				boost::property_tree::ptree layerPropertiesCopy=layerProperties;
				for(auto itr=layerPropertiesCopy.find("border");itr!=layerPropertiesCopy.not_found()&&itr->first=="border";++itr)
				{
					size_t border=itr->second.get_value<size_t>();
					if(calculateAngles)
					{
						const size_t angleBorderReq=ceiling((patchSize>>1)*orb::SQRT2);
						border=std::max<size_t>(harrisScore?angleBorderReq:std::max<size_t>(angleBorderReq,4),border);
					}
					else
					{
						border=harrisScore?std::max<size_t>(4,border):border;
					}
					itr->second.put_value<size_t>(border);
				}
				boost::property_tree::ptree properties;
				properties.put<std::string>("keyPointType",kpTypeStr<Orb>);
				properties.put_child("layerFactoryProperties",std::move(layerPropertiesCopy));
				properties.put<bool>("harrisScore",harrisScore);
				properties.put<bool>("calculateAngles",calculateAngles);
				properties.put<size_t>("numFeatures",numFeatures);
				properties.put<size_t>("numLevels",numLevels);
				properties.put<size_t>("patchSize",patchSize);
				properties.put<float>("scaleFactor",scaleFactor);
				return properties;
			}

			template<>
			cv::Ptr<cv::Feature2D> KeyPointGeneratorFactory::configureOpenCVKeyPointGenerator<Orb>(const boost::property_tree::ptree & properties,bool ignoreIncompatibleProperties)
			{
				const boost::property_tree::ptree layerProperties=properties.get_child("layerFactoryProperties");
				const size_t border=layerProperties.get<size_t>("border",31);
				const size_t patchSize=properties.get<size_t>("patchSize",31);
				const bool harrisScore=properties.get<bool>("harrisScore",true);
				const size_t numFeatures=properties.get<size_t>("numFeatures",500);
				const size_t numLevels=properties.get<size_t>("numLevels",8);
				const float scaleFactor=properties.get<float>("scaleFactor",1.2f);
				if(!ignoreIncompatibleProperties)
				{
					const std::string layerKeyPointType=layerProperties.get<std::string>("keyPointType",kpTypeStr<Fast>);
					if(layerKeyPointType!=kpTypeStr<Fast>)
					{
						CV_Error(1,"Cannot create cv::Orb with layerFactoryProperties.keyPointType="+layerKeyPointType);
					}
					const std::string shape=layerProperties.get<std::string>("shape",segmenttest::patternShapeStr<segmenttest::Circle>);
					if(shape!=segmenttest::patternShapeStr<segmenttest::Circle>)
					{
						CV_Error(1,"Cannot create cv::ORB with layerFactoryProperties.shape="+shape);
					}
					const std::string retentionPolicy=layerProperties.get<std::string>("retentionPolicy",kpRetentionPolicyStr<NON_MAX_SUPPRESSION>);
					if(retentionPolicy!=kpRetentionPolicyStr<NON_MAX_SUPPRESSION>)
					{
						CV_Error(1,"Cannot create cv::Orb with layerFactoryProperties.retentionPolicy="+retentionPolicy);
					}
					if(!layerProperties.get<bool>("compatibilityMode",true))
					{
						CV_Error(1,"Cannot create cv::Orb with layerFactoryProperties.compatibilityMode=false");
					}
					const size_t patternSize=layerProperties.get<size_t>("patternSize",16);
					if(patternSize!=16)
					{
						CV_Error(1,"Cannot create cv::Orb with layerFactoryProperties.patternSize="+std::to_string(patternSize));
					}
					if(!properties.get<bool>("calculateAngles",true))
					{
						CV_Error(1,"Cannot create cv::Orb with calculateAngles=false");
					}
					if(border>std::numeric_limits<int>::max())
					{
						CV_Error(1,"Cannot create cv::Orb with border="+std::to_string(border));
					}
					if(patchSize<2||patchSize>std::numeric_limits<int>::max())
					{
						CV_Error(1,"Cannot create cv::Orb with patchSize="+std::to_string(patchSize));
					}
					const size_t angleBorderReq=ceiling((patchSize>>1)*orb::SQRT2);
					if(harrisScore)
					{
						const size_t reqBorder=std::max<size_t>(angleBorderReq,4);
						if(border<reqBorder)
						{
							CV_Error(1,"Cannot create cv::Orb with harrisScore=true and border="+std::to_string(border)+"; min="+std::to_string(reqBorder));
						}
					}
					else if(border<angleBorderReq)
					{
						CV_Error(1,"Cannot create cv::Orb with harrisScore=false and border="+std::to_string(border)+"; min="+std::to_string(angleBorderReq));
					}
					if(numFeatures==0||numFeatures>std::numeric_limits<int>::max())
					{
						CV_Error(1,"Cannot create cv::Orb with numFeatures="+std::to_string(numFeatures));
					}
					if(numLevels==0||numLevels>std::numeric_limits<int>::max())
					{
						CV_Error(1,"Cannot create cv::Orb with numLevels="+std::to_string(numLevels));
					}
					if(scaleFactor!=scaleFactor||scaleFactor<1.0)
					{
						CV_Error(1,"Cannot create cv::Orb with scaleFactor="+std::to_string(scaleFactor));
					}
				}
				return cv::ORB::create((int)numFeatures,scaleFactor,(int)numLevels,(int)border,0,2,harrisScore?cv::ORB::HARRIS_SCORE:cv::ORB::FAST_SCORE,(int)patchSize,(int)layerProperties.get<uchar>("fastThreshold",20));
			}

			template<>
			KeyPointGeneratorFactory* KeyPointGeneratorFactory::configureKeyPointGeneratorFactory<Orb>(const boost::property_tree::ptree& properties,const lyonste::hal::HAL_FLAG hal_flag)
			{
				const bool harrisScore=properties.get<bool>("harrisScore",true);
				const bool calculateAngles=properties.get<bool>("calculateAngles",true);
				const size_t numFeatures=properties.get<size_t>("numFeatures",500);
				const size_t numLevels=properties.get<size_t>("numLevels",8);
				const size_t patchSize=properties.get<size_t>("patchSize",31);
				const float scaleFactor=properties.get<float>("scaleFactor",1.2f);
				const KeyPointGeneratorFactory* layerFactory=KeyPointGeneratorFactory::getKeyPointGeneratorFactory(properties.get_child("layerFactoryProperties"),hal_flag);
				if(numFeatures)
				{
					if(numLevels)
					{
						if(patchSize)
						{
							if(layerFactory)
							{
								const size_t border=layerFactory->getBorder();
								if(calculateAngles)
								{
									const size_t angleBorderReq=ceiling((patchSize>>1)*orb::SQRT2);
									if(harrisScore)
									{
										const size_t reqBorder=std::max<size_t>(angleBorderReq,4);
										if(border>=reqBorder)
										{
											if(numLevels>1)
											{
												if(scaleFactor>1.0)
												{
													if(lyonste::hal::supports<HAL_SSE2>(hal_flag))
													{
														return new orb::OrbKeyPointGeneratorFactory<true,true,true,HAL_SSE2>(numFeatures,patchSize,numLevels,scaleFactor,layerFactory);
													}
													return new orb::OrbKeyPointGeneratorFactory<true,true,true,HAL_NONE>(numFeatures,patchSize,numLevels,scaleFactor,layerFactory);
												}
												CV_Error(1,"scaleFactor "+std::to_string(scaleFactor)+" must be >1");
											}
											return new orb::OrbKeyPointGeneratorFactory<false,true,true,HAL_NONE>(numFeatures,patchSize,layerFactory);
										}
										CV_Error(1,"border = "+std::to_string(border)+"; must be at least "+std::to_string(reqBorder)+"; if patchSize="+std::to_string(patchSize)+"; and harrisScore=true and calculateAngles=true");
									}
									if(border>=angleBorderReq)
									{
										if(numLevels>1)
										{
											if(scaleFactor>1.0)
											{
												if(lyonste::hal::supports<HAL_SSE2>(hal_flag))
												{
													return new orb::OrbKeyPointGeneratorFactory<true,false,true,HAL_SSE2>(numFeatures,patchSize,numLevels,scaleFactor,layerFactory);
												}
												return new orb::OrbKeyPointGeneratorFactory<true,false,true,HAL_NONE>(numFeatures,patchSize,numLevels,scaleFactor,layerFactory);
											}
											CV_Error(1,"scaleFactor "+std::to_string(scaleFactor)+" must be >1");
										}
										return new orb::OrbKeyPointGeneratorFactory<false,false,true,HAL_NONE>(numFeatures,patchSize,layerFactory);
									}
									CV_Error(1,"border = "+std::to_string(border)+"; must be at least "+std::to_string(angleBorderReq)+"; if patchSize="+std::to_string(patchSize)+"; and harrisScore=false and calculateAngles=true");
								}
								if(harrisScore)
								{
									if(border>=4)
									{
										if(numLevels>1)
										{
											if(scaleFactor>1.0)
											{
												if(lyonste::hal::supports<HAL_SSE2>(hal_flag))
												{
													return new orb::OrbKeyPointGeneratorFactory<true,true,false,HAL_SSE2>(numFeatures,patchSize,numLevels,scaleFactor,layerFactory);
												}
												return new orb::OrbKeyPointGeneratorFactory<true,true,false,HAL_NONE>(numFeatures,patchSize,numLevels,scaleFactor,layerFactory);
											}
											CV_Error(1,"scaleFactor "+std::to_string(scaleFactor)+" must be >1");
										}
										return new orb::OrbKeyPointGeneratorFactory<false,true,false,HAL_NONE>(numFeatures,patchSize,layerFactory);
									}
									CV_Error(1,"border = "+std::to_string(border)+"; must be at least 3 if harrisScore=true and calculateAngles=false");
								}
								if(numLevels>1)
								{
									if(scaleFactor>1.0)
									{
										if(lyonste::hal::supports<HAL_SSE2>(hal_flag))
										{
											return new orb::OrbKeyPointGeneratorFactory<true,false,false,HAL_SSE2>(numFeatures,patchSize,numLevels,scaleFactor,layerFactory);
										}
										return new orb::OrbKeyPointGeneratorFactory<true,false,false,HAL_NONE>(numFeatures,patchSize,numLevels,scaleFactor,layerFactory);
									}
									CV_Error(1,"scaleFactor "+std::to_string(scaleFactor)+" must be >1");
								}
								return new orb::OrbKeyPointGeneratorFactory<false,false,false,HAL_NONE>(numFeatures,patchSize,layerFactory);
							}
							CV_Error(1,"layerFactory undefined");
						}
						CV_Error(1,"patchSize must be > 0");
					}
					CV_Error(1,"numLevels must be > 0");
				}
				CV_Error(1,"numFeatures must be > 0");
				return NULL;
			}
		}
	}
}
