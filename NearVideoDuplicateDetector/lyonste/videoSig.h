#pragma once
#include "fileManagement.h"
#include <boost/serialization/split_member.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include "feature2D/featureDescription/featureDescription.h"
#include <condition_variable>
#include <mutex>

//#define PRINT_COPY_VIDEO_METADATA


namespace lyonste
{
	namespace videoSig
	{

		class VideoMetaData
		{
		private:
			friend struct std::hash<VideoMetaData>;
			friend class boost::serialization::access;

			fileManagement::FileInfo vidFile;

			size_t cols;
			
			size_t rows;
			
			double numFrames;
			
			double fps;
			
			double duration;
			
			size_t hash;

			inline static size_t computeHash(const VideoMetaData* metaData) noexcept
			{
				return std::hash<fileManagement::FileInfo>()(metaData->vidFile)
					+31*(metaData->cols
						 +31*(metaData->rows
							  +31*(std::hash<double>()(metaData->numFrames)
								   +31*(std::hash<double>()(metaData->fps)
										+31*(std::hash<double>()(metaData->duration))))));
			}

			template<class Archive>
			constexpr void save(Archive& archive,const unsigned int version) const
			{
				archive & vidFile;
				archive & cols;
				archive & rows;
				archive & numFrames;
				archive & fps;
				archive & duration;
			}

			template<class Archive>
			constexpr void load(Archive& archive,const unsigned int version)
			{
				archive & vidFile;
				archive & cols;
				archive & rows;
				archive & numFrames;
				archive & fps;
				archive & duration;
				hash=computeHash(this);
			}

			template<class Archive>
			constexpr void serialize(Archive &archive,const unsigned int version)
			{
				boost::serialization::split_member(archive,*this,version);
			}

		public:

			 VideoMetaData(const VideoMetaData& that) noexcept
				:vidFile(that.vidFile)
				,cols(that.cols)
				,rows(that.rows)
				,numFrames(that.numFrames)
				,fps(that.fps)
				,duration(that.duration)
				,hash(that.hash)
			{
#ifdef _DEBUG
#ifdef PRINT_COPY_VIDEO_METADATA
				 std::cout<<"VideoMetaData copy constructor"<<std::endl;
#endif
#endif
			}



			 VideoMetaData(VideoMetaData&& that) noexcept
				 :vidFile(std::move(that.vidFile))
				 ,cols(that.cols)
				 ,rows(that.rows)
				 ,numFrames(that.numFrames)
				 ,fps(that.fps)
				 ,duration(that.duration)
				 ,hash(that.hash)
			 {

			 }

			 VideoMetaData& operator=(const VideoMetaData& that) noexcept
			 {
				 if(this!=&that)
				 {
					 vidFile=that.vidFile;
					 cols=that.cols;
					 rows=that.rows;
					 numFrames=that.numFrames;
					 fps=that.fps;
					 duration=that.duration;
					 hash=that.hash;
#ifdef _DEBUG
#ifdef PRINT_COPY_VIDEO_METADATA
					 std::cout<<"VideoMetaData assignment operator"<<std::endl;
#endif
#endif
				 }
				 return *this;
			 }
			 VideoMetaData& operator=(VideoMetaData&& that) noexcept
			 {
				 if(this!=&that)
				 {
					 vidFile=std::move(that.vidFile);
					 cols=that.cols;
					 rows=that.rows;
					 numFrames=that.numFrames;
					 fps=that.fps;
					 duration=that.duration;
					 hash=that.hash;
				 }
				 return *this;
			 }
			
			constexpr const fileManagement::FileInfo& getVidFile() const noexcept
			{
				return vidFile;
			}

			inline void updateVidFileLocation(const boost::filesystem::path& path) noexcept
			{
				vidFile.updatePath(path);
			}

			constexpr size_t getCols() const noexcept
			{
				return cols;
			}

			constexpr size_t getRows() const noexcept
			{
				return rows;
			}

