/*
 * $Id: PolygonModel.h 299 2015-08-07 14:57:10Z bernardt.duvenhage $
 */
/*
 *  PolygonModel.h
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

#ifndef STITCH_POLYGON_MODEL_H
#define STITCH_POLYGON_MODEL_H

namespace stitch {
	class PolygonModel;
}

#include "BallTree.h"
#include "Math/Line.h"
#include "Math/Plane.h"
#include "Math/VecN.h"
#include "Math/Colour.h"
#include "Materials/DiffuseMaterial.h"

#include <iostream>
#include <cstdio>


namespace stitch {
    
    //! One polygon with own vertices, vertex normals and plane.
    class Polygon : public Object
    {
    public:
        
		Polygon(Material * const pMaterial/*, NormalDescriptor * const pNormalDescriptor*/,
                size_t index0,
                std::vector<Vec3> *vertCoords,
                std::vector<Vec3> *vertNormals,
                std::vector<size_t> *indices) :
        Object(pMaterial/*, pNormalDescriptor*/),
        v0_((*vertCoords)[(*indices)[index0+0]]),
        v1_((*vertCoords)[(*indices)[index0+1]]),
        v2_((*vertCoords)[(*indices)[index0+2]]),
        n0_((*vertNormals)[(*indices)[index0+0]]),
        n1_((*vertNormals)[(*indices)[index0+1]]),
        n2_((*vertNormals)[(*indices)[index0+2]]),
        plane_(v0_,v1_,v2_),
        area_(stitch::Vec3::crossLength(v2_, v1_, v0_)*0.5f)
		{
            updateBoundingVolume();
        }
		
		Polygon(const Polygon &lValue) :
        Object(lValue),
        v0_(lValue.v0_),
        v1_(lValue.v1_),
        v2_(lValue.v2_),
        n0_(lValue.n0_),
        n1_(lValue.n1_),
        n2_(lValue.n2_),
        plane_(lValue.plane_),
        area_(lValue.area_)
		{}
        
#ifdef USE_CXX11
		Polygon(Polygon &&rValue) noexcept:
        Object(rValue),
        v0_(std::move(rValue.v0_)),
        v1_(std::move(rValue.v1_)),
        v2_(std::move(rValue.v2_)),
        n0_(std::move(rValue.n0_)),
        n1_(std::move(rValue.n1_)),
        n2_(std::move(rValue.n2_)),
        plane_(std::move(rValue.plane_)),
        area_(rValue.area_)
		{}
#endif// USE_CXX11
        
        virtual Polygon * clone() const
        {
            return new Polygon(*this);
        }
        
		virtual Polygon & operator = (const Polygon &lValue)
		{
            Object::operator=(lValue);
            
            v0_=lValue.v0_;
            v1_=lValue.v1_;
            v2_=lValue.v2_;
            
            n0_=lValue.n0_;
            n1_=lValue.n1_;
            n2_=lValue.n2_;
            
            plane_=lValue.plane_;
            area_=lValue.area_;
            
			return *this;
		}
        
#ifdef USE_CXX11
		virtual Polygon & operator = (Polygon &&rValue) noexcept
		{
            Object::operator=(rValue);
            
            v0_=std::move(rValue.v0_);
            v1_=std::move(rValue.v1_);
            v2_=std::move(rValue.v2_);
            
            n0_=std::move(rValue.n0_);
            n1_=std::move(rValue.n1_);
            n2_=std::move(rValue.n2_);
            
            plane_=std::move(rValue.plane_);
            area_=rValue.area_;
            
			return *this;
		}
#endif// USE_CXX11
        
        virtual ~Polygon()
		{
        }
		
#ifdef USE_OSG
		virtual osg::ref_ptr<osg::Node> constructOSGNode(const bool createOSGLineGeometry, const bool createOSGNormalGeometry, const bool wireframe, const uintptr_t key=0) const;
