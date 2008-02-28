/* 
* Delta3D Open Source Game and Simulation Engine 
* Copyright (C) 2006 MOVES Institute 
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
* Chris Osborn
*/ 
#include <prefix/dtgameprefix-src.h>
#include <cppunit/extensions/HelperMacros.h>
#include <dtUtil/mathdefines.h>
#include <dtCore/scene.h>
#include <dtCore/transformable.h>
#include <dtCore/object.h>

#include <osg/MatrixTransform>
#include <osg/io_utils>
#include <ode/collision.h>
#include <sstream>
#include <cmath>

using namespace dtCore;

class TransformableSubClass : public Transformable
{
   public:
   TransformableSubClass()
   {
   }

   void ExecuteReplaceMatrixNode()
   {
      osg::ref_ptr<osg::MatrixTransform> mNewNode( new osg::MatrixTransform() );
      mNewNode->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::ON );
      ReplaceMatrixNode( mNewNode.get() );
   }
   protected:
   virtual ~TransformableSubClass()
   {
   }
};

class TransformableTests : public CPPUNIT_NS::TestFixture 
{
   CPPUNIT_TEST_SUITE(TransformableTests);
   CPPUNIT_TEST(TestDefaultBools);
   CPPUNIT_TEST(TestGetCollisionGeomDimensions);
   CPPUNIT_TEST(TestSetCollisionBox);
   CPPUNIT_TEST(TestSetTransform);
   CPPUNIT_TEST(TestSetMatrix);
   CPPUNIT_TEST(TestSetPosHPR);
   CPPUNIT_TEST(TestSetPosQuat);
   CPPUNIT_TEST(TestScale);
   CPPUNIT_TEST(TestTranslation);
   CPPUNIT_TEST(TestMove);
   CPPUNIT_TEST(TestRotationHPR);
   CPPUNIT_TEST(TestRotationQuat);
   CPPUNIT_TEST(TestTransRotScaleGetSet);
   CPPUNIT_TEST(TestReplaceMatrixNode);
   CPPUNIT_TEST(TestConstructorTakingMatrixNode);
   CPPUNIT_TEST(TestRows);
   CPPUNIT_TEST_SUITE_END();

public:

   void setUp();
   void tearDown();

   void TestDefaultBools();
   void TestGetCollisionGeomDimensions();
   void TestSetCollisionBox();
   void TestSetTransform();
   void TestSetPosHPR();
   void TestSetPosQuat();
   void TestScale();
   void TestTranslation();
   void TestMove();
   void TestRotationHPR();
   void TestRotationQuat();
   void TestTransRotScaleGetSet();
   void TestReplaceMatrixNode();
   void TestConstructorTakingMatrixNode();
   void TestSetMatrix();
   void TestRows();

private:
    bool CompareMatrix(const osg::Matrix& rhs, const osg::Matrix& lhs) const;
    bool CompareVector(const osg::Vec3& rhs, const osg::Vec3& lhs) const;    

   RefPtr<Transformable> mTransformable;
   Transform mTransform;
      
   osg::Vec3 mBoxLengths; ///Dimensions for box

   float mRadius; ///Radius for sphere or cylinder
   float mHeight; ///Height for cylinder or ray
   static const osg::Vec3::value_type TEST_EPSILON;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TransformableTests);

const float TransformableTests::TEST_EPSILON(1e-5f);

void TransformableTests::setUp()
{
   mTransformable = new dtCore::Transformable("TestTransformable");
   mTransform.Set( 2.0f, 4.0f, 1.0f, 30.0f, -20.0f, 2.5f );
   mBoxLengths.set( 2.0f, 3.0f, 4.0f );
   mRadius = 3.2f;
   mHeight = 6.4f;
}

void TransformableTests::tearDown()
{
   mTransformable->SetCollisionMesh(NULL);
   mTransformable->SetCollisionBox( mBoxLengths[0], mBoxLengths[1], mBoxLengths[2] );
   mTransformable = NULL;
}

void TransformableTests::TestDefaultBools()
{
   CPPUNIT_ASSERT(!mTransformable->GetRenderCollisionGeometry());
   CPPUNIT_ASSERT(!mTransformable->GetIsRenderingProxyNode());
   CPPUNIT_ASSERT(!mTransformable->GetCollisionDetection());
}

