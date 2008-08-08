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
* William E. Johnson II
*/
#include <prefix/dtstageprefix-src.h>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QPushButton>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <QtGui/QListWidgetItem>
#include <QtGui/QListWidget>
#include <QtCore/QStringList>
#include <QtGui/QMainWindow>
#include <QtGui/QGroupBox>

#include <dtEditQt/librarypathseditor.h>
#include <dtEditQt/editordata.h>
#include <dtEditQt/editorevents.h>
#include <dtEditQt/editoractions.h>
#include <dtDAL/librarymanager.h>
#include <dtDAL/actorpluginregistry.h>
#include <dtDAL/map.h>
#include <dtUtil/log.h>

#include <osgDB/FileNameUtils>
#include <osgDB/Registry>

#include <cassert>

using dtDAL::ActorProxy;
using dtDAL::ActorType;
using dtDAL::Map;
using dtDAL::ActorPluginRegistry;
using dtDAL::LibraryManager;
/// @cond DOXYGEN_SHOULD_SKIP_THIS
using std::vector;
using std::string;
/// @endcond

enum { ERROR_LIB_NOT_LOADED = 0, ERROR_ACTORS_IN_LIB, ERROR_INVALID_LIB, ERROR_UNKNOWN };

namespace dtEditQt
{
   LibraryPathsEditor::LibraryPathsEditor(QWidget *parent) : QDialog(parent), numActorsInScene(0)
   {
      bool okay = true;
      setWindowTitle(tr("Library Editor"));
      
      QGroupBox *groupBox = new QGroupBox(tr("Library Search Path Order"),this);
      QGridLayout *gridLayout = new QGridLayout(groupBox);
      
      // add the lib names to the grid
      pathView = new QListWidget(groupBox);
      pathView->setSelectionMode(QAbstractItemView::SingleSelection);
      gridLayout->addWidget(pathView,0,0);
            
      // Create the arrow buttons for changing the library order.
      QVBoxLayout *arrowLayout = new QVBoxLayout;
      upPath = new QPushButton(tr("^"),groupBox);
      downPath = new QPushButton(tr("v"),groupBox);
      arrowLayout->addStretch(1);
      arrowLayout->addWidget(upPath);
      arrowLayout->addWidget(downPath);
      arrowLayout->addStretch(1);
      gridLayout->addLayout(arrowLayout,0,1);
      
      // create the buttons, default delete to disabled
      QHBoxLayout *buttonLayout = new QHBoxLayout;
      QPushButton *addPath = new QPushButton(tr("Add Path"),this);
      QPushButton *close = new QPushButton(tr("Close"),this);
      deletePath = new QPushButton(tr("Remove Path"),this);
      
      buttonLayout->addStretch(1);
      buttonLayout->addWidget(addPath);
      buttonLayout->addWidget(deletePath);
      buttonLayout->addWidget(close);
      buttonLayout->addStretch(1);

      // Hide functionality that does not yet exist
      upPath->hide();
      downPath->hide();

      // make the connections
      okay = okay && connect(deletePath, SIGNAL(clicked()),              this, SLOT(spawnDeleteConfirmation()));
      okay = okay && connect(addPath,    SIGNAL(clicked()),              this, SLOT(spawnFileBrowser()));
      okay = okay && connect(upPath,     SIGNAL(clicked()),              this, SLOT(shiftPathUp()));
      okay = okay && connect(downPath,   SIGNAL(clicked()),              this, SLOT(shiftPathDown()));
      okay = okay && connect(close,      SIGNAL(clicked()),              this, SLOT(close()));
      okay = okay && connect(pathView,   SIGNAL(itemSelectionChanged()), this, SLOT(refreshButtons()));

      // make sure all connections were successfully made
      assert(okay);

      QVBoxLayout *mainLayout = new QVBoxLayout(this);
      mainLayout->addWidget(groupBox);
      mainLayout->addLayout(buttonLayout);

      refreshPaths();
      refreshButtons();
   }

   LibraryPathsEditor::~LibraryPathsEditor()
   {
   }

