#include "videoSig.h"
#include "feature2D\featureDetection\featureDetection.h"
#include "feature2D/featureDescription/featureDescription.h"
#include <opencv2\videoio.hpp>
#include <iostream>
#include <condition_variable>
#include <mutex>
#include <boost/serialization/split_member.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

namespace lyonste
{
	namespace videoSig
	{
		template<feature2D::featureDescription::DescriptorType descType>
		using DescriptorMat = typename feature2D::featureDescription::DescTypeInfo<descType>::DescriptorMat;

		template<feature2D::featureDescription::DescriptorType descType>
		using KeyFramePair = std::pair<double, DescriptorMat<descType>>;

		template<feature2D::featureDescription::DescriptorType descType>
		using KeyFrameItr = typename std::vector<KeyFramePair<descType>>::const_iterator;


		template<feature2D::featureDescription::DescriptorType descType>
		class VideoSigCacheImpl : public VideoSigCache
		{
		protected:

			struct VideoSigImpl : public std::vector<KeyFramePair<descType>>
			{
				VideoSigCacheImpl* cache;

				std::atomic<size_t> holdCount;

				VideoSigImpl* next;

				VideoSigImpl* prev;

				size_t numKeyFrames;

				fileManagement::FileInfo keyFramesFile;

				template<class Archive>
				constexpr void serialize(Archive &archive, const unsigned int version)
				{
					archive & numKeyFrames;
					archive & keyFramesFile;
				}


				void clear() noexcept
				{
					if (!std::vector<KeyFramePair<descType>>::empty())
					{
						std::vector<KeyFramePair<descType>>::clear();
						cache->currentMemoryFootprint -= keyFramesFile.getFileSize();
						--cache->currentNumSigs;
					}
				}

				VideoSigImpl(VideoSigCacheImpl<descType>* cache) noexcept
					: std::vector<KeyFramePair<descType>>()
					, cache(cache)
					, holdCount(0)
					, next(NULL)
					, prev(NULL)
					, numKeyFrames(0)
					, keyFramesFile()
				{

				}

				VideoSigImpl(VideoSigCacheImpl<descType>* cache, size_t numKeyFrames, fileManagement::FileInfo&& keyFramesFile) noexcept
					: std::vector<KeyFramePair<descType>>()
					, cache(cache)
					, holdCount(0)
					, next(NULL)
					, prev(NULL)
					, numKeyFrames(numKeyFrames)
					, keyFramesFile(std::move(keyFramesFile))
				{

				}

				VideoSigImpl(const VideoSigImpl& that) noexcept
					:std::vector<KeyFramePair<descType>>(that)
					, cache(that.cache)
					, holdCount(that.holdCount.load())
					, next(that.next)
					, prev(that.prev)
					, numKeyFrames(that.numKeyFrames)
					, keyFramesFile(that.keyFramesFile)
				{}

				constexpr VideoSigImpl(VideoSigImpl&& that) noexcept
					:std::vector<KeyFramePair<descType>>(std::move(that))
					, cache(that.cache)
					, holdCount(that.holdCount.load())
					, next(that.next)
					, prev(that.prev)
					, numKeyFrames(that.numKeyFrames)
					, keyFramesFile(std::move(that.keyFramesFile))
				{}

				constexpr VideoSigImpl& operator=(const VideoSigImpl& that) noexcept
				{
					if (this != &that)
					{
						std::vector<KeyFramePair<descType>>::operator=(that);
						cache = that.cache;
						holdCount = that.holdCount.load();
						next = that.next;
						prev = that.prev;
						numKeyFrames = that.numKeyFrames;
						keyFramesFile = that.keyFramesFile;
					}
					return *this;
				}



				constexpr VideoSigImpl& operator=(VideoSigImpl&& that) noexcept
				{
					if (this != &that)
					{
						std::vector<KeyFramePair<descType>>::operator=(std::move(that));
						cache = that.cache;
						holdCount = that.holdCount.load();
						next = that.next;
						prev = that.prev;
						numKeyFrames = that.numKeyFrames;
						keyFramesFile = std::move(that.keyFramesFile);
					}
					return *this;
				}

				void emplaceKeyFrames(std::vector<KeyFramePair<descType>>&& keyFrames) noexcept
				{
					std::vector<KeyFramePair<descType>>::operator=(std::move(keyFrames));
				}
			};

			size_t maxMemoryFootprint;

			size_t maxNumSigs;

			size_t currentMemoryFootprint;

			size_t currentNumSigs;

			std::condition_variable cv;

			std::mutex mut;

			std::chrono::system_clock::time_point lastWriteTime;

			std::chrono::milliseconds writeInterval;

			boost::filesystem::path cacheFile;

			VideoSigImpl* head;

			VideoSigImpl* tail;

			std::unordered_map<VideoMetaData, VideoSigImpl> metaDatasToSigs;

			friend class boost::serialization::access;

			template<class Archive>
			constexpr void save(Archive& archive, const unsigned int version) const
			{
				size_t numSigs = metaDatasToSigs.size();
				archive & numSigs;
				for (const auto& pair : metaDatasToSigs)
				{
					archive & pair.first;
					archive & pair.second;
				}
			}

			template<class Archive>
			constexpr void load(Archive& archive, const unsigned int version)
			{
				metaDatasToSigs.clear();
				size_t numSigs;
				archive & numSigs;
				metaDatasToSigs.reserve(numSigs);
				for (size_t i = 0; i != numSigs; ++i)
				{
					VideoMetaData metaData;
					archive & metaData;
					VideoSigImpl sig(this);
					archive & sig;
					metaDatasToSigs.emplace_hint(metaDatasToSigs.end(), std::move(metaData), std::move(sig));
				}
			}

			template<class Archive>
			constexpr void serialize(Archive &archive, const unsigned int version)
			{
				boost::serialization::split_member(archive, *this, version);
			}

			void readFromFile()
			{
				if (boost::filesystem::exists(cacheFile))
				{
					boost::filesystem::ifstream cacheIn(cacheFile, std::ios::binary | std::ios::in);
					if (cacheIn.is_open())
					{
						boost::archive::binary_iarchive cacheArchive(cacheIn);
						cacheArchive & *this;
						cacheIn.close();
					}
				}
			}

