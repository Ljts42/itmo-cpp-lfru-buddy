#pragma once

#include <algorithm>
#include <cstddef>
#include <list>
#include <new>
#include <ostream>

template <class Key, class KeyProvider, class Allocator>
class Cache
{
public:
    template <class... AllocArgs>
    Cache(const std::size_t cache_size, AllocArgs &&... alloc_args)
        : m_max_top_size(cache_size)
        , m_max_low_size(cache_size)
        , m_alloc(std::forward<AllocArgs>(alloc_args)...)
    {
    }

    std::size_t size() const
    {
        return m_top.size() + m_low.size();
    }

    bool empty() const
    {
        return m_top.empty() && m_low.empty();
    }

    template <class T>
    T & get(const Key &);

    std::ostream & print(std::ostream &) const;

    friend std::ostream & operator<<(std::ostream & strm, const Cache & cache)
    {
        return cache.print(strm);
    }

private:
    const std::size_t m_max_top_size;
    const std::size_t m_max_low_size;
    Allocator m_alloc;

    std::list<KeyProvider *> m_top;
    std::list<KeyProvider *> m_low;
};

template <class Key, class KeyProvider, class Allocator>
template <class T>
inline T & Cache<Key, KeyProvider, Allocator>::get(const Key & key)
{
    auto it = std::find_if(m_top.begin(), m_top.end(), [&key](const KeyProvider * ptr) {
        return *ptr == key;
    });

    if (it != m_top.end()) {
        m_top.splice(m_top.begin(), m_top, it);
        return *static_cast<T *>(m_top.front());
    }

    it = std::find_if(m_low.begin(), m_low.end(), [&key](const KeyProvider * ptr) {
        return *ptr == key;
    });

    if (it != m_low.end()) {
        if (m_max_top_size == m_top.size()) {
            m_low.splice(m_low.begin(), m_top, --m_top.end());
        }

        m_top.splice(m_top.begin(), m_low, it);
        return *static_cast<T *>(m_top.front());
    }

    if (m_max_low_size == m_low.size()) {
        KeyProvider * last = m_low.back();
        m_alloc.destroy(last);
        m_low.pop_back();
    }

    m_low.push_front(m_alloc.template create<T>(key));
    return *static_cast<T *>(m_low.front());
}

template <class Key, class KeyProvider, class Allocator>
inline std::ostream & Cache<Key, KeyProvider, Allocator>::print(std::ostream & strm) const
{
    strm << "Priority:";
    if (m_top.empty()) {
        strm << " <empty>";
    }
    else {
        for (const auto ptr : m_top) {
            strm << ' ' << *ptr;
        }
    }

    strm << "\nRegular:";
    if (!m_low.empty()) {
        strm << " <empty>";
    }
    else {
        for (const auto ptr : m_low) {
            strm << ' ' << *ptr;
        }
    }
    return strm << "\n";
}
