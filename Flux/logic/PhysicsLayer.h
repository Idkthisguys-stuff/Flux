// holy shit, this took me a while

#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>

namespace Flux {
    namespace Layers {
        static constexpr JPH::ObjectLayer NON_MOVING = 0; // static
        static constexpr JPH::ObjectLayer MOVING = 1; // dynamic
        static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
    }

    namespace BroadPhaseLayers {
        static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
        static constexpr JPH::BroadPhaseLayer MOVING(1);
        static constexpr uint32_t NUM_LAYERS = 2;
    }

    class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
    public:
        virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override {
            if (inObject1 == Layers::MOVING || inObject2 == Layers::MOVING) {
                return true;
            }
            return false;
        }
    };

    class BroadPhaseLayerInterfaceImpl : public JPH::BroadPhaseLayerInterface {
        private:
            JPH::BroadPhaseLayer m_ObjectToBroadPhase[Layers::NUM_LAYERS];
        
        public:
            BroadPhaseLayerInterfaceImpl() {
                m_ObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
                m_ObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
            }

            virtual uint32_t GetNumBroadPhaseLayers() const override {
                return BroadPhaseLayers::NUM_LAYERS;
            }

            virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override {
                return m_ObjectToBroadPhase[inLayer];
            }

            #if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
                virtual const* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override {
                    switch ((JPH::BroadPhaseLayer::Type)inLayer) {
                        case (JPH::BroadPhaseLayer::Type)BroaBroadPhaseLayers::NON_MOVING: return "NON_MOVING";
                        case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING: return "MOVING";
                        default: return "INVALID";
                    }
                }
            #endif
    };

    class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter {
        public:
            virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override {
                JPH::ObjectLayer layer2AsObject = (JPH::ObjectLayer)inLayer2.GetValue();

                if (inLayer1 == Layers::MOVING || layer2AsObject == Layers::MOVING) {
                    return true;
                }
                return false;
            }
    };
    }