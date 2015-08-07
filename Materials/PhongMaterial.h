/*
 * $Id: PhongMaterial.h 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  PhongMaterial.h
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

#ifndef STITCH_PHONG_MATERIAL_H
#define STITCH_PHONG_MATERIAL_H

namespace stitch {
	class Material;
}

#include "Material.h"
#include "Math/VecN.h"
#include "Math/Colour.h"

namespace stitch {
    
    class PhongMaterial : public Material
    {//Blinn-Phong material in fact.
	public:
        PhongMaterial(const Colour_t &dRefl, const Colour_t &sRefl, const Colour_t &sTrans, const float sExp, std::string textureFileName) :
        Material(),
        dRefl_(dRefl),
        sRefl_(sRefl),
        sTrans_(sTrans),
        sExp_(sExp),
        albedo_(dRefl_ + sRefl_ + sTrans_),
        textureFileName_(textureFileName)
#ifdef USE_OSG
        , textureImage_(nullptr)
#endif// USE_OSG
        {
            if (textureFileName_!="")
            {
#ifdef USE_OSG
                textureImage_=osgDB::readImageFile(textureFileName_);
                
                if (!textureImage_)
                {
                    std::cout << "ERROR!: " << textureFileName_ << " not found! Is your working/running folder set to the source base folder?\n";
                    std::cout.flush();
                }
#endif// USE_OSG
            }
        }
        
		PhongMaterial(const PhongMaterial &lValue) :
        Material(lValue),
        dRefl_(lValue.dRefl_),
        sRefl_(lValue.sRefl_),
        sTrans_(lValue.sTrans_),
        sExp_(lValue.sExp_),
        albedo_(lValue.albedo_),
        textureFileName_(lValue.textureFileName_)
#ifdef USE_OSG
        , textureImage_(lValue.textureImage_)
#endif// USE_OSG
		{
        }
		
		virtual ~PhongMaterial()
		{}
		
		virtual PhongMaterial & operator = (const PhongMaterial &lValue)
		{
            Material::operator=(lValue);
            
            dRefl_=lValue.dRefl_;
            sRefl_=lValue.sRefl_;
            sTrans_=lValue.sTrans_;
            sExp_=lValue.sExp_;
            albedo_=lValue.albedo_;
            textureFileName_=lValue.textureFileName_;
#ifdef USE_OSG
            textureImage_=lValue.textureImage_;
#endif// USE_OSG
            
			return *this;
		}
        
        virtual PhongMaterial * clone() const
        {
            return new PhongMaterial(*this);
        }
        
        virtual const Colour_t getEmittedRadiance(const Vec3 &normal, const Vec3 &outDir, const Vec3 &worldPosition) const
        {
            return zero_;
        }
        
        virtual Colour_t const getDiffuseRefl(const Vec3 &worldPosition) const
        {
#ifdef USE_OSG            
            if (textureImage_)
            {
                ssize_t width=textureImage_->s();
                ssize_t height=textureImage_->t();
                
                ssize_t s=abs(((ssize_t)(worldPosition.x()*100.0f + 0.5f)+(width>>1)))%width;
                ssize_t t=abs(((ssize_t)(worldPosition.z()*100.0f + 0.5f)+(height>>1)))%height;
                
                osg::Vec4f osgTextureColour=textureImage_==0 ? osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f) : textureImage_->getColor(s,t);
                Colour_t textureColour=stitch::Colour_t(osgTextureColour._v[0], osgTextureColour._v[1], osgTextureColour._v[2]);
                
                return dRefl_.cmult(textureColour);
            } else
#endif//USE_OSG
            {
                return dRefl_;
            }
        }
        virtual Colour_t const & getSpecularRefl() const
        {
            return sRefl_;
        }
        virtual Colour_t const & getSpecularTrans() const
        {
            return sTrans_;
        }
        
        virtual Colour_t const & getAlbedo() const
        {
            return albedo_;//(dRefl_ + sRefl_ + sTrans_);
        }
        
        virtual Photon scatterPhoton_direct(const Vec3 &normal, const Vec3 &worldPosition, const Photon &photon) const;
        
        /*! Scatter distribution function
         @sa Page 452-457 of PBRT 2nd ed. Sections 8.4.2 and 8.4.3 on Torrance-Sparrow and Blinn BRDFs. 
         @param exp Specular exponent of Cosine scatter distribution.
         @param offSpecVector Scatter vector relative to specular scatter vector.*/
        virtual float S_pdf(const float exp, const Vec3 &offSpecVector) const
        {
            return stitch::Vec3::randCosineLobe_pdf_proj(exp, offSpecVector);//Use randCosineLobe_pdf_proj instead of randCosineLobe_pdf for a BRDF that is balanced down to a zero exponent. 
        }
        
        /*! Phong BRDF
         @sa Page 452-457 of PBRT 2nd ed. Sections 8.4.2 and 8.4.3 on Torrance-Sparrow and Blinn BRDFs.
         */
        virtual Colour_t BSDF(const Vec3 &worldPosition, const Vec3 &dirA, const Vec3 &dirB, const Vec3 &normal) const
        {
            //Phong.
            const Vec3 specReflectVector=whittedSpecReflectRay(dirA.zeroMinus(), normal);
            
            const Vec3 litNormal=((normal*dirA)>0.0f) ? normal : normal.zeroMinus();
            
                const Vec3 orthA=(specReflectVector.orthVec()).normalised();
                const Vec3 orthB=stitch::Vec3::crossNormalised(specReflectVector, orthA);
                
                const Vec3 dirB_relative_to_spec_reflect=Vec3(dirB*orthA,
                                                              dirB*orthB,
                                                              dirB*specReflectVector);
                
            return getDiffuseRefl(worldPosition) * (((float)M_1_PI)*((dirB*litNormal) >= 0.0f ? 1.0f : 0.0f)) +
                   getSpecularRefl() * (S_pdf(sExp_, dirB_relative_to_spec_reflect) / fabs(dirA * normal));
        }
        
        inline virtual Vec3 getBSDFPeak(const Vec3 &dirA, const Vec3 &normal) const
        {
            return whittedSpecReflectRay(dirA.zeroMinus(), normal);
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
            
            const float n=1.5f;
            const Vec3 refractedTangentComponent=(tangentComponent / n);
            
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
        
        virtual Vec3 stochasticSpecReflectRay(const Vec3 &toVertex, const Vec3 &normal) const;

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
            return PHONG_MATERIAL;
        }
        
        virtual const std::string getTypeString() const
        {
            return "PhongMat";
        }
        
        Colour_t dRefl_;
        Colour_t sRefl_;
        Colour_t sTrans_;
        float sExp_;
        Colour_t albedo_;
        
        std::string textureFileName_;
#ifdef USE_OSG
        osg::ref_ptr<osg::Image> textureImage_;
#endif//USE_OSG
    };
}


#endif// STITCH_PHONG_MATERIAL_H