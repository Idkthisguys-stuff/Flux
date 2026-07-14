#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <algorithm>

namespace Flux {
    class StackAllocator {
        struct Header {
            size_t totalBytes;
        };

        void* m_start;
        size_t m_size;
        size_t m_offset;

    public:
        StackAllocator(size_t size) : m_size(size), m_offset(0) {
            m_start = malloc(size);
        }

        ~StackAllocator() { free(m_start); }

        void* Allocate(size_t bytes, size_t alignment = alignof(std::max_align_t)) {
            uintptr_t currentAddr = reinterpret_cast<uintptr_t>(m_start) + m_offset;
            size_t padding = (alignment - (currentAddr % alignment)) % alignment;
            
            size_t totalRequested = padding + sizeof(Header) + bytes;
            if (m_offset + totalRequested > m_size) return nullptr;

            Header* header = reinterpret_cast<Header*>(reinterpret_cast<char*>(m_start) + m_offset + padding);
            header->totalBytes = totalRequested;
            
            void* dataPtr = reinterpret_cast<char*>(header) + sizeof(Header);
            m_offset += totalRequested;
            return dataPtr;
        }

        void Deallocate(void* ptr) {
            if (!ptr) return;

            Header* header = reinterpret_cast<Header*>(reinterpret_cast<char*>(ptr) - sizeof(Header));
            m_offset -= header->totalBytes;
        }

        void Reset() { m_offset = 0; }
    };
}