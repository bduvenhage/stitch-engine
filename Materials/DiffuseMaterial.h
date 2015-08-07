/*
 * $Id: DiffuseMaterial.h 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  DiffuseMaterial.h
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

#ifndef STITCH_DIFFUSE_MATERIAL_H
#define STITCH_DIFFUSE_MATERIAL_H

namespace stitch {
	class DiffuseMaterial;
}

#include "Material.h"
#include "Math/VecN.h"
#include "Math/Colour.h"
 
namespace stitch {
    
    class DiffuseMaterial : public Material
    {
	public:
        DiffuseMaterial(const Colour_t&refl) :
        Material(),
        refl_(refl)
        {}
        
		DiffuseMaterial(const DiffuseMaterial &lValue) :
        Material(lValue),
        refl_(lValue.refl_)
		{
        }
		
		virtual ~DiffuseMaterial()
		{}
		
		virtual DiffuseMaterial & operator = (const DiffuseMaterial &lValue)
		{
            Material::operator=(lValue);
            
            refl_=lValue.refl_;
            
			return *this;
		}
        
        /*! Virtual constructor idiom. Clone operator. */
        virtual DiffuseMaterial * clone() const
        {
            return new DiffuseMaterial(*this);
        }
        
        virtual const Colour_t getEmittedRadiance(const Vec3 &normal, const Vec3 &outDir, const Vec3 &worldPosition) const
        {
            return zero_;
        }
        virtual Colour_t const getDiffuseRefl(const Vec3 &worldPosition) const
        {
            return refl_;
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
            return refl_;
        }
        
        virtual Photon scatterPhoton_direct(const Vec3 &normal, const Vec3 &worldPosition, const Photon &photon) const;
        
        virtual Colour_t BSDF(const Vec3 &worldPosition, const Vec3 &dirA, const Vec3 &dirB, const Vec3 &normal) const
        {
            const Vec3 litNormal=((normal*dirA)>0.0f) ? normal : normal.zeroMinus();
            
            if ((dirB*litNormal)>0.0f)
            {
                return getDiffuseRefl(worldPosition)*((float)M_1_PI);
            } else
            {
                return Colour_t();
            }
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
        
        virtual Vec3 whittedSpecRefractRay(const Vec3 &toVertex, const Vec3 &normal) const
        {
            return Vec3();
        }
        
        virtual Vec3 stochasticSpecReflectRay(const Vec3 &toVertex, const Vec3 &normal) const
        {
            return Vec3();
        }

        virtual Vec3 stochasticDiffuseReflectRay(const Vec3 &toVertex, const Vec3 &normal) const
        {
            Vec3 litNormal=((normal*toVertex)<0.0f) ? normal : normal.zeroMinus();
            
            const Vec3 orthVecA=(litNormal.orthVec()).normalised();
            const Vec3 orthVecB=stitch::Vec3::crossNormalised(litNormal, orthVecA);
            
            Vec3 scatteredDirLocal=Vec3::randCosineLobe(1.0f);
            
            return stitch::Vec3(orthVecA, orthVecB, litNormal,
                                scatteredDirLocal.x(), scatteredDirLocal.y(), scatteredDirLocal.z());
        }

        /*! Get the material type contained in this object. Useful when one only has a base type pointer to the object.
         @return The material type contained in this object.*/
        virtual const MaterialType getType() const
        {
            return DIFFUSE_MATERIAL;
        }
        
        
        virtual const std::string getTypeString() const
        {
            return "DiffMat";
        }
        
        Colour_t refl_;
    };
}


#endif// STITCH_DIFFUSE_MATERIAL_H