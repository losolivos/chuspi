#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"


enum
{
};

// 337 - Angelic Feather
class sat_angelic_feather : public SpellAreaTriggerScript
{
public:
    sat_angelic_feather() : SpellAreaTriggerScript("sat_angelic_feather") {}

    class sat_angelic_feather_interface : public IAreaTriggerOnce
    {
        bool CheckTriggering(WorldObject* triggering)
        {
            Unit* unit = triggering->ToUnit();
            if (!unit)
                return false;

            if (unit->isInRoots())
                return false;

            return unit->IsAlive() && m_target->IsWithinDistInMap(unit, m_range) && m_caster->IsFriendlyTo(unit);
        }

        void OnTrigger(WorldObject* triggering)
        {
            // m_caster->CastSpell(triggering->ToUnit(), xxxxxx);
        }
    };

    IAreaTrigger* GetInterface() const override
    {
        return new sat_angelic_feather_interface();
    }
};


void AddSC_spell_areatrigger_scripts()
{
    new sat_angelic_feather();
}