void TransformableTests::TestGetCollisionGeomDimensions()
{
   std::vector<float> dimensions;
   mTransformable->GetCollisionGeomDimensions(dimensions);

   CPPUNIT_ASSERT_MESSAGE( "Vector filled by GetCollisionGeomDimensions should be empty",
      dimensions.empty() );

   // Box
   mTransformable->SetCollisionBox( mBoxLengths[0], mBoxLengths[1], mBoxLengths[2] );

   dimensions.clear();
   mTransformable->GetCollisionGeomDimensions(dimensions);

   CPPUNIT_ASSERT_MESSAGE( "Vector filled by GetCollisionGeomDimensions should only have 3 elements (since it's a cube",
      dimensions.size() ==  3 );

   CPPUNIT_ASSERT_MESSAGE( "Collision geomertry dimensions should be (2.0f, 3.0f, 4.0f)", 
      osg::equivalent( dimensions[0], mBoxLengths[0], 1e-2f ) && 
      osg::equivalent( dimensions[1], mBoxLengths[1], 1e-2f ) && 
      osg::equivalent( dimensions[2], mBoxLengths[2], 1e-2f ) );
}

void TransformableTests::TestSetCollisionBox()
{
   dGeomID geomID = mTransformable->GetGeomID();
   CPPUNIT_ASSERT_MESSAGE( "GeomID should be valid even without a valid collision shape", 
      geomID != 0 );

   CPPUNIT_ASSERT_MESSAGE( "Collision detection should by false without a valid collision shape", 
      mTransformable->GetCollisionDetection() == false );

   CPPUNIT_ASSERT_MESSAGE( "Collision geometry type should be NONE without a collision shape", 
      mTransformable->GetCollisionGeomType() == &Transformable::CollisionGeomType::NONE );

   mTransformable->SetCollisionBox( mBoxLengths[0], mBoxLengths[1], mBoxLengths[2] );

   geomID = mTransformable->GetGeomID();
   CPPUNIT_ASSERT_MESSAGE( "GeomID should still be valid", 
      geomID != 0 );

   CPPUNIT_ASSERT_MESSAGE( "Collision detection should by true", 
      mTransformable->GetCollisionDetection() == true );

   CPPUNIT_ASSERT_MESSAGE( "Collision geometry type should be CUBE", 
      mTransformable->GetCollisionGeomType() == &Transformable::CollisionGeomType::CUBE );
}

