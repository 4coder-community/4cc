/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

enum finalize_asset_operation
{
    FinalizeAsset_None,
    FinalizeAsset_Font,
};
struct load_asset_work
{
    task_with_memory *Task;
    asset *Asset;    

    platform_file_handle *Handle;
    u64 Offset;
    u64 Size;
    void *Destination;

    finalize_asset_operation FinalizeOperation;
    u32 FinalState;
};
internal void
LoadAssetWorkDirectly(load_asset_work *Work)
{
    TIMED_FUNCTION();
    
    Platform.ReadDataFromFile(Work->Handle, Work->Offset, Work->Size, Work->Destination);
    if(PlatformNoFileErrors(Work->Handle))
    {
        switch(Work->FinalizeOperation)
        {
            case FinalizeAsset_None:
            {
                // NOTE(casey): Nothing to do.
            } break;

            case FinalizeAsset_Font:
            {
                loaded_font *Font = &Work->Asset->Header->Font;
                hha_font *HHA = &Work->Asset->HHA.Font;
                for(u32 GlyphIndex = 1;
                    GlyphIndex < HHA->GlyphCount;
                    ++GlyphIndex)
                {
                    hha_font_glyph *Glyph = Font->Glyphs + GlyphIndex;

                    Assert(Glyph->UnicodeCodePoint < HHA->OnePastHighestCodepoint);
                    Assert((u32)(u16)GlyphIndex == GlyphIndex);
                    Font->UnicodeMap[Glyph->UnicodeCodePoint] = (u16)GlyphIndex;
                }
            } break;
        }
    }

    CompletePreviousWritesBeforeFutureWrites;

    if(!PlatformNoFileErrors(Work->Handle))
    {
        ZeroSize(Work->Size, Work->Destination);
    }

    Work->Asset->State = Work->FinalState;
}
internal PLATFORM_WORK_QUEUE_CALLBACK(LoadAssetWork)
{
    load_asset_work *Work = (load_asset_work *)Data;

    LoadAssetWorkDirectly(Work);

    EndTaskWithMemory(Work->Task);
}

inline asset_file *
GetFile(game_assets *Assets, u32 FileIndex)
{
    Assert(FileIndex < Assets->FileCount);
    asset_file *Result = Assets->Files + FileIndex;
    
    return(Result);
}

inline platform_file_handle *
GetFileHandleFor(game_assets *Assets, u32 FileIndex)
{
    platform_file_handle *Result = &GetFile(Assets, FileIndex)->Handle;
    
    return(Result);
}

internal asset_memory_block *
InsertBlock(asset_memory_block *Prev, u64 Size, void *Memory)
{
    Assert(Size > sizeof(asset_memory_block));
    asset_memory_block *Block = (asset_memory_block *)Memory;
    Block->Flags = 0;
    Block->Size = Size - sizeof(asset_memory_block);
    Block->Prev = Prev;
    Block->Next = Prev->Next;
    Block->Prev->Next = Block;
    Block->Next->Prev = Block;
    return(Block);
}

internal asset_memory_block *
FindBlockForSize(game_assets *Assets, memory_index Size)
{
    asset_memory_block *Result = 0;

    // TODO(casey): This probably will need to be accelerated in the
    // future as the resident asset count grows.
    
    // TODO(casey): Best match block!
    for(asset_memory_block *Block = Assets->MemorySentinel.Next;
        Block != &Assets->MemorySentinel;
        Block = Block->Next)
    {
        if(!(Block->Flags & AssetMemory_Used))
        {
            if(Block->Size >= Size)
            {
                Result = Block;
                break;
            }
        }
    }

    return(Result);
}

internal b32
MergeIfPossible(game_assets *Assets, asset_memory_block *First, asset_memory_block *Second)
{
    b32 Result = false;
    
    if((First != &Assets->MemorySentinel) &&
       (Second != &Assets->MemorySentinel))
    {
        if(!(First->Flags & AssetMemory_Used) &&
           !(Second->Flags & AssetMemory_Used))
        {
            u8 *ExpectedSecond = (u8 *)First + sizeof(asset_memory_block) + First->Size;
            if((u8 *)Second == ExpectedSecond)
            {
                Second->Next->Prev = Second->Prev;
                Second->Prev->Next = Second->Next;
                
                First->Size += sizeof(asset_memory_block) + Second->Size;

                Result = true;
            }
        }
    }

    return(Result);
}

