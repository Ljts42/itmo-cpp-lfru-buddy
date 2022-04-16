#include "pool.h"

#include <functional>

std::size_t PoolAllocator::find_empty_place(std::size_t order)
{
    if (order >= m_available_blocks.size()) {
        return npos;
    }
    std::size_t result;
    if (m_available_blocks[order].empty()) {
        result = find_empty_place(order + 1);
        if (result != npos) {
            m_available_blocks[order].insert(result + (1 << order));
        }
    }
    else {
        result = *m_available_blocks[order].begin();
        if (result != npos) {
            m_available_blocks[order].erase(result);
        }
    }
    return result;
}

void * PoolAllocator::allocate(const std::size_t n)
{
    if (n > m_pool_size) {
        throw std::bad_alloc{};
    }

    std::size_t order = 0;
    while (m_block_size * (1 << order) < n) {
        order++;
    }
    const auto pos = find_empty_place(order);

    if (pos != npos) {
        m_used_map[pos] = true;
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
        assert(((b_ptr - begin) % m_block_size) == 0);
        std::size_t offset = (b_ptr - begin) / m_block_size;

        if (offset < m_used_map.size()) {
            m_used_map[offset] = false;
            std::size_t order = 0;
            while (order + 1 < m_available_blocks.size()) {
                std::size_t united = offset - offset % (1 << (order + 1));
                std::size_t buddy = (united != offset) ? united : offset + (1 << order);
                if (m_used_map[buddy]) {
                    break;
                }
                if (m_available_blocks[order].count(buddy)) {
                    m_available_blocks[order].erase(buddy);
                    offset = united;
                }
                order++;
            }
            m_available_blocks[order].insert(offset);
        }
    }
}
