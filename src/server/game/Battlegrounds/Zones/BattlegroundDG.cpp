#include "BattlegroundDG.h"
#include "Player.h"
#include "Language.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "ScriptedCreature.h"
#include "BattlegroundMgr.h"
#include "Battleground.h"

BattlegroundDG::BattlegroundDG()
{
    m_IsInformedNearVictory = false;
    m_BuffChange = true;

    BgObjects.resize(BG_DG_OBJECT_MAX);
    BgCreatures.resize(BG_DG_CREATURES_MAX);

    bool isBGWeekend = sBattlegroundMgr->IsBGWeekend(GetTypeID());
    m_HonorTics = (isBGWeekend) ? BG_DG_BBGWeekendHonorTicks : BG_DG_NotBGWeekendHonorTicks;
}

BattlegroundDG::~BattlegroundDG() { }

void BattlegroundDG::StartingEventCloseDoors()
{
    // despawn everything
    for (int obj = BG_DG_OBJECT_AURA_ALLY; obj < BG_DG_OBJECT_MAX; ++obj)
        SpawnBGObject(obj, RESPAWN_ONE_DAY);
    for (int node = BG_DG_NODE_GOBLIN_MINE; node < BG_DG_ALL_NODES_COUNT; ++node)
    {
        // spawn collision PJ
        SpawnBGObject((node * BG_DG_OBJECT_DYNAMIC_TOTAL) + BG_DG_OBJECT_PJ_COLLISION, RESPAWN_IMMEDIATELY);
        //Spawn neutral aura
        SpawnBGObject((node * BG_DG_OBJECT_DYNAMIC_TOTAL) + BG_DG_OBJECT_AURA_CONTESTED, RESPAWN_IMMEDIATELY);
        //Add capt points auras
        if (Creature* captPoint = GetBgMap()->GetCreature(BgCreatures[BG_DG_OBJECT_CAPT_POINT_START + node]))
            captPoint->CastSpell(captPoint, BG_DG_CAPT_POINT_NEUTRAL, true);
    }

    //Spawn carts
    for (uint32 cart = BG_DG_OBJECT_CART_ALLIANCE; cart <= BG_DG_OBJECT_CART_HORDE; ++cart)
        SpawnBGObject(cart, RESPAWN_IMMEDIATELY);

    //Spawn Buffs
    for (uint32 buff = 0; buff < MAX_BUFFS; buff++)
        SpawnBGObject(BG_DG_OBJECT_BUFF_NORTH + buff, RESPAWN_IMMEDIATELY);

    // Starting doors
    for (int doorType = BG_DG_OBJECT_GATE_1; doorType <= BG_DG_OBJECT_GATE_4; ++doorType)
    {
        DoorClose(doorType);
        SpawnBGObject(doorType, RESPAWN_IMMEDIATELY);
    }
}

void BattlegroundDG::StartingEventOpenDoors()
{
    //Open doors
    for (int doorType = BG_DG_OBJECT_GATE_1; doorType <= BG_DG_OBJECT_GATE_4; ++doorType)
        DoorOpen(doorType);
    //Force update of nodes
    for (int i = 0; i < BG_DG_ALL_NODES_COUNT; ++i)
        if (Creature* captPoint = GetBgMap()->GetCreature(BgCreatures[BG_DG_OBJECT_CAPT_POINT_START + i]))
            for (BattlegroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
                if (Player* player = ObjectAccessor::FindPlayer(MAKE_NEW_GUID(itr->first, 0, HIGHGUID_PLAYER)))
                    captPoint->SendUpdateToPlayer(player);

}

void BattlegroundDG::StartingEventDespawnDoors()
{
    for (uint8 doorType = BG_DG_OBJECT_GATE_1; doorType <= BG_DG_OBJECT_GATE_4; ++doorType)
        DoorDespawn(doorType);
}

void BattlegroundDG::AddPlayer(Player* player)
{
    Battleground::AddPlayer(player);
    BattlegroundDGScore* score = new BattlegroundDGScore();
    PlayerScores[player->GetGUID()] = score;
}

void BattlegroundDG::PostUpdateImpl(uint32 diff)
{
    if (GetStatus() == STATUS_IN_PROGRESS)
    {
        if (GetElapsedTime() >= 27*MINUTE*IN_MILLISECONDS)
        {
            if (GetTeamScore(ALLIANCE) == 0)
            {
                if (GetTeamScore(HORDE) == 0)        // No one scored - result is tie
                    EndBattleground(WINNER_NONE);
                else                                 // Horde has more points and thus wins
                    EndBattleground(HORDE);
            }
            else if (GetTeamScore(HORDE) == 0)
                EndBattleground(ALLIANCE);           // Alliance has > 0, Horde has 0, alliance wins
            else if (GetTeamScore(HORDE) == GetTeamScore(ALLIANCE)) // Team score equal, winner is team that scored the last flag
                EndBattleground(_lastFlagCaptureTeam);
            else if (GetTeamScore(HORDE) > GetTeamScore(ALLIANCE))  // Last but not least, check who has the higher score
                EndBattleground(HORDE);
            else
                EndBattleground(ALLIANCE);
        }
        // first update needed after 1 minute of game already in progress
        else if (GetElapsedTime() > uint32(_minutesElapsed * MINUTE * IN_MILLISECONDS) +  3 * MINUTE * IN_MILLISECONDS)
        {
            ++_minutesElapsed;
            UpdateWorldState(4248, 25 - _minutesElapsed);
        }

        _postUpdateImpl_Flags(diff);
        _postUpdateImpl_Cart(diff);
    }
}

void BattlegroundDG::FillInitialWorldStates(WorldPacket& data)
{
    for (uint8 node = 0; node < BG_DG_ALL_NODES_COUNT; ++node)
    {
        for (uint8 status = BG_DG_NODE_TYPE_NEUTRAL; status <= BG_DG_NODE_STATUS_HORDE_OCCUPIED; status++)
        {
            if (m_Nodes[node] == status)
                data << uint32(BG_DG_NodesWs[node][status].worldstate) << uint32(BG_DG_NodesWs[node][status].valueToSendToShow);
            else if (status >= BG_DG_NODE_TYPE_OCCUPIED)
            {
                //this is there because the occupied worldstate is the same for ally & horde, so we dont want to worldstate that erase the good value
                //if the node is correct status, send the value, else if status not occuped send 0 (to hide)
                //BUT IF ITS OCCUPED, SEND 0 ONLY IF THE NODE ITSELF ISNT OCCUPED
                if (m_Nodes[node] < BG_DG_NODE_TYPE_OCCUPIED)
                    data << uint32(BG_DG_NodesWs[node][status].worldstate) << uint32(0);
            }
            else
                data << uint32(BG_DG_NodesWs[node][status].worldstate) << uint32(0);
        }
    }

    TC_LOG_ERROR("general", "Sending world states... #2");

    // How many bases each team owns
    uint8 ally = 0, horde = 0;
    for (uint8 node = 0; node < BG_DG_ALL_NODES_COUNT; ++node)
    {
        if (m_Nodes[node] == BG_DG_NODE_STATUS_ALLY_OCCUPIED)
            ++ally;
        else if (m_Nodes[node] == BG_DG_NODE_STATUS_HORDE_OCCUPIED)
            ++horde;
    }

    data << uint32(WORLDSTATE_DG_OCCUPIED_BASES_ALLIANCE) << uint32(ally);
    data << uint32(WORLDSTATE_DG_OCCUPIED_BASES_HORDE) << uint32(horde);

    //Team Score
    data << uint32(WORLDSTATE_DG_SCORE_ALLIANCE) << uint32(m_TeamScores[TEAM_ALLIANCE]);
    data << uint32(WORLDSTATE_DG_SCORE_HORDE) << uint32(m_TeamScores[TEAM_HORDE]);

    //Cart state
    data << uint32(WORLDSTATE_DG_CART_STATE_ALLIANCE) << uint32(_flagState[TEAM_ALLIANCE] == BG_DG_CART_STATE_ON_PLAYER ? 2 : 1);
    data << uint32(WORLDSTATE_DG_CART_STATE_HORDE) << uint32(_flagState[TEAM_HORDE] == BG_DG_CART_STATE_ON_PLAYER ? 2 : 1);

    data << uint32(WORLDSTATE_DG_TIMER_SHOW) << uint32(1);
    data << uint32(WORLDSTATE_DG_TIMER_MINUTES) << uint32(25-_minutesElapsed);
}

void BattlegroundDG::UpdatePlayerScore(Player* source, uint32 type, uint32 value, bool doAddHonor)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    BattlegroundScoreMap::iterator itr = PlayerScores.find(source->GetGUID());
    if (itr == PlayerScores.end())                         // player not found...
        return;

    uint32 teamIndex = GetTeamIndexByTeamId(source->GetTeam());
    switch (type)
    {
        case SCORE_CART_CAPTURES:
            ((BattlegroundDGScore*)itr->second)->cartCaptured += value;
            break;
        case SCORE_CART_RETURNS:
            ((BattlegroundDGScore*)itr->second)->cartReturned += value;
            break;
        case SCORE_BASES_ASSAULTED:
            ((BattlegroundDGScore*)itr->second)->minesAssaulted += value;
            break;
        case SCORE_BASES_DEFENDED:
            ((BattlegroundDGScore*)itr->second)->minesDefended += value;
            break;
        default:
            Battleground::UpdatePlayerScore(source, type, value, doAddHonor);
            break;
    }
}

