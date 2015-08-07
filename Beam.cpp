/*
 * $Id: Beam.cpp 299 2015-08-07 14:57:10Z bernardt.duvenhage $
 */
/*
 *  BeamSegment.cpp
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

#include "Beam.h"
#include "Math/GlobalRand.h"
#include "Materials/DiffuseMaterial.h"

#include <iostream>

float *stitch::BeamSegment::volImageData_=nullptr;//This data is never deleted! I accept.

//=======================================================================//
stitch::Vec3 stitch::BeamSegment::gaussVolumeBarycentricRandomABC(const stitch::Vec3 &A, const stitch::Vec3 &B, const stitch::Vec3 &C, const float sdRad, const size_t numSamples)
{
    const stitch::Vec3 ANorm=A / sdRad;
    const stitch::Vec3 BNorm=B / sdRad;
    const stitch::Vec3 CNorm=C / sdRad;
    
    double prob=0.0f;
    
    for (size_t i=0; i<numSamples; ++i)
    {
        //const float r1=stitch::GlobalRand::uniformSampler();
        //const float r2=stitch::GlobalRand::uniformSampler();
        const float r1=stitch::GlobalRand::uniformSamplerFromArray();
        const float r2=stitch::GlobalRand::uniformSamplerFromArray();
        
        const float sqrtR1=sqrtf(r1);
        
        //Uniform sampling of triangle using barycentric coordinates.
        const float a=1.0f-sqrtR1;
        const float b=(1.0f-r2)*sqrtR1;
        const float c=r2*sqrtR1;
        
        const float px=(ANorm.x()*a + BNorm.x()*b + CNorm.x()*c);
        const float py=(ANorm.y()*a + BNorm.y()*b + CNorm.y()*c);
        const float pz=(ANorm.z()*a + BNorm.z()*b + CNorm.z()*c);
        
        prob+=exp(-(px*px+py*py+pz*pz)*0.5);
    }
	
	return (stitch::Vec3::cross(ANorm,BNorm,CNorm)*0.5f) * ((prob/(2.0f*((float)M_PI)))/numSamples);
}

//=======================================================================//
float stitch::BeamSegment::gaussVolumeRecurABCNorm(const stitch::Vec3 &ANorm, const float A0Sq, const float probA,
                                                   const stitch::Vec3 &BNorm, const float B0Sq, const float probB,
                                                   const stitch::Vec3 &CNorm, const float C0Sq, const float probC,
                                                   const float ABSq,
                                                   const float BCSq,
                                                   const float CASq,
                                                   const float areaABCSq)
{
    //Use barycentric coords.
    const float areaAB0Sq=stitch::Vec3::crossLengthSq(ANorm, BNorm)*0.25f;//0.25 = 0.5^2
    const float areaBC0Sq=stitch::Vec3::crossLengthSq(BNorm, CNorm)*0.25f;
    const float areaCA0Sq=stitch::Vec3::crossLengthSq(CNorm, ANorm)*0.25f;
    
    
    const stitch::Line lineAB(ANorm, BNorm);
    const stitch::Line lineBC(BNorm, CNorm);
    const stitch::Line lineCA(CNorm, ANorm);
    
    //Find distance of the specular path (0,0,0) from the triangle boundaries.
    stitch::Vec3 lineABNearestToPoint;
    const float dABSq=lineAB.calcDistToPointSq(stitch::Vec3(), lineABNearestToPoint);
    stitch::Vec3 lineBCNearestToPoint;
    const float dBCSq=lineBC.calcDistToPointSq(stitch::Vec3(), lineBCNearestToPoint);
    stitch::Vec3 lineCANearestToPoint;
    const float dCASq=lineCA.calcDistToPointSq(stitch::Vec3(), lineCANearestToPoint);
    
    
    stitch::Vec3 const *nearestToPoint=nullptr;
    
    
    if ((dABSq<dBCSq)&&(dABSq<dCASq))
    {
        nearestToPoint=&lineABNearestToPoint;
    } else
        if (dBCSq<dCASq)
        {
            nearestToPoint=&lineBCNearestToPoint;
        } else
        {
            nearestToPoint=&lineCANearestToPoint;
        }
    
    
    float probRef=0.0f;
    
    if (((areaAB0Sq+areaBC0Sq)>areaABCSq) ||
        ((areaBC0Sq+areaCA0Sq)>areaABCSq) ||
        ((areaCA0Sq+areaAB0Sq)>areaABCSq) )
    {//Case 1/2: Gaussian-peak / specular-path / (0,0,0) outside of triangle.
        //=== Find edge ST that we are closest to outside of the triangle ===
        probRef=gaussProbVol(nearestToPoint->lengthSq());
    } else
    {//Case 2/2: Gaussian-peak / specular-path / (0,0,0) inside of triangle.
        /*
         if (nearestToPoint->lengthSq()>100.0f)
         {//The triangle domain contains the entire distribution.
         return 1.0;
         } else
         */
        {//The gaussian peak becomes the reference.
            probRef=gaussProbVol(0.0f);//At distribution mean.
        }
    }
    
    
    const float prob=(probA+probB+probC)/3.0f;
    const float probErrorThreshold=0.0005f;//! @todo Can this threshold be dynamically maximised.
    
    
    if (fabsf(prob-probRef)<probErrorThreshold)
    {
        return sqrtf(areaABCSq) * prob;
    } else
    {//Divide triangle into four similar triangles and recursively evaluate volume.
        const stitch::Vec3 ABNorm=stitch::Vec3::avrg(ANorm, BNorm);
        const stitch::Vec3 BCNorm=stitch::Vec3::avrg(BNorm, CNorm);
        const stitch::Vec3 CANorm=stitch::Vec3::avrg(CNorm, ANorm);
        
        const float AB0Sq=ABNorm.lengthSq();
        const float BC0Sq=BCNorm.lengthSq();
        const float CA0Sq=CANorm.lengthSq();
        
        const float halfABSq=0.25f*ABSq;
        const float halfBCSq=0.25f*BCSq;
        const float halfCASq=0.25f*CASq;
        
        const float subdvAreaABCSq=areaABCSq*0.0625f; //0.0625f = 0.25 ^ 2
        
        const float probAB=gaussProbVol(AB0Sq);
        const float probBC=gaussProbVol(BC0Sq);
        const float probCA=gaussProbVol(CA0Sq);
        
        //=== Recurse over four sub-triangles ===
        return gaussVolumeRecurABCNorm(ANorm, A0Sq, probA,
                                       ABNorm, AB0Sq, probAB,
                                       CANorm, CA0Sq, probCA,
                                       halfABSq, halfBCSq, halfCASq,
                                       subdvAreaABCSq)+
        gaussVolumeRecurABCNorm(ABNorm, AB0Sq, probAB,
                                BNorm, B0Sq, probB,
                                BCNorm, BC0Sq, probBC,
                                halfABSq, halfBCSq, halfCASq,
                                subdvAreaABCSq)+
        gaussVolumeRecurABCNorm(CANorm, CA0Sq, probCA,
                                BCNorm, BC0Sq, probBC,
                                CNorm, C0Sq, probC,
                                halfABSq, halfBCSq, halfCASq,
                                subdvAreaABCSq)+
        gaussVolumeRecurABCNorm(BCNorm, BC0Sq, probBC,
                                CANorm, CA0Sq, probCA,
                                ABNorm, AB0Sq, probAB,
                                halfABSq, halfBCSq, halfCASq,
                                subdvAreaABCSq);
        //=======================================
    }
}