			static constexpr void detach(VideoSigImpl* sig) noexcept
			{
				VideoSigImpl* sigNext = sig->next;
				VideoSigImpl* sigPrev = sig->prev;
				if (sigNext)
				{
					sigNext->prev = sigPrev;
				}
				if (sigPrev)
				{
					sigPrev->next = sigNext;
				}
			}

			constexpr void moveSigToSecondPosition(VideoSigImpl* sig) noexcept
			{
				if (head->next != sig)
				{
					if (sig == tail)
					{
						tail = sig->prev;
						tail->next = NULL;
					}
					else
					{
						detach(sig);
					}
					VideoSigImpl* headNext = head->next;
					sig->next = headNext;
					sig->prev = head;
					if (headNext)
					{
						headNext->prev = sig;
					}
					head->next = sig;
				}
			}

			constexpr void moveSigFromTail(VideoSigImpl* sig1, VideoSigImpl* sig2) noexcept
			{
				if (sig1->prev == sig2)
				{
					tail = sig2->prev;
					tail->next = NULL;
				}
				else
				{
					tail = sig1->prev;
					tail->next = NULL;
					detach(sig2);
					sig2->next = sig1;
					sig1->prev = sig2;
				}
				sig1->next = head;
				sig2->prev = NULL;
				head->prev = sig1;
				head = sig2;
			}

			constexpr void moveLinkedSigsToHead(VideoSigImpl* sig1, VideoSigImpl* sig2) noexcept
			{
				VideoSigImpl* sig2Next = sig2->next;
				if (sig2Next)
				{
					sig2Next->prev = sig1->prev;
				}
				VideoSigImpl* sig1Prev = sig1->prev;
				if (sig1Prev)
				{
					sig1Prev->next = sig2Next;
				}
				sig2->next = head;
				sig1->prev = NULL;
				head->prev = sig2;
				head = sig1;
			}

			constexpr void deflateAndRemoveRearSig(std::unique_lock<std::mutex>& lock) noexcept
			{
				VideoSigImpl* tmp = tail;
				for (;;)
				{
					if (tmp->holdCount == 0)
					{
						tmp->clear();
						if (tmp == tail)
						{
							if (tmp == head)
							{
								tail = head = NULL;
							}
							else
							{
								tail = tail->prev;
								tail->next = NULL;
							}
						}
						else
						{
							if (tmp == head)
							{
								head = head->next;
								head->prev = NULL;
							}
							else
							{
								detach(tmp);
							}
						}
						tmp->next = tmp->prev = NULL;
						return;
					}
					else
					{
						if (tmp == head)
						{
							cv.wait_for(lock, std::chrono::milliseconds(5000));
							tmp = tail;
							if (tmp == NULL)
							{
								break;
							}
						}
						else
						{
							tmp = tmp->prev;
						}
					}
				}
			}

			class VideoSigPtrImpl
			{
			private:
				friend class VideoSigCacheImpl<descType>;

				const VideoMetaData* metaData;

				VideoSigImpl* sig;

				inline VideoSigPtrImpl(const VideoMetaData* metaData, VideoSigImpl* sig) :
					metaData(metaData),
					sig(sig)
				{
					++sig->holdCount;
					size_t numKeyFrames = sig->numKeyFrames;
					if (numKeyFrames&&sig->empty())
					{
						fileManagement::FileInfo& keyFramesFile = sig->keyFramesFile;
						boost::filesystem::ifstream sigIn(keyFramesFile.getPath(), std::ios::binary | std::ios::in);
						if (sigIn.is_open())
						{
							std::vector<KeyFramePair<descType>> keyFrames;
							{
								boost::archive::binary_iarchive sigArchive(sigIn);
								for (; numKeyFrames--;)
								{
									double timeIndex;
									sigArchive & timeIndex;
									DescriptorMat<descType> keyFrame;
									sigArchive & keyFrame;
									keyFrames.emplace_back(timeIndex, keyFrame);
								}
							}
							sigIn.close();
							sig->emplaceKeyFrames(std::move(keyFrames));
						}
						VideoSigCacheImpl* cache = sig->cache;
						cache->currentMemoryFootprint += keyFramesFile.getFileSize();
						++cache->currentNumSigs;
					}
				}

				constexpr void release() noexcept
				{
					if (sig)
					{
						--(sig->holdCount);
						sig->cache->cv.notify_one();
					}
				}

			public:

				constexpr VideoSigPtrImpl operator=(const VideoSigPtrImpl& that) noexcept
				{
					if (sig != that.sig)
					{
						release();
						metaData = that.metaData;
						sig = that.sig;
						++sig->holdCount;
					}
					return *this;
				}

				constexpr VideoSigPtrImpl operator=(VideoSigPtrImpl&& that) noexcept
				{
					if (sig != that.sig)
					{
						release();
						metaData = that.metaData;
						sig = that.sig;
						that.sig = NULL;
					}
					return *this;
				}

				constexpr VideoSigPtrImpl(const VideoSigPtrImpl& that) noexcept
					:metaData(that.metaData),
					sig(that.sig)
				{
					++sig->holdCount;
				}

				constexpr VideoSigPtrImpl(VideoSigPtrImpl&& that) noexcept
					:metaData(that.metaData),
					sig(that.sig)
				{
					that.sig = NULL;
				}

				inline ~VideoSigPtrImpl() noexcept
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
					return sig->numKeyFrames == 0;
				}

				constexpr double getDuration() const noexcept
				{
					return metaData->getDuration();
				}

