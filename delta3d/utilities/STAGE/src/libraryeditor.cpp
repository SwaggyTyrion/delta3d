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

#include <dtEditQt/libraryeditor.h>
#include <dtEditQt/editordata.h>
#include <dtEditQt/editorevents.h>
#include <dtEditQt/editoractions.h>
#include <dtDAL/librarymanager.h>
#include <dtDAL/actorpluginregistry.h>
#include <dtDAL/map.h>
#include <dtUtil/log.h>

#include <osgDB/FileNameUtils>

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
   LibraryEditor::LibraryEditor(QWidget *parent) : QDialog(parent), numActorsInScene(0)
   {
      setWindowTitle(tr("Library Editor"));
      
      QGroupBox *groupBox = new QGroupBox(tr("Loaded Libraries"),this);
      QGridLayout *gridLayout = new QGridLayout(groupBox);
      
      // add the lib names to the grid
      libView = new QListWidget(groupBox);
      libView->setSelectionMode(QAbstractItemView::SingleSelection);
      gridLayout->addWidget(libView,0,0);
            
      //Create the arrow buttons for changing the library order.
      QVBoxLayout *arrowLayout = new QVBoxLayout;
      upLib = new QPushButton(tr("^"),groupBox);
      downLib = new QPushButton(tr("v"),groupBox);
      arrowLayout->addStretch(1);
      arrowLayout->addWidget(upLib);
      arrowLayout->addWidget(downLib);
      arrowLayout->addStretch(1);
      gridLayout->addLayout(arrowLayout,0,1);
      
      // create the buttons, default delete to disabled
      QHBoxLayout *buttonLayout = new QHBoxLayout;
      QPushButton *importLib = new QPushButton(tr("Import Library"),this);
      QPushButton *close = new QPushButton(tr("Close"),this);
      deleteLib = new QPushButton(tr("Remove Library"),this);
      
      deleteLib->setDisabled(true);
      buttonLayout->addStretch(1);
      buttonLayout->addWidget(deleteLib);
      buttonLayout->addWidget(importLib);
      buttonLayout->addWidget(close);
      buttonLayout->addStretch(1);
      
      // make the connections
      connect(deleteLib, SIGNAL(clicked()),   this, SLOT(spawnDeleteConfirmation()));
      connect(importLib, SIGNAL(clicked()),   this, SLOT(spawnFileBrowser()));
      connect(upLib,     SIGNAL(clicked()),   this, SLOT(shiftLibraryUp()));
      connect(downLib,   SIGNAL(clicked()),   this, SLOT(shiftLibraryDown()));
      connect(close,     SIGNAL(clicked()),   this, SLOT(close()));
      connect(this, SIGNAL(noLibsSelected()), this, SLOT(disableButtons()));
      connect(this, SIGNAL(librarySelected()),this, SLOT(enableButtons()));
      
      QVBoxLayout *mainLayout = new QVBoxLayout(this);
      mainLayout->addWidget(groupBox);
      mainLayout->addLayout(buttonLayout);
      
      refreshLibraries();
   }

   LibraryEditor::~LibraryEditor()
   {
      
   }

   void LibraryEditor::getMapLibNames(vector<QListWidgetItem*>& items) const
   {
      items.clear();
      Map *currentMap = EditorData::GetInstance().getCurrentMap();
      if (currentMap == NULL)
         return;
      
      const vector<string>& libNames = currentMap->GetAllLibraries();
      
      for(unsigned int i = 0; i < libNames.size(); i++)
      {
         QListWidgetItem *p = new QListWidgetItem;
         ActorPluginRegistry *reg =
            LibraryManager::GetInstance().GetRegistry(libNames[i]);
         QString toolTip = tr("File: ") +
            tr(LibraryManager::GetInstance().GetPlatformSpecificLibraryName(libNames[i]).c_str()) +
            tr(" \nDescription: ") + tr(reg->GetDescription().c_str());
         p->setText(tr(libNames[i].c_str()));
         p->setToolTip(toolTip);
         items.push_back(p);
      }
   }

   void LibraryEditor::refreshLibraries()
   {
      libView->clear();
   
      vector<QListWidgetItem*> libs;
      getMapLibNames(libs);
      for(unsigned int i = 0; i < libs.size(); i++)
         libView->addItem(libs[i]);
      
      connect(libView, SIGNAL(itemSelectionChanged()), this, SLOT(enableButtons()));
      if(libView->currentItem() == NULL)
         emit noLibsSelected();
      else
         libView->setItemSelected(libView->currentItem(), true);
   }

   ///////////////////////// Slots /////////////////////////
   void LibraryEditor::spawnFileBrowser()
   {
      QString file;
      string dir = EditorData::GetInstance().getCurrentLibraryDirectory();
      QString hack = dir.c_str();
      hack.replace('\\', '/');
      
      string libs= "Libraries(" + LibraryManager::GetInstance().GetPlatformSpecificLibraryName("*") + ")";
      
      file = QFileDialog::getOpenFileName(this, tr("Select a library"), "", tr(libs.c_str()));
      
      // did they hit cancel?
      if(file.isEmpty())
         return;
      
      std::string libName =
         LibraryManager::GetInstance().GetPlatformIndependentLibraryName(file.toStdString());
      
      if(libName == "dtActors" || libName == "dtActorsd")
      {
         QMessageBox::information(this, tr("Library already loaded"),
                                  tr("This is the base Delta3D actors library and is loaded by default"),
                                  tr("&OK"));
         return;
      }
      
      // If the map already contains this library, no point in continuing
      vector<string> curLibs = EditorData::GetInstance().getCurrentMap()->GetAllLibraries();
      for(unsigned int i = 0; i < curLibs.size(); i++)
         if(curLibs[i] == libName)
            return;
      try
      {
         LibraryManager::GetInstance().LoadActorRegistry(libName);
      }
      catch(const dtUtil::Exception &e)
      {
         LOG_ERROR(e.What());
         handleFailure(ERROR_INVALID_LIB, e.What());
         return;
      }
      EditorData::GetInstance().getCurrentMap()->AddLibrary(libName,"");
      
      refreshLibraries();
      EditorEvents::GetInstance().emitMapLibraryImported();
      libView->setCurrentItem(libView->item(libView->count() - 1));
      EditorData::GetInstance().setCurrentLibraryDirectory(osgDB::getFilePath(file.toStdString()));
      ((QMainWindow*)EditorData::GetInstance().getMainWindow())->setWindowTitle(
                                                                                EditorActions::GetInstance().getWindowName(/*true*/).c_str());
   }