internal b32
GenerationHasCompleted(game_assets *Assets, u32 CheckID)
{
    b32 Result = true;
    
    for(u32 Index = 0;
        Index < Assets->InFlightGenerationCount;
        ++Index)
    {
        if(Assets->InFlightGenerations[Index] == CheckID)
        {
            Result = false;
            break;
        }
    }

    return(Result);
}

internal asset_memory_header *
AcquireAssetMemory(game_assets *Assets, u32 Size, u32 AssetIndex)
{
    TIMED_FUNCTION();
    
    asset_memory_header *Result = 0;

    BeginAssetLock(Assets);
    
    asset_memory_block *Block = FindBlockForSize(Assets, Size);
    for(;;)
    {
        if(Block && (Size <= Block->Size))
        {
            Block->Flags |= AssetMemory_Used;
            
            Result = (asset_memory_header *)(Block + 1);

            memory_index RemainingSize = Block->Size - Size;
            memory_index BlockSplitThreshold = 4096; // TODO(casey): Set this based on the smallest asset?
            if(RemainingSize > BlockSplitThreshold)
            {
                Block->Size -= RemainingSize;
                InsertBlock(Block, RemainingSize, (u8 *)Result + Size);
            }
            else
            {
                // TODO(casey): Actually record the unused portion of the memory
                // in a block so that we can do the merge on blocks when neighbors
                // are freed.
            }
            
            break;
        }
        else
        {
            for(asset_memory_header *Header = Assets->LoadedAssetSentinel.Prev;
                Header != &Assets->LoadedAssetSentinel;
                Header = Header->Prev)
            {
                asset *Asset = Assets->Assets + Header->AssetIndex;
                if((Asset->State >= AssetState_Loaded) &&
                   (GenerationHasCompleted(Assets, Asset->Header->GenerationID)))
                {
                    u32 AssetIndex = Header->AssetIndex;
                    asset *Asset = Assets->Assets + AssetIndex;
    
                    Assert(Asset->State == AssetState_Loaded);

                    RemoveAssetHeaderFromList(Header);
                    
                    Block = (asset_memory_block *)Asset->Header - 1;
                    Block->Flags &= ~AssetMemory_Used;

                    if(MergeIfPossible(Assets, Block->Prev, Block))
                    {
                        Block = Block->Prev;
                    }

                    MergeIfPossible(Assets, Block, Block->Next);

                    Asset->State = AssetState_Unloaded;
                    Asset->Header = 0;    
                    break;
                }
            }
        }
    }

    if(Result)
    {
        Result->AssetIndex = AssetIndex;
        Result->TotalSize = Size;
        InsertAssetHeaderAtFront(Assets, Result);
    }

    EndAssetLock(Assets);
    
    return(Result);
}

struct asset_memory_size
{
    u32 Total;
    u32 Data;
    u32 Section;
};
    