void BattlegroundDG::HandleKillPlayer(Player* player, Player* killer)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    EventPlayerDroppedFlag(player);

    Battleground::HandleKillPlayer(player, killer);
}

WorldSafeLocsEntry const* BattlegroundDG::GetClosestGraveYard(Player* player)
{
    uint32 teamIndex = GetTeamIndexByTeamId(player->GetTeam());

    WorldSafeLocsEntry const* northern = sWorldSafeLocsStore.LookupEntry(BG_DG_GraveyardIds[teamIndex][0]);
    float distnorthern = player->GetExactDist2dSq(northern->x, northern->y);
    WorldSafeLocsEntry const* southern = sWorldSafeLocsStore.LookupEntry(BG_DG_GraveyardIds[teamIndex][1]);
    float distsouthern = player->GetExactDist2dSq(southern->x, southern->y);

    return distnorthern < distsouthern ? southern : northern;
}

void BattlegroundDG::_UpdateTeamScore(int team, int32 value)
{
    m_TeamScores[team] += value;

    m_TeamScores[team] = std::min(int32(BG_DG_MAX_VICTORY_POINTS), m_TeamScores[team]);
    m_TeamScores[team] = std::max(0, m_TeamScores[team]);

    UpdateWorldState(team == TEAM_ALLIANCE ? WORLDSTATE_DG_SCORE_ALLIANCE : WORLDSTATE_DG_SCORE_HORDE, m_TeamScores[team]);

    if (m_TeamScores[team] == BG_DG_MAX_VICTORY_POINTS)
    {
        //UpdateWorldState(BG_WS_FLAG_UNK_ALLIANCE, 0);
        //UpdateWorldState(BG_WS_FLAG_UNK_HORDE, 0);
        UpdateWorldState(WORLDSTATE_DG_CART_STATE_ALLIANCE, 1);
        UpdateWorldState(WORLDSTATE_DG_CART_STATE_HORDE, 1);
        EndBattleground(team);
    }

    if (m_IsInformedNearVictory && m_TeamScores[team] < BG_DG_NEAR_VICTORY_POINTS)
        m_IsInformedNearVictory = false;
    else if (!m_IsInformedNearVictory && m_TeamScores[team] > BG_DG_NEAR_VICTORY_POINTS)
    {
        SendMessageToAll(team == TEAM_ALLIANCE ? LANG_BG_DG_ALLIANCE_NEAR_VICTORY : LANG_BG_DG_HORDE_NEAR_VICTORY, CHAT_MSG_BG_SYSTEM_NEUTRAL);
        PlaySoundToAll((team == TEAM_ALLIANCE) ? BG_DG_SOUND_ALLIANCE_NEAR_VICTORY : BG_DG_SOUND_HORDE_NEAR_VICTORY);
        m_IsInformedNearVictory = true;
    }
}

/************************************************************************/
/*                          FLAGS UPDATES                               */
/************************************************************************/

