/* -*-c++-*-
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2010, Alion Science and Technology
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <prefix/dtgameprefix.h>
#include <dtGame/gmimpl.h>
#include <dtGame/basemessages.h>
#include <dtGame/messagetype.h>


//////////////////////////////////////////////////////////////////////////
void dtGame::GMImpl::ClearTimerSingleSet(std::set<TimerInfo> &timerSet, 
                                         const std::string &name,
                                         const dtGame::GameActorProxy *proxy)
{
   std::set<TimerInfo>::iterator i = timerSet.begin();
   while (i != timerSet.end())
   {
      std::set<TimerInfo>::iterator toDelete;
      const TimerInfo& timer = *i;
      if (timer.name == name &&  (proxy == NULL || timer.aboutActor == proxy->GetId()))
      {
         toDelete = i;
         ++i;
         timerSet.erase(toDelete);
      }
      else
      {
         ++i;
      }
   }
}

//////////////////////////////////////////////////////////////////////////
void dtGame::GMImpl::ClearTimersForActor(std::set<TimerInfo>& timerSet, const GameActorProxy& proxy)
{
   std::set<TimerInfo>::iterator i = timerSet.begin();
   while (i != timerSet.end())
   {
      std::set<TimerInfo>::iterator toDelete;
      const TimerInfo& timer = *i;
      if (timer.aboutActor == proxy.GetId())
      {
         toDelete = i;
         ++i;
         timerSet.erase(toDelete);
      }
      else
      {
         ++i;
      }
   }
}

////////////////////////////////////////////////////////////////////////////////
dtGame::GMImpl::GMImpl(dtCore::Scene& scene) : mGMStatistics()
, mMachineInfo( new MachineInfo())
, mEnvironment(NULL)
//, mSendCreatesAndDeletes(true)
//, mAddActorsToScene(true)
, mFactory("GameManager MessageFactory", *mMachineInfo, "")
, mScene(&scene)
, mLibMgr(&dtDAL::LibraryManager::GetInstance())
, mApplication(NULL)
, mLogger(&dtUtil::Log::GetInstance("gamemanager.cpp"))
, mGMSettings(new GMSettings())
, mRemoveGameEventsOnMapChange(true)
, mShuttingDown(false)
{

}
////////////////////////////////////////////////////////////////////////////////
void dtGame::GMImpl::ProcessTimers(GameManager& gm, std::set<TimerInfo>& listToProcess, dtCore::Timer_t clockTime)
{
   std::set<TimerInfo>::iterator itor;
   std::set<TimerInfo> repeatingTimers;
   for (itor=listToProcess.begin(); itor!=listToProcess.end();)
   {
      if (itor->time <= clockTime)
      {
         dtCore::RefPtr<TimerElapsedMessage> timerMsg =
            static_cast<TimerElapsedMessage*>(mFactory.CreateMessage(MessageType::INFO_TIMER_ELAPSED).get());

         timerMsg->SetTimerName(itor->name);
         float lateTime = float((clockTime - itor->time));
         // convert from microseconds to seconds
         lateTime /= 1e6;
         timerMsg->SetLateTime(lateTime);
         timerMsg->SetAboutActorId(itor->aboutActor);
         gm.SendMessage(*timerMsg.get());
         if (itor->repeat)
         {
            TimerInfo tInfo = *itor;
            tInfo.time += tInfo.interval;
            repeatingTimers.insert(tInfo);
         }

         std::set<TimerInfo>::iterator toDelete = itor;
         ++itor;
         listToProcess.erase(toDelete);
      }
      else
      {
         break;
      }
   }

   // Repeating timers have to be readded so they are processed again later
   listToProcess.insert(repeatingTimers.begin(), repeatingTimers.end());
}

////////////////////////////////////////////////////////////////////////////////
void dtGame::GMImpl::RemoveActorFromScene(GameManager& gm, dtDAL::BaseActorObject& proxy)
{
   dtCore::DeltaDrawable& dd = *proxy.GetActor();

   if (dd.GetSceneParent() != mScene.get())
   {
      return;
   }

   // find all of the children that have actor proxies associated with them to move them up
   // one level in the scene.
   std::vector< dtCore::RefPtr<dtDAL::BaseActorObject> > childrenToMove;
   for (unsigned i = 0; i < dd.GetNumChildren(); ++i)
   {
      dtCore::DeltaDrawable& child = *dd.GetChild(i);
      dtDAL::BaseActorObject* childProxy = gm.FindActorById(child.GetUniqueId());
      if (childProxy != NULL)
      {
         childrenToMove.push_back(childProxy);
      }
   }

   if (dd.GetParent() == NULL)
   {
      // remove the proxy drawable
      mScene->RemoveDrawable(&dd);

      // put all the children in the base scene.
      for (size_t i = 0; i < childrenToMove.size(); ++i)
      {
         mScene->AddDrawable(childrenToMove[i]->GetActor());
      }
   }
   else
   {
      // add all the children to the parent drawable.
      for (size_t i = 0; i < childrenToMove.size(); ++i)
      {
         dtCore::DeltaDrawable* child = childrenToMove[i]->GetActor();
         child->Emancipate();
         dd.GetParent()->AddChild(child);
      }
      // remove the proxy drawable from the parent.
      dd.Emancipate();
   }
}

////////////////////////////////////////////////////////////////////////////////
void dtGame::GMImpl::SendEnvironmentChangedMessage(GameManager& gm, IEnvGameActorProxy* envActor)
{
   dtCore::RefPtr<Message> msg = mFactory.CreateMessage(MessageType::INFO_ENVIRONMENT_CHANGED);
   msg->SetAboutActorId(mEnvironment.valid() ? envActor->GetActor()->GetUniqueId() : dtCore::UniqueId(""));
   msg->SetSource(*mMachineInfo);
   gm.SendMessage(*msg);
}