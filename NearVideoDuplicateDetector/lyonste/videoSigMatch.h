#pragma once

#include "videoSig.h"

namespace lyonste
{
	namespace videoSig
	{
		namespace videoSigMatch
		{

			struct SigMatch
			{
				VideoMetaData vid1;
				VideoMetaData vid2;
				double similarity;

				template<class Archive>
				constexpr void serialize(Archive &archive,const unsigned int version)
				{
					archive & vid1;
					archive & vid2;
					archive & similarity;
				}

				inline SigMatch() noexcept
				{}

				inline SigMatch(const std::pair<VideoMetaData,VideoMetaData>& pair,double similarity) noexcept
					:vid1(pair.first),
					vid2(pair.second),
					similarity(similarity)
				{}

				constexpr bool operator<(const SigMatch& that) const noexcept
				{
					return similarity<that.similarity
						||(similarity==that.similarity
						   &&((vid1.getVidFile().getFileSize()+vid2.getVidFile().getFileSize())
							  >(that.vid1.getVidFile().getFileSize()+that.vid2.getVidFile().getFileSize())));


				}
			};

			struct SigMatchEquals
			{
				constexpr bool operator()(const SigMatch& match1,const SigMatch& match2) const noexcept
				{
					return &match1==&match2||(match1.vid1==match2.vid1&&match1.vid2==match2.vid2);
				}
			};
			struct SigMatchHash
			{
				constexpr size_t operator()(const SigMatch& match) const noexcept
				{
					return std::hash<VideoMetaData>()(match.vid1)*31+std::hash<VideoMetaData>()(match.vid2);
				}
			};

			template<feature2D::featureDescription::DescriptorType descType>
			class SigMatchFunction
			{
			protected:
				feature2D::featureDescription::DescriptorMatcher<descType>* descMatcher;
				double keyFrameIntervalRatio;
				double skipRatio;

				constexpr double getSigSimilarity(size_t numUsedFrames,double keyFrameInterval,double nextPos,KeyFrameItr<descType> longerItr,KeyFrameItr<descType>shrterItr,KeyFrameItr<descType> longerEnd,KeyFrameItr<descType>shrterEnd,double similarityThreshold) const noexcept
				{
					//const double longerDuration=longer.getDuration();
					//const double keyFrameInterval=longerDuration*keyFrameIntervalRatio;
					double similarity=0;
					//double nextPos=longerDuration*skipRatio;
					//size_t numUsedFrames=longer.size();
					//KeyFrameItr<descType> shrterItr=shrter.begin();
					//const KeyFrameItr<descType> shrterEnd=shrter.end();
					//for(const auto&[longerTimeIndex,longerKF]:longer)
					for(;longerItr!=longerEnd;++longerItr)
					{
						const double longerTimeIndex=longerItr->first;
						if(longerTimeIndex<nextPos)
						{
							--numUsedFrames;
							continue;
						}
						const auto& longerKF=longerItr->second;
						similarity+=descMatcher->getFrameSimilarity(longerKF,VideoSigPtr<descType>::iterateKeyFrameItr(shrterItr,longerTimeIndex,shrterEnd)->second);
						if(similarity/numUsedFrames>similarityThreshold)
						{
							//no match
							return similarityThreshold+1;
						}
						nextPos+=keyFrameInterval;
					}
					if(numUsedFrames)
					{
						return similarity/numUsedFrames;
					}
					return similarityThreshold+1;
				}

			public:
				constexpr SigMatchFunction(const boost::property_tree::ptree& properties)
					:descMatcher(feature2D::featureDescription::DescriptorMatcher<descType>::getDescriptorMatcher(properties))
					,keyFrameIntervalRatio(properties.get<double>("keyFrameIntervalRatio",.01))
					,skipRatio(properties.get<double>("skipRatio",.05))
				{
				}
				constexpr virtual double getSigSimilarity(const std::pair<VideoSigPtr<descType>,VideoSigPtr<descType>>& ptrPair,double similarityThreshold) const noexcept
				{
					const VideoSigPtr<descType>& shrter=ptrPair.first;
					if(!shrter.empty())
					{
						const VideoSigPtr<descType>& longer=ptrPair.second;
						const size_t numLongerKeyFrames=longer.size();
						if(numLongerKeyFrames)
						{
							const double longerDuration=longer.getDuration();
							return getSigSimilarity(numLongerKeyFrames,longerDuration*keyFrameIntervalRatio,longerDuration*skipRatio,longer.begin(),shrter.begin(),longer.end(),shrter.end(),similarityThreshold);
							//const double longerDuration=longer.getDuration();
							//const double keyFrameInterval=longerDuration*keyFrameIntervalRatio;
							//double similarity=0;
							//double nextPos=longerDuration*skipRatio;
							//size_t numUsedFrames=numLongerKeyFrames;
							//KeyFrameItr<descType> shrterItr=shrter.begin();
							//const KeyFrameItr<descType> shrterEnd=shrter.end();
							//for(const auto&[longerTimeIndex,longerKF]:longer)
							//{
							//	if(longerTimeIndex<nextPos)
							//	{
							//		--numUsedFrames;
							//		continue;
							//	}
							//	similarity+=descMatcher->getFrameSimilarity(longerKF,VideoSigPtr<descType>::iterateKeyFrameItr(shrterItr,longerTimeIndex,shrterEnd)->second);
							//	if(similarity/numUsedFrames>similarityThreshold)
							//	{
							//		//no match
							//		return similarityThreshold+1;
							//	}
							//	nextPos+=keyFrameInterval;
							//}
							//if(numUsedFrames)
							//{
							//	return similarity/numUsedFrames;
							//}
						}
					}
					//no match
					return similarityThreshold+1;
				}