//=======================================================================//
stitch::Vec3 stitch::BeamSegment::gaussVolumeBarycentricRandomAB(const stitch::Vec3 &A, const stitch::Vec3 &B, const float sdRad)
{
    const stitch::Vec3 ANorm=A / sdRad;
    const stitch::Vec3 BNorm=B / sdRad;
    
    float prob=0.0f;
    
    const size_t numSamples=5000;
    for (size_t i=0; i<numSamples; ++i)
    {
        //const float r1=stitch::GlobalRand::uniformSampler();
        //const float r2=stitch::GlobalRand::uniformSampler();
        const float r1=stitch::GlobalRand::uniformSamplerFromArray();
        const float r2=stitch::GlobalRand::uniformSamplerFromArray();
        
        const float sqrtR1=sqrtf(r1);
        
        //Uniform sampling of triangle using barycentric coordinates.
        const float a=1.0f-sqrtR1;
        const float b=(1.0f-r2)*sqrtR1;
        
        const float px=(ANorm.x()*a + BNorm.x()*b);
        const float py=(ANorm.y()*a + BNorm.y()*b);
        const float pz=(ANorm.z()*a + BNorm.z()*b);
        
        prob+=expf(-(px*px+py*py+pz*pz)*0.5f);
    }
	
	return ((ANorm^BNorm)*0.5f) * (prob/(2.0f*((float)M_PI)))/numSamples;
}

