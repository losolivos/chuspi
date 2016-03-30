#include "GameObjectAI.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "heart_of_fear.h"

// Zorlok - 62980
class boss_shekzeer : public CreatureScript
{
    public:
        boss_shekzeer() : CreatureScript("boss_shekzeer") { }

        struct boss_shekzeerAI : public BossAI
        {
            boss_shekzeerAI(Creature* creature) : BossAI(creature, DATA_SHEKZEER)
            {
                pInstance = creature->GetInstanceScript();
            }

            InstanceScript* pInstance;
            EventMap events;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_shekzeerAI(creature);
        }
};

void AddSC_boss_shekzeer()
{
    new boss_shekzeer();
}