				~SigMatchFunction()
				{
					delete descMatcher;
				}
			};


			template<feature2D::featureDescription::DescriptorType descType>
			class DriftedSigMatchFunction: public SigMatchFunction<descType>
			{
			protected:
				double maxDriftRatio;
				double driftRatioThreshold;

			public:
				constexpr DriftedSigMatchFunction(const boost::property_tree::ptree& properties) noexcept
					:SigMatchFunction<descType>(properties)
					,maxDriftRatio(properties.get<double>("maxDriftRatio",.0025))
					,driftRatioThreshold(properties.get<double>("driftRatioThreshold",.30))
				{}
				constexpr double getSigSimilarity(const std::pair<VideoSigPtr<descType>,VideoSigPtr<descType>>& ptrPair,double similarityThreshold) const noexcept override
				{
					const VideoSigPtr<descType>& shrter=ptrPair.first;
					if(!shrter.empty())
					{
						const VideoSigPtr<descType>& longer=ptrPair.second;
						const size_t numLongerKeyFrames=longer.size();
						if(numLongerKeyFrames)
						{
							const double longerDuration=longer.getDuration();
							const double keyFrameInterval=longerDuration*keyFrameIntervalRatio;
							const double maxDrift=maxDriftRatio*longerDuration;
							double similarity=0;
							double currentDrift=0;
							double nextPos=longerDuration*skipRatio;
							size_t numUsedFrames=numLongerKeyFrames;
							KeyFrameItr<descType> shrterItr=shrter.begin();
							const KeyFrameItr<descType> shrterBegin=shrterItr;
							const KeyFrameItr<descType> shrterEnd=shrter.end();
							for(const auto&[longerTimeIndex,longerKF]:longer)
							{
								if(longerTimeIndex<nextPos)
								{
									--numUsedFrames;
									continue;
								}
								double centerScore=descMatcher->getFrameSimilarity(longerKF,VideoSigPtr<descType>::iterateKeyFrameItr(shrterItr,longerTimeIndex,shrterEnd)->second);
								double centerTime=shrterItr->first;
								nextPos+=keyFrameInterval;
								if(centerScore==0)
								{
									continue;
								}
								double bestForwardDrift=0;
								double bestForwardRatio=1;
								double bestForwardScore=1;
								KeyFrameItr<descType> forwardItr=shrterItr;
								for(;;)
								{
									double forwardDrift;
									if(++forwardItr==shrterEnd||(forwardDrift=(forwardItr->first-centerTime))>maxDrift)
									{
										break;
									}
									double forwardScore=descMatcher->getFrameSimilarity(longerKF,forwardItr->second);
									if(forwardScore<centerScore)
									{
										double forwardRatio=forwardScore/centerScore;
										if(forwardRatio<bestForwardRatio)
										{
											bestForwardDrift=forwardRatio;
											bestForwardScore=forwardScore;
											bestForwardDrift=forwardDrift;
										}
									}
								}
								double bestRearDrift=0;
								double bestRearRatio=1;
								double bestRearScore=1;
								for(;;)
								{
									double rearDrift;
									if(shrterItr==shrterBegin||(rearDrift=(centerTime-(--shrterItr)->first))>maxDrift)
									{
										break;
									}
									double rearScore=descMatcher->getFrameSimilarity(longerKF,shrterItr->second);
									if(rearScore<centerScore)
									{
										double rearRatio=rearScore/centerScore;
										if(rearRatio<bestRearRatio)
										{

											bestRearRatio=rearRatio;
											bestRearScore=rearScore;
											bestRearDrift=rearDrift;
										}
									}
								}
								if(bestRearRatio<bestForwardRatio)
								{
									if(bestRearRatio<driftRatioThreshold)
									{
										currentDrift-=bestRearDrift;
										similarity+=bestRearScore;
									}
									else
									{
										similarity+=centerScore;
									}
								}
								else
								{
									if(bestForwardRatio<driftRatioThreshold)
									{
										currentDrift+=bestForwardDrift;
										similarity+=bestForwardScore;
									}
									else
									{
										similarity+=centerScore;
									}
								}
								if(similarity/numUsedFrames>similarityThreshold)
								{
									//no match
									return similarityThreshold+1;
								}
							}
							if(numUsedFrames)
							{
								return similarity/numUsedFrames;
							}
						}
					}
					//no match
					return similarityThreshold+1;
				}
			};

