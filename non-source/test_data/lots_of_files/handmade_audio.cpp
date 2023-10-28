/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

internal void
OutputTestSineWave(game_state *GameState, game_sound_output_buffer *SoundBuffer, int ToneHz)
{
    int16 ToneVolume = 3000;
    int WavePeriod = SoundBuffer->SamplesPerSecond/ToneHz;    
    
    int16 *SampleOut = SoundBuffer->Samples;
    for(int SampleIndex = 0;
        SampleIndex < SoundBuffer->SampleCount;
        ++SampleIndex)
    {
        // TODO(casey): Draw this out for people
#if 1
        real32 SineValue = sinf(GameState->tSine);
        int16 SampleValue = (int16)(SineValue * ToneVolume);
#else
        int16 SampleValue = 0;
#endif
        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;

#if 1
        GameState->tSine += Tau32*1.0f/(real32)WavePeriod;
        if(GameState->tSine > Tau32)
        {
            GameState->tSine -= Tau32;
        }
#endif
    }
}

internal playing_sound *
PlaySound(audio_state *AudioState, sound_id SoundID)
{
    TIMED_FUNCTION();

    if(!AudioState->FirstFreePlayingSound)
    {
        AudioState->FirstFreePlayingSound = PushStruct(AudioState->PermArena, playing_sound);
        AudioState->FirstFreePlayingSound->Next = 0;
    }

    playing_sound *PlayingSound = AudioState->FirstFreePlayingSound;
    AudioState->FirstFreePlayingSound = PlayingSound->Next;

    PlayingSound->SamplesPlayed = 0;
    // TODO(casey): Should these default to 0.5f/0.5f for centerred?
    PlayingSound->CurrentVolume = PlayingSound->TargetVolume = V2(1.0f, 1.0f);
    PlayingSound->dCurrentVolume = V2(0, 0);
    PlayingSound->ID = SoundID;
    PlayingSound->dSample = 1.0f;

    PlayingSound->Next = AudioState->FirstPlayingSound;
    AudioState->FirstPlayingSound = PlayingSound;

    return(PlayingSound);
}

internal void
ChangeVolume(audio_state *AudioState, playing_sound *Sound, real32 FadeDurationInSeconds, v2 Volume)
{
    if(Sound)
    {
        if(FadeDurationInSeconds <= 0.0f)
        {
            Sound->CurrentVolume = Sound->TargetVolume = Volume;
        }
        else
        {
            real32 OneOverFade = 1.0f / FadeDurationInSeconds;
            Sound->TargetVolume = Volume;
            Sound->dCurrentVolume = OneOverFade*(Sound->TargetVolume - Sound->CurrentVolume);
        }
    }
}

internal void
ChangePitch(audio_state *AudioState, playing_sound *Sound, real32 dSample)
{
    if(Sound)
    {
        Sound->dSample = dSample;
    }
}

