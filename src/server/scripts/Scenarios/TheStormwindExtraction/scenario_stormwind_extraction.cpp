/*
 * Copyright (C) 2017-2018 AshamaneProject <https://github.com/AshamaneProject>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptMgr.h"
#include "CombatAI.h"
#include "Creature.h"
#include "CreatureGroups.h"
#include "GameObject.h"
#include "InstanceScript.h"
#include "Scenario.h"
#include "stormwind_extraction.h"

struct scenario_stormwind_extraction : public InstanceScript
{
    scenario_stormwind_extraction(InstanceMap* map) : InstanceScript(map) { }

    Position PrisonMagicWallsPositions[8] =
    {
        { -8740.16f,    866.198f,   53.8153f,   0.66650f }, // bottom (moving to push player)
        { -8702.05f,    904.297f,   53.8153f,   5.37439f }, // Arrival (moving to push player)
        { -8693.48f,    902.970f,   53.8153f,   3.77220f }, // front of saurfang cell
        { -8719.19f,    890.029f,   52.8987f,   5.40985f }, // first right
        { -8712.07f,    880.811f,   52.8987f,   2.22569f }, // first left
        { -8729.27f,    866.807f,   52.8987f,   2.23515f }, // down left
        { -8694.57f,    894.535f,   53.8153f,   2.22813f }, // exit (openned by Talanji)
        { -8736.71f,    876.128f,   52.8987f,   5.41640f }, // down right
    };

    std::vector<TempSummon*> magicWalls;

    void OnPlayerEnter(Player* player) override
    {
        CreatureGroup* talanjizulLionRest = SummonCreatureGroup(SUMMON_GROUP_LION_REST);
        SummonCreatureGroup(SUMMON_GROUP_TALANJI_ZUL_PRISON);

        // Temp introduction fix
        player->GetScheduler().Schedule(2s, [player, talanjizulLionRest](TaskContext /*context*/)
        {
            talanjizulLionRest->MoveGroupTo(-8671.096680f, 915.972229f, 89.469795f);
            player->GetScenario()->SendScenarioEvent(player, SCENARIO_EVENT_STORMWIND_INFILTRATION);
        });
    }

    void OnCompletedCriteriaTree(CriteriaTree const* tree) override
    {
        if (tree->ID == CRITERIA_TREE_OPEN_SEWERS)
            HandleGameObject(GetObjectGuid(GOB_SEWER_ACCESS_GATE), true);
    }

    void SetData(uint32 type, uint32 value) override
    {
        if (type == SCENARIO_EVENT_ENTER_STOCKADE)
        {
            DespawnCreatureGroup(SUMMON_GROUP_LION_REST);
            SummonCreatureGroup(SUMMON_GROUP_INSIDE_PRISON);

            SummonCreatureGroup(SUMMON_GROUP_GUARD_ENTRANCE);
            SummonCreatureGroup(SUMMON_GROUP_GUARD_FIRST_ROOM);
        }
        else if (type == SCENARIO_EVENT_FIND_ROKHAN)
        {
            DoSendScenarioEvent(SCENARIO_EVENT_FIND_ROKHAN);

            GetRokhan()->RemoveAurasDueToSpell(SPELL_ROKHAN_SOLO_STEALTH);
            GetRokhan()->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
        }
        else if (type == SCENARIO_EVENT_FREE_SAURFANG)
        {
            static_cast<CombatAI*>(GetRokhan()->AI())->MoveCombat(Position(-8743.460938f, 883.260620f, 52.815895f));
            GetThalyssra()->AI()->DoAction(2);
        }
        else if (type == EVENT_FIND_PRISONNERS)
        {
            DoPlayConversation(CONVERSATION_ESCORT_ARRIVED);
        }
        else if (type == SCENARIO_EVENT_FREE_PRISONNERS)
        {
            DoPlayConversation(CONVERSATION_THANK_YOU_PRISON);
            GetTalanji()->GetScheduler().Schedule(2s, [this](TaskContext /*context*/)
            {
                GetTalanji()->GetMotionMaster()->MovePoint(1, -8745.711914f, 886.244446f, 52.815662f);

                GetZul()->SetAIAnimKitId(0);
                GetZul()->HandleEmoteCommand(EMOTE_ONESHOT_NONE);
                GetZul()->GetMotionMaster()->MovePoint(1, -8743.465820f, 888.028198f, 52.815895f);
            }).Schedule(20s, [this](TaskContext /*context*/)
            {
                DespawnCreatureGroup(SUMMON_GROUP_INSIDE_PRISON);
                DespawnCreatureGroup(SUMMON_GROUP_TALANJI_ZUL_PRISON);

                SummonCreatureGroup(SUMMON_GROUP_ALL_AFTER_FREED);
            });

            GetSaurfang()->GetScheduler().Schedule(22s, [this](TaskContext /*context*/)
            {
                DoPlayConversation(CONVERSATION_NULLIFICATION);
                GetThalyssra()->AddAura(SPELL_NULLIFICATION_BARRIER);
                GetZul()->AddAura(SPELL_NULLIFICATION_BARRIER);

            }).Schedule(34s, [this](TaskContext /*context*/)
            {
                if (CreatureGroup* creGroup = GetCreatureGroup(SUMMON_GROUP_ALL_AFTER_FREED))
                    creGroup->MoveGroupTo(-8724.619141f, 877.957153f, 53.732788f);

            }).Schedule(37s, [this](TaskContext /*context*/)
            {
                std::list<TempSummon*> battleMages;
                SummonCreatureGroup(SUMMON_GROUP_BATTLEMAGE, &battleMages);
                for (TempSummon* summon : battleMages)
                {
                    summon->CastSpell(summon, SPELL_ARCANE_CHANNELING, true);
                    summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
                }

                for (uint8 i = 0; i < 8; ++i)
                {
                    if (TempSummon* wall = instance->SummonCreature(NPC_FX_STALKER, PrisonMagicWallsPositions[i]))
                    {
                        magicWalls.push_back(wall);

                        if (i < 2)
                            wall->CastSpell(wall, SPELL_ARCANE_BARRIER_MOVING, true);
                        else
                            wall->CastSpell(wall, SPELL_ARCANE_BARRIER, true);
                    }
                }
            }).Schedule(39s, [this](TaskContext /*context*/)
            {
                if (CreatureGroup* creGroup = GetCreatureGroup(SUMMON_GROUP_ALL_AFTER_FREED))
                    creGroup->MoveGroupTo(-8697.854492f, 899.180908f, 53.731392f);

            }).Schedule(42s, [this](TaskContext /*context*/)
            {
                DoPlayConversation(CONVERSATION_HOW_GET_OUT);

            }).Schedule(53s, [this](TaskContext /*context*/)
            {
                GetTalanji()->CastSpell(nullptr, SPELL_TALANJI_OPEN_ARCANE_BARRIER, false);

            }).Schedule(56s, [this](TaskContext /*context*/)
            {
                magicWalls[6]->DespawnOrUnsummon();
            }).Schedule(58s, [this](TaskContext /*context*/)
            {
                GetThalyssra()->AI()->DoAction(3);
                if (CreatureGroup* creGroup = GetCreatureGroup(SUMMON_GROUP_ALL_AFTER_FREED))
                    creGroup->MoveGroupTo(-8645.526367f, 773.394714f, 45.399426f, true);
            });
        }
        else if (type == EVENT_END_OF_PRISON_REACHED)
        {
            DoPlayConversation(CONVERSATION_BEFORE_ESCAPE);

            GetThalyssra()->GetScheduler().Schedule(15s, [this](TaskContext context)
            {
                if (Creature* escapeStockade = GetCreature(NPC_ESCAPE_STOCKADE))
                {
                    escapeStockade->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

                    //DoSendScenarioEvent(SCENARIO_EVENT_PRISON_ESCAPE);

                    // TEMP FIX
                    if (ScenarioStepEntry const* step = sScenarioStepStore.LookupEntry(3731))
                        GetThalyssra()->GetScenario()->SetStep(step);

                    if (CreatureGroup* creGroup = GetCreatureGroup(SUMMON_GROUP_ALL_AFTER_FREED))
                        creGroup->MoveGroupTo(escapeStockade->GetPositionX(), escapeStockade->GetPositionY(), escapeStockade->GetPositionZ());
                }

            }).Schedule(16s, [this](TaskContext context)
            {
                DespawnCreatureGroup(SUMMON_GROUP_ALL_AFTER_FREED);
                SummonCreatureGroup(SUMMON_GROUP_END_HARBOR_HACKFIX);

                GetThalyssra()->GetScheduler().Schedule(4s, [this](TaskContext context)
                {
                    DoPlayConversation(CONVERSATION_JAINA_END_OF_ESCAPE);

                }).Schedule(24s, [this](TaskContext context)
                {
                    DoCastSpellOnPlayers(SPELL_SCENE_JAINA_AND_ZUL);
                });
            });
        }
    }

    void OnCreatureGroupWipe(uint32 creatureGroupId) override
    {
        if (creatureGroupId == SUMMON_GROUP_GUARD_ENTRANCE)
        {
            DoPlayConversation(CONVERSATION_PRISON_ENTRANCE);

            if (Creature* rokhan = GetRokhan())
            {
                rokhan->GetScheduler().Schedule(2s, [rokhan](TaskContext /*context*/)
                {
                    rokhan->SetWalk(false);
                    rokhan->SetSpeed(MOVE_RUN, 6.f);

                    rokhan->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
                    rokhan->CastSpell(rokhan, SPELL_ROKHAN_SOLO_STEALTH, true);
                    rokhan->GetMotionMaster()->MovePoint(1, -8692.569336f, 905.782898f, 53.733604f);
                });
            }

            if (Creature* thalyssra = GetThalyssra())
            {
                thalyssra->GetScheduler().Schedule(5s, [this, thalyssra](TaskContext /*context*/)
                {
                    thalyssra->SetWalk(false);
                    thalyssra->SetSpeed(MOVE_RUN, 6.f);
                    thalyssra->GetMotionMaster()->MovePoint(1, -8743.606445f, 998.370361f, 44.149288f, 3.284534f);

                    if (CreatureGroup* group = GetCreatureGroup(SUMMON_GROUP_GUARD_FIRST_ROOM))
                        group->MoveGroupTo(-8747.977539f, 997.306824f, 44.148872f);
                });
            }
        }

        if (creatureGroupId == SUMMON_GROUP_GUARD_FIRST_ROOM)
        {
            if (Creature* thalyssra = GetThalyssra())
            {
                thalyssra->GetScheduler().Schedule(2s, [thalyssra](TaskContext /*context*/)
                {
                    thalyssra->AI()->DoAction(1);
                });
            }
        }
    }

private:
    Creature* GetRokhan()       { return GetCreature(NPC_ROKHAN); }
    Creature* GetThalyssra()    { return GetCreature(NPC_THALYSSRA); }
    Creature* GetTalanji()      { return GetCreature(NPC_TALANJI); }
    Creature* GetZul()          { return GetCreature(NPC_ZUL); }
    Creature* GetSaurfang()     { return GetCreature(NPC_SAURFANG); }
};

void AddSC_scenario_stormwind_extraction()
{
    RegisterInstanceScript(scenario_stormwind_extraction, 1904);
}