			template<feature2D::featureDescription::DescriptorType descType>
			class MultiPhaseSigMatchFunction: public SigMatchFunction<descType>
			{
			protected:
				size_t numProbes;
			public:
				constexpr MultiPhaseSigMatchFunction(const boost::property_tree::ptree& properties) noexcept
					:SigMatchFunction<descType>(properties)
					,numProbes(std::max<size_t>(1,properties.get<size_t>("numProbes",10)))
				{}

				constexpr double getSigSimilarity(const std::pair<VideoSigPtr<descType>,VideoSigPtr<descType>>& ptrPair,double similarityThreshold) const noexcept override
				{
					const VideoSigPtr<descType>& shrter=ptrPair.first;
					const size_t numShrterKeyFrames=int(shrter.size());
					if(numShrterKeyFrames)
					{
						const VideoSigPtr<descType>& longer=ptrPair.second;
						const size_t numLongerKeyFrames=int(longer.size());
						if(numLongerKeyFrames)
						{
							//phase 1
							//std::vector<std::pair<size_t,size_t>> probeOffsets(numProbes);
							std::vector<size_t> probes(numProbes);
							double shrterInterval=double(numShrterKeyFrames)/numProbes;
							//double longerInterval=double(numLongerKeyFrames)/numProbes;
							for(size_t i=0;i!=numProbes;++i)
							{
								const double offset=double(i+1);
								probes[i]=std::min<size_t>(numShrterKeyFrames-1,cvRound(offset*shrterInterval));
								//probeOffsets[i]=std::make_pair(std::min<size_t>(numShrterKeyFrames-1,cvRound(offset*shrterInterval)),std::min<size_t>(numLongerKeyFrames-1,cvRound(offset*longerInterval)));
							}
							const size_t offsetRange=numLongerKeyFrames-probes[numProbes-1];
							double bestAvgSimilarity=1;
							size_t bestOffset=0;
							const KeyFrameItr<descType> longerKeyFrames=longer.begin();
							const KeyFrameItr<descType> shrterKeyFrames=shrter.begin();
							for(size_t i=0;i!=offsetRange;++i)
							{
								double currSimilarity=descMatcher->getFrameSimilarity(longerKeyFrames[i].second,shrterKeyFrames[i].second);
								for(size_t j=0;j!=numProbes;++j)
								{
									const size_t offset=probes[j];
									//const std::pair<size_t,size_t>& offsetPair=probeOffsets[j];
									currSimilarity+=descMatcher->getFrameSimilarity(longerKeyFrames[i+offset].second,shrterKeyFrames[offset].second);
								}
								//const std::pair<size_t,size_t>& offsetPair=probeOffsets[numProbes-1];
								//currSimilarity+=descMatcher->getFrameSimilarity(longerKeyFrames[std::min<size_t>(i+offsetPair.second,numLongerKeyFrames-1)].second,shrterKeyFrames[std::min<size_t>(i+offsetPair.first,numShrterKeyFrames-1)].second);
								if((currSimilarity/=(numProbes+1))<bestAvgSimilarity)
								{
									bestAvgSimilarity=currSimilarity;
									bestOffset=i;
								}
							}
							//phase 2
							const double shrterDuration=shrter.getDuration();
							return SigMatchFunction<descType>::getSigSimilarity(numShrterKeyFrames,keyFrameIntervalRatio*shrterDuration,shrterDuration*skipRatio,longerKeyFrames+bestOffset,shrterKeyFrames,longer.end(),shrter.end(),similarityThreshold);							
						}
					}
					//no match
					return similarityThreshold+1;
				}
			};


			template<feature2D::featureDescription::DescriptorType descType>
			constexpr SigMatchFunction<descType>* getSigMatchFunction(const boost::property_tree::ptree& properties)
			{
				const std::string sigMatchFunction=properties.get<std::string>("sigMatchFunction","multiphase");
				if(sigMatchFunction=="drifted")
				{
					return new DriftedSigMatchFunction<descType>(properties);
				}
				else if(sigMatchFunction=="static")
				{
					return new SigMatchFunction<descType>(properties);
				}
				else if(sigMatchFunction=="multiphase")
				{
					return new MultiPhaseSigMatchFunction<descType>(properties);
				}
				CV_Error(1,"Unknown sig match session : "+sigMatchFunction);
				//To satisfy a compiler warning
				throw std::exception("Unspecified exception");
			}

			class CandidateSorter
			{
			private:
				class CandidateSortImpl
				{
				public:
					virtual bool operator()(const std::pair<VideoMetaData,VideoMetaData>& pair1,const std::pair<VideoMetaData,VideoMetaData>& pair2) const noexcept
					{
						return pair1.first<pair2.first||(pair1.first==pair2.first && pair1.second<pair2.second);
					}
					virtual ~CandidateSortImpl()
					{}
				};
				class DurationDiffCandidateSortImpl: public CandidateSortImpl
				{
				public:
					bool operator()(const std::pair<VideoMetaData,VideoMetaData>& pair1,const std::pair<VideoMetaData,VideoMetaData>& pair2) const noexcept override
					{
						const double diff1=std::fabs(pair1.first.getDuration()-pair1.second.getDuration());
						const double diff2=std::fabs(pair2.first.getDuration()-pair2.second.getDuration());
						return diff1<diff2||(diff1==diff2 && CandidateSortImpl::operator()(pair1,pair2));
					}
				};