void TransformableTests::TestSetTransform()
{
   // In order to test scale, we must have a valid collsion geom
   mTransformable->SetCollisionBox( mBoxLengths[0], mBoxLengths[1], mBoxLengths[2] );

   // Verify ODE transforms are initialized correct

   dGeomID geomID = mTransformable->GetGeomID();

   // Position
   const dReal* odePosition = dGeomGetPosition(geomID);
   CPPUNIT_ASSERT_MESSAGE( "ODE position should be (0.0f,0.0f,0.0f)",
      osg::equivalent( float(odePosition[0]), 0.0f, 1e-2f ) && 
      osg::equivalent( float(odePosition[1]), 0.0f, 1e-2f ) && 
      osg::equivalent( float(odePosition[2]), 0.0f, 1e-2f ) );

   // Rotation
   const dReal* odeRotation = dGeomGetRotation(geomID);
   CPPUNIT_ASSERT_MESSAGE( "ODE's rotation should be zero",
      osg::equivalent( double(odeRotation[0]), 1.0, 1e-2 ) && 
      osg::equivalent( double(odeRotation[1]), 0.0, 1e-2 ) && 
      osg::equivalent( double(odeRotation[2]), 0.0, 1e-2 ) && 
      osg::equivalent( double(odeRotation[4]), 0.0, 1e-2 ) && 
      osg::equivalent( double(odeRotation[5]), 1.0, 1e-2 ) && 
      osg::equivalent( double(odeRotation[6]), 0.0, 1e-2 ) && 
      osg::equivalent( double(odeRotation[8]), 0.0, 1e-2 ) && 
      osg::equivalent( double(odeRotation[9]), 0.0, 1e-2 ) && 
      osg::equivalent( double(odeRotation[10]), 1.0, 1e-2 ) );

   // Scale
   std::vector<float> dimensions;
   mTransformable->GetCollisionGeomDimensions(dimensions);
   CPPUNIT_ASSERT_MESSAGE( "Collision geomertry dimensions should be (2.0f, 3.0f, 4.0f)", 
      osg::equivalent( dimensions[0], mBoxLengths[0], 1e-2f ) && 
      osg::equivalent( dimensions[1], mBoxLengths[1], 1e-2f ) && 
      osg::equivalent( dimensions[2], mBoxLengths[2], 1e-2f ) );
   
   // Now let's set a real Transform and verify ODE knows about it
   mTransformable->SetTransform(mTransform);

   // Position
   osg::Vec3 osgPosition;
   mTransform.GetTranslation(osgPosition);
   odePosition = dGeomGetPosition(geomID);
   CPPUNIT_ASSERT_MESSAGE( "ODE and OSG's position should be in sync",
      osg::equivalent( osgPosition[0], float(odePosition[0]), 1e-2f ) && 
      osg::equivalent( osgPosition[1], float(odePosition[1]), 1e-2f ) && 
      osg::equivalent( osgPosition[2], float(odePosition[2]), 1e-2f ) );

   // Rotation
   osg::Matrix osgRotation;
   mTransform.GetRotation(osgRotation);
   odeRotation = dGeomGetRotation(geomID);
   CPPUNIT_ASSERT_MESSAGE( "ODE and OSG's rotation should be in sync",
      osg::equivalent( double(osgRotation(0,0)), double(odeRotation[0]), 1e-2 ) && 
      osg::equivalent( double(osgRotation(1,0)), double(odeRotation[1]), 1e-2 ) && 
      osg::equivalent( double(osgRotation(2,0)), double(odeRotation[2]), 1e-2 ) && 
      osg::equivalent( double(osgRotation(0,1)), double(odeRotation[4]), 1e-2 ) && 
      osg::equivalent( double(osgRotation(1,1)), double(odeRotation[5]), 1e-2 ) && 
      osg::equivalent( double(osgRotation(2,1)), double(odeRotation[6]), 1e-2 ) && 
      osg::equivalent( double(osgRotation(0,2)), double(odeRotation[8]), 1e-2 ) && 
      osg::equivalent( double(osgRotation(1,2)), double(odeRotation[9]), 1e-2 ) && 
      osg::equivalent( double(osgRotation(2,2)), double(odeRotation[10]), 1e-2 ) );

   // Scale
   dimensions.clear();
   mTransformable->GetCollisionGeomDimensions(dimensions);
   CPPUNIT_ASSERT_MESSAGE( "Collision geomertry dimensions should be (4.0f, 6.0f, 8.0f)", 
      osg::equivalent( dimensions[0], mBoxLengths[0], 1e-2f ) && 
      osg::equivalent( dimensions[1], mBoxLengths[1], 1e-2f ) && 
      osg::equivalent( dimensions[2], mBoxLengths[2], 1e-2f ) );
}

bool HasChild( dtCore::DeltaDrawable* parent, dtCore::DeltaDrawable* child )
{
   bool result(false);
   for( unsigned i = 0; i < parent->GetNumChildren(); ++i )
   {
      result = result || parent->GetChild(i) == child;
   }
   return result;
}

bool HasChild( osg::Group* parent, osg::Node* child )
{
   return parent->containsNode(child);
}

void TransformableTests::TestSetPosHPR()
{
   osg::Vec3 hpr (10.0, -30.0, 23.0);
   osg::Vec3 pos(21.0, 33.66, 293.9);

   dtCore::Transform xform;
   xform.Set(pos, hpr);

   osg::Vec3 retPos, retHpr;

   xform.Get(retPos, retHpr);

   CPPUNIT_ASSERT(CompareVector(pos, retPos));
   CPPUNIT_ASSERT(CompareVector(hpr, retHpr));
}

void TransformableTests::TestSetPosQuat()
{
   osg::Quat quat (std::sqrt(0.5), 0.0, std::sqrt(0.5), 0.0);
   osg::Vec3 pos(21.0, 33.66, 293.9);

   dtCore::Transform xform;
   xform.Set(pos, quat);

   osg::Vec3 retPos;
   osg::Quat retQuat;

   xform.Get(retPos, retQuat);

   CPPUNIT_ASSERT(CompareVector(pos, retPos));
   CPPUNIT_ASSERT(dtUtil::Equivalent(quat, retQuat, 4, osg::Quat::value_type(0.001f)));
}

