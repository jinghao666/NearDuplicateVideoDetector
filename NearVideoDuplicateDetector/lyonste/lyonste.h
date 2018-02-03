#pragma once

#include <boost/serialization/split_member.hpp>
#include <boost/filesystem.hpp>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <functional>
#include <chrono>

namespace lyonste
{
#define ENUM_STR(varName,enumVal) template<> constexpr const char* const varName <enumVal> = #enumVal
//#define EnumStringifier(StrClass,enumVal) template<> const char* const StrClass <enumVal>::str=#enumVal


	constexpr size_t getNumThreadsProperty(const boost::property_tree::ptree& properties,const std::string& propertyName="numThreads") noexcept
	{
#ifdef NDEBUG
		return std::max<size_t>(1,std::min<size_t>(cv::getNumberOfCPUs(),properties.get<size_t>(propertyName,cv::getNumberOfCPUs())));
#else
		return std::max<size_t>(1,std::min<size_t>(cv::getNumberOfCPUs(),properties.get<size_t>(propertyName,cv::getNumberOfCPUs())));
		//return 1;
#endif
		
	}


	template<typename dimensionType>
	constexpr dimensionType clipHiBorder(const dimensionType dimension,const dimensionType border)
	{
		return dimension<border?0:dimension-border;
	}

	template<typename dimensionType>
	constexpr dimensionType clip(dimensionType val,dimensionType exclusiveMax)
	{
		return val<exclusiveMax?val:exclusiveMax-1;
	}

	template<typename dimensionType>
	constexpr dimensionType clip(dimensionType min,dimensionType val,dimensionType exclusiveMax)
	{
		return std::max<dimensionType>(min,std::min<dimensionType>(val,exclusiveMax-1));
	}


	constexpr size_t ceiling(const float val)
	{
		size_t i=(size_t)val;
		return i+(i<val);
	}
	constexpr size_t ceiling(const double val)
	{
		size_t i=(size_t)val;
		return i+(i<val);
	}

	template<typename T>
	struct DerefPtrHash
	{
		constexpr size_t operator()(const T* ptr) const
		{
			if(ptr)
			{
				return std::hash<T>()(*ptr);
			}
			return 0;
		}
	};
	template<typename T>
	struct DerefPtrEqualTo
	{
		constexpr bool operator()(const T* ptr1,const T* ptr2) const
		{
			return ptr1==ptr2||(ptr1 && ptr2 && std::equal_to<T>()(*ptr1,*ptr2));
		}
	};

	template<typename T>
	struct DerefPtrLess
	{
		constexpr bool operator()(const T* ptr1,const T* ptr2) const
		{
			if(ptr1)
			{
				if(ptr2)
				{
					return std::less<T>()(*ptr1,*ptr2);
				}
				return false;
			}
			if(ptr2)
			{
				return true;
			}
			return false;;
			//return ptr1==ptr2||(ptr1 && ptr2 && std::equal_to<T>()(*ptr1,*ptr2));
		}
	};


	//TODO write/find a good implemenation of a disjoint set for the usage in white-listing file maps


