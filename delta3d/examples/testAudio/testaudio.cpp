#include <assert.h>
#include <stack>

#include "testaudio.h"



// name spaces
using namespace   dtCore;
using namespace   dtABC;
using namespace   dtAudio;
using namespace   std;

IMPLEMENT_MANAGEMENT_LAYER( TestAudioApp )

// static member variables
unsigned int   TestAudioApp::kNumSoundFiles(4L);
const char*    TestAudioApp::kSoundFile[] =
               {
                  "sounds/pow.wav",
                  "sounds/bang.wav",
                  "sounds/helo.wav",
                  "sounds/carhorn.wav"
               };
const char*    TestAudioApp::kGfxFile[kNumGfx] =
               {
                  "Dirt/flatdirt.ive",
                  "BRDM/brdm-2.ive",
                  "UH-1N/UH-1N.ive"
               };
const char*    TestAudioApp::kFxFile[kNumFx] =
               {
                  "explosion.osg",
                  "smoke.osg"
               };



TestAudioApp::TestAudioApp( string configFilename /*= "config.xml"*/ )
:  Application(configFilename),
   mSndGain(1.0f),
   mSndPitch(1.0f),
   mLooping(false),
   mMic(NULL),
   mInputDevice(NULL),
   mMotionModel(NULL),
   mSmokeCountA(0L),
   mSmokeCountC(0L)
{
   AddSender( dtCore::System::GetSystem() );

   AudioManager::Instantiate();

   AudioConfigData   data;//(32L, true);
   AudioManager::GetManager()->Config( data );

   for( unsigned int ii(0L); ii < kNumSoundFiles; ii++ )
   {
      AudioManager::GetManager()->LoadWaveFile( kSoundFile[ii] );
   }

   mMic  = AudioManager::GetListener();
   assert( mMic );

   SetUpVisuals();

   dtCore::Camera*   cam   = GetCamera();
   assert( cam );

   cam->AddChild( mMic );

   dtCore::Transform transform( 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f );
   mMic->SetTransform( &transform, dtCore::Transformable::REL_CS );

   mSFXBinder  = new dtAudio::SoundEffectBinder;
   assert( mSFXBinder.get() );

   if( mFXMgr.get() )
   {
      mSFXBinder->Initialize( mFXMgr.get() );
      mSFXBinder->AddEffectTypeMapping( dtCore::HighExplosiveDetonation, kSoundFile[1L] );
      mSFXBinder->AddEffectTypeRange( dtCore::HighExplosiveDetonation, 35.0f );
   }

}



TestAudioApp::~TestAudioApp()
{
   if( mSFXBinder.get() )
   {
      mSFXBinder->RemoveEffectTypeRange( dtCore::HighExplosiveDetonation );
      mSFXBinder->RemoveEffectTypeMapping( dtCore::HighExplosiveDetonation );
      mSFXBinder->Shutdown();
      mSFXBinder  = NULL;
   }

   StopAllSounds();
   FreeAllStoppedSounds( true );

   for( unsigned int ii(0L); ii < kNumSoundFiles; ii++ )
   {
      AudioManager::GetManager()->UnloadWaveFile( kSoundFile[ii] );
   }

   AudioManager::Destroy();

   RemoveSender( dtCore::System::GetSystem() );
}



void
TestAudioApp::PreFrame( const double deltaFrameTime )
{
   Application::PreFrame( deltaFrameTime );
   FlushQueuedSounds();
}



void
TestAudioApp::Frame( const double deltaFrameTime )
{
   Application::Frame( deltaFrameTime );
}



void
TestAudioApp::PostFrame( const double deltaFrameTime )
{
   Application::PostFrame( deltaFrameTime );
   FreeAllStoppedSounds();

   MoveTheObject( TRUCK );
   MoveTheObject( HELO );
}



