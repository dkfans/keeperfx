int __cdecl object_update_power_sight(Thing *objtng)
{
  int owner; // ecx
  struct Dungeon *dungeon; // ebx
  int sight_casted_splevel; // eax
  unsigned int max_time_active; // ecx
  int close_time; // ebp
  int sight_casted_gameturn; // edx
  int time_active; // eax
  int v8; // eax
  int time_active_2; // edi
  int v10; // eax
  int v11; // edi
  int v12; // ecx
  int strength; // kr08_4
  int v14; // ebp
  int pos_x; // edx
  int pos_y; // eax
  int shift_x; // eax
  int shift_y; // ecx
  int result; // eax
  int v20; // edi
  int splevel; // eax
  int close_time_2; // ecx
  int v23; // ebx
  int v24; // ecx
  int v25; // edi
  int v26; // eax
  int v27; // ebx
  int *v28; // ebp
  int v29; // eax
  __int16 v30; // ax
  struct Coord3d pos; // [esp+10h] [ebp-10h] BYREF
  int v32; // [esp+18h] [ebp-8h]
  int max_active_div_16; // [esp+1Ch] [ebp-4h]

  owner = objtng->owner;
  objtng->health = 2;
  dungeon = &game_dungeon_0 + owner;
  if ( !S3DEmitterIsPlayingSample((unsigned __int8)objtng->snd_emitter_id, 51, 0) )
    thing_play_sample(objtng, 51, 0x64u, -1, 3u, 1u, 3, 256);
  sight_casted_splevel = (unsigned __int8)dungeon->sight_casted_splevel;
  max_time_active = pwrdynst__PwrK_SIGHT__strength[sight_casted_splevel];
  if ( game.play_gameturn - objtng->creation_turn >= max_time_active
    && game.play_gameturn - dungeon->sight_casted_gameturn < max_time_active )
  {
    close_time = power_sight_close_instance_time[sight_casted_splevel];
    max_active_div_16 = (int)max_time_active / 4 / 4;
    sight_casted_gameturn = dungeon->sight_casted_gameturn;
    v32 = max_active_div_16 / close_time;
    time_active = game.play_gameturn - sight_casted_gameturn;
    if ( game.play_gameturn - sight_casted_gameturn >= 0 )
    {
      if ( max_active_div_16 < time_active )
        time_active = max_active_div_16;
    }
    else
    {
      time_active = 0;
    }
    v8 = game.play_gameturn - max_time_active + time_active / v32 - close_time;
    dungeon->sight_casted_gameturn = game.play_gameturn - max_time_active;
    dungeon->sight_casted_gameturn = v8;
  }
  time_active_2 = game.play_gameturn - dungeon->sight_casted_gameturn;
  v10 = pwrdynst__PwrK_SIGHT__strength[(unsigned __int8)dungeon->sight_casted_splevel];
  if ( v10 <= time_active_2 )
  {
    v20 = time_active_2 - v10;
    if ( power_sight_close_instance_time[(unsigned __int8)dungeon->sight_casted_splevel] <= v20 )
    {
      if ( (dungeon->computer_enabled & 4) != 0 )
      {
        dungeon->sight_casted_gameturn = game.play_gameturn;
        pos.x.val = ((unsigned __int8)dungeon->sight_casted_stl_x << 8) + 128;
        v30 = (unsigned __int8)dungeon->sight_casted_stl_y << 8;
        pos.z.val = 1408;
        pos.y.val = v30 + 128;
        memset(dungeon->soe_explored_flags, 0, sizeof(dungeon->soe_explored_flags));
        move_thing_in_map(objtng, &pos);
        result = 1;
        dungeon->computer_enabled &= ~4u;
      }
      else
      {
        dungeon->sight_casted_thing_idx = 0;
        memset(dungeon->soe_explored_flags, 0, sizeof(dungeon->soe_explored_flags));
        delete_thing_structure(objtng, 0);
        return 0;
      }
    }
    else
    {
      pos.z.val = 1408;
      splevel = (unsigned __int8)dungeon->sight_casted_splevel;
      close_time_2 = power_sight_close_instance_time[splevel];
      v23 = 4 * close_time_2;
      v24 = close_time_2 - v20;
      v25 = 4;
      v26 = 8 * (pwrdynst__PwrK_SIGHT__strength[splevel] / 4) / v23;
      v27 = 4 * v24 * v26;
      v32 = v26;
      do
      {
        v28 = lbSinTable;
        do
        {
          v29 = *v28;
          v28 += 64;
          pos.x.val = objtng->mappos.x.val + ((unsigned int)(v27 * v29) >> 16);
          pos.y.val = objtng->mappos.y.val + (-((v27 * v28[448]) >> 8) >> 8);
          create_effect_element(&pos, *((_DWORD *)&effkind + objtng->owner), objtng->owner);
        }
        while ( v28 < &lbCosTable[1536] );
        v27 -= v32;
        --v25;
      }
      while ( v25 );
      return 1;
    }
  }
  else
  {
    v32 = 4;
    v11 = 4 * time_active_2;
    do
    {
      v12 = v11;
      if ( v11 >= 0 )
      {
        strength = pwrdynst__PwrK_SIGHT__strength[(unsigned __int8)dungeon->sight_casted_splevel];
        if ( strength / 4 < v11 )
          v12 = strength / 4;
      }
      else
      {
        v12 = 0;
      }
      v14 = (v11 & 0x1F) << 8;
      pos_x = (unsigned __int16)objtng->mappos.x.val + (__int16)((v12 * *(int *)((char *)lbSinTable + v14)) >> 13);
      pos_y = (-(char)((v12 * *(int *)((char *)lbCosTable + v14)) >> 5) >> 8) + (unsigned __int16)objtng->mappos.y.val;
      if ( pos_x >= 0 && pos_x < 65280 && pos_y >= 0 && pos_y < 65280 )
      {
        pos.z.val = 1408;
        pos.x.val = pos_x;
        pos.y.val = pos_y;
        create_effect_element(&pos, *((_DWORD *)&effkind + objtng->owner), objtng->owner);
        shift_x = (unsigned __int8)pos.x.stl.pos - (unsigned __int8)objtng->mappos.x.stl.pos + 13;
        shift_y = (unsigned __int8)pos.y.stl.pos - (unsigned __int8)objtng->mappos.y.stl.pos + 13;
        dungeon->soe_explored_flags[26 * shift_y + shift_x] = pos.x.val < 0xFF00u && pos.y.val < 0xFF00u;
      }
      ++v11;
      --v32;
    }
    while ( v32 );
    return 1;
  }
  return result;
}