	template<typename keyT>
	class MutualMap: private std::unordered_map<const keyT*,std::unordered_set<const keyT*,DerefPtrHash<keyT>,DerefPtrEqualTo<keyT>>,DerefPtrHash<keyT>,DerefPtrEqualTo<keyT>>
	{
	private:
		using valT=std::unordered_set<const keyT*,DerefPtrHash<keyT>,DerefPtrEqualTo<keyT>>;
		friend class boost::serialization::access;
		template<class Archive>
		void save(Archive& archive,const unsigned int version) const
		{
			archive & size();
			std::unordered_map<const keyT*,size_t,DerefPtrHash<keyT>,DerefPtrEqualTo<keyT>> keyIndices;
			size_t index=0;
			for(auto itr=cbegin();itr!=cend();++itr)
			{
				const auto& key=itr->first;
				keyIndices[key]=index++;
				archive & *key;
			}
			for(auto itr=cbegin();itr!=cend();++itr)
			{
				const auto& key=itr->first;
				const auto& value=itr->second;
				archive & keyIndices[key];
				archive & value.size();
				for(const auto& val:value)
				{
					archive & keyIndices[val];
				}
			}
		}
		template<class Archive>
		void load(Archive& archive,const unsigned int version)
		{
			size_t mySize;
			archive & mySize;
			std::vector<keyT*> infoVec(mySize);
			reserve(mySize);
			for(size_t i=0;i!=mySize;++i)
			{
				auto info=new keyT();
				archive & *info;
				infoVec[i]=info;
			}
			for(size_t i=0;i!=mySize;++i)
			{
				size_t index1;
				archive & index1;
				size_t setSize;
				archive & setSize;
				valT set(setSize);
				for(size_t j=0;j!=setSize;++j)
				{
					size_t index2;
					archive & index2;
					set.insert(infoVec[index2]);
				}
				insert(std::make_pair(infoVec[index1],set));
			}
		}
		template<class Archive>
		void serialize(Archive &archive,const unsigned int version)
		{
			boost::serialization::split_member(archive,*this,version);
		}
		void releaseData()
		{
			for(const auto& pair:*this)
			{
				delete pair.first;
			}
		}
	public:
		typename std::unordered_map<const keyT*,std::unordered_set<const keyT*,DerefPtrHash<keyT>,DerefPtrEqualTo<keyT>>,DerefPtrHash<keyT>,DerefPtrEqualTo<keyT>>::const_iterator cbegin() const
		{
			return  std::unordered_map<const keyT*,std::unordered_set<const keyT*,DerefPtrHash<keyT>,DerefPtrEqualTo<keyT>>,DerefPtrHash<keyT>,DerefPtrEqualTo<keyT>>::cbegin();
		}
		typename std::unordered_map<const keyT*,std::unordered_set<const keyT*,DerefPtrHash<keyT>,DerefPtrEqualTo<keyT>>,DerefPtrHash<keyT>,DerefPtrEqualTo<keyT>>::const_iterator cend() const 
		{
			return  std::unordered_map<const keyT*,std::unordered_set<const keyT*,DerefPtrHash<keyT>,DerefPtrEqualTo<keyT>>,DerefPtrHash<keyT>,DerefPtrEqualTo<keyT>>::cend();
		}
		typename std::unordered_map<const keyT*,std::unordered_set<const keyT*,DerefPtrHash<keyT>,DerefPtrEqualTo<keyT>>,DerefPtrHash<keyT>,DerefPtrEqualTo<keyT>>::iterator begin()
		{
			return  std::unordered_map<const keyT*,std::unordered_set<const keyT*,DerefPtrHash<keyT>,DerefPtrEqualTo<keyT>>,DerefPtrHash<keyT>,DerefPtrEqualTo<keyT>>::begin();
		}
		typename std::unordered_map<const keyT*,std::unordered_set<const keyT*,DerefPtrHash<keyT>,DerefPtrEqualTo<keyT>>,DerefPtrHash<keyT>,DerefPtrEqualTo<keyT>>::iterator end()
		{
			return  std::unordered_map<const keyT*,std::unordered_set<const keyT*,DerefPtrHash<keyT>,DerefPtrEqualTo<keyT>>,DerefPtrHash<keyT>,DerefPtrEqualTo<keyT>>::end();
		}
		bool add(const keyT& key1,const keyT& key2)
		{
			if(key1==key2)
			{
				return false;
			}
			const auto itr1=find(&key1);
			const auto itr2=find(&key2);
			const auto myEnd=cend();
			if(itr1==myEnd)
			{
				auto ptr1=new keyT(key1);
				if(itr2==myEnd)
				{
					auto ptr2=new keyT(key2);
					insert(std::make_pair(ptr1,valT{ptr2}));
					insert(std::make_pair(ptr2,valT{ptr1}));
				}
				else
				{
					insert(std::make_pair(ptr1,valT{itr2->first}));
					itr2->second.insert(ptr1);
				}
				return true;
			}
			if(itr2==myEnd)
			{
				auto ptr2=new keyT(key2);
				insert(std::make_pair(ptr2,valT{itr1->first}));
				itr1->second.insert(ptr2);
				return true;
			}
			return itr1->second.insert(itr2->first).second | itr2->second.insert(itr1->first).second;
		}
		bool remove(const keyT& key)
		{
			const auto itr1=find(&key);
			const auto myEnd=cend();
			if(itr1!=myEnd)
			{
				for(const auto& v:itr1->second)
				{
					auto itr2=find(&v);
					if(itr2!=myEnd)
					{
						itr2->second.erase(&key);
					}
				}
				erase(itr1);
				return true;
			}
			return false;
		}
		bool remove(const keyT& key1,const keyT& key2)
		{
			if(key1==key2)
			{
				return false;
			}
			bool changed=false;
			const auto itr1=find(&key1);
			valT* set1Ptr=NULL;
			const auto myEnd=cend();
			if(itr1!=myEnd)
			{
				set1Ptr=&itr1->second;
				changed=(set1Ptr->erase(&key2)!=0);
			}
			const auto itr2=find(&key2);
			valT* set2Ptr=NULL;
			if(itr2!=myEnd)
			{
				set2Ptr=&itr2->second;
				changed|=(set2Ptr->erase(&key1)!=0);
			}
			if(set1Ptr && set1Ptr->empty())
			{
				delete itr1->first;
				erase(itr1);
				changed=true;
			}
			if(set2Ptr && set2Ptr->empty())
			{
				delete itr2->first;
				erase(itr2);
				changed=true;
			}
			return changed;
		}
		bool contains(const keyT& key) const
		{
			return find(&key)!=cend();
		}
		bool contains(const keyT& key1,const keyT& key2) const
		{
			const auto itr1=find(&key1);
			if(itr1!=cend())
			{
				const auto& set=itr1->second;
				if(set.find(&key2)!=set.cend())
				{
					return true;
				}
			}
			return false;
		}
		MutualMap()
		{}
		~MutualMap()
		{
			releaseData();
		}
		void clear()
		{
			releaseData();
			std::unordered_map<const keyT*,std::unordered_set<const keyT*,DerefPtrHash<keyT>,DerefPtrEqualTo<keyT>>,DerefPtrHash<keyT>,DerefPtrEqualTo<keyT>>::clear();
		}
		constexpr void readFromFile(const boost::filesystem::path& file)
		{
			if(boost::filesystem::exists(file))
			{
				boost::filesystem::ifstream mapIn(file,std::ios::binary|std::ios::in);
				if(mapIn.is_open())
				{
					boost::archive::binary_iarchive mapArchive(mapIn);
					mapArchive & *this;
					mapIn.close();
				}
			}
		}
		constexpr void writeToFile(const boost::filesystem::path& file) const
		{
			boost::filesystem::create_directories(file.parent_path());
			boost::filesystem::path tmp=file.string()+".tmp";
			boost::filesystem::ofstream mapOut(tmp,std::ios::binary|std::ios::out);
			if(mapOut.is_open())
			{
				boost::archive::binary_oarchive mapArchive(mapOut);
				mapArchive & *this;
				mapOut.close();
				boost::filesystem::rename(tmp,file);
				return;
			}
			CV_Error(1,"Unable to open "+tmp.string()+" for writing");
		}
	};

