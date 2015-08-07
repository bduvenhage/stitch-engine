/*
 * $Id: KDTree.h 299 2015-08-07 14:57:10Z bernardt.duvenhage $
 */
/*
 *  KDTree.h
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

#ifndef STITCH_KDTREE_H
#define STITCH_KDTREE_H

#define KDTREE_DEFAULT_CHUNK_SIZE 48

namespace stitch {
    class KDTree;
}

#include "Math/Vec3.h"
#include "Math/Plane.h"
#include "Photon.h"
#include "KNearestItems.h"

#ifdef _LIBCPP_VERSION
#include <unordered_map>
#else
#include <tr1/unordered_map>
#endif

#ifdef USE_CXX11
#include <cstdint>
#else
#include <stdint.h>
#endif

#include <iostream>

namespace stitch {
    
    //! Implements a KDTree (2D) of bounding volumes.
    class KDTree
    {
    public:
        KDTree();
        
        ~KDTree()
        {
            clear();
        }
        
        KDTree(const KDTree &lvalue) :
        binarySpacePartition_(lvalue.binarySpacePartition_),
        totalItems_(lvalue.totalItems_)
        {
            if (lvalue.left_)
            {
                left_ = lvalue.left_->clone();
            } else
            {
                left_ = nullptr;
            }
            
            if (lvalue.right_)
            {
                right_ = lvalue.right_->clone();
            } else
            {
                right_ = nullptr;
            }
            
            {
                const size_t numItems=lvalue.itemVector_.size();
                itemVector_.reserve(numItems);
                for (size_t itemNum=0; itemNum<numItems; ++itemNum)
                {
                    itemVector_.push_back(lvalue.itemVector_[itemNum]->clone());
                }
            }
        }
        
#ifdef USE_CXX11
        KDTree(KDTree &&rvalue) noexcept:
        binarySpacePartition_(std::move(rvalue.binarySpacePartition_)),
        itemVector_(std::move(rvalue.itemVector_)),
        totalItems_(std::move(rvalue.totalItems_))
        {
            left_ = rvalue.left_;
            rvalue.left_=nullptr;
            
            right_ = rvalue.right_;
            rvalue.right_=nullptr;
        }
#endif
        
        KDTree & operator = (const KDTree &lvalue)
        {
            if (&lvalue!=this)
            {
                clear();
                
                binarySpacePartition_=lvalue.binarySpacePartition_;
                
                if (lvalue.left_)
                {
                    left_ = lvalue.left_->clone();
                } else
                {
                    left_ = nullptr;
                }
                
                if (lvalue.right_)
                {
                    right_ = lvalue.right_->clone();
                } else
                {
                    right_ = nullptr;
                }
                
                {
                    const size_t numItems=lvalue.itemVector_.size();
                    itemVector_.reserve(numItems);
                    for (size_t itemNum=0; itemNum<numItems; ++itemNum)
                    {
                        itemVector_.push_back(lvalue.itemVector_[itemNum]->clone());
                    }
                }
                
                totalItems_=lvalue.totalItems_;
            }
            
            return (*this);
        }
        
#ifdef USE_CXX11
        KDTree & operator = (KDTree &&rvalue) noexcept
        {
            binarySpacePartition_=std::move(rvalue.binarySpacePartition_);
            itemVector_=std::move(rvalue.itemVector_);
            
            left_ = rvalue.left_;
            rvalue.left_=nullptr;
            
            right_ = rvalue.right_;
            rvalue.right_=nullptr;
            
            totalItems_=rvalue.totalItems_;
            
            return (*this);
        }
#endif
        
        /*! Virtual constructor idiom. Clone operator. */
        KDTree * clone() const //virtual removed from class.
        {
            return new KDTree(*this);
        }
        
        
        inline void reserveLinear(size_t n)
        {
            itemVector_.reserve(n);
        }
        
        inline void addItem(BoundingVolume * const item)
        {
            itemVector_.push_back(item);
            ++totalItems_;
        }
        
        /*! Get the number of objects in the tree. */
        size_t getNumItems() const
        {
            return totalItems_;
        }
        
        inline bool empty() const
        {
            return (getNumItems() == 0);
        }
        
        void clear();
        
        /*! Collect all items into a linear list and delete the tree structure. */
        inline void linearise()
        {
            std::cout << "KDTree::linearise not implemented yet!\n";
            std::cout.flush();
            exit(-1);
        }
        
        /*! Build/update the tree structure from the linear list of objects. */
        void build(const size_t chunkSize,
                   const uint8_t splitAxis, const size_t depth,
                   std::vector<stitch::Vec3> const &splitAxisVec);
        
        void balance();
        
        BoundingVolume *getNearest(const Vec3 &centre, float &searchRadiusSq) const;
        
        void getNearestK(KNearestItems * const kNearestItems) const;
        
#ifdef _LIBCPP_VERSION
        void getNearestKPerGroupID(KNearestItems const * const kNearestItemsRef, std::unordered_map<uint32_t, KNearestItems * const> *kNearestItemsMap) const;
#else
        void getNearestKPerGroupID(KNearestItems const * const kNearestItemsRef, std::tr1::unordered_map<uint32_t, KNearestItems * const> *kNearestItemsMap) const;
#endif// USE_CXX11
        
        stitch::Plane binarySpacePartition_;
        
        std::vector<BoundingVolume *> itemVector_;
        
        KDTree * left_;
        KDTree * right_;
        
        size_t totalItems_;
    };
    
}

#endif// STITCH_KDTREE_H