void TransformableTests::TestSetMatrix()
{
   osg::Vec3 trans(10.0f, 7.0f, 2.0f);
   osg::Vec3 scale(3.0f, 1.0f, 5.0f);
   osg::Matrix matRotate, matScale, matTest;

   matRotate.makeRotate(osg::DegreesToRadians(45.0), osg::Vec3(1.0f, 0.0f, 0.0f));
   matScale.makeScale(scale);
   matRotate = matRotate * matScale;
   matTest = matRotate;
   matTest.setTrans(trans);

   osg::Vec3 testScale, testTrans;
   osg::Matrix testRot;

   dtCore::Transform transformTest;
    
   transformTest.Set(matTest);
   transformTest.GetTranslation(testTrans);
   transformTest.CalcScale(testScale);
   transformTest.GetRotation(testRot);

   CPPUNIT_ASSERT(CompareMatrix(matRotate, testRot));
   CPPUNIT_ASSERT(CompareVector(scale, testScale));
   CPPUNIT_ASSERT(CompareVector(trans, testTrans));
}

void TransformableTests::TestScale()
{
   osg::Vec3 scale(3.0f, 1.0f, 5.0f);

   dtCore::Transform transformTest;
   transformTest.Rescale(scale);
   mTransformable->SetTransform(transformTest);
   mTransformable->GetTransform(transformTest);

   osg::Vec3 testScale;
   transformTest.CalcScale(testScale);

   std::ostringstream ss;
   ss << "Value is: \"" << testScale << "\" but it should be \"" << scale << "\"";
   CPPUNIT_ASSERT_MESSAGE(ss.str(), CompareVector(scale, testScale));
}

void TransformableTests::TestTranslation()
{
   osg::Vec3 trans(10.0f, 7.0f, 2.0f);

   dtCore::Transform transformTest;
   transformTest.SetTranslation(trans);
   mTransformable->SetTransform(transformTest);
   mTransformable->GetTransform(transformTest);

   osg::Vec3 testTrans;
   transformTest.GetTranslation(testTrans);

   std::ostringstream ss;
   ss << "Value is: \"" << testTrans << "\" but it should be \"" << trans << "\"";
   CPPUNIT_ASSERT_MESSAGE(ss.str(), CompareVector(trans, testTrans));
}

void TransformableTests::TestMove()
{
   osg::Vec3 trans(10.0f, 7.0f, 2.0f);
   osg::Vec3f moveF(-1.1, -3.6, 21.44);
   osg::Vec3d moveD(moveF);

   dtCore::Transform transformTest;
   transformTest.SetTranslation(trans);

   transformTest.Move(moveF);
   
   osg::Vec3 testTrans;
   transformTest.GetTranslation(testTrans);

   std::ostringstream ss;
   ss << "Value is: \"" << testTrans << "\" but it should be \"" << trans + moveF << "\"";
   CPPUNIT_ASSERT_MESSAGE(ss.str(), CompareVector(trans + moveF, testTrans));

   transformTest.SetTranslation(trans);

   transformTest.Move(moveD);
   
   transformTest.GetTranslation(testTrans);

   ss.str("");
   ss << "Value is: \"" << testTrans << "\" but it should be \"" << trans + osg::Vec3(moveD) << "\"";
   CPPUNIT_ASSERT_MESSAGE(ss.str(), CompareVector(trans + osg::Vec3(moveD), testTrans));
}

void TransformableTests::TestRotationHPR()
{
   osg::Vec3 rot(10.0f, 7.0f, 2.0f);

   dtCore::Transform transformTest;
   transformTest.SetRotation(rot);
   mTransformable->SetTransform(transformTest);
   mTransformable->GetTransform(transformTest);

   osg::Vec3 testRot;
   transformTest.GetRotation(testRot);

   std::ostringstream ss;
   ss << "Value is: \"" << testRot << "\" but it should be \"" << rot << "\"";
   CPPUNIT_ASSERT_MESSAGE(ss.str(), CompareVector(rot, testRot));
}

