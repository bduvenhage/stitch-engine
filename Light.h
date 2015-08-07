/*
 * $Id: Light.h 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  Light.h
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2011/03/01.
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

#ifndef STITCH_LIGHT_H
#define STITCH_LIGHT_H

namespace stitch {
	class Light;
}

#include "Math/Colour.h"
#include "Intersection.h"
#include "Photon.h"
#include "Object.h"

namespace stitch {
	
    //! Abstract base class for lights. 
	class Light : public Object
	{
	public:
        /*! Constructor given a SPD and material; to be used by child class.
         @param SPD Spectral power distribution of light source.
         @param pMaterial Pointer to material object of light that is setup to match the SPD. Object takes ownership of object that pMaterial points to. */
		Light(const Colour_t &SPD, Material * const pMaterial) :
        Object(pMaterial),
		SPD_(SPD),
        normalisedSPD_(SPD.cnormalised())
		{}
		
        /*! Copy constructor. */
		Light(const Light &lValue) :
        Object(lValue),
		SPD_(lValue.SPD_),
        normalisedSPD_(lValue.normalisedSPD_)
		{}
		
        /*! Assignment constructor. */
		virtual Light & operator = (const Light &lValue)
		{
            Object::operator=(lValue);
            
            SPD_=lValue.SPD_;
            normalisedSPD_=lValue.normalisedSPD_;
            
			return *this;
		}
		
        /*! Virtual destructor */
		virtual ~Light()
		{}
        
#ifdef USE_OSG
        /*! From base class stitch::Object. Creates an OSG node that may be used to create a preview of the object.
         @sa stitch::Object */
		virtual osg::ref_ptr<osg::Node> constructOSGNode(const bool createOSGLineGeometry, const bool createOSGNormalGeometry, const bool wireframe, const uintptr_t key=0) const = 0;
#endif// USE_OSG
        
        
        /*! Gets the lights spectral power distribution.
         @return The spectral power distribution of type Colour_t. */
        virtual Colour_t getSPD()
        {
            return SPD_;
        }
		
        
        /*! Get the photons that would be radiated in duration t from this light source.
         @param time The duration in seconds for which to radiate photons.
         @param photonVector The supplied vector in which to store the radiated photons. */
		virtual void radiate(const float time, std::vector<Photon *> &photonVector) const = 0;
        
        /*! Get the photons that would be radiated in duration t from this light source.
         @param time The duration in seconds for which to radiate photons.
         @param photonVector The supplied vector in which to store the radiated photons.
         @param blockerDirection The direction of a non-transparent blocker. The blocker may be used to for performance reasons limit the amount of photons emitted into the scene.
         @param blockerSubtendHA The half angle size of the blocker. The blocker may be used to for performance reasons limit the amount of photons emitted into the scene.*/
		virtual void radiate(const float time, std::vector<Photon *> &photonVector, const Vec3 blockerDirection, float blockerSubtendHA) const = 0;
        
    protected:
        /*! The spectral power distribution of this light.*/
        Colour_t SPD_;
        
        /*! The normalised spectral power distribution of this light. Normalised means that the area under distribution is now one.*/
        Colour_t normalisedSPD_;
	};
    
}

#endif// STITCH_LIGHT_H
