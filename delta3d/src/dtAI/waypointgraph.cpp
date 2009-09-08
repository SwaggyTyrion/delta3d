/*
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2009 Alion Science and Technology
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
 * Bradley Anderegg 
 */

#include <dtAI/waypointgraph.h>
#include <dtAI/waypointcollection.h>
#include <dtAI/aiplugininterface.h>
#include <dtAI/tree.h>
#include <dtAI/navmesh.h>
#include <dtAI/waypointpair.h>
#include <dtAI/aidebugdrawable.h>
#include <dtAI/waypointgraphbuilder.h>


#include <dtCore/observerptr.h>

#include <dtUtil/log.h>

#include <algorithm>
#include <map>


namespace dtAI
{
   //////////////////////////////////////////////////////////////////////////
   //WaypointGraph::SearchLevel
   //////////////////////////////////////////////////////////////////////////
   WaypointGraph::SearchLevel::SearchLevel()
      : mLevelNum(0)
   {

   }

   WaypointGraph::SearchLevel::~SearchLevel()
   {
      mNodes.clear();
      mNavMesh->Clear();
      mNavMesh = NULL;
   }



   //////////////////////////////////////////////////////////////////////////
   //WaypointGraphImpl
   //////////////////////////////////////////////////////////////////////////
   struct WaypointGraphImpl
   {
      struct WaypointHolder
      {
         WaypointHolder()
            : mLevel(0) 
            , mParent(0)
            , mWaypoint(0){}
              
         unsigned mLevel;
         WaypointCollection* mParent;
         const WaypointInterface* mWaypoint;
      };

      typedef std::map<WaypointID, WaypointHolder> WaypointMap;
      
      WaypointGraphImpl()
      {

      }

      void CleanUp()
      {
         mSearchLevels.clear();
         mWaypointOwnership.clear();
      }

      WaypointCollection* CastToCollection(WaypointCollection::WaypointTree* wt)
      {
         const WaypointCollection* col = dynamic_cast<const WaypointCollection*>(wt);
         if(col != NULL)
         {
            return const_cast<WaypointCollection*>(col);
         }

         return NULL;
      }

      const WaypointCollection* CastToCollection(const WaypointCollection::WaypointTree* wt) const 
      {
         const WaypointCollection* col = dynamic_cast<const WaypointCollection*>(wt);

         return col;
      }

      WaypointGraph::SearchLevel* CreateSearchLevel(unsigned level)
      {
         WaypointGraph::SearchLevel* sl = new WaypointGraph::SearchLevel();
         sl->mNavMesh = new NavMesh();
         sl->mLevelNum = level;
         
         //insert sorted
         WaypointGraph::SearchLevelArray::iterator iter = mSearchLevels.begin();
         WaypointGraph::SearchLevelArray::iterator iterEnd = mSearchLevels.end();

         for(;iter != iterEnd && (level > (*iter)->mLevelNum); ++iter)
         {
            //keep iterating
         }

         mSearchLevels.insert(iter, sl);

         return sl;
      }

      WaypointGraph::SearchLevel* FindSearchLevel(unsigned levelNum)
      {
         //we need to iterate through all the levels
         //to find the one with the matching level
         for(size_t i = 0; i < mSearchLevels.size(); ++i)
         {
            if(mSearchLevels[i]->mLevelNum == levelNum)
            {
               return mSearchLevels[i].get();
            }
         }
         return NULL;
      }

      const WaypointGraph::SearchLevel* FindSearchLevel(unsigned levelNum) const
      {
         //we need to iterate through all the levels
         //to find the one with the matching level
         for(size_t i = 0; i < mSearchLevels.size(); ++i)
         {
            if(mSearchLevels[i]->mLevelNum == levelNum)
            {
               return mSearchLevels[i].get();
            }
         }
         return NULL;
      }


      WaypointGraph::SearchLevel* GetSearchLevel(unsigned levelNum)
      {
         if(levelNum < mSearchLevels.size())
         {
            //try the obvious, which is each level is actually the index,
            //this is not guaranteed but the default builder will set it up that way
            if(mSearchLevels[levelNum]->mLevelNum == levelNum)
            {
               return mSearchLevels[levelNum];
            }
         }

         return FindSearchLevel(levelNum);
      }

      WaypointGraph::SearchLevel* GetOrCreateSearchLevel(unsigned levelNum)
      {
         WaypointGraph::SearchLevel* sl = GetSearchLevel(levelNum);
         if(sl == NULL)
         {
            sl = CreateSearchLevel(levelNum);            
         }

         return sl;
      }