void BattlegroundDG::_ChangeBanner(uint8 node, uint8 type)
{
    //1. Manage auras objects
    if (type == BG_DG_NODE_TYPE_CONTESTED && (m_prevNodes[node] == BG_DG_NODE_STATUS_ALLY_OCCUPIED || m_prevNodes[node] == BG_DG_NODE_STATUS_HORDE_OCCUPIED))
    {
        //Capt point goes from occuped to contested
        //1.1. Despawn controlled aura
        SpawnBGObject(node * BG_DG_OBJECT_DYNAMIC_TOTAL + BG_DG_OBJECT_AURA_ALLY + (m_Nodes[node] - 1), RESPAWN_ONE_DAY);
        //1.2. Spawn contested aura
        SpawnBGObject(node * BG_DG_OBJECT_DYNAMIC_TOTAL + BG_DG_OBJECT_AURA_CONTESTED, RESPAWN_IMMEDIATELY);
    }
    else if (type == BG_DG_NODE_TYPE_OCCUPIED && (m_prevNodes[node] == BG_DG_NODE_STATUS_ALLY_CONTESTED || m_prevNodes[node] == BG_DG_NODE_STATUS_HORDE_CONTESTED))
    {
        //Capt point goes from contested to occuped
        //1.1. Despawn contested aura
        SpawnBGObject(node * BG_DG_OBJECT_DYNAMIC_TOTAL + BG_DG_OBJECT_AURA_CONTESTED, RESPAWN_ONE_DAY);
        //1.2. Spawn controlled aura
        SpawnBGObject(node * BG_DG_OBJECT_DYNAMIC_TOTAL + BG_DG_OBJECT_AURA_ALLY + (m_Nodes[node] - 1), RESPAWN_IMMEDIATELY);
    }

    //2. Manage the capt point
    if (Creature* captPoint = GetBgMap()->GetCreature(BgCreatures[BG_DG_OBJECT_CAPT_POINT_START + node]))
    {
        //2.1. Remove the previous aura
        uint32 removeAura = BG_DG_CAPT_POINT_NEUTRAL;
        switch (m_prevNodes[node])
        {
            case BG_DG_NODE_STATUS_ALLY_CONTESTED:  removeAura = BG_DG_CAPT_POINT_ALLIANCE_CONTEST; break;
            case BG_DG_NODE_STATUS_HORDE_CONTESTED: removeAura = BG_DG_CAPT_POINT_HORDE_CONTEST;    break;
            case BG_DG_NODE_STATUS_ALLY_OCCUPIED:   removeAura = BG_DG_CAPT_POINT_ALLIANCE_CONTROL; break;
            case BG_DG_NODE_STATUS_HORDE_OCCUPIED:  removeAura = BG_DG_CAPT_POINT_HORDE_CONTROL;    break;
            default: break;
        }

        captPoint->RemoveAura(removeAura);
        //2.2. Add the new aura
        uint32 addAura = BG_DG_CAPT_POINT_NEUTRAL;
        switch (m_Nodes[node])
        {
            case BG_DG_NODE_STATUS_ALLY_CONTESTED:  addAura = BG_DG_CAPT_POINT_ALLIANCE_CONTEST; break;
            case BG_DG_NODE_STATUS_HORDE_CONTESTED: addAura = BG_DG_CAPT_POINT_HORDE_CONTEST;    break;
            case BG_DG_NODE_STATUS_ALLY_OCCUPIED:   addAura = BG_DG_CAPT_POINT_ALLIANCE_CONTROL; break;
            case BG_DG_NODE_STATUS_HORDE_OCCUPIED:  addAura = BG_DG_CAPT_POINT_HORDE_CONTROL;    break;
            default: break;
        }

        switch (m_Nodes[node])
        {
            case BG_DG_NODE_STATUS_ALLY_CONTESTED:
            case BG_DG_NODE_STATUS_ALLY_OCCUPIED:
                captPoint->setFaction(BG_DG_FACTION_FLAG_HORDE);
                break;
            case BG_DG_NODE_STATUS_HORDE_CONTESTED:
            case BG_DG_NODE_STATUS_HORDE_OCCUPIED:  
                captPoint->setFaction(BG_DG_FACTION_FLAG_ALLIANCE);
                break;
            default: break;
        }

        captPoint->CastSpell(captPoint, addAura, true);
        //2.3 Force update to players
        for (BattlegroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
            if (Player* player = ObjectAccessor::FindPlayer(MAKE_NEW_GUID(itr->first, 0, HIGHGUID_PLAYER)))
                captPoint->SendUpdateToPlayer(player);
    }
}

void BattlegroundDG::_SendNodeUpdate(uint8 node)
{
    //Node map
    UpdateWorldState(BG_DG_NodesWs[node][m_prevNodes[node]].worldstate, 0);
    UpdateWorldState(BG_DG_NodesWs[node][m_Nodes[node]].worldstate, BG_DG_NodesWs[node][m_Nodes[node]].valueToSendToShow);

    // How many bases each team owns
    uint8 ally = 0, horde = 0;
    for (uint8 i = 0; i < BG_DG_ALL_NODES_COUNT; ++i)
    if (m_Nodes[i] == BG_DG_NODE_STATUS_ALLY_OCCUPIED)
        ++ally;
    else if (m_Nodes[i] == BG_DG_NODE_STATUS_HORDE_OCCUPIED)
        ++horde;

    UpdateWorldState(WORLDSTATE_DG_OCCUPIED_BASES_ALLIANCE, ally);
    UpdateWorldState(WORLDSTATE_DG_OCCUPIED_BASES_HORDE, horde);
}

bool BattlegroundDG::CanSeeSpellClick(Player const* player, Unit const* clicked)
{
    if (GetStatus() != STATUS_IN_PROGRESS) return false;

    for (int i = 0; i < BG_DG_ALL_NODES_COUNT; ++i)
    {
        if (clicked->GetGUID() == BgCreatures[BG_DG_OBJECT_CAPT_POINT_START + i])
        {
            BattlegroundTeamId teamIndex = GetTeamIndexByTeamId(player->GetTeam());
            // Check if player really could use this banner, not cheated
            // Horde team give 0 as rest of modulo (resp. 1 for ally), but horde is 1 as a team (resp. 0 for ally)
            // So if teamIndex != m_Nodes[node] % 2 means that ur team HAS the flag
            if (m_Nodes[i] != BG_DG_NODE_TYPE_NEUTRAL && teamIndex != m_Nodes[i] % 2)
                return false;

            return true;
        }
    }

    return false;
}

void BattlegroundDG::EventPlayerClickedOnFlag(Player* source, Unit* flag)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    uint8 node = BG_DG_NODE_GOBLIN_MINE;
    Creature* captPoint = GetBgMap()->GetCreature(BgCreatures[BG_DG_OBJECT_CAPT_POINT_START + node]);
    while ((node < BG_DG_ALL_NODES_COUNT) && ((!captPoint) || (!source->IsWithinDistInMap(captPoint, 10))))
    {
        ++node;
        captPoint = GetBgMap()->GetCreature(BgCreatures[BG_DG_OBJECT_CAPT_POINT_START + node]);
    }

    // this means our player isn't close to any of banners - maybe cheater ??
    if (node == BG_DG_ALL_NODES_COUNT) return;

    BattlegroundTeamId teamIndex = GetTeamIndexByTeamId(source->GetTeam());

    // Check if player really could use this banner, not cheated
    // Horde team give 0 as rest of modulo (resp. 1 for ally), but horde is 1 as a team (resp. 0 for ally)
    // So if teamIndex != m_Nodes[node] % 2 means that ur team HAS the flag
    if (m_Nodes[node] != BG_DG_NODE_TYPE_NEUTRAL && teamIndex != m_Nodes[node] % 2)
        return;

    source->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);
    uint32 sound = 0;
    // If node is neutral, change to contested
    if (m_Nodes[node] == BG_DG_NODE_TYPE_NEUTRAL)
    {
        UpdatePlayerScore(source, SCORE_BASES_ASSAULTED, 1);
        m_prevNodes[node] = m_Nodes[node];
        m_Nodes[node] = BG_DG_NODE_TYPE_CONTESTED + teamIndex;
        // create new contested banner
        _ChangeBanner(node, BG_DG_NODE_TYPE_CONTESTED);
        _SendNodeUpdate(node);
        m_NodeTimers[node] = BG_DG_FLAG_CAPTURING_TIME;
        // warn players
        SendMessage2ToAll(LANG_BG_DG_NODE_CLAIMED, ChatMsg(CHAT_MSG_BG_SYSTEM_ALLIANCE + teamIndex), source, _GetNodeNameId(node), LANG_BG_DG_ALLY + teamIndex);
        sound = BG_DG_SOUND_NODE_CLAIMED;
    }
    // If node is contested
    else if ((m_Nodes[node] == BG_DG_NODE_STATUS_ALLY_CONTESTED) || (m_Nodes[node] == BG_DG_NODE_STATUS_HORDE_CONTESTED))
    {
        // If last state is NOT occupied, change node to enemy-contested
        if (m_prevNodes[node] < BG_DG_NODE_TYPE_OCCUPIED)
        {
            UpdatePlayerScore(source, SCORE_BASES_ASSAULTED, 1);
            m_prevNodes[node] = m_Nodes[node];
            m_Nodes[node] = BG_DG_NODE_TYPE_CONTESTED + teamIndex;
            // create new contested banner
            _ChangeBanner(node, BG_DG_NODE_TYPE_CONTESTED);
            _SendNodeUpdate(node);
            m_NodeTimers[node] = BG_DG_FLAG_CAPTURING_TIME;

            SendMessage2ToAll(LANG_BG_DG_NODE_CLAIMED, ChatMsg(CHAT_MSG_BG_SYSTEM_ALLIANCE + teamIndex), source, _GetNodeNameId(node), LANG_BG_DG_ALLY + teamIndex);
        }
        // If contested, change back to occupied
        else
        {
            UpdatePlayerScore(source, SCORE_BASES_DEFENDED, 1);
            m_prevNodes[node] = m_Nodes[node];
            m_Nodes[node] = BG_DG_NODE_TYPE_OCCUPIED + teamIndex;
            // create new occupied banner
            _ChangeBanner(node, BG_DG_NODE_TYPE_OCCUPIED);
            _SendNodeUpdate(node);
            m_NodeTimers[node] = 0;

            SendMessage2ToAll(LANG_BG_DG_NODE_DEFENDED, ChatMsg(CHAT_MSG_BG_SYSTEM_ALLIANCE + teamIndex), source, _GetNodeNameId(node), LANG_BG_DG_ALLY + teamIndex);
        }
        sound = (teamIndex == TEAM_ALLIANCE) ? BG_DG_SOUND_NODE_ASSAULTED_ALLIANCE : BG_DG_SOUND_NODE_ASSAULTED_HORDE;
    }
    // If node is occupied, change to enemy-contested
    else
    {
        UpdatePlayerScore(source, SCORE_BASES_ASSAULTED, 1);
        m_prevNodes[node] = m_Nodes[node];
        m_Nodes[node] = teamIndex + BG_DG_NODE_TYPE_CONTESTED;
        // create new contested banner
        _ChangeBanner(node, BG_DG_NODE_TYPE_CONTESTED);
        _SendNodeUpdate(node);
        m_NodeTimers[node] = BG_DG_FLAG_CAPTURING_TIME;

        SendMessage2ToAll(LANG_BG_DG_NODE_ASSAULTED, ChatMsg(CHAT_MSG_BG_SYSTEM_ALLIANCE + teamIndex), source, _GetNodeNameId(node), LANG_BG_DG_ALLY + teamIndex);

        sound = (teamIndex == TEAM_ALLIANCE) ? BG_DG_SOUND_NODE_ASSAULTED_ALLIANCE : BG_DG_SOUND_NODE_ASSAULTED_HORDE;
    }

    // If node is occupied again, send "X has taken the Y" msg.
    if (m_Nodes[node] >= BG_DG_NODE_TYPE_OCCUPIED)
        SendMessage2ToAll(LANG_BG_DG_NODE_TAKEN, ChatMsg(CHAT_MSG_BG_SYSTEM_ALLIANCE + teamIndex), source, _GetNodeNameId(node), LANG_BG_DG_ALLY + teamIndex);

    PlaySoundToAll(sound);
}