			constexpr double getNumFrames() const noexcept
			{
				return numFrames;
			}

			constexpr double getFPS() const noexcept
			{
				return fps;
			}

			constexpr double getDuration() const noexcept
			{
				return duration;
			}

			inline VideoMetaData() noexcept
			{}

			inline VideoMetaData(const fileManagement::FileInfo& vidFile,size_t cols,size_t rows,double numFrames,double fps,double duration) noexcept
				:vidFile(vidFile),
				cols(cols),
				rows(rows),
				numFrames(numFrames),
				fps(fps),
				duration(duration),
				hash(computeHash(this))
			{}

			constexpr bool operator==(const VideoMetaData& that) const noexcept
			{
				return this==&that||(vidFile==that.vidFile && cols==that.cols && rows==that.rows && numFrames==that.numFrames && fps==that.fps && duration==that.duration);
			}

			constexpr bool operator!=(const VideoMetaData& that) const noexcept
			{
				return !(*this==that);
			}

			constexpr bool operator<(const VideoMetaData& that) const noexcept
			{
				return duration<that.duration||(duration==that.duration&&vidFile<that.vidFile);
			}

			constexpr bool operator>(const VideoMetaData& that) const noexcept
			{
				return that<*this;
			}

			constexpr bool operator<=(const VideoMetaData& that) const noexcept
			{
				return !(that<*this);
			}

			constexpr bool operator>=(const VideoMetaData& that) const noexcept
			{
				return !(*this<that);
			}
		};

		template<feature2D::featureDescription::DescriptorType descType>
		using DescriptorMat=typename feature2D::featureDescription::DescTypeInfo<descType>::DescriptorMat;

		template<feature2D::featureDescription::DescriptorType descType>
		using KeyFramePair=std::pair<double,DescriptorMat<descType>>;

		template<feature2D::featureDescription::DescriptorType descType>
		using KeyFrameItr=typename std::vector<KeyFramePair<descType>>::const_iterator;

		template<feature2D::featureDescription::DescriptorType descType>
		class VideoSigCache
		{
		protected:

			struct VideoSig : public std::vector<KeyFramePair<descType>>
			{
				VideoSigCache<descType>& cache;

				std::atomic<size_t> holdCount;

				VideoSig* next;

				VideoSig* prev;

				size_t numKeyFrames;

				fileManagement::FileInfo keyFramesFile;

				template<class Archive>
				constexpr void serialize(Archive &archive,const unsigned int version)
				{
					archive & numKeyFrames;
					archive & keyFramesFile;
				}

				inline void clear() noexcept
				{
					if(!std::vector<KeyFramePair<descType>>::empty())
					{
						std::vector<KeyFramePair<descType>>::clear();
						cache.currentMemoryFootprint-=keyFramesFile.getFileSize();
						--cache.currentNumSigs;
					}
				}

				constexpr VideoSig(VideoSigCache<descType>& cache) noexcept
					:std::vector<KeyFramePair<descType>>()
				    ,cache(cache)
					,holdCount(0)
					,next(NULL)
					,prev(NULL)
					,numKeyFrames(0)
					,keyFramesFile()
				{

				}

				constexpr VideoSig(VideoSigCache<descType>& cache,size_t numKeyFrames,fileManagement::FileInfo&& keyFramesFile) noexcept
					:std::vector<KeyFramePair<descType>>()
					,cache(cache)
					,holdCount(0)
					,next(NULL)
					,prev(NULL)
					,numKeyFrames(numKeyFrames)
					,keyFramesFile(std::move(keyFramesFile))
				{

				}

				constexpr VideoSig(const VideoSig& that) noexcept
					:std::vector<KeyFramePair<descType>>(that)
					,cache(that.cache)
					,holdCount(that.holdCount.load())
					,next(that.next)
					,prev(that.prev)
					,numKeyFrames(that.numKeyFrames)
					,keyFramesFile(that.keyFramesFile)
				{}