void LibraryEditor::spawnDeleteConfirmation()
{
   if(QMessageBox::question(this, tr("Confirm deletion"),
                            tr("Are you sure you want to remove this library?"),
                            tr("&Yes"), tr("&No"), QString::null, 1) == 0)
   {
      Map *curMap = EditorData::GetInstance().getCurrentMap();
      vector<dtCore::RefPtr<ActorProxy> > proxies;
      curMap->GetAllProxies(proxies);
      vector<string> loadedLibs = curMap->GetAllLibraries();
      
      std::string libToRemove = libView->currentItem()->text().toStdString();
      // Does the map have this library?
      for(unsigned int i = 0; i < loadedLibs.size(); i++)
      {         
         if(loadedLibs[i] == libToRemove)
         {
            ActorPluginRegistry *reg =
            LibraryManager::GetInstance().GetRegistry(loadedLibs[i]);
            
            // fail if actors are in the library
            for(unsigned int j = 0; j < proxies.size(); j++)
            {
               std::cout << "Proxy: " << proxies[j]->GetName() << " RefCount: " <<
               proxies[j]->referenceCount() << std::endl;
               
               dtCore::RefPtr<const ActorType> type = &proxies[j]->GetActorType();
               if(reg->IsActorTypeSupported(type))
                  ++numActorsInScene;
            }
            
            if(numActorsInScene > 0)
            {
               handleFailure(ERROR_ACTORS_IN_LIB);
               numActorsInScene = 0;
               return;
            }
            
            EditorEvents::GetInstance().emitLibraryAboutToBeRemoved();
            curMap->RemoveLibrary(libToRemove);
            LibraryManager::GetInstance().UnloadActorRegistry(libToRemove);
            refreshLibraries();
            EditorEvents::GetInstance().emitMapLibraryRemoved();
            if(curMap->GetAllLibraries().size() > 0)
               libView->setCurrentItem(libView->item(libView->count() - 1));
            // we're done
            break;
         }
      }
   }
}

