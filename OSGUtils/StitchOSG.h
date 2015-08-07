/*
 * $Id: StitchOSG.h 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  StitchOSG.h
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2011/04/26.
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

#ifndef STITCH_OSG_H
#define STITCH_OSG_H

#include "Math/VecN.h"
#include "Math/Colour.h"

#ifdef USE_OSG

#include "Viewer.h"

#include <OpenThreads/Thread>
#include <OpenThreads/Mutex>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Geometry>
#include <osg/Image>
#include <osg/Material>
#include <osg/Point>
#include <osg/PolygonMode>
#include <osg/Timer>
#include <osg/Vec3f>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgGA/GUIEventHandler>
#include <osgUtil/Optimizer>
#include <osgViewer/CompositeViewer>


namespace stitch {
	osg::ref_ptr<osg::Node> constructOSGNode_Sphere(const VecN &centre, const float radius, const bool wireframe, uintptr_t key=0);
}

#endif //USE_OSG

#endif //STITCH_OSG_H
