/*
 * $Id: StitchOSG.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  StitchOSG.cpp
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


#include "Objects/PolygonModel.h"
#include "Materials/DiffuseMaterial.h"
#include "StitchOSG.h"


//=======================================================================//
#ifdef USE_OSG
osg::ref_ptr<osg::Node> stitch::constructOSGNode_Sphere(const stitch::VecN &centre, const float radius, const bool wireframe, uintptr_t key)
{
    //Use the icosahedron code in PolygonModel.
    
    stitch::PolygonModel *polygonModel=new stitch::PolygonModel(new stitch::DiffuseMaterial(stitch::Colour_t(1.0, 1.0, 1.0)));
    polygonModel->loadIcosahedronBasedSphere(162, Vec3(centre), radius, true);//There are 12 vertices in a base icosahedron.
    //polygonModel->loadFibonacciSpiralSphere(162, centre, radius);
    
    polygonModel->generatePolygonObjectsFromVertices();
    
    osg::ref_ptr<osg::Node> node=polygonModel->constructOSGNode(false, false, wireframe, key);
    
    delete polygonModel;
    
    return node;
}
#endif// USE_OSG