internal void
LoadBitmap(game_assets *Assets, bitmap_id ID, b32 Immediate)
{
    TIMED_FUNCTION();
    
    asset *Asset = Assets->Assets + ID.Value;        
    if(ID.Value)
    {
        if(AtomicCompareExchangeUInt32((uint32 *)&Asset->State, AssetState_Queued, AssetState_Unloaded) ==
           AssetState_Unloaded)
        {
            task_with_memory *Task = 0;

            if(!Immediate)
            {
                Task = BeginTaskWithMemory(Assets->TranState);
            }
        
            if(Immediate || Task)        
            {
                asset *Asset = Assets->Assets + ID.Value;
                hha_bitmap *Info = &Asset->HHA.Bitmap;

                asset_memory_size Size = {};
                u32 Width = Info->Dim[0];
                u32 Height = Info->Dim[1];
                Size.Section = 4*Width;
                Size.Data = Height*Size.Section;
                Size.Total = Size.Data + sizeof(asset_memory_header);

                Asset->Header = AcquireAssetMemory(Assets, Size.Total, ID.Value);

                loaded_bitmap *Bitmap = &Asset->Header->Bitmap;            
                Bitmap->AlignPercentage = V2(Info->AlignPercentage[0], Info->AlignPercentage[1]);
                Bitmap->WidthOverHeight = (r32)Info->Dim[0] / (r32)Info->Dim[1];
                Bitmap->Width = Info->Dim[0];
                Bitmap->Height = Info->Dim[1];
                Bitmap->Pitch = Size.Section;
                Bitmap->Memory = (Asset->Header + 1);

                load_asset_work Work;
                Work.Task = Task;
                Work.Asset = Assets->Assets + ID.Value;
                Work.Handle = GetFileHandleFor(Assets, Asset->FileIndex);
                Work.Offset = Asset->HHA.DataOffset;
                Work.Size = Size.Data;
                Work.Destination = Bitmap->Memory;
                Work.FinalizeOperation = FinalizeAsset_None;
                Work.FinalState = AssetState_Loaded;            
                if(Task)
                {
                    load_asset_work *TaskWork = PushStruct(&Task->Arena, load_asset_work);
                    *TaskWork = Work;
                    Platform.AddEntry(Assets->TranState->LowPriorityQueue, LoadAssetWork, TaskWork);
                }
                else
                {
                    LoadAssetWorkDirectly(&Work);
                }
            }
            else
            {
                Asset->State = AssetState_Unloaded;
            }
        }
        else if(Immediate)
        {
            // TODO(casey): Do we want to have a more coherent story here
            // for what happens when two force-load people hit the load
            // at the same time?
            asset_state volatile *State = (asset_state volatile *)&Asset->State;
            while(*State == AssetState_Queued) {}
        }
    }    
}

internal void
LoadSound(game_assets *Assets, sound_id ID)
{
    TIMED_FUNCTION();
    
    asset *Asset = Assets->Assets + ID.Value;        
    if(ID.Value &&
       (AtomicCompareExchangeUInt32((uint32 *)&Asset->State, AssetState_Queued, AssetState_Unloaded) ==
        AssetState_Unloaded))
    {    
        task_with_memory *Task = BeginTaskWithMemory(Assets->TranState);
        if(Task)        
        {
            asset *Asset = Assets->Assets + ID.Value;
            hha_sound *Info = &Asset->HHA.Sound;

            asset_memory_size Size = {};
            Size.Section = Info->SampleCount*sizeof(int16);
            Size.Data = Info->ChannelCount*Size.Section;
            Size.Total = Size.Data + sizeof(asset_memory_header);

            Asset->Header = (asset_memory_header *)AcquireAssetMemory(Assets, Size.Total, ID.Value);
            loaded_sound *Sound = &Asset->Header->Sound;
            
            Sound->SampleCount = Info->SampleCount;
            Sound->ChannelCount = Info->ChannelCount;
            u32 ChannelSize = Size.Section;

            void *Memory = (Asset->Header + 1);
            int16 *SoundAt = (int16 *)Memory;
            for(u32 ChannelIndex = 0;
                ChannelIndex < Sound->ChannelCount;
                ++ChannelIndex)
            {
                Sound->Samples[ChannelIndex] = SoundAt;
                SoundAt += ChannelSize;
            }
 
            load_asset_work *Work = PushStruct(&Task->Arena, load_asset_work);
            Work->Task = Task;
            Work->Asset = Assets->Assets + ID.Value;
            Work->Handle = GetFileHandleFor(Assets, Asset->FileIndex);
            Work->Offset = Asset->HHA.DataOffset;
            Work->Size = Size.Data;
            Work->Destination = Memory;
            Work->FinalizeOperation = FinalizeAsset_None;
            Work->FinalState = (AssetState_Loaded);

            Platform.AddEntry(Assets->TranState->LowPriorityQueue, LoadAssetWork, Work);
        }
        else
        {
            Assets->Assets[ID.Value].State = AssetState_Unloaded;
        }
    }
}
    