				const CandidateSortImpl* const impl;

				static const CandidateSortImpl* const getImpl(const boost::property_tree::ptree& properties)
				{
					const std::string sorterType=properties.get<std::string>("sorterType","default");
					if(sorterType=="default")
					{
						return new CandidateSortImpl();
					}
					if(sorterType=="durationDiff")
					{
						return new DurationDiffCandidateSortImpl();
					}
					CV_Error(1,"Unknown sorterType "+sorterType);
					return NULL;
				}

			public:
				CandidateSorter(const boost::property_tree::ptree& properties):
					impl(getImpl(properties))
				{

				}

				bool operator()(const std::pair<VideoMetaData,VideoMetaData>& pair1,const std::pair<VideoMetaData,VideoMetaData>& pair2) const noexcept
				{
					return impl->operator()(pair1,pair2);
				}
				virtual ~CandidateSorter()
				{
					delete impl;
				}
			};

			class MatchCandidatePredicate
			{
			private:
				class MatchCandidatePredicateImpl
				{
				public:
					virtual bool operator()(const VideoMetaData& metaData1,const VideoMetaData& metaData2) const noexcept=0;
					virtual ~MatchCandidatePredicateImpl()
					{}
				};

				class DefaultMatchCandidatePredicateImpl: public MatchCandidatePredicateImpl
				{
				private:
					double minDuration;
					double maxDuration;
					double maxAbsoluteDurationDiff;
					double maxDurationRatio;
					DisjointSet<fileManagement::FileInfo> whiteList;
					MutualMap<fileManagement::FileInfo> blackList;
				public:
					inline DefaultMatchCandidatePredicateImpl(const boost::property_tree::ptree& properties)
						:minDuration(properties.get<double>("minDuration",0)),
						maxDuration(properties.get<double>("maxDuration",std::numeric_limits<double>::infinity())),
						maxAbsoluteDurationDiff(properties.get<double>("maxAbsoluteDurationDiff",std::numeric_limits<double>::infinity())),
						maxDurationRatio(properties.get<double>("maxDurationRatio",std::numeric_limits<double>::infinity()))
					{
						const boost::optional<std::string>& optionalWhiteListPath=properties.get_optional<std::string>("whiteListFile");
						if(optionalWhiteListPath.is_initialized())
						{
							whiteList.readFromFile(boost::filesystem::path(optionalWhiteListPath.get()));
						}
						const boost::optional<std::string>& optionalBlackListPath=properties.get_optional<std::string>("blackListFile");
						if(optionalBlackListPath.is_initialized())
						{
							blackList.readFromFile(boost::filesystem::path(optionalBlackListPath.get()));
						}
					}


					virtual bool operator()(const VideoMetaData& metaData1,const VideoMetaData& metaData2) const noexcept override
					{
						const double duration1=metaData1.getDuration();
						const double duration2=metaData2.getDuration();
						if(duration1<duration2)
						{
							if(duration1<minDuration||duration2>maxDuration)
							{
								return false;
							}
							const double diff=duration2-duration1;
							if(diff>maxAbsoluteDurationDiff)
							{
								return false;
							}
							if(diff/duration2>maxDurationRatio)
							{
								return false;
							}
						}
						else
						{
							if(duration2<minDuration||duration1>maxDuration)
							{
								return false;
							}
							const double diff=duration1-duration2;
							if(diff>maxAbsoluteDurationDiff)
							{
								return false;
							}
							if(diff/duration1>maxDurationRatio)
							{
								return false;
							}
						}
						const fileManagement::FileInfo& fileInfo1=metaData1.getVidFile();
						const fileManagement::FileInfo& fileInfo2=metaData2.getVidFile();
						return !whiteList.areJoined(fileInfo1,fileInfo2)&&!blackList.contains(fileInfo1,fileInfo2);
					}
				
				};

				class PartitionedMatchCandidatePredicateImpl : public DefaultMatchCandidatePredicateImpl
				{
				public:
					inline PartitionedMatchCandidatePredicateImpl(const boost::property_tree::ptree& properties) :
						DefaultMatchCandidatePredicateImpl(properties)
					{

					}
					bool operator()(const VideoMetaData& metaData1, const VideoMetaData& metaData2) const noexcept override
					{
						//if (DefaultMatchCandidatePredicateImpl::operator()(metaData1, metaData2))
						//{
						//	const boost::filesystem::path& parentPath1=metaData1.getVidFile().getPath().parent_path();
						//	const boost::filesystem::path& parentPath2=metaData2.getVidFile().getPath().parent_path();
						//	if (parentPath1 != parentPath2)
						//	{
						//		std::cout << "parent path 1 : " << parentPath1 << "; parent path2 : " << parentPath2 << std::endl;
						//		return true;
						//	}
						//}
						//return false;
						return DefaultMatchCandidatePredicateImpl::operator()(metaData1, metaData2) && 
							metaData1.getVidFile().getPath().parent_path() != metaData2.getVidFile().getPath().parent_path();
					}

				};


