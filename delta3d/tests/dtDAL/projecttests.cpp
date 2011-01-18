/* -*-c++-*-
* allTests - This source file (.h & .cpp) - Using 'The MIT License'
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
* @author David Guthrie
*/
#include <prefix/unittestprefix.h>
#include <vector>
#include <set>
#include <string>

#include <cstdio>

#include <dtUtil/datetime.h>
#include <dtUtil/stringutils.h>
#include <dtUtil/tree.h>
#include <dtUtil/log.h>
#include <dtUtil/exception.h>
#include <dtUtil/datetime.h>
#include <dtUtil/fileutils.h>
#include <dtUtil/datapathutils.h>

#include <dtDAL/mapxml.h>
#include <dtDAL/datatype.h>
#include <dtDAL/project.h>
#include <dtDAL/map.h>
#include <dtDAL/exceptionenum.h>

#include <cppunit/extensions/HelperMacros.h>

namespace dtDAL 
{
   class ResourceTreeNode;
   class DataType;
}

const std::string TEST_PROJECT_DIR("TestProject");

class ProjectTests : public CPPUNIT_NS::TestFixture
{
   CPPUNIT_TEST_SUITE(ProjectTests);
   CPPUNIT_TEST(TestReadonlyFailure);
   CPPUNIT_TEST(TestProject);
   CPPUNIT_TEST(TestMultipleContexts);
   CPPUNIT_TEST(TestCategories);
   CPPUNIT_TEST(TestResources);
   CPPUNIT_TEST(TestDeletingBackupFromReadOnlyContext);
   CPPUNIT_TEST(TestNonModifiedMapBackup);
   CPPUNIT_TEST(TestModifiedMapBackup);
   CPPUNIT_TEST(TestMapSaveAndLoadMapName);
   CPPUNIT_TEST(TestMapBackupFilename);
   CPPUNIT_TEST(TestOpenMapBackupCreatesBackups);
   CPPUNIT_TEST(TestMapSaveAsExceptions);
   CPPUNIT_TEST(TestMapSaveAsBackups);
   CPPUNIT_TEST_SUITE_END();

   public:
      void setUp();
      void tearDown();

      void TestProject();
      void TestMultipleContexts();
      void TestFileIO();
      void TestCategories();
      void TestReadonlyFailure();
      void TestResources();
      void TestDeletingBackupFromReadOnlyContext();
      void TestNonModifiedMapBackup();
      void TestModifiedMapBackup();
      void TestMapSaveAndLoadMapName();
      void TestMapBackupFilename();
      void TestOpenMapBackupCreatesBackups();
      void TestMapSaveAsExceptions();
      void TestMapSaveAsBackups();

   private:
      dtUtil::Log* logger;
      void printTree(const dtUtil::tree<dtDAL::ResourceTreeNode>::const_iterator& iter);
      dtUtil::tree<dtDAL::ResourceTreeNode>::const_iterator findTreeNodeFromCategory(
            const dtUtil::tree<dtDAL::ResourceTreeNode>& currentTree,
            const dtDAL::DataType* dt, const std::string& category) const;
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ProjectTests);

const std::string DATA_DIR = dtUtil::GetDeltaRootPath() + dtUtil::FileUtils::PATH_SEPARATOR+"examples/data";
const std::string TESTS_DIR = dtUtil::GetDeltaRootPath() + dtUtil::FileUtils::PATH_SEPARATOR+"tests";
const std::string MAPPROJECTCONTEXT = TESTS_DIR + dtUtil::FileUtils::PATH_SEPARATOR + "dtDAL" + dtUtil::FileUtils::PATH_SEPARATOR + "WorkingMapProject";
const std::string PROJECTCONTEXT = TESTS_DIR + dtUtil::FileUtils::PATH_SEPARATOR + "dtDAL" + dtUtil::FileUtils::PATH_SEPARATOR + "WorkingProject";


void ProjectTests::setUp()
{
   try
   {
      dtUtil::SetDataFilePathList(dtUtil::GetDeltaDataPathList());
      std::string logName("projectTest");

      //        logger = &dtUtil::Log::GetInstance("project.cpp");
      //        logger->SetLogLevel(dtUtil::Log::LOG_DEBUG);
      //        logger = &dtUtil::Log::GetInstance("fileutils.cpp");
      //        logger->SetLogLevel(dtUtil::Log::LOG_DEBUG);
      //        logger = &dtUtil::Log::GetInstance("mapxml.cpp");
      //        logger->SetLogLevel(dtUtil::Log::LOG_DEBUG);

      logger = &dtUtil::Log::GetInstance(logName);

      //        logger->SetLogLevel(dtUtil::Log::LOG_DEBUG);
      //        logger->LogMessage(dtUtil::Log::LOG_DEBUG, __FUNCTION__,  __LINE__, "Log initialized.\n");
      dtUtil::FileUtils& fileUtils = dtUtil::FileUtils::GetInstance();
      fileUtils.ChangeDirectory(TESTS_DIR);
      fileUtils.PushDirectory("dtDAL");

      if (!fileUtils.DirExists("WorkingProject"))
      {
         fileUtils.MakeDirectory("WorkingProject");
      }
      fileUtils.PushDirectory("WorkingProject");
      fileUtils.DirDelete(dtDAL::DataType::STATIC_MESH.GetName(), true);
      fileUtils.DirDelete(dtDAL::DataType::TERRAIN.GetName(), true);
      fileUtils.PopDirectory();

      fileUtils.FileDelete("terrain_simple.ive");
      fileUtils.FileDelete("flatdirt.ive");
      fileUtils.DirDelete("Testing", true);
      fileUtils.DirDelete("recursiveDir", true);

      fileUtils.FileCopy(DATA_DIR + "/models/terrain_simple.ive", ".", false);
      fileUtils.FileCopy(DATA_DIR + "/models/flatdirt.ive", ".", false);

      dtDAL::Project::GetInstance().ClearAllContexts();
      dtDAL::Project::GetInstance().SetReadOnly(false);
   }
   catch (const dtUtil::Exception& ex)
   {
      CPPUNIT_FAIL(ex.ToString());
   }
}