internal void
LoadFont(game_assets *Assets, font_id ID, b32 Immediate)
{
    TIMED_FUNCTION();
    
    // TODO(casey): Merge all this boilerplate!!!!  Same between LoadBitmap, LoadSound, and LoadFont
    asset *Asset = Assets->Assets + ID.Value;        
    if(ID.Value)
    {
        if(AtomicCompareExchangeUInt32((uint32 *)&Asset->State, AssetState_Queued, AssetState_Unloaded) ==
           AssetState_Unloaded)
        {
            task_with_memory *Task = 0;

            if(!Immediate)
            {
                Task = BeginTaskWithMemory(Assets->TranState);
            }
        
            if(Immediate || Task)        
            {
                asset *Asset = Assets->Assets + ID.Value;
                hha_font *Info = &Asset->HHA.Font;

                u32 HorizontalAdvanceSize = sizeof(r32)*Info->GlyphCount*Info->GlyphCount;
                u32 GlyphsSize = Info->GlyphCount*sizeof(hha_font_glyph);
                u32 UnicodeMapSize = sizeof(u16)*Info->OnePastHighestCodepoint;
                u32 SizeData = GlyphsSize + HorizontalAdvanceSize;
                u32 SizeTotal = SizeData + sizeof(asset_memory_header) + UnicodeMapSize;

                Asset->Header = AcquireAssetMemory(Assets, SizeTotal, ID.Value);

                loaded_font *Font = &Asset->Header->Font;
                Font->BitmapIDOffset = GetFile(Assets, Asset->FileIndex)->FontBitmapIDOffset;
                Font->Glyphs = (hha_font_glyph *)(Asset->Header + 1);
                Font->HorizontalAdvance = (r32 *)((u8 *)Font->Glyphs + GlyphsSize);
                Font->UnicodeMap = (u16 *)((u8 *)Font->HorizontalAdvance + HorizontalAdvanceSize);

                ZeroSize(UnicodeMapSize, Font->UnicodeMap);

                load_asset_work Work;
                Work.Task = Task;
                Work.Asset = Assets->Assets + ID.Value;
                Work.Handle = GetFileHandleFor(Assets, Asset->FileIndex);
                Work.Offset = Asset->HHA.DataOffset;
                Work.Size = SizeData;
                Work.Destination = Font->Glyphs;
                Work.FinalizeOperation = FinalizeAsset_Font;
                Work.FinalState = AssetState_Loaded;            
                if(Task)
                {
                    load_asset_work *TaskWork = PushStruct(&Task->Arena, load_asset_work);
                    *TaskWork = Work;
                    Platform.AddEntry(Assets->TranState->LowPriorityQueue, LoadAssetWork, TaskWork);
                }
                else
                {
                    LoadAssetWorkDirectly(&Work);
                }
            }
            else
            {
                Asset->State = AssetState_Unloaded;
            }
        }
        else if(Immediate)
        {
            // TODO(casey): Do we want to have a more coherent story here
            // for what happens when two force-load people hit the load
            // at the same time?
            asset_state volatile *State = (asset_state volatile *)&Asset->State;
            while(*State == AssetState_Queued) {}
        }
    }    
}

internal uint32
GetBestMatchAssetFrom(game_assets *Assets, asset_type_id TypeID,
                      asset_vector *MatchVector, asset_vector *WeightVector)
{
    TIMED_FUNCTION();
    
    uint32 Result = 0;

    real32 BestDiff = Real32Maximum;
    asset_type *Type = Assets->AssetTypes + TypeID;
    for(uint32 AssetIndex = Type->FirstAssetIndex;
        AssetIndex < Type->OnePastLastAssetIndex;
        ++AssetIndex)
    {
        asset *Asset = Assets->Assets + AssetIndex;

        real32 TotalWeightedDiff = 0.0f;
        for(uint32 TagIndex = Asset->HHA.FirstTagIndex;
            TagIndex < Asset->HHA.OnePastLastTagIndex;
            ++TagIndex)
        {
            hha_tag *Tag = Assets->Tags + TagIndex;

            real32 A = MatchVector->E[Tag->ID];
            real32 B = Tag->Value;
            real32 D0 = AbsoluteValue(A - B);
            real32 D1 = AbsoluteValue((A - Assets->TagRange[Tag->ID]*SignOf(A)) - B);
            real32 Difference = Minimum(D0, D1);
            
            real32 Weighted = WeightVector->E[Tag->ID]*Difference;
            TotalWeightedDiff += Weighted;
        }

        if(BestDiff > TotalWeightedDiff)
        {
            BestDiff = TotalWeightedDiff;
            Result = AssetIndex;
        }
    }

    return(Result);
}

