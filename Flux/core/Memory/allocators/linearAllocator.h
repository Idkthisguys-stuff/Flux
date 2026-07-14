#pragma once

#include <cstddef>
#include <cstdlib>
#include <vector>
#include <algorithm>

namespace Flux {
    class LinearAllocator {
        struct Block {
            void* data;
            size_t size;
            size_t offset;
            Block* next = nullptr;
        };

        Block* m_head;
        Block* m_current;
        size_t m_defaultChunkSize;

        public:
            LinearAllocator(size_t chunkSize = 1024 * 1024) : m_defaultChunkSize(chunkSize) {
                m_head = CreateBlock(m_defaultChunkSize);
                m_current = m_head;
            }

            ~LinearAllocator() {
                Block* b = m_head;
                while (b) {
                    Block* next = b->next;
                    free(b->data);
                    delete b;
                    b = next;
                }
            }

            void* Allocate(size_t size, size_t alignment = alignof(std::max_align_t)) {
                uintptr_t currentPtr = reinterpret_cast<uintptr_t>(m_current->data) + m_current->offset;
                size_t padding = (alignment - (currentPtr % alignment)) % alignment;

                if ((m_current->offset + padding + size > m_current->size)) {
                    size_t newSize = std::max(m_defaultChunkSize, size + alignment);
                    m_current->next = CreateBlock(newSize);
                    m_current = m_current->next;

                    currentPtr = reinterpret_cast<uintptr_t>(m_current->data);
                    padding = (alignment - (currentPtr % alignment)) % alignment;
                }

                void* result = reinterpret_cast<void*>(currentPtr + padding);
                m_current-> offset += (padding + size);
                return result;
            }

            void Reset() {
                m_current = m_head;
                m_current->offset = 0;
            }

        private:
            Block* CreateBlock(size_t size) {
                Block* b = new Block();
                b->data = malloc(size);
                b->size = size;
                b->offset = 0;
                return b;
            };
    };
}