void
TestAudioApp::KeyPressed(  dtCore::Keyboard*       keyboard,
                           Producer::KeyboardKey   key,
                           Producer::KeyCharacter  character   )
{
   dtABC::Application::KeyPressed( keyboard, key, character );
   sgVec3   pos   = { 0.0f, 0.0f, 0.0f };

   switch( key )
   {
      case  Producer::Key_A:
         LoadPlaySound( kSoundFile[0L] );
         break;

      case  Producer::Key_S:
         mFXMgr->AddDetonation( pos, dtCore::HighExplosiveDetonation );
         break;

      case  Producer::Key_D:
         LoadPlaySound( kSoundFile[2L], TRUCK );
         break;

      case  Producer::Key_F:
         LoadPlaySound( kSoundFile[3L], HELO );
         break;

      case  Producer::Key_0:
      case  Producer::Key_KP_Insert:
         ChangeSoundGain( 0.0f / 9.0f );
         break;

      case  Producer::Key_1:
      case  Producer::Key_KP_End:
         ChangeSoundGain( 1.0f / 9.0f );
         break;

      case  Producer::Key_2:
      case  Producer::Key_KP_Down:
         ChangeSoundGain( 2.0f / 9.0f );
         break;

      case  Producer::Key_3:
      case  Producer::Key_KP_Page_Down:
         ChangeSoundGain( 3.0f / 9.0f );
         break;

      case  Producer::Key_4:
      case  Producer::Key_KP_Left:
         ChangeSoundGain( 4.0f / 9.0f );
         break;

      case  Producer::Key_5:
      case  Producer::Key_KP_Begin:
         ChangeSoundGain( 5.0f / 9.0f );
         break;

      case  Producer::Key_6:
      case  Producer::Key_KP_Right:
         ChangeSoundGain( 6.0f / 9.0f );
         break;

      case  Producer::Key_7:
      case  Producer::Key_KP_Home:
         ChangeSoundGain( 7.0f / 9.0f );
         break;

      case  Producer::Key_8:
      case  Producer::Key_KP_Up:
         ChangeSoundGain( 8.0f / 9.0f );
         break;

      case  Producer::Key_9:
      case  Producer::Key_KP_Page_Up:
         ChangeSoundGain( 9.0f / 9.0f );
         break;

      case  Producer::Key_minus:
      case  Producer::Key_KP_Subtract:
         ChangeSoundPitch( 0.9f );
         break;

      case  Producer::Key_equal:
      case  Producer::Key_KP_Add:
         ChangeSoundPitch( 1.1f );
         break;

      case  Producer::Key_L:
         ToggleSoundLooping();
         break;

      case  Producer::Key_Pause:
         PauseAllSounds();
         break;

      case  Producer::Key_Return:
      case  Producer::Key_KP_Enter:
         RewindAllSounds();
         break;

      case  Producer::Key_space:
         StopAllSounds();
         break;

      default:
         break;
   }
}



void
TestAudioApp::LoadPlaySound( const char* fname, unsigned int box /*= 0L*/ )
{
   assert( fname );

   dtCore::Notify( dtCore::ALWAYS, " LoadPlaySound( %s )", fname );

   Sound*   snd = AudioManager::GetManager()->NewSound();
   assert( snd );

   snd->LoadFile( fname );
   snd->SetGain( mSndGain );
   snd->SetPitch( mSndPitch );
   snd->SetLooping( mLooping );
   if( box )
   {
      snd->SetMinDistance( 30.0f );
      snd->SetRolloffFactor( 10.0f );
   }
   snd->Play();
   mQueued.push( snd );

   if( box )
   {
      dtCore::Object*  obj   = mGfxObj[box].get();
      assert( obj );

      obj->AddChild( snd );

      dtCore::Transform transform( 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f );
      snd->SetTransform( &transform, dtCore::Transformable::REL_CS );

      snd->SetPlayCallback( MakeSmoke, this );
      snd->SetStopCallback( StopSmoke, this );
   }
}



void
TestAudioApp::StopAllSounds( void )
{
   Sound*   snd(NULL);
   SND_ITR  iter(NULL);
   for( iter = mActive.begin(); iter != mActive.end(); iter++ )
   {
      snd   = *iter;
      if( snd == NULL )
         continue;

      if( ! snd->IsStopped() )
      {
         snd->Stop();

         dtCore::Notify( dtCore::ALWAYS, " StopAllSounds( %s )", snd->GetFilename() );
      }
   }
}



