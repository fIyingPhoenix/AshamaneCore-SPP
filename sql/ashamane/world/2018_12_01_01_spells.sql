-- 196406 - Back Draft Aura
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_warl_back_draft_aura';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES 
(196406, 'spell_warl_back_draft_aura');

DELETE FROM `spell_proc` WHERE `SpellId`=196406;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `SpellFamilyMask3`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES 
(196406, 4, 5, 0, 0x00820000, 0, 0x00400000, 0x00010000, 1, 1, 0, 0, 0, 100, 0, 0);

-- 205184 - Roaring Blaze
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_warr_roaring_blaze';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES 
(205184, 'spell_warr_roaring_blaze');

-- 264178 - Demonbolt
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_warl_demonbolt';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES 
(264178, 'spell_warl_demonbolt');

-- 265412 - Doom
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_warl_doom';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES 
(265412, 'spell_warl_doom');