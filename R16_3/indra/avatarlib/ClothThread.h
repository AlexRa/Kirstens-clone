// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __ClothThread_h__
#define __ClothThread_h__

#ifdef REX_THREADED
#  include "llthread.h"
#endif
#include <set>

namespace Rex
{
   class Cloth;

   class FakeMutex
   {
   public:
      void lock() {};
      void unlock() {};
   };

   class ClothThread 
#ifdef REX_THREADED
      : public LLThread
#endif
   {
   public:
      ClothThread() : 
#ifdef REX_THREADED
         LLThread("ClothThread"),
#endif
         CollisionMutex(0)
      {
#     ifdef REX_THREADED
         CollisionMutex = new LLMutex(0);
         ClothListMutex = new LLMutex(0);
//         ClothMutex = new LLMutex(0);
#     endif
      }
      virtual ~ClothThread()
      {
         if (CollisionMutex)
         {
            delete (CollisionMutex);
            CollisionMutex = 0;
         }
#ifdef REX_THREADED
         if (ClothListMutex)
         {
            delete (ClothListMutex);
            ClothListMutex = 0;
         }

         std::map<const Cloth*, LLMutex*>::iterator iter = VertexMutexes.begin();
         for ( ; iter != VertexMutexes.end() ; ++iter)
         {
            delete iter->second;
         }
#endif
      }

      virtual void run(void);

      //! Add new threaded cloth item
      void addCloth(Cloth *cloth);

      //! Remove threaded cloth item
      void removeCloth(Cloth *cloth);

      //! Locks vertex mutex
      void lockVertexMutex(const Cloth *cloth);

      //! Unlocks vertex mutex
      void unlockVertexMutex(const Cloth *cloth);

#ifndef REX_THREADED
      void start(void) {}
      void pause(void) {}
      void unpause(void) {}
      void shutdown(void) {}

      FakeMutex *CollisionMutex;
      FakeMutex *ClothMutex;
#else
      LLMutex *CollisionMutex;
      LLMutex *ClothListMutex;

      std::map<const Cloth*, LLMutex*> VertexMutexes;
//      LLMutex *ClothMutex;
#endif

   private:
      double LeftOverMs;

      std::set<Cloth*> ClothList;
   };
}

#endif // __ClothThread_h__