void
TestAudioApp::FreeAllStoppedSounds( bool forced /*= false*/ )
{
   SND_ITR              iter(NULL);
   std::stack<SND_ITR>  stk;
   Sound*               snd(NULL);

   for( iter = mActive.begin(); iter != mActive.end(); iter++ )
   {
      snd   = *iter;
      if( snd == NULL )
         continue;

      if( snd->IsStopped() || forced )
      {
         stk.push( iter );
      }
   }

   while( stk.size() )
   {
      iter  = stk.top();
      stk.pop();

      snd   = *iter;
      if( snd == NULL )
         continue;

      dtCore::Notify( dtCore::ALWAYS, " FreeAllStoppedSounds( %s )", snd->GetFilename() );

      AudioManager::GetManager()->FreeSound( snd );
      mActive.erase( iter );
   }
}



void
TestAudioApp::FlushQueuedSounds( void )
{
   Sound*   snd(NULL);

   while( mQueued.size() )
   {
      snd   = mQueued.front();
      if( snd == NULL )
      {
         mQueued.pop();
         continue;
      }

      if( ! snd->IsStopped() )
      {
         mQueued.pop();
         mActive.push_back( snd );
         continue;
      }

      break;
   }
}



void
TestAudioApp::ChangeSoundGain( float gain )
{
   mSndGain = gain;

   dtCore::Notify( dtCore::ALWAYS, " ChangeSoundGain( %1.1f )", mSndGain );

   Sound*   snd(NULL);
   SND_ITR  iter(NULL);

   for( iter = mActive.begin(); iter != mActive.end(); iter++ )
   {
      snd   = *iter;
      assert( snd );

      snd->SetGain( mSndGain );
   }
}



void
TestAudioApp::ChangeSoundPitch( float pitch )
{
   mSndPitch   *= pitch;

   dtCore::Notify( dtCore::ALWAYS, " ChangeSoundPitch( %1.4f )", mSndPitch );

   Sound*   snd(NULL);
   SND_ITR  iter(NULL);

   for( iter = mActive.begin(); iter != mActive.end(); iter++ )
   {
      snd   = *iter;
      assert( snd );

      snd->SetPitch( mSndPitch );
   }
}



void
TestAudioApp::ToggleSoundLooping( void )
{
   mLooping =  !mLooping;

   dtCore::Notify( dtCore::ALWAYS, " ToggleSoundLooping( %s )", (mLooping)? "true": "false" );

   Sound*   snd(NULL);
   SND_ITR  iter(NULL);

   for( iter = mActive.begin(); iter != mActive.end(); iter++ )
   {
      snd   = *iter;
      assert( snd );

      snd->SetLooping( mLooping );
   }
}



void
TestAudioApp::PauseAllSounds( void )
{
   Sound*   snd(NULL);
   SND_ITR  iter(NULL);
   for( iter = mActive.begin(); iter != mActive.end(); iter++ )
   {
      snd   = *iter;
      if( snd == NULL )
         continue;

      snd->Pause();

      if( snd->IsPlaying() )
      {
         dtCore::Notify( dtCore::ALWAYS, " PauseAllSounds( %s ) paused", snd->GetFilename() );
      }
      else  if( snd->IsPaused() )
      {
         dtCore::Notify( dtCore::ALWAYS, " PauseAllSounds( %s ) un-paused", snd->GetFilename() );
      }
   }
}



void
TestAudioApp::RewindAllSounds( void )
{
   Sound*   snd(NULL);
   SND_ITR  iter(NULL);
   for( iter = mActive.begin(); iter != mActive.end(); iter++ )
   {
      snd   = *iter;
      if( snd == NULL )
         continue;

      snd->Rewind();

      dtCore::Notify( dtCore::ALWAYS, " RewindAllSounds( %s )", snd->GetFilename() );
   }
}



