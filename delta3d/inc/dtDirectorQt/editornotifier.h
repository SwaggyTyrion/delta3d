/*
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2008 MOVES Institute
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
 * Author: Jeff P. Houde
 */

#ifndef EDITOR_NOTIFIER
#define EDITOR_NOTIFIER

#include <dtDirectorQt/export.h>
#include <dtDirectorQt/directoreditor.h>

#include <dtDirector/directornotifier.h>

#include <dtCore/timer.h>
#include <dtCore/uniqueid.h>

#include <map>


namespace dtDirector
{
   /**
    * This is the base class for the director notifier.
    *
    * @note
    *       Each of the functions provided by this class should
    *       be overloaded if you wish to receive the notifications
    *       provided by a script.
    */
   class DT_DIRECTOR_QT_EXPORT EditorNotifier : public DirectorNotifier
   {
   public:

      struct GlowData
      {
         dtCore::UniqueId  nodeID;
         float             glow;

         std::vector<float> outputGlows;
      };

      /**
       * Constructs the Notifier.
       */
      EditorNotifier(DirectorEditor* editor);

      /**
       * Update.
       */
      virtual void Update();

      /**
       * Event handler when a node has been executed.
       *
       * @param[in]  node     The node that was executed.
       * @param[in]  input    The input that was triggered.
       * @param[in]  outputs  A list of all outputs that were triggered.
       */
      virtual void OnNodeExecution(Node* node, const std::string& input, const std::vector<std::string>& outputs);

      /**
       * Event handler when a value has been changed on a value node.
       * 
       * @param[in]  node  The node that was changed.
       */
      virtual void OnValueChanged(Node* node);

      GlowData* GetGlowData(Node* node);

   protected:

      /**
       *	Protected Destructor.  dtCore::RefPtr will handle its destruction.
       */
      virtual ~EditorNotifier();

      DirectorEditor* mEditor;
      dtCore::Timer_t mTime;

      std::map<Node*, GlowData> mGlowMap;
   };
}

#endif // EDITOR_NOTIFIER
