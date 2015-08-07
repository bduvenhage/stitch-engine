/*
 * $Id: BlinnPhongMaterial.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  BlinnPhongMaterial.cpp
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2010/03/27.
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

#include "BlinnPhongMaterial.h"
#include "Photon.h"

//=======================================================================//
stitch::Vec3 stitch::BlinnPhongMaterial::stochasticSpecReflectRay(const Vec3 &toVertex, const Vec3 &normal) const
{
    Vec3 scatteredDir;

    while (scatteredDir.lengthSq()==0.0f)
    {
        const Vec3 litNormal=((normal*toVertex)<0.0f) ? normal : normal.zeroMinus();
        Vec3 orthVecA=litNormal.orthVec();
        orthVecA.normalise();
        const Vec3 orthVecB=stitch::Vec3::crossNormalised(litNormal, orthVecA);
        
        const Vec3 mfNormal_relative_to_lit_normal=Vec3::randCosineLobe(this->sExp_);
        
        const Vec3 mfNormal=stitch::Vec3(orthVecA, orthVecB, litNormal,
                                         mfNormal_relative_to_lit_normal.x(), mfNormal_relative_to_lit_normal.y(), mfNormal_relative_to_lit_normal.z());
        
        if ((toVertex*mfNormal)<0.0f)
        {
            scatteredDir=whittedSpecReflectRay(toVertex, mfNormal);
        }
    }

    return scatteredDir;
}

//=======================================================================//
stitch::Photon stitch::BlinnPhongMaterial::scatterPhoton_direct(const Vec3 &normal, const Vec3 &worldPosition, const Photon &photon) const
{
    const float r=GlobalRand::uniformSampler();
    const float dRefl=dRefl_.cavrg();
    const float sRefl=sRefl_.cavrg();
    const float sTrans=sTrans_.cavrg();
    
    Vec3 scatteredDir;
    Colour_t scatteredEnergy;
    
    if (r<dRefl)
    {
        const Colour_t energyMask=dRefl_.rouletteMask(dRefl);//Colour_t::allOnes();
        scatteredEnergy=photon.energy_.cmult(energyMask);
        
        if (scatteredEnergy.lengthSq()>0.0f)
        {
            const Vec3 litNormal=((normal*photon.normDir_)<0.0f) ? normal : normal.zeroMinus();
            Vec3 orthVecA=litNormal.orthVec();
            orthVecA.normalise();
            const Vec3 orthVecB=stitch::Vec3::crossNormalised(litNormal, orthVecA);
            
            Vec3 scatteredDirLocal=Vec3::randCosineLobe(1.0f);
            scatteredDir=stitch::Vec3(orthVecA, orthVecB, litNormal,
                                      scatteredDirLocal.x(), scatteredDirLocal.y(), scatteredDirLocal.z());
        }
    } else if (r<(dRefl+sRefl))
    {
        const Colour_t energyMask=sRefl_.rouletteMask(sRefl);//Colour_t::allOnes();
        scatteredEnergy=photon.energy_.cmult(energyMask);
        
        if (scatteredEnergy.lengthSq()>0.0f)
        {
            while (scatteredDir.lengthSq()==0.0f)
            {
                const Vec3 litNormal=((normal*photon.normDir_)<0.0f) ? normal : normal.zeroMinus();
                Vec3 orthVecA=(litNormal.orthVec());
                orthVecA.normalise();
                const Vec3 orthVecB=stitch::Vec3::crossNormalised(litNormal, orthVecA);
                
                const Vec3 mfNormal_relative_to_lit_normal=Vec3::randCosineLobe(this->sExp_);
                
                const Vec3 mfNormal=stitch::Vec3(orthVecA, orthVecB, litNormal,
                                                 mfNormal_relative_to_lit_normal.x(), mfNormal_relative_to_lit_normal.y(), mfNormal_relative_to_lit_normal.z());
                
                //ToDo: Shadowing and masking to be investigated further for all methods of all BRDFs.
                if ((photon.normDir_*mfNormal)<0.0f)
                {
                    scatteredDir=whittedSpecReflectRay(photon.normDir_, mfNormal);
                }
            }
        }
    } else if (r<(dRefl+sRefl+sTrans))
    {
        const Colour_t energyMask=sTrans_.rouletteMask(sTrans);//Colour_t::allOnes();
        scatteredEnergy=photon.energy_.cmult(energyMask);
        if (scatteredEnergy.lengthSq()>0.0f)
        {
            while (scatteredDir.lengthSq()==0.0f)
            {
                const Vec3 litNormal=((normal*photon.normDir_)<0.0f) ? normal : normal.zeroMinus();
                Vec3 orthVecA=litNormal.orthVec();
                orthVecA.normalise();
                const Vec3 orthVecB=stitch::Vec3::crossNormalised(litNormal, orthVecA);
                
                const Vec3 mfNormal_relative_to_lit_normal=Vec3::randCosineLobe(this->sExp_);
                
                const Vec3 mfNormal=stitch::Vec3(orthVecA, orthVecB, litNormal,
                                                 mfNormal_relative_to_lit_normal.x(), mfNormal_relative_to_lit_normal.y(), mfNormal_relative_to_lit_normal.z());
                
                //ToDo: Shadowing and masking to be investigated further for all methods of all BRDFs.
                if ((photon.normDir_*mfNormal)<0.0f)
                {
                    scatteredDir=whittedSpecRefractRay(photon.normDir_, mfNormal);
                }
            }
        }
    }
    
    return Photon(worldPosition, scatteredDir, scatteredEnergy, photon.scatterCount_+1);
}
