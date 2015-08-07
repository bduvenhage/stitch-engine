/*
 * $Id: KNearestItems.h 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  KNearestItems.h
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

#ifndef STITCH_KNEARESTITEMS_H
#define STITCH_KNEARESTITEMS_H

namespace stitch {
    class KNearestItems;
}

#include "Math/Vec3.h"
#include "BoundingVolume.h"

namespace stitch
{
    //!Stores k-nearest items in binary heap during kd-tree query.
    class KNearestItems
    {
    private:
        //!Copy constructor (Private because some of the class members are constant).
        KNearestItems(const KNearestItems &lValue) :
        searchRadiusSq_(lValue.searchRadiusSq_),
        centre_(lValue.centre_),
        k_(lValue.k_),
        numItems_(0)
        {
            heapArray_=new std::pair<float, BoundingVolume const *>[k_];
        }
        
        //!Assignment operator (Private because some of the class members are constant).
        KNearestItems &operator = (const KNearestItems &lValue)
        {
            return *this;
        }
        
    public:
        KNearestItems(const Vec3 &centre, const float searchRadiusSq, const size_t k) :
        searchRadiusSq_(searchRadiusSq),
        centre_(centre),
        k_(k),
        numItems_(0)
        {
            heapArray_=new std::pair<float, BoundingVolume const *>[k_];
        }
        
        ~KNearestItems()
        {
            delete [] heapArray_;
        }
        
        //!Insert an item into the k-nearest-neighbour heap.
        inline void insert(BoundingVolume const * const item)
        {
            const float itemDistSq=Vec3::calcDistToPointSq(item->centre_, centre_);
            
            if (itemDistSq<=searchRadiusSq_)
            {
                if (numItems_<k_)
                {//Add to bottom of heap and up-heap.
                    heapArray_[numItems_]=std::make_pair(itemDistSq, item);
                    ++numItems_;
                    
                    if (numItems_>1)
                    {
                        //Up-heap.
                        size_t childNode=numItems_-1;
                        size_t parentNode=(childNode-1) >> 1;
                        
                        while (heapArray_[childNode].first > heapArray_[parentNode].first)
                        {
                            std::swap(heapArray_[childNode], heapArray_[parentNode]);
                            
                            if (parentNode!=0)
                            {
                                childNode=parentNode;
                                parentNode=(childNode-1) >> 1;
                            } else
                            {
                                break;
                            }
                        }
                    }
                } else
                {//numItems_==k_;
                    //if (itemDistSq < heapArray_[0].first) Already checked (itemDistSq<=searchRadiusSq_) and searchRadiusSq_==heapArray_[0].first
                    {//Replace root (max node) of heap and down-heap.
                        heapArray_[0]=std::make_pair(itemDistSq, item);
                        
                        //Down-heap.
                        size_t parentNode=0;
                        size_t childNodeLeft=(parentNode<<1) + 1;
                        size_t childNodeRight=(parentNode<<1) + 2;
                        size_t childNodeBiggest=(heapArray_[childNodeLeft].first > heapArray_[childNodeRight].first) ? childNodeLeft : childNodeRight;
                        
                        while (heapArray_[parentNode].first<heapArray_[childNodeBiggest].first)
                        {
                            std::swap(heapArray_[parentNode], heapArray_[childNodeBiggest]);
                            
                            if (((childNodeBiggest<<1) + 1)<numItems_)
                            {
                                parentNode=childNodeBiggest;
                                childNodeLeft=(parentNode<<1) + 1;
                                childNodeRight=(parentNode<<1) + 2;
                                childNodeBiggest=(heapArray_[childNodeLeft].first > heapArray_[childNodeRight].first) ? childNodeLeft : childNodeRight;
                            } else
                            {
                                break;
                            }
                        }
                        
                        searchRadiusSq_=heapArray_[0].first;
                    }
                }
            }
        }
        
    public:
        float searchRadiusSq_;
        const Vec3 centre_;
        const size_t k_;
                
        std::pair<float, BoundingVolume const *> *heapArray_;
        size_t numItems_;
    };
    
}

#endif// STITCH_KNEARESTITEMS_H
