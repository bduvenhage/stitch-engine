/*
 * $Id: Viewer.h 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  Viewer.h
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

#ifndef PHOTO3D_VIEWER_H
#define PHOTO3D_VIEWER_H

namespace stitch {
	class View2D;
	class View3D;
}

#include "Camera.h"

#include <osgGA/TrackballManipulator>
#include <osgViewer/View>

namespace stitch
{
    class View2D
    {
    public:
        View2D(const size_t i_imageWidth, const size_t i_imageHeight, uint8_t const * const i_imgData);
        virtual ~View2D();
        
 		virtual void init(const size_t windowWidth=0, const size_t windowHeight=0);
        
        virtual void preframe();
                
        virtual osgViewer::View* getOSGView()
        {
            return m_view_.get();
        }
        
    private:
        uint8_t const * const m_imgData_;//m_imgData_ is not owned by View2D!
        
        size_t m_imageWidth_;
        size_t m_imageHeight_;
        
		osg::ref_ptr<osgViewer::View> m_view_;
		osg::ref_ptr<osg::Image> m_image_;
    };
	
    
    class View3D
    {
    public:
        View3D(const size_t i_imageWidth, const size_t i_imageHeight, osg::ref_ptr<osg::Group> i_rootGroup);
        virtual ~View3D();
		
 		virtual void init();
        virtual void preframe();
		        
        virtual stitch::SimplePinholeCamera getCamera()
        {
            osg::Camera *cam=view_->getCamera();
            osg::Matrixd invViewMat=cam->getInverseViewMatrix();
            
            osg::Vec4d eyePos=osg::Vec4d(0.0, 0.0, 0.0, 1.0) * invViewMat;
            osg::Vec4d centrePos=osg::Vec4d(0.0, 0.0, -100.0, 1.0) * invViewMat;
            osg::Vec4d upVec=osg::Vec4d(0.0, 1.0, 0.0, 0.0) * invViewMat;
            
            return stitch::SimplePinholeCamera(stitch::Vec3(eyePos._v[0], eyePos._v[1], eyePos._v[2]), 
                                               stitch::Vec3(centrePos._v[0], centrePos._v[1], centrePos._v[2]), stitch::Vec3(upVec._v[0], upVec._v[1], upVec._v[2]));
        }
        
        virtual void setCamera(Vec3 eyePos, Vec3 centrePos, Vec3 upVec)
        {
            manipulator_->setTransformation(
                                            osg::Vec3d(eyePos.x(),eyePos.y(),eyePos.z()),
                                            osg::Vec3d(centrePos.x(),centrePos.y(),centrePos.z()),
                                            osg::Vec3d(upVec.x(),upVec.y(),upVec.z())
                                            );
        }
        
        virtual osgViewer::View* getOSGView()
        {
            return view_.get();
        }
		
    private:
		osg::ref_ptr<osg::Group> m_rootGroup_;
        size_t m_imageWidth_;
        size_t m_imageHeight_;
		
		osg::ref_ptr<osgViewer::View> view_;
        osg::ref_ptr<osgGA::TrackballManipulator> manipulator_;
        osg::ref_ptr<osg::LightSource> lightSource_;
    };
}

#endif