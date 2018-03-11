#pragma once

#include "videoSig.h"
#include <boost/serialization/split_member.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

namespace lyonste
{
	namespace videoSig
	{
		namespace videoSigMatch
		{
			

			class SigMatchSessionImpl
			{
			protected:
				friend class boost::serialization::access;

				size_t numThreads;

				SigMatchFunction* sigMatchFunction;

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

				void tryWriteToFile()
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
					if (matches.size() < maxNumMatches)
					{
						matches.emplace(candidate, similarity);
						matchesToWrite = true;
						tryWriteToFile();
						return std::pair<bool, double>(true, maxSimilarity);
					}
					else if (similarity < maxSimilarity)
					{
						matches.emplace(candidate, similarity);
						matches.erase(--matches.end());
						maxSimilarity = matches.rbegin()->similarity;
						matchesToWrite = true;
						tryWriteToFile();
						return std::pair<bool, double>(true, maxSimilarity);
					}
					else if (matchesToWrite)
					{
						tryWriteToFile();
					}
					return std::pair<bool,double>(false,maxSimilarity);
				}

				template<typename ret>
				constexpr std::enable_if_t<std::is_floating_point_v<ret>,ret> accept(const std::pair<VideoMetaData,VideoMetaData>& candidate,double similarity)
				{
					std::lock_guard<std::mutex> lock(matchPutMut);
					if (matches.size() < maxNumMatches)
					{
						matches.emplace(candidate, similarity);
						matchesToWrite = true;
						tryWriteToFile();
					}
					else if (similarity < maxSimilarity)
					{
						matches.emplace(candidate, similarity);
						matches.erase(--matches.end());
						maxSimilarity = matches.rbegin()->similarity;
						matchesToWrite = true;
						tryWriteToFile();
					}
					else if (matchesToWrite)
					{
						tryWriteToFile();
					}
					return static_cast<ret>(maxSimilarity);
				}

				virtual void sigMatchThread()
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
						maxSimilarity=accept<double>(candidateRef,sigMatchFunction->getSigSimilarity(candidateRef,maxSimilarity));
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

				static std::string getFolderNum(std::string str,size_t totalNumDigits) noexcept
				{
					if(str.size()<totalNumDigits)
					{
						return std::string(totalNumDigits-str.size(),'0')+str;
					}
					return str;
				}

				void pruneMatches()
				{
					//std::cout << "Pruning matchings. Before size: " << matches.size() << std::endl;
					std::cout << "Pruning matches" << std::endl;
					size_t numPrunedMatches = 0;
					for (auto itr = matches.begin(); itr != matches.end();)
					{
						if(!sigMatchFunction->cacheContains(itr->vid1) || !sigMatchFunction->cacheContains(itr->vid2))
						{
							++numPrunedMatches;
							itr = matches.erase(itr);
						}
						else
						{
							++itr;
						}
					}
					std::cout << "Pruned " << numPrunedMatches << " matches"<<std::endl;
					//std::cout << "Done pruning matches. After size: " << matches.size() << std::endl;
				}

			public:

				size_t getNumMatches() const noexcept
				{
					return matches.size();
				}

				size_t getNumCandidates() const noexcept
				{
					return candidateContainer.size();
				}

				void requestShutdown()
				{
					std::lock_guard<std::mutex> lock(candidateGetMut);
					candidateItr=candidateEnd;
				}

				void writeToFile(const boost::filesystem::path& file) const
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

				void readFromFile(const boost::filesystem::path& file)
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

				void readFromFile()
				{
					readFromFile(matchFile.wstring()+L".ser");
				}

				void writeToFile() const
				{
					writeToFile(matchFile.wstring()+L".ser");
				}

				void writeToTextFile(const boost::filesystem::path& file) const
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
					}
					outStream.close();
					boost::filesystem::rename(tmpPath,file);
				}

				void writeToTextFile() const
				{
					writeToTextFile(matchFile.wstring()+L".txt");
				}

				DisjointSet<VideoMetaData> getMatchDisjointSet(size_t maxNumMatches = std::numeric_limits<size_t>::max(),const MutualMap<fileManagement::FileInfo>& blackList=MutualMap<fileManagement::FileInfo>()) const noexcept
				{
					DisjointSet<VideoMetaData> disjointSet;
					size_t numMatches = 0;
					for(const SigMatch& sigMatch:matches)
					{
						const VideoMetaData& vid1 = sigMatch.vid1;
						const VideoMetaData& vid2 = sigMatch.vid2;
						const fileManagement::FileInfo& vidFile1 = vid1.getVidFile();
						const fileManagement::FileInfo& vidFile2 = vid2.getVidFile();
						if (vidFile1.pathExists() && vidFile2.pathExists() && !blackList.contains(vidFile1,vidFile2))
						{
							disjointSet.join(vid1, vid2);
							if (++numMatches >= maxNumMatches)
							{
								break;
							}
						}
						
					}
					return disjointSet;
				}

