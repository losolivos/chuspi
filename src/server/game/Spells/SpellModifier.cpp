#include "SpellModifier.hpp"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "SpellAuraEffects.h"

bool SpellModifier::isAffectingSpell(SpellInfo const *spell) const
{
    if (!spell->IsAffectedBySpellMods())
        return false;

    // False if affect_spell == NULL or spellFamily not equal
    auto const affectSpell = sSpellMgr->GetSpellInfo(ownerEffect->GetId());
    if (!affectSpell || affectSpell->SpellFamilyName != spell->SpellFamilyName)
        return false;

    if (mask & spell->SpellFamilyFlags)
        return true;

    return false;
}
