#pragma once

#include "videoSig.h"
#include <opencv2\videoio.hpp>
#include "feature2D/featureDetection/featureDetection.h"
#include <boost/exception/diagnostic_information.hpp>

namespace lyonste
{
	namespace videoSig
	{
		namespace videoSigGen
		{
			template<feature2D::featureDescription::DescriptorType descType>
			class SigGenFunction
			{
			protected:
				VideoSigCache<descType>* cache;
				feature2D::featureDetection::KeyPointGeneratorFactory* kpGenFactory;
				feature2D::featureDescription::DescriptorGeneratorFactory<descType>* descGenFactory;
				double keyFrameInterval;
			public:
				constexpr SigGenFunction(VideoSigCache<descType>& cache,const boost::property_tree::ptree& properties):
					cache(&cache),
					kpGenFactory(feature2D::featureDetection::KeyPointGeneratorFactory::getKeyPointGeneratorFactory(properties.get_child("keyPointGeneratorProperties"))),
					descGenFactory(feature2D::featureDescription::DescriptorGeneratorFactory<descType>::getDescriptorGeneratorFactory(properties.get_child("descriptorGeneratorProperities"))),
					keyFrameInterval(properties.get<double>("keyFrameInterval",2000.0))
				{}

				constexpr std::tuple<VideoMetaData,fileManagement::FileInfo,size_t> generateSig(const fileManagement::FileInfo& vidFile) const
				{
					const std::string& fileName=vidFile.getFilePathStr();
					if(vidFile.pathExists())
					{
						cv::VideoCapture videoCapture(fileName);
						if(videoCapture.isOpened())
						{
							cv::Mat captureMat;
							if(videoCapture.read(captureMat))
							{
								size_t cols(captureMat.cols);
								size_t rows(captureMat.rows);
								if(cols && rows)
								{
									std::unique_ptr<feature2D::featureDetection::KeyPointGenerator> kpGen(kpGenFactory->getKeyPointGenerator(cols,rows));
									std::unique_ptr<feature2D::featureDescription::DescriptorGenerator<descType>> descGen(descGenFactory->getDescriptorGenerator(cols,rows));
									std::vector<KeyFramePair<descType>> keyFrames;
									matrix::Mat2D<uchar> gray(captureMat);
									std::vector<cv::KeyPoint> keyPoints;
									double nextPosition=keyFrameInterval;
									double timeIndex;
									for(;;)
									{
										kpGen->detect(gray,keyPoints);
										keyFrames.emplace_back(timeIndex=videoCapture.get(cv::CAP_PROP_POS_MSEC),descGen->compute(gray,keyPoints));
										for(;;)
										{
											if(!videoCapture.grab())
											{
												goto endHere;
											}
											if((timeIndex=videoCapture.get(cv::CAP_PROP_POS_MSEC))>=nextPosition)
											{
												nextPosition+=keyFrameInterval;
												videoCapture.retrieve(captureMat);
												if(!videoCapture.retrieve(captureMat))
												{
													goto endHere;
												}
												gray.overwrite(captureMat);
												break;
											}
										}
									}
endHere:
									//return cache.addSig(VideoMetaData(vidFile,cols,rows,videoCapture.get(cv::CAP_PROP_FRAME_COUNT),videoCapture.get(cv::CAP_PROP_FPS),timeIndex),std::move(keyFrames));
									return cache->addSig(VideoMetaData(vidFile,cols,rows,videoCapture.get(cv::CAP_PROP_FRAME_COUNT),videoCapture.get(cv::CAP_PROP_FPS),timeIndex),std::move(keyFrames));
								}
								CV_Error(1,"Video dimensions for file "+fileName+" are invalid");
							}
							CV_Error(1,"Unable to grab frame from video file "+fileName);
						}
						CV_Error(1,"Unable to open video file "+fileName);
					}
					CV_Error(1,"Video file "+fileName+" does not exist");
					throw std::exception(("Unspecified error on file "+fileName).c_str());//this is here to placate a compiler warning about not returning a value
				}

				inline ~SigGenFunction() noexcept
				{
					delete kpGenFactory;
					delete descGenFactory;
				}
			};