				std::vector<std::unordered_set<VideoMetaData>> getSortedMatchGroups(size_t maxNumMatches=std::numeric_limits<size_t>::max(), const MutualMap<fileManagement::FileInfo>& blackList = MutualMap<fileManagement::FileInfo>()) const noexcept
				{
					DisjointSet<VideoMetaData> disjointSet=getMatchDisjointSet(maxNumMatches,blackList);
					std::unordered_set<SigMatch,SigMatchHash,SigMatchEquals> sigPairSet(matches.begin(),matches.end());
					std::vector<std::unordered_set<VideoMetaData>> vectorView=disjointSet.getVectorView();
					std::multimap<double,std::unordered_set<VideoMetaData>> sortedGroups;
					for(const std::unordered_set<VideoMetaData>& set:vectorView)
					{
						double bestVal = std::numeric_limits<double>::infinity();
						for (auto itr1 = set.begin(); itr1 != set.end(); ++itr1)
						{
							const VideoMetaData& vid1 = *itr1;
							for (auto itr2 = set.begin(); itr2 != set.end(); ++itr2)
							{
								if (itr1 != itr2)
								{
									auto itr3 = sigPairSet.find(SigMatch(std::pair<VideoMetaData,VideoMetaData>(vid1, *itr2), 0.0));
									if (itr3 != sigPairSet.end())
									{
										double similarity = itr3->similarity;
										if (similarity < bestVal)
										{
											bestVal = similarity;
										}
									}

								}
							}
						}
						sortedGroups.emplace(bestVal, set);
						//double avg=0.0;
						//size_t numMatches=0;
						//for(auto itr1=set.begin();itr1!=set.end();++itr1)
						//{
						//	const VideoMetaData& vid1=*itr1;
						//	for(auto itr2=set.begin();itr2!=set.end();++itr2)
						//	{
						//		if(itr1!=itr2)
						//		{
						//			auto itr3=sigPairSet.find(SigMatch(std::pair<VideoMetaData,VideoMetaData>(vid1,*itr2),0.0));
						//			if(itr3!=sigPairSet.end())
						//			{
						//				++numMatches;
						//				avg+=itr3->similarity;
						//			}
						//		}
						//	}
						//}
						//if(numMatches!=0)
						//{
						//	avg/=numMatches;
						//}
						//else
						//{
						//	avg=std::numeric_limits<double>::infinity();
						//}
						//sortedGroups.emplace(avg,set);
					}
					std::vector<std::unordered_set<VideoMetaData>> sortedVectorView;
					sortedVectorView.reserve(sortedGroups.size());
					for(auto itr=sortedGroups.begin();itr!=sortedGroups.end();++itr)
					{
						sortedVectorView.emplace_back(itr->second);
					}
					return sortedVectorView;
				}