void BattlegroundDG::_postUpdateImpl_Flags(uint32 diff)
{
    int team_points[BG_TEAMS_COUNT] = { 0, 0 };

    for (int node = 0; node < BG_DG_ALL_NODES_COUNT; ++node)
    {
        // 1-minute to occupy a node from contested state
        _contestedTime(node, diff);

        for (int team = 0; team < BG_TEAMS_COUNT; ++team)
        if (m_Nodes[node] == team + BG_DG_NODE_TYPE_OCCUPIED)
            ++team_points[team];
    }

    // Accumulate points
    for (int team = 0; team < BG_TEAMS_COUNT; ++team)
    {
        int points = team_points[team];
        if (!points) continue;

        m_lastTick[team] += diff;

        if (m_lastTick[team] > BG_DG_TickIntervals[points])
        {
            m_lastTick[team] -= BG_DG_TickIntervals[points];
            m_HonorScoreTics[team] += BG_DG_TickPoints[points];
            if (m_HonorScoreTics[team] >= m_HonorTics)
            {
                RewardHonorToTeam(uint32(GetBonusHonorFromKill(10) / 100), (team == TEAM_ALLIANCE) ? ALLIANCE : HORDE);
                m_HonorScoreTics[team] -= m_HonorTics;
            }

            //need to be last because there can be a last tick for honor before end (cart give 200, it's a lot)
            _UpdateTeamScore(team, BG_DG_TickPoints[points]);
        }
    }
}

void BattlegroundDG::_contestedTime(int node, uint32 diff)
{
    if (m_NodeTimers[node] == 0) return; //time already elapsed

    if (m_NodeTimers[node] > diff)
        m_NodeTimers[node] -= diff;
    else
    {
        m_NodeTimers[node] = 0;
        // Change from contested to occupied !
        uint8 teamIndex = m_Nodes[node] - 1;
        m_prevNodes[node] = m_Nodes[node];
        m_Nodes[node] += 2;
        // create new occupied banner
        _ChangeBanner(node, BG_DG_NODE_TYPE_OCCUPIED);
        _SendNodeUpdate(node);
        // Message to chatlog
        SendMessage2ToAll(LANG_BG_DG_NODE_TAKEN, ChatMsg(CHAT_MSG_BG_SYSTEM_ALLIANCE + teamIndex), NULL, LANG_BG_DG_ALLY + teamIndex, _GetNodeNameId(node));
        PlaySoundToAll((teamIndex == TEAM_ALLIANCE) ? BG_DG_SOUND_NODE_CAPTURED_ALLIANCE : BG_DG_SOUND_NODE_CAPTURED_HORDE);
    }
}

int32 BattlegroundDG::_GetNodeNameId(uint8 node)
{
    switch (node)
    {
    case BG_DG_NODE_GOBLIN_MINE:       return LANG_BG_DG_NODE_SOUTH_MINE;
    case BG_DG_NODE_CENTER_MINE:       return LANG_BG_DG_NODE_CENTER;
    case BG_DG_NODE_PANDAREN_MINE:     return LANG_BG_DG_NODE_NORTH_MINE;
    default:
        ASSERT(false);
    }
    return 0;
}

/************************************************************************/
/*                          CARTS UPDATES                               */
/************************************************************************/