void
TestAudioApp::SetUpVisuals( void )
{
   for( unsigned int ii(0L); ii < kNumGfx; ii++ )
   {
      mGfxObj[ii] = LoadGfxFile( kGfxFile[ii] );
      assert( mGfxObj[ii].get() );
   }

   mFXMgr   = LoadFxFile( kFxFile[EXPLODE] );
   assert( mFXMgr.get() );

   mPSysA   = LoadPSFile( kFxFile[SMOKE] );
   assert( mPSysA.get() );

   mGfxObj[TRUCK]->AddChild( mPSysA.get() );

   mPSysC   = LoadPSFile( kFxFile[SMOKE] );
   assert( mPSysC.get() );

   mGfxObj[HELO]->AddChild( mPSysC.get() );

   InitInputDevices();
   SetUpCamera();
}



dtCore::Object*
TestAudioApp::LoadGfxFile( const char* fname )
{
   if( fname == NULL )
      // no file name, bail...
      return   NULL;

   std::string filename = osgDB::findDataFile( fname );
   if( filename == "" )
   {
      // still no file name, bail...
      dtCore::Notify( dtCore::WARN, "AudioManager: can't load file %s", fname );
      return   NULL;
   }


   dtCore::Object*  fileobj  = new dtCore::Object;
   assert( fileobj );


   // load the graphics file from disk
   bool fileLoaded = false;
   fileLoaded = fileobj->LoadFile( filename );

   if( ! fileLoaded )
   {
      Notify( WARN, "can't load gfx file '%s'", filename.c_str() );
      delete   fileobj;
      return   NULL;
   }

   if( string(fname) == kGfxFile[1] )
   {
      //rotate BRDM 90 degrees
      Transform trans = Transform( 0.0f, 0.0f, 0.0f, 90.0f, 0.0f, 0.0f );
      fileobj->SetTransform( &trans );
   }

   // add the object to the scene
   AddDrawable( fileobj );

   osg::Node*  filenode = fileobj->GetOSGNode();
   assert( filenode );

   filenode->setNodeMask( 0xFFFFFFFF );
   return   fileobj;
}



dtCore::EffectManager*
TestAudioApp::LoadFxFile( const char* fname )
{
   dtCore::EffectManager* effectManager  = new dtCore::EffectManager;
   assert( effectManager );

   effectManager->AddDetonationTypeMapping(
      dtCore::HighExplosiveDetonation,
      fname
   );

   AddDrawable( effectManager );

   return   effectManager;
}



dtCore::ParticleSystem*
TestAudioApp::LoadPSFile( const char* fname )
{
   dtCore::ParticleSystem*   particlesystem = new dtCore::ParticleSystem;
   
   assert( particlesystem );
   
   particlesystem->LoadFile( fname );

   osg::Node*  filenode = particlesystem->GetOSGNode();
   assert( filenode );

   filenode->setNodeMask( 0x00000000 );

   AddDrawable( particlesystem );

   return   particlesystem;
}



