/*
 * $Id: GlossyMaterial.h 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  GlossyMaterial.h
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

#ifndef STITCH_GLOSSY_MATERIAL_H
#define STITCH_GLOSSY_MATERIAL_H

namespace stitch {
    class GlossyMaterial;
}

#include "Material.h"
#include "Math/Colour.h"

namespace stitch {
    
    class GlossyMaterial : public Material
    {
	public:
        //
        GlossyMaterial(const Colour_t &refl, const float sd) :
        Material(),
        refl_(refl), sd_(sd)
        {}
        
        GlossyMaterial(const GlossyMaterial &lValue) :
        Material(lValue),
        refl_(lValue.refl_),
        sd_(lValue.sd_)
		{}
		
        virtual ~GlossyMaterial()
		{}
		
        virtual GlossyMaterial & operator = (const GlossyMaterial &lValue)
		{
            Material::operator=(lValue);
            
            refl_=lValue.refl_;
            sd_=lValue.sd_;
            
			return *this;
		}
        
        /*! Virtual constructor idiom. Clone operator. */
        virtual GlossyMaterial * clone() const
        {
            return new GlossyMaterial(*this);
        }
        
        virtual const Colour_t getEmittedRadiance(const Vec3 &normal, const Vec3 &outDir, const Vec3 &worldPosition) const
        {
            return zero_;
        }
        virtual Colour_t const getDiffuseRefl(const Vec3 &worldPosition) const
        {
            return zero_;
        }
        virtual Colour_t const & getSpecularRefl() const
        {
            return refl_;
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
        
        /*! Scatter distribution function
         @sa Page 452-457 of PBRT 2nd ed. Sections 8.4.2 and 8.4.3 on Torrance-Sparrow and Blinn BRDFs. 
         @param sd Standard deviation of Gaussian scatter distribution.
         @param offSpecVector Scatter vector relative to specular scatter vector.*/
        virtual float S_pdf(const float sd, const Vec3 &offSpecVector) const
        {
            return stitch::Vec3::randGaussianLobe_pdf(sd, offSpecVector);
        }

        /*! Spec Microfacet BRDF
         @sa Page 452-457 of PBRT 2nd ed. Sections 8.4.2 and 8.4.3 on Torrance-Sparrow and Blinn BRDFs.
         */
        virtual Colour_t BSDF(const Vec3 &worldPosition, const Vec3 &dirA, const Vec3 &dirB, const Vec3 &normal) const
        {
            //Similar to Phong, but with Gaussian distribution instead of cosine lobe.
            const Vec3 specReflectVector=whittedSpecReflectRay(dirA.zeroMinus(), normal);
            
                const Vec3 orthA=(specReflectVector.orthVec()).normalised();
                const Vec3 orthB=stitch::Vec3::crossNormalised(specReflectVector, orthA);
                
                const Vec3 dirB_relative_to_spec_reflect=Vec3(dirB*orthA,
                                                              dirB*orthB,
                                                              dirB*specReflectVector);
                
                return getSpecularRefl() * (S_pdf(sd_, dirB_relative_to_spec_reflect) / fabs(dirA * normal));
        }
        
        inline virtual Vec3 getBSDFPeak(const Vec3 &dirA, const Vec3 &normal) const
        {
            return whittedSpecReflectRay(dirA.zeroMinus(), normal);
        }
        
        inline virtual Vec3 whittedSpecReflectRay(const Vec3 &toVertex, const Vec3 &normal) const
        {
            return (toVertex - normal*((toVertex*normal)*2.0f));
        }
        
        virtual Vec3 whittedSpecRefractRay(const Vec3 &toVertex, const Vec3 &normal) const
        {
            return Vec3();
        }
        
        virtual Vec3 stochasticSpecReflectRay(const Vec3 &toVertex, const Vec3 &normal) const;

        virtual Vec3 stochasticDiffuseReflectRay(const Vec3 &toVertex, const Vec3 &normal) const
        {
            return Vec3();
        }

        /*! Get the material type contained in this object. Useful when one only has a base type pointer to the object.
         @return The material type contained in this object.*/
        virtual const MaterialType getType() const
        {
            return GLOSSY_MATERIAL;
        }        
        
        virtual const std::string getTypeString() const
        {
            return "GlossyMat";
        }
        
        Colour_t refl_;
        float sd_;
    };
}


#endif// STITCH_GLOSSY_MATERIAL_H
