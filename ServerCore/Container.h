#pragma once

#include "Types.h"
#include "Allocator.h"

#include <vector>
#include <string>
#include <stack>
#include <queue>
#include <map>
#include <unordered_map>
#include <set>
#include <deque>

using namespace std;

template<typename Type>
using kvector = vector<Type, STLAllocator<Type>>;

template<typename Type>
using klist = list<Type, STLAllocator<Type>>;

template<typename Key, typename Type, typename Pred=less<Key>>
using kmap = map<Key, Type, Pred, STLAllocator<pair<const Key, Type>>>;

template<typename Key, typename Pred = less<Key>>
using kset = set<Key, Pred, STLAllocator<Key>>;

template<typename Type>
using kdeque = deque<Type, STLAllocator<Type>>;

template<typename Type, typename Container = kdeque<Type>>
using kstack = stack<Type, Container>;

template<typename Type, typename Container = kdeque<Type>>
using kqueue = queue<Type, Container>;

template<typename Type, typename Container = kvector<Type>, typename Pred = less<typename Container::value_type>>
using kpriority_queue = priority_queue<Type, Container, Pred>;

using kstring = basic_string<char, char_traits<char>, STLAllocator<char>>;

using kwstring = basic_string<wchar_t, char_traits<wchar_t>, STLAllocator<wchar_t>>;

template<typename Key, typename Type, typename Hasher = hash<Key>, typename KeyEq = equal_to<Key>>
using khash_map = unordered_map<Key, Type, Hasher, KeyEq, STLAllocator<pair<const Key, Type>>>;

template<typename Key, typename Hasher = hash<Key>, typename KeyEq = equal_to<Key>>
using khash_set = unordered_map<Key, Hasher, KeyEq, STLAllocator<Key>>;