#ifndef _TINYAP_REGISTRY_H__
#define _TINYAP_REGISTRY_H__

#include <unordered_map>
#include <cstring>
#include <iostream>

template <typename T, typename Alloc>
struct registry {
    typedef T value_type;
    typedef std::unordered_map<T, size_t> map_type;

    typedef typename map_type::iterator iterator;
    typedef typename map_type::const_iterator const_iterator;

    map_type m_items;
    Alloc m_alloc;

    registry() : m_items(1ULL << 16), m_alloc() {}
    ~registry() {}

    registry(const registry<T, Alloc>& r) : m_items(r.m_items), m_alloc() { std::cout << "registry copy ctor" << std::endl; }
    registry(registry<T, Alloc>&& r) : m_items(std::move(r)), m_alloc() { std::cout << "registry move ctor" << std::endl; }

    registry<T, Alloc>& operator = (const registry<T, Alloc>& r) { m_items = r.m_items; return *this; }
    registry<T, Alloc>& operator = (registry<T, Alloc>&& r) { std::swap(m_items, r.m_items); return *this; }

    const T& find(T&& v)
    {
        /*std::cout << "Calling find(" << v << ") #" << typename map_type::hasher()(v) << ' ';*/
        /*dump(std::cout);*/
        /*std::cout << std::endl;*/
        /*std::cout << "find " << v << std::endl;*/
        auto i = m_items.find(v);
        if (i == m_items.end()) {
            i = m_items.emplace(m_alloc.own(v), (size_t) 0).first;
        }
        /*std::cout << "Called find(" << v << ") ";*/
        /*dump(std::cout);*/
        /*std::cout << std::endl;*/
        return i->first;
    }

    const value_type& ref(const T& v)
    {
        /*std::cout << "Calling ref(" << v << ") #" << typename map_type::hasher()(v) << ' ';*/
        /*dump(std::cout);*/
        /*std::cout << std::endl;*/
        auto i = m_items.find(v);
        if (i == m_items.end()) {
            i = m_items.emplace(m_alloc.own(v), (size_t) 0).first;
        }
        i->second++;
        /*std::cout << "Called ref(" << v << ") ";*/
        /*dump(std::cout);*/
        /*std::cout << std::endl;*/
        return i->first;
    }

    void unref(const value_type& v)
    {
        /*std::cout << "Calling unref(" << v << ") #" << typename map_type::hasher()(v) << ' ';*/
        /*dump(std::cout);*/
        /*std::cout << std::endl;*/
        auto i = m_items.find(v);
        if (i == m_items.end()) {
            /*std::cout << "Called unref(" << v << ") but no ref." << std::endl;*/
            return;
        }
        if (i->second <= 1) {
            /*std::cout << "Called unref(" << v << ") [DISOWN]";*/
            m_alloc.disown(i->first);
            m_items.erase(i);
        } else {
            i->second--;
            /*std::cout << "Called unref(" << v << ") ref=" << i->second;*/
        }
        /*dump(std::cout);*/
        /*std::cout << std::endl;*/
    }

    size_t ref_count(const T& v) const
    {
        auto i = m_items.find(v);
        if (i == m_items.end()) {
            return 0;
        }
        return i->second;
    }

    iterator begin() { return m_items.begin(); }
    iterator end() { return m_items.end(); }
    const_iterator begin() const { return m_items.begin(); }
    const_iterator end() const { return m_items.end(); }
    const_iterator cbegin() const { return m_items.cbegin(); }
    const_iterator cend() const { return m_items.cend(); }

    void clear() { for (auto& kv: m_items) { m_alloc.disown(kv.first); } m_items.clear(); }
    size_t size() const { return m_items.size(); }

    void dump(std::ostream& os) const
    {
        for (const auto& kv: m_items) {
            os << '[' << kv.first << "]:" << kv.second << ' ';
        }
    }
};


template <typename T, typename A>
std::ostream& operator << (std::ostream& os, const registry<T, A>& r) { r.dump(os); return os; }


#endif