				static const MatchCandidatePredicateImpl* const getImpl(const boost::property_tree::ptree& properties)
				{
					if (properties.get<std::string>("matchCandidatePredicateType", "default") == "partitioned")
					{
						return new PartitionedMatchCandidatePredicateImpl(properties);
					}
					return new DefaultMatchCandidatePredicateImpl(properties);
				}

				const MatchCandidatePredicateImpl* const impl;
			public:

				MatchCandidatePredicate(const boost::property_tree::ptree& properties):
					impl(getImpl(properties))
				{}

				bool operator()(const VideoMetaData& metaData1,const VideoMetaData& metaData2) const noexcept
				{
					return impl->operator()(metaData1,metaData2);
				}
				~MatchCandidatePredicate()
				{
					delete impl;
				}
			};


			template<feature2D::featureDescription::DescriptorType descType>
			class SigMatchSession
			{
			protected:
				friend class boost::serialization::access;

				VideoSigCache<descType>* cache;

				size_t numThreads;

				SigMatchFunction<descType>* sigMatchFunction;

				std::mutex candidateGetMut;

				boost::filesystem::path matchFile;

				std::chrono::system_clock::time_point lastWriteTime;

				std::chrono::milliseconds writeInterval;

				size_t maxNumMatches;

				double maxSimilarity;

				bool matchesToWrite;

				std::multiset<SigMatch> matches;

				std::mutex matchPutMut;

				std::vector<std::pair<VideoMetaData,VideoMetaData>> candidateContainer;

				typename std::vector<std::pair<VideoMetaData,VideoMetaData>>::const_iterator candidateItr;
				
				typename std::vector<std::pair<VideoMetaData,VideoMetaData>>::const_iterator candidateEnd;

				template<class Archive>
				constexpr void save(Archive& archive,const unsigned int version) const
				{
					archive & maxNumMatches;
					archive & maxSimilarity;
					archive & matches.size();
					for(const auto& match:matches)
					{
						archive & match;
					}
				}

				template<class Archive>
				constexpr void load(Archive& archive,const unsigned int version)
				{
					archive & maxNumMatches;
					archive & maxSimilarity;
					size_t numMatches;
					archive & numMatches;
					matches.clear();
					for(;numMatches--;)
					{
						SigMatch match;
						archive & match;
						matches.emplace_hint(matches.end(),match);
					}
				}

				template<class Archive>
				constexpr void serialize(Archive &archive,const unsigned int version)
				{
					boost::serialization::split_member(archive,*this,version);
				}

				constexpr void addMatchNoCheck(const std::pair<VideoMetaData,VideoMetaData>& candidate,double similarity)
				{
					if(matches.size()<maxNumMatches)
					{
						matches.emplace(candidate,similarity);
					}
					else
					{
						matches.emplace(candidate,similarity);
						matches.erase(--matches.end());
						maxSimilarity=matches.rbegin()->similarity;
					}
					std::chrono::system_clock::time_point now=std::chrono::system_clock::now();
					if(now-lastWriteTime>=writeInterval)
					{
						writeToFile();
						writeToTextFile();
						lastWriteTime=now;
						matchesToWrite=false;
					}
					else
					{
						matchesToWrite=true;
					}
				}

				constexpr void tryWriteToFile()
				{
					std::chrono::system_clock::time_point now=std::chrono::system_clock::now();
					if(now-lastWriteTime>=writeInterval)
					{
						writeToFile();
						writeToTextFile();
						lastWriteTime=now;
						matchesToWrite=false;
					}
				}

				template<typename ret=std::pair<bool,double>>
				constexpr std::enable_if_t<std::is_convertible_v<std::pair<bool,double>,ret>,ret> accept(const std::pair<VideoMetaData,VideoMetaData>& candidate,double similarity)
				{
					std::lock_guard<std::mutex> lock(matchPutMut);
					if(similarity<=maxSimilarity)
					{
						addMatchNoCheck(candidate,similarity);
						return std::pair<bool,double>(true,maxSimilarity);
					}
					if(matchesToWrite)
					{
						tryWriteToFile();
					}
					return std::pair<bool,double>(false,maxSimilarity);
				}

				template<typename ret>
				constexpr std::enable_if_t<std::is_floating_point_v<ret>,ret> accept(const std::pair<VideoMetaData,VideoMetaData>& candidate,double similarity)
				{
					std::lock_guard<std::mutex> lock(matchPutMut);
					if(similarity<=maxSimilarity)
					{
						addMatchNoCheck(candidate,similarity);
					}
					else if(matchesToWrite)
					{
						tryWriteToFile();
					}
					return static_cast<ret>(maxSimilarity);
				}