	template<typename valT>
	class DisjointSet
	{
	private:
		struct DisjointNode
		{
			size_t rank;
			DisjointNode* parent;
			constexpr DisjointNode() noexcept
				:rank(0),
				parent(this)
			{
			}
			constexpr bool operator==(const DisjointNode& node) const noexcept
			{
				return this==&node;
			}
		};
		struct DisjointNodeHash
		{
			constexpr size_t operator()(const DisjointNode* node) const noexcept
			{
				return (size_t)(node);
			}
		};
		std::unordered_map<valT,DisjointNode> forest;
		constexpr const DisjointNode* findSetHelper(const DisjointNode* node) const noexcept
		{
			if(node->parent!=node)
			{
				return findSetHelper(node->parent);
			}
			return node;
		}
		constexpr DisjointNode* findSetHelper(DisjointNode* node) noexcept
		{
			if(node->parent!=node)
			{
				node->parent=findSetHelper(node->parent);
			}
			return node->parent;
		}
		
		
		constexpr void linkNodes(DisjointNode* node1,DisjointNode* node2) noexcept
		{
			CV_DbgAssert(node1);
			CV_DbgAssert(node2);
			if(node1!=node2)
			{
				node1=findSetHelper(node1);
				node2=findSetHelper(node2);
				size_t node1Rank=node1->rank;
				size_t node2Rank=node2->rank;
				if(node1Rank>node2Rank)
				{
					node2->parent=node1;
				}
				else if(node1Rank<node2Rank)
				{
					node1->parent=node2;
				}
				else
				{
					node1->parent=node2;
					++node2->rank;
				}
			}
		}
		constexpr DisjointNode* findNode(const valT& val) noexcept
		{
			const auto itr=forest.find(val);
			return itr==forest.end()?NULL:&(itr->second);
		}
		constexpr const DisjointNode* findNode(const valT& val) const noexcept
		{
			const auto itr=forest.find(val);
			return itr==forest.end()?NULL:&(itr->second);
		}
		constexpr bool nodesAreJoined(const DisjointNode* node1,const DisjointNode* node2) const noexcept
		{
			size_t node1Rank=node1->rank;
			size_t node2Rank=node2->rank;
			if(node1Rank>node2Rank)
			{
				for(;;)
				{
					const DisjointNode* nodeParent=node2->parent;
					if(nodeParent==node2)
					{
						return false;
					}
					node2=nodeParent;
					if(node2->rank==node1Rank)
					{
						return node2==node1;
					}
				}
			}
			else if(node1Rank<node2Rank)
			{
				for(;;)
				{
					const DisjointNode* nodeParent=node1->parent;
					if(nodeParent==node1)
					{
						return false;
					}
					node1=nodeParent;
					if(node1->rank==node2Rank)
					{
						return node1==node2;
					}
				}
			}
			else
			{
				for(;;)
				{
					if(node1==node2)
					{
						return true;
					}
					const DisjointNode* node1Parent=node1->parent;
					if(node1Parent==node1)
					{
						return false;
					}
					const DisjointNode* node2Parent=node2->parent;
					if(node2Parent==node2)
					{
						return false;
					}
					node1=node1Parent;
					node2=node2Parent;
				}
			}
		}
		//template<typename valT>
		using tmpMapItr=typename std::unordered_map<valT,DisjointNode>::const_iterator;
	