				constexpr VideoSig(VideoSig&& that) noexcept
					:std::vector<KeyFramePair<descType>>(std::move(that))
					,cache(that.cache)
					,holdCount(that.holdCount.load())
					,next(that.next)
					,prev(that.prev)
					,numKeyFrames(that.numKeyFrames)
					,keyFramesFile(std::move(that.keyFramesFile))
				{}

				constexpr VideoSig& operator=(const VideoSig& that) noexcept
				{
					if(this!=&that)
					{
						std::vector<KeyFramePair<descType>>::operator=(that);
						cache=that.cache;
						holdCount=that.holdCount.load();
						next=that.next;
						prev=that.prev;
						numKeyFrames=that.numKeyFrames;
						keyFramesFile=that.keyFramesFile;
					}
					return *this;
				}

				constexpr VideoSig& operator=(VideoSig&& that) noexcept
				{
					if(this!=&that)
					{
						std::vector<KeyFramePair<descType>>::operator=(std::move(that));
						cache=that.cache;
						holdCount=that.holdCount.load();
						next=that.next;
						prev=that.prev;
						numKeyFrames=that.numKeyFrames;
						keyFramesFile=std::move(that.keyFramesFile);
					}
					return *this;
				}
			
				void emplaceKeyFrames(std::vector<KeyFramePair<descType>>&& keyFrames) noexcept
				{
					std::vector<KeyFramePair<descType>>::operator=(std::move(keyFrames));
				}
			};

			friend class boost::serialization::access;

			size_t maxMemoryFootprint;

			size_t maxNumSigs;

			size_t currentMemoryFootprint;

			size_t currentNumSigs;

			std::condition_variable cv;

			std::mutex mut;

			VideoSig* head;

			VideoSig* tail;

			std::unordered_map<VideoMetaData,VideoSig> metaDatasToSigs;

			std::chrono::system_clock::time_point lastWriteTime;

			std::chrono::milliseconds writeInterval;

			boost::filesystem::path cacheFile;

			template<class Archive>
			constexpr void save(Archive& archive,const unsigned int version) const
			{
				size_t numSigs=metaDatasToSigs.size();
				archive & numSigs;
				for(const auto& pair:metaDatasToSigs)
				{
					archive & pair.first;
					archive & pair.second;
				}
			}

			template<class Archive>
			constexpr void load(Archive& archive,const unsigned int version)
			{
				metaDatasToSigs.clear();
				size_t numSigs;
				archive & numSigs;
				metaDatasToSigs.reserve(numSigs);
				for(size_t i=0;i!=numSigs;++i)
				{
					VideoMetaData metaData;
					archive & metaData;
					VideoSig sig(*this);
					archive & sig;
					metaDatasToSigs.emplace_hint(metaDatasToSigs.end(),std::move(metaData),std::move(sig));
				}
			}

			template<class Archive>
			constexpr void serialize(Archive &archive,const unsigned int version)
			{
				boost::serialization::split_member(archive,*this,version);
			}

			static constexpr void detach(VideoSig* sig) noexcept
			{
				VideoSig* sigNext=sig->next;
				VideoSig* sigPrev=sig->prev;
				if(sigNext)
				{
					sigNext->prev=sigPrev;
				}
				if(sigPrev)
				{
					sigPrev->next=sigNext;
				}
			}

			constexpr void moveSigToSecondPosition(VideoSig* sig) noexcept
			{
				if(head->next!=sig)
				{
					if(sig==tail)
					{
						tail=sig->prev;
						tail->next=NULL;
					}
					else
					{
						detach(sig);
					}
					VideoSig* headNext=head->next;
					sig->next=headNext;
					sig->prev=head;
					if(headNext)
					{
						headNext->prev=sig;
					}
					head->next=sig;
				}
			}