void ProjectTests::tearDown()
{
   dtDAL::Project::GetInstance().ClearAllContexts();
   dtDAL::Project::GetInstance().SetReadOnly(false);

   dtUtil::FileUtils& fileUtils = dtUtil::FileUtils::GetInstance();

   fileUtils.FileDelete("terrain_simple.ive");
   fileUtils.FileDelete("flatdirt.ive");
   fileUtils.DirDelete("Testing", true);
   fileUtils.DirDelete("recursiveDir", true);

   //Delete a couple other projects
   if (fileUtils.DirExists(TEST_PROJECT_DIR))
   {
      fileUtils.DirDelete(TEST_PROJECT_DIR, true);
   }
   if (fileUtils.DirExists("TestProject1"))
   {
      fileUtils.DirDelete("TestProject1", true);
   }
   if (fileUtils.DirExists("TestProject2"))
   {
      fileUtils.DirDelete("TestProject2", true);
   }
   if (fileUtils.DirExists("Test2Project"))
   {
      fileUtils.DirDelete("Test2Project", true);
   }
   if (fileUtils.DirExists("WorkingProject"))
   {
      fileUtils.DirDelete("WorkingProject", true);
   }
   if (fileUtils.DirExists("WorkingProject2"))
   {
      fileUtils.DirDelete("WorkingProject2", true);
   }

   std::string currentDir = fileUtils.CurrentDirectory();
   std::string projectDir("dtDAL");
   if (currentDir.substr(currentDir.size() - projectDir.size()) == projectDir)
   {
      fileUtils.PopDirectory();
   }
}

dtUtil::tree<dtDAL::ResourceTreeNode>::const_iterator ProjectTests::findTreeNodeFromCategory(
      const dtUtil::tree<dtDAL::ResourceTreeNode>& currentTree,
      const dtDAL::DataType* dt, const std::string& category) const {

   if (dt != NULL && !dt->IsResource())
      return currentTree.end();

   std::vector<std::string> tokens;
   dtUtil::StringTokenizer<dtDAL::IsCategorySeparator>::tokenize(tokens, category);
   //if dt == NULL, assume that the datatype name is at the front of the category.
   if (dt != NULL)
      //Push the name of the datetype because it's the top level of the tree.
      tokens.insert(tokens.begin(), dt->GetName());

   std::string currentCategory;

   dtUtil::tree<dtDAL::ResourceTreeNode>::const_iterator ti = currentTree.get_tree_iterator();

   for (std::vector<std::string>::const_iterator i = tokens.begin(); i != tokens.end(); ++i) {
      if (ti == currentTree.end())
         return currentTree.end();

      //std::cout << *i << std::endl;

      //keep a full category string running
      //to create an accurate tree node to compare against.
      //Skip the first token because it is the datatype, not the category.
      if (i != tokens.begin())
      {
         if (currentCategory == "")
         {
            currentCategory += *i;
         }
         else
         {
            currentCategory += dtDAL::ResourceDescriptor::DESCRIPTOR_SEPARATOR + *i;
         }
      }

      ti = ti.tree_ref().find(dtDAL::ResourceTreeNode(*i, currentCategory, NULL, 0));
   }
   return ti;
}

void ProjectTests::printTree(const dtUtil::tree<dtDAL::ResourceTreeNode>::const_iterator& iter)
{
   for (unsigned tabs = 0; tabs < iter.level(); ++tabs)
   {
      std::cout << "\t";
   }

   std::cout << iter->getNodeText();
   if (!iter->isCategory())
   {
      std::cout << " -- " << iter->getResource().GetResourceIdentifier();
   }
   else
   {
      std::cout << " -> " << iter->getFullCategory();

      std::cout << std::endl;

      for (dtUtil::tree<dtDAL::ResourceTreeNode>::const_iterator i = iter.tree_ref().in();
         i != iter.tree_ref().end();
         ++i)
      {
         printTree(i);
      }
   }
}

