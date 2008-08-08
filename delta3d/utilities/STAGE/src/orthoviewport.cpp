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
* Matthew W. Campbell
*/
#include <prefix/dtstageprefix-src.h>
#include <QtGui/QMouseEvent>
#include <osg/Math>
#include <osg/CullSettings>
#include <dtEditQt/orthoviewport.h>
#include <dtEditQt/viewportoverlay.h>
#include <dtEditQt/editorevents.h>
#include <dtDAL/exceptionenum.h>
#include <dtDAL/transformableactorproxy.h>

namespace dtEditQt
{
    ///////////////////////////////////////////////////////////////////////////////
    IMPLEMENT_ENUM(OrthoViewport::OrthoViewType);
    const OrthoViewport::OrthoViewType OrthoViewport::OrthoViewType::TOP("TOP");
    const OrthoViewport::OrthoViewType OrthoViewport::OrthoViewType::FRONT("FRONT");
    const OrthoViewport::OrthoViewType OrthoViewport::OrthoViewType::SIDE("SIDE");
    ///////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////
    IMPLEMENT_ENUM(OrthoViewport::InteractionModeExt);
    const OrthoViewport::InteractionModeExt
        OrthoViewport::InteractionModeExt::CAMERA_PAN("CAMERA_PAN");
    const OrthoViewport::InteractionModeExt
        OrthoViewport::InteractionModeExt::CAMERA_ZOOM("CAMERA_ZOOM");
    const OrthoViewport::InteractionModeExt
        OrthoViewport::InteractionModeExt::ACTOR_AXIS_HORIZ("ACTOR_AXIS_HORIZ");
    const OrthoViewport::InteractionModeExt
        OrthoViewport::InteractionModeExt::ACTOR_AXIS_VERT("ACTOR_AXIS_VERT");
    const OrthoViewport::InteractionModeExt
        OrthoViewport::InteractionModeExt::ACTOR_AXIS_BOTH("ACTOR_AXIS_BOTH");
    const OrthoViewport::InteractionModeExt
        OrthoViewport::InteractionModeExt::NOTHING("NOTHING");
    ///////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////
    OrthoViewport::OrthoViewport(const std::string &name, QWidget *parent,
        QGLWidget *shareWith) :
            Viewport(ViewportManager::ViewportType::ORTHOGRAPHIC,name,parent,shareWith)
    {
        this->camera = new Camera();
        this->currentMode = &OrthoViewport::InteractionModeExt::NOTHING;
        setViewType(OrthoViewType::TOP,false);
    }

    ///////////////////////////////////////////////////////////////////////////////
    void OrthoViewport::initializeGL()
    {
        Viewport::initializeGL();

        //We do not want OSG to compute our near and far clipping planes when in
        //orthographic view
        getSceneView()->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

        //Default to wireframe view.
        setRenderStyle(Viewport::RenderStyle::WIREFRAME,false);
    }

    ///////////////////////////////////////////////////////////////////////////////
    void OrthoViewport::resizeGL(int width, int height)
    {
        double xDim = (double)width * 0.5;
        double yDim = (double)height * 0.5;

        getCamera()->makeOrtho(-xDim,xDim,-yDim,yDim,-5000.0,5000.0);
        Viewport::resizeGL(width,height);
    }

    ///////////////////////////////////////////////////////////////////////////////
    void OrthoViewport::setViewType(const OrthoViewType &type, bool refreshView)
    {
        if (type == OrthoViewType::TOP) 
        {
            this->viewType = &OrthoViewType::TOP;
            getCamera()->resetRotation();
            getCamera()->pitch(-90);
        }
        else if (type == OrthoViewType::FRONT) 
        {
            this->viewType = &OrthoViewType::FRONT;
            getCamera()->resetRotation();
        }
        else if (type == OrthoViewType::SIDE) 
        {
            this->viewType = &OrthoViewType::SIDE;
            getCamera()->resetRotation();
            getCamera()->yaw(90);
        }

        if (refreshView)
        {
            if (!isInitialized())
                throw dtUtil::Exception(dtDAL::ExceptionEnum::BaseException,"Cannot refresh the viewport. "
                       "It has not been initialized.", __FILE__, __LINE__);
            refresh();
        }
    }