			constexpr void moveSigFromTail(VideoSig* sig1,VideoSig* sig2) noexcept
			{
				if(sig1->prev==sig2)
				{
					tail=sig2->prev;
					tail->next=NULL;
				}
				else
				{
					tail=sig1->prev;
					tail->next=NULL;
					detach(sig2);
					sig2->next=sig1;
					sig1->prev=sig2;
				}
				sig1->next=head;
				sig2->prev=NULL;
				head->prev=sig1;
				head=sig2;
			}

			constexpr void moveLinkedSigsToHead(VideoSig* sig1,VideoSig* sig2) noexcept
			{
				VideoSig* sig2Next=sig2->next;
				if(sig2Next)
				{
					sig2Next->prev=sig1->prev;
				}
				VideoSig* sig1Prev=sig1->prev;
				if(sig1Prev)
				{
					sig1Prev->next=sig2Next;
				}
				sig2->next=head;
				sig1->prev=NULL;
				head->prev=sig2;
				head=sig1;
			}

			constexpr void deflateAndRemoveRearSig(std::unique_lock<std::mutex>& lock) noexcept
			{
				VideoSig* tmp=tail;
				for(;;)
				{
					if(tmp->holdCount==0)
					{
						tmp->deflate();
						if(tmp==tail)
						{
							if(tmp==head)
							{
								tail=head=NULL;
							}
							else
							{
								tail=tail->prev;
								tail->next=NULL;
							}
						}
						else
						{
							if(tmp==head)
							{
								head=head->next;
								head->prev=NULL;
							}
							else
							{
								detach(tmp);
							}
						}
						tmp->next=tmp->prev=NULL;
						return;
					}
					else
					{
						if(tmp==head)
						{
							cv.wait_for(lock,std::chrono::milliseconds(5000));
							tmp=tail;
							if(tmp==NULL)
							{
								break;
							}
						}
						else
						{
							tmp=tmp->prev;
						}
					}
				}
			}

			constexpr void readFromFile()
			{
				if(boost::filesystem::exists(cacheFile))
				{
					boost::filesystem::ifstream cacheIn(cacheFile,std::ios::binary|std::ios::in);
					if(cacheIn.is_open())
					{
						boost::archive::binary_iarchive cacheArchive(cacheIn);
						cacheArchive & *this;
						cacheIn.close();
					}
				}
			}

			

		public:

			class VideoSigPtr
			{
			private:
				friend class VideoSigCache<descType>;

				const VideoMetaData* metaData;

				VideoSig* sig;

				inline VideoSigPtr(const VideoMetaData* metaData,VideoSig* sig):
					metaData(metaData),
					sig(sig)
				{
					++sig->holdCount;
					size_t numKeyFrames=sig->numKeyFrames;
					if(numKeyFrames&&sig->empty())
					{
						fileManagement::FileInfo& keyFramesFile=sig->keyFramesFile;
						boost::filesystem::ifstream sigIn(keyFramesFile.getPath(),std::ios::binary|std::ios::in);
						if(sigIn.is_open())
						{
							std::vector<KeyFramePair<descType>> keyFrames;
							{
								boost::archive::binary_iarchive sigArchive(sigIn);
								for(;numKeyFrames--;)
								{
									double timeIndex;
									sigArchive & timeIndex;
									DescriptorMat<descType> keyFrame;
									sigArchive & keyFrame;
									keyFrames.emplace_back(timeIndex,keyFrame);
								}
							}
							sigIn.close();
							sig->emplaceKeyFrames(std::move(keyFrames));
						}
						VideoSigCache<descType>& cache=sig->cache;
						cache.currentMemoryFootprint+=keyFramesFile.getFileSize();
						++cache.currentNumSigs;
					}
				}

				constexpr void release() noexcept
				{
					if(sig)
					{
						--(sig->holdCount);
						sig->cache.cv.notify_one();
					}
				}

			public:

				constexpr VideoSigPtr operator=(const VideoSigPtr& that) noexcept
				{
					if(sig!=that.sig)
					{
						release();
						metaData=that.metaData;
						sig=that.sig;
						++sig.holdCount;
					}
					return *this;
				}