			template<feature2D::featureDescription::DescriptorType descType,class VidFileContainer>
			class SigGenSession
			{
			protected:
				VideoSigCache<descType>* cache;
				size_t numThreads;
				SigGenFunction<descType> sigGenFunction;
				typename VidFileContainer::const_iterator vidFileItr;
				typename VidFileContainer::const_iterator vidFileEnd;
				std::mutex vidFileGetMut;

				constexpr virtual void sigGenThread() noexcept
				{
					for(;;)
					{
						const fileManagement::FileInfo* vidFile;
						{
							std::lock_guard<std::mutex> lock(vidFileGetMut);
							if(vidFileItr==vidFileEnd)
							{
								break;
							}
							vidFile=&(*(vidFileItr++));
						}
						try
						{
							sigGenFunction.generateSig(*vidFile);
						}
						catch(...)
						{

						}
					}
				}

			public:

				constexpr void requestShutdown() noexcept
				{
					std::lock_guard<std::mutex> lock(vidFileGetMut);
					vidFileItr=vidFileEnd;
				}

				constexpr SigGenSession(VidFileContainer& vidFileContainer,VideoSigCache<descType>& cache,const boost::property_tree::ptree& properties):
					cache(&cache),
					numThreads(getNumThreadsProperty(properties)),
					sigGenFunction(cache,properties),
					vidFileItr(vidFileContainer.cbegin()),
					vidFileEnd(vidFileContainer.cend()),
					vidFileGetMut()
				{}

				constexpr void generateSigs()
				{
					std::vector<std::thread> threads;
					size_t numThreads=this->numThreads;
					threads.reserve(--numThreads);
					for(;numThreads--;)
					{
						threads.emplace_back(&SigGenSession<descType,VidFileContainer>::sigGenThread,this);
					}
					sigGenThread();
					for(auto& thread:threads)
					{
						thread.join();
					}
					cache->writeToFile();
				}
			};

			template<feature2D::featureDescription::DescriptorType descType,class VidFileContainer>
			class VerboseSigGenSession: public SigGenSession<descType,VidFileContainer>
			{
			protected:

				static constexpr size_t countFileSize(const VidFileContainer& vidFileContainer)
				{
					size_t size=0;
					for(const auto& vidFile:vidFileContainer)
					{
						size+=vidFile.getFileSize();
					}
					return size;
				}

				std::mutex reportMut;
				size_t finishIndex;
				size_t totalNumVids;
				size_t consumedFileSize;
				size_t totalFileSize;
				std::chrono::system_clock::time_point sessionStartTime;

				constexpr void reportSuccess(const std::chrono::system_clock::time_point& startTime,const std::tuple<VideoMetaData,fileManagement::FileInfo,size_t>& sigTuple) noexcept
				{
					const std::chrono::system_clock::time_point& finishTime=std::chrono::system_clock::now();
					const VideoMetaData& metaData=std::get<0>(sigTuple);
					const fileManagement::FileInfo& vidFile=metaData.getVidFile();
					const size_t vidFileSize=vidFile.getFileSize();
					const std::chrono::system_clock::duration& thisSigGenTime=finishTime-startTime;
					const double elapsedMilliseconds=double(std::chrono::duration_cast<std::chrono::milliseconds>(finishTime-sessionStartTime).count());
					std::stringstream ss;
					ss<<" SUCCESS : "<<vidFile.getFileNameStr()<<std::endl
						<<"\tVid File Size           = "<<DataSizeToString(vidFileSize)<<std::endl
						<<"\tCols                    = "<<(metaData.getCols())<<std::endl
						<<"\tRows                    = "<<(metaData.getRows())<<std::endl
						<<"\tNum Frames              = "<<(metaData.getNumFrames())<<std::endl
						<<"\tFPS                     = "<<(metaData.getFPS())<<std::endl
						<<"\tDuration                = "<<DurationToString(metaData.getDuration())<<std::endl
						<<"\tNum KeyFrames           = "<<(std::get<2>(sigTuple))<<std::endl
						<<"\tKeyFrame File Size      = "<<DataSizeToString(std::get<1>(sigTuple).getFileSize())<<std::endl
						<<"\tSig Generated In        = "<<DurationToString(thisSigGenTime)<<std::endl
						<<"\tTotal Elapsed Time      = "<<DurationToString(elapsedMilliseconds)<<std::endl
						<<"\tData Rate               = ";
					{
						std::lock_guard<std::mutex> lock(reportMut);
						const double bytesPerSecond=double(consumedFileSize+=vidFileSize)/(elapsedMilliseconds/1000.0);
						std::cout<<(++finishIndex)<<" / "<<(totalNumVids)<<ss.str()<<DataSizeToString(bytesPerSecond)<<"/s"<<std::endl
							<<"\tEstimated Time Left     = "<<DurationToString(totalFileSize,consumedFileSize,bytesPerSecond)<<std::endl;
					}
				}

