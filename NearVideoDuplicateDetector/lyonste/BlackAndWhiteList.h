#pragma once
#include "lyonste.h"
#include <unordered_map>
#include <unordered_set>

namespace lyonste
{

	template<typename keyT>
	class MutualMap : private std::unordered_map<const keyT*, std::unordered_set<const keyT*, DerefPtrHash<keyT>, DerefPtrEqualTo<keyT>>, DerefPtrHash<keyT>, DerefPtrEqualTo<keyT>>
	{
	private:
		using valT = std::unordered_set<const keyT*, DerefPtrHash<keyT>, DerefPtrEqualTo<keyT>>;
		using super = std::unordered_map<const keyT*, valT, DerefPtrHash<keyT>, DerefPtrEqualTo<keyT>>;

		friend class boost::serialization::access;
		template<class Archive>
		void save(Archive& archive, const unsigned int version) const
		{
			archive & super::size();
			std::unordered_map<const keyT*, size_t, DerefPtrHash<keyT>, DerefPtrEqualTo<keyT>> keyIndices;
			size_t index = 0;
			for (auto itr = super::cbegin(); itr != super::cend(); ++itr)
			{
				const auto& key = itr->first;
				keyIndices[key] = index++;
				archive & *key;
			}
			for (auto itr = super::cbegin(); itr != super::cend(); ++itr)
			{
				const auto& key = itr->first;
				const auto& value = itr->second;
				archive & keyIndices[key];
				archive & value.size();
				for (const auto& val : value)
				{
					archive & keyIndices[val];
				}
			}
		}
		template<class Archive>
		void load(Archive& archive, const unsigned int version)
		{
			size_t mySize;
			archive & mySize;
			std::vector<keyT*> infoVec(mySize);
			super::reserve(mySize);
			for (size_t i = 0; i != mySize; ++i)
			{
				auto info = new keyT();
				archive & *info;
				infoVec[i] = info;
			}
			for (size_t i = 0; i != mySize; ++i)
			{
				size_t index1;
				archive & index1;
				size_t setSize;
				archive & setSize;
				valT set(setSize);
				for (size_t j = 0; j != setSize; ++j)
				{
					size_t index2;
					archive & index2;
					set.insert(infoVec[index2]);
				}
				super::insert(std::make_pair(infoVec[index1], set));
			}
		}
		template<class Archive>
		void serialize(Archive &archive, const unsigned int version)
		{
			boost::serialization::split_member(archive, *this, version);
		}
		void releaseData()
		{
			for (const auto& pair : *this)
			{
				delete pair.first;
			}
		}
	public:
		typename super::const_iterator cbegin() const
		{
			return  super::cbegin();
		}
		typename super::const_iterator cend() const
		{
			return  super::cend();
		}
		typename super::iterator begin()
		{
			return  super::begin();
		}
		typename super::iterator end()
		{
			return  super::end();
		}
		bool add(const keyT& key1, const keyT& key2)
		{
			if (key1 == key2)
			{
				return false;
			}
			const auto itr1 = super::find(&key1);
			const auto itr2 = super::find(&key2);
			const auto myEnd = super::cend();
			if (itr1 == myEnd)
			{
				auto ptr1 = new keyT(key1);
				if (itr2 == myEnd)
				{
					auto ptr2 = new keyT(key2);
					super::insert(std::make_pair(ptr1, valT{ ptr2 }));
					super::insert(std::make_pair(ptr2, valT{ ptr1 }));
				}
				else
				{
					super::insert(std::make_pair(ptr1, valT{ itr2->first }));
					itr2->second.insert(ptr1);
				}
				return true;
			}
			if (itr2 == myEnd)
			{
				auto ptr2 = new keyT(key2);
				super::insert(std::make_pair(ptr2, valT{ itr1->first }));
				itr1->second.insert(ptr2);
				return true;
			}
			return itr1->second.insert(itr2->first).second | itr2->second.insert(itr1->first).second;
		}
		bool remove(const keyT& key)
		{
			const auto itr1 = super::find(&key);
			const auto myEnd = super::cend();
			if (itr1 != myEnd)
			{
				for (const auto& v : itr1->second)
				{
					auto itr2 = super::find(&v);
					if (itr2 != myEnd)
					{
						itr2->second.erase(&key);
					}
				}
				super::erase(itr1);
				return true;
			}
			return false;
		}
		bool remove(const keyT& key1, const keyT& key2)
		{
			if (key1 == key2)
			{
				return false;
			}
			bool changed = false;
			const auto itr1 = super::find(&key1);
			valT* set1Ptr = NULL;
			const auto myEnd = super::cend();
			if (itr1 != myEnd)
			{
				set1Ptr = &itr1->second;
				changed = (set1Ptr->erase(&key2) != 0);
			}
			const auto itr2 = super::find(&key2);
			valT* set2Ptr = NULL;
			if (itr2 != myEnd)
			{
				set2Ptr = &itr2->second;
				changed |= (set2Ptr->erase(&key1) != 0);
			}
			if (set1Ptr && set1Ptr->empty())
			{
				delete itr1->first;
				super::erase(itr1);
				changed = true;
			}
			if (set2Ptr && set2Ptr->empty())
			{
				delete itr2->first;
				super::erase(itr2);
				changed = true;
			}
			return changed;
		}
		bool contains(const keyT& key) const
		{
			return super::find(&key) != super::cend();
		}
		bool contains(const keyT& key1, const keyT& key2) const
		{
			const auto itr1 = super::find(&key1);
			if (itr1 != cend())
			{
				const auto& set = itr1->second;
				if (set.find(&key2) != set.cend())
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
			super::clear();
		}
		constexpr void readFromFile(const boost::filesystem::path& file)
		{
			if (boost::filesystem::exists(file))
			{
				boost::filesystem::ifstream mapIn(file, std::ios::binary | std::ios::in);
				if (mapIn.is_open())
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
			boost::filesystem::path tmp = file.string() + ".tmp";
			boost::filesystem::ofstream mapOut(tmp, std::ios::binary | std::ios::out);
			if (mapOut.is_open())
			{
				boost::archive::binary_oarchive mapArchive(mapOut);
				mapArchive & *this;
				mapOut.close();
				boost::filesystem::rename(tmp, file);
				return;
			}
			CV_Error(1, "Unable to open " + tmp.string() + " for writing");
		}
		size_t size() const noexcept
		{
			return super::size();
		}
		bool empty() const noexcept
		{
			return super::empty();
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
				: rank(0)
				, parent(this)
			{
			}
			constexpr bool operator==(const DisjointNode& node) const noexcept
			{
				return this == &node;
			}
			constexpr bool operator!=(const DisjointNode& node) const noexcept
			{
				return this != &node;
			}
		};
		struct DisjointNodeHash
		{
			constexpr size_t operator()(const DisjointNode* node) const noexcept
			{
				return (size_t)(node);
			}
		};
		std::unordered_map<valT, DisjointNode> forest;
		static constexpr const DisjointNode* constFindSetHelper(const DisjointNode* node) noexcept
		{
			const DisjointNode* nodeParent = node->parent;
			if (nodeParent != node)
			{
				return constFindSetHelper(nodeParent);
			}
			return nodeParent;
		}
		static constexpr DisjointNode* findSetHelper(DisjointNode* const node) noexcept
		{
			DisjointNode* nodeParent = node->parent;
			if (nodeParent != node)
			{
				return (node->parent = findSetHelper(nodeParent));
			}
			return nodeParent;
		}
		static constexpr void joinNodes(DisjointNode* const node1, DisjointNode* const node2) noexcept
		{
			if (node1 != node2)
			{
				const size_t node1Rank = node1->rank;
				const size_t node2Rank = node2->rank;
				if (node1Rank > node2Rank)
				{
					node2->parent = node1;
				}
				else if (node1Rank < node2Rank)
				{
					node1->parent = node2;
				}
				else
				{
					node1->parent = node2;
					++node2->rank;
				}
			}

		}
		static constexpr bool walkAndSearch(const DisjointNode* walkMe, const DisjointNode* const target) noexcept
		{
			for (;;)
			{
				const DisjointNode* const parent = walkMe->parent;
				if (parent == walkMe)
				{
					return false;
				}
				if (parent == target)
				{
					return true;
				}
				walkMe = parent;
			}
		}
		static constexpr bool nodesAreJoined(const DisjointNode* const node1, const DisjointNode* const node2) noexcept
		{
			const size_t node1Rank = node1->rank;
			const size_t node2Rank = node2->rank;
			if (node1Rank > node2Rank)
			{
				return walkAndSearch(node2, node1);
			}
			else if (node1Rank < node2Rank)
			{
				return walkAndSearch(node1, node2);
			}
			return node1->parent == node2->parent;
		}
		template<typename valT_ = valT> //use template for a universal reference
		constexpr DisjointNode* findOrEmplace(valT_&& val) noexcept
		{
			return findSetHelper(&(forest.try_emplace(std::move(val)).first->second));
		}
		constexpr DisjointNode* findOrEmplace(const valT& val) noexcept
		{
			return findSetHelper(&(forest.try_emplace(val).first->second));
		}
		constexpr DisjointNode* findNode(const valT& val) noexcept
		{
			const auto itr = forest.find(val);
			return itr == forest.end() ? NULL : &(itr->second);
		}
		constexpr const DisjointNode* findNode(const valT& val) const noexcept
		{
			const auto itr = forest.find(val);
			return itr == forest.end() ? NULL : &(itr->second);
		}
		using tmpMapItr = typename std::unordered_map<valT, DisjointNode>::const_iterator;
		constexpr std::unordered_map<const DisjointNode*, std::unordered_set<valT>, DisjointNodeHash, DerefPtrEqualTo<DisjointNode>> getTmpMap(tmpMapItr itr, tmpMapItr end)  const noexcept
		{
			std::unordered_map<const DisjointNode*, std::unordered_set<valT>, DisjointNodeHash, DerefPtrEqualTo<DisjointNode>> tmpMap;
			for (;;)
			{
				const valT& val = itr->first;
				const DisjointNode& node = itr->second;
				const DisjointNode* representative = constFindSetHelper(&node);
				auto mapItr = tmpMap.find(representative);
				if (mapItr == tmpMap.end())
				{
					tmpMap.emplace(representative, std::unordered_set<valT>{val});
				}
				else
				{
					mapItr->second.insert(val);
				}
				if (++itr == end)
				{
					break;
				}
			}
			return tmpMap;
		}

	public:
		constexpr DisjointSet() noexcept
		{
		}
		constexpr void makeSet(const valT& val) noexcept
		{
			forest.try_emplace(val);
		}
		constexpr void makeSet(valT&& val) noexcept
		{
			forest.try_emplace(std::move(val));
		}
		constexpr bool contains(const valT& val) const noexcept
		{
			return forest.find(val) != forest.end();
		}
		constexpr void join(const valT& val1, const valT& val2) noexcept
		{
			joinNodes(findOrEmplace(val1), findOrEmplace(val2));
		}
		constexpr void join(valT&& val1, const valT& val2) noexcept
		{
			joinNodes(findOrEmplace(std::move(val1)), findOrEmplace(val2));
		}
		constexpr void join(const valT& val1, valT&& val2) noexcept
		{
			joinNodes(findOrEmplace(val1), findOrEmplace(std::move(val2)));
		}
		constexpr void join(valT&& val1, valT&& val2) noexcept
		{
			joinNodes(findOrEmplace(std::move(val1)), findOrEmplace(std::move(val2)));
		}

		constexpr void readFromFile(const boost::filesystem::path& file)
		{
			if (boost::filesystem::exists(file))
			{
				boost::filesystem::ifstream disjointSetIn(file, std::ios::binary | std::ios::in);
				if (disjointSetIn.is_open())
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
			boost::filesystem::path tmp = file.string() + ".tmp";
			boost::filesystem::ofstream disjointSetOut(tmp, std::ios::binary | std::ios::out);
			if (disjointSetOut.is_open())
			{
				boost::archive::binary_oarchive disjointSetArchive(disjointSetOut);
				disjointSetArchive & *this;
				disjointSetOut.close();
				boost::filesystem::rename(tmp, file);
				return;
			}
			CV_Error(1, "Unable to open " + tmp.string() + " for writing");
		}
		constexpr bool areJoined(const valT& val1, const valT& val2) noexcept
		{
			DisjointNode* node1 = findNode(val1);
			if (node1)
			{
				DisjointNode* node2 = findNode(val2);
				if (node2)
				{
					return nodesAreJoined(findSetHelper(node1), findSetHelper(node2));
				}
			}
			return false;
		}
		constexpr std::vector<std::unordered_set<valT>> getVectorView()  const noexcept
		{
			std::vector<std::unordered_set<valT>> vec;
			auto itr = forest.cbegin();
			auto end = forest.cend();
			if (itr != end)
			{
				std::unordered_map<const DisjointNode*, std::unordered_set<valT>, DisjointNodeHash, DerefPtrEqualTo<DisjointNode>> tmpMap = getTmpMap(itr, end);
				for (auto mapItr = tmpMap.begin(); mapItr != tmpMap.end(); ++mapItr)
				{
					vec.emplace_back(std::move(mapItr->second));
				}
			}
			return vec;
		}
		constexpr std::unordered_map<valT, std::unordered_set<valT>> getMapView()  const noexcept
		{
			std::unordered_map<valT, std::unordered_set<valT>> map;
			std::vector<std::unordered_set<valT>> vec;
			auto itr = forest.cbegin();
			auto end = forest.cend();
			if (itr != end)
			{
				std::unordered_map<const DisjointNode*, std::unordered_set<valT>, DisjointNodeHash, DerefPtrEqualTo<DisjointNode>> tmpMap = getTmpMap(itr, end);
				for (auto mapItr = tmpMap.begin(); mapItr != tmpMap.end(); ++mapItr)
				{
					const auto& set = mapItr->second;
					for (const auto& val : set)
					{
						map.emplace(val, set);
					}
				}
			}
			return map;
		}

	private:
		friend class boost::serialization::access;
		template<class Archive>
		void save(Archive& archive, const unsigned int version) const
		{
			const std::vector<std::unordered_set<valT>>& vectorView = getVectorView();
			archive & vectorView.size();
			for (const std::unordered_set<valT>& set : vectorView)
			{
				archive & set.size();
				for (const valT& val : set)
				{
					archive & val;
				}
			}
		}
		template<class Archive>
		void load(Archive& archive, const unsigned int version)
		{
			size_t numGroups;
			archive & numGroups;
			for (size_t i = 0; i != numGroups; ++i)
			{
				size_t groupSize;
				archive & groupSize;
				valT representative;
				archive & representative;
				DisjointNode* node1 = &(forest.try_emplace(std::move(representative)).first->second);
				for (size_t j = 1; j<groupSize; ++j)
				{
					valT val;
					archive & val;
					joinNodes(node1, &(forest.try_emplace(std::move(val)).first->second));
				}
			}
		}
		template<class Archive>
		void serialize(Archive &archive, const unsigned int version)
		{
			boost::serialization::split_member(archive, *this, version);
		}
	};


	//template<typename valT>
	//class DisjointSetOld
	//{
	//private:
	//	struct DisjointNode
	//	{
	//		size_t rank;
	//		DisjointNode* parent;
	//		constexpr DisjointNode() noexcept
	//			:rank(0),
	//			parent(this)
	//		{
	//		}
	//		constexpr bool operator==(const DisjointNode& node) const noexcept
	//		{
	//			return this==&node;
	//		}
	//	};
	//	struct DisjointNodeHash
	//	{
	//		constexpr size_t operator()(const DisjointNode* node) const noexcept
	//		{
	//			return (size_t)(node);
	//		}
	//	};
	//	std::unordered_map<valT,DisjointNode> forest;
	//	constexpr const DisjointNode* findSetHelper(const DisjointNode* node) const noexcept
	//	{
	//		if(node->parent!=node)
	//		{
	//			return findSetHelper(node->parent);
	//		}
	//		return node;
	//	}
	//	constexpr DisjointNode* findSetHelper(DisjointNode* node) noexcept
	//	{
	//		if(node->parent!=node)
	//		{
	//			node->parent=findSetHelper(node->parent);
	//		}
	//		return node->parent;
	//	}
	//	
	//	
	//	constexpr void linkNodes(DisjointNode* node1,DisjointNode* node2) noexcept
	//	{
	//		CV_DbgAssert(node1);
	//		CV_DbgAssert(node2);
	//		if(node1!=node2)
	//		{
	//			node1=findSetHelper(node1);
	//			node2=findSetHelper(node2);
	//			size_t node1Rank=node1->rank;
	//			size_t node2Rank=node2->rank;
	//			if(node1Rank>node2Rank)
	//			{
	//				node2->parent=node1;
	//			}
	//			else if(node1Rank<node2Rank)
	//			{
	//				node1->parent=node2;
	//			}
	//			else
	//			{
	//				node1->parent=node2;
	//				++node2->rank;
	//			}
	//		}
	//	}
	//	constexpr DisjointNode* findNode(const valT& val) noexcept
	//	{
	//		const auto itr=forest.find(val);
	//		return itr==forest.end()?NULL:&(itr->second);
	//	}
	//	constexpr const DisjointNode* findNode(const valT& val) const noexcept
	//	{
	//		const auto itr=forest.find(val);
	//		return itr==forest.end()?NULL:&(itr->second);
	//	}
	//	constexpr bool nodesAreJoined(const DisjointNode* node1,const DisjointNode* node2) const noexcept
	//	{
	//		//TODO there is a bug here
	//		//When the ranks are equal, the nodes are supposed to be the same distance from the
	//		//representative node. However, tests have shown that linked nodes with identical ranks
	//		//somes times are different distances from the representative node
	//		//This implementation is a temporary fix
	//
	//		for (;;)
	//		{
	//			const DisjointNode* nodeParent = node1->parent;
	//			if (nodeParent == node1)
	//			{
	//				break;
	//			}
	//			node1 = nodeParent;
	//		}
	//		for (;;)
	//		{
	//			const DisjointNode* nodeParent = node2->parent;
	//			if (nodeParent == node2)
	//			{
	//				break;
	//			}
	//			node2 = nodeParent;
	//		}
	//		return node1 == node2;
	//
	//		//size_t node1Rank=node1->rank;
	//		//size_t node2Rank=node2->rank;
	//		//if(node1Rank>node2Rank)
	//		//{
	//		//	for(;;)
	//		//	{
	//		//		const DisjointNode* nodeParent=node2->parent;
	//		//		if(nodeParent==node2)
	//		//		{
	//		//			return false;
	//		//		}
	//		//		node2=nodeParent;
	//		//		if(node2->rank==node1Rank)
	//		//		{
	//		//			return node2==node1;
	//		//		}
	//		//	}
	//		//}
	//		//else if(node1Rank<node2Rank)
	//		//{
	//		//	for(;;)
	//		//	{
	//		//		const DisjointNode* nodeParent=node1->parent;
	//		//		if(nodeParent==node1)
	//		//		{
	//		//			return false;
	//		//		}
	//		//		node1=nodeParent;
	//		//		if(node1->rank==node2Rank)
	//		//		{
	//		//			return node1==node2;
	//		//		}
	//		//	}
	//		//}
	//		//else
	//		//{
	//		//	
	//		//	for(;;)
	//		//	{
	//		//		if(node1==node2)
	//		//		{
	//		//			return true;
	//		//		}
	//		//		const DisjointNode* node1Parent=node1->parent;
	//		//		if(node1Parent==node1)
	//		//		{
	//		//			for (;;)
	//		//			{
	//		//				const DisjointNode* node2Parent = node2->parent;
	//		//				if (node2Parent == node2)
	//		//				{
	//		//					return false;
	//		//				}
	//		//				node2 = node2Parent;
	//		//				if (node1 == node2)
	//		//				{
	//		//					return true;
	//		//				}
	//		//			}
	//		//		}
	//		//		const DisjointNode* node2Parent=node2->parent;
	//		//		if(node2Parent==node2)
	//		//		{
	//		//			for (;;)
	//		//			{
	//		//				node1 = node1Parent;
	//		//				if (node1 == node2)
	//		//				{
	//		//					return true;
	//		//				}
	//		//				node1Parent = node1->parent;
	//		//				if (node1Parent == node1)
	//		//				{
	//		//					return false;
	//		//				}
	//		//			}
	//		//		}
	//		//		node1=node1Parent;
	//		//		node2=node2Parent;
	//		//	}
	//		//}
	//	}
	//	//template<typename valT>
	//	using tmpMapItr=typename std::unordered_map<valT,DisjointNode>::const_iterator;
	//
	//	constexpr std::unordered_map<const DisjointNode*,std::unordered_set<valT>,DisjointNodeHash,DerefPtrEqualTo<DisjointNode>> getTmpMap(tmpMapItr itr,tmpMapItr end) const noexcept
	//	{
	//		std::unordered_map<const DisjointNode*,std::unordered_set<valT>,DisjointNodeHash,DerefPtrEqualTo<DisjointNode>> tmpMap;
	//		for(;;)
	//		{
	//			const valT& val=itr->first;
	//			const DisjointNode& node=itr->second;
	//			const DisjointNode* representative=findSetHelper(&node);
	//			auto mapItr=tmpMap.find(representative);
	//			if(mapItr==tmpMap.end())
	//			{
	//				tmpMap.emplace(representative,std::unordered_set<valT>{val});
	//			}
	//			else
	//			{
	//				mapItr->second.insert(val);
	//			}
	//			if(++itr==end)
	//			{
	//				break;
	//			}
	//		}
	//		return tmpMap;
	//	}
	//
	//public:
	//	constexpr void remove(const valT& val) noexcept
	//	{
	//		auto nodeItr=forest.find(val);
	//		if(nodeItr!=forest.end())
	//		{
	//			DisjointNode* node=&(nodeItr->second);
	//			DisjointNode* parent=node->parent;
	//			if(parent==node)
	//			{
	//				auto itr=forest.begin();
	//				if(itr!=forest.end())
	//				{
	//					for(;;)
	//					{
	//						if(itr->second.parent==node && (&itr->second)!=(node))
	//						{
	//							parent=&(itr->second);
	//							++parent->rank;
	//							break;
	//						}
	//						if(++itr==forest.end())
	//						{
	//							return;
	//						}
	//					}
	//					for(;;)
	//					{
	//						if(++itr==forest.end())
	//						{
	//							break;
	//						}
	//						if(itr->second.parent==node &&(&itr->second)!=(node))
	//						{
	//							itr->second.parent=parent;
	//						}
	//					}
	//				}
	//			}
	//			else
	//			{
	//				for(auto pair:forest)
	//				{
	//					DisjointNode& second=pair.second;
	//					if(second.parent==node)
	//					{
	//						second.parent=parent;
	//					}
	//				}
	//			}
	//			forest.erase(nodeItr);
	//		}
	//	}
	//	constexpr void readFromFile(const boost::filesystem::path& file)
	//	{
	//		if(boost::filesystem::exists(file))
	//		{
	//			boost::filesystem::ifstream disjointSetIn(file,std::ios::binary|std::ios::in);
	//			if(disjointSetIn.is_open())
	//			{
	//				boost::archive::binary_iarchive disjointSetArchive(disjointSetIn);
	//				disjointSetArchive & *this;
	//				disjointSetIn.close();
	//			}
	//		}
	//	}
	//	constexpr void writeToFile(const boost::filesystem::path& file) const
	//	{
	//		boost::filesystem::create_directories(file.parent_path());
	//		boost::filesystem::path tmp=file.string()+".tmp";
	//		boost::filesystem::ofstream disjointSetOut(tmp,std::ios::binary|std::ios::out);
	//		if(disjointSetOut.is_open())
	//		{
	//			boost::archive::binary_oarchive disjointSetArchive(disjointSetOut);
	//			disjointSetArchive & *this;
	//			disjointSetOut.close();
	//			boost::filesystem::rename(tmp,file);
	//			return;
	//		}
	//		CV_Error(1,"Unable to open "+tmp.string()+" for writing");
	//	}
	//	constexpr void makeSet(const valT& val) noexcept
	//	{
	//		forest.try_emplace(val);
	//	}
	//	constexpr void makeSet(valT&& val) noexcept
	//	{
	//		forest.try_emplace(std::move(val));
	//	}
	//	constexpr void join(const valT& val1,const valT& val2) noexcept
	//	{
	//		DisjointNode* node1=findNode(val1);
	//		if(node1)
	//		{
	//			DisjointNode* node2=findNode(val2);
	//			if(node2)
	//			{
	//				linkNodes(node1,node2);
	//			}
	//		}
	//	}
	//	constexpr bool areJoined(const valT& val1,const valT& val2) const noexcept
	//	{
	//		const DisjointNode* node1=findNode(val1);
	//		if(node1)
	//		{
	//			const DisjointNode* node2=findNode(val2);
	//			if(node2)
	//			{
	//				return nodesAreJoined(node1,node2);
	//			}
	//		}
	//		return false;
	//	}
	//	constexpr bool contains(const valT& val) const noexcept
	//	{
	//		return forest.find(val)!=forest.end();
	//	}
	//	constexpr void makeUnion(const valT& val1,const valT& val2) noexcept
	//	{
	//		auto node1=forest.try_emplace(val1);
	//		auto node2=forest.try_emplace(val2);
	//		linkNodes(&(node1.first->second),&(node2.first->second));
	//	}
	//	constexpr void makeUnion(valT&& val1,const valT& val2) noexcept
	//	{
	//		auto node1=forest.try_emplace(std::move(val1));
	//		auto node2=forest.try_emplace(val2);
	//		linkNodes(&(node1.first->second),&(node2.first->second));
	//	}
	//	constexpr void makeUnion(const valT& val1,valT&& val2) noexcept
	//	{
	//		auto node1=forest.try_emplace(val1);
	//		auto node2=forest.try_emplace(std::move(val2));
	//		linkNodes(&(node1.first->second),&(node2.first->second));
	//	}
	//	constexpr void makeUnion(valT&& val1,valT&& val2) noexcept
	//	{
	//		auto node1=forest.try_emplace(std::move(val1));
	//		auto node2=forest.try_emplace(std::move(val2));
	//		linkNodes(&(node1.first->second),&(node2.first->second));
	//	}
	//	constexpr std::vector<std::unordered_set<valT>> getVectorView() const noexcept
	//	{
	//		std::vector<std::unordered_set<valT>> vec;
	//		auto itr=forest.cbegin();
	//		auto end=forest.cend();
	//		if(itr!=end)
	//		{
	//			std::unordered_map<const DisjointNode*,std::unordered_set<valT>,DisjointNodeHash,DerefPtrEqualTo<DisjointNode>> tmpMap=getTmpMap(itr,end);
	//			for(auto mapItr=tmpMap.begin();mapItr!=tmpMap.end();++mapItr)
	//			{
	//				vec.emplace_back(std::move(mapItr->second));
	//			}
	//		}
	//		return vec;
	//	}
	//	constexpr std::unordered_map<valT,std::unordered_set<valT>> getMapView() const noexcept
	//	{
	//		std::unordered_map<valT,std::unordered_set<valT>> map;
	//		std::vector<std::unordered_set<valT>> vec;
	//		auto itr=forest.cbegin();
	//		auto end=forest.cend();
	//		if(itr!=end)
	//		{
	//			std::unordered_map<const DisjointNode*,std::unordered_set<valT>,DisjointNodeHash,DerefPtrEqualTo<DisjointNode>> tmpMap=getTmpMap(itr,end);
	//			for(auto mapItr=tmpMap.begin();mapItr!=tmpMap.end();++mapItr)
	//			{
	//				const auto& set=mapItr->second;
	//				for(const auto& val:set)
	//				{
	//					map.emplace(val,set);
	//				}
	//			}
	//		}
	//		return map;
	//	}
	//
	//private:
	//	friend class boost::serialization::access;
	//	template<class Archive>
	//	void save(Archive& archive,const unsigned int version) const
	//	{
	//		const std::vector<std::unordered_set<valT>>& vectorView=getVectorView();
	//		archive & vectorView.size();
	//		for(const std::unordered_set<valT>& set:vectorView)
	//		{
	//			archive & set.size();
	//			for(const valT& val:set)
	//			{
	//				archive & val;
	//			}
	//		}
	//	}
	//	template<class Archive>
	//	void load(Archive& archive,const unsigned int version)
	//	{
	//		size_t numGroups;
	//		archive & numGroups;
	//		for(size_t i=0;i!=numGroups;++i)
	//		{
	//			size_t groupSize;
	//			archive & groupSize;
	//			valT representative;
	//			archive & representative;
	//			DisjointNode* node1=&(forest.try_emplace(std::move(representative)).first->second);
	//			for(size_t j=1;j<groupSize;++j)
	//			{
	//				valT val;
	//				archive & val;
	//				linkNodes(node1,&(forest.try_emplace(std::move(val)).first->second));
	//			}
	//		}
	//	}
	//	template<class Archive>
	//	void serialize(Archive &archive,const unsigned int version)
	//	{
	//		boost::serialization::split_member(archive,*this,version);
	//	}
	//};


}