   bool LibraryPathsEditor::AnyItemsSelected() const
   {
      //return pathView->currentItem() != NULL;  // note: this test returns false positives
      return !pathView->selectedItems().empty(); // this one is better
   }

   void LibraryPathsEditor::getPathNames(vector<QListWidgetItem*>& items) const
   {
      items.clear();
      
      std::vector<std::string> pathList;
      dtUtil::LibrarySharingManager::GetInstance().GetSearchPath(pathList);
      if(pathList.empty())
         return;

      for(std::vector<std::string>::const_iterator iter = pathList.begin(); 
          iter != pathList.end(); 
          ++iter)
      {
         items.push_back(new QListWidgetItem(tr((*iter).c_str())));
      }
   }

   ///////////////////////// Slots /////////////////////////
   void LibraryPathsEditor::spawnFileBrowser()
   {
      QString file;
      string dir = EditorData::GetInstance().getCurrentLibraryDirectory();
      QString hack = dir.c_str();
      hack.replace('\\', '/');
      
      file = QFileDialog::getExistingDirectory(this, tr("Select a directory to add to the library path"));
      
      // did they hit cancel?
      if(file.isEmpty())
         return;

      dtUtil::LibrarySharingManager::GetInstance().AddToSearchPath(file.toStdString());

      refreshPaths();
   }

   void LibraryPathsEditor::spawnDeleteConfirmation()
   {
      if(QMessageBox::question(this, tr("Confirm deletion"),
                              tr("Are you sure you want to remove this path?"),
                              tr("&Yes"), tr("&No"), QString::null, 1) == 0)
      {
         std::string pathToRemove = pathView->currentItem()->text().toStdString();
         
         dtUtil::LibrarySharingManager::GetInstance().RemoveFromSearchPath(pathToRemove);
          
         refreshPaths();
      }
   }

   void LibraryPathsEditor::shiftPathUp()
   {
      QListWidgetItem *item = pathView->item(pathView->count() - 1);
      if(item != NULL)
      {
         pathView->setCurrentItem(item);
         if(item == pathView->item(0))
         {
            upPath->setDisabled(true);
         }

         if(item == pathView->item(pathView->count() - 1))
         {
            downPath->setDisabled(true);
         }
      }

      dtUtil::LibrarySharingManager::GetInstance().ClearSearchPath();

      for(int i = 0; i < pathView->count(); i++)
      {
         dtUtil::LibrarySharingManager::GetInstance().AddToSearchPath(pathView->item(i)->text().toStdString());
      }

      refreshPaths();
   }

   void LibraryPathsEditor::shiftPathDown()
   {
      // ensure the current item is selected
      QListWidgetItem *item = pathView->item(pathView->count() + 1);
      if(item != NULL)
      {
         pathView->setCurrentItem(item);
         if(item == pathView->item(0))
         {
            upPath->setDisabled(true);
         }
           
         if(item == pathView->item(pathView->count() - 1))
         {
            downPath->setDisabled(true);
         }
      }

      dtUtil::LibrarySharingManager::GetInstance().ClearSearchPath();

      for(int i = 0; i < pathView->count(); i++)
      {
         dtUtil::LibrarySharingManager::GetInstance().AddToSearchPath(pathView->item(i)->text().toStdString());
      }

      refreshPaths();
   }

   void LibraryPathsEditor::refreshButtons()
   {
      bool pathIsSelected = AnyItemsSelected();

      deletePath->setEnabled(pathIsSelected);
      upPath->setEnabled(pathIsSelected);
      downPath->setEnabled(pathIsSelected);
   }

   void LibraryPathsEditor::refreshPaths()
   {
      pathView->clear();

      std::vector<QListWidgetItem*> paths;
      getPathNames(paths);
       
      for(size_t i = 0; i < paths.size(); i++)
      {   
         pathView->addItem(paths[i]);
      }

      if(AnyItemsSelected())
         pathView->setItemSelected(pathView->currentItem(), true);
   }
}