void
TestAudioApp::InitInputDevices( void )
{
   mInputDevice   = new dtCore::LogicalInputDevice;
   assert( mInputDevice );

   dtCore::Keyboard* k  = GetKeyboard();
   assert( k );

   dtCore::Mouse*    m  = GetMouse();
   assert( m );

   dtCore::Axis* leftButtonUpAndDown  =
         mInputDevice->AddAxis(
                                 "left mouse button up/down",
                                 new dtCore::ButtonAxisToAxis(
                                       m->GetButton( dtCore::LeftButton ),
                                       m->GetAxis( 1 )
                                                             )
                              );


   dtCore::Axis* leftButtonLeftAndRight  =
         mInputDevice->AddAxis(
                                 "left mouse button left/right",
                                 new dtCore::ButtonAxisToAxis(
                                       m->GetButton( dtCore::LeftButton ),
                                       m->GetAxis( 0 )
                                                             )
                              );


   dtCore::Axis* middleButtonUpAndDown   =
         mInputDevice->AddAxis(
                                 "middle mouse button up/down",
                                 new dtCore::ButtonAxisToAxis(
                                       m->GetButton( dtCore::MiddleButton ),
                                       m->GetAxis( 1 )
                                                             )
                              );


   dtCore::Axis* rightButtonUpAndDown    =
         mInputDevice->AddAxis(
                                 "right mouse button up/down",
                                 new dtCore::ButtonAxisToAxis(
                                       m->GetButton( dtCore::RightButton ),
                                       m->GetAxis( 1 )
                                                             )
                              );


   dtCore::Axis* rightButtonLeftAndRight =
         mInputDevice->AddAxis(
                                 "right mouse button left/right",
                                 new dtCore::ButtonAxisToAxis(
                                       m->GetButton( dtCore::RightButton ),
                                       m->GetAxis( 0 )
                                                             )
                              );


   dtCore::Axis* arrowKeysUpAndDown      =
         mInputDevice->AddAxis(
                                 "arrow keys up/down",
                                 new dtCore::ButtonsToAxis(
                                       k->GetButton( Producer::Key_Down ),
                                       k->GetButton( Producer::Key_Up )
                                                          )
                              );


   dtCore::Axis* arrowKeysLeftAndRight   =
         mInputDevice->AddAxis(
                                 "arrow keys left/right",
                                 new dtCore::ButtonsToAxis(
                                       k->GetButton( Producer::Key_Left ),
                                       k->GetButton( Producer::Key_Right )
                                                          )
                              );


   dtCore::Axis* wsKeysUpAndDown         =
         mInputDevice->AddAxis(
                                 "w/s keys up/down",
                                 new dtCore::ButtonsToAxis(
                                       k->GetButton( Producer::Key_S ),
                                       k->GetButton( Producer::Key_W )
                                                          )
                              );


   dtCore::Axis* adKeysLeftAndRight      =
         mInputDevice->AddAxis(
                                 "a/d keys left/right",
                                 new dtCore::ButtonsToAxis(
                                       k->GetButton( Producer::Key_A ),
                                       k->GetButton( Producer::Key_D )
                                                          )
                              );


   dtCore::Axis* primaryUpAndDown        =
         mInputDevice->AddAxis(
                                 "primary up/down",
                                 new dtCore::AxesToAxis(
                                       arrowKeysUpAndDown,
                                       leftButtonUpAndDown
                                                       )
                              );


   dtCore::Axis* secondaryUpAndDown      =
         mInputDevice->AddAxis(
                                 "secondary up/down",
                                 new dtCore::AxesToAxis(
                                       wsKeysUpAndDown,
                                       rightButtonUpAndDown
                                                       )
                              );


   dtCore::Axis* primaryLeftAndRight     =
         mInputDevice->AddAxis(
                                 "primary left/right",
                                 new dtCore::AxesToAxis(
                                       arrowKeysLeftAndRight,
                                       leftButtonLeftAndRight
                                                       )
                              );


   dtCore::Axis* secondaryLeftAndRight   =
         mInputDevice->AddAxis(
                                 "secondary left/right",
                                 new dtCore::AxesToAxis(
                                       adKeysLeftAndRight,
                                       rightButtonLeftAndRight
                                                       )
                              );


   dtCore::OrbitMotionModel* omm   = new dtCore::OrbitMotionModel;
   assert( omm );

   omm->SetAzimuthAxis( primaryLeftAndRight );
   omm->SetElevationAxis( primaryUpAndDown );
   omm->SetDistanceAxis( middleButtonUpAndDown );
   omm->SetLeftRightTranslationAxis( secondaryLeftAndRight );
   omm->SetUpDownTranslationAxis( secondaryUpAndDown );
   mMotionModel   = omm;

   mMotionModel->SetTarget( GetCamera() );
   mMotionModel->SetEnabled( true );
}