				constexpr virtual void sigMatchThread()
				{
					for(;;)
					{
						const std::pair<VideoMetaData,VideoMetaData>* candidate;
						{
							std::lock_guard<std::mutex> lock(candidateGetMut);
							if(candidateItr==candidateEnd)
							{
								break;
							}
							candidate=&(*(candidateItr++));
						}
						const std::pair<VideoMetaData,VideoMetaData>& candidateRef=*candidate;
						maxSimilarity=accept<double>(candidateRef,sigMatchFunction->getSigSimilarity(cache->getMatchPairFromMetaDatas(candidateRef),maxSimilarity));
					}
				}

				static constexpr size_t getNumDigits(size_t val) noexcept
				{
					size_t numDigits=1;
					while(val>=10)
					{
						val/=10;
						++numDigits;
					}
					return numDigits;
				}

				static constexpr std::string getFolderNum(std::string str,size_t totalNumDigits) noexcept
				{
					if(str.size()<totalNumDigits)
					{
						return std::string(totalNumDigits-str.size(),'0')+str;
					}
					return str;
				}

			public:

				SigMatchSession(VideoSigCache<descType>& cache,const boost::property_tree::ptree& properties)
					:cache(&cache)
					,numThreads(getNumThreadsProperty(properties))
					,sigMatchFunction(getSigMatchFunction<descType>(properties))
					,candidateGetMut()
					,matchFile(properties.get<std::string>("matchFile"))
					,lastWriteTime(std::chrono::system_clock::now())
					,writeInterval(properties.get<size_t>("writeIntervalMillis",5000))
					,maxNumMatches(properties.get<size_t>("maxNumMatches",std::numeric_limits<size_t>::max()))
					,maxSimilarity(properties.get<double>("maxSimilarity",std::numeric_limits<double>::infinity()))
					,matchesToWrite(false)
					,matches()
					,matchPutMut()
					,candidateContainer(cache.getMatchCandidates<MatchCandidatePredicate,std::vector<std::pair<VideoMetaData,VideoMetaData>>>(properties,numThreads))
					,candidateItr(candidateContainer.begin())
					,candidateEnd(candidateContainer.end())
				{
					std::sort(candidateContainer.begin(),candidateContainer.end(),CandidateSorter(properties));
				}

				constexpr void requestShutdown()
				{
					std::lock_guard<std::mutex> lock(candidateGetMut);
					candidateItr=candidateEnd;
				}

				constexpr void writeToFile(const boost::filesystem::path& file) const
				{
					boost::filesystem::create_directories(file.parent_path());
					boost::filesystem::path tmp=file.string()+".tmp";
					boost::filesystem::ofstream matchOut(tmp,std::ios::binary|std::ios::out);
					if(matchOut.is_open())
					{
						{
							boost::archive::binary_oarchive matchArchive(matchOut);
							matchArchive & *this;
						}
						matchOut.close();
						boost::filesystem::rename(tmp,file);
						return;
					}
					CV_Error(1,"Unable to open "+tmp.string()+" for writing");
				}

				constexpr void readFromFile(const boost::filesystem::path& file)
				{
					if(boost::filesystem::exists(file))
					{
						boost::filesystem::ifstream matchIn(file,std::ios::binary|std::ios::in);
						if(matchIn.is_open())
						{
							{
								boost::archive::binary_iarchive matchArchive(matchIn);
								matchArchive & *this;
							}
							matchIn.close();
							return;
						}
						CV_Error(1,file.string()+" could not be opened");
					}
					CV_Error(1,file.string()+" does not exist");
				}

				constexpr void readFromFile()
				{
					readFromFile(matchFile.wstring()+L".ser");
				}

				constexpr void writeToFile() const
				{
					writeToFile(matchFile.wstring()+L".ser");
				}

				constexpr void writeToTextFile(const boost::filesystem::path& file) const
				{
					boost::filesystem::create_directories(file.parent_path());
					boost::filesystem::path tmpPath=(file.wstring()+L".tmp");
					std::ofstream outStream;
					outStream.open(tmpPath.string());
					for(const auto& match:matches)
					{
						outStream<<match.vid1.getVidFile().getFilePathStr()<<std::endl;
						outStream<<match.vid2.getVidFile().getFilePathStr()<<std::endl;
						outStream<<match.similarity<<std::endl;
						//outStream<<std::endl;
					}
					outStream.close();
					boost::filesystem::rename(tmpPath,file);
				}

				constexpr void writeToTextFile() const
				{
					writeToTextFile(matchFile.wstring()+L".txt");
				}

				constexpr DisjointSet<VideoMetaData> getMatchDisjointSet() const noexcept
				{
					DisjointSet<VideoMetaData> disjointSet;
					for(const SigMatch& sigMatch:matches)
					{
						disjointSet.makeUnion(sigMatch.vid1,sigMatch.vid2);
					}
					return disjointSet;
				}