void TransformableTests::TestRotationQuat()
{
   osg::Quat qrot(0.0, 0.0, 1.0, 0.0);

   dtCore::Transform transformTest;
   transformTest.SetRotation(qrot);

   osg::Quat resultQuat;
   
   transformTest.GetRotation(resultQuat);
      
   std::ostringstream ss;
   ss << "Value is: \"" << resultQuat << "\" but it should be \"" << qrot << "\"";
   CPPUNIT_ASSERT_MESSAGE(ss.str(), dtUtil::Equivalent(qrot, resultQuat, 4, 0.001));   

   osg::Matrix expectedMatrix, resultMatrix;
   expectedMatrix.makeIdentity();

   expectedMatrix.setRotate(qrot);
   
   transformTest.GetRotation(resultMatrix);
   ss.str("Value is: \"");
   ss << expectedMatrix << "\" but it should be \"" << resultMatrix << "\"";
   CPPUNIT_ASSERT_MESSAGE(ss.str(), CompareMatrix(expectedMatrix, resultMatrix));   
}

void TransformableTests::TestTransRotScaleGetSet()
{
   osg::Vec3 trans(10.0f, 7.0f, 2.0f);
   osg::Vec3 rot(10.0f, 7.0f, 2.0f);

   dtCore::Transform transformTest;
   transformTest.SetTranslation(trans);
   transformTest.SetRotation(rot);
   mTransformable->SetTransform(transformTest);
   mTransformable->GetTransform(transformTest);

   osg::Vec3 testScale, testRot, testTrans;
   transformTest.GetTranslation(testTrans);
   transformTest.GetRotation(testRot);

   std::ostringstream ss;
   ss << "rotation value is: \"" << testRot << "\" but it should be \"" << rot << "\"";
   CPPUNIT_ASSERT_MESSAGE(ss.str(), CompareVector(rot, testRot));
   ss.str("");
   ss << "translation value is: \"" << testTrans << "\" but it should be \"" << trans << "\"";
   CPPUNIT_ASSERT_MESSAGE(ss.str(), CompareVector(trans, testTrans));
}


bool TransformableTests::CompareMatrix(const osg::Matrix& rhs, const osg::Matrix& lhs) const
{
   for(int i = 0; i < 4; ++i)
   {   
      for(int j = 0; j < 4; ++j)
      {
         if ( std::abs(rhs(i, j) - lhs(i, j)) > TEST_EPSILON ) return false;
      }
   }

   return true;
}

bool TransformableTests::CompareVector(const osg::Vec3& rhs, const osg::Vec3& lhs) const
{
   return dtUtil::Equivalent(rhs, lhs, TEST_EPSILON);
}