				constexpr VideoSigPtr operator=(VideoSigPtr&& that) noexcept
				{
					if(sig!=that.sig)
					{
						release();
						metaData=that.metaData;
						sig=that.sig;
						that.sig=NULL;
					}
					return *this;
				}

				constexpr VideoSigPtr(const VideoSigPtr& that) noexcept
					:metaData(that.metaData),
					sig(that.sig)
				{
					++sig.holdCount;
				}

				constexpr VideoSigPtr(VideoSigPtr&& that) noexcept
					:metaData(that.metaData),
					sig(that.sig)
				{
					that.sig=NULL;
				}

				inline ~VideoSigPtr() noexcept
				{
					release();
				}

				inline KeyFrameItr<descType> begin() const noexcept
				{
					return sig->cbegin();
				}

				inline KeyFrameItr<descType> end() const noexcept
				{
					return sig->cend();
				}

				constexpr size_t size() const noexcept
				{
					return sig->numKeyFrames;
				}

				constexpr bool empty() const noexcept
				{
					return sig->numKeyFrames==0;
				}

				constexpr double getDuration() const noexcept
				{
					return metaData->getDuration();
				}

				static inline KeyFrameItr<descType>& iterateKeyFrameItr(KeyFrameItr<descType>& lo,double timeIndex,const KeyFrameItr<descType>& hi) noexcept
				{
					double diff1=timeIndex-lo->first;
					if(diff1>0)
					{
						for(;;)
						{
							if(++lo==hi)
							{
								return --lo;
							}
							double diff2=diff1;
							if((diff1=timeIndex-lo->first)<0)
							{
								if(-diff1<diff2)
								{
									break;
								}
								return --lo;
							}
						}
					}
					return lo;
				}
			};

			constexpr void writeToFile(const boost::filesystem::path& file) const
			{
				boost::filesystem::create_directories(file.parent_path());
				boost::filesystem::path tmp=file.string()+".tmp";
				boost::filesystem::ofstream cacheOut(tmp,std::ios::binary|std::ios::out);
				if(cacheOut.is_open())
				{
					{
						boost::archive::binary_oarchive cacheArchive(cacheOut);
						cacheArchive & *this;
					}
					cacheOut.close();
					boost::filesystem::rename(tmp,file);
					return;
				}
				CV_Error(1,"Unable to open "+tmp.string()+" for writing");
			}
			constexpr void writeToFile() const
			{
				writeToFile(cacheFile);
			}

			constexpr const std::tuple<VideoMetaData,fileManagement::FileInfo,size_t> addSig(VideoMetaData&& metaData,std::vector<KeyFramePair<descType>>&& keyFrames)
			{
				const boost::filesystem::path kfFile=metaData.getVidFile().resolveToDirectory<true>(cacheFile.parent_path(),L".sig");
				boost::filesystem::create_directories(kfFile.parent_path());
				const boost::filesystem::path tmpFile=(kfFile.wstring())+L".tmp";
				boost::filesystem::ofstream sigOut(tmpFile,std::ios::binary|std::ios::out);
				if(sigOut.is_open())
				{
					{
						boost::archive::binary_oarchive sigArchive(sigOut);
						for(const KeyFramePair<descType>& framePair:keyFrames)
						{
							sigArchive & framePair.first;
							sigArchive & framePair.second;
						}
					}
					sigOut.close();
					boost::filesystem::rename(tmpFile,kfFile);

					VideoSig sig(*this,keyFrames.size(),fileManagement::FileInfo(kfFile));
					keyFrames.clear();
					std::lock_guard<std::mutex> lock(mut);

					auto itr=metaDatasToSigs.emplace_hint(metaDatasToSigs.end(),metaData,std::move(sig));
					std::chrono::system_clock::time_point now=std::chrono::system_clock::now();
					if(now-lastWriteTime>=writeInterval)
					{
						writeToFile();
						lastWriteTime=now;
					}
					return std::make_tuple(itr->first,itr->second.keyFramesFile,itr->second.numKeyFrames);
				}
				CV_Error(1,"Unable to open keyframe file "+kfFile.string()+" for writing");
				//to satisfy compiler warning
				throw std::exception("unspecified error");
			}