				constexpr std::vector<std::unordered_set<VideoMetaData>> getSortedMatchGroups() const noexcept
				{
					DisjointSet<VideoMetaData> disjointSet=getMatchDisjointSet();
					std::unordered_set<SigMatch,SigMatchHash,SigMatchEquals> sigPairSet(matches.begin(),matches.end());
					std::vector<std::unordered_set<VideoMetaData>> vectorView=disjointSet.getVectorView();
					std::multimap<double,std::unordered_set<VideoMetaData>> sortedGroups;
					for(const std::unordered_set<VideoMetaData>& set:vectorView)
					{
						double avg=0.0;
						size_t numMatches=0;
						for(auto itr1=set.begin();itr1!=set.end();++itr1)
						{
							const VideoMetaData& vid1=*itr1;
							for(auto itr2=set.begin();itr2!=set.end();++itr2)
							{
								if(itr1!=itr2)
								{
									auto itr3=sigPairSet.find(SigMatch(std::pair<VideoMetaData,VideoMetaData>(vid1,*itr2),0.0));
									if(itr3!=sigPairSet.end())
									{
										++numMatches;
										avg+=itr3->similarity;
									}
								}
							}
						}
						if(numMatches!=0)
						{
							avg/=numMatches;
						}
						else
						{
							avg=std::numeric_limits<double>::infinity();
						}
						sortedGroups.emplace(avg,set);
					}
					std::vector<std::unordered_set<VideoMetaData>> sortedVectorView;
					sortedVectorView.reserve(sortedGroups.size());
					for(auto itr=sortedGroups.begin();itr!=sortedGroups.end();++itr)
					{
						sortedVectorView.emplace_back(itr->second);
					}
					return sortedVectorView;
				}

				template<class VidFileContainer>
				constexpr void moveMatches(const VidFileContainer& vidFileContainer,const boost::filesystem::path& rootDir) const
				{
					const auto& vectorView=getSortedMatchGroups();
					size_t numDigits=getNumDigits(vectorView.size());
					size_t folderNum=0;
					for(const auto& set:vectorView)
					{
						std::string folderName=std::to_string(folderNum);
						boost::filesystem::path dir=rootDir/boost::filesystem::path(getFolderNum(std::to_string(folderNum++),numDigits));
						boost::filesystem::create_directories(dir);
						for(const auto& metaData:set)
						{
							const fileManagement::FileInfo& cacheVidFile=metaData.getVidFile();
							auto itr=std::find(vidFileContainer.begin(),vidFileContainer.end(),cacheVidFile);
							if(itr!=vidFileContainer.end())
							{
								const fileManagement::FileInfo& vidFile=*itr;
								if(vidFile.pathExists())
								{
									const boost::filesystem::path& vidPath=vidFile.getPath();
									boost::filesystem::path dst=dir/vidFile.getPath().filename();
									if(boost::filesystem::exists(dst))
									{
										const std::string fullPath=fileManagement::FileInfo(dst).getFilePathStr();
										size_t index=fullPath.find_last_of(".");
										const std::string rawPath=fullPath.substr(0,index);
										const std::string extension=fullPath.substr(index+1);
										size_t num=1;
										for(;;)
										{
											if(!boost::filesystem::exists(dst=boost::filesystem::path(rawPath+"("+std::to_string(num++)+")."+extension)))
											{
												break;
											}
										}
									}
									std::cout<<"Moving "<<vidPath<<" to "<<dst<<std::endl;
									boost::filesystem::rename(vidPath,dst);
								}
							}

						}
					}
				}

				constexpr void generateMatches()
				{
					std::vector<std::thread> threads;
					size_t numThreads=this->numThreads;
					threads.reserve(--numThreads);
					for(;numThreads--;)
					{
						threads.emplace_back(&SigMatchSession<descType>::sigMatchThread,this);
					}
					sigMatchThread();
					for(auto& thread:threads)
					{
						thread.join();
					}
					writeToFile();
					writeToTextFile();
				}

				inline ~SigMatchSession() noexcept
				{
					delete sigMatchFunction;
				}

			};

			template<feature2D::featureDescription::DescriptorType descType>
			class VerboseSigMatchSession: public SigMatchSession<descType>
			{
			protected:
				constexpr size_t countFileSize() noexcept
				{
					size_t fileSize=0;
					for(auto fileSizeCountItr=candidateItr;fileSizeCountItr!=candidateEnd;++fileSizeCountItr)
					{
						fileSize+=cache->getKeyFrameFileSize(*fileSizeCountItr);
					}
					return fileSize;
				}
				std::mutex reportMut;
				size_t finishIndex;
				size_t totalNumMatches;
				size_t consumedFileSize;
				size_t totalFileSize;
				std::chrono::system_clock::time_point sessionStartTime;
				std::chrono::system_clock::time_point lastReportTime;
				std::chrono::milliseconds reportInterval;