#endif// USE_OSG
        
		virtual void calcIntersection(const Ray &ray, Intersection &intersect) const;
        
        void updateBoundingVolume()
        {
            centre_=(v0_+v1_+v2_)/3.0f;
            
            radiusBV_=Vec3::calcDistToPointSq(v0_, centre_);
            radiusBV_=MathUtil::max(radiusBV_, Vec3::calcDistToPointSq(v1_, centre_));
            radiusBV_=MathUtil::max(radiusBV_, Vec3::calcDistToPointSq(v2_, centre_));
            radiusBV_=sqrtf(radiusBV_)*1.001f;//1% bigger to ensure rendering of full poly.
        }
        
        
    public:
        Vec3 v0_;
        Vec3 v1_;
        Vec3 v2_;
        
        Vec3 n0_;
        Vec3 n1_;
        Vec3 n2_;
        
        Plane plane_;
        float area_;
    };
    
    
    //! A ball tree of polygons PLUS fused list of vertices and indices.
	class PolygonModel : public Object
	{
	public:
		PolygonModel() :
        Object(new stitch::DiffuseMaterial(stitch::Colour_t(1.0f, 1.0f, 1.0f))),
        smoothSurface_(false)
        {
            ballTree_=new BallTree();
        }
        
        
        PolygonModel(Material * const pMaterial) :
        Object(pMaterial),
        smoothSurface_(false)
        {
            ballTree_=new BallTree();
        }
        
        PolygonModel(const PolygonModel &lValue) :
        Object(lValue),
        vertCoords_(lValue.vertCoords_),
        vertNormals_(lValue.vertNormals_),
        indices_(lValue.indices_),
        ballTree_(new BallTree(*lValue.ballTree_)),
        smoothSurface_(lValue.smoothSurface_)
        {}
        
#ifdef USE_CXX11
        PolygonModel(PolygonModel &&rValue) noexcept:
        Object(rValue),
        vertCoords_(std::move(rValue.vertCoords_)),
        vertNormals_(std::move(rValue.vertNormals_)),
        indices_(std::move(rValue.indices_)),
        ballTree_(rValue.ballTree_),
        smoothSurface_(rValue.smoothSurface_)
        {
            rValue.ballTree_=nullptr;
        }
#endif// USE_CXX11
        
        virtual PolygonModel * clone() const
        {
            return new PolygonModel(*this);
        }
        
        inline PolygonModel & operator = (const PolygonModel &lValue)
        {
            if (&lValue!=this)
            {
                Object::operator=(lValue);
                vertCoords_=lValue.vertCoords_;
                vertNormals_=lValue.vertNormals_;
                indices_=lValue.indices_;
                
                delete ballTree_;
                ballTree_=new BallTree(*lValue.ballTree_);
                
                smoothSurface_=lValue.smoothSurface_;
            }
            
            return *this;
        }
        
#ifdef USE_CXX11
        inline PolygonModel & operator = (PolygonModel &&rValue) noexcept
        {
            Object::operator=(rValue);
            vertCoords_=std::move(rValue.vertCoords_);
            vertNormals_=std::move(rValue.vertNormals_);
            indices_=std::move(rValue.indices_);
            
            delete ballTree_;
            ballTree_=rValue.ballTree_;
            rValue.ballTree_=nullptr;
            
            smoothSurface_=rValue.smoothSurface_;
            
            return *this;
        }
#endif// USE_CXX11
        
        
        //=== Vertex representation stuff ===//
        bool loadPLYVertices(const std::string fileName, const stitch::Vec3 &centre, const float scale, const bool invertNormals);
        
        bool loadOBJVertices(const std::string fileName, const stitch::Vec3 &centre, const float scale, const bool invertNormals);
        
        bool loadMDLVertices(const std::string fileName, const stitch::Vec3 &centre, const float scale, const bool invertNormals);
        
        bool loadTetrahedron(const stitch::Vec3 &centre, const float scale);
        
        
        //=== ===
        bool loadVectorsAndIndices(const std::vector<stitch::Vec3> &vectors,
                                   const std::vector<size_t> &indices,
                                   const stitch::Vec3 &centre, const float scale,
                                   const stitch::Vec3 &upVector,
                                   bool smoothNormals);
        //=== ===
        
        
        bool loadFibonacciSpiralSphere(const size_t numVertices, const stitch::Vec3 &centre, const float scale, bool smoothNormals);
        
        bool loadIcosahedronBasedSphere(const size_t minimumNumVertices, const stitch::Vec3 &centre, const float scale, bool smoothNormals);
        
        void calculateVertexNormals();
        
#ifdef USE_OSG
        virtual osg::ref_ptr<osg::Node> constructOSGNode(const bool createOSGLineGeometry, const bool createOSGNormalGeometry, const bool wireframe, const uintptr_t key=0) const;
#endif// USE_OSG
        
        //=== ===///
        
        
        //=== PolygonObject representation stuff ===//
        void generatePolygonObjectsFromVertices()
        {
            for (size_t index0=0; index0<indices_.size(); index0+=3)
            {
                Polygon *polygon=new Polygon(pMaterial_->clone()/*, pNormalDescriptor_->clone()*/, index0, &vertCoords_, &vertNormals_, &indices_);
                
                if (polygon->area_!=0.0f)
                {//Protect against needle and point polygons.
                    ballTree_->addItem(polygon);
                } else
                {
                    delete polygon;
                }
            }
            
            ballTree_->updateBV();
            updateBoundingVolume();
        }
        
        
        
        void buildBallTree(size_t chunkSize)
        {
            if (ballTree_->itemVector_.size()>chunkSize)
            {
                //std::cout << "Building ball tree...";
                //std::cout.flush();
                ballTree_->build(chunkSize, 0);
                //std::cout << "done.\n";
                //std::cout.flush();
            }
            
            ballTree_->updateBV();
            updateBoundingVolume();
        }
        
        virtual ~PolygonModel()
        {
            delete ballTree_;
        }
        
        virtual void calcIntersection(const Ray &ray, Intersection &intersect) const;
        
        
    public:
        std::vector<Vec3> vertCoords_;
        std::vector<Vec3> vertNormals_;
        std::vector<size_t> indices_;
        
    private:
        BallTree *ballTree_;//Internal ball tree for polygons.
        bool smoothSurface_;
        
        void updateBoundingVolume();
    };
    
}


#endif// STITCH_POLYGON_MODEL_H