void ProjectTests::TestReadonlyFailure()
{
   try 
   {
      dtDAL::Project& p = dtDAL::Project::GetInstance();


      p.CreateContext(TEST_PROJECT_DIR);

      try 
      {
         p.SetContext(TEST_PROJECT_DIR);
      } 
      catch (const dtUtil::Exception& e)
      {
         CPPUNIT_FAIL(std::string("Project should have been able to set context. Exception: ") + e.ToString());
      }

      try 
      {
         p.SetContext(TEST_PROJECT_DIR, true);
      } 
      catch (const dtUtil::Exception& e) 
      {
         CPPUNIT_FAIL(std::string("Project should have been able to set context. Exception: ") + e.ToString());
      }

      CPPUNIT_ASSERT_MESSAGE("context should be valid", p.IsContextValid());
      CPPUNIT_ASSERT_MESSAGE("context should be valid", p.IsReadOnly());

      try 
      {
         p.Refresh();
      } 
      catch (const dtUtil::Exception& e) 
      {
         CPPUNIT_FAIL(std::string("Project should have been able to call refresh: ") + e.ToString());
      }

      try 
      {
         dtUtil::tree<dtDAL::ResourceTreeNode> toFill;
         p.GetResourcesOfType(dtDAL::DataType::STATIC_MESH, toFill);
      } 
      catch (const dtUtil::Exception& e) 
      {
         CPPUNIT_FAIL(std::string("Project should have been able to call GetResourcesOfType: ") + e.ToString());
      }

      try 
      {
         p.GetAllResources();
      } 
      catch (const dtUtil::Exception& e) 
      {
         CPPUNIT_FAIL(std::string("Project should have been able to call GetResourcesOfType: ") + e.ToString());
      }

      CPPUNIT_ASSERT_THROW_MESSAGE("DeleteMap() should have thrown exception on a ReadOnly ProjectContext",
                                   p.DeleteMap("mojo"), dtDAL::ProjectReadOnlyException);

      CPPUNIT_ASSERT_THROW_MESSAGE("SaveMap() should have thrown exception on a ReadOnly ProjectContext",
                                    p.SaveMap("mojo"), dtDAL::ProjectReadOnlyException);

      CPPUNIT_ASSERT_THROW_MESSAGE("SaveMapAs() should have thrown exception on a ReadOnly ProjectContext",
                                    p.SaveMapAs("mojo", "a", "b"), dtDAL::ProjectReadOnlyException);

      CPPUNIT_ASSERT_THROW_MESSAGE("AddResource() should have thrown exception on a ReadOnly ProjectContext",
                                    p.AddResource("mojo", "../jojo.ive","fun:bigmamajama", dtDAL::DataType::STATIC_MESH),
                                    dtDAL::ProjectReadOnlyException);

      CPPUNIT_ASSERT_THROW_MESSAGE("RemoveResource() should have thrown exception on a ReadOnly ProjectContext",
                                   p.RemoveResource(dtDAL::ResourceDescriptor("","")),dtDAL::ProjectReadOnlyException);

      CPPUNIT_ASSERT_THROW_MESSAGE("CreateResourceCategory() should have thrown exception on a ReadOnly ProjectContext",
                                   p.CreateResourceCategory("name-o", dtDAL::DataType::STRING),dtDAL::ProjectReadOnlyException);

      CPPUNIT_ASSERT_THROW_MESSAGE("RemoveResourceCategory() should have thrown exception on a ReadOnly ProjectContext",
                                   p.RemoveResourceCategory("name-o", dtDAL::DataType::SOUND, true),dtDAL::ProjectReadOnlyException);

      CPPUNIT_ASSERT_THROW_MESSAGE("CreateMap() should have thrown exception on a ReadOnly ProjectContext",
                                   p.CreateMap("name-o", "testFile"),dtDAL::ProjectReadOnlyException);

   } 
   catch (const dtUtil::Exception& ex) 
   {
      CPPUNIT_FAIL(ex.ToString());
   }
   //    catch (const std::exception &e) {
   //       CPPUNIT_FAIL(std::string("Caught an exception of type") + typeid(e).name() + " with message " + e.what());
   //    }

}

void ProjectTests::TestCategories()
{
   try
   {
      dtDAL::Project& p = dtDAL::Project::GetInstance();

      dtUtil::FileUtils& fileUtils = dtUtil::FileUtils::GetInstance();

      std::string projectDir2("TestProject2");

      try {
         p.CreateContext(TEST_PROJECT_DIR);
         p.CreateContext(projectDir2);
         p.SetContext(TEST_PROJECT_DIR);
         p.AddContext(projectDir2);
      } catch (const dtUtil::Exception& e) {
         CPPUNIT_FAIL(std::string(std::string("Project should have been able to set context. Exception: ") + e.ToString()).c_str());
      }

      for (std::vector<dtDAL::DataType*>::const_iterator i = dtDAL::DataType::EnumerateType().begin();
      i != dtDAL::DataType::EnumerateType().end(); ++i) {
         dtDAL::DataType& d = **i;

         //don't index the first time so it will be tested both ways.
         if (i != dtDAL::DataType::EnumerateType().begin())
            p.GetAllResources();

         if (!d.IsResource()) {
            const std::string PRIM_CATEGORY_MSG("Project should not be able to create a category for a primitive type.");
            CPPUNIT_ASSERT_THROW_MESSAGE(PRIM_CATEGORY_MSG, p.CreateResourceCategory("littleFoot", d), dtUtil::Exception);
            CPPUNIT_ASSERT_THROW_MESSAGE(PRIM_CATEGORY_MSG, p.CreateResourceCategory("littleFoot", d, 0), dtUtil::Exception);
            CPPUNIT_ASSERT_THROW_MESSAGE(PRIM_CATEGORY_MSG, p.CreateResourceCategory("littleFoot", d, 1), dtUtil::Exception);
         } else {
            p.CreateResourceCategory("abomb", d);

            CPPUNIT_ASSERT_MESSAGE(
                  "attempting to remove a simple category should succeed.",
                  p.RemoveResourceCategory("abomb", d, false));

            p.CreateResourceCategory("abomb:hbomb", d);

            CPPUNIT_ASSERT_MESSAGE(
                  "attempting to remove a simple category should succeed.",
                  p.RemoveResourceCategory("abomb:hbomb", d, false));

            std::string catPath(p.GetContext(0) + dtUtil::FileUtils::PATH_SEPARATOR
                  + d.GetName() + dtUtil::FileUtils::PATH_SEPARATOR + "abomb");

            std::string catPath2(p.GetContext(1) + dtUtil::FileUtils::PATH_SEPARATOR
                  + d.GetName() + dtUtil::FileUtils::PATH_SEPARATOR + "abomb");

            CPPUNIT_ASSERT_MESSAGE("Static mesh category abomb should exist.",
                  fileUtils.DirExists(catPath));

            p.CreateResourceCategory("abomb:jojo:eats:hummus", d);

            std::string longPath(catPath + dtUtil::FileUtils::PATH_SEPARATOR + "jojo"
                  + dtUtil::FileUtils::PATH_SEPARATOR + "eats"
                  + dtUtil::FileUtils::PATH_SEPARATOR + "hummus");

            CPPUNIT_ASSERT_MESSAGE(std::string("Static mesh category ") + longPath + " should exist.",
                  fileUtils.DirExists(longPath));
            //printTree(p.GetAllResources());
            CPPUNIT_ASSERT(p.RemoveResourceCategory("abomb:jojo:eats:hummus", d, false));
            CPPUNIT_ASSERT_MESSAGE(std::string("Static mesh category ") + longPath + " should NOT exist.",
                  !fileUtils.DirExists(longPath));

            CPPUNIT_ASSERT_MESSAGE(
                  "Attempting to non-recursivly remove a category with contents should return false.",
                  !p.RemoveResourceCategory("abomb:jojo", d, false));

            CPPUNIT_ASSERT_MESSAGE("Static mesh category abomb should exist.",
                  fileUtils.DirExists(catPath + dtUtil::FileUtils::PATH_SEPARATOR + "jojo"
                        + dtUtil::FileUtils::PATH_SEPARATOR + "eats"));

            CPPUNIT_ASSERT(p.RemoveResourceCategory("abomb", d, true));
            CPPUNIT_ASSERT_MESSAGE("Static mesh category abomb should not exist.",
                  !fileUtils.DirExists(catPath));
         }
      }



   }
   catch (const dtUtil::Exception& ex)
   {
      CPPUNIT_FAIL(ex.ToString());
   }
   //    catch (const std::exception& ex) {
   //        CPPUNIT_FAIL(ex.what());
   //    }

}