		constexpr std::unordered_map<const DisjointNode*,std::unordered_set<valT>,DisjointNodeHash,DerefPtrEqualTo<DisjointNode>> getTmpMap(tmpMapItr itr,tmpMapItr end) const noexcept
		{
			std::unordered_map<const DisjointNode*,std::unordered_set<valT>,DisjointNodeHash,DerefPtrEqualTo<DisjointNode>> tmpMap;
			for(;;)
			{
				const valT& val=itr->first;
				const DisjointNode& node=itr->second;
				const DisjointNode* representative=findSetHelper(&node);
				auto mapItr=tmpMap.find(representative);
				if(mapItr==tmpMap.end())
				{
					tmpMap.emplace(representative,std::unordered_set<valT>{val});
				}
				else
				{
					mapItr->second.insert(val);
				}
				if(++itr==end)
				{
					break;
				}
			}
			return tmpMap;
		}

	public:
		constexpr void remove(const valT& val) noexcept
		{
			auto nodeItr=forest.find(val);
			if(nodeItr!=forest.end())
			{
				DisjointNode* node=&(nodeItr->second);
				DisjointNode* parent=node->parent;
				if(parent==node)
				{
					auto itr=forest.begin();
					if(itr!=forest.end())
					{
						for(;;)
						{
							if(itr->second.parent==node && (&itr->second)!=(node))
							{
								parent=&(itr->second);
								++parent->rank;
								break;
							}
							if(++itr==forest.end())
							{
								return;
							}
						}
						for(;;)
						{
							if(++itr==forest.end())
							{
								break;
							}
							if(itr->second.parent==node &&(&itr->second)!=(node))
							{
								itr->second.parent=parent;
							}
						}
					}
				}
				else
				{
					for(auto pair:forest)
					{
						DisjointNode& second=pair.second;
						if(second.parent==node)
						{
							second.parent=parent;
						}
					}
				}
				forest.erase(nodeItr);
			}
		}


