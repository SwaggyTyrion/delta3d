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
 * Teague Coonan
 */

#include <prefix/dtstageprefix-src.h>
#include <QtGui/QHeaderView>

#include <QtGui/QHBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QSplitter>
#include <QtCore/QString>
#include <QtCore/QDir>

#include <QtGui/QGroupBox>

#include <QtGui/QPushButton>
#include <QtGui/QCheckBox>

#include <QtGui/QAction>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QIcon>

#include <QtGui/QPixmap>
#include <QtGui/QScrollArea>
#include <QtGui/QLabel>
#include <QtGui/QIcon>

#include <dtEditQt/resourcetreewidget.h>
#include <dtEditQt/resourceimportdialog.h>
#include <dtEditQt/editordata.h>
#include <dtEditQt/editorevents.h>
#include <dtEditQt/texturebrowser.h>
#include <dtEditQt/uiresources.h>

#include <dtDAL/project.h>

namespace dtEditQt
{
   ///////////////////////////////////////////////////////////////////////////////
   TextureBrowser::TextureBrowser(dtDAL::DataType& type, QWidget* parent)
      : ResourceAbstractBrowser(&type, parent)
   {
      // This sets our resource icon that is visible on leaf nodes
      QIcon resourceIcon;
      resourceIcon.addPixmap(QPixmap(UIResources::ICON_TEXTURE_RESOURCE.c_str()));
      ResourceAbstractBrowser::resourceIcon = resourceIcon;

      QSplitter* splitter = new QSplitter(Qt::Vertical, this);

      splitter->addWidget(previewTextureGroup());
      splitter->addWidget(listTextureGroup());

      // setup the grid layouts
      grid = new QGridLayout(this);
      grid->addWidget(splitter, 0, 0);
      grid->addWidget(standardButtons(QString("Resource Tools")), 1, 0, Qt::AlignCenter);

      // stop the buttons from stretching
      grid->setRowStretch(1, 0);
   }
   ///////////////////////////////////////////////////////////////////////////////
   TextureBrowser::~TextureBrowser() {}
   ///////////////////////////////////////////////////////////////////////////////
   QGroupBox* TextureBrowser::listTextureGroup()
   {
      QGroupBox* groupBox = new QGroupBox(tr("Textures"));
      QGridLayout* grid = new QGridLayout(groupBox);
      QHBoxLayout* hbox = new QHBoxLayout();

      // Checkbox for auto preview
      previewChk = new QCheckBox(tr("Auto Preview"), groupBox);
      connect(previewChk, SIGNAL(stateChanged(int)), this, SLOT(checkBoxSelected()));
      previewChk->setChecked(false);

      // Preview button for a selected mesh
      previewBtn = new QPushButton("Preview", groupBox);
      connect(previewBtn, SIGNAL(clicked()), this, SLOT(previewTexture()));
      previewBtn->setDisabled(true);

      hbox->addWidget(previewChk, 0, Qt::AlignLeft);
      hbox->addWidget(previewBtn, 0, Qt::AlignRight);
      grid->addLayout(hbox, 0, 0);
      grid->addWidget(tree, 1, 0);

      return groupBox;
   }

   ///////////////////////////////////////////////////////////////////////////////
   QGroupBox* TextureBrowser::previewTextureGroup()
   {
      QGroupBox*   group = new QGroupBox(tr("Preview"));
      QHBoxLayout* hbox  = new QHBoxLayout(group);

      scrollArea = new QScrollArea(group);

      pixmapWrapper = new QWidget(scrollArea);

      preview = new QLabel(pixmapWrapper);
      preview->setPixmap(QPixmap());
      preview->setShown(true);

      scrollArea->setWidget(pixmapWrapper);

      hbox->addWidget(scrollArea);

      return group;
   }
   ///////////////////////////////////////////////////////////////////////////////
   // Keyboard Event filter
   ///////////////////////////////////////////////////////////////////////////////
   bool TextureBrowser::eventFilter(QObject* obj, QEvent* e)
   {
      if (obj == tree)
      {
         // For some reason, KeyPress is getting defined by something...
         // Without this undef, it will not compile under Linux..
         // It would be great if someone could figure out exactly what's
         // going on.
#undef KeyPress
         if (e->type() == QEvent::KeyPress)
         {
            QKeyEvent* keyEvent = (QKeyEvent*)e;
            switch (keyEvent->key())
            {
            case Qt::Key_Return:
               if (selection->isResource())
               {
                  previewTexture();
               }
               break;
            case Qt::Key_Enter:
               if (selection->isResource())
               {
                  previewTexture();
               }
               break;
            default:
               return tree->eventFilter(obj,e);
            }
         }
         else
         {
            // pass the event on to the parent class
            return tree->eventFilter(obj, e);
         }
      }
      return false;
   }

   ///////////////////////////////////////////////////////////////////////////////
   // Slots
   ///////////////////////////////////////////////////////////////////////////////
   void TextureBrowser::previewTexture()
   {
      ResourceTreeWidget* selection = currentSelection();
      bool validFile = false;

      if (selection != NULL)
      {
         QString file;
         QString context;

         dtDAL::Project& project = dtDAL::Project::GetInstance();

         // Find the currently selected tree item
         dtDAL::ResourceDescriptor resource = EditorData::GetInstance().getCurrentTextureResource();

         try
         {
            file = QString(project.GetResourcePath(resource).c_str());
            validFile = true;
         }
         catch (dtUtil::Exception&)
         {
            validFile = false;
         }

         if (file != NULL)
         {
            context = QString(project.GetContext().c_str());
            // The following is performed to comply with linux and windows file systems
            file = context + "\\" + file;
            file.replace("\\","/");
            file.replace("//","/");

            scrollArea->setShown(true);
            //Load the new file.
            delete preview;
            QPixmap image;
            image.load(file);
            preview = new QLabel(pixmapWrapper);
            preview->setPixmap(image);
            preview->setShown(true);

            pixmapWrapper->setMinimumSize(preview->sizeHint());
            pixmapWrapper->setMaximumSize(preview->sizeHint());
         }
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void TextureBrowser::selectionChanged()
   {
      // This is the abstract base classes original functionality
      ResourceAbstractBrowser::selectionChanged();

      if (selection != NULL)
      {
         if (selection->isResource())
         {
            // auto preview
            if (previewChk->isChecked())
            {
               previewTexture();
            }
            //context sensitive menu items
            previewBtn->setDisabled(false);
         }
         else
         {
            previewBtn->setDisabled(true);
            clearTextureWidget();
         }
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void TextureBrowser::checkBoxSelected()
   {
      if (previewChk->isChecked())
      {
         if (selection->isResource())
         {
            // preview current item
            selectionChanged();
            previewTexture();
         }
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void TextureBrowser::doubleClickEvent()
   {
      if (selection->isResource())
      {
         previewTexture();
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void TextureBrowser::deleteItemEvent()
   {
      if (selection->isResource())
      {
         clearTextureWidget();
         previewBtn->setDisabled(true);
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void TextureBrowser::clearTextureWidget()
   {
      // When any item is selected, clear the texture
      delete preview;
      preview = new QLabel(pixmapWrapper);
      preview->setPixmap(QPixmap());
   }

} // namespace dtEditQt