void BattlegroundDG::EventPlayerClickedOnFlag(Player* player, GameObject* target_obj)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    int32 message_id = 0;
    ChatMsg type = CHAT_MSG_BG_SYSTEM_NEUTRAL;

    //alliance flag picked up from base
    if (player->GetTeam() == HORDE && GetFlagState(ALLIANCE) == BG_DG_CART_STATE_ON_BASE
        && BgObjects[BG_DG_OBJECT_CART_ALLIANCE] == target_obj->GetGUID())
    {
        message_id = LANG_BG_DG_PICKEDUP_ALLIANCE_CART;
        type = CHAT_MSG_BG_SYSTEM_HORDE;
        PlaySoundToAll(BG_DG_SOUND_ALLIANCE_CART_PICKED_UP);
        SpawnBGObject(BG_DG_OBJECT_CART_ALLIANCE, RESPAWN_ONE_DAY);
        SetAllianceCartPicker(player->GetGUID());
        _flagState[TEAM_ALLIANCE] = BG_DG_CART_STATE_ON_PLAYER;
        //update world state to show correct flag carrier
        UpdateCartState(HORDE, BG_DG_CART_STATE_ON_PLAYER);
        //UpdateWorldState(BG_WS_FLAG_UNK_ALLIANCE, 1);
        player->CastSpell(player, BG_DG_ALLIANCE_MINE_CART, true);
        player->CastSpell(player, BG_DG_ALLIANCE_CART_HOLDER_AURA, true);
        _flagGold[TEAM_ALLIANCE] = (m_TeamScores[TEAM_ALLIANCE] > 200) ? 200 : m_TeamScores[TEAM_ALLIANCE];
        _UpdateTeamScore(TEAM_ALLIANCE, -_flagGold[TEAM_ALLIANCE]);
    }

    //horde flag picked up from base
    if (player->GetTeam() == ALLIANCE && GetFlagState(HORDE) == BG_DG_CART_STATE_ON_BASE
        && BgObjects[BG_DG_OBJECT_CART_HORDE] == target_obj->GetGUID())
    {
        message_id = LANG_BG_DG_PICKEDUP_HORDE_CART;
        type = CHAT_MSG_BG_SYSTEM_ALLIANCE;
        PlaySoundToAll(BG_DG_SOUND_HORDE_CART_PICKED_UP);
        SpawnBGObject(BG_DG_OBJECT_CART_HORDE, RESPAWN_ONE_DAY);
        SetHordeFlagPicker(player->GetGUID());
        _flagState[TEAM_HORDE] = BG_DG_CART_STATE_ON_PLAYER;
        //update world state to show correct flag carrier
        UpdateCartState(ALLIANCE, BG_DG_CART_STATE_ON_PLAYER);
        //UpdateWorldState(BG_WS_FLAG_UNK_HORDE, 1);
        player->CastSpell(player, BG_DG_HORDE_MINE_CART, true);
        player->CastSpell(player, BG_DG_HORDE_CART_HOLDER_AURA, true);
        _flagGold[TEAM_HORDE] = (m_TeamScores[TEAM_HORDE] > 200) ? 200 : m_TeamScores[TEAM_HORDE];
        _UpdateTeamScore(TEAM_HORDE, -_flagGold[TEAM_HORDE]);
    }

    //Alliance flag on ground(not in base) (returned or picked up again from ground!)
    if (GetFlagState(ALLIANCE) == BG_DG_CART_STATE_ON_GROUND && player->IsWithinDistInMap(target_obj, 10)
        && target_obj->GetGOInfo()->entry == BG_DG_OBJECTID_CART_ALLY_GROUND)
    {
        if (player->GetTeam() == ALLIANCE)
        {
            message_id = LANG_BG_DG_RETURNED_ALLIANCE_CART;
            type = CHAT_MSG_BG_SYSTEM_ALLIANCE;
            UpdateCartState(HORDE, BG_DG_CART_STATE_ON_BASE);
            _flagState[TEAM_ALLIANCE] = BG_DG_CART_STATE_ON_BASE;
            SpawnBGObject(BG_DG_OBJECT_CART_ALLIANCE, RESPAWN_IMMEDIATELY);
            PlaySoundToAll(BG_DG_SOUND_CART_RETURNED);
            UpdatePlayerScore(player, SCORE_CART_RETURNS, 1);
            _UpdateTeamScore(TEAM_ALLIANCE, _flagGold[TEAM_ALLIANCE]);
            _flagGold[TEAM_ALLIANCE] = 0;
        }
        else
        {
            message_id = LANG_BG_DG_PICKEDUP_ALLIANCE_CART;
            type = CHAT_MSG_BG_SYSTEM_HORDE;
            PlaySoundToAll(BG_DG_SOUND_ALLIANCE_CART_PICKED_UP);
            SpawnBGObject(BG_DG_OBJECT_CART_ALLIANCE, RESPAWN_ONE_DAY);
            SetAllianceCartPicker(player->GetGUID());
            player->CastSpell(player, BG_DG_ALLIANCE_MINE_CART, true);
            player->CastSpell(player, BG_DG_ALLIANCE_CART_HOLDER_AURA, true);
            _flagState[TEAM_ALLIANCE] = BG_DG_CART_STATE_ON_PLAYER;
            UpdateCartState(HORDE, BG_DG_CART_STATE_ON_PLAYER);
            //UpdateWorldState(BG_WS_FLAG_UNK_ALLIANCE, 1);
        }
        target_obj->Delete();
    }

    //Horde flag on ground(not in base) (returned or picked up again)
    if (GetFlagState(HORDE) == BG_DG_CART_STATE_ON_GROUND && player->IsWithinDistInMap(target_obj, 10)
        && target_obj->GetGOInfo()->entry == BG_DG_OBJECTID_CART_HORDE_GROUND)
    {
        if (player->GetTeam() == HORDE)
        {
            message_id = LANG_BG_DG_RETURNED_HORDE_CART;
            type = CHAT_MSG_BG_SYSTEM_HORDE;
            UpdateCartState(ALLIANCE, BG_DG_CART_STATE_ON_BASE);
            _flagState[TEAM_HORDE] = BG_DG_CART_STATE_ON_BASE;
            SpawnBGObject(BG_DG_OBJECT_CART_HORDE, RESPAWN_IMMEDIATELY);
            PlaySoundToAll(BG_DG_SOUND_CART_RETURNED);
            UpdatePlayerScore(player, SCORE_CART_RETURNS, 1);
            _UpdateTeamScore(TEAM_HORDE, _flagGold[TEAM_HORDE]);
            _flagGold[TEAM_HORDE] = 0;
        }
        else
        {
            message_id = LANG_BG_DG_PICKEDUP_HORDE_CART;
            type = CHAT_MSG_BG_SYSTEM_ALLIANCE;
            PlaySoundToAll(BG_DG_SOUND_HORDE_CART_PICKED_UP);
            SpawnBGObject(BG_DG_OBJECT_CART_HORDE, RESPAWN_ONE_DAY);
            SetHordeFlagPicker(player->GetGUID());
            player->CastSpell(player, BG_DG_HORDE_MINE_CART, true);
            player->CastSpell(player, BG_DG_HORDE_CART_HOLDER_AURA, true);
            _flagState[TEAM_HORDE] = BG_DG_CART_STATE_ON_PLAYER;
            UpdateCartState(ALLIANCE, BG_DG_CART_STATE_ON_PLAYER);
            //UpdateWorldState(BG_WS_FLAG_UNK_HORDE, 1);
        }
        target_obj->Delete();
    }

    if (!message_id)
        return;

    SendMessageToAll(message_id, type, player);
    player->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);
}

void BattlegroundDG::HandleAreaTrigger(Player* player, uint32 trigger)
{
    switch (trigger)
    {
    case 9012: //Alliance cart spawn
        if (_flagState[TEAM_HORDE] && !_flagState[TEAM_ALLIANCE])
            if (GetFlagPickerGUID(TEAM_HORDE) == player->GetGUID())
                EventPlayerCapturedFlag(player);
        break;
    case 9013: //horde cart spawn
        if (_flagState[TEAM_ALLIANCE] && !_flagState[TEAM_HORDE])
            if (GetFlagPickerGUID(TEAM_ALLIANCE) == player->GetGUID())
                EventPlayerCapturedFlag(player);
        break;
    case 9139:
    case 9140: 
    case 9159: 
    case 9160: 
    case 9161: 
    case 9162: 
    case 9299: 
    case 9301: 
    case 9302: 
    case 9303: 
        TC_LOG_DEBUG("bg.battleground", "BattlegroundDG : Handled AreaTrigger(ID : %u) have been activated by Player %s (ID : %u) with flags : %u",
            trigger, player->GetName().c_str(), GUID_LOPART(player->GetGUID()));
        break;
    default:
        Battleground::HandleAreaTrigger(player, trigger);
        break;
    }
}

void BattlegroundDG::_postUpdateImpl_Cart(uint32 diff)
{
    if (_flagState[TEAM_ALLIANCE] == BG_DG_CART_STATE_ON_GROUND)
    {
        _flagsDropTimer[TEAM_ALLIANCE] -= diff;

        if (_flagsDropTimer[TEAM_ALLIANCE] <= 0)
        {
            _flagsDropTimer[TEAM_ALLIANCE] = 0;
            RespawnFlagAfterDrop(TEAM_ALLIANCE);
        }
    }

    if (_flagState[TEAM_HORDE] == BG_DG_CART_STATE_ON_GROUND)
    {
        _flagsDropTimer[TEAM_HORDE] -= diff;

        if (_flagsDropTimer[TEAM_HORDE] <= 0)
        {
            _flagsDropTimer[TEAM_HORDE] = 0;
            RespawnFlagAfterDrop(TEAM_HORDE);
        }
    }
}

void BattlegroundDG::RespawnFlagAfterDrop(uint32 team)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    _flagState[team] = BG_DG_CART_STATE_ON_BASE;

    SpawnBGObject(BG_DG_OBJECT_CART_ALLIANCE + team, RESPAWN_IMMEDIATELY);
    SendMessageToAll(LANG_BG_DG_ALLIANCE_FLAG_RESPAWNED + team, CHAT_MSG_BG_SYSTEM_NEUTRAL);

    //PlaySoundToAll(BG_DG_SOUND_CARTS_RESPAWNED);

    if (GameObject* obj = GetBGObject(BG_DG_OBJECT_CART_ALLY_GROUND + team))
        obj->Delete();
    else
        TC_LOG_ERROR("bg.battleground", "Battleground DG : unknown dropped flag bg");
}