		constexpr void readFromFile(const boost::filesystem::path& file)
		{
			if(boost::filesystem::exists(file))
			{
				boost::filesystem::ifstream disjointSetIn(file,std::ios::binary|std::ios::in);
				if(disjointSetIn.is_open())
				{
					boost::archive::binary_iarchive disjointSetArchive(disjointSetIn);
					disjointSetArchive & *this;
					disjointSetIn.close();
				}
			}
		}
		constexpr void writeToFile(const boost::filesystem::path& file) const
		{
			boost::filesystem::create_directories(file.parent_path());
			boost::filesystem::path tmp=file.string()+".tmp";
			boost::filesystem::ofstream disjointSetOut(tmp,std::ios::binary|std::ios::out);
			if(disjointSetOut.is_open())
			{
				boost::archive::binary_oarchive disjointSetArchive(disjointSetOut);
				disjointSetArchive & *this;
				disjointSetOut.close();
				boost::filesystem::rename(tmp,file);
				return;
			}
			CV_Error(1,"Unable to open "+tmp.string()+" for writing");
		}
		constexpr void makeSet(const valT& val) noexcept
		{
			forest.try_emplace(val);
		}
		constexpr void makeSet(valT&& val) noexcept
		{
			forest.try_emplace(std::move(val));
		}
		constexpr void join(const valT& val1,const valT& val2) noexcept
		{
			DisjointNode* node1=findNode(val1);
			if(node1)
			{
				DisjointNode* node2=findNode(val2);
				if(node2)
				{
					linkNodes(node1,node2);
				}
			}
		}
		constexpr bool areJoined(const valT& val1,const valT& val2) const noexcept
		{
			const DisjointNode* node1=findNode(val1);
			if(node1)
			{
				const DisjointNode* node2=findNode(val2);
				if(node2)
				{
					return nodesAreJoined(node1,node2);
				}
			}
			return false;
		}
		constexpr bool contains(const valT& val) const noexcept
		{
			return forest.find(val)!=forest.end();
		}
		constexpr void makeUnion(const valT& val1,const valT& val2) noexcept
		{
			auto node1=forest.try_emplace(val1);
			auto node2=forest.try_emplace(val2);
			linkNodes(&(node1.first->second),&(node2.first->second));
		}
		constexpr void makeUnion(valT&& val1,const valT& val2) noexcept
		{
			auto node1=forest.try_emplace(std::move(val1));
			auto node2=forest.try_emplace(val2);
			linkNodes(&(node1.first->second),&(node2.first->second));
		}
		constexpr void makeUnion(const valT& val1,valT&& val2) noexcept
		{
			auto node1=forest.try_emplace(val1);
			auto node2=forest.try_emplace(std::move(val2));
			linkNodes(&(node1.first->second),&(node2.first->second));
		}
		constexpr void makeUnion(valT&& val1,valT&& val2) noexcept
		{
			auto node1=forest.try_emplace(std::move(val1));
			auto node2=forest.try_emplace(std::move(val2));
			linkNodes(&(node1.first->second),&(node2.first->second));
		}

		

		constexpr std::vector<std::unordered_set<valT>> getVectorView() const noexcept
		{
			std::vector<std::unordered_set<valT>> vec;
			auto itr=forest.cbegin();
			auto end=forest.cend();
			if(itr!=end)
			{
				std::unordered_map<const DisjointNode*,std::unordered_set<valT>,DisjointNodeHash,DerefPtrEqualTo<DisjointNode>> tmpMap=getTmpMap(itr,end);
				for(auto mapItr=tmpMap.begin();mapItr!=tmpMap.end();++mapItr)
				{
					vec.emplace_back(std::move(mapItr->second));
				}
			}
			return vec;
		}
		constexpr std::unordered_map<valT,std::unordered_set<valT>> getMapView() const noexcept
		{
			std::unordered_map<valT,std::unordered_set<valT>> map;
			std::vector<std::unordered_set<valT>> vec;
			auto itr=forest.cbegin();
			auto end=forest.cend();
			if(itr!=end)
			{
				std::unordered_map<const DisjointNode*,std::unordered_set<valT>,DisjointNodeHash,DerefPtrEqualTo<DisjointNode>> tmpMap=getTmpMap(itr,end);
				for(auto mapItr=tmpMap.begin();mapItr!=tmpMap.end();++mapItr)
				{
					const auto& set=mapItr->second;
					for(const auto& val:set)
					{
						map.emplace(val,set);
					}
				}
			}
			return map;
		}
	
