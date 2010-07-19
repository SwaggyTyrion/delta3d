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

#include <dtDirectorQt/lockitem.h>
#include <dtDirectorQt/directoreditor.h>
#include <dtDirectorQt/editorscene.h>
#include <dtDirectorQt/undomanager.h>
#include <dtDirectorQt/undolinkevent.h>

#include <dtDirector/outputlink.h>

#include <dtDAL/datatype.h>

#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QMenu>


namespace dtDirector
{
   //////////////////////////////////////////////////////////////////////////
   LockItem::LockItem(NodeItem* nodeItem, QGraphicsItem* parent, EditorScene* scene)
      : QGraphicsPolygonItem(parent, scene)
      , mScene(scene)
      , mNodeItem(nodeItem)
      , mIsLocked(true)
   {
   }

   ////////////////////////////////////////////////////////////////////////////////
   void LockItem::Init()
   {
      setFlag(QGraphicsItem::ItemIsMovable, false);
      //setFlag(QGraphicsItem::ItemIsSelectable, true);
      setFlag(QGraphicsItem::ItemIgnoresParentOpacity, true);

#if(QT_VERSION >= 0x00040600)
      setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
#endif

      setAcceptHoverEvents(true);

      const float size = 10;
      QPolygonF poly;
      poly << QPointF(1, 1)
         << QPointF(size, 1)
         << QPointF(size, size)
         << QPointF(1, size);
      setPolygon(poly);

      SetHighlight(false);
   }

   //////////////////////////////////////////////////////////////////////////
   void LockItem::SetHighlight(bool enable)
   {
      QColor color;
      if (enable)
      {
         color = Qt::yellow;
      }
      else
      {
         color = QColor(0, 0, 0, 255);
      }

      setPen(QPen(color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

      if (mIsLocked)
      {
         setBrush(QBrush(QColor(255, 0, 0, 255)));
      }
      else
      {
         setBrush(QBrush(QColor(255, 255, 255, 255)));
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   void LockItem::SetLocked(bool locked)
   {
      mIsLocked = locked;
   }

   //////////////////////////////////////////////////////////////////////////
   void LockItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
   {
      SetHighlight(true);
   }

   //////////////////////////////////////////////////////////////////////////
   void LockItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
   {
      SetHighlight(false);
   }

   //////////////////////////////////////////////////////////////////////////
   void LockItem::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
   {
      QGraphicsPolygonItem::mousePressEvent(mouseEvent);

      if (!mouseEvent) return;

      SetLocked(!IsLocked());
      SetHighlight(true);
   }

   //////////////////////////////////////////////////////////////////////////
   void LockItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
   {
      QGraphicsPolygonItem::mouseReleaseEvent(mouseEvent);

      if (!mouseEvent) return;
   }

   //////////////////////////////////////////////////////////////////////////
   void LockItem::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
   {
      QGraphicsPolygonItem::mouseMoveEvent(mouseEvent);

      if (!mouseEvent) return;
   }

   //////////////////////////////////////////////////////////////////////////
   QVariant LockItem::itemChange(GraphicsItemChange change, const QVariant &value)
   {
      if (mNodeItem)
      {
         mNodeItem->childItemChange(this, change, value);
      }

      return value;
   }
}

//////////////////////////////////////////////////////////////////////////
