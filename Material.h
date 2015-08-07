/*
 * $Id: Material.h 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  Material.h
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

#ifndef STITCH_MATERIAL_H
#define STITCH_MATERIAL_H

namespace stitch {
	class Material;
}

#include "Math/Vec3.h"
#include "Math/Colour.h"
#include "Photon.h"

#include <string>

namespace stitch {
    
    //! Abstract base class for a material including its BRDF.
	class Material
	{
	public:
        /*! Enum for the material type. Mostly used to cast pointers to child classes
         to the correct type */
        enum MaterialType {
            ABSTRACT_MATERIAL,
            EMISSIVE_MATERIAL,
            DIFFUSE_MATERIAL,
            SPECULAR_MATERIAL,
            PHONG_MATERIAL,
            BLINN_PHONG_MATERIAL,
            GLOSSY_MATERIAL,
            GLOSSY_TRANS_MATERIAL
        };
        
        /*! Constructor */
		Material()
		{}
		
        /*! Virtual destructor */
		virtual ~Material()
		{}
		
        /*! Copy constructor */
		Material(const Material &lValue)
		{}
        
        /*! Assignment constructor */
		virtual Material & operator = (const Material &lValue)
		{
			return *this;
		}
        
        /*! Virtual constructor idiom. Clone operator. */
        virtual Material * clone() const = 0;  //Uses the copy constructor.
        
        /*! Access the material's emission distribution.
         @param normal The surface normal of the material.
         @param outDir The emission direction being queried.
         @param worldPosition The position on/in the material being queried.
         @return The returned radiance of type Colour_t. */
        virtual const Colour_t getEmittedRadiance(const Vec3 &normal, const Vec3 &outDir, const Vec3 &worldPosition) const = 0;
        
        /*! Access the material's diffuse/Lambertian reflectance.
         @param worldPosition The position on/in the material being queried.
         @return The diffuse reflectance of type Colour_t. */
        virtual Colour_t const getDiffuseRefl(const Vec3 &worldPosition) const = 0;
        
        /*! Access the material's specular reflectance.
         @return The specular reflectance of type Colour_t. */
        virtual Colour_t const & getSpecularRefl() const = 0;
        
        /*! Access the material's specular transmittance.
         @return The specular transmittance of type Colour_t. */
        virtual Colour_t const & getSpecularTrans() const = 0;
        
        /*! Get the materials albedo.
         @return The albedo of the material. */
        virtual Colour_t const & getAlbedo() const = 0;
        
        /*! Calculate the MC+roundRobbin scattered photon given an incoming photon. The scattered direction is directly generated.
         @param normal The surface normal of the material.
         @param worldPosition The position on/in the material being queried.
         @param photon The incoming photon to be scattered or absorbed.
         @return The new scattered photon.
         @sa scatterPhoton_reject_samp*/ 
        virtual Photon scatterPhoton_direct(const Vec3 &normal, const Vec3 &worldPosition, const Photon &photon) const = 0;
        
        /*! Calculate the MC+roundRobbin scattered photon given an incoming photon. The BRDF and rejection sampling is used to generate a scattered direction.
         @param normal The surface normal of the material.
         @param worldPosition The position on/in the material being queried.
         @param photon The incoming photon to be scattered or absorbed.
         @return The new scattered photon.
         @sa scatterPhoton_reject_samp*/
        virtual Photon scatterPhoton_reject_samp(const Vec3 &normal, const Vec3 &worldPosition, const Photon &photon) const;
        
        /*! Access the materials bidirectional scatterance distribution function.
         @param worldPosition The position on/in the material being queried.
         @param dirA The first BSDF direction.
         @param dirB The second BSDF direction.
         @param normal The surface normal of the material.
         @return The scatterance given the bidirectional geometry.*/
        virtual Colour_t BSDF(const Vec3 &worldPosition, const Vec3 &dirA, const Vec3 &dirB, const Vec3 &normal) const = 0;
        
        /*! Get the direction of the BSDF peak given the first BSDF direction. It assumes a single BRDF lobe.
         @param dirA The first BSDF direction.
         @param normal The surface normal of the material.
         @return The peak of the scatter distribution given the first BSDF direction.*/
        virtual Vec3 getBSDFPeak(const Vec3 &dirA, const Vec3 &normal) const = 0;

        /*! Calculate the specular reflection direction such as would be used by a Whitted ray tracer.
         @param toVertex The incoming direction pointing towards the surface.
         @param normal The surface normal of the material.
         @return The resulting reflection direction.*/
        virtual Vec3 whittedSpecReflectRay(const Vec3 &toVertex, const Vec3 &normal) const = 0;
        
        /*! Calculate the specular refraction direction such as would be used by a Whitted ray tracer.
         @param toVertex The incoming direction pointing towards the surface.
         @param normal The surface normal of the material.
         @return The resulting refracted direction.*/
        virtual Vec3 whittedSpecRefractRay(const Vec3 &toVertex, const Vec3 &normal) const = 0;
        
        /*! Calculate the stochastic specular reflection direction such as would be used by a distributed ray tracer.
         @param toVertex The incoming direction pointing towards the surface.
         @param normal The surface normal of the material.
         @return The resulting stochastic reflection direction.*/
        virtual Vec3 stochasticSpecReflectRay(const Vec3 &toVertex, const Vec3 &normal) const = 0;

        /*! Calculate the stochastic specular reflection direction such as would be used by a distributed ray tracer.
         @param toVertex The incoming direction pointing towards the surface.
         @param normal The surface normal of the material.
         @return The resulting stochastic reflection direction.*/
        virtual Vec3 stochasticDiffuseReflectRay(const Vec3 &toVertex, const Vec3 &normal) const = 0;
        
        /*! Get the material type contained in this object. Useful when one only has a base type pointer to the object.
         @return The material type contained in this object.*/
        virtual const MaterialType getType() const = 0;
        
        /*! Get the material type string contained in this object. Useful when one only has a base type pointer to the object.
         @return The material type string contained in this object.*/
        virtual const std::string getTypeString() const = 0;
        
        const static Colour_t zero_;
    };
    
}


#endif// STITCH_MATERIAL_H