internal void
OutputPlayingSounds(audio_state *AudioState,
                    game_sound_output_buffer *SoundBuffer, game_assets *Assets,
                    memory_arena *TempArena)
{    
    TIMED_FUNCTION();

    temporary_memory MixerMemory = BeginTemporaryMemory(TempArena);

    u32 GenerationID = BeginGeneration(Assets);

    Assert((SoundBuffer->SampleCount & 3) == 0);
    u32 ChunkCount = SoundBuffer->SampleCount / 4;
    
    __m128 *RealChannel0 = PushArray(TempArena, ChunkCount, __m128, 16);
    __m128 *RealChannel1 = PushArray(TempArena, ChunkCount, __m128, 16);

    real32 SecondsPerSample = 1.0f / (real32)SoundBuffer->SamplesPerSecond;
#define AudioStateOutputChannelCount 2

    __m128 One = _mm_set1_ps(1.0f);
    __m128 Zero = _mm_set1_ps(0.0f);

    // NOTE(casey): Clear out the mixer channels    
    {
        __m128 *Dest0 = RealChannel0;
        __m128 *Dest1 = RealChannel1;
        for(u32 SampleIndex = 0;
            SampleIndex < ChunkCount;
            ++SampleIndex)
        {
            _mm_store_ps((float *)Dest0++, Zero);
            _mm_store_ps((float *)Dest1++, Zero);
        }
    }
    
    // NOTE(casey): Sum all sounds
    for(playing_sound **PlayingSoundPtr = &AudioState->FirstPlayingSound;
        *PlayingSoundPtr;
        )
    {
        playing_sound *PlayingSound = *PlayingSoundPtr;
        bool32 SoundFinished = false;

        uint32 TotalChunksToMix = ChunkCount;
        __m128 *Dest0 = RealChannel0;
        __m128 *Dest1 = RealChannel1;
        
        while(TotalChunksToMix && !SoundFinished)
        {
            loaded_sound *LoadedSound = GetSound(Assets, PlayingSound->ID, GenerationID);
            if(LoadedSound)
            {
                sound_id NextSoundInChain = GetNextSoundInChain(Assets, PlayingSound->ID);
                PrefetchSound(Assets, NextSoundInChain);

                v2 Volume = PlayingSound->CurrentVolume;
                v2 dVolume = SecondsPerSample*PlayingSound->dCurrentVolume;
                v2 dVolumeChunk = 4.0f*dVolume;
                real32 dSample = PlayingSound->dSample;
                // TODO(casey): If you use the 1.9f version, it can clearly repro a bug
                // in the accuracy of the samples played calcs!
//                real32 dSample = PlayingSound->dSample*1.9f;
                real32 dSampleChunk = 4.0f*dSample;

                // NOTE(casey): Channel 0
                __m128 MasterVolume0 = _mm_set1_ps(AudioState->MasterVolume.E[0]);
                __m128 Volume0 = _mm_setr_ps(Volume.E[0] + 0.0f*dVolume.E[0],
                                             Volume.E[0] + 1.0f*dVolume.E[0],
                                             Volume.E[0] + 2.0f*dVolume.E[0],
                                             Volume.E[0] + 3.0f*dVolume.E[0]);
                __m128 dVolume0 = _mm_set1_ps(dVolume.E[0]);
                __m128 dVolumeChunk0 = _mm_set1_ps(dVolumeChunk.E[0]);

                // NOTE(casey): Channel 1
                __m128 MasterVolume1 = _mm_set1_ps(AudioState->MasterVolume.E[1]);
                __m128 Volume1 = _mm_setr_ps(Volume.E[1] + 0.0f*dVolume.E[1],
                                             Volume.E[1] + 1.0f*dVolume.E[1],
                                             Volume.E[1] + 2.0f*dVolume.E[1],
                                             Volume.E[1] + 3.0f*dVolume.E[1]);
                __m128 dVolume1 = _mm_set1_ps(dVolume.E[1]);
                __m128 dVolumeChunk1 = _mm_set1_ps(dVolumeChunk.E[1]);
                            
                Assert(PlayingSound->SamplesPlayed >= 0.0f);

                u32 ChunksToMix = TotalChunksToMix;
                r32 RealChunksRemainingInSound =
                    (LoadedSound->SampleCount - RoundReal32ToInt32(PlayingSound->SamplesPlayed)) / dSampleChunk;
                u32 ChunksRemainingInSound = RoundReal32ToInt32(RealChunksRemainingInSound);
                if(ChunksToMix > ChunksRemainingInSound)
                {
                    ChunksToMix = ChunksRemainingInSound;
                }

                u32 VolumeEndsAt[AudioStateOutputChannelCount] = {};
                for(u32 ChannelIndex = 0;
                    ChannelIndex < ArrayCount(VolumeEndsAt);
                    ++ChannelIndex)
                {
                    if(dVolumeChunk.E[ChannelIndex] != 0.0f)
                    {
                        real32 DeltaVolume = (PlayingSound->TargetVolume.E[ChannelIndex] -
                                              Volume.E[ChannelIndex]);
                        u32 VolumeChunkCount = (u32)(((DeltaVolume / dVolumeChunk.E[ChannelIndex]) + 0.5f));
                        if(ChunksToMix > VolumeChunkCount)
                        {
                            ChunksToMix = VolumeChunkCount;
                            VolumeEndsAt[ChannelIndex] = VolumeChunkCount;
                        }
                    }
                }
                
                // TODO(casey): Handle stereo!
                real32 BeginSamplePosition = PlayingSound->SamplesPlayed;
                real32 EndSamplePosition = BeginSamplePosition + ChunksToMix*dSampleChunk;
                real32 LoopIndexC = (EndSamplePosition - BeginSamplePosition) / (r32)ChunksToMix;
                for(u32 LoopIndex = 0;
                    LoopIndex < ChunksToMix;
                    ++LoopIndex)
                {
                    real32 SamplePosition = BeginSamplePosition + LoopIndexC*(r32)LoopIndex;
                    // TODO(casey): Move volume up here to explicit.
#if 1
                    __m128 SamplePos = _mm_setr_ps(SamplePosition + 0.0f*dSample,
                                                   SamplePosition + 1.0f*dSample,
                                                   SamplePosition + 2.0f*dSample,
                                                   SamplePosition + 3.0f*dSample);
                    __m128i SampleIndex = _mm_cvttps_epi32(SamplePos);
                    __m128 Frac = _mm_sub_ps(SamplePos, _mm_cvtepi32_ps(SampleIndex));
                    
                    __m128 SampleValueF = _mm_setr_ps(LoadedSound->Samples[0][((int32 *)&SampleIndex)[0]],
                                                      LoadedSound->Samples[0][((int32 *)&SampleIndex)[1]],
                                                      LoadedSound->Samples[0][((int32 *)&SampleIndex)[2]],
                                                      LoadedSound->Samples[0][((int32 *)&SampleIndex)[3]]);
                    __m128 SampleValueC = _mm_setr_ps(LoadedSound->Samples[0][((int32 *)&SampleIndex)[0] + 1],
                                                      LoadedSound->Samples[0][((int32 *)&SampleIndex)[1] + 1],
                                                      LoadedSound->Samples[0][((int32 *)&SampleIndex)[2] + 1],
                                                      LoadedSound->Samples[0][((int32 *)&SampleIndex)[3] + 1]);

                    __m128 SampleValue = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(One, Frac), SampleValueF),
                                                    _mm_mul_ps(Frac, SampleValueC));
#else                    
                    __m128 SampleValue = _mm_setr_ps(LoadedSound->Samples[0][RoundReal32ToInt32(SamplePosition + 0.0f*dSample)],
                                                     LoadedSound->Samples[0][RoundReal32ToInt32(SamplePosition + 1.0f*dSample)],
                                                     LoadedSound->Samples[0][RoundReal32ToInt32(SamplePosition + 2.0f*dSample)],
                                                     LoadedSound->Samples[0][RoundReal32ToInt32(SamplePosition + 3.0f*dSample)]);
#endif

                    __m128 D0 = _mm_load_ps((float *)&Dest0[0]);
                    __m128 D1 = _mm_load_ps((float *)&Dest1[0]);

                    D0 = _mm_add_ps(D0, _mm_mul_ps(_mm_mul_ps(MasterVolume0, Volume0), SampleValue));
                    D1 = _mm_add_ps(D1, _mm_mul_ps(_mm_mul_ps(MasterVolume1, Volume1), SampleValue));

                    _mm_store_ps((float *)&Dest0[0], D0);
                    _mm_store_ps((float *)&Dest1[0], D1);

                    ++Dest0;
                    ++Dest1;
                    Volume0 = _mm_add_ps(Volume0, dVolumeChunk0);
                    Volume1 = _mm_add_ps(Volume1, dVolumeChunk1);
                }

                PlayingSound->CurrentVolume.E[0] = ((real32 *)&Volume0)[0];
                PlayingSound->CurrentVolume.E[1] = ((real32 *)&Volume1)[1];
                for(u32 ChannelIndex = 0;
                    ChannelIndex < ArrayCount(VolumeEndsAt);
                    ++ChannelIndex)
                {
                    if(ChunksToMix == VolumeEndsAt[ChannelIndex])
                    {
                        PlayingSound->CurrentVolume.E[ChannelIndex] =
                            PlayingSound->TargetVolume.E[ChannelIndex];
                        PlayingSound->dCurrentVolume.E[ChannelIndex] = 0.0f;
                    }
                }
                
                PlayingSound->SamplesPlayed = EndSamplePosition;
                Assert(TotalChunksToMix >= ChunksToMix);
                TotalChunksToMix -= ChunksToMix;

                if(ChunksToMix == ChunksRemainingInSound)
                {
                    if(IsValid(NextSoundInChain))
                    {
                        PlayingSound->ID = NextSoundInChain;                            
                        Assert(PlayingSound->SamplesPlayed >= LoadedSound->SampleCount);
                        PlayingSound->SamplesPlayed -= (real32)LoadedSound->SampleCount;
                        if(PlayingSound->SamplesPlayed < 0)
                        {
                            PlayingSound->SamplesPlayed = 0.0f;
                        }
                    }
                    else
                    {
                        SoundFinished = true;
                    }
                }
            }
            else
            {
                LoadSound(Assets, PlayingSound->ID);
                break;
            }
        }

        if(SoundFinished)
        {
            *PlayingSoundPtr = PlayingSound->Next;
            PlayingSound->Next = AudioState->FirstFreePlayingSound;
            AudioState->FirstFreePlayingSound = PlayingSound;
        }
        else
        {
            PlayingSoundPtr = &PlayingSound->Next;
        }
    }

    // NOTE(casey): Convert to 16-bit
    {
        __m128 *Source0 = RealChannel0;
        __m128 *Source1 = RealChannel1;

        __m128i *SampleOut = (__m128i *)SoundBuffer->Samples;
        for(u32 SampleIndex = 0;
            SampleIndex < ChunkCount;
            ++SampleIndex)
        {
            __m128 S0 = _mm_load_ps((float *)Source0++);
            __m128 S1 = _mm_load_ps((float *)Source1++);
            
            __m128i L = _mm_cvtps_epi32(S0);
            __m128i R = _mm_cvtps_epi32(S1);
            
            __m128i LR0 = _mm_unpacklo_epi32(L, R);
            __m128i LR1 = _mm_unpackhi_epi32(L, R);
            
            __m128i S01 = _mm_packs_epi32(LR0, LR1);

            *SampleOut++ = S01;
        }
    }

    EndGeneration(Assets, GenerationID);
    EndTemporaryMemory(MixerMemory);
}

internal void
InitializeAudioState(audio_state *AudioState, memory_arena *PermArena)
{
    AudioState->PermArena = PermArena;
    AudioState->FirstPlayingSound = 0;
    AudioState->FirstFreePlayingSound = 0;

    AudioState->MasterVolume = V2(1.0f, 1.0f);
}
