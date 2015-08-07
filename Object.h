/*
 * $Id: Object.h 299 2015-08-07 14:57:10Z bernardt.duvenhage $
 */
/*
 *  Object.h
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2011/02/22.
 *  Copyright $Date: 2015-08-07 16:57:10 +0200 (Fri, 07 Aug 2015) $ Bernardt Duvenhage. All rights reserved.
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

/*! \mainpage StitchEngine
 
 \section intro_sec Introduction
 The StitchEngine is a by-product of my ongoing computer graphics research. The
 main objective is to provide a running code base with examples to accompany my
 research outputs.
 
 \section install_sec Installation
 
 StitchEngine makes use of a CMakeLists.txt project description.
 
 
   StitchEngine is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
 
   StitchEngine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.
 
   You should have received a copy of the GNU Lesser General Public License
   along with StitchEngine.  If not, see <http://www.gnu.org/licenses/>.
 
*/

#ifndef STITCH_OBJECT_H
#define STITCH_OBJECT_H

namespace stitch {
	class Object;
    class Sphere;
}

#include "Math/Vec3.h"
#include "Math/Ray.h"
#include "BoundingVolume.h"
#include "Intersection.h"
#include "Material.h"

#include "OSGUtils/StitchOSG.h"

//! The namespace of StitchEngine.
namespace stitch {
    
    //! The abstract base object class. Inherits from BoundingVolume and adds an instance of a stitch::Material.
    class Object : public BoundingVolume
    {
    public:
        /*! Construct the base object class given the material. */
        Object(Material * const pMaterial);
        
        /*! Copy constructor. */
        Object(const Object &lValue);
        
#ifdef USE_CXX11
        /*! Move constructor. */
        Object(Object &&rValue) noexcept;
#endif// USE_CXX11

        virtual ~Object();
        
        /*! Assignment operator. */
		virtual Object & operator = (const Object &lValue);
        
#ifdef USE_CXX11
        /*! Move assignment operator. */
		virtual Object & operator = (Object &&rValue) noexcept;
#endif// USE_CXX11
        		      
    public:
        Material *pMaterial_;//! @todo Update the object and materials to have multiple/layered materials per object. In some cases it is preferred to have multiple single lobe meterial BSDFs.*/
    };
    
    
    /*! Simple concrete sphere object. */
    class Sphere : public Object
    {
    public:
        Sphere(Material * const pMaterial, const Vec3 centre, const float radius);
        
        /*! Copy constructor. */
        Sphere(const Sphere &lValue);
        
#ifdef USE_CXX11
        /*! Move constructor. */
        Sphere(Sphere &&rValue) noexcept;
#endif// USE_CXX11

        /*! Virtual constructor idiom. Clone operator. */
        virtual Sphere * clone() const
        {
            return new Sphere(*this);
        }

        virtual ~Sphere();
        
        /*! Assignment operator. */
		virtual Sphere & operator = (const Sphere &lValue);
        
#ifdef USE_CXX11
        /*! Move assignment operator. */
		virtual Sphere & operator = (Sphere &&rValue) noexcept;
#endif// USE_CXX11
        
#ifdef USE_OSG
        /*! Creates an OSG node that may be used to create a preview of the object.
         @param createOSGLineGeometry Boolean flag to indicate whether or not line geometry in addition to the polygon geometry should be created.
         @param createOSGNormalGeometry Boolean flag to indicate whether or not geometry to visualise the surface normal distributions should be created.
         @param wireframe Boolean flag to indicate whether or not the polygon geometry should be set to solid or wireframe.
         @return An osg node that may be used to preview the object.*/
		virtual osg::ref_ptr<osg::Node> constructOSGNode(const bool createOSGLineGeometry, const bool createOSGNormalGeometry, const bool wireframe, const uintptr_t key=0) const;
#endif// USE_OSG
        
		virtual void calcIntersection(const Ray &ray, Intersection &intersect) const;
    };
	
}


#endif// STITCH_OBJECT_H
