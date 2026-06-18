#pragma once

#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <stdexcept>

namespace Flux {
    class PoolAllocator {
        union Node {
            Node* next;
            char data[1];
        };

        void* m_poolStart;
        Node* m_freeList;
        size_t m_blockSize;
        size_t m_numBlocks;

        public:
            PoolAllocator(size_t numBlocks, size_t blockSize) : m_numBlocks(numBlocks), m_blockSize(std::max(blockSize, sizeof(Node*))) {
                m_poolStart = malloc(m_numBlocks * m_blockSize);
                m_freeList = static_cast<Node*>(m_poolStart);

                Node* current = m_freeList;
                for (size_t i = 0; i < m_numBlocks - 1; ++i) {
                    current->next = reinterpret_cast<Node*>(reinterpret_cast<char*>(current));
                    current = current->next;
                }

                current->next = nullptr;
            }

            ~PoolAllocator() {
                free(m_poolStart);
            }

            void* Allocate() {
                if (!m_freeList) return nullptr;

                void* ptr = m_freeList;
                m_freeList = m_freeList->next;
                return ptr;
            }

            void Deallocate(void* ptr) {
                if (!ptr) return;

                Node* node = static_cast<Node*>(ptr);
                node->next = m_freeList;
                m_freeList = node;
            }
    };
}