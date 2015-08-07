/*
 * $Id: BrushModel.h 299 2015-08-07 14:57:10Z bernardt.duvenhage $
 */
/*
 *  Brush.h
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2009/09/17.
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

#ifndef STITCH_BRUSH_H
#define STITCH_BRUSH_H

namespace stitch {
	class Brush;
}

#include "Math/Vec3.h"
#include "Math/Plane.h"
#include "Math/Line.h"
#include "EntryExit.h"
#include "Material.h"
#include "Object.h"
#include "BallTree.h"

namespace stitch {
    
    //! One face of a k-DOP brush.
    class BrushFace
    {
    public:
        BrushFace(Plane plane, bool hidden) :
        plane_(plane), hidden_(hidden), area_(0.0f)
        {}
        
        virtual ~BrushFace() {}
        
        BrushFace(const BrushFace &lValue) :
        plane_(lValue.plane_),
        vertexCoordVector_(lValue.vertexCoordVector_),
        //vertexNormalVector_(lValue.vertexNormalVector_),
        lineVector_(lValue.lineVector_),
        hidden_(lValue.hidden_), area_(lValue.area_)
        {}
        
#ifdef USE_CXX11
        BrushFace(BrushFace &&rValue) noexcept:
        plane_(std::move(rValue.plane_)),
        vertexCoordVector_(std::move(rValue.vertexCoordVector_)),
        //vertexNormalVector_(std::move(rValue.vertexNormalVector_)),
        lineVector_(std::move(rValue.lineVector_)),
        hidden_(rValue.hidden_), area_(rValue.area_)
        {}
#endif// USE_CXX11
        
        inline BrushFace & operator=(const BrushFace &lValue)
        {
            plane_=lValue.plane_;
            vertexCoordVector_=lValue.vertexCoordVector_;
            //vertexNormalVector_=lValue.vertexNormalVector_;
            lineVector_=lValue.lineVector_;
            hidden_=lValue.hidden_;
            area_=lValue.area_;
            
            return (*this);
        }
        
#ifdef USE_CXX11
        inline BrushFace & operator=(BrushFace &&rValue) noexcept
        {
            plane_=std::move(rValue.plane_);
            vertexCoordVector_=std::move(rValue.vertexCoordVector_);
            //vertexNormalVector_=std::move(rValue.vertexNormalVector_);
            lineVector_=std::move(rValue.lineVector_);
            hidden_=rValue.hidden_;
            area_=rValue.area_;
            
            return (*this);
        }
#endif// USE_CXX11
        
        Plane plane_;
        
		//Members to define non-infinite plane.
		std::vector<stitch::Vec3> vertexCoordVector_;
		//std::vector<stitch::Vec3> vertexNormalVector_;
		std::vector<stitch::Line> lineVector_;
        
        bool hidden_;
        float area_;
    };
    
    
    //! A brush is a k-DOP.
	class Brush : public Object
	{
	public:
		Brush(Material * const pMaterial);
        
        /*! Constructs a k-sided discrete oriented polytope (kDOP) bounding volume with at least minNumBrushFaces planes around the supplied vertex cloud. Some plane may be degenerate. */
		Brush(Material * const pMaterial, std::vector<stitch::Vec3> const &vertexCloud, const size_t minNumBrushFaces);
		
		Brush(const Brush &lValue);
        
#ifdef USE_CXX11
		Brush(Brush &&rValue) noexcept;
#endif// USE_CXX11
        
        /*! Virtual constructor idiom. Clone operator. */
        virtual Brush * clone() const
        {
            return new Brush(*this);
        }
        
		virtual Brush & operator=(const Brush &lValue);
        
#ifdef USE_CXX11
		virtual Brush & operator=(Brush &&rValue) noexcept;
#endif// USE_CXX11
        
		virtual ~Brush();
		
		void addFace(const BrushFace &face);
		
        /*! Merge brush A and B into one Brush. Use material from brushA.*/
        static Brush mergeBrush(const Brush &brushA, const Brush &brushB, const bool trashLineGeometry);
		
        void updateLinesVerticesAndBoundingVolume(const bool trashLineGeometry);
        
        /*! Get a vector of vertices that define the brush. There might be duplication of vertices. */
        std::vector<stitch::Vec3> getVertexCloud() const
        {
            std::vector<stitch::Vec3> vertexCloud;
            
            std::vector<BrushFace>::const_iterator faceIter=faceVector_.begin();
            const std::vector<BrushFace>::const_iterator faceIterEnd=faceVector_.end();
            
            for (; faceIter!=faceIterEnd; ++faceIter)
            {
                vertexCloud.insert(vertexCloud.end(), faceIter->vertexCoordVector_.begin(), faceIter->vertexCoordVector_.end());
            }
            
            return vertexCloud;
        }
        