void ProjectTests::TestMultipleContexts()
{

}

void ProjectTests::TestResources()
{
   try
   {
      dtDAL::Project& p = dtDAL::Project::GetInstance();

      dtUtil::FileUtils& fileUtils = dtUtil::FileUtils::GetInstance();

      //Open an existing project with two directories.
      const std::string projectDir = "WorkingProject";
      const std::string projectDir2 = "WorkingProject2";

      try
      {
         p.CreateContext(projectDir);
         p.CreateContext(projectDir2);
         p.AddContext(projectDir);
         p.AddContext(projectDir2);
      }
      catch (const dtUtil::Exception& e)
      {
         CPPUNIT_FAIL((std::string("Project should have been able to Set context. Exception: ")
               + e.ToString()).c_str());
      }

      CPPUNIT_ASSERT_MESSAGE("Project should not be read only.", !p.IsReadOnly());
      CPPUNIT_ASSERT_MESSAGE("Delta3D search path should contain the context.",
            dtUtil::GetDataFilePathList().find(p.GetContext(0)) != std::string::npos);
      CPPUNIT_ASSERT_MESSAGE("Delta3D search path should contain the context.",
            dtUtil::GetDataFilePathList().find(p.GetContext(1)) != std::string::npos);

      const std::set<std::string>& mapNames = p.GetMapNames();

      std::vector<dtCore::RefPtr<dtDAL::Map> > maps;


      const std::string& utcTime = dtUtil::DateTime::ToString(dtUtil::DateTime(dtUtil::DateTime::TimeOrigin::LOCAL_TIME),
         dtUtil::DateTime::TimeFormat::CALENDAR_DATE_AND_TIME_FORMAT);

      logger->LogMessage(dtUtil::Log::LOG_DEBUG, __FUNCTION__,  __LINE__,
            "Current time as UTC is %s", utcTime.c_str());

      std::vector<const dtDAL::ResourceTypeHandler* > handlers;

      p.GetHandlersForDataType(dtDAL::DataType::TERRAIN, handlers);
      CPPUNIT_ASSERT_MESSAGE("There should be 4 terrain type handlers",  handlers.size() == 4);

      std::string testResult;

      CPPUNIT_ASSERT_THROW_MESSAGE("The AddResource() to add a non-existent file should have failed.",
                                   p.AddResource("mojo", "../jojo.ive", "fun:bigmamajama", dtDAL::DataType::STATIC_MESH, 0),
                                   dtUtil::Exception);

      CPPUNIT_ASSERT_THROW_MESSAGE("The AddResource() to add a mis-matched DataType should have failed.",
                                   p.AddResource("dirt", "../terrain_simple.ive", "fun:bigmamajama", dtDAL::DataType::BOOLEAN, 1),
                                   dtUtil::Exception);

      std::string dirtCategory = "fun:bigmamajama";


      dtDAL::ResourceDescriptor terrain1RD = p.AddResource("terrain1", DATA_DIR + "/models/exampleTerrain", "terrain",
            dtDAL::DataType::TERRAIN, 0);

      //force resources to be indexed.
      p.GetAllResources();


      dtDAL::ResourceDescriptor terrain2RD = p.AddResource("terrain2", DATA_DIR + "/models/exampleTerrain/terrain.3ds", "",
            dtDAL::DataType::TERRAIN, 1);

      //printTree(p.GetAllResources());

      dtUtil::tree<dtDAL::ResourceTreeNode> toFill;

      p.GetResourcesOfType(dtDAL::DataType::TERRAIN, toFill);
      dtUtil::tree<dtDAL::ResourceTreeNode>::const_iterator terrainCategory =
         findTreeNodeFromCategory(toFill, NULL, "");

      CPPUNIT_ASSERT_MESSAGE(std::string("the category \"")
            + "\" should have been found in the resource tree", terrainCategory != p.GetAllResources().end());

      dtUtil::tree<dtDAL::ResourceTreeNode>::const_iterator terrain2Resource =
         terrainCategory.tree_ref().find(dtDAL::ResourceTreeNode("terrain2", terrainCategory->getFullCategory(), &terrain2RD, 0));

      terrainCategory = findTreeNodeFromCategory(toFill, NULL, "terrain");
      printTree(p.GetAllResources());

      CPPUNIT_ASSERT_MESSAGE(std::string("the category \"terrain")
            + "\" should have been found in the resource tree", terrainCategory != p.GetAllResources().end());

      dtUtil::tree<dtDAL::ResourceTreeNode>::const_iterator terrain1Resource =
         terrainCategory.tree_ref().find(dtDAL::ResourceTreeNode("terrain1", terrainCategory->getFullCategory(), &terrain1RD, 0));

      CPPUNIT_ASSERT_MESSAGE("The terrain2 resource should have been found.", terrain2Resource != p.GetAllResources().end());
      CPPUNIT_ASSERT_MESSAGE("The terrain1 resource should have been found.", terrain1Resource != p.GetAllResources().end());

      std::string terrainDir0(p.GetContext(0) + dtUtil::FileUtils::PATH_SEPARATOR + dtDAL::DataType::TERRAIN.GetName() + dtUtil::FileUtils::PATH_SEPARATOR);
      std::string terrainDir1(p.GetContext(1) + dtUtil::FileUtils::PATH_SEPARATOR + dtDAL::DataType::TERRAIN.GetName() + dtUtil::FileUtils::PATH_SEPARATOR);

      CPPUNIT_ASSERT(fileUtils.DirExists(terrainDir1 + "terrain2.3dst" ) &&
            fileUtils.FileExists(terrainDir1 + "terrain2.3dst" + dtUtil::FileUtils::PATH_SEPARATOR + "terrain.3ds"));

      CPPUNIT_ASSERT(fileUtils.DirExists(terrainDir0 + "terrain" +
            dtUtil::FileUtils::PATH_SEPARATOR + "terrain1.3dst") &&
            fileUtils.FileExists(terrainDir0 + "terrain" + dtUtil::FileUtils::PATH_SEPARATOR + "terrain1.3dst" + dtUtil::FileUtils::PATH_SEPARATOR + "terrain.3ds"));

      //Done with the terrains

      dtDAL::ResourceDescriptor rd = p.AddResource("flatdirt", std::string(DATA_DIR + "/models/flatdirt.ive"),
            dirtCategory, dtDAL::DataType::STATIC_MESH, 0);

      CPPUNIT_ASSERT_MESSAGE("Descriptor id should not be empty.", !rd.GetResourceIdentifier().empty());

      testResult = p.GetResourcePath(rd);

      CPPUNIT_ASSERT_EQUAL_MESSAGE("Getting the resource path returned the wrong value",
            testResult, p.GetContext(0) + dtUtil::FileUtils::PATH_SEPARATOR + dtDAL::DataType::STATIC_MESH.GetName() + dtUtil::FileUtils::PATH_SEPARATOR
            + "fun" + dtUtil::FileUtils::PATH_SEPARATOR + "bigmamajama"
            + dtUtil::FileUtils::PATH_SEPARATOR + "flatdirt.ive");

      for (std::set<std::string>::const_iterator i = mapNames.begin(); i != mapNames.end(); i++)
      {
         logger->LogMessage(dtUtil::Log::LOG_DEBUG, __FUNCTION__,  __LINE__, "Found map named %s.", i->c_str());
         //dtDAL::Map& m = p.GetMap(*i);

         //maps.Push_back(dtCore::RefPtr<dtDAL::Map>(&m));
      }

      p.GetResourcesOfType(dtDAL::DataType::STATIC_MESH, toFill);

      CPPUNIT_ASSERT_MESSAGE("The head of the tree should be static mesh",
            toFill.data().getNodeText() == dtDAL::DataType::STATIC_MESH.GetName());

      dtUtil::tree<dtDAL::ResourceTreeNode>::const_iterator treeResult =
         findTreeNodeFromCategory(p.GetAllResources(),
               &dtDAL::DataType::STATIC_MESH, dirtCategory);

      //printTree(toFill.tree_iterator());

      CPPUNIT_ASSERT_MESSAGE(std::string("the category \"") + dirtCategory
            + "\" should have been found in the resource tree", treeResult != p.GetAllResources().end());


      CPPUNIT_ASSERT_MESSAGE(std::string("the resource \"") + rd.GetResourceIdentifier()
            + "\" should have been found in the resource tree",
            treeResult.tree_ref().find(dtDAL::ResourceTreeNode(rd.GetDisplayName(), dirtCategory, &rd, 0))
            != p.GetAllResources().end());

      p.RemoveResource(rd);

      treeResult = findTreeNodeFromCategory(p.GetAllResources(),
            &dtDAL::DataType::STATIC_MESH, dirtCategory);

      CPPUNIT_ASSERT_MESSAGE(std::string("the category \"") + dirtCategory
            + "\" should have been found in the resource tree", treeResult != p.GetAllResources().end());

      CPPUNIT_ASSERT_MESSAGE(std::string("the resource \"") + rd.GetResourceIdentifier()
            + "\" should have NOT been found in the resource tree",
            treeResult.tree_ref().find(dtDAL::ResourceTreeNode(rd.GetDisplayName(), dirtCategory,  &rd, 0))
            == p.GetAllResources().end());

      CPPUNIT_ASSERT(!p.RemoveResourceCategory("fun", dtDAL::DataType::STATIC_MESH, false));
      CPPUNIT_ASSERT(p.RemoveResourceCategory("fun", dtDAL::DataType::STATIC_MESH, true));

      treeResult = findTreeNodeFromCategory(p.GetAllResources(),
            &dtDAL::DataType::STATIC_MESH, dirtCategory);
      CPPUNIT_ASSERT_MESSAGE(std::string("the category \"") + dirtCategory
            + "\" should not have been found in the resource tree", treeResult == p.GetAllResources().end());

      treeResult = findTreeNodeFromCategory(p.GetAllResources(),
            &dtDAL::DataType::STATIC_MESH, "fun");
      CPPUNIT_ASSERT_MESSAGE(std::string("the category \"") + "fun"
            + "\" should not have been found in the resource tree", treeResult == p.GetAllResources().end());

      rd = p.AddResource("pow", std::string(DATA_DIR + "/sounds/pow.wav"), std::string("tea:money"), dtDAL::DataType::SOUND, 0);
      testResult = p.GetResourcePath(rd);

      CPPUNIT_ASSERT_EQUAL_MESSAGE("Getting the resource path returned the wrong value: ",
            testResult, p.GetContext(0) + dtUtil::FileUtils::PATH_SEPARATOR +
            dtDAL::DataType::SOUND.GetName() + dtUtil::FileUtils::PATH_SEPARATOR
            + "tea" + dtUtil::FileUtils::PATH_SEPARATOR
            + "money" + dtUtil::FileUtils::PATH_SEPARATOR + "pow.wav");

      dtDAL::ResourceDescriptor rd1 = p.AddResource("bang", std::string(DATA_DIR + "/sounds/bang.wav"),
            std::string("tee:cash"), dtDAL::DataType::SOUND, 0);
      testResult = p.GetResourcePath(rd1);

      CPPUNIT_ASSERT_EQUAL_MESSAGE("Getting the resource path returned the wrong value:",
            testResult, p.GetContext(0) + dtUtil::FileUtils::PATH_SEPARATOR +
            dtDAL::DataType::SOUND.GetName() + dtUtil::FileUtils::PATH_SEPARATOR + "tee"
            + dtUtil::FileUtils::PATH_SEPARATOR + "cash" + dtUtil::FileUtils::PATH_SEPARATOR + "bang.wav");

      CPPUNIT_ASSERT_THROW_MESSAGE("Getting the path to a resource directory should throw an exception",
               p.GetResourcePath(dtDAL::DataType::SOUND.GetName()), dtDAL::ProjectResourceErrorException);

      CPPUNIT_ASSERT_EQUAL_MESSAGE("Getting the path to a resource directory should work in this case.",
               p.GetContext(0) + dtUtil::FileUtils::PATH_SEPARATOR + dtDAL::DataType::SOUND.GetName(),
               p.GetResourcePath(dtDAL::DataType::SOUND.GetName(), true));

      p.Refresh();

      //const dtUtil::tree<dtDAL::ResourceTreeNode>& allTree = p.GetAllResources();

      //printTree(allTree.tree_iterator());

      p.RemoveResource(rd);
      p.RemoveResource(rd1);
      p.RemoveResource(terrain1RD);
      p.RemoveResource(terrain2RD);

      fileUtils.PushDirectory(projectDir);
      CPPUNIT_ASSERT_MESSAGE("Resource should have been deleted, but the file still exists.",
            !fileUtils.FileExists(dtDAL::DataType::SOUND.GetName() + std::string("/tea/money/pow.wav")));
      CPPUNIT_ASSERT_MESSAGE("Resource should have been deleted, but the file still exists.",
            !fileUtils.DirExists(dtDAL::DataType::TERRAIN.GetName() + std::string("/terrain/terrain1.3dst")));
      fileUtils.PopDirectory();

      fileUtils.PushDirectory(projectDir2);
      CPPUNIT_ASSERT_MESSAGE("Resource should have never been in the project, but the file still exists.",
            !fileUtils.FileExists(dtDAL::DataType::STATIC_MESH.GetName() + std::string("/fun/bigmamajama/terrain_simple.ive")));

      CPPUNIT_ASSERT_MESSAGE("Resource should have been deleted, but the file still exists.",
            !fileUtils.DirExists(dtDAL::DataType::TERRAIN.GetName() + std::string("/terrain2.3dst")));
      CPPUNIT_ASSERT_MESSAGE("Resource should have been deleted, but the file still exists.",
            !fileUtils.FileExists(dtDAL::DataType::SOUND.GetName() + std::string("/tee/cash/bang.wav")));
      fileUtils.PopDirectory();

      //this should work fine even if the file is deleted.
      p.RemoveResource(rd);

   }
   catch (const dtUtil::Exception& ex)
   {
      CPPUNIT_FAIL(ex.ToString());
   }
   //    catch (const std::exception& ex) {
   //        CPPUNIT_FAIL(ex.what());
   //    }

}