internal uint32
GetRandomAssetFrom(game_assets *Assets, asset_type_id TypeID, random_series *Series)
{
    TIMED_FUNCTION();
    
    uint32 Result = 0;

    asset_type *Type = Assets->AssetTypes + TypeID;
    if(Type->FirstAssetIndex != Type->OnePastLastAssetIndex)
    {
        uint32 Count = (Type->OnePastLastAssetIndex - Type->FirstAssetIndex);
        uint32 Choice = RandomChoice(Series, Count);
        Result = Type->FirstAssetIndex + Choice;
    }

    return(Result);
}

internal uint32
GetFirstAssetFrom(game_assets *Assets, asset_type_id TypeID)
{
    TIMED_FUNCTION();
    
    uint32 Result = 0;

    asset_type *Type = Assets->AssetTypes + TypeID;
    if(Type->FirstAssetIndex != Type->OnePastLastAssetIndex)
    {
        Result = Type->FirstAssetIndex;
    }

    return(Result);
}

inline bitmap_id
GetBestMatchBitmapFrom(game_assets *Assets, asset_type_id TypeID,
                       asset_vector *MatchVector, asset_vector *WeightVector)
{
    bitmap_id Result = {GetBestMatchAssetFrom(Assets, TypeID, MatchVector, WeightVector)};
    return(Result);
}

inline bitmap_id
GetFirstBitmapFrom(game_assets *Assets, asset_type_id TypeID)
{
    bitmap_id Result = {GetFirstAssetFrom(Assets, TypeID)};
    return(Result);
}

inline bitmap_id
GetRandomBitmapFrom(game_assets *Assets, asset_type_id TypeID, random_series *Series)
{
    bitmap_id Result = {GetRandomAssetFrom(Assets, TypeID, Series)};
    return(Result);
}

inline sound_id
GetBestMatchSoundFrom(game_assets *Assets, asset_type_id TypeID,
                       asset_vector *MatchVector, asset_vector *WeightVector)
{
    sound_id Result = {GetBestMatchAssetFrom(Assets, TypeID, MatchVector, WeightVector)};
    return(Result);
}

inline sound_id
GetFirstSoundFrom(game_assets *Assets, asset_type_id TypeID)
{
    sound_id Result = {GetFirstAssetFrom(Assets, TypeID)};
    return(Result);
}

inline sound_id
GetRandomSoundFrom(game_assets *Assets, asset_type_id TypeID, random_series *Series)
{
    sound_id Result = {GetRandomAssetFrom(Assets, TypeID, Series)};
    return(Result);
}

internal font_id
GetBestMatchFontFrom(game_assets *Assets, asset_type_id TypeID, asset_vector *MatchVector, asset_vector *WeightVector)
{
    font_id Result = {GetBestMatchAssetFrom(Assets, TypeID, MatchVector, WeightVector)};
    return(Result);
}

