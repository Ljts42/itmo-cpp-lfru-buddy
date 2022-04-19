#pragma once

#include <cstddef>
#include <functional>
#include <new>
#include <set>
#include <vector>

class PoolAllocator
{
public:
    PoolAllocator(const unsigned, const unsigned);

    void * allocate(const std::size_t);
    void deallocate(const void *);

private:
    static constexpr std::size_t npos = static_cast<std::size_t>(-1);

    std::size_t find_empty_place(std::size_t);

    const std::size_t m_block_size;
    const std::size_t m_pool_size;

    std::vector<std::byte> m_storage;
    std::vector<std::set<std::size_t>> m_available_blocks;
    std::vector<bool> m_used_map;
};