void ProjectTests::TestProject()
{
   try
   {
      dtDAL::Project& p = dtDAL::Project::GetInstance();
      dtUtil::FileUtils& fileUtils = dtUtil::FileUtils::GetInstance();
      std::string originalPathList = dtUtil::GetDataFilePathList();

      std::string crapPath("/usr/:%**/../^^jojo/funky/\\\\/,/,.uchor");
      
      CPPUNIT_ASSERT_THROW(p.CreateContext(crapPath), dtUtil::Exception);
      CPPUNIT_ASSERT_THROW(p.SetContext(crapPath), dtUtil::Exception);


      if (fileUtils.FileExists(TEST_PROJECT_DIR))
      {
         try
         {
            fileUtils.DirDelete(TEST_PROJECT_DIR, true);
         }
         catch (const dtUtil::Exception& ex)
         {
            CPPUNIT_FAIL(ex.ToString().c_str());
         }

         CPPUNIT_ASSERT_MESSAGE("The project Directory should not yet exist.", !fileUtils.FileExists(TEST_PROJECT_DIR));
      }

      fileUtils.MakeDirectory(TEST_PROJECT_DIR);

      try
      {
         p.SetContext(TEST_PROJECT_DIR);
         CPPUNIT_FAIL("Project should not have been able to Set the context because it is empty.");
      }
      catch (const dtUtil::Exception&)
      {
         //correct
      }

      try {
         p.CreateContext(TEST_PROJECT_DIR);
         p.SetContext(TEST_PROJECT_DIR);
      } catch (const dtUtil::Exception& e) {
         CPPUNIT_FAIL(std::string(std::string("Project should have been able to Set context. Exception: ") + e.ToString()).c_str());
      }

      CPPUNIT_ASSERT_MESSAGE("Project should not be read only.", !p.IsReadOnly());
      CPPUNIT_ASSERT_MESSAGE("Delta3D search path should contain the context.",
            dtUtil::GetDataFilePathList().find(p.GetContext()) != std::string::npos);
      CPPUNIT_ASSERT_MESSAGE("Delta3D search path should contain the original path list.",
            dtUtil::GetDataFilePathList().find(originalPathList) != std::string::npos);

      try {
         p.SetContext(TEST_PROJECT_DIR, true);
      } catch (const dtUtil::Exception& e) {
         CPPUNIT_FAIL(std::string(std::string("Project should have been able to Set context. Exception: ") + e.ToString()).c_str());
      }

      CPPUNIT_ASSERT_MESSAGE("Project should be read only.", p.IsReadOnly());
      CPPUNIT_ASSERT_MESSAGE("Delta3D search path should contain the context.",
            dtUtil::GetDataFilePathList().find(p.GetContext()) != std::string::npos);
      CPPUNIT_ASSERT_MESSAGE("Delta3D search path should contain the original path list.",
            dtUtil::GetDataFilePathList().find(originalPathList) != std::string::npos);

      std::string projectDir2("Test2Project");
      try 
      {
         p.CreateContext(projectDir2);
         p.SetContext(projectDir2);
      } 
      catch (const dtUtil::Exception& e) 
      {
         CPPUNIT_FAIL(std::string(std::string("Project should have been able to Set context. Exception: ") + e.ToString()).c_str());
      }

      CPPUNIT_ASSERT_MESSAGE("Project should be read only.", !p.IsReadOnly());
      CPPUNIT_ASSERT_MESSAGE("Delta3D search path should contain the context.",
            dtUtil::GetDataFilePathList().find(p.GetContext()) != std::string::npos);

      CPPUNIT_ASSERT_MESSAGE("Delta3D search path should contain the context.",
            dtUtil::GetDataFilePathList().find(projectDir2) != std::string::npos);
      CPPUNIT_ASSERT_MESSAGE("Delta3D search path should NOT contain the old context.",
            dtUtil::GetDataFilePathList().find(TEST_PROJECT_DIR) == std::string::npos);
      CPPUNIT_ASSERT_MESSAGE("Delta3D search path should contain the original path list.",
            dtUtil::GetDataFilePathList().find(originalPathList) != std::string::npos);

      try {
         fileUtils.DirDelete(TEST_PROJECT_DIR, true);
      } catch (const dtUtil::Exception& ex) {
         CPPUNIT_FAIL(ex.ToString().c_str());
      }


      CPPUNIT_ASSERT_MESSAGE("The project Directory should have been deleted.", !fileUtils.FileExists(TEST_PROJECT_DIR));

   } catch (const dtUtil::Exception& ex) {
      CPPUNIT_FAIL(ex.ToString());
   }
   //    catch (const std::exception& ex) {
   //        CPPUNIT_FAIL(ex.what());
   //    }

}