void BattlegroundDG::EventPlayerCapturedFlag(Player* player)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    uint32 winner = 0;

    player->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);
    if (player->GetTeam() == ALLIANCE)
    {
        if (!IsHordeFlagPickedup())
            return;
        SetHordeFlagPicker(0);                              // must be before aura remove to prevent 2 events (drop+capture) at the same time
        // horde flag in base (but not respawned yet)
        _flagState[TEAM_HORDE] = BG_DG_CART_STATE_ON_BASE;
        _UpdateTeamScore(TEAM_ALLIANCE, _flagGold[TEAM_HORDE]);
        _flagGold[TEAM_HORDE] = 0;
        // Drop Horde Flag from Player
        player->RemoveAurasDueToSpell(BG_DG_HORDE_MINE_CART);
        player->RemoveAurasDueToSpell(BG_DG_HORDE_CART_HOLDER_AURA);
        PlaySoundToAll(BG_DG_SOUND_CART_CAPTURED_ALLIANCE);
        SpawnBGObject(BG_DG_OBJECT_CART_HORDE, RESPAWN_IMMEDIATELY);
    }
    else
    {
        if (!IsAllianceFlagPickedup())
            return;
        SetAllianceCartPicker(0);                           // must be before aura remove to prevent 2 events (drop+capture) at the same time
        // alliance flag in base (but not respawned yet)
        _flagState[TEAM_ALLIANCE] = BG_DG_CART_STATE_ON_BASE;
        _UpdateTeamScore(TEAM_HORDE, _flagGold[TEAM_ALLIANCE]);
        _flagGold[TEAM_ALLIANCE] = 0;
        // Drop Alliance Flag from Player
        player->RemoveAurasDueToSpell(BG_DG_ALLIANCE_MINE_CART);
        player->RemoveAurasDueToSpell(BG_DG_ALLIANCE_CART_HOLDER_AURA);
        PlaySoundToAll(BG_DG_SOUND_CART_CAPTURED_HORDE);
        SpawnBGObject(BG_DG_OBJECT_CART_ALLIANCE, RESPAWN_IMMEDIATELY);
    }
    //for flag capture is reward 2 honorable kills
    RewardHonorToTeam(GetBonusHonorFromKill(20), player->GetTeam());

    if (player->GetTeam() == ALLIANCE)
        SendMessageToAll(LANG_BG_DG_CAPTURED_HORDE_CART, CHAT_MSG_BG_SYSTEM_ALLIANCE, player);
    else
        SendMessageToAll(LANG_BG_DG_CAPTURED_ALLIANCE_CART, CHAT_MSG_BG_SYSTEM_HORDE, player);

    UpdateCartState(player->GetTeam(), 1);                  // flag state none
    // only flag capture should be updated
    UpdatePlayerScore(player, SCORE_CART_CAPTURES, 1);      // +1 flag captures

    // update last flag capture to be used if teamscore is equal
    SetLastFlagCapture(player->GetTeam());

    _flagsTimer[GetTeamIndexByTeamId(player->GetTeam()) ? 0 : 1] = BG_DG_CART_RESPAWN_TIME;
}

void BattlegroundDG::EventPlayerDroppedFlag(Player* player)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
    {
        // if not running, do not cast things at the dropper player (prevent spawning the "dropped" flag), neither send unnecessary messages
        // just take off the aura
        if (player->GetTeam() == ALLIANCE)
        {
            if (!IsHordeFlagPickedup())
                return;

            if (GetFlagPickerGUID(TEAM_HORDE) == player->GetGUID())
            {
                SetHordeFlagPicker(0);
                player->RemoveAurasDueToSpell(BG_DG_HORDE_MINE_CART);
                player->RemoveAurasDueToSpell(BG_DG_HORDE_CART_HOLDER_AURA);
            }
        }
        else
        {
            if (!IsAllianceFlagPickedup())
                return;

            if (GetFlagPickerGUID(TEAM_ALLIANCE) == player->GetGUID())
            {
                SetAllianceCartPicker(0);
                player->RemoveAurasDueToSpell(BG_DG_ALLIANCE_MINE_CART);
                player->RemoveAurasDueToSpell(BG_DG_ALLIANCE_CART_HOLDER_AURA);
            }
        }
        return;
    }

    bool set = false;

    if (player->GetTeam() == ALLIANCE)
    {
        if (!IsHordeFlagPickedup())
            return;
        if (GetFlagPickerGUID(TEAM_HORDE) == player->GetGUID())
        {
            SetHordeFlagPicker(0);
            player->RemoveAurasDueToSpell(BG_DG_HORDE_MINE_CART);
            player->RemoveAurasDueToSpell(BG_DG_HORDE_CART_HOLDER_AURA);
            _flagState[TEAM_HORDE] = BG_DG_CART_STATE_ON_GROUND;
            if (AddObject(BG_DG_OBJECT_CART_HORDE_GROUND, BG_DG_OBJECTID_CART_HORDE_GROUND, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation(), 0.0f, 0.0f, 0.0f, 0.0f))
            {
                SpawnBGObject(BG_DG_OBJECT_CART_HORDE_GROUND, RESPAWN_IMMEDIATELY);
                //dunno how to cast spell on gob :/
                //GetBGObject(BG_DG_OBJECT_CART_HORDE_GROUND)->CastSpell(GetBGObject(BG_DG_OBJECT_CART_HORDE_GROUND), true);
            }
            _flagsDropTimer[TEAM_HORDE] = BG_DG_CART_DROP_TIME;
            set = true;
        }
    }
    else
    {
        if (!IsAllianceFlagPickedup())
            return;
        if (GetFlagPickerGUID(TEAM_ALLIANCE) == player->GetGUID())
        {
            SetAllianceCartPicker(0);
            player->RemoveAurasDueToSpell(BG_DG_ALLIANCE_MINE_CART);
            player->RemoveAurasDueToSpell(BG_DG_ALLIANCE_CART_HOLDER_AURA);
            _flagState[TEAM_ALLIANCE] = BG_DG_CART_STATE_ON_GROUND;
            if (AddObject(BG_DG_OBJECT_CART_ALLY_GROUND, BG_DG_OBJECTID_CART_ALLY_GROUND, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation(), 0.0f, 0.0f, 0.0f, 0.0f))
            {
                SpawnBGObject(BG_DG_OBJECT_CART_ALLY_GROUND, RESPAWN_IMMEDIATELY);
                //dunno how to cast spell on gob :/
                //GetBGObject(BG_DG_OBJECT_CART_ALLY_GROUND)->CastSpell(GetBGObject(BG_DG_OBJECT_CART_ALLY_GROUND), true);
            }
            _flagsDropTimer[TEAM_ALLIANCE] = BG_DG_CART_DROP_TIME;
            set = true;
        }
    }

    if (set)
    {
        player->CastSpell(player, SPELL_RECENTLY_DROPPED_FLAG, true);
        UpdateCartState(player->GetTeam(), 1);

        if (player->GetTeam() == ALLIANCE)
        {
            SendMessageToAll(LANG_BG_DG_DROPPED_HORDE_CART, CHAT_MSG_BG_SYSTEM_HORDE, player);
            //UpdateWorldState(BG_WS_FLAG_UNK_HORDE, uint32(-1));
        }
        else
        {
            SendMessageToAll(LANG_BG_DG_DROPPED_ALLIANCE_CART, CHAT_MSG_BG_SYSTEM_ALLIANCE, player);
            //UpdateWorldState(BG_WS_FLAG_UNK_ALLIANCE, uint32(-1));
        }
    }
}

void BattlegroundDG::UpdateCartState(uint32 team, uint32 value)
{
    if (team == ALLIANCE)
        UpdateWorldState(WORLDSTATE_DG_CART_STATE_ALLIANCE, value);
    else
        UpdateWorldState(WORLDSTATE_DG_CART_STATE_HORDE, value);
}

/************************************************************************/
/*                        ENDING BATTLEGROUND                           */
/************************************************************************/

void BattlegroundDG::RemovePlayer(Player* player, uint64 guid, uint32 team)
{
    // sometimes flag aura not removed :(
    if (IsAllianceFlagPickedup() && m_FlagKeepers[TEAM_ALLIANCE] == guid)
    {
        if (!player)
        {
            //TC_LOG_ERROR("bg.battleground", "BattlegroundWS: Removing offline player who has the FLAG!!");
            SetAllianceCartPicker(0);
            _flagState[TEAM_ALLIANCE] = BG_DG_CART_STATE_ON_BASE;
            SpawnBGObject(BG_DG_OBJECT_CART_ALLIANCE, RESPAWN_IMMEDIATELY);
        }
        else
            EventPlayerDroppedFlag(player);
    }
    if (IsHordeFlagPickedup() && m_FlagKeepers[TEAM_HORDE] == guid)
    {
        if (!player)
        {
            //TC_LOG_ERROR("bg.battleground", "BattlegroundWS: Removing offline player who has the FLAG!!");
            SetHordeFlagPicker(0);
            _flagState[TEAM_HORDE] = BG_DG_CART_STATE_ON_BASE;
            SpawnBGObject(BG_DG_OBJECT_CART_HORDE, RESPAWN_IMMEDIATELY);
        }
        else
            EventPlayerDroppedFlag(player);
    }
}

