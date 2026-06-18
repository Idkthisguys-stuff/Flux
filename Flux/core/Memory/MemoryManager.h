#pragma once
#include "allocators/linearAllocator.h"
#include "allocators/poolAllocator.h"
#include "allocators/stackAllocator.h"

namespace Flux {
    struct MemoryContext {
        LinearAllocator frameArena{1024 * 1024 * 5}; 
        LinearAllocator scriptArena{1024 * 1024 * 2};
        
        PoolAllocator* entityPool = nullptr;
    };

    class MemoryManager {
    public:
        static MemoryManager& Get() {
            static MemoryManager instance;
            return instance;
        }

        void Initialize(bool isEditor) {
            size_t count = isEditor ? 50000 : 10000;
            m_context.entityPool = new PoolAllocator(count, 128);
        }

        void Shutdown() {
            // Clean up heap-allocated pool
            if (m_context.entityPool) {
                delete m_context.entityPool;
                m_context.entityPool = nullptr;
            }
        }

        MemoryContext& GetContext() { return m_context; }

    private:
        MemoryManager() = default;
        MemoryContext m_context;
    };
}