internal game_assets *
AllocateGameAssets(memory_arena *Arena, memory_index Size, transient_state *TranState)
{
    TIMED_FUNCTION();
    
    game_assets *Assets = PushStruct(Arena, game_assets);

    Assets->NextGenerationID = 0;
    Assets->InFlightGenerationCount = 0;    
    
    Assets->MemorySentinel.Flags = 0;
    Assets->MemorySentinel.Size = 0;
    Assets->MemorySentinel.Prev = &Assets->MemorySentinel;
    Assets->MemorySentinel.Next = &Assets->MemorySentinel;

    InsertBlock(&Assets->MemorySentinel, Size, PushSize(Arena, Size));
    
    Assets->TranState = TranState;

    Assets->LoadedAssetSentinel.Next = 
        Assets->LoadedAssetSentinel.Prev =
        &Assets->LoadedAssetSentinel;
    
    for(uint32 TagType = 0;
        TagType < Tag_Count;
        ++TagType)
    {
        Assets->TagRange[TagType] = 1000000.0f;
    }
    Assets->TagRange[Tag_FacingDirection] = Tau32;

    Assets->TagCount = 1;
    Assets->AssetCount = 1;

    // NOTE(casey): This code was written using Snuffleupagus-Oriented Programming (SOP)
    {
        platform_file_group FileGroup = Platform.GetAllFilesOfTypeBegin(PlatformFileType_AssetFile);
        Assets->FileCount = FileGroup.FileCount;
        Assets->Files = PushArray(Arena, Assets->FileCount, asset_file);
        for(u32 FileIndex = 0;
            FileIndex < Assets->FileCount;
            ++FileIndex)
        {
            asset_file *File = Assets->Files + FileIndex;

            File->FontBitmapIDOffset = 0;
            File->TagBase = Assets->TagCount;

            ZeroStruct(File->Header);
            File->Handle = Platform.OpenNextFile(&FileGroup);
            Platform.ReadDataFromFile(&File->Handle, 0, sizeof(File->Header), &File->Header);
            
            u32 AssetTypeArraySize = File->Header.AssetTypeCount*sizeof(hha_asset_type);
            File->AssetTypeArray = (hha_asset_type *)PushSize(Arena, AssetTypeArraySize);
            Platform.ReadDataFromFile(&File->Handle, File->Header.AssetTypes,
                                      AssetTypeArraySize, File->AssetTypeArray);
            
            if(File->Header.MagicValue != HHA_MAGIC_VALUE)
            {
                Platform.FileError(&File->Handle, "HHA file has an invalid magic value.");
            }
        
            if(File->Header.Version > HHA_VERSION)
            {
                Platform.FileError(&File->Handle, "HHA file is of a later version.");
            }
        
            if(PlatformNoFileErrors(&File->Handle))
            {
                // NOTE(casey): The first asset and tag slot in every
                // HHA is a null (reserved) so we don't count it as
                // something we will need space for!
                Assets->TagCount += (File->Header.TagCount - 1);
                Assets->AssetCount += (File->Header.AssetCount - 1);
            }
            else
            {
                // TODO(casey): Eventually, have some way of notifying users of bogus files?
                InvalidCodePath;
            }
        }
        Platform.GetAllFilesOfTypeEnd(&FileGroup);
    }

    // NOTE(casey): Allocate all metadata space
    Assets->Assets = PushArray(Arena, Assets->AssetCount, asset);
    Assets->Tags = PushArray(Arena, Assets->TagCount, hha_tag);

    // NOTE(casey): Reserve one null tag at the beginning
    ZeroStruct(Assets->Tags[0]);

    // NOTE(casey): Load tags
    for(u32 FileIndex = 0;
        FileIndex < Assets->FileCount;
        ++FileIndex)
    {
        asset_file *File = Assets->Files + FileIndex;
        if(PlatformNoFileErrors(&File->Handle))
        {
            // NOTE(casey): Skip the first tag, since it's null
            u32 TagArraySize = sizeof(hha_tag)*(File->Header.TagCount - 1);
            Platform.ReadDataFromFile(&File->Handle, File->Header.Tags + sizeof(hha_tag),
                                      TagArraySize, Assets->Tags + File->TagBase);
        }
    }

    // NOTE(casey): Reserve one null asset at the beginning
    u32 AssetCount = 0;
    ZeroStruct(*(Assets->Assets + AssetCount));
    ++AssetCount;

    // TODO(casey): Excersize for the reader - how would you do this in a way
    // that scaled gracefully to hundreds of asset pack files?  (or more!)
    for(u32 DestTypeID = 0;
        DestTypeID < Asset_Count;
        ++DestTypeID)
    {
        asset_type *DestType = Assets->AssetTypes + DestTypeID;
        DestType->FirstAssetIndex = AssetCount;
        
        for(u32 FileIndex = 0;
            FileIndex < Assets->FileCount;
            ++FileIndex)
        {
            asset_file *File = Assets->Files + FileIndex;
            if(PlatformNoFileErrors(&File->Handle))
            {
                for(u32 SourceIndex = 0;
                    SourceIndex < File->Header.AssetTypeCount;
                    ++SourceIndex)
                {
                    hha_asset_type *SourceType = File->AssetTypeArray + SourceIndex;
                    
                    if(SourceType->TypeID == DestTypeID)
                    {
                        if(SourceType->TypeID == Asset_FontGlyph)
                        {
                            File->FontBitmapIDOffset = AssetCount - SourceType->FirstAssetIndex;
                        }
                        
                        u32 AssetCountForType = (SourceType->OnePastLastAssetIndex -
                                                 SourceType->FirstAssetIndex);

                        temporary_memory TempMem = BeginTemporaryMemory(&TranState->TranArena);
                        hha_asset *HHAAssetArray = PushArray(&TranState->TranArena,
                                                             AssetCountForType, hha_asset);
                        Platform.ReadDataFromFile(&File->Handle,
                                                  File->Header.Assets +
                                                  SourceType->FirstAssetIndex*sizeof(hha_asset),
                                                  AssetCountForType*sizeof(hha_asset),
                                                  HHAAssetArray);
                        for(u32 AssetIndex = 0;
                            AssetIndex < AssetCountForType;
                            ++AssetIndex)
                        {
                            hha_asset *HHAAsset = HHAAssetArray + AssetIndex;

                            Assert(AssetCount < Assets->AssetCount);
                            asset *Asset = Assets->Assets + AssetCount++;
                            
                            Asset->FileIndex = FileIndex;
                            Asset->HHA = *HHAAsset;
                            if(Asset->HHA.FirstTagIndex == 0)
                            {
                                Asset->HHA.FirstTagIndex = Asset->HHA.OnePastLastTagIndex = 0;
                            }
                            else
                            {
                                Asset->HHA.FirstTagIndex += (File->TagBase - 1);
                                Asset->HHA.OnePastLastTagIndex += (File->TagBase - 1);
                            }
                        }

                        EndTemporaryMemory(TempMem);
                    }
                }
            }
        }

        DestType->OnePastLastAssetIndex = AssetCount;
    }
    
    Assert(AssetCount == Assets->AssetCount);
    
    return(Assets);
}

