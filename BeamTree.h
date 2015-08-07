/*
 * $Id: BeamTree.h 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  BeamTree.h
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

#ifndef STITCH_BEAMTREE_H
#define STITCH_BEAMTREE_H

namespace stitch {
	class BeamTree;
}

#include "Math/Vec3.h"
#include "Math/Colour.h"
#include "Beam.h"
#include "Objects/BrushModel.h"

#include <iostream>
#ifdef USE_CXX11
#include <cstdint>
#else
#include <stdint.h>
#endif

namespace stitch {
    
    //! Implements a hierarchy of beam segments acceleration structure.
    class BeamTree
    {
    public:
        BeamTree();
        
        /*! Copy constructor. */
        BeamTree(const BeamTree &lValue);
        
        /*! Assignment operator. */
        BeamTree & operator=( const BeamTree &lValue);
        
        ~BeamTree();
        
		void addBeamSegment(BeamSegment * const beamSegment);
        
        /*! Return wether or not the tree is empty. */
        inline bool empty() const
        {
            return (getNumBeamSegments() == 0);
        }
        
        /*! Get the number of objects in the tree. */
        size_t getNumBeamSegments() const;
                
        void clear();
        
        /*! Collect all items into a linear list and delete the tree structure. */
        inline void linearise()
        {
            std::cout << "BeamTree::linearise not implemented yet!\n";
            std::cout.flush();
            exit(-1);
        }
        
        /*! Build/update the tree structure with the beam segment in the beamSegmentVector. */
        void build(const size_t chunkSize, const uint8_t splitAxis);
        
        
        //=======================================================================//
        void updateBV();
        
        /*! Test whether or not a point is within the tree of glossy beam segments' bounding volume. */
        inline bool pointInBV(const Vec3 &point) const
        {
            return BVBrush_.pointInBrush(point);
        }
        
        //=======================================================================//
        //=======================================================================//
        /*! Get the list of flux densities and approximate flux directions that contribute to a point in world space. */
        inline void getSpecContributingBeamSegmentVector(const Vec3 &worldPosition, std::vector<stitch::BeamSegment const *> & contribBeamSegmentVector) const
        {
            if (beamTreeVector_.size()>0)
            {
                std::vector<stitch::BeamTree *>::const_iterator beamTreeIter=beamTreeVector_.begin();
                const std::vector<stitch::BeamTree *>::const_iterator beamTreeIterEnd=beamTreeVector_.end();
                for (; beamTreeIter!=beamTreeIterEnd; ++beamTreeIter)
                {
                    if ((*beamTreeIter)->pointInBV(worldPosition))
                    {
                        (*beamTreeIter)->getSpecContributingBeamSegmentVector(worldPosition, contribBeamSegmentVector);
                    }
                }
            } else
            {
                std::vector<stitch::BeamSegment *>::const_iterator beamSegmentIter=beamSegmentVector_.begin();
                const std::vector<stitch::BeamSegment *>::const_iterator beamSegmentIterEnd=beamSegmentVector_.end();
                for (; beamSegmentIter!=beamSegmentIterEnd; ++beamSegmentIter)
                {
                    stitch::BeamSegment const * const beamSegment=*beamSegmentIter;
                    if (beamSegment->pointInSpecBeamSegment(worldPosition))
                    {
                        contribBeamSegmentVector.push_back(beamSegment);
                    }
                }
            }
        }
        //=======================================================================//
        /*! Get the list of flux densities and approximate flux directions that contribute to a point in world space. */
        inline void getContributingBeamSegmentVector(const Vec3 &worldPosition, std::vector<stitch::BeamSegment const *> & contribBeamSegmentVector) const
        {
            if (beamTreeVector_.size()>0)
            {
                for (stitch::BeamTree const * const beamTreePtr : beamTreeVector_)
                {
                    if (beamTreePtr->pointInBV(worldPosition))
                    {
                        beamTreePtr->getContributingBeamSegmentVector(worldPosition, contribBeamSegmentVector);
                    }
                }
            } else
            {
                for (stitch::BeamSegment const * const beamSegmentPtr : beamSegmentVector_)
                {
                    if (beamSegmentPtr->pointInBV(worldPosition))
                    {
                        contribBeamSegmentVector.push_back(beamSegmentPtr);
                    }
                }
            }
        }
        //=======================================================================//
        //=======================================================================//
        
        
        
#ifdef USE_OSG
		osg::ref_ptr<osg::Node> constructOSGNode(const uintptr_t key=0) const;
#endif// USE_OSG
        
        
    public:
        std::vector<stitch::BeamSegment *> beamSegmentVector_;
        std::vector<stitch::BeamTree *> beamTreeVector_;
        
    private:
        stitch::Brush BVBrush_;
        bool BVUpdated_;
    };
    
}


#endif// STITCH_BEAMTREE_H
