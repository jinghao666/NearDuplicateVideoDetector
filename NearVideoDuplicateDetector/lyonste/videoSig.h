#pragma once
#include "fileManagement.h"
#include "BlackAndWhiteList.h"

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
					+ 31 * (metaData->cols
						+ 31 * (metaData->rows
							+ 31 * (std::hash<double>()(metaData->numFrames)
								+ 31 * (std::hash<double>()(metaData->fps)
									+ 31 * (std::hash<double>()(metaData->duration))))));
			}

			template<class Archive>
			constexpr void save(Archive& archive, const unsigned int version) const
			{
				archive & vidFile;
				archive & cols;
				archive & rows;
				archive & numFrames;
				archive & fps;
				archive & duration;
			}

			template<class Archive>
			constexpr void load(Archive& archive, const unsigned int version)
			{
				archive & vidFile;
				archive & cols;
				archive & rows;
				archive & numFrames;
				archive & fps;
				archive & duration;
				hash = computeHash(this);
			}

			template<class Archive>
			constexpr void serialize(Archive &archive, const unsigned int version)
			{
				boost::serialization::split_member(archive, *this, version);
			}

		public:

			VideoMetaData(const VideoMetaData& that) noexcept
				: vidFile(that.vidFile)
				, cols(that.cols)
				, rows(that.rows)
				, numFrames(that.numFrames)
				, fps(that.fps)
				, duration(that.duration)
				, hash(that.hash)
			{
			}

			VideoMetaData(VideoMetaData&& that) noexcept
				:vidFile(std::move(that.vidFile))
				, cols(that.cols)
				, rows(that.rows)
				, numFrames(that.numFrames)
				, fps(that.fps)
				, duration(that.duration)
				, hash(that.hash)
			{

			}

			VideoMetaData& operator=(const VideoMetaData& that) noexcept
			{
				if (this != &that)
				{
					vidFile = that.vidFile;
					cols = that.cols;
					rows = that.rows;
					numFrames = that.numFrames;
					fps = that.fps;
					duration = that.duration;
					hash = that.hash;
				}
				return *this;
			}
			
			VideoMetaData& operator=(VideoMetaData&& that) noexcept
			{
				if (this != &that)
				{
					vidFile = std::move(that.vidFile);
					cols = that.cols;
					rows = that.rows;
					numFrames = that.numFrames;
					fps = that.fps;
					duration = that.duration;
					hash = that.hash;
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

			inline VideoMetaData(const fileManagement::FileInfo& vidFile, size_t cols, size_t rows, double numFrames, double fps, double duration) noexcept
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
				return this == &that || (vidFile == that.vidFile && cols == that.cols && rows == that.rows && numFrames == that.numFrames && fps == that.fps && duration == that.duration);
			}

			constexpr bool operator!=(const VideoMetaData& that) const noexcept
			{
				return !(*this == that);
			}

			constexpr bool operator<(const VideoMetaData& that) const noexcept
			{
				return duration < that.duration || (duration == that.duration&&vidFile < that.vidFile);
			}

			constexpr bool operator>(const VideoMetaData& that) const noexcept
			{
				return that < *this;
			}

			constexpr bool operator<=(const VideoMetaData& that) const noexcept
			{
				return !(that < *this);
			}

			constexpr bool operator>=(const VideoMetaData& that) const noexcept
			{
				return !(*this < that);
			}
		};
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
namespace lyonste
{
	namespace videoSig
	{
		namespace videoSigGen
		{
			class SigGenFunction
			{
			public:
				virtual std::tuple<VideoMetaData, fileManagement::FileInfo, size_t> generateSig(const fileManagement::FileInfo& vidFile) const = 0;
				virtual void writeCacheToFile() const = 0;
				virtual size_t getNumSigsInCache() const noexcept = 0;
				virtual ~SigGenFunction()
				{
				}
			};
			class SigGenSession
			{
			public:
				//virtual size_t getTotalNumVids() const noexcept = 0;
				//virtual size_t getTotalFileSize() const noexcept = 0;
				//virtual size_t getNumSigGeneratedSoFar() const noexcept = 0;
				//virtual size_t getConsumedDataSize() const noexcept = 0;
				//virtual std::chrono::system_clock::time_point getSessionStartTime() const noexcept = 0;

				virtual const SigGenFunction* getSigGenFunction() const noexcept = 0;
				virtual size_t getNumThreads() const noexcept = 0;
				virtual void requestShutdown() = 0;
				virtual size_t getNumSigsInCache() const noexcept = 0;
				virtual void generateSigs() = 0;
				virtual ~SigGenSession()
				{
				}
			};
		}