				constexpr void reportFailure(const std::chrono::system_clock::time_point& startTime,const fileManagement::FileInfo& vidFile) noexcept
				{
					const std::chrono::system_clock::time_point& finishTime=std::chrono::system_clock::now();
					const size_t vidFileSize=vidFile.getFileSize();
					const std::chrono::system_clock::duration& thisSigGenTime=finishTime-startTime;
					const double elapsedMilliseconds=double(std::chrono::duration_cast<std::chrono::milliseconds>(finishTime-sessionStartTime).count());
					std::stringstream ss;
					ss<<" FAILURE : "<<vidFile.getFileNameStr()<<std::endl
						<<"\tVid File Size           = "<<(((double)vidFileSize)/(1024.0*1024.0))<<"MB"<<std::endl
						<<"\tSig Failed In           = "<<DurationToString(thisSigGenTime)<<std::endl
						<<"\tTotal Elapsed Time      = "<<DurationToString(elapsedMilliseconds)<<std::endl
						<<"\tData Rate               = ";
					{
						std::lock_guard<std::mutex> lock(reportMut);
						const double bytesPerSecond=double(consumedFileSize)/(elapsedMilliseconds/1000.0);
						std::cout<<(finishIndex)<<" / "<<(--totalNumVids)<<ss.str()<<DataSizeToString(bytesPerSecond)<<"/s"<<std::endl
							<<"\tEstimated Time Left     = "<<DurationToString(totalFileSize-=vidFileSize,consumedFileSize,bytesPerSecond)<<std::endl
							<<"\t"<<boost::current_exception_diagnostic_information()<<std::endl;
					}
				}

				constexpr void sigGenThread() noexcept override
				{
					for(;;)
					{
						const fileManagement::FileInfo* vidFile;
						{
							std::lock_guard<std::mutex> lock(vidFileGetMut);
							if(vidFileItr==vidFileEnd)
							{
								break;
							}
							vidFile=&(*(vidFileItr++));
						}
						const std::chrono::system_clock::time_point& startTime=std::chrono::system_clock::now();
						try
						{
							reportSuccess(startTime,sigGenFunction.generateSig(*vidFile));
						}
						catch(...)
						{
							reportFailure(startTime,*vidFile);
						}
					}
				}

			public:

				constexpr VerboseSigGenSession(VidFileContainer& vidFileContainer,VideoSigCache<descType>& cache,const boost::property_tree::ptree& properties)
					:SigGenSession<descType,VidFileContainer>(vidFileContainer,cache,properties),
					reportMut(),
					finishIndex(0),
					totalNumVids(vidFileContainer.size()),
					consumedFileSize(0),
					totalFileSize(countFileSize(vidFileContainer)),
					sessionStartTime(std::chrono::system_clock::now())
				{

				}
			};

			template<feature2D::featureDescription::DescriptorType descType,class VidFileContainer>
			constexpr SigGenSession<descType,VidFileContainer>* getSigGenSession(VidFileContainer& vidFileContainer,VideoSigCache<descType>& cache,const boost::property_tree::ptree& properties)
			{
				cache.pruneCache(vidFileContainer);
				if(properties.get<boolean>("verbose",true))
				{
					return new VerboseSigGenSession<descType,VidFileContainer>(vidFileContainer,cache,properties);
				}
				return new SigGenSession<descType,VidFileContainer>(vidFileContainer,cache,properties);
			}

		}
	}
}