	private:
		friend class boost::serialization::access;
		template<class Archive>
		void save(Archive& archive,const unsigned int version) const
		{
			const std::vector<std::unordered_set<valT>>& vectorView=getVectorView();
			archive & vectorView.size();
			for(const std::unordered_set<valT>& set:vectorView)
			{
				archive & set.size();
				for(const valT& val:set)
				{
					archive & val;
				}
			}
		}
		template<class Archive>
		void load(Archive& archive,const unsigned int version)
		{
			size_t numGroups;
			archive & numGroups;
			for(size_t i=0;i!=numGroups;++i)
			{
				size_t groupSize;
				archive & groupSize;
				valT representative;
				archive & representative;
				DisjointNode* node1=&(forest.try_emplace(std::move(representative)).first->second);
				for(size_t j=1;j<groupSize;++j)
				{
					valT val;
					archive & val;
					linkNodes(node1,&(forest.try_emplace(std::move(val)).first->second));
				}
			}
		}
		template<class Archive>
		void serialize(Archive &archive,const unsigned int version)
		{
			boost::serialization::split_member(archive,*this,version);
		}
	};

	struct DataSizeToString
	{
		const double numBytes;

		constexpr DataSizeToString(double numBytes):
			numBytes(numBytes)
		{

		}

		constexpr DataSizeToString(size_t numBytes):
			numBytes(static_cast<double>(numBytes))
		{

		}
		friend std::ostream& operator<<(std::ostream& stream,const DataSizeToString& dataSize);

	};

	inline std::ostream& operator<<(std::ostream& stream,const DataSizeToString& dataSize)
	{
		double numBytes=dataSize.numBytes;
		if(numBytes>=(1024ull*1024ull*1024ull*1024ull))
		{
			stream<<(numBytes/(1024ull*1024ull*1024ull*1024ull))<<"TB";
		}
		else if(numBytes>=(1024ull*1024ull*1024ull))
		{
			stream<<(numBytes/(1024ull*1024ull*1024ull))<<"GB";
		}
		else if(numBytes>=(1024ull*1024ull))
		{
			stream<<(numBytes/(1024ull*1024ull))<<"MB";
		}
		else if(numBytes>=1024ull)
		{
			stream<<(numBytes/(1024ull))<<"KB";
		}
		else
		{
			stream<<numBytes<<"B";
		}
		return stream;
		//double dNumBytes=static_cast<double>(dataSize.numBytes);

	}

	struct DurationToString
	{
	
		const size_t millis;

		

		DurationToString(const std::chrono::system_clock::duration& dur):
			millis(std::chrono::duration_cast<std::chrono::milliseconds>(dur).count())
		{
		}
	
		DurationToString(double millis):
			millis(size_t(millis))
		{
		}
		DurationToString(size_t total,size_t consumed,double numPerSec):
			millis(size_t(double(total-consumed)/(numPerSec/1000.0)))
		{
		}




		friend std::ostream& operator<<(std::ostream& stream,const DurationToString& durr);
	};


	inline std::ostream& operator<<(std::ostream& stream,const DurationToString& durr)
	{
		size_t millis=durr.millis;
		size_t hrs=millis/(1000*60*60);
		size_t min=(millis-=(hrs*1000*60*60))/(1000*60);
		size_t sec=(millis-=(min*1000*60))/1000;
		millis-=(sec*1000);
		stream<<hrs;
		if(min>9)
		{
			stream<<":"<<min;
		}
		else
		{
			stream<<":0"<<min;
		}
		if(sec>9)
		{
			stream<<":"<<sec;
		}
		else
		{
			stream<<":0"<<sec;
		}
		if(millis>99)
		{
			stream<<"."<<millis;
		}
		else if(millis>9)
		{
			stream<<".0"<<millis;
		}
		else
		{
			stream<<".00"<<millis;
		}
		return stream;
	}

	//enum ExceptionCause
	//{
	//	UnspecifiedCause,
	//	VideoFileDoesNotExist,
	//	UnableToOpenVideoFile,
	//	UnableToGrabVideoFrame,
	//	InvalidVideoDimensions,
	//	UnableToOpenKeyFrameFileForWriting
	//};

	//template<ExceptionCause cause>
	//std::exception createException(const std::string& message)
	//{
	//	return std::exception(message.c_str(),(int)cause);
	//}
}