inline u32
GetGlyphFromCodePoint(hha_font *Info, loaded_font *Font, u32 CodePoint)
{
    u32 Result = 0;
    if(CodePoint < Info->OnePastHighestCodepoint)
    {
        Result = Font->UnicodeMap[CodePoint];
        Assert(Result < Info->GlyphCount);
    }

    return(Result);
}

internal r32
GetHorizontalAdvanceForPair(hha_font *Info, loaded_font *Font, u32 DesiredPrevCodePoint, u32 DesiredCodePoint)
{
    u32 PrevGlyph = GetGlyphFromCodePoint(Info, Font, DesiredPrevCodePoint);
    u32 Glyph = GetGlyphFromCodePoint(Info, Font, DesiredCodePoint);
    
    r32 Result = Font->HorizontalAdvance[PrevGlyph*Info->GlyphCount + Glyph];
        
    return(Result);
}

internal bitmap_id
GetBitmapForGlyph(game_assets *Assets, hha_font *Info, loaded_font *Font, u32 DesiredCodePoint)
{
    u32 Glyph = GetGlyphFromCodePoint(Info, Font, DesiredCodePoint);    
    bitmap_id Result = Font->Glyphs[Glyph].BitmapID;
    Result.Value += Font->BitmapIDOffset;
    
    return(Result);
}

internal r32
GetLineAdvanceFor(hha_font *Info)
{
    r32 Result = Info->AscenderHeight + Info->DescenderHeight + Info->ExternalLeading;

    return(Result);
}

internal r32
GetStartingBaselineY(hha_font *Info)
{
    r32 Result = Info->AscenderHeight;

    return(Result);
}
