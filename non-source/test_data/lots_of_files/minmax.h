/***
*minmax.h - familiar min & max macros
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Defines min and max macros.
*
*       [Public]
*
****/

#pragma once

#ifndef _INC_MINMAX
#define _INC_MINMAX

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif  /* max */

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif  /* min */

#endif  /* _INC_MINMAX */
