#pragma once

#include <boost/serialization/split_member.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/property_tree/ptree.hpp>
#include <codecvt>
#define NOMINMAX
#ifdef _WIN32
#include <boost/algorithm/string/case_conv.hpp>
#endif
#ifdef BOOST_WINDOWS_API
#include <windows.h>
#include <winnt.h>
#include <fileapi.h>
#include <WinBase.h>
#endif
namespace lyonste
{
	namespace fileManagement
	{

		struct HandleWrapper
		{
			HANDLE handle;

			inline HandleWrapper(const boost::filesystem::path& p):
				handle(::CreateFileW(
					p.c_str()
					,0
					,FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE
					,0
					,OPEN_EXISTING
					,FILE_FLAG_BACKUP_SEMANTICS
					,0))
			{
			}
			inline ~HandleWrapper()
			{
				if(handle!=INVALID_HANDLE_VALUE)
				{
					::CloseHandle(handle);
				}
			}
		};

		class FileInfo
		{
		private:
			friend class boost::serialization::access;
			friend std::hash<FileInfo>;
			friend std::hash<FileInfo*>;
			friend std::equal_to<FileInfo*>;
			boost::filesystem::path path;
			DWORD volSNo;
			DWORD sizeLo;
			DWORD sizeHi;
			DWORD indxLo;
			DWORD indxHi;
			size_t hash;

			constexpr static size_t computeHash(const FileInfo* info) noexcept
			{
				return info->sizeHi
				+31*(info->sizeLo
				+31*(info->indxHi
				+31*(info->indxLo
				+31*(info->volSNo))));
			}
			static inline BY_HANDLE_FILE_INFORMATION getBHFI(const boost::filesystem::path& path)
			{
				HandleWrapper handleWrapper(path);
				if(handleWrapper.handle==INVALID_HANDLE_VALUE)
				{
					BOOST_THROW_EXCEPTION(boost::filesystem::filesystem_error("Invalid handle",path,boost::system::errc::make_error_code(boost::system::errc::bad_file_descriptor)));
				}
				BY_HANDLE_FILE_INFORMATION bhfi;
				if(!GetFileInformationByHandle(handleWrapper.handle,&bhfi))
				{
					BOOST_THROW_EXCEPTION(boost::filesystem::filesystem_error("Unable to get file info",path,boost::system::errc::make_error_code(boost::system::errc::bad_file_descriptor)));
				}
				return bhfi;
			}
			template<class Archive> constexpr void save(Archive& archive,const unsigned int version) const
			{
				archive & volSNo;
				archive & sizeLo;
				archive & sizeHi;
				archive & indxLo;
				archive & indxHi;
				archive & path.wstring();
			}
			template<class Archive> constexpr void load(Archive& archive,const unsigned int version)
			{
				archive & volSNo;
				archive & sizeLo;
				archive & sizeHi;
				archive & indxLo;
				archive & indxHi;
				std::wstring wStr;
				archive & wStr;
				path=wStr;
				hash=computeHash(this);
			}
			template<class Archive> constexpr void serialize(Archive &archive,const unsigned int version)
			{
				boost::serialization::split_member(archive,*this,version);
			}
			//delegate constructor
			inline FileInfo(const boost::filesystem::path& path,const BY_HANDLE_FILE_INFORMATION& bhfi)
				:path(path),
				volSNo(bhfi.dwVolumeSerialNumber),
				sizeLo(bhfi.nFileSizeLow),
				sizeHi(bhfi.nFileSizeHigh),
				indxLo(bhfi.nFileIndexLow),
				indxHi(bhfi.nFileIndexHigh),
				hash(computeHash(this))
			{
			}

