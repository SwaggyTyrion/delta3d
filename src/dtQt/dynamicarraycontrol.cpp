/* -*-c++-*-
 * Delta3D Simulation Training And Game Editor (STAGE)
 * STAGE - This source file (.h & .cpp) - Using 'The MIT License'
 * Copyright (C) 2005-2008, Alion Science and Technology Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * This software was developed by Alion Science and Technology Corporation under
 * circumstances in which the U. S. Government may have rights in the software.
 *
 * Jeffrey Houde
 */

#include <prefix/dtqtprefix.h>
#include <QtGui/QWidget>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QDoubleValidator>
#include <dtQt/dynamicarraycontrol.h>
#include <dtQt/propertyeditormodel.h>
#include <dtQt/propertyeditortreeview.h>

#include <dtCore/actorproxy.h>
#include <dtCore/actorproperty.h>
#include <dtCore/datatype.h>
#include <dtCore/arrayactorpropertybase.h>
#include <dtUtil/log.h>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>

namespace dtQt
{

   /////////////////////////////////////////////////////////////////////////////
   DynamicArrayControl::DynamicArrayControl()
      : mTextLabel(NULL)
      , mAddButton(NULL)
      , mClearButton(NULL)
   {
   }

   /////////////////////////////////////////////////////////////////////////////
   DynamicArrayControl::~DynamicArrayControl()
   {
   }

   /////////////////////////////////////////////////////////////////////////////
   void DynamicArrayControl::updateEditorFromModel(QWidget* widget)
   {
      DynamicAbstractParentControl::updateEditorFromModel(widget);
   }