			constexpr const std::pair<VideoSigPtr,VideoSigPtr> getMatchPairFromMetaDatas(const std::pair<VideoMetaData,VideoMetaData>& pair)
			{
				CV_DbgAssert(pair.first!=pair.second);
				std::unique_lock<std::mutex> lock(mut);
				const VideoMetaData& metaData1=pair.first;
				const VideoMetaData& metaData2=pair.second;
				VideoSig* sig1=&metaDatasToSigs.at(metaData1);
				VideoSig* sig2=&metaDatasToSigs.at(metaData2);
				std::pair<VideoSigPtr,VideoSigPtr> matchPair(VideoSigPtr(&metaData1,sig1),VideoSigPtr(&metaData2,sig2));
				if(head)
				{
					if(sig1==head)
					{
						moveSigToSecondPosition(sig2);
					}
					else if(sig2==head)
					{
						moveSigToSecondPosition(sig1);
					}
					else
					{
						if(sig1==tail)
						{
							moveSigFromTail(sig1,sig2);
						}
						else if(sig2==tail)
						{
							moveSigFromTail(sig2,sig1);
						}
						else
						{
							if(sig1->next==sig2)
							{
								moveLinkedSigsToHead(sig1,sig2);
							}
							else if(sig1->prev==sig2)
							{
								moveLinkedSigsToHead(sig2,sig1);
							}
							else
							{
								detach(sig2);
								detach(sig1);
								sig1->next=head;
								sig1->prev=sig2;
								sig2->prev=NULL;
								sig2->next=sig1;
								head->prev=sig1;
								head=sig2;
							}
						}
					}
				}
				else
				{
					head=sig2;
					tail=sig1;
					sig2->next=sig1;
					sig2->prev=NULL;
					sig1->prev=sig2;
					sig1->next=NULL;
				}
				if(currentNumSigs>maxNumSigs||currentMemoryFootprint>maxMemoryFootprint)
				{
					for(;;)
					{
						VideoSig* tmp=tail;
						for(;;)
						{
							if(tmp->holdCount==0)
							{
								tmp->clear();
								if(tmp==tail)
								{
									if(tmp==head)
									{
										tail=head=NULL;
									}
									else
									{
										tail=tail->prev;
										tail->next=NULL;
									}
								}
								else
								{
									if(tmp==head)
									{
										head=head->next;
										head->prev=NULL;
									}
									else
									{
										detach(tmp);
									}
								}
								tmp->next=tmp->prev=NULL;
								break;
							}
							else
							{
								if(tmp==head)
								{
									cv.wait_for(lock,std::chrono::milliseconds(5000));
									tmp=tail;
									if(tmp==NULL)
									{
										break;
									}
								}
								else
								{
									tmp=tmp->prev;
								}
							}
						}
						if((currentNumSigs<=maxNumSigs&&currentMemoryFootprint<maxMemoryFootprint)||currentNumSigs<=2||currentMemoryFootprint==0)
						{
							break;
						}
					}
				}
				return matchPair;
			}

			constexpr size_t getKeyFrameFileSize(const std::pair<VideoMetaData,VideoMetaData>& pair) const
			{
				return metaDatasToSigs.at(pair.first).keyFramesFile.getFileSize()+metaDatasToSigs.at(pair.second).keyFramesFile.getFileSize();
			}

			constexpr VideoSigCache(const VideoSigCache<descType>& that) noexcept
				:maxMemoryFootprint(that.maxMemoryFootprint),
				maxNumSigs(that.maxNumSigs),
				currentMemoryFootprint(that.currentMemoryFootprint),
				currentNumSigs(that.currentNumSigs),
				cv(),
				mut(),
				head(that.head),
				tail(that.tail),
				sigs(that.sigs)
			{}

