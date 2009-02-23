// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#include "CommonHeaders.h"
#ifdef REX_THREADED
#  include "linden_common.h"
#undef REX_THREADED
#endif
#include "ClothThread.h"
#include "Cloth.h"

namespace Rex
{
   void ClothThread::run(void)
   {
#ifdef REX_THREADED
//      size_t n;
//      std::set<Cloth*>::iterator iter = ClothList.begin();
//      for ( ; iter != ClothList.end() ; ++iter)
////      for (n=0 ; n<ClothList.size() ; ++n)
//      {
//         (*iter)->reform();
////         ClothList[n]->reform();
//      }
      LLTimer timer;
      timer.start();
      F32 LeftOverMs = 0;
      while (1)
      {
         F32 elapsedTime = timer.getElapsedTimeAndResetF32();
////         Ogre::LogManager::getSingleton().logMessage("elapsed time ms: " + Ogre::StringConverter::toString(elapsedTime*1000.0f));

////         LeftOverMs = std::min(16.0f, std::max(0.0f, 16.0f - (elapsedTime - LeftOverMs)));
////         LeftOverMs = std::max(0.0f, 16.0f - (elapsedTime - LeftOverMs));
         LeftOverMs = std::max(0.0f, 0.016f - (elapsedTime - LeftOverMs));
         ms_sleep(LeftOverMs * 1000.0f);
////         ms_sleep(16);
////         Ogre::LogManager::getSingleton().logMessage("elapsed time ms: " + Ogre::StringConverter::toString(1000.0f/(elapsedTime*1000.0f)) + " leftOverMs: " + Ogre::StringConverter::toString(LeftOverMs));
//         Ogre::LogManager::getSingleton().logMessage("elapsed time ms: " + Ogre::StringConverter::toString(elapsedTime*1000.0f) + " leftOverMs: " + Ogre::StringConverter::toString(LeftOverMs));

         this->checkPause();
         if (mStatus == QUITTING)
            return;
         

         // Instead of using mutex here, we could have a list clothes that need to be removed
         // and remove them here. Using mutex here might be bad since calculating all the cloth
         // phycis might take quite a few ms, on the other hand, we rarely need to remove
         // clothes so it probably is not too bad.
         ClothListMutex->lock();
         std::set<Cloth*>::iterator iter = ClothList.begin();
         for ( ; iter != ClothList.end() ; ++iter)
         {
            lockVertexMutex(*iter);
         //   ClothList[n]->generateCollisionData();
            (*iter)->applyConstraints();
            (*iter)->applyForceAndModifiers(elapsedTime, false);
//            (*iter)->applyForceAndModifiers(0.01f, false);
            (*iter)->recalculateNormals();
            unlockVertexMutex(*iter);
         }
         ClothListMutex->unlock();
         

         //Ogre::Vector3 pointA(-10, 0, 0);
         //Ogre::Vector3 pointB(10, 0, 0);
         //Ogre::Vector3 pointC(0, 0, 10);

         //Ogre::Ray collisionRay(Ogre::Vector3(0, 50, 0), Ogre::Vector3::NEGATIVE_UNIT_Y);
         //for (n=0 ; n<500*2000 ; ++n)
         //{
         //   pointA.x = -10;
         //   pointA.y = 0;
         //   pointB.y = 0;
         //   pointC.x = 0;
         //   pointC.y = 0;
      
         //   std::pair<bool, Ogre::Real> result = Ogre::Math::intersects(collisionRay, pointA, pointB, pointC, Ogre::Vector3::UNIT_Y, true, true);
         //   if (result.first)
         //   {
         //      pointC.x += result.second;
         //   }
         //}
      }
#endif
   }

   void ClothThread::addCloth(Cloth *cloth)
   {
#ifdef REX_THREADED
      ClothList.insert(cloth);
#endif
//      ClothList.push
//      ClothList.push_back(cloth);
   }

   void ClothThread::removeCloth(Cloth *cloth)
   {
#ifdef REX_THREADED
      if (ClothList.find(cloth) != ClothList.end())
      {
         ClothListMutex->lock();
         ClothList.erase(cloth);
         ClothListMutex->unlock();
      }
#endif
   }

   void ClothThread::lockVertexMutex(const Cloth *cloth)
   {
#ifdef REX_THREADED
      if (VertexMutexes.find(cloth) == VertexMutexes.end())
         VertexMutexes[cloth] = new LLMutex(0);
      VertexMutexes[cloth]->lock();
#endif
   }

   void ClothThread::unlockVertexMutex(const Cloth *cloth)
   {
#ifdef REX_THREADED
      VertexMutexes[cloth]->unlock();
#endif
   }
}