		public:
			FileInfo(const FileInfo& that) noexcept
				:path(that.path)
				,volSNo(that.volSNo)
				,sizeLo(that.sizeLo)
				,sizeHi(that.sizeHi)
				,indxLo(that.indxLo)
				,indxHi(that.indxHi)
				,hash(that.hash)
			{
			}
			FileInfo(FileInfo&& that) noexcept
				:path(std::move(that.path))
				,volSNo(that.volSNo)
				,sizeLo(that.sizeLo)
				,sizeHi(that.sizeHi)
				,indxLo(that.indxLo)
				,indxHi(that.indxHi)
				,hash(that.hash)
			{}
			FileInfo& operator=(const FileInfo& that)
			{
				if(this!=&that)
				{
					path=that.path;
					volSNo=that.volSNo;
					sizeLo=that.sizeLo;
					sizeHi=that.sizeHi;
					indxLo=that.indxLo;
					indxHi=that.indxHi;
					hash=that.hash;
				}
				return *this;
			}
			FileInfo& operator=(FileInfo&& that)
			{
				if(this!=&that)
				{
					path=std::move(that.path);
					volSNo=that.volSNo;
					sizeLo=that.sizeLo;
					sizeHi=that.sizeHi;
					indxLo=that.indxLo;
					indxHi=that.indxHi;
					hash=that.hash;
				}
				return *this;
			}


			friend std::ostream& operator<<(std::ostream& stream,const FileInfo& info);

			inline void updatePath(const boost::filesystem::path& path) noexcept
			{
				this->path=path;
			}
			

			inline FileInfo() noexcept
			{}
			inline FileInfo(const boost::filesystem::path& path)
				:FileInfo(path,getBHFI(path))
			{
				
			}
			constexpr bool operator==(const FileInfo& that) const noexcept
			{
				return this==&that
			     || (volSNo==that.volSNo
				 && (indxLo==that.indxLo
				 && (indxHi==that.indxHi
				 && (sizeLo==that.sizeLo
				 && (sizeHi==that.sizeHi)))));
			}
			constexpr bool operator<(const FileInfo& that) const noexcept
			{
				return sizeHi<that.sizeHi||(sizeHi==that.sizeHi
					&&(sizeLo<that.sizeLo||(sizeLo==that.sizeLo
					&&(volSNo<that.volSNo||(volSNo==that.volSNo
					&&(indxHi<that.indxHi||(indxHi==that.indxHi
					&&(indxLo<that.indxLo))))))));
			}
			constexpr bool operator!=(const FileInfo& that) const noexcept
			{
				return !(*this==that);
			}
			constexpr bool operator>(const FileInfo& that) const noexcept
			{
				return that<*this;
			}
			constexpr bool operator<=(const FileInfo& that) const noexcept
			{
				return !(that<*this);
			}
			constexpr bool operator>=(const FileInfo& that) const noexcept
			{
				return !(*this<that);
			}
			constexpr const boost::filesystem::path& getPath() const noexcept
			{
				return path;
			}
			constexpr boost::filesystem::path& getPath() noexcept
			{
				return path;
			}
			constexpr size_t getFileSize() const noexcept
			{
				return (static_cast<size_t>(sizeHi)<<sizeof(DWORD)*8)|sizeLo;
			}
			inline bool pathExists() const noexcept
			{
				return boost::filesystem::exists(path);
			}
			inline bool pathExistsAndIsExpectedFileSize() const noexcept
			{
				return pathExists()&&boost::filesystem::file_size(path)==getFileSize();
			}
			inline std::string getFilePathStr() const noexcept
			{
				return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().to_bytes(path.wstring());
			}
			inline std::string getFileNameStr() const noexcept
			{
				return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().to_bytes(path.filename().wstring());
			}

			template<bool createDirectories=true>
			constexpr boost::filesystem::path resolveToDirectory(const boost::filesystem::path& dir,const std::wstring& suffix=L"") const
			{
				boost::filesystem::path resolvedFile(dir.wstring()+L'\\'+path.parent_path().lexically_relative(path.root_path()).wstring()+L'\\'+path.filename().wstring()+suffix);
				if(createDirectories)
				{
					boost::filesystem::create_directories(resolvedFile.parent_path());
				}
				return resolvedFile;
			}

		};