      const WaypointGraph::SearchLevel* GetSearchLevel(unsigned levelNum) const
      {
         if(levelNum < mSearchLevels.size())
         {
            //try the obvious, which is each level is actually the index,
            //this is not guaranteed but the default builder will set it up that way
            if(mSearchLevels[levelNum]->mLevelNum == levelNum)
            {
               return mSearchLevels[levelNum];
            }
         }

         return FindSearchLevel(levelNum);
      }


      //wc can be NULL, if so it will make a new collection and add it
      void Insert(const WaypointInterface& waypoint, WaypointCollection* wcParent)
      {
         dtCore::RefPtr<WaypointCollection> wc = wcParent;

         //add to the waypoint map
         WaypointHolder wh;
         wh.mLevel = 0;
         //may be NULL
         wh.mParent = wc.get();
         wh.mWaypoint = &waypoint;

         if(wc.valid())
         {
            //Add to collection
            wc->Insert(&waypoint);
         }

         //add to search level 0
         GetOrCreateSearchLevel(0)->mNodes.push_back(&waypoint);

         mWaypointOwnership.insert(std::make_pair(waypoint.GetID(), wh));
      }

      void InsertCollection(dtCore::RefPtr<WaypointCollection> wc, unsigned level)
      {
         WaypointGraph::SearchLevel* sl = GetOrCreateSearchLevel(level);
         sl->mNodes.push_back(wc);

         //insert ownership
         WaypointHolder wh;
         wh.mLevel = level;
         //on waypoint collections the pointer points back to itelf         
         wh.mWaypoint = wc.get();
         wh.mParent = wc.get();

         mWaypointOwnership.insert(std::make_pair(wc->GetID(), wh));
      }


      //returns the top most node for a specific child
      WaypointCollection* GetRoot(WaypointCollection* wc)
      {
         WaypointCollection::WaypointTree* rootNode = wc;
         
         while(rootNode->parent() != NULL)
         {
            rootNode = rootNode->parent();
         }
            
         return CastToCollection(rootNode);
      }

      //returns the top most node for a specific child
      const WaypointCollection* GetRoot(const WaypointCollection* wc) const
      {
         const WaypointCollection::WaypointTree* rootNode = wc;

         while(rootNode->parent() != NULL)
         {
            rootNode = rootNode->parent();
         }

         return CastToCollection(rootNode);
      }


      void Remove(const WaypointInterface* waypoint)
      {
         const WaypointCollection* wcConst = dynamic_cast<const WaypointCollection*>(waypoint);
         if(wcConst != NULL)
         {
            WaypointCollection* wc = const_cast<WaypointCollection*>(wcConst);

            //erase all children
            WaypointCollection::WaypointTree::child_iterator iter = wc->begin_child();
            WaypointCollection::WaypointTree::child_iterator iterEnd = wc->end_child();
            for(;iter != iterEnd; ++iter)
            {            
               Remove(iter->value);
            }

            RemoveWaypoint(wc);
            wc->CleanUp();
            wc = NULL;

         }
         else
         {
            RemoveWaypoint(waypoint);               
         }
      }

      void RemoveWaypoint(const WaypointInterface* waypoint)
      {
         WaypointMap::iterator iter = mWaypointOwnership.find(waypoint->GetID());
         if(iter != mWaypointOwnership.end())
         {
            //remove the waypoint from the graph
            WaypointHolder& wh = (*iter).second;

            //remove the edges before you remove the waypoint
            WaypointGraph::SearchLevel* sl = GetSearchLevel(wh.mLevel);
            if(sl != NULL)
            {
               sl->mNavMesh->RemoveAllEdges(wh.mWaypoint);
               sl->mNodes.erase(std::remove(sl->mNodes.begin(), sl->mNodes.end(), wh.mWaypoint), sl->mNodes.end());
            }

            //erase the waypoint from the map
            mWaypointOwnership.erase(iter);
         }

      }
      

      //they have a path if they share a common parent
      bool HasPath(const WaypointCollection& lhs, const WaypointCollection& rhs) const
      {
         const WaypointCollection* wcParent = GetRoot(&lhs);
         if(rhs.is_descendant_of(*wcParent))
         {
            return true;
         }
         return false;
      }

      //void AddSearchLevel(dtCore::RefPtr<WaypointGraph::SearchLevel> newLevel)
      //{
      //   mSearchLevels.push_back(newLevel.get());

      //   //insert all nodes into the ownership map-?
      //}


