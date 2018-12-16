#pragma once

#include <AzCore/Component/Component.h>
#include <MultiplayerCharacter/MyCharacterBus.h>
#include <AzCore/Component/TickBus.h>
#include <AzFramework/Physics/Base.h>
#include <AzFramework/Physics/World.h>

namespace MultiplayerCharacter
{
    /**
     * \brief
     */
    class MyCharacterComponent
        : public AZ::Component
        , public MyCharacterBus::Handler
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(MyCharacterComponent, "{989C0977-24E6-4F51-BD9C-8AABA9808BBB}");

        ~MyCharacterComponent() override = default;

        static void Reflect(AZ::ReflectContext* reflection);

        void Init() override;
        void Activate() override;
        void Deactivate() override;

    protected:
        // MyCharacterBus
        void PhysicalMove(const AZ::Vector3& direction) override;
        void RequestVelocity(const AZ::Vector3& direction) override;

        // TickBus
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

    private:
        AZ::Vector3 m_desiredDirectionAndSpeed;

        Physics::Ptr<Physics::World> m_physXWorld;

        void KeepUpRight();
    };
}
