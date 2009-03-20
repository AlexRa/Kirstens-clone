// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

//! Contains delete definitions


//! compile-time assert
#undef SAFE_DELETE
#define SAFE_DELETE(obj) if (obj) {delete obj; obj=NULL;}

#undef SAFE_DELETE_ASSERT
#define SAFE_DELETE_ASSERT(obj) assert(obj); if (obj) {delete obj; obj=NULL};