			constexpr VideoSigCache(VideoSigCache<descType>&& that) noexcept
				:maxMemoryFootprint(that.maxMemoryFootprint),
				maxNumSigs(that.maxNumSigs),
				currentMemoryFootprint(that.currentMemoryFootprint),
				currentNumSigs(that.currentNumSigs),
				cv(),
				mut(),
				head(that.head),
				tail(that.tail),
				sigs(std::move(that.sigs))
			{}

			constexpr VideoSigCache<descType>& operator=(const VideoSigCache<descType>& that) noexcept
			{
				if(this!=&that)
				{
					maxMemoryFootprint=that.maxMemoryFootprint;
					maxNumSigs=that.maxNumSigs;
					currentMemoryFootprint=that.currentMemoryFootprint;
					currentNumSigs=that.currentNumSigs;
					head=that.head;
					tail=fhat.tail;
					sigs=that.sigs;
				}
				return *this;
			}

			constexpr VideoSigCache<descType>& operator=(VideoSigCache<descType>&& that) noexcept
			{
				if(this!=&that)
				{
					maxMemoryFootprint=that.maxMemoryFootprint;
					maxNumSigs=that.maxNumSigs;
					currentMemoryFootprint=that.currentMemoryFootprint;
					currentNumSigs=that.currentNumSigs;
					head=that.head;
					tail=fhat.tail;
					sigs=std::move(that.sigs);
				}
				return *this;
			}

			constexpr VideoSigCache(const boost::property_tree::ptree& properties)
				:maxMemoryFootprint(properties.get<size_t>("maxMemoryFootprint",std::numeric_limits<size_t>::max())),
				maxNumSigs(std::numeric_limits<size_t>::max()),
				currentMemoryFootprint(0),
				currentNumSigs(0),
				cv(),
				mut(),
				head(NULL),
				tail(NULL),
				lastWriteTime(std::chrono::system_clock::now()),
				writeInterval(properties.get<size_t>("writeIntervalMillis",0)),
				cacheFile(properties.get<std::string>("cachePath"))
			{
				readFromFile();
			}

			constexpr void setMaxNumSigs(const size_t maxNumSigs=std::numeric_limits<size_t>::max()) noexcept
			{
				this->maxNumSigs=maxNumSigs;
			}

			template<class MatchCandidatePred,class CandidateContainer,class MetaDataSort=std::less<VideoMetaData>>
			constexpr CandidateContainer getMatchCandidates(const MatchCandidatePred& matchCandidatePred,size_t numThreads,const MetaDataSort metaDataSort=std::less<VideoMetaData>()) noexcept
			{
				CandidateContainer matchCandidates;
				std::insert_iterator<CandidateContainer> inserterItr=std::inserter(matchCandidates,matchCandidates.end());
				if(metaDatasToSigs.size()>1)
				{
					const auto metaDataEnd=metaDatasToSigs.end();
					size_t maxRun=0;
					for(auto itr1=metaDatasToSigs.begin();itr1!=metaDataEnd;++itr1)
					{
						const VideoMetaData& metaData1=itr1->first;
						size_t currentRun=0;
						for(auto itr2=itr1;++itr2!=metaDataEnd;)
						{
							const VideoMetaData& metaData2=itr2->first;
							if(matchCandidatePred(metaData1,metaData2))
							{
								++currentRun;
								if(metaDataSort(metaData1,metaData2))
								{
									inserterItr=std::make_pair(metaData1,metaData2);
								}
								else
								{
									inserterItr=std::make_pair(metaData2,metaData1);
								}
							}
						}
						if(currentRun>maxRun)
						{
							maxRun=currentRun;
						}
					}
					this->head=NULL;
					this->tail=NULL;
					this->currentMemoryFootprint=0;
					this->currentNumSigs=0;
					this->maxNumSigs=std::max<size_t>(numThreads<<1,maxRun+1);
				}
				return matchCandidates;
			}

