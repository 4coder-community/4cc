#if !defined(HANDMADE_AUDIO_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

struct playing_sound
{
    v2 CurrentVolume;
    v2 dCurrentVolume;
    v2 TargetVolume;

    real32 dSample;

    sound_id ID;
    real32 SamplesPlayed;
    playing_sound *Next;
};

struct audio_state
{
    memory_arena *PermArena;
    playing_sound *FirstPlayingSound;
    playing_sound *FirstFreePlayingSound;

    v2 MasterVolume;
};

#define HANDMADE_AUDIO_H
#endif