uint32 BattlegroundDG::GetPrematureWinner()
{
    if (m_TeamScores[TEAM_ALLIANCE] > m_TeamScores[TEAM_HORDE])
        return ALLIANCE;
    else if (m_TeamScores[TEAM_HORDE] > m_TeamScores[TEAM_ALLIANCE])
        return HORDE;

    //it's really even, then look who captured the last cart
    if (_lastFlagCaptureTeam) return _lastFlagCaptureTeam;
    //if no cart were taken, default option
    return Battleground::GetPrematureWinner();
}

bool BattlegroundDG::SetupBattleground()
{
    //Nodes related
    for (int i = 0; i < BG_DG_ALL_NODES_COUNT; ++i)
    {
        if (!AddCreature(BG_DG_OBJECTID_CAPT_POINT, BG_DG_OBJECT_CAPT_POINT_START + i, TEAM_NEUTRAL, BG_DG_NodePositions[i][0], BG_DG_NodePositions[i][1], BG_DG_NodePositions[i][2], BG_DG_NodePositions[i][3], RESPAWN_IMMEDIATELY))
        {
            TC_LOG_ERROR("bg.battleground", "BattlegroundDG: Failed to spawn flags PNJ. Battleground not created!");
            return false;
        }
        if (!AddObject(BG_DG_OBJECT_AURA_ALLY + BG_DG_OBJECT_DYNAMIC_TOTAL * i, BG_DG_OBJECTID_AURA_A, BG_DG_AuraPositions[i][0], BG_DG_AuraPositions[i][1], BG_DG_AuraPositions[i][2], BG_DG_AuraPositions[i][3], 0, 0, std::sin(BG_DG_AuraPositions[i][3] / 2), std::cos(BG_DG_AuraPositions[i][3] / 2), RESPAWN_ONE_DAY)
            || !AddObject(BG_DG_OBJECT_AURA_HORDE + BG_DG_OBJECT_DYNAMIC_TOTAL * i, BG_DG_OBJECTID_AURA_H, BG_DG_AuraPositions[i][0], BG_DG_AuraPositions[i][1], BG_DG_AuraPositions[i][2], BG_DG_AuraPositions[i][3], 0, 0, std::sin(BG_DG_AuraPositions[i][3] / 2), std::cos(BG_DG_AuraPositions[i][3] / 2), RESPAWN_ONE_DAY)
            || !AddObject(BG_DG_OBJECT_AURA_CONTESTED + BG_DG_OBJECT_DYNAMIC_TOTAL * i, BG_DG_OBJECTID_AURA_C, BG_DG_AuraPositions[i][0], BG_DG_AuraPositions[i][1], BG_DG_AuraPositions[i][2], BG_DG_AuraPositions[i][3], 0, 0, std::sin(BG_DG_AuraPositions[i][3] / 2), std::cos(BG_DG_AuraPositions[i][3] / 2), RESPAWN_ONE_DAY)
            )
        {
            TC_LOG_ERROR("bg.battleground", "BattlegroundDG: Failed to spawn flags aura objects. Battleground not created!");
            return false;
        }
        if (!AddObject(BG_DG_OBJECT_PJ_COLLISION + BG_DG_OBJECT_DYNAMIC_TOTAL * i, BG_DG_OBJECTID_PJ_COLLISION, BG_DG_CollisionPJPositions[i][0], BG_DG_CollisionPJPositions[i][1], BG_DG_CollisionPJPositions[i][2], BG_DG_CollisionPJPositions[i][3], 0, 0, std::sin(BG_DG_CollisionPJPositions[i][3] / 2), std::cos(BG_DG_CollisionPJPositions[i][3] / 2), RESPAWN_ONE_DAY))
        {
            TC_LOG_ERROR("bg.battleground", "BattlegroundDG: Failed to spawn flags collision objects. Battleground not created!");
            return false;
        }
    }
    //Carts
    if (!AddObject(BG_DG_OBJECT_CART_ALLIANCE, BG_DG_OBJECTID_CART_ALLY, BG_DG_CartPositions[0][0], BG_DG_CartPositions[0][1], BG_DG_CartPositions[0][2], BG_DG_CartPositions[0][3], 0, 0, std::sin(BG_DG_CartPositions[0][3] / 2), std::cos(BG_DG_CartPositions[0][3] / 2), BG_DG_CART_RESPAWN_TIME / 1000)
        || !AddObject(BG_DG_OBJECT_CART_HORDE, BG_DG_OBJECTID_CART_HORDE, BG_DG_CartPositions[1][0], BG_DG_CartPositions[1][1], BG_DG_CartPositions[1][2], BG_DG_CartPositions[1][3], 0, 0, std::sin(BG_DG_CartPositions[1][3] / 2), std::cos(BG_DG_CartPositions[1][3] / 2), BG_DG_CART_RESPAWN_TIME / 1000)
        )
    {
        TC_LOG_ERROR("bg.battleground", "BattlegroundDG: Failed to spawn carts. Battleground not created!");
        return false;
    }
    //Doors
    if (!AddObject(BG_DG_OBJECT_GATE_1, BG_DG_OBJECTID_GATE, BG_DG_DoorPositions[0][0], BG_DG_DoorPositions[0][1], BG_DG_DoorPositions[0][2], BG_DG_DoorPositions[0][3], BG_DG_DoorPositions[0][4], BG_DG_DoorPositions[0][5], BG_DG_DoorPositions[0][6], BG_DG_DoorPositions[0][7], RESPAWN_IMMEDIATELY)
        || !AddObject(BG_DG_OBJECT_GATE_2, BG_DG_OBJECTID_GATE, BG_DG_DoorPositions[1][0], BG_DG_DoorPositions[1][1], BG_DG_DoorPositions[1][2], BG_DG_DoorPositions[1][3], BG_DG_DoorPositions[1][4], BG_DG_DoorPositions[1][5], BG_DG_DoorPositions[1][6], BG_DG_DoorPositions[1][7], RESPAWN_IMMEDIATELY)
        || !AddObject(BG_DG_OBJECT_GATE_3, BG_DG_OBJECTID_GATE, BG_DG_DoorPositions[2][0], BG_DG_DoorPositions[2][1], BG_DG_DoorPositions[2][2], BG_DG_DoorPositions[2][3], BG_DG_DoorPositions[2][4], BG_DG_DoorPositions[2][5], BG_DG_DoorPositions[2][6], BG_DG_DoorPositions[2][7], RESPAWN_IMMEDIATELY)
        || !AddObject(BG_DG_OBJECT_GATE_4, BG_DG_OBJECTID_GATE, BG_DG_DoorPositions[3][0], BG_DG_DoorPositions[3][1], BG_DG_DoorPositions[3][2], BG_DG_DoorPositions[3][3], BG_DG_DoorPositions[3][4], BG_DG_DoorPositions[3][5], BG_DG_DoorPositions[3][6], BG_DG_DoorPositions[3][7], RESPAWN_IMMEDIATELY)
        )
    {
        TC_LOG_ERROR("bg.battleground", "BattlegroundDG: Failed to spawn door object. Battleground not created!");
        return false;
    }
    //Buffs
    for (int i = 0; i < MAX_BUFFS; i++)
    {
        if (!AddObject(BG_DG_OBJECT_BUFF_NORTH + i, Buff_Entries[urand(0, 2)], BG_DG_BuffPositions[i][0], BG_DG_BuffPositions[i][1], BG_DG_BuffPositions[i][2], BG_DG_BuffPositions[i][3], 0, 0, std::sin(BG_DG_BuffPositions[i][3] / 2), std::cos(BG_DG_BuffPositions[i][3] / 2), BUFF_RESPAWN_TIME))
        {
            TC_LOG_ERROR("bg.battleground", "BattlegroundDG: Failed to spawn buff object. Battleground not created!");
            return false;
        }
    }

    WorldSafeLocsEntry const* sg = sWorldSafeLocsStore.LookupEntry(BG_DG_GraveyardIds[TEAM_ALLIANCE][BG_DG_GRAVEYARD_NORTHERN]);
    if (!sg || !AddSpiritGuide(BG_DG_SPIRIT_NORTHERN_ALLIANCE, sg->x, sg->y, sg->z, sg->facing + M_PI, ALLIANCE))
    {
        TC_LOG_ERROR("general", "BattlegroundDG: Failed to spawn Alliance-Northern spirit guide! Battleground not created!");
        return false;
    }

    sg = sWorldSafeLocsStore.LookupEntry(BG_DG_GraveyardIds[TEAM_ALLIANCE][BG_DG_GRAVEYARD_SOUTHERN]);
    if (!sg || !AddSpiritGuide(BG_DG_SPIRIT_SOUTHERN_ALLIANCE, sg->x, sg->y, sg->z, sg->facing + M_PI, ALLIANCE))
    {
        TC_LOG_ERROR("general", "BattlegroundDG: Failed to spawn Alliance-Southern spirit guide! Battleground not created!");
        return false;
    }

    sg = sWorldSafeLocsStore.LookupEntry(BG_DG_GraveyardIds[TEAM_HORDE][BG_DG_GRAVEYARD_NORTHERN]);
    if (!sg || !AddSpiritGuide(BG_DG_SPIRIT_NORTHERN_HORDE, sg->x, sg->y, sg->z, sg->facing + M_PI, HORDE))
    {
        TC_LOG_ERROR("general", "BattlegroundDG: Failed to spawn Horde-Northern spirit guide! Battleground not created!");
        return false;
    }

    sg = sWorldSafeLocsStore.LookupEntry(BG_DG_GraveyardIds[TEAM_HORDE][BG_DG_GRAVEYARD_SOUTHERN]);
    if (!sg || !AddSpiritGuide(BG_DG_SPIRIT_SOUTHERN_HORDE, sg->x, sg->y, sg->z, sg->facing + M_PI, HORDE))
    {
        TC_LOG_ERROR("general", "BattlegroundDG: Failed to spawn Horde-Southern spirit guide! Battleground not created!");
        return false;
    }

    return true;
}

