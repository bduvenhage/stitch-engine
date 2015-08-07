/*
 * $Id: EmissiveMaterial.h 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  EmissiveMaterial.h
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

#ifndef STITCH_EMISSIVE_MATERIAL_H
#define STITCH_EMISSIVE_MATERIAL_H

namespace stitch {
	class Material;
}

#include "Material.h"
#include "Math/VecN.h"
#include "Math/Colour.h"
 
namespace stitch {    
    
    class EmissiveMaterial : public Material
    {
	public:
        EmissiveMaterial(const Colour_t &radiance) :
        Material(),
        radiance_(radiance)
        {}
        
		EmissiveMaterial(const EmissiveMaterial &lValue) :
        Material(lValue),
        radiance_(lValue.radiance_)
		{}
		
		virtual ~EmissiveMaterial()
		{}
		
		virtual EmissiveMaterial & operator = (const EmissiveMaterial &lValue)
		{
            Material::operator=(lValue);
            
            radiance_=lValue.radiance_;
            
			return *this;
		}
        
        /*! Virtual constructor idiom. Clone operator. */
        virtual EmissiveMaterial * clone() const
        {
            return new EmissiveMaterial(*this);
        }
        
        virtual const Colour_t getEmittedRadiance(const Vec3 &normal, const Vec3 &outDir, const Vec3 &worldPosition) const
        {
            return radiance_;
        }
        virtual Colour_t const getDiffuseRefl(const Vec3 &worldPosition) const
        {
            return zero_;
        }
        virtual Colour_t const & getSpecularRefl() const
        {
            return zero_;
        }
        virtual Colour_t const & getSpecularTrans() const
        {
            return zero_;
        }
        
        virtual Colour_t const & getAlbedo() const
        {
            return zero_;
        }

        virtual Photon scatterPhoton_direct(const Vec3 &normal, const Vec3 &worldPosition, const Photon &photon) const
        {
            return Photon(worldPosition, Vec3(), Colour_t(), photon.scatterCount_+1);
        }
        
        virtual Colour_t BSDF(const Vec3 &worldPosition, const Vec3 &dirA, const Vec3 &dirB, const Vec3 &normal) const
        {
            return Colour_t();
        }
        
        inline virtual Vec3 getBSDFPeak(const Vec3 &dirA, const Vec3 &normal) const
        {
            const Vec3 litNormal=((normal*dirA)>0.0f) ? normal : normal.zeroMinus();
            return litNormal;
        }

        inline virtual Vec3 whittedSpecReflectRay(const Vec3 &toVertex, const Vec3 &normal) const
        {
            return Vec3();
        }

        inline virtual Vec3 whittedSpecRefractRay(const Vec3 &toVertex, const Vec3 &normal) const
        {
            return Vec3();
        }
        
        virtual Vec3 stochasticSpecReflectRay(const Vec3 &toVertex, const Vec3 &normal) const
        {
            return Vec3();
        }
        
        virtual Vec3 stochasticDiffuseReflectRay(const Vec3 &toVertex, const Vec3 &normal) const
        {
            return Vec3();
        }

        /*! Get the material type contained in this object. Useful when one only has a base type pointer to the object.
         @return The material type contained in this object.*/
        virtual const MaterialType getType() const
        {
            return EMISSIVE_MATERIAL;
        }

        virtual const std::string getTypeString() const
        {
            return "EmissMat";
        }
    
        Colour_t radiance_;
    };
}


#endif// STITCH_EMISSIVE_MATERIAL_H