/*
 * $Id: Viewer.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  Viewer.cpp
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2009/09/17.
 *  Copyright $Date: 2015-03-25 15:00:40 +0200 (Wed, 25 Mar 2015) $ Bernardt Duvenhage. All rights reserved.
 *
 *
 *  This file is part of StitchEngine.
 
 *  StitchEngine is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 
 *  StitchEngine is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with StitchEngine.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "Viewer.h"

#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Quat>
#include <osg/Matrix>
#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Texture2D>

#include <osg/Light>
#include <osg/LightSource>
#include <osg/LightModel>

#include <osgDB/FileUtils>
#include <osgDB/ReadFile>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>
#include <osgGA/SphericalManipulator>

#include <osgGA/StateSetManipulator>

#include <iostream>


// class to handle events with a pick
class PickHandler : public osgGA::GUIEventHandler {
public: 
    
    PickHandler(osgGA::TrackballManipulator *manipulator):
    manipulator_(manipulator) {}
    
    virtual ~PickHandler() {}
    
    bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa);
    
    virtual void pick(osgViewer::View* view, const osgGA::GUIEventAdapter& ea);
    
protected:
    
    osg::ref_ptr<osgGA::TrackballManipulator> manipulator_;
};

bool PickHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
{
    if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT) 
    {
        switch(ea.getEventType())
        {
            case(osgGA::GUIEventAdapter::PUSH):
            {
                osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
                if (view) pick(view,ea);
                return false;
            }    
            default:
                return false;
        }
    }
    
    return false;
}

void PickHandler::pick(osgViewer::View* view, const osgGA::GUIEventAdapter& ea)
{
    osgUtil::LineSegmentIntersector::Intersections intersections;
    
    float x = ea.getX();
    float y = ea.getY();
    
    if (view->computeIntersections(x,y,intersections))
    {
        for(osgUtil::LineSegmentIntersector::Intersections::iterator hitr = intersections.begin();
            hitr != intersections.end();
            ++hitr)
        {
            osg::Vec3d eye, center, up;
            manipulator_->getTransformation(eye, center, up);            
            osg::Vec3d newCenter=hitr->getWorldIntersectPoint();
            manipulator_->setTransformation(eye, newCenter, up);            
            break; //from for-loop.
        }
    }
}




//=======================================================================//
stitch::View2D::View2D(const size_t i_imageWidth, const size_t i_imageHeight, uint8_t const * const i_imgData) :
m_imgData_(i_imgData), m_imageWidth_(i_imageWidth), m_imageHeight_(i_imageHeight)
{
    m_view_=nullptr;
	m_image_=nullptr;
}

//=======================================================================//
stitch::View2D::~View2D()
{
}

//=======================================================================//
void stitch::View2D::init(const size_t windowWidth, const size_t windowHeight)
{
    osg::ref_ptr<osg::Group> rootnode = new osg::Group;
    osg::ref_ptr<osg::Geode> geode = new osg::Geode();
	
    osg::ref_ptr<osg::DrawArrays> drawArrays = new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4);
    osg::ref_ptr<osg::Vec4Array> colourArray = new osg::Vec4Array;
    osg::ref_ptr<osg::Vec2Array> tcoordArray = new osg::Vec2Array;
    osg::ref_ptr<osg::Vec3Array> vertexArray = new osg::Vec3Array;
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet;
	
    colourArray->push_back(osg::Vec4f(1.0, 1.0, 1.0, 1.0));
    colourArray->push_back(osg::Vec4f(1.0, 1.0, 1.0, 1.0));
    colourArray->push_back(osg::Vec4f(1.0, 1.0, 1.0, 1.0));
    colourArray->push_back(osg::Vec4f(1.0, 1.0, 1.0, 1.0));
	
    vertexArray->push_back(osg::Vec3d(0, 0, 0));
    vertexArray->push_back(osg::Vec3d(m_imageWidth_, 0, 0));
    vertexArray->push_back(osg::Vec3d(m_imageWidth_, 0, m_imageHeight_));
    vertexArray->push_back(osg::Vec3d(0, 0, m_imageHeight_));
	
    tcoordArray->push_back(osg::Vec2(0, 1.0));
    tcoordArray->push_back(osg::Vec2(1.0, 1.0));
    tcoordArray->push_back(osg::Vec2(1.0, 0));
    tcoordArray->push_back(osg::Vec2(0, 0));
	
    geom->setVertexArray(vertexArray.get());
    geom->setTexCoordArray(0,tcoordArray.get());
    geom->addPrimitiveSet(drawArrays.get());
    geom->setColorArray(colourArray.get());
    geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    geode->addDrawable(geom.get());
    rootnode->addChild(geode.get());
	
    stateset = geom->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	
	
    m_image_ = new osg::Image;
    m_image_->setImage(m_imageWidth_, m_imageHeight_, 1, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, (uint8_t *)m_imgData_, osg::Image::NO_DELETE);
	
    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
    texture->setTextureSize(m_imageWidth_, m_imageHeight_);
    texture->setInternalFormat(GL_RGBA);
    texture->setResizeNonPowerOfTwoHint(false);
    texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
    texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
    texture->setImage(m_image_.get());
    stateset->setTextureAttributeAndModes(0, texture.get(), osg::StateAttribute::ON);
	
	
	m_view_=new osgViewer::View();
/*	
	//////	
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->x = 0;
    traits->y = 10;
    traits->width = 640;
    traits->height = 480;
    traits->windowDecoration = true;
    traits->doubleBuffer = true;
	
	traits->red=8;
	traits->green=8;
	traits->blue=8;
	traits->alpha=8;
	traits->depth=24;
	traits->stencil=8;
	
	traits->setInheritedWindowPixelFormat=false;
    
    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
    m_view_->getCamera()->setGraphicsContext(gc);
    m_view_->getCamera()->setViewport(new osg::Viewport(0,0,640,480));
	//////	
*/	
	m_view_->setUpViewInWindow(0, 0, windowWidth!=0 ? windowWidth : m_imageWidth_,
                                      windowHeight!=0 ? windowHeight : m_imageHeight_);
	
	m_view_->setSceneData(rootnode.get());
	
	//m_view_->addEventHandler(new osgViewer::StatsHandler);
	
    osgViewer::ScreenCaptureHandler *scHandler=new osgViewer::ScreenCaptureHandler;
    m_view_->addEventHandler(scHandler);
            
	
	if ((!m_view_->getCameraManipulator()) && (m_view_->getCamera()->getAllowEventFocus()))
	{
		osg::Camera *camera=m_view_->getCamera();
		camera->setProjectionMatrixAsOrtho(0.0, m_imageWidth_, 0.0, m_imageHeight_, 100.0, -100.0);
		camera->setViewMatrixAsLookAt(osg::Vec3d(0.0, -10.0, 0.0), osg::Vec3d(0.0, 0.0, 0.0), osg::Vec3d(0.0, 0.0, 1.0));
	}
    
    m_view_->getCamera()->setClearColor(osg::Vec4(0.0, 0.0, 0.0, 0.0));
        
    scHandler->setFramesToCapture(1);
    scHandler->setKeyEventTakeScreenShot('c');
    //scHandler->startCapture();
    //==========================//    
}


