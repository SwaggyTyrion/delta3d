/* -*-c++-*-
* allTests - This source file (.h & .cpp) - Using 'The MIT License'
* Copyright (C) 2006-2008, MOVES Institute
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
* @author Bradley Anderegg
*/
#include <prefix/dtgameprefix-src.h>
#include <cppunit/extensions/HelperMacros.h>

#include "testplannerutils.h"
#include <dtAI/basenpc.h>
#include <dtAI/npcparser.h>
#include <dtCore/globals.h>
#include <dtCore/refptr.h>

using namespace dtAI;


namespace dtTest
{
   class PlannerTests : public CPPUNIT_NS::TestFixture
   {
      CPPUNIT_TEST_SUITE(PlannerTests );
      CPPUNIT_TEST(TestCreatePlan);
      CPPUNIT_TEST(TestPlannerScript);
      CPPUNIT_TEST_SUITE_END();

   public:
      void setUp();
      void tearDown();

      void TestCreatePlan();
      void TestPlannerScript();

   private:

      void VerifyPlan(std::list<const Operator*>& pPlan, bool pCallGrandma);

   };


   // Registers the fixture into the 'registry'
   CPPUNIT_TEST_SUITE_REGISTRATION( PlannerTests );



   void PlannerTests::setUp()
   {
      
   }


   void PlannerTests::tearDown()
   {
      
   }

   void PlannerTests::TestCreatePlan()
   {
      MyNPC mNPC;
      mNPC.InitNPC();
      mNPC.SpawnNPC();
      mNPC.GeneratePlan();
      std::list<const Operator*> pOperators = mNPC.GetPlan();
      VerifyPlan(pOperators, true);    

      //step through the plan
      while(!mNPC.GetPlan().empty())
      {
         mNPC.Update(0.05);
      }

      mNPC.MakeHungry();
      mNPC.GeneratePlan();
      pOperators = mNPC.GetPlan();
      VerifyPlan(pOperators, false);

      //step through the next plan
      while(!mNPC.GetPlan().empty())
      {
         mNPC.Update(0.05);
      }

   }

   void PlannerTests::TestPlannerScript()
   {
      NPCParser parser;            
      dtCore::RefPtr<BaseNPC> pTestNPC = new BaseNPC("TestNPC");
      pTestNPC->LoadNPCScript(dtCore::GetDeltaRootPath() + "/tests/dtAI/npcscript_test.txt");
      pTestNPC->InitNPC();
      pTestNPC->SpawnNPC();

      pTestNPC->GeneratePlan();
      std::list<const Operator*> pOperators = pTestNPC->GetPlan();
      VerifyPlan(pOperators, true);
   }

   void PlannerTests::VerifyPlan(std::list<const Operator*>& pOperators, bool pCallGrandma)
   {
      if(pCallGrandma)
      {
         std::string callGrandma("CallGrandma");
         CPPUNIT_ASSERT_EQUAL(callGrandma, pOperators.front()->GetName());
         pOperators.pop_front();
      }

      std::string goToStore("GoToStore");
      CPPUNIT_ASSERT_EQUAL(goToStore, pOperators.front()->GetName());
      pOperators.pop_front();

      std::string cook("Cook");
      CPPUNIT_ASSERT_EQUAL(cook, pOperators.front()->GetName());
      pOperators.pop_front();

      std::string eat("Eat");
      CPPUNIT_ASSERT_EQUAL(eat, pOperators.front()->GetName());
      pOperators.pop_front();

   }

}