		extern std::ostream & operator<<(std::ostream & stream, const FileInfo & info);

		class FileGatherer
		{
		private:
			boost::filesystem::path root;
			size_t maxDepth;
			bool followSymlink;
			std::vector<boost::regex> includePatterns;
			std::vector<boost::regex> excludePatterns;

			static bool matchesOneOf(const std::string& path,const std::vector<boost::regex>& regexes)
			{
				for each(const boost::regex& regex in regexes)
				{
					if(regex_match(path,regex))
					{
						return true;
					}
				}
				return false;
			}

			static std::vector<boost::regex> createPatternVector(const std::string& key,const boost::property_tree::ptree& properties)
			{
				std::vector<boost::regex> patterns;
				for(auto itr=properties.find(key);itr!=properties.not_found()&&itr->first==key;++itr)
				{
#ifdef _WIN32
					patterns.emplace_back(boost::algorithm::to_lower_copy(itr->second.get_value<std::string>()));
#else
					patterns.emplace_back(itr->second.get_value<std::string>());
#endif
				}
				return patterns;
			}

		public:

			const boost::filesystem::path getRoot() const noexcept
			{
				return root;
			}

			FileGatherer(const boost::filesystem::path& root,size_t maxDepth,bool followSymlink,std::vector<boost::regex>&& includePatterns,std::vector<boost::regex>&& excludePatterns):
				root(root),
				maxDepth(maxDepth),
				followSymlink(followSymlink),
				includePatterns(std::move(includePatterns)),
				excludePatterns(std::move(excludePatterns))
			{}

			FileGatherer(boost::filesystem::path&& root,size_t maxDepth,bool followSymlink,std::vector<boost::regex>&& includePatterns,std::vector<boost::regex>&& excludePatterns):
				root(std::move(root)),
				maxDepth(maxDepth),
				followSymlink(followSymlink),
				includePatterns(std::move(includePatterns)),
				excludePatterns(std::move(excludePatterns))
			{}

			FileGatherer(const boost::property_tree::ptree& properties):
				FileGatherer(
					properties.get<std::string>("root"),
					properties.get<size_t>("maxDepth",std::numeric_limits<size_t>::max()),
					properties.get<bool>("followSymlink",false),
					createPatternVector("includePattern",properties),
					createPatternVector("excludePattern",properties)
				)
			{}


			template<class InsertionItr>
			constexpr void gatherFiles(InsertionItr insertionItr) const
			{
				boost::filesystem::recursive_directory_iterator itr;
				try
				{
					itr=boost::filesystem::recursive_directory_iterator(root);
				}
				catch(const boost::filesystem::filesystem_error& err)
				{
					std::cerr<<err.what()<<std::endl;
					itr=boost::filesystem::recursive_directory_iterator();
				}
				boost::filesystem::recursive_directory_iterator end;
				while(itr!=end)
				{
					boost::filesystem::path currentPath=*itr;
					if(is_directory(currentPath)&&((is_symlink(currentPath)&&!followSymlink)||itr.depth()==maxDepth))
					{
						itr.no_push();
					}
					else
					{
						std::string pathStr=currentPath.string();
#ifdef _WIN32
						boost::algorithm::to_lower(pathStr);
#endif
						if(matchesOneOf(pathStr,excludePatterns))
						{
							itr.no_push();
						}
						else if(is_regular_file(currentPath)&&matchesOneOf(pathStr,includePatterns))
						{
							*insertionItr++=std::move(currentPath);
						}
					}
					try
					{
						++itr;
					}
					catch(std::exception&)
					{
						itr.no_push();
						try
						{
							++itr;
						}
						catch(...)
						{
							continue;
						}
					}
				}
			}

		};
	}
}

namespace std
{
	template<>
	struct hash<lyonste::fileManagement::FileInfo>
	{
		constexpr size_t operator()(const lyonste::fileManagement::FileInfo& fileInfo) const noexcept
		{
			return fileInfo.hash;
		}
	};
}