////////////////////////////////////////////////////////////////////////////////
void ProjectTests::TestDeletingBackupFromReadOnlyContext()
{
   const std::string mapName("mapWithBackup");
   dtDAL::Project& proj = dtDAL::Project::GetInstance();

   proj.CreateContext(TEST_PROJECT_DIR);
   proj.SetContext(TEST_PROJECT_DIR, false);

   //create a Map and save a backup
   dtDAL::Map& testMap = proj.CreateMap(mapName, "mapWithBackup");
   testMap.SetModified(true); //needs to be modified before creating a backup
   proj.SaveMapBackup(testMap);

   CPPUNIT_ASSERT_MESSAGE("Didn't find a Map backup, can't go on with test",
                          true == proj.HasBackup(testMap));
   
   proj.SetContext(TEST_PROJECT_DIR, true); //now we're read only

   //trying to delete a Map backup in a read-only ProjectContext should throw
   CPPUNIT_ASSERT_THROW(proj.ClearBackup(mapName), dtUtil::Exception);

   proj.SetContext(TEST_PROJECT_DIR, false); //now we're not read only
   proj.ClearBackup(mapName); //this should work
}
//////////////////////////////////////////////////////////////////////////
void ProjectTests::TestNonModifiedMapBackup()
{
   std::string mapName("UnmodifiedMap");
   std::string mapFileName("UnmodifiedMap");

   dtDAL::Project& project = dtDAL::Project::GetInstance();
   project.CreateContext(TEST_PROJECT_DIR);
   project.SetContext(TEST_PROJECT_DIR, false);

   dtDAL::Map* map = &project.CreateMap(mapName, mapFileName);

   project.SaveMapBackup(*map);

   //test both versions of the call.
   CPPUNIT_ASSERT_MESSAGE("Map was not modified.  There should be no backup saves.",
      !project.HasBackup(*map) && !project.HasBackup(mapName));

   project.DeleteMap(*map);
}