		namespace videoSigMatch
		{
			class SigMatchFunction
			{
			public:
				virtual size_t getKeyFrameFileSize(const std::pair<VideoMetaData, VideoMetaData>& pair) const noexcept = 0;
				virtual double getSigSimilarity(const std::pair<VideoMetaData, VideoMetaData>& candidate, double similarityThreshold) const = 0;
				virtual bool cacheContains(const VideoMetaData& metaData) const noexcept = 0;
				virtual ~SigMatchFunction()
				{
				}
			};
			class SigMatchSession
			{
			public:
				virtual const SigMatchFunction* getSigMatchFunction() const noexcept = 0;
				virtual size_t getNumThreads() const noexcept = 0;
				virtual void requestShutdown() const = 0;
				virtual size_t getNumSigsInCache() const noexcept = 0;
				virtual void generateMatches() = 0;
				virtual ~SigMatchSession()
				{

				}
			};
			struct SigMatch
			{
				VideoMetaData vid1;
				VideoMetaData vid2;
				double similarity;

				template<class Archive>
				constexpr void serialize(Archive &archive, const unsigned int version)
				{
					archive & vid1;
					archive & vid2;
					archive & similarity;
				}

				inline SigMatch() noexcept
				{}

				inline SigMatch(const std::pair<VideoMetaData, VideoMetaData>& pair) noexcept
					:vid1(pair.first),
					vid2(pair.second),
					similarity(1)
				{

				}

				inline SigMatch(const std::pair<VideoMetaData, VideoMetaData>& pair, double similarity) noexcept
					:vid1(pair.first),
					vid2(pair.second),
					similarity(similarity)
				{}

				constexpr bool operator<(const SigMatch& that) const noexcept
				{
					return similarity<that.similarity
						|| (similarity == that.similarity
							&& ((vid1.getVidFile().getFileSize() + vid2.getVidFile().getFileSize())
					>(that.vid1.getVidFile().getFileSize() + that.vid2.getVidFile().getFileSize())));


				}
			};

			struct SigMatchEquals
			{
				constexpr bool operator()(const SigMatch& match1, const SigMatch& match2) const noexcept
				{
					return &match1 == &match2 || (match1.vid1 == match2.vid1&&match1.vid2 == match2.vid2);
				}
			};

			struct SigMatchHash
			{
				constexpr size_t operator()(const SigMatch& match) const noexcept
				{
					return std::hash<VideoMetaData>()(match.vid1) * 31 + std::hash<VideoMetaData>()(match.vid2);
				}
			};




		}
		
		class VideoSigCache
		{
		protected:

		public:
			

			

			


			virtual videoSigMatch::SigMatchFunction* getSigMatchFunction(const boost::property_tree::ptree& properties) = 0;

			virtual videoSigGen::SigGenFunction* getSigGenFunction(const boost::property_tree::ptree& properties) = 0;

			//virtual videoSigMatch::SigMatchSession* getSigMatchSession(const boost::property_tree::ptree& properties, bool incremental = false) = 0;

			//virtual videoSigGen::SigGenSession* getSigGenSession(const boost::property_tree::ptree& properties)

			virtual size_t size() const noexcept = 0;

			//virtual size_t max_size() const noexcept = 0;

			//virtual std::unique_ptr<VideoSigCacheIterator> begin() const noexcept=0;
			//virtual std::unique_ptr<VideoSigCacheIterator> end() const noexcept = 0;
			//std::unique_ptr<VideoSigCacheIterator> cbegin() const noexcept { return begin(); }
			//std::unique_ptr<VideoSigCacheIterator> cend() const noexcept { return end(); }

			bool empty() const noexcept
			{
				return size() == 0;
			}

			//TODO provide begin and end iterators for video metadata. Make this just like an STL container

			virtual void writeToFile() const = 0;

			virtual bool contains(const VideoMetaData& metaData) const noexcept = 0;

			//TODO find a replacement for the gatMatchCandidates method.
			//Perhaps return metadatas in a container and process them that way

			//TODO I don't like that this returns a specific container type. Find a way to generalize it.
			virtual std::vector<std::pair<VideoMetaData, VideoMetaData>> getMatchCandidates(const boost::property_tree::ptree& properties, std::unordered_set<videoSigMatch::SigMatch, videoSigMatch::SigMatchHash, videoSigMatch::SigMatchEquals>& existingMatches = std::unordered_set<videoSigMatch::SigMatch, videoSigMatch::SigMatchHash, videoSigMatch::SigMatchEquals>()) noexcept = 0;

			//TODO I don't like that these have separate implementations. Find a way to consolidate them.
			virtual void pruneCache(std::set<fileManagement::FileInfo>& vidFileContainer) noexcept = 0;
			
			virtual void pruneCache(std::vector<fileManagement::FileInfo>& vidFileContainer) noexcept=0;

			virtual ~VideoSigCache()
			{
			}
		};

		VideoSigCache* getVideoSigCache(const boost::property_tree::ptree& properties);
	}
}
