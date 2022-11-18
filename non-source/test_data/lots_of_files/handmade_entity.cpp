/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

inline move_spec
DefaultMoveSpec(void)
{
    move_spec Result;

    Result.UnitMaxAccelVector = false;
    Result.Speed = 1.0f;
    Result.Drag = 0.0f;

    return(Result);
}