   /////////////////////////////////////////////////////////////////////////////
   void DynamicArrayControl::InitializeData(DynamicAbstractControl* newParent,
      PropertyEditorModel* newModel, dtCore::PropertyContainer* newPC, dtCore::ActorProperty* newProperty)
   {
      // Note - We used to have dynamic_cast in here, but it was failing to properly cast in
      // all cases in Linux with gcc4.  So we replaced it with a static cast.
      if (newProperty != NULL && newProperty->GetDataType() == dtCore::DataType::ARRAY)
      {
         mProperty = static_cast<dtCore::ArrayActorPropertyBase*>(newProperty);
         DynamicAbstractControl::InitializeData(newParent, newModel, newPC, newProperty);

         // Create each element.
         resizeChildren(false, false, true);
      }
      else
      {
         std::string propertyName = (newProperty != NULL) ? newProperty->GetName() : "NULL";
         LOG_ERROR("Cannot create dynamic control because property [" +
            propertyName + "] is not the correct type.");
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   QWidget* DynamicArrayControl::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index)
   {
      QWidget* wrapper = DynamicAbstractParentControl::createEditor(parent, option, index);

      if (!mInitialized)
      {
         LOG_ERROR("Tried to add itself to the parent widget before being initialized");
         return wrapper;
      }

      // label
      mTextLabel = new SubQLabel(getValueAsString(), wrapper, this);
      SetBackgroundColor(mTextLabel, PropertyEditorTreeView::ROW_COLOR_ODD);

      mAddButton   = new SubQToolButton(wrapper, this);
      mAddButton->setIcon(QIcon(":dtQt/icons/add.png"));
      mAddButton->setToolTip(tr("Add a new element"));
      mAddButton->setText("Add");

      mClearButton = new SubQToolButton(wrapper, this);
      mClearButton->setIcon(QIcon(":dtQt/icons/trashcan.png"));
      mClearButton->setText("Clear");
      mClearButton->setToolTip(tr("Remove all elements"));

      UpdateButtonStates();

      connect(mAddButton,   SIGNAL(clicked()), this, SLOT(onAddClicked()));
      connect(mClearButton, SIGNAL(clicked()), this, SLOT(onClearClicked()));

      mTextLabel->setToolTip(getDescription());

      mGridLayout->addWidget(mTextLabel,   0, 0, 1, 1);
      mGridLayout->addWidget(mAddButton,   0, 1, 1, 1);
      mGridLayout->addWidget(mClearButton, 0, 2, 1, 1);
      mGridLayout->setColumnMinimumWidth(1, mAddButton->sizeHint().width() / 2);
      mGridLayout->setColumnMinimumWidth(2, mClearButton->sizeHint().width() / 2);
      mGridLayout->setColumnStretch(0, 2);
      mGridLayout->setColumnStretch(1, 0);
      mGridLayout->setColumnStretch(2, 0);

      wrapper->setFocusProxy(mAddButton);
      return wrapper;
   }

   /////////////////////////////////////////////////////////////////////////////
   void DynamicArrayControl::handleSubEditDestroy(QWidget* widget, QAbstractItemDelegate::EndEditHint hint)
   {
      if (widget == mWrapper)
      {
         mTextLabel   = NULL;
         mAddButton   = NULL;
         mClearButton = NULL;
      }

      DynamicAbstractParentControl::handleSubEditDestroy(widget, hint);
   }

   /////////////////////////////////////////////////////////////////////////////
   const QString DynamicArrayControl::getDisplayName()
   {
      QString name = DynamicAbstractControl::getDisplayName();
      if (!name.isEmpty())
      {
         return name;
      }

      if (mProperty.valid())
      {
         return QString(tr(mProperty->GetLabel().c_str()));
      }
      else
      {
         LOG_ERROR("Dynamic array control has an invalid property type");
         return tr("");
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   const QString DynamicArrayControl::getDescription()
   {
      if(mProperty.valid())
      {
         return QString(tr(mProperty->GetDescription().c_str())) + QString("  [Type: ") +
            QString(tr(mProperty->GetDataType().GetName().c_str())) + QString("]");
      }
      else
      {
         LOG_ERROR("Dynamic array control has an invalid property type");
         return tr("");
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   const QString DynamicArrayControl::getValueAsString()
   {
      DynamicAbstractControl::getValueAsString();

      if (mProperty.valid())
      {
         int arrayCount = mProperty->GetArraySize();
         int minCount   = mProperty->GetMinArraySize();
         int maxCount   = mProperty->GetMaxArraySize();

         QString minText("0");
         if (minCount >  0) { minText = QString::number(minCount); }

         QString maxText("*");
         if (maxCount > -1) { maxText = QString::number(maxCount); }

         return QString::number(arrayCount) + " Elements (" + minText + "-" + maxText + ")";
      }
      else
      {
         LOG_ERROR("Dynamic array control has an invalid property type");
         return tr("");
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   bool DynamicArrayControl::isEditable()
   {
      return !mProperty->IsReadOnly();
   }

   /////////////////////////////////////////////////////////////////////////////
   void DynamicArrayControl::resizeChildren(bool forceRefresh, bool isChild, bool initializing)
   {
      int childCount = getChildCount();
      int size = mProperty->GetArraySize();
      bool dataChanged = false;

      PropertyEditorModel* model = GetModel();
      if (model != NULL)
      {
         std::string oldValue = mProperty->ToString();

         // If we are removing, clear all the children and then add the proper amount back.
         // This is done so all items will be refreshed.
         if (size < childCount || forceRefresh)
         {
            model->removeRows(0, childCount, model->IndexOf(this));

            removeAllChildren(model);

            childCount = 0;
         }

         // Add new children...
         if (size > childCount)
         {
            int addCount = size - childCount;

            for (int childIndex = 0; childIndex < addCount; childIndex++)
            {
               mProperty->SetIndex(childCount + childIndex);

               dtCore::ActorProperty* propType = mProperty->GetArrayProperty();
               if (propType)
               {
                  DynamicAbstractControl* element = GetDynamicControlFactory()->CreateDynamicControl(*propType);
                  element->SetTreeView(mPropertyTree);
                  element->SetDynamicControlFactory(GetDynamicControlFactory());
                  element->SetArrayIndex(childCount + childIndex);
                  element->InitializeData(this, GetModel(), mPropContainer.get(), propType);
                  connect(element, SIGNAL(PropertyAboutToChange(dtCore::PropertyContainer&, dtCore::ActorProperty&,
                     const std::string&, const std::string&)),
                     this, SLOT(PropertyAboutToChangePassThrough(dtCore::PropertyContainer&, dtCore::ActorProperty&,
                     const std::string&, const std::string&)));

                  connect(element, SIGNAL(PropertyChanged(dtCore::PropertyContainer&, dtCore::ActorProperty&)),
                     this, SLOT(PropertyChangedPassThrough(dtCore::PropertyContainer&, dtCore::ActorProperty&)));
                  mChildren.push_back(element);
               }
            }

            model->insertRows(childCount, addCount, model->IndexOf(this));

            if (!isChild)
            {
               UpdateButtonStates();
            }
         }

         if (!initializing)
         {
            PropertyAboutToChange(*mPropContainer, *mProperty,
               oldValue, mProperty->ToString());

            dataChanged = true;
         }
      }

      // update our label
      if (mTextLabel !=  NULL)
      {
         mTextLabel->setText(getValueAsString());
      }

      if (dataChanged)
      {
         PropertyChanged(*mPropContainer, *mProperty);
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   void DynamicArrayControl::SetIndexFocus(int index)
   {
      if (index >= 0 && index < (int)mChildren.size())
      {
         mPropertyTree->setCurrentIndex(mModel->IndexOf(mChildren[index]));
      }
      else
      {
         mPropertyTree->setCurrentIndex(mModel->IndexOf(this));
      }
   }

   /////////////////////////////////////////////////////////////////////////////////
   // SLOTS
   /////////////////////////////////////////////////////////////////////////////////

   /////////////////////////////////////////////////////////////////////////////
   bool DynamicArrayControl::updateData(QWidget* widget)
   {
      // this guy doesn't have any editors.  All the data is edited in child controls
      return false;
   }

   /////////////////////////////////////////////////////////////////////////////
   void DynamicArrayControl::onAddClicked()
   {
      NotifyParentOfPreUpdate();
      mProperty->Insert(mProperty->GetArraySize());
      resizeChildren();
   }

   /////////////////////////////////////////////////////////////////////////////
   void DynamicArrayControl::onClearClicked()
   {
      NotifyParentOfPreUpdate();
      mProperty->Clear();
      resizeChildren();
   }

   /////////////////////////////////////////////////////////////////////////////
   void DynamicArrayControl::onResetClicked()
   {
      DynamicAbstractParentControl::onResetClicked();
      resizeChildren();
   }

   /////////////////////////////////////////////////////////////////////////////////
   // PRIVATE
   /////////////////////////////////////////////////////////////////////////////////

   /////////////////////////////////////////////////////////////////////////////
   void DynamicArrayControl::UpdateButtonStates()
   {
      // Don't do anything if we have no buttons.
      if (!mAddButton || !mClearButton)
      {
         return;
      }

      // Check if we are at our max array size.
      int curSize = mProperty->GetArraySize();
      int maxSize = mProperty->GetMaxArraySize();
      if (maxSize > -1 && curSize >= maxSize)
      {
         mAddButton->setDisabled(true);
      }
      else
      {
         mAddButton->setDisabled(false);
      }

      // Check if we are at our min array size.
      int minSize = mProperty->GetMinArraySize();
      if (minSize > -1 && curSize <= minSize)
      {
         mClearButton->setDisabled(true);
      }
      else
      {
         mClearButton->setDisabled(false);
      }
   }

} // namespace dtEditQt