void LibraryEditor::shiftLibraryUp()
{
   Map *curMap = EditorData::GetInstance().getCurrentMap();
   unsigned int i = 0;
   for(; i < curMap->GetAllLibraries().size(); i++)
      if(libView->currentItem())
         if(curMap->GetAllLibraries()[i] == libView->currentItem()->text().toStdString())
            break;
   
   // already at the top of the list?
   if(i == 0)
      return;
   
   // ensure the current item is selected
   if(libView->currentItem())
   {
      curMap->InsertLibrary(i - 1, libView->currentItem()->text().toStdString(), "");
      refreshLibraries();
      QListWidgetItem *item = libView->item(i - 1);
      if(item)
      {
         libView->setCurrentItem(item);
         if(item == libView->item(0))
            upLib->setDisabled(true);
         if(item == libView->item(libView->count()-1))
            downLib->setDisabled(true);
      }
   }
}

void LibraryEditor::shiftLibraryDown()
{
   Map *curMap = EditorData::GetInstance().getCurrentMap();
   unsigned int i = 0;
   for(; i < curMap->GetAllLibraries().size(); i++)
      if(libView->currentItem())
         if(curMap->GetAllLibraries()[i] == libView->currentItem()->text().toStdString())
            break;
   
   // already at the bottom of the list?
   if(i == curMap->GetAllLibraries().size() - 1)
      return;
   
   // gotta love my QA
   if(libView->currentItem())
   {
      curMap->InsertLibrary(i + 1, libView->currentItem()->text().toStdString(), "");
      refreshLibraries();
      QListWidgetItem *item = libView->item(i + 1);
      if(item)
      {
         libView->setCurrentItem(item);
         if(item == libView->item(0))
            upLib->setDisabled(true);
         if(item == libView->item(libView->count()-1))
            downLib->setDisabled(true);
      }
   }
}

void LibraryEditor::handleFailure(const int errorCode, const std::string &errorMsg)
{
   if(errorCode == ERROR_LIB_NOT_LOADED)
   {
      QMessageBox::critical(this, tr("Failed to delete library"),
                            tr("Library is not currently loaded"),
                            tr("&OK"));
   }
   else if(errorCode == ERROR_ACTORS_IN_LIB)
   {
      QString str = "Failed to remove the library, " + QString::number(numActorsInScene) +
      " actor(s) are currently implemented";
      
      QMessageBox::critical(this, tr("Failed to delete library"),
                            str,
                            tr("&OK"));
   }
   else if(errorCode == ERROR_INVALID_LIB)
   {
      QString message(tr("Error Message: "));
      message.append(errorMsg.c_str());
      message.append("\n\nPlease ensure that the name is correct, the library is in the path (or the working directory), ");
      message.append("the library can load correctly, and dependent libraries are available.");

      QMessageBox::critical(this, tr("Failed to import library"),
                            message,
                            tr("&OK"));
   }
   else
   {
      QMessageBox::critical(this, tr("Failed to delete library"),
                            tr("Failed to remove the library, unknown error"),
                            tr("&OK"));
   }
}

void LibraryEditor::enableButtons()
{
   deleteLib->setDisabled(false);
   upLib->setDisabled(false);
   downLib->setDisabled(false);
}

   void LibraryEditor::disableButtons()
   {
      deleteLib->setDisabled(true);
      upLib->setDisabled(true);
      downLib->setDisabled(true);
   }

}
