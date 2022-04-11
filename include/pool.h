#pragma once

#include <cassert>
#include <cstddef>
#include <new>
#include <vector>

class PoolAllocator
{
public:
    PoolAllocator(const unsigned min_p, const unsigned max_p)
        : m_block_size(1 << min_p)
        , m_pool_size(1 << max_p)
        , m_storage(1 << max_p)
        , m_used_map(1 << (max_p - min_p))
    {
        assert(max_p > min_p);
        m_used_map[0].first = 1 << (max_p - min_p);
    }

    void * allocate(const std::size_t);
    void deallocate(const void * ptr);

private:
    static constexpr std::size_t npos = static_cast<std::size_t>(-1);

    std::size_t find_empty_place(std::size_t);

    const std::size_t m_block_size;
    const std::size_t m_pool_size;

    std::vector<std::byte> m_storage;
    std::vector<std::pair<std::size_t, bool>> m_used_map;
};