      //WaypointCollection* FindCommonParent(const WaypointCollection& lhs, const WaypointCollection& rhs)
      //{
      //   return NULL;
      //}

      //dtCore::RefPtr<NavMesh> mNavMesh;
      //WaypointMap mWaypointOwnership;
      //WaypointCollectionArray mRootNode;


      WaypointMap mWaypointOwnership;
      WaypointGraph::SearchLevelArray mSearchLevels;
   };



   //////////////////////////////////////////////////////////////////////////
   //WaypointGraph
   //////////////////////////////////////////////////////////////////////////

   ///////////////////////////////////////////////////////////////////////////////
   WaypointGraph::WaypointGraph()
      : mImpl(new WaypointGraphImpl())
   {
   }

   /////////////////////////////////////////////////////////////////////////////
   WaypointGraph::~WaypointGraph()
   {      
      Clear();

      delete mImpl;
      mImpl = NULL;
   }

   /////////////////////////////////////////////////////////////////////////////
   void WaypointGraph::Clear()
   {
      OnClear();
   }
   
   /////////////////////////////////////////////////////////////////////////////
   void WaypointGraph::OnClear()
   {
      mImpl->CleanUp();
   }

   /////////////////////////////////////////////////////////////////////////////
   void WaypointGraph::CreateSearchLevel(WaypointGraphBuilder* builder, unsigned level)
   {
      if(level > 0)
      {
         builder->CreateNextSearchLevel(mImpl->GetSearchLevel(level - 1));
      }
      else
      {
         LOG_ERROR("Cannot create search level 0, search level 0 represents the concrete waypoints.");
      }

   }
  