float stitch::BeamSegment::gaussVolumeBarycentricRandomAB(const float ANorm, const float BNorm, const float theta)
{
	if ((ANorm!=0.0f)&&(BNorm!=0.0f))
	{
		return (gaussVolumeBarycentricRandomAB(Vec3(0.0f, ANorm, 0.0f), Vec3(BNorm*sinf(theta), BNorm*cosf(theta), 0.0f), 1.0f).length());
	} else
	{
		return 0.0f;
	}
}


//=======================================================================//
void stitch::BeamSegment::generateVolumeTexture()
{
	std::cout << "  Loading volume-under-2D-Gaussian texture...";
	std::cout.flush();
    
	//=== Volume under prob dist ===//
    ssize_t dim=512;
    ssize_t dimZ=256;
	
    volImageData_=new float[dim * dim * dimZ];
	
	{
        FILE *fp=fopen("Data/gTable.dat", "rb");
		if (fp!=nullptr)
		{
			float *volData=volImageData_;
			
			for (ssize_t iz=0; iz<dimZ; ++iz)
			{
				fread((unsigned char *)volData, sizeof(float), dim*dim, fp);
				volData+=dim*dim;
			}
            
			fclose(fp);
		} else
		{
			std::cout << "file not found! Generating...\n";
			std::cout.flush();
			
			float* volData=volImageData_;
			
			for (ssize_t iz=0; iz<dimZ; ++iz)
			{
                double theta=(((double)iz)/dimZ) * ((float)M_PI_2);        // /(dimZ-1) instead of /(dimZ) removes some noise.
				
				for (ssize_t iy=0; iy<dim; ++iy)
				{
					double A=(((double)iy)/dim) * SD_TEX_MAX;
					
					for (ssize_t ix=0; ix<dim; ++ix)
					{
						double B=(((double)ix)/dim) * SD_TEX_MAX;
						
						double volume=gaussVolumeBarycentricRandomAB(A, B, theta);
						
						*(volData++)=(float)volume;
					}
				}
                
				std::cout << "    " << 100.0f*iz/((float)dimZ-1.0f) << "%\n";
				std::cout.flush();
			}
			
			volData=volImageData_;
            
            FILE *fp=fopen("Data/gTable.dat", "wb");
			
            for (ssize_t iz=0; iz<dimZ; ++iz)
			{
				fwrite((unsigned char *)volData, sizeof(float), dim*dim, fp);
				volData+=dim*dim;
			}
            
			fclose(fp);
			
		}
	}
	
	//=== Volume under prob dist ===//
	std::cout << "  done.\n";
	std::cout.flush();
}


//=======================================================================//
stitch::BeamSegment::BeamSegment(const Plane generator,
                                 std::vector<Vec3> &&vertices,
                                 std::vector<Vec3> &&vertVects,
                                 const float glossySDRad, const Colour_t fluxSPD) :
BVBrush_(new stitch::DiffuseMaterial(stitch::Colour_t(1.0f, 1.0f, 1.0f))),
generator_(generator),
glossySDRad_(glossySDRad), fluxSPD_(fluxSPD),
vertices_(vertices), vertVects_(vertVects),
hasEndCap_(false),
endCap_(stitch::Vec3(), 0.0f),
BVUpdated_(false)
{
}

//=======================================================================//
stitch::BeamSegment::BeamSegment(const BeamSegment &lValue) :
BVBrush_(lValue.BVBrush_),
generator_(lValue.generator_),
glossySDRad_(lValue.glossySDRad_), fluxSPD_(lValue.fluxSPD_),
vertices_(lValue.vertices_), vertVects_(lValue.vertVects_),
hasEndCap_(lValue.hasEndCap_),
endCap_(lValue.endCap_),
endCapVertices_(lValue.endCapVertices_),
BVUpdated_(lValue.BVUpdated_)
{
}

//=======================================================================//
stitch::BeamSegment & stitch::BeamSegment::operator=( const BeamSegment &lValue)
{
    BVBrush_=lValue.BVBrush_;
    
    generator_=lValue.generator_;
    
    endCap_=lValue.endCap_;
    endCapVertices_=lValue.endCapVertices_;
    hasEndCap_=lValue.hasEndCap_;
    
    glossySDRad_=lValue.glossySDRad_;
    fluxSPD_=lValue.fluxSPD_;
    
    vertices_=lValue.vertices_;
    vertVects_=lValue.vertVects_;
    
    BVUpdated_=lValue.BVUpdated_;
    
    return (*this);
}



