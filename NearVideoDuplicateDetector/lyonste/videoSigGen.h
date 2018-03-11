#pragma once

#include "videoSig.h"
#include <mutex>

#include "feature2D/featureDetection/featureDetection.h"
#include <boost/exception/diagnostic_information.hpp>

namespace lyonste
{
	namespace videoSig
	{
		namespace videoSigGen
		{
			template<class VidFileContainer>
			class SigGenSessionImpl
			{
			protected:
				size_t numThreads;
				SigGenFunction* sigGenFunction;
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
							sigGenFunction->generateSig(*vidFile);
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

				constexpr SigGenSessionImpl(VidFileContainer& vidFileContainer,VideoSigCache* cache,const boost::property_tree::ptree& properties):
					numThreads(getNumThreadsProperty(properties)),
					sigGenFunction(cache->getSigGenFunction(properties)),
					vidFileItr(vidFileContainer.cbegin()),
					vidFileEnd(vidFileContainer.cend()),
					vidFileGetMut()
				{}

				size_t getNumSigsInCache() const noexcept
				{
					return sigGenFunction->getNumSigsInCache();
				}

				constexpr void generateSigs()
				{
					std::vector<std::thread> threads;
					size_t numThreads=this->numThreads;
					threads.reserve(--numThreads);
					for(;numThreads--;)
					{
						threads.emplace_back(&SigGenSessionImpl<VidFileContainer>::sigGenThread,this);
					}
					sigGenThread();
					for(auto& thread:threads)
					{
						thread.join();
					}
					sigGenFunction->writeCacheToFile();
					//std::cout << "Finished generating signatures. Total number of signatures in cache = " << sigGenFunction->getNumSigsInCache() << std::endl;
				}
				
				virtual ~SigGenSessionImpl()
				{
					delete sigGenFunction;
				}
			};

			template<class VidFileContainer>
			class VerboseSigGenSessionImpl: public SigGenSessionImpl<VidFileContainer>
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
							reportSuccess(startTime,sigGenFunction->generateSig(*vidFile));
						}
						catch(...)
						{
							reportFailure(startTime,*vidFile);
						}
					}
				}

			public:

				constexpr VerboseSigGenSessionImpl(VidFileContainer& vidFileContainer, VideoSigCache* cache,const boost::property_tree::ptree& properties)
					:SigGenSessionImpl<VidFileContainer>(vidFileContainer,cache,properties),
					reportMut(),
					finishIndex(0),
					totalNumVids(vidFileContainer.size()),
					consumedFileSize(0),
					totalFileSize(countFileSize(vidFileContainer)),
					sessionStartTime(std::chrono::system_clock::now())
				{

				}
			};




			template<class VidFileContainer>
			constexpr SigGenSessionImpl<VidFileContainer>* getSigGenSession(VidFileContainer& vidFileContainer,VideoSigCache* cache,const boost::property_tree::ptree& properties)
			{
				if(properties.get<boolean>("verbose",true))
				{
					return new VerboseSigGenSessionImpl<VidFileContainer>(vidFileContainer,cache,properties);
				}
				return new SigGenSessionImpl<VidFileContainer>(vidFileContainer,cache,properties);
			}

		}
	}
}