////////////////////////////////////////////////////////////////////////////////
void ProjectTests::TestModifiedMapBackup()
{
   std::string mapName("ModifiedMap");
   std::string mapFileName("ModifiedMap");

   dtDAL::Project& project = dtDAL::Project::GetInstance();
   project.CreateContext(TEST_PROJECT_DIR);
   project.SetContext(TEST_PROJECT_DIR, false);

   dtDAL::Map* map = &project.CreateMap(mapName, mapFileName);

   //modify the map
   map->SetDescription("Teague is league with a \"t\".");

   const std::string filenameBeforeBackup = map->GetFileName();

   project.SaveMapBackup(*map);

   //test both versions of the call.
   CPPUNIT_ASSERT_MESSAGE("A backup was just saved.  The map should have backups.",
                           project.HasBackup(*map) && project.HasBackup(mapName));

   project.ClearBackup(*map);

   //test both versions of the call.
   CPPUNIT_ASSERT_MESSAGE("Backups were cleared.  The map should have no backups.",
                           !project.HasBackup(*map) && !project.HasBackup(mapName));

   project.DeleteMap(*map);
}

//////////////////////////////////////////////////////////////////////////
void ProjectTests::TestMapSaveAndLoadMapName()
{
   dtDAL::Project& project = dtDAL::Project::GetInstance();
   project.CreateContext(TEST_PROJECT_DIR);
   project.SetContext(TEST_PROJECT_DIR, false);

   const std::string mapName("Neato Map");
   const std::string mapFileName("neatomap");

   dtDAL::Map* map = &project.CreateMap(mapName, mapFileName);

   const std::string newMapName("Weirdo Map");

   //set the name to make sure it can be changed.
   map->SetName(newMapName);

   CPPUNIT_ASSERT_EQUAL_MESSAGE("Map should have the new name.", newMapName, map->GetName());

   CPPUNIT_ASSERT_EQUAL_MESSAGE("Map should have the old saved name", mapName, map->GetSavedName());

   project.SaveMapBackup(*map);

   CPPUNIT_ASSERT_MESSAGE("A backup was just saved.  The map should have backups.",
                          project.HasBackup(*map) && project.HasBackup(mapName));

   project.SaveMap(*map);

   //test both versions of the call.
   CPPUNIT_ASSERT_MESSAGE("Map was saved.  The map should have no backups.",
                          !project.HasBackup(*map) && !project.HasBackup(newMapName));

   CPPUNIT_ASSERT_EQUAL_MESSAGE("Map should have the new saved name", newMapName, map->GetSavedName());     

   project.DeleteMap(*map);
}