//=======================================================================//
float stitch::BeamSegment::gaussProbLookUpOptimisedAB(const float lengthANorm, const float lengthBNorm, const float theta)
{
    if (volImageData_!=nullptr)
    {
        const float indexA=(stitch::MathUtil::min(lengthANorm/SD_TEX_MAX, 1.0f));
        const float indexB=(stitch::MathUtil::min(lengthBNorm/SD_TEX_MAX, 1.0f));
        const float indexTheta=stitch::MathUtil::min(theta/((float)M_PI_2), 1.0f);
        
        //===//
        const size_t intIndexA=indexA * 511.0f+0.5f;
        const size_t intIndexB=indexB * 511.0f+0.5f;
        const size_t intIndexTheta=indexTheta * 255.0f+0.5f;
        const size_t offset=(intIndexA + (intIndexB<<9) + (intIndexTheta<<18));
        
        const float prob=*(volImageData_+offset);
        //===//
        
        return  prob;
    } else
    {
        return 0.0f;
    }
}



void stitch::BeamSegment::updateBV()
{
    if (!BVUpdated_)
    {
        size_t numVertices=vertices_.size();
        std::vector<stitch::Vec3> vertexCloud;
        
        //Add generator vertices to vertex cloud.
        vertexCloud=vertices_;
        
        //=== Generate glossy BV end-cap vertices ===
        stitch::Plane BVEndCap=hasEndCap_ ? endCap_ : stitch::Plane(vertVects_[0]*(-1.0f), -10000.0f - vertices_[0]*vertVects_[0]);
        
        std::vector<stitch::Vec3> BVEndCapVertex;
        std::vector<float> BVEndCapVertexDistance;
        
#ifdef USE_CXX11
        BVEndCapVertex.emplace_back();
#else
        BVEndCapVertex.push_back(stitch::Vec3());
#endif
        
        BVEndCapVertexDistance.push_back(0.0f);
        const float sdCutOff=2.5f;//Note: This should always be 2.5 from the cosine approximation made.
        const float vertexOffsetAngle=(glossySDRad_ * sdCutOff);
        const float cosVertexOffsetAngle=cosf(vertexOffsetAngle);
        const float sinVertexOffsetAngle=sqrtf(1.0f-cosVertexOffsetAngle*cosVertexOffsetAngle);
        
        for (size_t vertNum=1; vertNum<numVertices; ++vertNum)
        {
            BVEndCapVertexDistance.push_back(BVEndCap.calcIntersectDist(vertices_[vertNum], vertVects_[vertNum]));
            BVEndCapVertex.push_back(vertices_[vertNum] + vertVects_[vertNum]*BVEndCapVertexDistance[vertNum]);
            
            BVEndCapVertex[0]+=BVEndCapVertex[vertNum];
        }
        BVEndCapVertex[0]/=numVertices;
        
        
        //Add end-cap vertices to vertex cloud.
        for (size_t vertNum=1; vertNum<numVertices; ++vertNum)
        {
            stitch::Vec3 vertexOffsetDir=BVEndCapVertex[vertNum] - BVEndCapVertex[0];
            vertexOffsetDir.normalise();
            
            const stitch::Vec3 vertexOffsetOrthDir=stitch::Vec3::crossNormalised(vertexOffsetDir, BVEndCap.normal_);
            
            const float cosPhi0=vertexOffsetDir * (BVEndCapVertex[vertNum] - vertices_[vertNum]).normalised();
            const float phi0=acos(fabsf(cosPhi0));
            
            float phi1=phi0+vertexOffsetAngle;
            const float sinPhi1=sin(phi1);
            
            const stitch::Vec3 endCapVertexOffset=vertexOffsetDir * ((BVEndCapVertexDistance[vertNum]/(sinPhi1+0.000001f)) * sinVertexOffsetAngle);
            vertexCloud.push_back(BVEndCapVertex[vertNum]+endCapVertexOffset);
            
            const stitch::Vec3 endCapVertexOrthOffset=vertexOffsetOrthDir * ((BVEndCapVertexDistance[vertNum]/(sinPhi1+0.000001f)) * sinVertexOffsetAngle);
            vertexCloud.push_back(BVEndCapVertex[vertNum]+endCapVertexOrthOffset);
            vertexCloud.push_back(BVEndCapVertex[vertNum]-endCapVertexOrthOffset);
        }
        
        //=== ===
        
        BVBrush_=stitch::Brush(new stitch::DiffuseMaterial(stitch::Colour_t(1.0f, 1.0f, 1.0f)), vertexCloud, 12);
        BVBrush_.updateLinesVerticesAndBoundingVolume(true);
        BVBrush_.optimiseFaceOrder();
        BVUpdated_=true;
    }
}





