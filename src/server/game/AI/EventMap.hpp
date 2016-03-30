#ifndef TRINITY_GAME_AI_EVENT_MAP_HPP
#define TRINITY_GAME_AI_EVENT_MAP_HPP

#include "Define.h"
#include <map>

class EventMap final
{
    /**
     * Internal storage type.
     * Key: Time as uint32 when the event should occur.
     * Value: The event data as uint32.
     *
     * Structure of event data:
     * - Bit  0 - 15: Event Id.
     * - Bit 16 - 23: Group
     * - Bit 24 - 31: Phase
     * - Pattern: 0xPPGGEEEE
     */
    typedef std::multimap<uint32, uint32> EventStore;

public:
    EventMap() : _time(0), _phase(0) { }

    /**
     * @name Reset
     * @brief Removes all scheduled events and resets time and phase.
     */
    void Reset()
    {
        _eventMap.clear();
        _time = 0;
        _phase = 0;
    }

    /**
     * @name Update
     * @brief Updates the timer of the event map.
     * @param time Value to be added to time.
     */
    void Update(uint32 time)
    {
        _time += time;
    }

    /**
     * @name GetTimer
     * @return Current timer value.
     */
    uint32 GetTimer() const
    {
        return _time;
    }

    /**
     * @name GetPhaseMask
     * @return Active phases as mask.
     */
    uint8 GetPhaseMask() const
    {
        return _phase;
    }

    /**
     * @name Empty
     * @return True, if there are no events scheduled.
     */
    bool Empty() const
    {
        return _eventMap.empty();
    }

    /**
     * @name SetPhase
     * @brief Sets the phase of the map (absolute).
     * @param phase Phase which should be set. Values: 1 - 8. 0 resets phase.
     */
    void SetPhase(uint8 phase)
    {
        if (!phase)
            _phase = 0;
        else if (phase <= 8)
            _phase = (1 << (phase - 1));
    }

    /**
     * @name AddPhase
     * @brief Activates the given phase (bitwise).
     * @param phase Phase which should be activated. Values: 1 - 8
     */
    void AddPhase(uint8 phase)
    {
        if (phase && phase <= 8)
            _phase |= (1 << (phase - 1));
    }

    /**
     * @name RemovePhase
     * @brief Deactivates the given phase (bitwise).
     * @param phase Phase which should be deactivated. Values: 1 - 8.
     */
    void RemovePhase(uint8 phase)
    {
        if (phase && phase <= 8)
            _phase &= ~(1 << (phase - 1));
    }

    /**
     * @name ScheduleEvent
     * @brief Creates new event entry in map.
     * @param eventId The id of the new event.
     * @param time The time in milliseconds until the event occurs.
     * @param group The group which the event is associated to. Has to be between 1 and 8. 0 means it has no group.
     * @param phase The phase in which the event can occur. Has to be between 1 and 8. 0 means it can occur in all phases.
     */
    void ScheduleEvent(uint32 eventId, uint32 time, uint32 group = 0, uint8 phase = 0)
    {
        if (group && group <= 8)
            eventId |= (1 << (group + 15));

        if (phase && phase <= 8)
            eventId |= (1 << (phase + 23));

        _eventMap.insert(EventStore::value_type(_time + time, eventId));
    }

    /**
     * @name RescheduleEvent
     * @brief Cancels the given event and reschedules it.
     * @param eventId The id of the event.
     * @param time The time in milliseconds until the event occurs.
     * @param group The group which the event is associated to. Has to be between 1 and 8. 0 means it has no group.
     * @param phase The phase in which the event can occur. Has to be between 1 and 8. 0 means it can occur in all phases.
     */
    void RescheduleEvent(uint32 eventId, uint32 time, uint32 group = 0, uint8 phase = 0)
    {
        CancelEvent(eventId);
        ScheduleEvent(eventId, time, group, phase);
    }

    /**
     * @name RepeatEvent
     * @brief Cancels the closest event and reschedules it.
     * @param time Time until the event occurs.
     */
    void RepeatEvent(uint32 time)
    {
        if (Empty())
            return;

        uint32 eventId = _eventMap.begin()->second;
        _eventMap.erase(_eventMap.begin());
        ScheduleEvent(eventId, time);
    }

    /**
     * @name PopEvent
     * @brief Remove the first event in the map.
     */
    void PopEvent()
    {
        if (!Empty())
            _eventMap.erase(_eventMap.begin());
    }