			template<class VidFileContainer>
			constexpr void pruneCache(VidFileContainer& vidFileContainer) noexcept
			{
				////std::unordered_map<VideoMetaData,VideoSig> tmp;
				//for(const auto& vidFile:vidFileContainer)
				//{
				//	//Update the video file locations
				//	//TODO: implement a constant time lookup with video files as keys
				//	for(auto itr=metaDatasToSigs.begin();itr!=metaDatasToSigs.end();++itr)
				//	{
				//		const fileManagement::FileInfo& cacheVidFile=itr->first.getVidFile();
				//		if(vidFile==cacheVidFile)
				//		{
				//			if(vidFile.getPath()!=cacheVidFile.getPath())
				//			{
				//				const VideoSig& sig=itr->second;
				//				VideoMetaData metaData=itr->first;
				//				metaDatasToSigs.erase(itr);
				//				metaData.updateVidFileLocation(vidFile.getPath());
				//				metaDatasToSigs.emplace(std::move(metaData),std::move(sig));
				//			}
				//			break;
				//		}
				//	}
				//}

				std::unordered_set<fileManagement::FileInfo> kfFiles;
				fileManagement::FileGatherer<fileManagement::FileInfo> sigGatherer(cacheFile.parent_path(),std::numeric_limits<size_t>::max(),false,std::vector<boost::regex>{boost::regex("(.*)\\.sig")},std::vector<boost::regex>{boost::regex(cacheFile.string())});
				sigGatherer.gatherFiles(std::inserter(kfFiles,kfFiles.end()));
				for(auto cacheItr=metaDatasToSigs.begin();cacheItr!=metaDatasToSigs.end();)
				{
					VideoSig& sig=cacheItr->second;
					const fileManagement::FileInfo& cacheKFFile=sig.keyFramesFile;
					const auto kfFileEnd=kfFiles.end();
					const auto kfFileItr=std::find(kfFiles.begin(),kfFileEnd,cacheKFFile);
					if(kfFileItr==kfFileEnd)
					{
						boost::filesystem::path keyFrameFilePath=cacheKFFile.getPath();
						if(boost::filesystem::exists(keyFrameFilePath))
						{
							boost::filesystem::remove(keyFrameFilePath);
						}
						cacheItr=metaDatasToSigs.erase(cacheItr);
					}
					else
					{
						const auto vidFileEnd=vidFileContainer.end();
						const VideoMetaData& cacheMetaData=cacheItr->first;
						const auto vidFileItr=std::find(vidFileContainer.begin(),vidFileEnd,cacheMetaData.getVidFile());
						const fileManagement::FileInfo& foundKFFile=*kfFileItr;
						if(vidFileItr==vidFileEnd||foundKFFile.getFileSize()!=cacheKFFile.getFileSize())
						{
							boost::filesystem::path keyFrameFilePath=foundKFFile.getPath();
							if(boost::filesystem::exists(keyFrameFilePath))
							{
								boost::filesystem::remove(keyFrameFilePath);
							}
							cacheItr=metaDatasToSigs.erase(cacheItr);
						}
						else
						{
							vidFileContainer.erase(vidFileItr);
							sig.keyFramesFile=foundKFFile;
							++cacheItr;
						}
						kfFiles.erase(kfFileItr);
					}
				}
				for(const fileManagement::FileInfo& kfFile:kfFiles)
				{
					const boost::filesystem::path& kfFilePath=kfFile.getPath();
					if(boost::filesystem::exists(kfFilePath))
					{
						boost::filesystem::remove(kfFilePath);
					}
				}
			}

		};

		template<feature2D::featureDescription::DescriptorType descType>
		using VideoSigPtr=typename VideoSigCache<descType>::VideoSigPtr;
	}
}
namespace std
{
	template<>
	struct hash<lyonste::videoSig::VideoMetaData>
	{
		constexpr size_t operator()(const lyonste::videoSig::VideoMetaData& metaData) const noexcept
		{
			return metaData.hash;
		}
	};
}