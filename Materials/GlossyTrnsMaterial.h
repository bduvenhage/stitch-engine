/*
 * $Id: GlossyTrnsMaterial.h 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  GlossyTransMaterial.h
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

#ifndef STITCH_GLOSSY_TRANS_MATERIAL_H
#define STITCH_GLOSSY_TRANS_MATERIAL_H

namespace stitch {
    class GlossyTransMaterial;
}

#include "Material.h"
#include "Math/Colour.h"

namespace stitch {
    
    class GlossyTransMaterial : public Material
    {
	public:
        GlossyTransMaterial(const Colour_t &trans, const float sd, const float n) :
        Material(),
        trans_(trans), sd_(sd), n_(n)
        {}
        
        GlossyTransMaterial(const GlossyTransMaterial &lValue) :
        Material(lValue),
        trans_(lValue.trans_),
        sd_(lValue.sd_),
        n_(lValue.n_)
		{}
		
        virtual ~GlossyTransMaterial()
		{}
		
        virtual GlossyTransMaterial & operator = (const GlossyTransMaterial &lValue)
		{
            Material::operator=(lValue);
            
            trans_=lValue.trans_;
            sd_=lValue.sd_;
            n_=lValue.n_;
            
			return *this;
		}
        
        /*! Virtual constructor idiom. Clone operator. */
        virtual GlossyTransMaterial * clone() const
        {
            return new GlossyTransMaterial(*this);
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
            return zero_;
        }
        virtual Colour_t const & getSpecularTrans() const
        {
            return trans_;
        }
                
        virtual Colour_t const & getAlbedo() const
        {
            return trans_;
        }

        virtual Photon scatterPhoton_direct(const Vec3 &normal, const Vec3 &worldPosition, const Photon &photon) const;
        
        virtual Colour_t BSDF(const Vec3 &worldPosition, const Vec3 &dirA, const Vec3 &dirB, const Vec3 &normal) const
        {
            //Similar to Phong, but with Gaussian distribution instead of cosine lobe.
            const Vec3 specRefractVector=whittedSpecRefractRay(dirA.zeroMinus(), normal);
            
            const Vec3 orthA=(specRefractVector.orthVec()).normalised();
            const Vec3 orthB=stitch::Vec3::crossNormalised(specRefractVector, orthA);
            
            const Vec3 dirB_relative_to_spec_refract=Vec3(dirB*orthA,
                                                          dirB*orthB,
                                                          dirB*specRefractVector);
            
            return getSpecularTrans() * Vec3::randGaussianLobe_pdf(sd_, dirB_relative_to_spec_refract) / fabs(dirA * normal);
        }
        
        inline virtual Vec3 getBSDFPeak(const Vec3 &dirA, const Vec3 &normal) const
        {
            return whittedSpecRefractRay(dirA.zeroMinus(), normal);
        }
        
        inline virtual Vec3 whittedSpecReflectRay(const Vec3 &toVertex, const Vec3 &normal) const
        {
            return (toVertex - normal*((toVertex*normal)*2.0f));
        }
        
        inline virtual Vec3 whittedSpecRefractRay(const Vec3 &toVertex, const Vec3 &normal) const
        {
            const float cosTheta=toVertex*normal;
            const Vec3 normalComponent=normal*cosTheta;
            const Vec3 tangentComponent=toVertex-normalComponent;
                        
                const Vec3 refractedTangentComponent=(tangentComponent / n_);
            
                const float refractedNormalComponentLengthSq=1.0f - refractedTangentComponent.lengthSq();
            
                if (refractedNormalComponentLengthSq>0.0f)
                {
                    const float refractedNormalComponentLength=sqrtf(refractedNormalComponentLengthSq);
                    
                    const Vec3 refractedNormalComponent=(cosTheta>0.0f) ? normal*refractedNormalComponentLength : normal*(-refractedNormalComponentLength);
                    return (refractedNormalComponent+refractedTangentComponent).normalised();
                } else
                {
                    return Vec3(0.0f, 0.0f, 0.0f);
                }
        }
        
        inline virtual Vec3 stochasticSpecReflectRay(const Vec3 &toVertex, const Vec3 &normal) const
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
            return GLOSSY_TRANS_MATERIAL;
        }
                
        virtual const std::string getTypeString() const
        {
            return "GlossyTransMat";
        }
        
        Colour_t trans_;
        float sd_;
        float n_;
    };
}


#endif// STITCH_GLOSSY_TRANS_MATERIAL_H