////////////////////////////////////////////////////////////////////////////////
void ProjectTests::TestMapBackupFilename()
{
   dtDAL::Project& project = dtDAL::Project::GetInstance();
   project.CreateContext(TEST_PROJECT_DIR);
   project.SetContext(TEST_PROJECT_DIR, false);
   const std::string mapName("Neato Map");
   const std::string mapFileName("neatomap");


   dtDAL::Map* map = &project.CreateMap(mapName, mapFileName);

   const std::string newAuthor("Dr. Eddie");
   map->SetAuthor(newAuthor);

   const std::string filenameBeforeBackup = map->GetFileName();;
   project.SaveMapBackup(*map);

   CPPUNIT_ASSERT_EQUAL_MESSAGE("Map filename should not have changed after preforming a backup",
                                filenameBeforeBackup, map->GetFileName());

   map = &project.OpenMapBackup(mapName);

   CPPUNIT_ASSERT_EQUAL_MESSAGE("Map filename should not have changed after loading a backup.",
                                filenameBeforeBackup, map->GetFileName());

   project.DeleteMap(*map);
}

////////////////////////////////////////////////////////////////////////////////
void ProjectTests::TestOpenMapBackupCreatesBackups()
{
   dtDAL::Project& project = dtDAL::Project::GetInstance();
   project.CreateContext(TEST_PROJECT_DIR);
   project.SetContext(TEST_PROJECT_DIR, false);
   const std::string mapName("Neato Map");
   const std::string mapFileName("neatomap");

   dtDAL::Map* map = &project.CreateMap(mapName, mapFileName);

   map->SetAuthor("Dr. Eddie");

   project.SaveMapBackup(*map);
   map = &project.OpenMapBackup(mapName);

   CPPUNIT_ASSERT_MESSAGE("Map was loaded as a backup, so it should have backups.",
                           project.HasBackup(*map) && project.HasBackup(mapName));

   project.SaveMapBackup(*map);
   CPPUNIT_ASSERT_MESSAGE("A backup was just saved.  The map should have a backup.",
                           project.HasBackup(*map) && project.HasBackup(mapName));

   project.DeleteMap(*map);
}

////////////////////////////////////////////////////////////////////////////////
void ProjectTests::TestMapSaveAsExceptions()
{
   dtDAL::Project& project = dtDAL::Project::GetInstance();
   project.CreateContext(TEST_PROJECT_DIR);
   project.SetContext(TEST_PROJECT_DIR, false);
   const std::string mapName("Neato Map");
   const std::string mapFileName("neatomap");

   dtDAL::Map* map = &project.CreateMap(mapName, mapFileName);
   map->SetAuthor("unit test");

   CPPUNIT_ASSERT_THROW_MESSAGE("Calling SaveAs on a map with the same name and filename should fail.",
      project.SaveMapAs(*map, mapName, mapFileName),
      dtUtil::Exception);

   CPPUNIT_ASSERT_THROW_MESSAGE("Calling SaveAs on a map with the same filename should fail.",
      project.SaveMapAs(*map, "asdf", mapFileName),
      dtUtil::Exception);

   CPPUNIT_ASSERT_THROW_MESSAGE("Calling SaveAs on a map with the same name should fail.",
      project.SaveMapAs(*map, mapName, "asdf"),
      dtUtil::Exception);

   project.DeleteMap(*map);
}

////////////////////////////////////////////////////////////////////////////////
void ProjectTests::TestMapSaveAsBackups()
{
   dtDAL::Project& project = dtDAL::Project::GetInstance();
   project.CreateContext(TEST_PROJECT_DIR);
   project.SetContext(TEST_PROJECT_DIR, false);
   const std::string mapName("Neato Map");
   const std::string newMapName("new map name");
   const std::string mapFileName("neatomap");
   const std::string newMapFileName("oo");

   dtDAL::Map* map = &project.CreateMap(mapName, mapFileName);
   map->SetAuthor("unit test");

   project.SaveMapAs(*map, newMapName, newMapFileName);

   //test both versions of the call.
   CPPUNIT_ASSERT_MESSAGE("Map was just saved AS.  The map should have no backups.",
      !project.HasBackup(*map) && !project.HasBackup(mapName));

   CPPUNIT_ASSERT_MESSAGE("Map was just saved AS.  The old map should have no backups.",
      !project.HasBackup(mapName));

   CPPUNIT_ASSERT_EQUAL_MESSAGE("Map file name should have changed during a SaveAs",
      newMapFileName + dtDAL::Map::MAP_FILE_EXTENSION, map->GetFileName());

   CPPUNIT_ASSERT_MESSAGE("Map name didn't change during a SaveAs.",
      map->GetName() == newMapName && map->GetSavedName() == newMapName);

   std::string newMapFilePath = project.GetContext() + dtUtil::FileUtils::PATH_SEPARATOR + "maps"
      + dtUtil::FileUtils::PATH_SEPARATOR + "oo" + dtDAL::Map::MAP_FILE_EXTENSION;

   CPPUNIT_ASSERT_MESSAGE(std::string("The new map file should exist: ") + newMapFilePath,
      dtUtil::FileUtils::GetInstance().FileExists(newMapFilePath));

   project.DeleteMap(*map);
}