void TransformableTests::TestReplaceMatrixNode()
{
   // This subclass should immediately call SetMatrixNode in the constructor with
   // GL_LIGHTING turned on.
   dtCore::RefPtr<TransformableSubClass> testTrans( new TransformableSubClass() );
   testTrans->ExecuteReplaceMatrixNode();

   // So, the new node should...
   // 1) Have a StateSet
   osg::ref_ptr<const osg::StateSet> stateSet( testTrans->GetOSGNode()->getStateSet() );
   CPPUNIT_ASSERT( stateSet.valid() );

   // 2) Still have GL_LIGHTING turned on (from new Node)
   CPPUNIT_ASSERT( osg::StateAttribute::ON == stateSet->getMode( GL_LIGHTING ) );

   // 3) Still have GL_NORMAL_RESCALING turned on (from original Node)
   CPPUNIT_ASSERT( osg::StateAttribute::ON == stateSet->getMode( GL_RESCALE_NORMAL ) );


   //////////

   dtCore::RefPtr<TransformableSubClass> testTransTwo( new TransformableSubClass() );
   testTransTwo->SetCollisionBox(2.0f, 3.0f, 4.0f);
   
   dtCore::RefPtr<Transformable> parent( new Transformable("Parent") );
                     
   dtCore::RefPtr<Transformable> childOne( new Transformable("ChildOne") );
   dtCore::RefPtr<Transformable> childTwo( new Transformable("ChildTwo") );

   parent->AddChild( testTransTwo.get() );
   testTransTwo->AddChild( childTwo.get() );
   testTransTwo->AddChild( childOne.get() );

   // Verify Delta3D hierarchy before replacement
   CPPUNIT_ASSERT( HasChild( parent.get(), testTransTwo.get() ) );
   CPPUNIT_ASSERT( HasChild( testTransTwo.get(), childTwo.get() ) );
   CPPUNIT_ASSERT( HasChild( testTransTwo.get(), childOne.get() ) );

   // Verify OSG hierarchy before replacement
   CPPUNIT_ASSERT( HasChild( parent->GetMatrixNode(), testTransTwo->GetOSGNode() ) );
   CPPUNIT_ASSERT( HasChild( testTransTwo->GetMatrixNode(), childTwo->GetOSGNode() ) );
   CPPUNIT_ASSERT( HasChild( testTransTwo->GetMatrixNode(), childOne->GetOSGNode() ) );

   CPPUNIT_ASSERT_EQUAL( 2U, testTransTwo->GetMatrixNode()->getNumChildren() );

   bool renderingCollisionGeometry = testTransTwo->GetRenderCollisionGeometry();
   
   testTransTwo->ExecuteReplaceMatrixNode();

   CPPUNIT_ASSERT_EQUAL(   &Transformable::CollisionGeomType::CUBE, 
                           testTransTwo->GetCollisionGeomType() );

   CPPUNIT_ASSERT_EQUAL( renderingCollisionGeometry, testTransTwo->GetRenderCollisionGeometry() );

   // Verify Delta3D hierarchy after replacement
   CPPUNIT_ASSERT( HasChild( parent.get(), testTransTwo.get() ) );
   CPPUNIT_ASSERT( HasChild( testTransTwo.get(), childTwo.get() ) );
   CPPUNIT_ASSERT( HasChild( testTransTwo.get(), childOne.get() ) );

   CPPUNIT_ASSERT_EQUAL( 2U, testTransTwo->GetMatrixNode()->getNumChildren() );
   
   CPPUNIT_ASSERT( HasChild( parent->GetMatrixNode(), testTransTwo->GetOSGNode() ) );
   CPPUNIT_ASSERT( HasChild( testTransTwo->GetMatrixNode(), childTwo->GetOSGNode() ) );
   CPPUNIT_ASSERT( HasChild( testTransTwo->GetMatrixNode(), childOne->GetOSGNode() ) );
}

void TransformableTests::TestConstructorTakingMatrixNode()
{
   osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform();
   dtCore::RefPtr<dtCore::Object> object = new dtCore::Object( *mt );

   CPPUNIT_ASSERT_EQUAL_MESSAGE("Object does not contain the supplied MatrixNode",
                                 mt.get(), object->GetMatrixNode() );

   CPPUNIT_ASSERT_MESSAGE("Transformable does not contain the supplied node",
                           mt.get() == object->GetOSGNode() );
}

void TransformableTests::TestRows()
{
   dtCore::Transform transformTest;
   osg::Matrix m;
   float count = 0.0;
   for (size_t i = 0; i < 4; ++i)
   {
      for (size_t j = 0; j < 4; ++j)
      {
         m(i,j) = count;
         count++;
      }
   }
   
   transformTest.Set(m);
   
   osg::Vec3 row3, row3Expected;
   osg::Vec4 row4, row4Expected;

   transformTest.GetRow(1, row3);
   row3Expected.set(4.0, 5.0, 6.0);

   CPPUNIT_ASSERT(dtUtil::Equivalent(row3Expected, row3, TEST_EPSILON));

   row3Expected.set(9.0, 8.0, 1.0);
   transformTest.SetRow(2, row3Expected);
   transformTest.GetRow(2, row3);
   CPPUNIT_ASSERT(dtUtil::Equivalent(row3Expected, row3, TEST_EPSILON));
   
   transformTest.GetRow(1, row4);
   row4Expected.set(4.0, 5.0, 6.0, 7.0);
   CPPUNIT_ASSERT(dtUtil::Equivalent(row4Expected, row4, TEST_EPSILON));

   row4Expected.set(104.3, 90.7, 98.6, 101.6);
   transformTest.SetRow(2, row4Expected);
   transformTest.GetRow(2, row4);
   CPPUNIT_ASSERT(dtUtil::Equivalent(row4Expected, row4, TEST_EPSILON));
}