void
TestAudioApp::SetUpCamera( void )
{
   sgVec3   pos      = { 0.0f, -150.0f, 30.0f };
   sgVec3   lookat   = { 0.f, 0.f, 0.f };
   sgVec3   up       = { 0.f, 0.f, 1.f };

   dtCore::Transform   xform;
   xform.SetLookAt( pos, lookat, up );

   float dist(sgDistanceVec3( lookat, pos ));

   dtCore::Camera*   cam(GetCamera());
   assert( cam );

   cam->SetTransform( &xform );


   dtCore::OrbitMotionModel*  omm   =
      static_cast<dtCore::OrbitMotionModel*>(mMotionModel);
   assert( omm );

   omm->SetDistance( dist );
}



void
TestAudioApp::MoveTheObject( unsigned int obj )
{
   // figure out which vehicle gets what velocity
   static   long              X(0L);
   static   const double      A(1.0f/2500.0f);
   static   const double      C(1.0f/2500.0f);
            double            D((obj==HELO)? C: A);
            double            P(sin( double(X++) * D ));
            double            V(cos( double(X++) * D ));
            unsigned int      I((obj==TRUCK)? 1L: 0L);
            dtCore::Transform xform;
            sgVec3            pos   = { 0.0f, 0.0f, 0.0f };
            sgVec3            vel   = { 0.0f, 0.0f, 0.0f };
            OBJ_PTR           gfx(mGfxObj[obj]);

   // move the vehicle
   assert( gfx.get() );

   gfx->GetTransform( &xform );
   xform.GetTranslation( pos );

   pos[I]   = static_cast<SGfloat>(P * 50.0f);

   xform.SetTranslation( pos );
   gfx->SetTransform( &xform );

   // don't set velocity for TRUCK
   if( obj == TRUCK )
      return;

   // set the velocity for all children of the vehicle
   vel[I]   = static_cast<ALfloat>(V * 50.0f);
   Sound*   snd(NULL);
   for( unsigned int ii(0L); ii < gfx->GetNumChildren(); ii++ )
   {
      snd   = dynamic_cast<Sound*>(gfx->GetChild( ii ));
      if( snd == NULL )
         continue;

      snd->SetVelocity( vel );
   }
}



void
TestAudioApp::MakeSmoke( dtAudio::Sound* sound, void* param )
{
   assert( sound );
   assert( param );

   sound->SetPlayCallback( NULL, NULL );

   std::string    fname = sound->GetFilename();
   TestAudioApp*  app   = static_cast<TestAudioApp*>(param);

   if( fname == app->kSoundFile[2L] )
   {
      assert( app->mPSysA.get() );

      app->mSmokeCountA++;

      osg::Node*  node  = app->mPSysA->GetOSGNode();
      assert( node );

      node->setNodeMask( 0xFFFFFFFF );
      return;
   }

   if( fname == app->kSoundFile[3L] )
   {
      assert( app->mPSysC.get() );

      app->mSmokeCountC++;

      osg::Node*  node  = app->mPSysC->GetOSGNode();
      assert( node );

      node->setNodeMask( 0xFFFFFFFF );
      return;
   }
}



void
TestAudioApp::StopSmoke( dtAudio::Sound* sound, void* param )
{
   assert( sound );
   assert( param );

   sound->SetStopCallback( NULL, NULL );

   std::string    fname = sound->GetFilename();
   TestAudioApp*  app   = static_cast<TestAudioApp*>(param);

   if( fname == app->kSoundFile[2L] )
   {
      assert( app->mPSysA.get() );

      app->mSmokeCountA--;

      if( app->mSmokeCountA )
         return;

      osg::Node*  node  = app->mPSysA->GetOSGNode();
      assert( node );

      node->setNodeMask( 0x00000000 );
   }

   if( fname == app->kSoundFile[3L] )
   {
      assert( app->mPSysC.get() );

      app->mSmokeCountC--;

      if( app->mSmokeCountC )
         return;

      osg::Node*  node  = app->mPSysC->GetOSGNode();
      assert( node );

      node->setNodeMask( 0x00000000 );
   }
}

int
main( int argc, const char* argv[] )
{
   SetDataFilePathList( "..;" + GetDeltaDataPathList() );

   TestAudioApp*  app   = new TestAudioApp( "config.xml" );
   assert( app );

   app->Run();

   delete   app;
   app   = NULL;

   return   0L;
}