   /////////////////////////////////////////////////////////////////////////////
   void WaypointGraph::InsertWaypoint(WaypointInterface* waypoint)
   {
      if(!Contains(waypoint->GetID()))
      {
         //passing in null will create a new one
         mImpl->Insert(*waypoint, NULL);
      }
      else //since we already have the waypoint, assume we are moving it,
           //a valid operation and noted in the header
      {
         //recalculate up the tree
         WaypointCollection* wc = FindCollection(waypoint->GetID());
         if(wc != NULL)
         {            
            wc->Recalculate();
         }
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   void WaypointGraph::InsertCollection(WaypointCollection* waypoint, unsigned level)
   {
      if(!Contains(waypoint->GetID()))
      {
         mImpl->InsertCollection(waypoint, level);
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   void WaypointGraph::RemoveWaypoint(WaypointID waypoint)
   {
      const WaypointInterface* wpPtr = FindWaypoint(waypoint);

      if(wpPtr != NULL)
      {
         RemoveWaypoint_Protected(wpPtr);
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   void WaypointGraph::RemoveWaypoint_Protected(const WaypointInterface* waypoint)
   {      
      mImpl->Remove(waypoint);
   }

   /////////////////////////////////////////////////////////////////////////////
   const WaypointInterface* WaypointGraph::FindWaypoint(WaypointID id) const
   {
      WaypointGraphImpl::WaypointMap::const_iterator iter = mImpl->mWaypointOwnership.find(id);
      if(iter != mImpl->mWaypointOwnership.end())
      {
         return (*iter).second.mWaypoint;
      }

      return NULL;
   }

   //if waypoint is a collection returns it, else returns parent of actual waypoint
   //may return NULL
   /////////////////////////////////////////////////////////////////////////////
   WaypointCollection* WaypointGraph::FindCollection( WaypointID id )
   {
      WaypointGraphImpl::WaypointMap::iterator iter = mImpl->mWaypointOwnership.find(id);
      if(iter != mImpl->mWaypointOwnership.end())
      {
         WaypointGraphImpl::WaypointHolder& wh = (*iter).second;
         /*const WaypointCollection* wc = dynamic_cast<const WaypointCollection*>(wh.mWaypoint);
         if(wc != NULL)
         {
            return const_cast<WaypointCollection*>(wc);
         }
         else*/
         {
            //the parent of a collection in the ownership map is the collection itself
            return wh.mParent;
         }
      }

      return NULL;
   }

   //if waypoint is a collection returns it, else returns parent of actual waypoint
   //may return NULL
   /////////////////////////////////////////////////////////////////////////////
   const WaypointCollection* WaypointGraph::FindCollection(WaypointID id) const
   {
      WaypointGraphImpl::WaypointMap::const_iterator iter = mImpl->mWaypointOwnership.find(id);
      if(iter != mImpl->mWaypointOwnership.end())
      {
         const WaypointGraphImpl::WaypointHolder& wh = (*iter).second;
         /*const WaypointCollection* wc = dynamic_cast<const WaypointCollection*>(wh.mWaypoint);
         if(wc != NULL)
         {
            return wc;
         }
         else*/
         {
            //the parent of a collection in the ownership map is the collection itself
            return wh.mParent;
         }
      }

      return NULL;
   }

   //returns the top most node for a specific child
   /////////////////////////////////////////////////////////////////////////////
   WaypointCollection* WaypointGraph::GetRootParent(WaypointID id)
   {
      WaypointCollection* wc = FindCollection(id);
      if(wc != NULL)
      {
         wc = mImpl->GetRoot(wc);
      }

      return wc;
   }

   //returns the top most node for a specific child
   /////////////////////////////////////////////////////////////////////////////
   const WaypointCollection* WaypointGraph::GetRootParent(WaypointID id) const
   {
      const WaypointCollection* wc = FindCollection(id);
      if(wc != NULL)
      {
         wc = mImpl->GetRoot(wc);
      }

      return wc;
   }


   /////////////////////////////////////////////////////////////////////////////
   void WaypointGraph::AddEdge(WaypointID pFrom, const WaypointID pTo)
   {
      const WaypointInterface* wpLhs = FindWaypoint(pFrom);
      const WaypointInterface* wpRhs = FindWaypoint(pTo);

      if(wpLhs != NULL && wpRhs != NULL)
      {
         AddEdge_Protected(wpLhs, wpRhs);
      }
      else
      {
         LOG_ERROR("Waypoints must be explicitly added before an edge can be added containing either of them.");
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   void WaypointGraph::AddEdge_Protected(const WaypointInterface* from, const WaypointInterface* to)
   {
      //lets make sure they are on the same level
      WaypointGraphImpl::WaypointMap::iterator iterFrom = mImpl->mWaypointOwnership.find(from->GetID());
      WaypointGraphImpl::WaypointMap::iterator iterTo = mImpl->mWaypointOwnership.find(to->GetID());
         if(iterFrom != mImpl->mWaypointOwnership.end() && iterTo != mImpl->mWaypointOwnership.end())
         {
            WaypointGraphImpl::WaypointHolder& whFrom = (*iterFrom).second;
            WaypointGraphImpl::WaypointHolder& whTo = (*iterTo).second;         
            if(whFrom.mLevel == whTo.mLevel)
            {
               //now for each search level, add the new path to all parents
               SearchLevel* sl = mImpl->GetSearchLevel(whFrom.mLevel);
               sl->mNavMesh->AddEdge(whFrom.mWaypoint, whTo.mWaypoint);                             
            }
            else
            {
               LOG_ERROR("Cannot insert edge between waypoints that are not on the same level.");
            }
         }         
   }

   /////////////////////////////////////////////////////////////////////////////
   bool WaypointGraph::RemoveEdge(WaypointID wayFrom, WaypointID wayTo)
   {
      bool result = false;
      const WaypointInterface* wpLhs = FindWaypoint(wayFrom);
      const WaypointInterface* wpRhs = FindWaypoint(wayTo);

      if(wpLhs != NULL && wpRhs != NULL)
      {
         return RemoveEdge_Protected(wpLhs, wpRhs);
      }
      return false;
   }
   /////////////////////////////////////////////////////////////////////////////
   bool WaypointGraph::RemoveEdge_Protected(const WaypointInterface* from, const WaypointInterface* to)
   {
      //lets make sure they are on the same level
      WaypointGraphImpl::WaypointMap::iterator iterFrom = mImpl->mWaypointOwnership.find(from->GetID());
      WaypointGraphImpl::WaypointMap::iterator iterTo = mImpl->mWaypointOwnership.find(to->GetID());
      if(iterFrom != mImpl->mWaypointOwnership.end() && iterTo != mImpl->mWaypointOwnership.end())
      {
         WaypointGraphImpl::WaypointHolder& whFrom = (*iterFrom).second;
         WaypointGraphImpl::WaypointHolder& whTo = (*iterTo).second;         
         if(whFrom.mLevel == whTo.mLevel)
         {
            SearchLevel* sl = mImpl->GetSearchLevel(whFrom.mLevel);
            sl->mNavMesh->RemoveEdge(whFrom.mWaypoint, whTo.mWaypoint);                             
         }
         else
         {
            LOG_ERROR("Cannot remove edge between waypoints that are not on the same level.");
         }
      }         
      return false;
   }

   /////////////////////////////////////////////////////////////////////////////
   void WaypointGraph::RemoveAllEdgesFromWaypoint(WaypointID pFrom)
   {
      const WaypointInterface* wpFrom = FindWaypoint(pFrom);

      if(wpFrom != NULL)
      {
         RemoveAllEdgesFromWaypoint_Protected(wpFrom);
      }
   }
   
   ////////////////////////////////////////////////////////////////////////////
   void WaypointGraph::RemoveAllEdgesFromWaypoint_Protected(const WaypointInterface* from)
   {
      WaypointGraphImpl::WaypointMap::iterator iterFrom = mImpl->mWaypointOwnership.find(from->GetID());
      if(iterFrom != mImpl->mWaypointOwnership.end())
      {
         WaypointGraphImpl::WaypointHolder& whFrom = (*iterFrom).second;
   
         SearchLevel* sl = mImpl->GetSearchLevel(whFrom.mLevel);
         sl->mNavMesh->RemoveAllEdges(whFrom.mWaypoint);
      }         
   }

   /////////////////////////////////////////////////////////////////////////////
   void WaypointGraph::GetAllEdgesFromWaypoint(WaypointID pFrom, ConstWaypointArray& result) const
   {
      const WaypointInterface* wpFrom = FindWaypoint(pFrom);

      if(wpFrom != NULL)
      {
         GetAllEdgesFromWaypoint_Protected(*wpFrom, result);
      }
   }
      
   /////////////////////////////////////////////////////////////////////////////
   void WaypointGraph::GetAllEdgesFromWaypoint_Protected(const WaypointInterface& pFrom, ConstWaypointArray& result) const
   {
      WaypointGraphImpl::WaypointMap::const_iterator iter = mImpl->mWaypointOwnership.find(pFrom.GetID());
      if(iter != mImpl->mWaypointOwnership.end())
      {
         const WaypointGraphImpl::WaypointHolder& wh = (*iter).second;
         WaypointGraph::SearchLevel* sl = mImpl->GetSearchLevel(wh.mLevel);
         
         if(sl != NULL)
         {
            NavMesh::NavMeshContainer::iterator nm_iter = sl->mNavMesh->begin(wh.mWaypoint);
            NavMesh::NavMeshContainer::iterator nm_iterEnd = sl->mNavMesh->end(wh.mWaypoint);
            
            for(;nm_iter != nm_iterEnd; ++nm_iter)
            {
               result.push_back( (*nm_iter).second->GetWaypointTo() );
            }
         }
      }
      
   }

  
   /////////////////////////////////////////////////////////////////////////////
   bool WaypointGraph::HasPath(WaypointID lhs, WaypointID rhs) const
   {
      bool result = false;

      const WaypointCollection* lhsC = FindCollection(lhs);
      const WaypointCollection* rhsC = FindCollection(rhs);

      if(lhsC != NULL && rhsC != NULL)
      {
         result = HasPath_Protected(lhsC, rhsC);
      }
      
      return result;
   }

   /////////////////////////////////////////////////////////////////////////////
   bool WaypointGraph::HasPath_Protected(const WaypointCollection* lhs, const WaypointCollection* rhs) const
   {      
      return mImpl->HasPath(*lhs, *rhs);
   }

   /////////////////////////////////////////////////////////////////////////////
   bool WaypointGraph::Contains(WaypointID id) const
   {
      return (mImpl->mWaypointOwnership.find(id) != mImpl->mWaypointOwnership.end());
   }

   /////////////////////////////////////////////////////////////////////////////
   const NavMesh* WaypointGraph::GetNavMeshAtSearchLevel(unsigned level) const
   {
      WaypointGraph::SearchLevel* sl = mImpl->GetSearchLevel(level);
      if(sl != NULL)
      {
         return sl->mNavMesh.get();
      }

      return NULL;
   }
   
   NavMesh* WaypointGraph::GetNavMeshAtSearchLevel(unsigned level)
   {
      WaypointGraph::SearchLevel* sl = mImpl->GetSearchLevel(level);
      if(sl != NULL)
      {
         return sl->mNavMesh.get();
      }

      return NULL;
   }

   AIDebugDrawable* WaypointGraph::GetDrawableAtSearchLevel(const WaypointCollection* root, unsigned level) const
   {
      const NavMesh* nm = GetNavMeshAtSearchLevel(level);

      if(nm != NULL)
      {
         AIDebugDrawable* dd = new AIDebugDrawable();

         dd->UpdateWaypointGraph(*nm);
         return dd;
      }

      return NULL;
   }   

   unsigned WaypointGraph::GetNumSearchLevels() const
   {
      //note this is not necessarily the max mLevel of all search levels
      //the distinction is made because this is the index used to access the search level array directly      
      return mImpl->mSearchLevels.size();
   }

   const WaypointGraph::SearchLevel* WaypointGraph::GetSearchLevel(unsigned levelNum) const
   {
      //we are guaranteed to have a search level 0
      return mImpl->GetSearchLevel(levelNum);
   }

   WaypointGraph::SearchLevel* WaypointGraph::GetSearchLevel(unsigned levelNum) 
   {
      //we are guaranteed to have a search level 0
      return mImpl->GetSearchLevel(levelNum);
   }

   WaypointGraph::SearchLevel* WaypointGraph::GetOrCreateSearchLevel(unsigned levelNum) 
   {
      //we are guaranteed to have a search level 0
      return mImpl->GetOrCreateSearchLevel(levelNum);
   }

   int WaypointGraph::GetSearchLevelNum(WaypointID id) const
   {
      WaypointGraphImpl::WaypointMap::const_iterator iter = mImpl->mWaypointOwnership.find(id);
      if(iter != mImpl->mWaypointOwnership.end())
      {
         const WaypointGraphImpl::WaypointHolder& wh = (*iter).second;
         return wh.mLevel;
      }
       
      return -1;
   }

   //void WaypointGraph::AddSearchLevel(dtCore::RefPtr<WaypointGraph::SearchLevel> newLevel)
   //{
   //   mImpl->AddSearchLevel(newLevel.get());
   //}

   //commented out because the mImpl::FindCommonParent() is not implemented
   //WaypointCollection* WaypointGraph::FindCommonParent( const WaypointInterface& lhs, const WaypointInterface& rhs)
   //{
   //   const WaypointCollection* wpColLhs = FindCollection(lhs.GetID());
   //   const WaypointCollection* wpColRhs = FindCollection(rhs.GetID());
   //   if(wpColRhs != NULL && wpColLhs != NULL)
   //   {
   //      return mImpl->FindCommonParent(*wpColLhs, *wpColRhs);
   //   } 

   //   return NULL;
   //}

   WaypointCollection* WaypointGraph::GetParent(WaypointID id)
   {
      WaypointCollection* result = NULL;

      WaypointGraphImpl::WaypointMap::iterator iter = mImpl->mWaypointOwnership.find(id);
      if(iter != mImpl->mWaypointOwnership.end())
      {
         WaypointGraphImpl::WaypointHolder& wh = (*iter).second;
         //the order of this if is important, (wh.mParent == wh.mWaypoint) means that the waypoint is a collection
         //in which case we want to return the waypoint collection's parent in the tree
         if(wh.mParent != NULL && (wh.mParent == wh.mWaypoint) && (wh.mParent->parent() != NULL) )
         {
             result = mImpl->CastToCollection(wh.mParent->parent());
         }
         else
         {
            //else this was a regular waypoint return immediate parent collection
            result = wh.mParent;
         }
      }

      return result;
   }

   const WaypointCollection* WaypointGraph::GetParent(WaypointID id) const
   {
      const WaypointCollection* result = NULL;

      WaypointGraphImpl::WaypointMap::const_iterator iter = mImpl->mWaypointOwnership.find(id);
      if(iter != mImpl->mWaypointOwnership.end())
      {
         const WaypointGraphImpl::WaypointHolder& wh = (*iter).second;
         //the order of this if is important, (wh.mParent == wh.mWaypoint) means that the waypoint is a collection
         //in which case we want to return the waypoint collection's parent in the tree
         if(wh.mParent != NULL && (wh.mParent == wh.mWaypoint) && (wh.mParent->parent() != NULL) )
         {
            result = mImpl->CastToCollection(wh.mParent->parent());
         }
         else
         {
            //else this was a regular waypoint return immediate parent collection
            result = wh.mParent;
         }
      }

      return result;
   }

   void WaypointGraph::SetParent(WaypointID id, WaypointCollection* parent)
   {
      WaypointGraphImpl::WaypointMap::iterator iter = mImpl->mWaypointOwnership.find(id);
      if(iter != mImpl->mWaypointOwnership.end())
      {
         WaypointGraphImpl::WaypointHolder& wh = (*iter).second;

         wh.mParent = parent;
      }
   }

} // namespace dtAI
