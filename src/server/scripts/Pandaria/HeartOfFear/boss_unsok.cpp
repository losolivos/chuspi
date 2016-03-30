#include "GameObjectAI.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "heart_of_fear.h"

// Zorlok - 62980
class boss_unsok : public CreatureScript
{
    public:
        boss_unsok() : CreatureScript("boss_unsok") { }

        struct boss_unsokAI : public BossAI
        {
            boss_unsokAI(Creature* creature) : BossAI(creature, DATA_UNSOK)
            {
                pInstance = creature->GetInstanceScript();
            }

            InstanceScript* pInstance;
            EventMap events;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_unsokAI(creature);
        }
};

void AddSC_boss_unsok()
{
    new boss_unsok();
}