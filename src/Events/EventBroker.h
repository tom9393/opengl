#pragma once

#include "pch.h"
#include "Event.h"

namespace Engine 
{
    class EventBroker 
    {
    public:
        using EventCallback = std::function<void(Event&)>;
        using SubscriberID = unsigned long long;

        struct Subscription
        {
            SubscriberID id;
            void* owner;
            EventCallback callback;
        };

        EventBroker() = default;

        template<typename T>
        SubscriberID Subscribe(std::function<void(T&)> callback, void* owner) 
        {
            EventType type = T::GetStaticType();
            SubscriberID id = m_NextID++;

            EventCallback base_callback = 
                [cb = std::move(callback)](Event& e) 
                {
                    cb(static_cast<T&>(e));
                };

            m_Subscribers[type].push_back({id, owner, std::move(base_callback)});
            m_OwnerToIDs[owner].push_back({type, id});
            
            return id;
        }

        SubscriberID SubscribeCategory(EventCategory category, EventCallback callback, void* owner) 
        {
            SubscriberID id = m_NextID++;
            Subscription sub = { id, owner, std::move(callback) };
            
            m_CategorySubscribers[category].push_back(std::move(sub));
            m_OwnerToCategoryIDs[owner].push_back({category, id});

            return id;
        }

        void UnsubscribeAll(void* owner) 
        {
            if (m_OwnerToIDs.count(owner))
            {
                for (const auto& pair : m_OwnerToIDs.at(owner))
                {
                    EventType type = pair.first;
                    SubscriberID id = pair.second;
                    
                    auto& list = m_Subscribers[type];
                    list.erase(std::remove_if(list.begin(), list.end(), 
                        [id](const Subscription& sub) { return sub.id == id; }), 
                        list.end());
                }
                m_OwnerToIDs.erase(owner);
            }

            if (m_OwnerToCategoryIDs.count(owner))
            {
                for (const auto& pair : m_OwnerToCategoryIDs.at(owner))
                {
                    EventCategory category = pair.first;
                    SubscriberID id = pair.second;

                    auto& list = m_CategorySubscribers[category];
                    list.erase(std::remove_if(list.begin(), list.end(), 
                        [id](const Subscription& sub) { return sub.id == id; }), 
                        list.end());
                }
                m_OwnerToCategoryIDs.erase(owner);
            }
        }

        template<typename T>
        void Unsubscribe(SubscriberID id) 
        {
            EventType type = T::getStaticType();
            
            if (m_Subscribers.count(type)) 
            {
                auto& list = m_Subscribers.at(type);

                list.erase(std::remove_if(list.begin(), list.end(), 
                    [id](const Subscription& sub) { return sub.id == id; }), 
                    list.end());
            }
        }

        void UnsubscribeCategory(EventCategory category, SubscriberID id) 
        {
            if (m_CategorySubscribers.count(category)) 
            {
                auto& list = m_CategorySubscribers.at(category);
                
                list.erase(std::remove_if(list.begin(), list.end(), 
                    [id](const Subscription& sub) { return sub.id == id; }), 
                    list.end());
            }
        }

        void Publish(Event& event) 
        {
            EventType type = event.GetEventType();
            if (m_Subscribers.count(type)) {
                for (const auto& sub : m_Subscribers.at(type)) {
                    sub.callback(event);
                    if (event.handled) return; 
                }
            }
            
            if (event.handled) return;

            for (const auto& [category, sub_list] : m_CategorySubscribers) {
                if (event.IsInCategory(static_cast<EventCategory>(category))) {
                    for (const auto& sub : sub_list) {
                        sub.callback(event);
                        if (event.handled) return;
                    }
                }
            }
        }

        template<typename T>
        void GetSubCount()
        {
            EventType type = T::GetStaticType();
            if (m_Subscribers.count(type)) {
                for (const auto& sub : m_Subscribers.at(type)) {
                        std::cout << sub.id << std::endl;
                }
            }
        }

        static EventBroker &Get()
        {
                static EventBroker instance;
                return instance;
        }

    private:
        std::atomic<SubscriberID> m_NextID = 0;

        std::unordered_map<EventType, std::vector<Subscription>> m_Subscribers;
        std::unordered_map<EventCategory, std::vector<Subscription>> m_CategorySubscribers;
        
        std::unordered_map<void*, std::vector<std::pair<EventType, SubscriberID>>> m_OwnerToIDs;
        std::unordered_map<void*, std::vector<std::pair<EventCategory, SubscriberID>>> m_OwnerToCategoryIDs;
    };

}