void BattlegroundDG::Reset()
{
    //call parent's class reset
    Battleground::Reset();

    m_TeamScores[TEAM_ALLIANCE] = 0;
    m_TeamScores[TEAM_HORDE] = 0;
    m_lastTick[TEAM_ALLIANCE] = 0;
    m_lastTick[TEAM_HORDE] = 0;
    m_HonorScoreTics[TEAM_ALLIANCE] = 0;
    m_HonorScoreTics[TEAM_HORDE] = 0;
    m_IsInformedNearVictory = false;

    for (uint8 i = 0; i < BG_DG_ALL_NODES_COUNT; ++i)
    {
        m_Nodes[i] = BG_DG_NODE_TYPE_NEUTRAL;
        m_prevNodes[i] = BG_DG_NODE_TYPE_NEUTRAL;
        m_NodeTimers[i] = 0;
    }

    m_FlagKeepers[TEAM_ALLIANCE] = 0;
    m_FlagKeepers[TEAM_HORDE] = 0;
    _flagState[TEAM_ALLIANCE] = BG_DG_CART_STATE_ON_BASE;
    _flagState[TEAM_HORDE] = BG_DG_CART_STATE_ON_BASE;
    _flagGold[TEAM_ALLIANCE] = 0;
    _flagGold[TEAM_HORDE] = 0;
    m_TeamScores[TEAM_ALLIANCE] = 0;
    m_TeamScores[TEAM_HORDE] = 0;
    _lastFlagCaptureTeam = 0;
    _flagsDropTimer[TEAM_ALLIANCE] = 0;
    _flagsDropTimer[TEAM_HORDE] = 0;
    _flagsTimer[TEAM_ALLIANCE] = 0;
    _flagsTimer[TEAM_HORDE] = 0;
    _minutesElapsed = 0;
}

void BattlegroundDG::EndBattleground(uint32 winner)
{
    Battleground::EndBattleground(winner);
}

/************************************************************************/
/*                     CART SCRIPTS (CREATURE)                          */
/************************************************************************/

class mine_cart_spell : public SpellScriptLoader
{
public:
    mine_cart_spell() : SpellScriptLoader("mine_cart_DG_spell") { }

    AuraScript* GetAuraScript() const 
    {
        return new mine_cart_spell_AuraScript();
    }

    class mine_cart_spell_AuraScript : public AuraScript
    {
        PrepareAuraScript(mine_cart_spell_AuraScript);

        void HandleOnRemove(AuraEffect const* aurEff, AuraEffectHandleModes mode)
        {
            uint32 entry = 0;
            if (GetSpellInfo()->Id == BG_DG_HORDE_MINE_CART)
                entry = 71073;
            else if (GetSpellInfo()->Id == BG_DG_ALLIANCE_MINE_CART)
                entry = 71071;

            if (entry != 0)
            {
                std::list<Creature*> carts;
                GetCaster()->GetCreatureListWithEntryInGrid(carts, entry, 500.f);
                for (std::list<Creature*>::iterator itr = carts.begin(); itr != carts.end(); ++itr)
                {
                    if (TempSummon* tmp = (*itr)->ToTempSummon())
                        tmp->UnSummon();
                }
            }
        }

        void Register() 
        {
            OnEffectRemove += AuraEffectRemoveFn(mine_cart_spell_AuraScript::HandleOnRemove, EFFECT_3, SPELL_AURA_LINKED_2, AURA_EFFECT_HANDLE_REAL);
        }
    };
};
class npc_cart : public CreatureScript
{
public:
    npc_cart() : CreatureScript("npc_DG_cart") { }

    enum
    {
        EVENT_NEW_WAYPOINT = 1,
    };

    enum faction
    {
        FACTION_FRIENDLY = 35
    };

    struct npc_cart_AI : public ScriptedAI
    {
        npc_cart_AI(Creature* c) : ScriptedAI(c)
        {
            c->setFaction(FACTION_FRIENDLY);
            c->SetReactState(REACT_PASSIVE);
        }

        EventMap events;

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);
            if (me->GetOwner())
            {
                if (me->GetDistance(me->GetOwner()) > 15.f)
                    me->SetSpeed(MOVE_RUN, ((me->GetOwner()->GetSpeed(MOVE_RUN) + 1.f) / playerBaseMoveSpeed[MOVE_RUN]) * 2.f, true);
                else
                    me->SetSpeed(MOVE_RUN, (me->GetOwner()->GetSpeed(MOVE_RUN) + 1.f) / playerBaseMoveSpeed[MOVE_RUN], true);

                if (!me->isMoving() && events.Empty())
                    events.ScheduleEvent(EVENT_NEW_WAYPOINT, 400);

                if (events.ExecuteEvent() == EVENT_NEW_WAYPOINT)
                    me->GetMotionMaster()->MoveChase(me->GetOwner(), frand(0.5f, 1.f), frand(0.f, 2.f * M_PI));

                if (!me->HasAura(BG_DG_TRACK_MINE_CART))
                    me->GetOwner()->CastSpell(me, BG_DG_TRACK_MINE_CART, true);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const 
    {
        return new npc_cart_AI(creature);
    }
};

void AddSC_BattlegroundDGScripts()
{
    new mine_cart_spell();
    new npc_cart();
}