#ifdef USE_OSG
		virtual osg::ref_ptr<osg::Node> constructOSGNode(const bool createOSGLineGeometry, const bool createOSGNormalGeometry, const bool wireframe, const uintptr_t key=0) const;
#endif// USE_OSG
        
		virtual void calcIntersection(const Ray &ray, Intersection &intersect) const;
		
        /*! Optimise the face order for pointInBrush and intersection operations. */
        void optimiseFaceOrder();
        
        /*! Test whether or not a point is within the glossy beam's bounding volume. */
        inline bool pointInBrush(const Vec3 &point) const
        {
            if (pointInBV(point))//This initial ball BV check possibly does not contribute much towards performance.
            {
                bool inBrush=true;
                
#ifdef USE_CXX11
                    for (const auto &face : faceVector_)
                    {
                        const stitch::Plane &plane=face.plane_;
#else
                    const size_t numFaces=faceVector_.size();
                    for (size_t faceNum=0; faceNum<numFaces; ++faceNum)
                    {
                        const stitch::Plane &plane=(faceVector_[faceNum].plane_);
#endif
                        if ((plane.normal_ * point) > plane.d_)
                        {
                            inBrush=false;
                            break;//from for-loop
                        }
                    }
                    
                    return inBrush;
                } else
                {
                    return false;
                }
            }
            
            
            const BrushFace & getFaceByConstRef(const size_t faceNum) const
            {
                return faceVector_[faceNum];
            }
            
            BrushFace & getFaceByRef(const size_t faceNum)
            {
                return faceVector_[faceNum];
            }
            
            size_t getNumFaces() const
            {
                return faceVector_.size();
            }
            
        private:
            //!
            std::vector<BrushFace> faceVector_;
            
        private:
#ifdef USE_OSG
            virtual osg::ref_ptr<osg::Node> constructOSGLineNode() const;
            virtual osg::ref_ptr<osg::Node> constructOSGNormalNode() const;
#endif// USE_OSG
            
        };
        
        
        
        //! A ball tree of brushes.
        class BrushModel : public Object
        {
        public:
            BrushModel(Material * const pMaterial) :
            Object(pMaterial)
            {
                ballTree_=new BallTree();
            }
            
            
            BrushModel(const BrushModel &lValue) :
            Object(lValue),
            ballTree_(new BallTree(*lValue.ballTree_))
            {}
            
#ifdef USE_CXX11
            BrushModel(BrushModel &&rValue) noexcept:
            Object(rValue),
            ballTree_(rValue.ballTree_)
            {
                rValue.ballTree_=nullptr;
            }
#endif// USE_CXX11
            
            /*! Virtual constructor idiom. Clone operator. */
            virtual BrushModel * clone() const
            {
                return new BrushModel(*this);
            }
            
            inline BrushModel & operator = (const BrushModel &lValue)
            {
                if (&lValue!=this)
                {
                    Object::operator=(lValue);
                    
                    delete ballTree_;
                    ballTree_=new BallTree(*lValue.ballTree_);
                }
                return *this;
            }
            
#ifdef USE_CXX11
            inline BrushModel & operator = (BrushModel &&rValue) noexcept
            {
                Object::operator=(rValue);
                
                delete ballTree_;
                ballTree_=rValue.ballTree_;
                rValue.ballTree_=nullptr;
                return *this;
            }
#endif// USE_CXX11
            
            void addBrush(Brush *brush)
            {
                ballTree_->addItem(brush);//This tree takes ownership of the added brush!
                ballTree_->updateBV();
                updateBoundingVolume();
            }
            
            void buildBallTree(size_t chunkSize)
            {
                if (ballTree_->itemVector_.size()>chunkSize)
                {
                    ballTree_->build(chunkSize, 0);
                }
                
                ballTree_->updateBV();
                updateBoundingVolume();
            }
            
            virtual ~BrushModel()
            {
                delete ballTree_;
            }
            
            
#ifdef USE_OSG
            virtual osg::ref_ptr<osg::Node> constructOSGNode(const bool createOSGLineGeometry, const bool createOSGNormalGeometry, const bool wireframe, const uintptr_t key=0) const;
#endif// USE_OSG
            
            virtual void calcIntersection(const Ray &ray, Intersection &intersect) const;
            
        private:
            BallTree *ballTree_;
            
            void updateBoundingVolume()
            {
                ballTree_->updateBV();
                
                this->centre_=ballTree_->centre_;
                this->radiusBV_=ballTree_->radiusBV_;
            }
            
        };
    }
    
    
#endif// STITCH_BRUSH_H