				void moveMatches(const boost::filesystem::path& rootDir, size_t maxNumMatches = std::numeric_limits<size_t>::max(),  size_t maxGroupSize= std::numeric_limits<size_t>::max(),const MutualMap<fileManagement::FileInfo>& blackList = MutualMap<fileManagement::FileInfo>()) const
				{
					const auto& vectorView=getSortedMatchGroups(maxNumMatches,blackList);
					size_t numDigits=getNumDigits(vectorView.size());
					size_t folderNum=0;
					for(const auto& set:vectorView)
					{
						if (set.size() > maxGroupSize)
						{
							continue;
						}
						std::string folderName=std::to_string(folderNum);
						boost::filesystem::path dir=rootDir/boost::filesystem::path(getFolderNum(std::to_string(folderNum++),numDigits));
						boost::filesystem::create_directories(dir);
						for(const auto& metaData:set)
						{
							const fileManagement::FileInfo& vidFile=metaData.getVidFile();
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

				void generateMatches()
				{
					//std::cout << "Beginning matching process. Processing " << candidateContainer.size() << " matches." << std::endl;
					std::vector<std::thread> threads;
					size_t numThreads=this->numThreads;
					threads.reserve(--numThreads);
					for(;numThreads--;)
					{
						threads.emplace_back(&SigMatchSessionImpl::sigMatchThread,this);
					}
					sigMatchThread();
					for(auto& thread:threads)
					{
						thread.join();
					}
					writeToFile();
					writeToTextFile();
					//std::cout << "Finished generating matches. Total number of matches on file = " << matches.size() << std::endl;
				}

				inline ~SigMatchSessionImpl() noexcept
				{
					delete sigMatchFunction;
				}

				SigMatchSessionImpl(VideoSigCache* cache, const boost::property_tree::ptree& properties, bool incremental = false)
					: numThreads(getNumThreadsProperty(properties))
					, sigMatchFunction(cache->getSigMatchFunction(properties))
					, candidateGetMut()
					, matchFile(properties.get<std::string>("matchFile"))
					, lastWriteTime(std::chrono::system_clock::now())
					, writeInterval(properties.get<size_t>("writeIntervalMillis", 5000))
					, maxNumMatches(properties.get<size_t>("maxNumMatches", std::numeric_limits<size_t>::max()))
					, maxSimilarity(properties.get<double>("maxSimilarity", std::numeric_limits<double>::infinity()))
					, matchesToWrite(false)
					, matches()
					, matchPutMut()
				{
					if (incremental)
					{
						readFromFile();

						pruneMatches();
						writeToFile();
						
						candidateContainer = cache->getMatchCandidates(properties, std::unordered_set<SigMatch, SigMatchHash, SigMatchEquals>(matches.begin(), matches.end()));
					}
					else
					{
						candidateContainer = cache->getMatchCandidates(properties);
					}
					candidateItr = candidateContainer.begin();
					candidateEnd = candidateContainer.end();
				}
			};

			class VerboseSigMatchSessionImpl: public SigMatchSessionImpl
			{
			protected:
				size_t countFileSize() noexcept
				{
					size_t fileSize=0;
					for(auto fileSizeCountItr=candidateItr;fileSizeCountItr!=candidateEnd;++fileSizeCountItr)
					{
						fileSize+=sigMatchFunction->getKeyFrameFileSize(*fileSizeCountItr);
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
				bool printProgress;
				bool printMatches;

				 void reportMatch(const std::pair<VideoMetaData,VideoMetaData>& candidate,double similarity) noexcept
				{
					const std::chrono::system_clock::time_point& finishTime=std::chrono::system_clock::now();
					const size_t matchFileSize= sigMatchFunction->getKeyFrameFileSize(candidate);
					std::stringstream ss;
					if (printMatches)
					{
						ss << "MATCH" << std::endl
							<< "\tVid1       = " << candidate.first.getVidFile().getFilePathStr() << std::endl
							<< "\tVid2       = " << candidate.second.getVidFile().getFilePathStr() << std::endl
							<< "\tSimilarity = " << similarity << std::endl;
					}
					
					std::lock_guard<std::mutex> lock(reportMut);
					if(printProgress && finishTime-lastReportTime>=reportInterval)
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

				 void reportNoMatch(const std::pair<VideoMetaData,VideoMetaData>& candidate) noexcept
				{
					const std::chrono::system_clock::time_point& finishTime=std::chrono::system_clock::now();
					const size_t matchFileSize= sigMatchFunction->getKeyFrameFileSize(candidate);
					std::lock_guard<std::mutex> lock(reportMut);
					if(printProgress && finishTime-lastReportTime>=reportInterval)
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

				void sigMatchThread() override
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
						const double similarity=sigMatchFunction->getSigSimilarity(candidateRef,maxSimilarity);
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
				VerboseSigMatchSessionImpl(VideoSigCache* cache,const boost::property_tree::ptree& properties,bool incremental=false)
					:SigMatchSessionImpl(cache,properties,incremental)
					,reportMut()
					,finishIndex(0)
					,totalNumMatches(candidateContainer.size())
					,consumedFileSize(0)
					,totalFileSize(countFileSize())
					,sessionStartTime(std::chrono::system_clock::now())
					,lastReportTime(std::chrono::system_clock::now())
					,reportInterval(properties.get<size_t>("reportIntervalMillis",5000))
					,printProgress(properties.get<bool>("printProgress",true))
					,printMatches(properties.get<bool>("printMatches",true))
				{

				}
			};

			SigMatchSessionImpl* getSigMatchSession(VideoSigCache* cache,const boost::property_tree::ptree& properties,bool incremental=false)
			{
				SigMatchSessionImpl* matchSession;
				if(properties.get<bool>("verbose",true))
				{
					matchSession=new VerboseSigMatchSessionImpl(cache,properties,incremental);
				}
				else
				{
					matchSession = new SigMatchSessionImpl(cache, properties, incremental);
				}
				return matchSession;
			}
		}
	}
}