//=======================================================================//
void stitch::View2D::preframe()
{
	m_image_->dirty();
}





//=======================================================================//
stitch::View3D::View3D(const size_t i_imageWidth, const size_t i_imageHeight, osg::ref_ptr<osg::Group> i_rootGroup) :
m_rootGroup_(i_rootGroup), m_imageWidth_(i_imageWidth), m_imageHeight_(i_imageHeight)
{
    view_=nullptr;
    manipulator_=nullptr;
}

//=======================================================================//
stitch::View3D::~View3D()
{
}

//=======================================================================//
void stitch::View3D::init()
{
    osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet;
    stateset = m_rootGroup_->getOrCreateStateSet();
	
    stateset->setMode(GL_LIGHTING, osg::StateAttribute::ON);
    
		view_=new osgViewer::View();

    view_->addEventHandler(new osgGA::StateSetManipulator(view_->getCamera()->getOrCreateStateSet()) );
    
    //view_->addEventHandler(new osgViewer::ScreenCaptureHandler);
	
	view_->setUpViewInWindow(640, 10, m_imageWidth_, m_imageHeight_);
	
    //m_viewer_->addEventHandler(new osgViewer::WindowSizeHandler);
    
	view_->setSceneData(m_rootGroup_.get());
	
	view_->addEventHandler(new osgViewer::StatsHandler);
		
    // add the screen capture handler
    //view_->addEventHandler(new osgViewer::ScreenCaptureHandler);
    
    //==========================//

    
        
     //=== Camera light source ===//
     lightSource_ = new osg::LightSource;
     
     osg::Light* light = lightSource_->getLight();
     light->setLightNum(0);
     light->setPosition( osg::Vec4( 0.0f, 0.0f, 0.0f, 1.0f ) );
     light->setAmbient( osg::Vec4( 0.2f, 0.2f, 0.2f, 1.0f ) );
     light->setDiffuse( osg::Vec4( 0.9f, 0.9f, 0.9f, 1.0f ) );
     light->setSpecular( osg::Vec4( 0.0f, 0.0f, 0.0f, 1.0f ) );
     light->setConstantAttenuation(1.0); //1.0=No distance attenuation.
     
     lightSource_->setReferenceFrame(osg::LightSource::RELATIVE_RF);
     lightSource_->setLocalStateSetModes( osg::StateAttribute::ON );
     lightSource_->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON);
     
     osg::LightModel* lightModel = new osg::LightModel;
     lightModel->setTwoSided(true);
     lightModel->setLocalViewer(false);
     lightSource_->getOrCreateStateSet()->setAttribute(lightModel);
     
     m_rootGroup_->addChild(lightSource_);
     //=========================//
     
    manipulator_ = new osgGA::TrackballManipulator;
    view_->setCameraManipulator( manipulator_.get() );
    
    osg::Vec3d eye(0.0, 0.0, 30.0), center(0.0, 0.0, 0.0), up(0.0, 1.0, 0.0);
    manipulator_->setTransformation(eye, center, up);            
    
    view_->getCamera()->setClearColor(osg::Vec4(0.5*111.0/255.0, 0.5*166.0/255.0, 0.5*226.0/255.0, 0.0));
    
    // add the handler for doing the picking
    view_->addEventHandler(new PickHandler(manipulator_));
}


//=======================================================================//
void stitch::View3D::preframe()
{
    osg::Camera *cam=view_->getCamera();
    osg::Matrixd invViewMat=cam->getInverseViewMatrix();
    
    osg::Vec4d eyePos=osg::Vec4d(0.0, 0.0, 0.0, 1.0) * invViewMat;
    
    lightSource_->getLight()->setPosition(eyePos);
}