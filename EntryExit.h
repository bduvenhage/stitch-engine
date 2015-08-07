/*
 * $Id: EntryExit.h 299 2015-08-07 14:57:10Z bernardt.duvenhage $
 */
/*
 *  EntryExit.h
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


#ifndef STITCH_ENTRYEXIT_H
#define STITCH_ENTRYEXIT_H

namespace stitch {
	class EntryExit;
}

#include "Math/Vec3.h"
#include "Object.h"

namespace stitch {
	
    //! The intersection between a line and an object such as a brush or a sphere which would have both an entry and an exit.
	class EntryExit
	{
	public:
        /*! Constructor when the entry and exit distances are known.
         * @param entryDistance The distance of the entry intersection.
         * @param exitDistance The distance of the exit intersection. */
		EntryExit(const float entryDistance, const float exitDistance) :
		entryDistance_(entryDistance), entryNormal_(),
		exitDistance_(exitDistance), exitNormal_(),
        object_(nullptr)
		{}
        
        /*! Destructor */
		~EntryExit() {}
		
        /*! Copy constructor */
		EntryExit(const EntryExit &lValue) :
		entryDistance_(lValue.entryDistance_), entryNormal_(lValue.entryNormal_),
		exitDistance_(lValue.exitDistance_), exitNormal_(lValue.exitNormal_),
        object_(lValue.object_)
		{}
		
#ifdef USE_CXX11
        /*! Move constructor */
		EntryExit(EntryExit &&rValue) noexcept:
		entryDistance_(rValue.entryDistance_), entryNormal_(std::move(rValue.entryNormal_)),
		exitDistance_(rValue.exitDistance_), exitNormal_(std::move(rValue.exitNormal_)),
        object_(rValue.object_)
		{}
#endif// USE_CXX11
		
        
        /*! Assignment operator */
		inline EntryExit & operator=(const EntryExit &lValue)
		{
			entryDistance_=lValue.entryDistance_;
			exitDistance_=lValue.exitDistance_;
			entryNormal_=lValue.entryNormal_;
			exitNormal_=lValue.exitNormal_;
            object_=lValue.object_;
			return (*this);
		}
		
#ifdef USE_CXX11
        /*! Assignment operator with move */
		inline EntryExit & operator=(EntryExit &&rValue) noexcept
		{
			entryDistance_=rValue.entryDistance_;
			exitDistance_=rValue.exitDistance_;
			entryNormal_=std::move(rValue.entryNormal_);
			exitNormal_=std::move(rValue.exitNormal_);
            object_=rValue.object_;
			return (*this);
		}
#endif// USE_CXX11
		
        /*! The distance to the entrance intersection */
		float entryDistance_;
        
        /*! The surface normal at the entrance intersection */
		Vec3 entryNormal_;
		
        /*! The distance to the exit intersection */
		float exitDistance_;
        
        /*! The surface normal at the exit intersection */
		Vec3 exitNormal_;
        
        /*! The object intersected */
        Object const *object_;
	};
}


#endif// STITCH_ENTRYEXIT_H