				constexpr void reportMatch(const std::pair<VideoMetaData,VideoMetaData>& candidate,double similarity) noexcept
				{
					const std::chrono::system_clock::time_point& finishTime=std::chrono::system_clock::now();
					const size_t matchFileSize=cache->getKeyFrameFileSize(candidate);
					std::stringstream ss;
					ss<<"MATCH"<<std::endl
						<<"\tVid1       = "<<candidate.first.getVidFile().getFilePathStr()<<std::endl
						<<"\tVid2       = "<<candidate.second.getVidFile().getFilePathStr()<<std::endl
						<<"\tSimilarity = "<<similarity<<std::endl;
					std::lock_guard<std::mutex> lock(reportMut);
					if(finishTime-lastReportTime>=reportInterval)
					{

						const double elapsedMilliseconds=double(std::chrono::duration_cast<std::chrono::milliseconds>(finishTime-sessionStartTime).count());
						const double bytesPerSecond=double(consumedFileSize+=matchFileSize)/(elapsedMilliseconds/1000.0);
						std::cout<<ss.str()
							<<"Progress            = "<<(++finishIndex)<<" / "<<totalNumMatches<<std::endl
							<<"Data Rate           = "<<(bytesPerSecond/(1024.0*1024.0))<<"MB/s"<<std::endl
							<<"Total Elapsed Time  = "<<DurationToString(elapsedMilliseconds)<<std::endl
							<<"Estimated Time Left = "<<DurationToString(totalFileSize,consumedFileSize,bytesPerSecond)<<std::endl;
						lastReportTime=finishTime;
					}
					else
					{
						consumedFileSize+=matchFileSize;
						++finishIndex;
						std::cout<<ss.str();
					}
				}

				constexpr void reportNoMatch(const std::pair<VideoMetaData,VideoMetaData>& candidate) noexcept
				{
					const std::chrono::system_clock::time_point& finishTime=std::chrono::system_clock::now();
					const size_t matchFileSize=cache->getKeyFrameFileSize(candidate);
					std::lock_guard<std::mutex> lock(reportMut);
					if(finishTime-lastReportTime>=reportInterval)
					{
						const double elapsedMilliseconds=double(std::chrono::duration_cast<std::chrono::milliseconds>(finishTime-sessionStartTime).count());
						const double bytesPerSecond=double(consumedFileSize+=matchFileSize)/(elapsedMilliseconds/1000.0);
						std::cout
							<<"Progress            = "<<(++finishIndex)<<" / "<<totalNumMatches<<std::endl
							<<"Data Rate           = "<<(bytesPerSecond/(1024.0*1024.0))<<"MB/s"<<std::endl
							<<"Total Elapsed Time  = "<<DurationToString(elapsedMilliseconds)<<std::endl
							<<"Estimated Time Left = "<<DurationToString(totalFileSize,consumedFileSize,bytesPerSecond)<<std::endl;
						lastReportTime=finishTime;
					}
					else
					{
						consumedFileSize+=matchFileSize;
						++finishIndex;
					}
				}

				constexpr void sigMatchThread() override
				{
					for(;;)
					{
						const std::pair<VideoMetaData,VideoMetaData>* candidate;
						{
							std::lock_guard<std::mutex> lock(candidateGetMut);
							if(candidateItr==candidateEnd)
							{
								break;
							}
							candidate=&(*(candidateItr++));
						}
						const std::pair<VideoMetaData,VideoMetaData>& candidateRef=*candidate;
						const double similarity=sigMatchFunction->getSigSimilarity(cache->getMatchPairFromMetaDatas(candidateRef),maxSimilarity);
						const std::pair<bool,double>& resultPair=accept<std::pair<bool,double>>(candidateRef,similarity);
						maxSimilarity=resultPair.second;
						if(resultPair.first)
						{
							reportMatch(candidateRef,similarity);
						}
						else
						{
							reportNoMatch(candidateRef);
						}
					}
				}


			public:
				VerboseSigMatchSession(VideoSigCache<descType>& cache,const boost::property_tree::ptree& properties)
					:SigMatchSession<descType>(cache,properties)
					,reportMut()
					,finishIndex(0)
					,totalNumMatches(candidateContainer.size())
					,consumedFileSize(0)
					,totalFileSize(countFileSize())
					,sessionStartTime(std::chrono::system_clock::now())
					,lastReportTime(std::chrono::system_clock::now())
					,reportInterval(properties.get<size_t>("reportIntervalMillis",5000))
				{

				}
			};

			template<feature2D::featureDescription::DescriptorType descType>
			constexpr SigMatchSession<descType>* getSigMatchSession(VideoSigCache<descType>& cache,const boost::property_tree::ptree& properties)
			{
				SigMatchSession<descType>* matchSession;
				if(properties.get<bool>("verbose",true))
				{
					matchSession=new VerboseSigMatchSession<descType>(cache,properties);
				}
				else
				{
					matchSession=new SigMatchSession<descType>(cache,properties);
				}
				if(properties.get<std::string>("sorterType","default")=="durationDiff")
				{
					cache.setMaxNumSigs(std::numeric_limits<size_t>::max());
				}
				return matchSession;
			}
		}
	}
}