    /**
     * @name ExecuteEvent
     * @brief Returns the next event to execute and removes it from map.
     * @return Id of the event to execute.
     */
    uint32 ExecuteEvent()
    {
        while (!Empty())
        {
            EventStore::iterator itr = _eventMap.begin();

            if (itr->first > _time)
                return 0;
            else if (_phase && (itr->second & 0xFF000000) && !((itr->second >> 24) & _phase))
                _eventMap.erase(itr);
            else
            {
                uint32 eventId = (itr->second & 0x0000FFFF);
                _eventMap.erase(itr);
                return eventId;
            }
        }

        return 0;
    }

    /**
     * @name GetEvent
     * @brief Returns the next event to execute.
     * @return Id of the event to execute.
     */
    uint32 GetEvent()
    {
        while (!Empty())
        {
            EventStore::iterator itr = _eventMap.begin();

            if (itr->first > _time)
                return 0;
            else if (_phase && (itr->second & 0xFF000000) && !(itr->second & (_phase << 24)))
                _eventMap.erase(itr);
            else
                return (itr->second & 0x0000FFFF);
        }

        return 0;
    }

    /**
     * @name DelayEvents
     * @brief Delays all events in the map. If delay is greater than or equal internal timer, delay will be 0.
     * @param delay Amount of delay.
     */
    void DelayEvents(uint32 delay)
    {
        _time = delay < _time ? _time - delay : 0;
    }

    /**
     * @name DelayEvents
     * @brief Delay all events of the same group.
     * @param delay Amount of delay.
     * @param group Group of the events.
     */
    void DelayEvents(uint32 delay, uint32 group)
    {
        if (!group || group > 8 || Empty())
            return;

        EventStore delayed;

        for (EventStore::iterator itr = _eventMap.begin(); itr != _eventMap.end();)
        {
            if (itr->second & (1 << (group + 15)))
            {
                delayed.insert(EventStore::value_type(itr->first + delay, itr->second));
                itr = _eventMap.erase(itr);
            }
            else
                ++itr;
        }

        _eventMap.insert(delayed.begin(), delayed.end());
    }

    /**
     * @name CancelEvent
     * @brief Cancels all events of the specified id.
     * @param eventId Event id to cancel.
     */
    void CancelEvent(uint32 eventId)
    {
        if (Empty())
            return;

        for (EventStore::iterator itr = _eventMap.begin(); itr != _eventMap.end();)
        {
            if (eventId == (itr->second & 0x0000FFFF))
                itr = _eventMap.erase(itr);
            else
                ++itr;
        }
    }

    /**
     * @name CancelEventGroup
     * @brief Cancel events belonging to specified group.
     * @param group Group to cancel.
     */
    void CancelEventGroup(uint32 group)
    {
        if (!group || group > 8 || Empty())
            return;

        for (EventStore::iterator itr = _eventMap.begin(); itr != _eventMap.end();)
        {
            if (itr->second & (1 << (group + 15)))
                itr = _eventMap.erase(itr);
            else
                ++itr;
        }
    }

    /**
     * @name GetNextEventTime
     * @brief Returns closest occurence of specified event.
     * @param eventId Wanted event id.
     * @return Time of found event.
     */
    uint32 GetNextEventTime(uint32 eventId) const
    {
        if (Empty())
            return 0;

        for (EventStore::const_iterator itr = _eventMap.begin(); itr != _eventMap.end(); ++itr)
            if (eventId == (itr->second & 0x0000FFFF))
                return itr->first;

        return 0;
    }

    /**
     * @name GetNextEventTime
     * @return Time of next event.
     */
    uint32 GetNextEventTime() const
    {
        return Empty() ? 0 : _eventMap.begin()->first;
    }

    /**
     * @name IsInPhase
     * @brief Returns wether event map is in specified phase or not.
     * @param phase Wanted phase.
     * @return True, if phase of event map contains specified phase.
     */
    bool IsInPhase(uint8 phase) const
    {
        return phase <= 8 && (!phase || _phase & (1 << (phase - 1)));
    }

private:
    /**
     * @name _time
     * @brief Internal timer.
     *
     * This does not represent the real date/time value.
     * It's more like a stopwatch: It can run, it can be stopped,
     * it can be resetted and so on. Events occur when this timer
     * has reached their time value. Its value is changed in the
     * Update method.
     */
    uint32 _time;

    /**
     * @name _phase
     * @brief Phase mask of the event map.
     *
     * Contains the phases the event map is in. Multiple
     * phases from 1 to 8 can be set with SetPhase or
     * AddPhase. RemovePhase deactives a phase.
     */
    uint8 _phase;

    /**
     * @name _eventMap
     * @brief Internal event storage map. Contains the scheduled events.
     *
     * See typedef at the beginning of the class for more
     * details.
     */
    EventStore _eventMap;
};

#endif // TRINITY_GAME_AI_EVENT_MAP_HPP