				static inline KeyFrameItr<descType>& iterateKeyFrameItr(KeyFrameItr<descType>& lo, double timeIndex, const KeyFrameItr<descType>& hi) noexcept
				{
					double diff1 = timeIndex - lo->first;
					if (diff1>0)
					{
						for (;;)
						{
							if (++lo == hi)
							{
								return --lo;
							}
							double diff2 = diff1;
							if ((diff1 = timeIndex - lo->first)<0)
							{
								if (-diff1<diff2)
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

			class SigGenFunctionImpl : public videoSigGen::SigGenFunction
			{
			protected:
				VideoSigCacheImpl<descType>* cache;
				feature2D::featureDetection::KeyPointGeneratorFactory* kpGenFactory;
				feature2D::featureDescription::DescriptorGeneratorFactoryImpl<descType>* descGenFactory;
				double keyFrameInterval;
			public:
				SigGenFunctionImpl(VideoSigCacheImpl<descType>* cache, const boost::property_tree::ptree& properties) :
					cache(cache),
					kpGenFactory(feature2D::featureDetection::KeyPointGeneratorFactory::getKeyPointGeneratorFactory(properties.get_child("keyPointGeneratorProperties"))),
					descGenFactory(feature2D::featureDescription::DescriptorGeneratorFactoryImpl<descType>::getDescriptorGeneratorFactory(properties.get_child("descriptorGeneratorProperities"))),
					keyFrameInterval(properties.get<double>("keyFrameInterval", 2000.0))
				{}

				std::tuple<VideoMetaData, fileManagement::FileInfo, size_t> generateSig(const fileManagement::FileInfo& vidFile) const override
				{
					const std::string& fileName = vidFile.getFilePathStr();
					if (vidFile.pathExists())
					{
						cv::VideoCapture videoCapture(fileName);
						if (videoCapture.isOpened())
						{
							cv::Mat captureMat;
							if (videoCapture.read(captureMat))
							{
								size_t cols(captureMat.cols);
								size_t rows(captureMat.rows);
								if (cols && rows)
								{
									std::unique_ptr<feature2D::featureDetection::KeyPointGenerator> kpGen(kpGenFactory->getKeyPointGenerator(cols, rows));
									std::unique_ptr<feature2D::featureDescription::DescriptorGeneratorImpl<descType>> descGen(descGenFactory->getDescriptorGenerator(cols, rows));
									std::vector<KeyFramePair<descType>> keyFrames;
									matrix::Mat2D<uchar> gray(captureMat);
									std::vector<cv::KeyPoint> keyPoints;
									double nextPosition = keyFrameInterval;
									double timeIndex;
									for (;;)
									{
										kpGen->detect(gray, keyPoints);
										keyFrames.emplace_back(timeIndex = videoCapture.get(cv::CAP_PROP_POS_MSEC), descGen->compute(gray, keyPoints));
										for (;;)
										{
											if (!videoCapture.grab())
											{
												goto endHere;
											}
											if ((timeIndex = videoCapture.get(cv::CAP_PROP_POS_MSEC)) >= nextPosition)
											{
												nextPosition += keyFrameInterval;
												if (!videoCapture.retrieve(captureMat))
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
									return cache->addSig(VideoMetaData(vidFile, cols, rows, videoCapture.get(cv::CAP_PROP_FRAME_COUNT), videoCapture.get(cv::CAP_PROP_FPS), timeIndex), std::move(keyFrames));
								}
								CV_Error(1, "Video dimensions for file " + fileName + " are invalid");
							}
							CV_Error(1, "Unable to grab frame from video file " + fileName);
						}
						CV_Error(1, "Unable to open video file " + fileName);
					}
					CV_Error(1, "Video file " + fileName + " does not exist");
					throw std::exception(("Unspecified error on file " + fileName).c_str());//this is here to placate a compiler warning about not returning a value
				}

				void writeCacheToFile() const override
				{
					cache->writeToFile();
				}

				size_t getNumSigsInCache() const noexcept override
				{
					return cache->size();
				}

				inline ~SigGenFunctionImpl() noexcept
				{
					delete kpGenFactory;
					delete descGenFactory;
				}
			};

			class SigMatchFunctionImpl : public videoSigMatch::SigMatchFunction
			{
			protected:
				VideoSigCacheImpl<descType>* cache;
				feature2D::featureDescription::DescriptorMatcherImpl<descType>* descMatcher;
				double keyFrameIntervalRatio;
				double skipRatio;

				double getSigSimilarity(size_t numUsedFrames, double keyFrameInterval, double nextPos, KeyFrameItr<descType> longerItr, KeyFrameItr<descType>shrterItr, KeyFrameItr<descType> longerEnd, KeyFrameItr<descType>shrterEnd, double similarityThreshold) const noexcept
				{
					double similarity = 0;
					for (; longerItr != longerEnd; ++longerItr)
					{
						const double longerTimeIndex = longerItr->first;
						if (longerTimeIndex<nextPos)
						{
							--numUsedFrames;
							continue;
						}
						const auto& longerKF = longerItr->second;
						similarity += descMatcher->getFrameSimilarity(longerKF, VideoSigPtrImpl::iterateKeyFrameItr(shrterItr, longerTimeIndex, shrterEnd)->second);
						if (similarity / numUsedFrames>similarityThreshold)
						{
							//no match
							return similarityThreshold;
						}
						nextPos += keyFrameInterval;
					}
					if (numUsedFrames)
					{
						return similarity / numUsedFrames;
					}
					//no match
					return similarityThreshold;
				}

			public:

				size_t getKeyFrameFileSize(const std::pair<VideoMetaData, VideoMetaData>& pair) const noexcept override
				{
					return cache->getKeyFrameFileSize(pair);
				}
				SigMatchFunctionImpl(VideoSigCacheImpl<descType>* cache, const boost::property_tree::ptree& properties)
					: cache(cache)
					, descMatcher(feature2D::featureDescription::DescriptorMatcherImpl<descType>::getDescriptorMatcher(properties))
					, keyFrameIntervalRatio(properties.get<double>("keyFrameIntervalRatio", .01))
					, skipRatio(properties.get<double>("skipRatio", .05))
				{
				}

				bool cacheContains(const VideoMetaData& metaData) const noexcept override
				{
					return cache->contains(metaData);
				}
				virtual double getSigSimilarity(const std::pair<VideoMetaData, VideoMetaData>& candidate, double similarityThreshold) const noexcept
				{
					const std::pair<VideoSigPtrImpl, VideoSigPtrImpl>& ptrPair = cache->getMatchPairFromMetaDatas(candidate);
					const VideoSigPtrImpl& shrter = ptrPair.first;
					if (!shrter.empty())
					{
						const VideoSigPtrImpl& longer = ptrPair.second;
						const size_t numLongerKeyFrames = longer.size();
						if (numLongerKeyFrames)
						{
							const double longerDuration = longer.getDuration();
							return getSigSimilarity(numLongerKeyFrames, longerDuration*keyFrameIntervalRatio, longerDuration*skipRatio, longer.begin(), shrter.begin(), longer.end(), shrter.end(), similarityThreshold);
						}
					}
					//no match
					return similarityThreshold;
				}


				~SigMatchFunctionImpl()
				{
					delete descMatcher;
				}
			};

			class DriftedSigMatchFunctionImpl : public SigMatchFunctionImpl
			{
			protected:
				double maxDriftRatio;
				double driftRatioThreshold;

			public:
				DriftedSigMatchFunctionImpl(VideoSigCacheImpl<descType>* cache, const boost::property_tree::ptree& properties) noexcept
					: SigMatchFunctionImpl(cache, properties)
					, maxDriftRatio(properties.get<double>("maxDriftRatio", .0025))
					, driftRatioThreshold(properties.get<double>("driftRatioThreshold", .30))
				{}
				double getSigSimilarity(const std::pair<VideoMetaData, VideoMetaData>& candidate, double similarityThreshold) const noexcept override
				{
					const std::pair<VideoSigPtrImpl, VideoSigPtrImpl>& ptrPair = cache->getMatchPairFromMetaDatas(candidate);
					const VideoSigPtrImpl& shrter = ptrPair.first;
					if (!shrter.empty())
					{
						const VideoSigPtrImpl& longer = ptrPair.second;
						const size_t numLongerKeyFrames = longer.size();
						if (numLongerKeyFrames)
						{
							const double longerDuration = longer.getDuration();
							const double keyFrameInterval = longerDuration * keyFrameIntervalRatio;
							const double maxDrift = maxDriftRatio * longerDuration;
							double similarity = 0;
							double currentDrift = 0;
							double nextPos = longerDuration * skipRatio;
							size_t numUsedFrames = numLongerKeyFrames;
							KeyFrameItr<descType> shrterItr = shrter.begin();
							const KeyFrameItr<descType> shrterBegin = shrterItr;
							const KeyFrameItr<descType> shrterEnd = shrter.end();
							for (const auto& pair : longer)
							{
								const auto longerTimeIndex = pair.first;
								if (longerTimeIndex<nextPos)
								{
									--numUsedFrames;
									continue;
								}
								const auto& longerKF = pair.second;
								double centerScore = descMatcher->getFrameSimilarity(longerKF, VideoSigPtrImpl::iterateKeyFrameItr(shrterItr, longerTimeIndex, shrterEnd)->second);
								double centerTime = shrterItr->first;
								nextPos += keyFrameInterval;
								if (centerScore == 0)
								{
									continue;
								}
								double bestForwardDrift = 0;
								double bestForwardRatio = 1;
								double bestForwardScore = 1;
								KeyFrameItr<descType> forwardItr = shrterItr;
								for (;;)
								{
									double forwardDrift;
									if (++forwardItr == shrterEnd || (forwardDrift = (forwardItr->first - centerTime))>maxDrift)
									{
										break;
									}
									double forwardScore = descMatcher->getFrameSimilarity(longerKF, forwardItr->second);
									if (forwardScore<centerScore)
									{
										double forwardRatio = forwardScore / centerScore;
										if (forwardRatio<bestForwardRatio)
										{
											bestForwardDrift = forwardRatio;
											bestForwardScore = forwardScore;
											bestForwardDrift = forwardDrift;
										}
									}
								}
								double bestRearDrift = 0;
								double bestRearRatio = 1;
								double bestRearScore = 1;
								for (;;)
								{
									double rearDrift;
									if (shrterItr == shrterBegin || (rearDrift = (centerTime - (--shrterItr)->first))>maxDrift)
									{
										break;
									}
									double rearScore = descMatcher->getFrameSimilarity(longerKF, shrterItr->second);
									if (rearScore<centerScore)
									{
										double rearRatio = rearScore / centerScore;
										if (rearRatio<bestRearRatio)
										{

											bestRearRatio = rearRatio;
											bestRearScore = rearScore;
											bestRearDrift = rearDrift;
										}
									}
								}
								if (bestRearRatio<bestForwardRatio)
								{
									if (bestRearRatio<driftRatioThreshold)
									{
										currentDrift -= bestRearDrift;
										similarity += bestRearScore;
									}
									else
									{
										similarity += centerScore;
									}
								}
								else
								{
									if (bestForwardRatio<driftRatioThreshold)
									{
										currentDrift += bestForwardDrift;
										similarity += bestForwardScore;
									}
									else
									{
										similarity += centerScore;
									}
								}
								if (similarity / numUsedFrames>similarityThreshold)
								{
									//no match
									return similarityThreshold;
								}
							}
							if (numUsedFrames)
							{
								return similarity / numUsedFrames;
							}
						}
					}
					//no match
					return similarityThreshold;
				}

			};

			class MultiPhaseSigMatchFunctionImpl : public SigMatchFunctionImpl
			{
			protected:
				size_t numProbes;
			public:
				constexpr MultiPhaseSigMatchFunctionImpl(VideoSigCacheImpl<descType>* cache, const boost::property_tree::ptree& properties) noexcept
					: SigMatchFunctionImpl(cache, properties)
					, numProbes(std::max<size_t>(1, properties.get<size_t>("numProbes", 10)))
				{}

				double getSigSimilarity(const std::pair<VideoMetaData, VideoMetaData>& candidate, double similarityThreshold) const noexcept override
				{
					const std::pair<VideoSigPtrImpl, VideoSigPtrImpl>& ptrPair = cache->getMatchPairFromMetaDatas(candidate);
					const VideoSigPtrImpl& shrter = ptrPair.first;
					const size_t numShrterKeyFrames = int(shrter.size());
					if (numShrterKeyFrames)
					{
						const VideoSigPtrImpl& longer = ptrPair.second;
						const size_t numLongerKeyFrames = int(longer.size());
						if (numLongerKeyFrames)
						{
							//phase 1
							std::vector<size_t> probes(numProbes);
							double shrterInterval = double(numShrterKeyFrames) / numProbes;
							for (size_t i = 0; i != numProbes; ++i)
							{
								const double offset = double(i + 1);
								probes[i] = std::min<size_t>(numShrterKeyFrames - 1, cvRound(offset*shrterInterval));
							}
							const size_t offsetRange = numLongerKeyFrames - probes[numProbes - 1];
							double bestAvgSimilarity = 1;
							size_t bestOffset = 0;
							const KeyFrameItr<descType> longerKeyFrames = longer.begin();
							const KeyFrameItr<descType> shrterKeyFrames = shrter.begin();
							for (size_t i = 0; i != offsetRange; ++i)
							{
								double currSimilarity = descMatcher->getFrameSimilarity(longerKeyFrames[i].second, shrterKeyFrames[i].second);
								for (size_t j = 0; j != numProbes; ++j)
								{
									const size_t offset = probes[j];
									currSimilarity += descMatcher->getFrameSimilarity(longerKeyFrames[i + offset].second, shrterKeyFrames[offset].second);
								}
								if ((currSimilarity /= (numProbes + 1))<bestAvgSimilarity)
								{
									bestAvgSimilarity = currSimilarity;
									bestOffset = i;
								}
							}
							//phase 2
							const double shrterDuration = shrter.getDuration();
							return SigMatchFunctionImpl::getSigSimilarity(numShrterKeyFrames, keyFrameIntervalRatio*shrterDuration, shrterDuration*skipRatio, longerKeyFrames + bestOffset, shrterKeyFrames, longer.end(), shrter.end(), similarityThreshold);
						}
					}
					//no match
					return similarityThreshold;
				}


			};

			class MatchCandidatePredicate
			{
			public:
				virtual bool test(const VideoMetaData& metaData1, const VideoMetaData& metaData2) noexcept = 0;
				virtual ~MatchCandidatePredicate()
				{}
			};

			template<typename T>
			class RatioAndDiffPredicate
			{
			private:
				const T minVal;
				const T maxVal;
				const T maxAbsoluteDiff;
				const double maxRatio;
			public:
				RatioAndDiffPredicate(const T& minVal, const T& maxVal, const T& maxAbsoluteDiff, const double maxRatio)
					: minVal(minVal)
					, maxVal(maxVal)
					, maxAbsoluteDiff(maxAbsoluteDiff)
					, maxRatio(maxRatio)
				{
				}
				bool operator()(const T& val1, const T& val2) const noexcept
				{
					if (val1 < val2)
					{
						if (val1<minVal || val2>maxVal)
						{
							return false;
						}
						const T diff = val2 - val1;
						if (diff>maxAbsoluteDiff)
						{
							return false;
						}
						if ((double)diff / (double)val2>maxRatio)
						{
							return false;
						}
					}
					else
					{
						if (val2<minVal || val1>maxVal)
						{
							return false;
						}
						const T diff = val1 - val2;
						if (diff>maxAbsoluteDiff)
						{
							return false;
						}
						if ((double)diff / (double)val1>maxRatio)
						{
							return false;
						}
					}
					return true;
				}
			};

			class BlackAndWhiteListPredicate
			{
			private:
				DisjointSet<fileManagement::FileInfo> whiteList;
				MutualMap<fileManagement::FileInfo> blackList;
			public:
				BlackAndWhiteListPredicate(const boost::optional<std::string>& optionalWhiteListPath, const boost::optional<std::string>& optionalBlackListPath)
				{
					if (optionalWhiteListPath.is_initialized())
					{
						whiteList.readFromFile(boost::filesystem::path(optionalWhiteListPath.get()));
					}
					if (optionalBlackListPath.is_initialized())
					{
						blackList.readFromFile(boost::filesystem::path(optionalBlackListPath.get()));
					}
				}
				bool operator()(const fileManagement::FileInfo& file1, const fileManagement::FileInfo& file2) noexcept
				{
					return !whiteList.areJoined(file1, file2) && !blackList.contains(file1, file2);
				}
			};

			class DefaultMatchCandidatePredicate : public MatchCandidatePredicate
			{
			private:
				RatioAndDiffPredicate<double> durationPredicate;
				RatioAndDiffPredicate<size_t> fileSizePredicate;
				BlackAndWhiteListPredicate blackAndWhiteListPredicate;
			public:
				DefaultMatchCandidatePredicate(const boost::property_tree::ptree& properties)
					: durationPredicate(
						properties.get<double>("minDuration", 0),
						properties.get<double>("maxDuration", std::numeric_limits<double>::infinity()),
						properties.get<double>("maxAbsoluteDurationDiff", std::numeric_limits<double>::infinity()),
						properties.get<double>("maxDurationRatio", std::numeric_limits<double>::infinity())
					)
					, fileSizePredicate(
						properties.get<size_t>("minFileSize", 0),
						properties.get<size_t>("maxFileSize", std::numeric_limits<size_t>::max()),
						properties.get<size_t>("maxAbsoluteFileSizeDiff", std::numeric_limits<size_t>::max()),
						properties.get<double>("maxFileSizeRatio", std::numeric_limits<double>::infinity())
					)
					, blackAndWhiteListPredicate(
						properties.get_optional<std::string>("whiteListFile"),
						properties.get_optional<std::string>("blackListFile")
					)
				{

				}
				virtual bool test(const VideoMetaData& metaData1, const VideoMetaData& metaData2) noexcept override
				{
					if (durationPredicate(metaData1.getDuration(), metaData2.getDuration()))
					{
						const fileManagement::FileInfo& file1 = metaData1.getVidFile();
						const fileManagement::FileInfo& file2 = metaData2.getVidFile();
						return fileSizePredicate(file1.getFileSize(), file2.getFileSize())
							&& blackAndWhiteListPredicate(file1, file2);
					}
					return false;
				}
			};

			class PartitionedMatchCandidatePredicate : public DefaultMatchCandidatePredicate
			{
			public:
				inline PartitionedMatchCandidatePredicate(const boost::property_tree::ptree& properties) :
					DefaultMatchCandidatePredicate(properties)
				{

				}
				bool test(const VideoMetaData& metaData1, const VideoMetaData& metaData2) noexcept override
				{
					return DefaultMatchCandidatePredicate::test(metaData1, metaData2) &&
						metaData1.getVidFile().getPath().parent_path() != metaData2.getVidFile().getPath().parent_path();
				}

			};



			static MatchCandidatePredicate* getMatchCandidatePredicate(const boost::property_tree::ptree& properties)
			{
				if (properties.get<std::string>("matchCandidatePredicateType", "default") == "partitioned")
				{
					return new PartitionedMatchCandidatePredicate(properties);
				}
				return new DefaultMatchCandidatePredicate(properties);
			}
		

			template<class VidFileContainer>
			void pruneCacheTemplated(VidFileContainer& vidFileContainer)
			{
				std::vector<std::pair<VideoMetaData, VideoSigImpl>> outOfSync;
				for (const auto& vidFile : vidFileContainer)
				{
					for (auto itr = metaDatasToSigs.begin(); itr != metaDatasToSigs.end(); ++itr)
					{
						const fileManagement::FileInfo& cacheVidFile = itr->first.getVidFile();
						if (vidFile == cacheVidFile)
						{
							if (vidFile.getPath() != cacheVidFile.getPath())
							{
								auto outOfSyncPair = std::make_pair(itr->first, itr->second);
								std::cout << "Cached location for " << cacheVidFile.getPath().filename() << " was out of sync. Updating to " << vidFile.getPath() << std::endl;
								metaDatasToSigs.erase(itr);
								outOfSyncPair.first.updateVidFileLocation(vidFile.getPath());
								outOfSync.emplace_back(outOfSyncPair);
							}
							break;
						}
					}
				}
				for (auto& outOfSyncPair : outOfSync)
				{
					metaDatasToSigs.emplace(outOfSyncPair.first, outOfSyncPair.second);
				}
				std::unordered_set<fileManagement::FileInfo> kfFiles;
				fileManagement::FileGatherer sigGatherer(cacheFile.parent_path(), std::numeric_limits<size_t>::max(), false, std::vector<boost::regex>{boost::regex("(.*)\\.sig")}, std::vector<boost::regex>{boost::regex(cacheFile.string())});
				sigGatherer.gatherFiles(std::inserter(kfFiles, kfFiles.end()));
				for (auto cacheItr = metaDatasToSigs.begin(); cacheItr != metaDatasToSigs.end();)
				{
					VideoSigImpl& sig = cacheItr->second;
					const fileManagement::FileInfo& cacheKFFile = sig.keyFramesFile;
					const auto kfFileEnd = kfFiles.end();
					const auto kfFileItr = std::find(kfFiles.begin(), kfFileEnd, cacheKFFile);
					if (kfFileItr == kfFileEnd)
					{
						boost::filesystem::path keyFrameFilePath = cacheKFFile.getPath();
						if (boost::filesystem::exists(keyFrameFilePath))
						{
							boost::filesystem::remove(keyFrameFilePath);
						}
						cacheItr = metaDatasToSigs.erase(cacheItr);
					}
					else
					{
						const auto vidFileEnd = vidFileContainer.end();
						const VideoMetaData& cacheMetaData = cacheItr->first;
						const auto vidFileItr = std::find(vidFileContainer.begin(), vidFileEnd, cacheMetaData.getVidFile());
						const fileManagement::FileInfo& foundKFFile = *kfFileItr;
						if (vidFileItr == vidFileEnd || foundKFFile.getFileSize() != cacheKFFile.getFileSize())
						{
							boost::filesystem::path keyFrameFilePath = foundKFFile.getPath();
							if (boost::filesystem::exists(keyFrameFilePath))
							{
								boost::filesystem::remove(keyFrameFilePath);
							}
							cacheItr = metaDatasToSigs.erase(cacheItr);
						}
						else
						{
							vidFileContainer.erase(vidFileItr);
							sig.keyFramesFile = foundKFFile;
							++cacheItr;
						}
						kfFiles.erase(kfFileItr);
					}
				}
				for (const fileManagement::FileInfo& kfFile : kfFiles)
				{
					const boost::filesystem::path& kfFilePath = kfFile.getPath();
					if (boost::filesystem::exists(kfFilePath))
					{
						boost::filesystem::remove(kfFilePath);
					}
				}
			}
		public:

			size_t size() const noexcept override
			{
				return metaDatasToSigs.size();
			}

			
			void setMaxNumSigs(const size_t maxNumSigs = std::numeric_limits<size_t>::max()) noexcept
			{
				this->maxNumSigs = maxNumSigs;
			}

			void pruneCache(std::vector<fileManagement::FileInfo>& vidFileContainer) noexcept override
			{
				pruneCacheTemplated(vidFileContainer);
			}

			void pruneCache(std::set<fileManagement::FileInfo>& vidFileContainer) noexcept override
			{
				pruneCacheTemplated(vidFileContainer);
			}

			size_t getKeyFrameFileSize(const std::pair<VideoMetaData, VideoMetaData>& pair) const noexcept
			{
				return metaDatasToSigs.at(pair.first).keyFramesFile.getFileSize() + metaDatasToSigs.at(pair.second).keyFramesFile.getFileSize();
			}

			bool contains(const VideoMetaData& metaData) const noexcept override
			{
				return metaDatasToSigs.find(metaData) != metaDatasToSigs.end();
			}

			videoSigMatch::SigMatchFunction* getSigMatchFunction(const boost::property_tree::ptree& properties)
			{
				const std::string sigMatchFunction = properties.get<std::string>("sigMatchFunction", "multiphase");
				if (sigMatchFunction == "drifted")
				{
					return new DriftedSigMatchFunctionImpl(this,properties);
				}
				else if (sigMatchFunction == "static")
				{
					return new SigMatchFunctionImpl(this,properties);
				}
				else if (sigMatchFunction == "multiphase")
				{
					return new MultiPhaseSigMatchFunctionImpl(this,properties);
				}
				CV_Error(1, "Unknown sig match session : " + sigMatchFunction);
				//To satisfy a compiler warning
				throw std::exception("Unspecified exception");
			}

			videoSigGen::SigGenFunction* getSigGenFunction(const boost::property_tree::ptree& properties)
			{
				return new SigGenFunctionImpl(this, properties);
			}


			void writeToFile(const boost::filesystem::path& file) const
			{
				boost::filesystem::create_directories(file.parent_path());
				boost::filesystem::path tmp = file.string() + ".tmp";
				boost::filesystem::ofstream cacheOut(tmp, std::ios::binary | std::ios::out);
				if (cacheOut.is_open())
				{
					{
						boost::archive::binary_oarchive cacheArchive(cacheOut);
						cacheArchive & *this;
					}
					cacheOut.close();
					boost::filesystem::rename(tmp, file);
					return;
				}
				CV_Error(1, "Unable to open " + tmp.string() + " for writing");
			}
			void writeToFile() const override
			{
				writeToFile(cacheFile);
			}

			constexpr const std::tuple<VideoMetaData, fileManagement::FileInfo, size_t> addSig(VideoMetaData&& metaData, std::vector<KeyFramePair<descType>>&& keyFrames)
			{
				const boost::filesystem::path kfFile = metaData.getVidFile().resolveToDirectory<true>(cacheFile.parent_path(), L".sig");
				boost::filesystem::create_directories(kfFile.parent_path());
				const boost::filesystem::path tmpFile = (kfFile.wstring()) + L".tmp";
				boost::filesystem::ofstream sigOut(tmpFile, std::ios::binary | std::ios::out);
				if (sigOut.is_open())
				{
					{
						boost::archive::binary_oarchive sigArchive(sigOut);
						for (const KeyFramePair<descType>& framePair : keyFrames)
						{
							sigArchive & framePair.first;
							sigArchive & framePair.second;
						}
					}
					sigOut.close();
					boost::filesystem::rename(tmpFile, kfFile);

					VideoSigImpl sig(this, keyFrames.size(), fileManagement::FileInfo(kfFile));
					keyFrames.clear();
					std::lock_guard<std::mutex> lock(mut);

					auto itr = metaDatasToSigs.emplace_hint(metaDatasToSigs.end(), metaData, std::move(sig));
					std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
					if (now - lastWriteTime >= writeInterval)
					{
						writeToFile();
						lastWriteTime = now;
					}
					return std::make_tuple(itr->first, itr->second.keyFramesFile, itr->second.numKeyFrames);
				}
				CV_Error(1, "Unable to open keyframe file " + kfFile.string() + " for writing");
				//to satisfy compiler warning
				throw std::exception("unspecified error");
			}

			constexpr const std::pair<VideoSigPtrImpl, VideoSigPtrImpl> getMatchPairFromMetaDatas(const std::pair<VideoMetaData, VideoMetaData>& pair)
			{
				CV_DbgAssert(pair.first != pair.second);
				std::unique_lock<std::mutex> lock(mut);
				const VideoMetaData& metaData1 = pair.first;
				const VideoMetaData& metaData2 = pair.second;
				VideoSigImpl* sig1 = &metaDatasToSigs.at(metaData1);
				VideoSigImpl* sig2 = &metaDatasToSigs.at(metaData2);
				std::pair<VideoSigPtrImpl, VideoSigPtrImpl> matchPair(VideoSigPtrImpl(&metaData1, sig1), VideoSigPtrImpl(&metaData2, sig2));
				if (head)
				{
					if (sig1 == head)
					{
						moveSigToSecondPosition(sig2);
					}
					else if (sig2 == head)
					{
						moveSigToSecondPosition(sig1);
					}
					else
					{
						if (sig1 == tail)
						{
							moveSigFromTail(sig1, sig2);
						}
						else if (sig2 == tail)
						{
							moveSigFromTail(sig2, sig1);
						}
						else
						{
							if (sig1->next == sig2)
							{
								moveLinkedSigsToHead(sig1, sig2);
							}
							else if (sig1->prev == sig2)
							{
								moveLinkedSigsToHead(sig2, sig1);
							}
							else
							{
								detach(sig2);
								detach(sig1);
								sig1->next = head;
								sig1->prev = sig2;
								sig2->prev = NULL;
								sig2->next = sig1;
								head->prev = sig1;
								head = sig2;
							}
						}
					}
				}
				else
				{
					head = sig2;
					tail = sig1;
					sig2->next = sig1;
					sig2->prev = NULL;
					sig1->prev = sig2;
					sig1->next = NULL;
				}
				if (currentNumSigs>maxNumSigs || currentMemoryFootprint>maxMemoryFootprint)
				{
					for (;;)
					{
						VideoSigImpl* tmp = tail;
						for (;;)
						{
							if (tmp->holdCount == 0)
							{
								tmp->clear();
								if (tmp == tail)
								{
									if (tmp == head)
									{
										tail = head = NULL;
									}
									else
									{
										tail = tail->prev;
										tail->next = NULL;
									}
								}
								else
								{
									if (tmp == head)
									{
										head = head->next;
										head->prev = NULL;
									}
									else
									{
										detach(tmp);
									}
								}
								tmp->next = tmp->prev = NULL;
								break;
							}
							else
							{
								if (tmp == head)
								{
									cv.wait_for(lock, std::chrono::milliseconds(5000));
									tmp = tail;
									if (tmp == NULL)
									{
										break;
									}
								}
								else
								{
									tmp = tmp->prev;
								}
							}
						}
						if ((currentNumSigs <= maxNumSigs && currentMemoryFootprint<maxMemoryFootprint) || currentNumSigs <= 2 || currentMemoryFootprint == 0)
						{
							break;
						}
					}
				}
				return matchPair;
			}


			VideoSigCacheImpl(const VideoSigCacheImpl<descType>& that) noexcept = delete;

			VideoSigCacheImpl(VideoSigCacheImpl<descType>&& that) noexcept
				:maxMemoryFootprint(that.maxMemoryFootprint)
				,maxNumSigs(that.maxNumSigs)
				,currentMemoryFootprint(that.currentMemoryFootprint)
				,currentNumSigs(that.currentNumSigs)
				,cv()
				,mut()
				,lastWriteTime(that.lastWriteTime)
				,writeInterval(that.writeInterval)
				,cacheFile(std::move(that.cacheFile))
				,head(that.head)
				,tail(that.tail)
				,metaDatasToSigs(std::move(that.metaDatasToSigs))
			{}

			class CandidateSorter
			{
			private:
				class CandidateSortImpl
				{
				public:
					virtual bool operator()(const std::pair<VideoMetaData, VideoMetaData>& pair1, const std::pair<VideoMetaData, VideoMetaData>& pair2) const noexcept
					{
						return pair1.first<pair2.first || (pair1.first == pair2.first && pair1.second<pair2.second);
					}
					virtual ~CandidateSortImpl()
					{}
				};
				class DurationDiffCandidateSortImpl : public CandidateSortImpl
				{
				public:
					bool operator()(const std::pair<VideoMetaData, VideoMetaData>& pair1, const std::pair<VideoMetaData, VideoMetaData>& pair2) const noexcept override
					{
						const double diff1 = std::fabs(pair1.first.getDuration() - pair1.second.getDuration());
						const double diff2 = std::fabs(pair2.first.getDuration() - pair2.second.getDuration());
						return diff1<diff2 || (diff1 == diff2 && CandidateSortImpl::operator()(pair1, pair2));
					}
				};

				const CandidateSortImpl* const impl;

				static const CandidateSortImpl* const getImpl(const boost::property_tree::ptree& properties)
				{
					const std::string sorterType = properties.get<std::string>("sorterType", "default");
					if (sorterType == "default")
					{
						return new CandidateSortImpl();
					}
					if (sorterType == "durationDiff")
					{
						return new DurationDiffCandidateSortImpl();
					}
					CV_Error(1, "Unknown sorterType " + sorterType);
					return NULL;
				}

			public:
				CandidateSorter(const boost::property_tree::ptree& properties) :
					impl(getImpl(properties))
				{

				}

				bool operator()(const std::pair<VideoMetaData, VideoMetaData>& pair1, const std::pair<VideoMetaData, VideoMetaData>& pair2) const noexcept
				{
					return impl->operator()(pair1, pair2);
				}
				virtual ~CandidateSorter()
				{
					delete impl;
				}
			};

			std::vector<std::pair<VideoMetaData, VideoMetaData>> getMatchCandidates(const boost::property_tree::ptree& properties, std::unordered_set<videoSigMatch::SigMatch, videoSigMatch::SigMatchHash, videoSigMatch::SigMatchEquals>& existingMatches = std::unordered_set<videoSigMatch::SigMatch, videoSigMatch::SigMatchHash, videoSigMatch::SigMatchEquals>()) noexcept override
			{
				const size_t numThreads = lyonste::getNumThreadsProperty(properties);
				std::unique_ptr<MatchCandidatePredicate> candidatePredicate(getMatchCandidatePredicate(properties));
				std::vector<std::pair<VideoMetaData, VideoMetaData>> matchCandidates;
				std::insert_iterator< std::vector<std::pair<VideoMetaData, VideoMetaData>>> inserterItr = std::inserter(matchCandidates, matchCandidates.end());
				if (metaDatasToSigs.size() > 1)
				{
					const auto metaDataEnd = metaDatasToSigs.end();
					size_t maxRun = 0;
					for (auto itr1 = metaDatasToSigs.begin(); itr1 != metaDataEnd; ++itr1)
					{
						const VideoMetaData& metaData1 = itr1->first;
						size_t currentRun = 0;
						for (auto itr2 = itr1; ++itr2 != metaDataEnd;)
						{
							const VideoMetaData& metaData2 = itr2->first;
							//if (candidatePredicate->test(metaData1, metaData2))
							if(candidatePredicate->test(metaData1,metaData2) && existingMatches.find(videoSigMatch::SigMatch(std::make_pair(metaData1,metaData2)))==existingMatches.end())
						    //if(candidatePredicate->test(metaData1,metaData2) && !existingMatches.areJoined(metaData1, metaData2))
							{
								//const std::string& str1 = metaData1.getVidFile().getFilePathStr();
								//const std::string& str2 = metaData2.getVidFile().getFilePathStr();

								//const std::string vidFileStr1 = "F:\\uncategorized\\gts\\ate goldfish for 50.mp4";
								//const std::string vidFileStr2 = "F:\\uncategorized\\gts\\Alice - Mouse Swallowing-5.mp4";
								//if ((str1 == vidFileStr1 && str2 == vidFileStr2) || (str2 == vidFileStr1 && str1 == vidFileStr2))
								//{
								//	std::cout << "here" << std::endl;
								//}

								//if (!existingMatches.areJoined(metaData1, metaData2))
								{
									++currentRun;
									if (metaData1<metaData2)
									{
										inserterItr = std::make_pair(metaData1, metaData2);
									}
									else
									{
										inserterItr = std::make_pair(metaData2, metaData1);
									}
								}

								
							}
						}
						if (currentRun > maxRun)
						{
							maxRun = currentRun;
						}
					}
					this->head = NULL;
					this->tail = NULL;
					this->currentMemoryFootprint = 0;
					this->currentNumSigs = 0;
					if (properties.get<std::string>("sorterType", "default") == "durationDiff")
					{
						this->maxNumSigs=std::numeric_limits<size_t>::max();
					}
					else
					{
						this->maxNumSigs = std::max<size_t>(numThreads << 1, maxRun + 1);
					}

					std::sort(matchCandidates.begin(), matchCandidates.end(), CandidateSorter(properties));
					
				}
				return matchCandidates;
			}

			VideoSigCacheImpl<descType>& operator=(const VideoSigCacheImpl<descType>& that) noexcept = delete;

			constexpr VideoSigCacheImpl<descType>& operator=(VideoSigCacheImpl<descType>&& that) noexcept
			{
				if (this != &that)
				{
					maxMemoryFootprint = that.maxMemoryFootprint;
					maxNumSigs = that.maxNumSigs;
					currentMemoryFootprint = that.currentMemoryFootprint;
					currentNumSigs = that.currentNumSigs;

					lastWriteTime = that.lastWriteTime;
					writeInterval = that.writeInterval;
					cacheFile = std::move(that.cacheFile);
					head = that.head;
					tail = that.tail;
					metaDatasToSigs = std::move(that.metaDatasToSigs);
				}
				return *this;
			}

			constexpr VideoSigCacheImpl(const boost::property_tree::ptree& properties)
				:maxMemoryFootprint(properties.get<size_t>("maxMemoryFootprint", std::numeric_limits<size_t>::max())),
				maxNumSigs(std::numeric_limits<size_t>::max()),
				currentMemoryFootprint(0),
				currentNumSigs(0),
				cv(),
				mut(),
				lastWriteTime(std::chrono::system_clock::now()),
				writeInterval(properties.get<size_t>("writeIntervalMillis", 0)),
				cacheFile(properties.get<std::string>("cachePath"))
			{
				readFromFile();
			}
		};

		VideoSigCache* getVideoSigCache(const boost::property_tree::ptree& properties)
		{
			const std::string& descriptorType = properties.get<std::string>("descriptorType");
			if (descriptorType == "freak")
			{
				return new VideoSigCacheImpl<feature2D::featureDescription::Freak>(properties);
			}
			CV_Error(1, "Unknown descriptorType " + descriptorType);
			return NULL;
		}

	}
}