    ///////////////////////////////////////////////////////////////////////////////
    void OrthoViewport::mousePressEvent(QMouseEvent *e)
    {
        setFocus();
        Qt::KeyboardModifiers shiftAndControl = Qt::ControlModifier | Qt::ShiftModifier;

        if (getInteractionMode() == Viewport::InteractionMode::CAMERA) {
            if (getEnableKeyBindings()) {
                if (e->modifiers() == Qt::ShiftModifier) {
                    setActorTranslateMode();
                    beginActorMode(e);
                }
                else if (e->modifiers() == Qt::AltModifier) {
                    setActorRotateMode();
                    beginActorMode(e);
                }
                else if (e->modifiers() == Qt::ControlModifier) {
                    setActorSelectMode();
                    ViewportManager::GetInstance().getViewportOverlay()->setMultiSelectMode(false);
                    if (e->button() == Qt::LeftButton)
                        pick(e->pos().x(),e->pos().y());
                }
                else if (e->modifiers() == shiftAndControl) {
                    setActorSelectMode();
                    ViewportManager::GetInstance().getViewportOverlay()->setMultiSelectMode(true);
                    if (e->button() == Qt::LeftButton)
                        pick(e->pos().x(),e->pos().y());
                    ViewportManager::GetInstance().getViewportOverlay()->setMultiSelectMode(false);
                }
                else {
                    beginCameraMode(e);
                }
            }
            else {
                beginCameraMode(e);
            }
        }
        else if (getInteractionMode() == Viewport::InteractionMode::SELECT_ACTOR) {
            if (e->modifiers() == Qt::ControlModifier)
                ViewportManager::GetInstance().getViewportOverlay()->setMultiSelectMode(true);
            else
                ViewportManager::GetInstance().getViewportOverlay()->setMultiSelectMode(false);

            if (e->button() == Qt::LeftButton)
                pick(e->pos().x(),e->pos().y());
        }
        else {
            beginActorMode(e);
        }
    }

    ///////////////////////////////////////////////////////////////////////////////
    void OrthoViewport::mouseReleaseEvent(QMouseEvent *e)
    {
        if (getInteractionMode() != Viewport::InteractionMode::SELECT_ACTOR) {
            if (getInteractionMode() == Viewport::InteractionMode::CAMERA)
               endCameraMode(e);
            else
                endActorMode(e);
        }
        else
            syncWithModeActions();
    }

    ///////////////////////////////////////////////////////////////////////////////
    void OrthoViewport::onMouseMoveEvent(QMouseEvent *e, float dx, float dy)
    {
      if (getInteractionMode() == Viewport::InteractionMode::SELECT_ACTOR)
      {
          return;
      }
      else if (getInteractionMode() == Viewport::InteractionMode::CAMERA)
      {
          if (*this->currentMode == InteractionModeExt::NOTHING || getCamera() == NULL)
              return;

          moveCamera(dx,dy);
      }
      else if (getInteractionMode() == Viewport::InteractionMode::TRANSLATE_ACTOR)
      {
          translateCurrentSelection(e,dx,dy);
      }
      else if (getInteractionMode() == Viewport::InteractionMode::ROTATE_ACTOR)
      {
          rotateCurrentSelection(e,dx,dy);
      }
    }

