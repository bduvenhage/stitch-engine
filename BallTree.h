/*
 * $Id: BallTree.h 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  BallTree.h
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

#ifndef STITCH_BALL_TREE_H
#define STITCH_BALL_TREE_H

//#define USE_AXIS_ALIGNED_ITEM_SPLIT_PLANES 1

namespace stitch {
	class BallTree;
}

#include "BoundingVolume.h"
#include "Math/Ray.h"
#include "OSGUtils/StitchOSG.h"

namespace stitch {
	
	//! Implements a ball tree acceleration structure of BoundingVolumes.
	class BallTree : public BoundingVolume
	{
	public:
		BallTree();
        
        BallTree(const BallTree &lValue);
		
        virtual ~BallTree();
		
        /*! Virtual constructor idiom. Clone operator. */
        virtual BallTree * clone() const//Uses the copy constructor.
        {
            return new BallTree(*this);
        }
        
		inline void addItem(BoundingVolume * const item)
        {
            itemVector_.push_back(item);
        }
        
        inline bool empty() const
        {
            return (getNumItems() == 0);
        }
        
        /*! Get the number of items in the tree. */
        size_t getNumItems() const;
        
        void clear();
        
        void linearise();
        
        /*! Build/update the tree structure with the items in the itemVector. */
        void build(const size_t chunkSize, const uint8_t splitAxis);
        
        void updateBV();
        
        virtual void calcIntersection(const Ray &ray, Intersection &intersect) const;
        
#ifdef USE_OSG
		virtual osg::ref_ptr<osg::Node> constructOSGNode(const bool createOSGLineGeometry, const bool createOSGNormalGeometry, const bool wireframe, const uintptr_t key=0) const;
                
        virtual osg::ref_ptr<osg::Node> constructOSGBVNode() const;
#endif// USE_OSG
        
        
    public:
        std::vector<stitch::BoundingVolume *> itemVector_;
        std::vector<stitch::BallTree *> ballTreeVector_;
        
	};
	
}


#endif// STITCH_BALL_TREE_H
