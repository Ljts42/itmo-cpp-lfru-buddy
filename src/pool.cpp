#include "pool.h"

#include <cassert>
#include <functional>

std::size_t PoolAllocator::find_empty_place(std::size_t k)
{
    if (k > m_used_map.size()) {
        return npos;
    }
    for (std::size_t i = 0; i < m_used_map.size(); i += m_used_map[i].first) {
        if (!m_used_map[i].second && m_used_map[i].first == k) {
            return i;
        }
    }

    std::size_t res = find_empty_place(k << 1);
    if (res < m_used_map.size()) {
        m_used_map[res].first >>= 1;
        m_used_map[res + m_used_map[res].first].first = m_used_map[res].first;
        return res;
    }

    return npos;
}

void * PoolAllocator::allocate(const std::size_t n)
{
    std::size_t k = m_block_size;

    while (k < n) {
        k <<= 1;
    }

    const auto pos = find_empty_place(k / m_block_size);

    if (pos != npos) {
        m_used_map[pos].second = true;
        return &m_storage[pos * m_block_size];
    }
    throw std::bad_alloc{};
}

void PoolAllocator::deallocate(const void * ptr)
{
    auto b_ptr = static_cast<const std::byte *>(ptr);
    const auto begin = &m_storage[0];
    std::less<const std::byte *> cmp;
    if (b_ptr == begin || (cmp(begin, b_ptr) && cmp(b_ptr, &m_storage.back() + 1))) {
        std::size_t offset = (b_ptr - begin) / m_block_size;
        assert(((b_ptr - begin) % m_block_size) == 0);
        if (offset < m_used_map.size()) {
            m_used_map[offset].second = false;

            std::size_t size = m_used_map[offset].first;
            while (offset + size < m_used_map.size()) {
                offset -= offset % (size * 2);
                if (!m_used_map[offset].second && !m_used_map[offset + size].second) {
                    m_used_map[offset + size].first = 0;
                    size <<= 1;
                    m_used_map[offset].first = size;
                }
                else {
                    break;
                }
            }
        }
    }
}