    ///////////////////////////////////////////////////////////////////////////////
    void OrthoViewport::moveCamera(float dx, float dy)
    {
        float xAmount = (-dx/getMouseSensitivity()*4.0f) / getCamera()->getZoom();
        float yAmount = (dy/getMouseSensitivity()*4.0f) / getCamera()->getZoom();

        if (*this->currentMode == OrthoViewport::InteractionModeExt::CAMERA_PAN) {
            getCamera()->move(getCamera()->getRightDir() * xAmount);
            getCamera()->move(getCamera()->getUpDir() * yAmount);
        }
        else if (*this->currentMode == OrthoViewport::InteractionModeExt::CAMERA_ZOOM) {
            osg::Vec3 moveVec = this->zoomToPosition-getCamera()->getPosition();

            moveVec.normalize();
            if (dy <= -1.0f) {
                getCamera()->zoom(1.1f);
                //getCamera()->move(getCamera()->getRightDir()*
            }
            else if (dy >= 1.0f) {
                getCamera()->zoom(0.9f);
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////
    void OrthoViewport::wheelEvent(QWheelEvent *e)
    {
         if (e->delta() > 0)
            getCamera()->zoom(1.3f);
         else
            getCamera()->zoom(0.7f);
//
//         osg::Vec3 nearPoint,farPoint;
//         int xLoc = e->pos().x();
//         int yLoc = getSceneView()->getViewport()->height()-e->pos().y();
//         getSceneView()->projectWindowXYIntoObject(xLoc,yLoc,nearPoint,farPoint);

        //double s = exp( e->delta( ) / 4 * c_sizeFactor );
        //double deltaX = screenToInternalX( e->x( ) );
        //double deltaY = screenToInternalY( e->y( ) );
        //double c = 1.0 / ( m_dScale * s ) - 1.0 / m_dScale;
        //m_dTransX += deltaX * c;
        //m_dTransY += deltaY * c;
        //m_dScale *= s;


//         std::cout << "Zoom: " << getCamera()->getZoom() << std::endl;
//         std::cout << "ZoomTo: " << nearPoint << std::endl;
//         std::cout << "Current Location: " << getCamera()->getPosition() << std::endl;
//         std::cout << "Current View Dir: " << getCamera()->getViewDir() << std::endl << std::endl;
        refresh();
    }

    ///////////////////////////////////////////////////////////////////////////////
    void OrthoViewport::beginCameraMode(QMouseEvent *e)
    {
        if (e->button() == Qt::LeftButton) {
            this->currentMode = &OrthoViewport::InteractionModeExt::CAMERA_PAN;
        }
        else if (e->button() == Qt::RightButton) {
            osg::Vec3 nearPoint,farPoint;
            int xLoc = e->pos().x();
            int yLoc = int(getSceneView()->getViewport()->height()-e->pos().y());
            getSceneView()->projectWindowXYIntoObject(xLoc,yLoc,nearPoint,farPoint);
            this->zoomToPosition = nearPoint;
            this->currentMode = &OrthoViewport::InteractionModeExt::CAMERA_ZOOM;
        }

        setCameraMode();
        trapMouseCursor();
    }

    ///////////////////////////////////////////////////////////////////////////////
    void OrthoViewport::endCameraMode(QMouseEvent *e)
    {
        this->currentMode = &OrthoViewport::InteractionModeExt::NOTHING;
        releaseMouseCursor();
        syncWithModeActions();
    }

    ///////////////////////////////////////////////////////////////////////////////
    void OrthoViewport::beginActorMode(QMouseEvent *e)
    {
        Qt::MouseButtons bothButtons = Qt::LeftButton | Qt::RightButton;

        if (e->buttons() == bothButtons || e->buttons() == Qt::MidButton) {
            this->currentMode = &InteractionModeExt::ACTOR_AXIS_VERT;
        }
        else if (e->button() == Qt::LeftButton) {
            this->currentMode = &InteractionModeExt::ACTOR_AXIS_BOTH;
        }
        else if (e->button() == Qt::RightButton) {
            this->currentMode = &InteractionModeExt::ACTOR_AXIS_HORIZ;
        }
        else {
            this->currentMode = &InteractionModeExt::NOTHING;
            return;
        }

        // Save the original values of trans/rotate so undo/redo can track it.
        if (getInteractionMode() == Viewport::InteractionMode::TRANSLATE_ACTOR)
            saveSelectedActorOrigValues(dtDAL::TransformableActorProxy::PROPERTY_TRANSLATION);
        else if (getInteractionMode() == Viewport::InteractionMode::ROTATE_ACTOR)
            saveSelectedActorOrigValues(dtDAL::TransformableActorProxy::PROPERTY_ROTATION);

        trapMouseCursor();
    }

    ///////////////////////////////////////////////////////////////////////////////
    void OrthoViewport::endActorMode(QMouseEvent *e)
    {
        if (e->button() == Qt::LeftButton && e->buttons() == Qt::RightButton) {
            this->currentMode = &InteractionModeExt::ACTOR_AXIS_HORIZ;
        }
        else if (e->button() == Qt::RightButton && e->buttons() == Qt::LeftButton) {
            this->currentMode = &InteractionModeExt::ACTOR_AXIS_BOTH;
        }
        else {
            this->currentMode = &InteractionModeExt::NOTHING;
            releaseMouseCursor();

            // we could send hundreds of translation and rotation events, so make sure
            // we surround it in a change transaction
            EditorEvents::GetInstance().emitBeginChangeTransaction();

            //Update the selected actor proxies with their new values.
            if (getInteractionMode() == Viewport::InteractionMode::TRANSLATE_ACTOR)
                updateActorSelectionProperty(dtDAL::TransformableActorProxy::PROPERTY_TRANSLATION);
            else if (getInteractionMode() == Viewport::InteractionMode::ROTATE_ACTOR)
                updateActorSelectionProperty(dtDAL::TransformableActorProxy::PROPERTY_ROTATION);

            EditorEvents::GetInstance().emitEndChangeTransaction();

            //If a modifier key was pressed the current interaction mode was
            //temporarily overridden, so make sure we restore the previous mode.
            syncWithModeActions();
        }
    }

    ///////////////////////////////////////////////////////////////////////////////
    void OrthoViewport::translateCurrentSelection(QMouseEvent *e, float dx, float dy)
    {
        osg::Vec3 trans;
        ViewportOverlay::ActorProxyList::iterator itor;
        ViewportOverlay::ActorProxyList &selection =
                ViewportManager::GetInstance().getViewportOverlay()->getCurrentActorSelection();

        float xAmount = (dx/getMouseSensitivity()*4.0f) / getCamera()->getZoom();
        float yAmount = (-dy/getMouseSensitivity()*4.0f) / getCamera()->getZoom();

        //Actors are translated along the camera's view plane.
        if (*this->currentMode == InteractionModeExt::ACTOR_AXIS_HORIZ)
            trans = getCamera()->getRightDir() * xAmount;
        else if (*this->currentMode == InteractionModeExt::ACTOR_AXIS_VERT)
            trans = getCamera()->getUpDir() * yAmount;
        else if (*this->currentMode == InteractionModeExt::ACTOR_AXIS_BOTH) {
            trans = getCamera()->getRightDir() * xAmount;
            trans += getCamera()->getUpDir() * yAmount;
        }

        osg::Vec3 currTrans;
        for (itor=selection.begin(); itor!=selection.end(); ++itor) {
            dtDAL::ActorProxy *proxy = const_cast<dtDAL::ActorProxy *>(itor->get());
            dtDAL::TransformableActorProxy *tProxy =
                dynamic_cast<dtDAL::TransformableActorProxy *>(proxy);

            if (tProxy != NULL)
            {
                currTrans = tProxy->GetTranslation();
                currTrans += trans;
                tProxy->SetTranslation(currTrans);
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////
    void OrthoViewport::rotateCurrentSelection(QMouseEvent *e, float dx, float dy)
    {
        ViewportOverlay::ActorProxyList &selection =
                ViewportManager::GetInstance().getViewportOverlay()->getCurrentActorSelection();
        ViewportOverlay::ActorProxyList::iterator itor;

        for (itor=selection.begin(); itor!=selection.end(); ++itor) {
            dtDAL::ActorProxy *proxy = const_cast<dtDAL::ActorProxy *>(itor->get());
            dtDAL::TransformableActorProxy *tProxy =
                    dynamic_cast<dtDAL::TransformableActorProxy *>(proxy);

            if (tProxy != NULL)
            {
                osg::Vec3 hpr = tProxy->GetRotation();
                float delta = dy / getMouseSensitivity();

                if (*this->viewType == OrthoViewType::TOP) {
                    if (*this->currentMode == InteractionModeExt::ACTOR_AXIS_BOTH)
                        hpr.z() += delta;
                    if (*this->currentMode == InteractionModeExt::ACTOR_AXIS_HORIZ)
                        hpr.x() += delta;
                    if (*this->currentMode == InteractionModeExt::ACTOR_AXIS_VERT)
                        hpr.y() += delta;
                }
                else if (*this->viewType == OrthoViewType::SIDE) {
                    if (*this->currentMode == InteractionModeExt::ACTOR_AXIS_BOTH)
                        hpr.x() += delta;
                    if (*this->currentMode == InteractionModeExt::ACTOR_AXIS_HORIZ)
                        hpr.y() += delta;
                    if (*this->currentMode == InteractionModeExt::ACTOR_AXIS_VERT)
                        hpr.z() += delta;
                }
                else if (*this->viewType == OrthoViewType::FRONT) {
                    if (*this->currentMode == InteractionModeExt::ACTOR_AXIS_BOTH)
                        hpr.y() += delta;
                    if (*this->currentMode == InteractionModeExt::ACTOR_AXIS_HORIZ)
                        hpr.x() += delta;
                    if (*this->currentMode == InteractionModeExt::ACTOR_AXIS_VERT)
                        hpr.z() += delta;
                }

                tProxy->SetRotation(hpr);
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////
    void OrthoViewport::warpWorldCamera(int x, int y)
    {

    }

}
