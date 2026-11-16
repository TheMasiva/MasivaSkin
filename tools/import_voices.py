"""Extract event identifiers (no audio) from GameTracking-CS2 agent vsndevts.
Usage: python tools/import_voices.py DIRECTORY COMMIT
Without arguments, regenerate from the included data/voices.json snapshot.
"""
from pathlib import Path
import json,re,sys,hashlib
root=Path(__file__).resolve().parents[1]
# Legacy banks and newer agents use different names for the same call.
aliases={
 'onarollbrag':'cheer',
 'plantingbomb':'i_plant_bomb','radiobotplanting':'i_plant_bomb',
 'defusingbomb':'defusing','radiobotdefusing':'defusing','radiobotbombdefusing':'defusing',
 'radiobotcoverdefuse':'cover_defuse',
 'coverme':'request_coverme','radio_followme':'request_follow_me','radiobotfollowme':'request_follow_me',
 'radio_letsgo':'request_move','radiobotgo':'request_move','radiobothold':'request_hold',
 'radiobotregroup':'request_regroup','radiobotfallback':'request_fallback',
 'radio_needbackup':'request_backup','radiobotreponsecoverrequest':'request_coverme',
 'requestreport':'request_report','radiobotquery':'request_report',
 'reportingin':'reporting_in','radiobotreport':'reporting_in','inposition':'in_position',
 'affirmative':'affirmation','radiobotreponsepositive':'affirmation',
 'negativeno':'negative','radiobotreponsenegative':'negative',
 'radio_enemyspotted':'sees_enemy','radiobottarget':'sees_enemy',
 'radio_takingfire':'takingfire','radiobotunderfire':'takingfire',
 'friendlyfire':'takingfire_friendly','radiobotunderfirefriendly':'takingfire_friendly',
 'followingfriend':'following_friend','radiobotfollowyou':'following_friend',
 'onmyway':'omw','radiobotreponseomw':'omw','waitinghere':'waiting_here',
 'heardnoise':'heard_sound','radiobothear':'heard_sound',
 'radiobotpostflash':'blinded','radiobotcheer':'cheer','thanks':'thankful',
 'radio_locknload':'round_start','radiobotstart':'round_start',
 'radiobotreponseattacking':'attacking','help':'request_help',
 'clearedarea':'sees_area_clear','clear':'sees_area_clear','radiobotclear':'sees_area_clear',
 'enemydown':'sees_enemy_killed','radiobotkill':'i_killed_enemy',
 'sniperwarning':'sees_sniper','radiobotunderfiresniper':'sees_sniper',
 'sniperkilled':'i_killed_sniper','radiobotkillsniper':'i_killed_sniper',
 'niceshot':'compliment','radiobotniceshot':'compliment','pinneddown':'pinned',
 'spottedloosebomb':'sees_dropped_bomb','radiobotbombpick':'see_bomb_pickup',
 'twoenemiesleft':'enemies_left_two','threeenemiesleft':'enemies_left_three',
 'oneenemyleft':'enemies_left_one','coveringfriend':'covering_friend',
 'lastmanstanding':'last_unit_alive','lostenemy':'lost_sight_enemy',
}
for prefix in ('ct','t'):
 for suffix,new in [('flashbang','flashbang'),('grenade','grenade'),('smoke','smoke'),('decoy','decoy'),('molotov','molotov')]:
  aliases[prefix+'_'+suffix]='throwing_'+new
 aliases[prefix+'_death']='death'
for key,value in list(aliases.items()):
 if key.startswith('radio_'):aliases[key.replace('radio_','radio.',1)]=value
actor_prefix={'gendarmerie_fem':'ff1_','gendarmerie_fem_epic':'ff2_','gendarmerie_male':'fm1_',
 'jungle_fem':'aff1_','jungle_fem_epic':'aff1_','jungle_male':'afm1_','jungle_male_epic':'afm2_',
 'seal_diver_01':'am1_','seal_diver_02':'am1_','seal_diver_03':'am3_','seal_fem':'af1_'}
def stem(s):return re.sub(r'_?\d+[a-z]?$', '',s)
if len(sys.argv)>1:
 banks={};hashes={}
 for p in sorted(Path(sys.argv[1]).glob('*.vsndevts')):
  data=p.read_bytes();bank=p.stem.removeprefix('game_sounds_')
  names=re.findall(r'^\t('+re.escape(bank)+r'\.[^\s=]+)\s*=',data.decode(),re.M)
  banks[bank]=sorted(n for n in names if not n.endswith('.vo_preview'))
  hashes[p.name]=hashlib.sha256(data).hexdigest()
 x={'source':'https://github.com/SteamDatabase/GameTracking-CS2','commit':sys.argv[2],
    'sha256':hashes,'aliases':aliases,'actor_prefix':actor_prefix,'banks':banks}
 (root/'data/voices.json').write_text(json.dumps(x,indent=2)+'\n')
x=json.loads((root/'data/voices.json').read_text());groups={}
for bank,names in x['banks'].items():
 for name in names:
  s=stem(name.split('.',1)[1].lower()).removeprefix(x['actor_prefix'].get(bank,''));s=x['aliases'].get(s,s)
  groups.setdefault(bank+'.'+s,[]).append(name)
q=json.dumps
out='#pragma once\n#include <cstddef>\nnamespace masiva::voice_data {\nstruct Alias {const char* from; const char* to;};\ninline constexpr Alias aliases[] = {\n'
out+=''.join('{'+q(a)+','+q(b)+'},\n' for a,b in sorted(x['aliases'].items()))+'};\n'
out+='struct Bank {const char* name;const char* prefix;};\ninline constexpr Bank banks[] = {\n'+''.join('{'+q(b)+','+q(x['actor_prefix'].get(b,''))+'},\n' for b in sorted(x['banks']))+'};\n'
out+='inline constexpr const char* events[] = {\n';rows=[];offset=0
for key,names in sorted(groups.items()):
 names=sorted(set(names));rows.append((key,offset,len(names)));offset+=len(names)
 out+=''.join(q(n)+',\n' for n in names)
out+='};\nstruct Group {const char* key; unsigned first,count;};\ninline constexpr Group groups[] = {\n'
out+=''.join('{'+q(k)+','+str(f)+','+str(n)+'},\n' for k,f,n in rows)+'};\n}\n'
(root/'src/voice_catalog.hpp').write_text(out)
print(len(x['banks']),'banks',offset,'events',len(rows),'groups')
