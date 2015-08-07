/*
 * $Id: Beam.h 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  BeamSegment.h
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

#ifndef STITCH_BEAM_SEGMENT_H
#define STITCH_BEAM_SEGMENT_H

const float SD_TEX_MAX=7.5f;

namespace stitch {
    class BeamSegment;
}

#include "Math/Vec3.h"
#include "Math/Colour.h"
#include "Math/Plane.h"
#include "Objects/BrushModel.h"

namespace stitch {
    
    //! Models a single light beam segment with k-DOP Brush bounding volume.
    class BeamSegment
    {
    public:
        /*! Construct beam given the generator plane and flux. Move construction of std::vectors.
         @param generator The plane of the generator polygon.
         @param vertices Beam generator plane vertices.
         @param vertVects Beam vertex vectors.
         @param glossySDRad The glossy scatter standard deviation.
         @param fluxSPD The flux contained within the specular beam. Of type Colour_t.*/
        BeamSegment(const Plane generator,
                    std::vector<Vec3> &&vertices,
                    std::vector<Vec3> &&vertVects,
                    const float glossySDRad, const Colour_t fluxSPD);
        
        
        /*! Copy constructor.*/
        BeamSegment(const BeamSegment &lValue);
        
        /*! Assignment operator.*/
        BeamSegment & operator=( const BeamSegment &lValue);
        
        ~BeamSegment() {}
        
        /*! Add an end cap plane to the beam segment. */
        void setEndCap(const Plane &endCap, const std::vector<Vec3> &vertices)
        {
            endCap_=endCap;
            endCapVertices_=vertices;
            hasEndCap_=true;
            
            BVUpdated_=false;
        }
        
        //=======================================================================//
        /*! Test whether or not a point is within the spec beam segment geometry i.e. within the specular beam segment. */
        inline bool pointInSpecBeamSegment(const Vec3 &point, float &o_specFluxArea, stitch::Vec3 &o_specScatterDir) const
        {
            bool inBeamSegment=false;
            o_specFluxArea=0.0f;
            
            if (( (point*generator_.normal_)>(generator_.d_ + 0.001f) ) &&
                ( (!hasEndCap_) || ((point*endCap_.normal_) > endCap_.d_ )) )//0.001 to prevent self illumination...
            {//point between generator and endcap.
                const Plane fluxPlane(vertVects_[0], point);
                const size_t numVertices=vertices_.size();
                
                if (numVertices==4)
                {
                    const Vec3 pVO(vertices_[1], vertVects_[1], fluxPlane.calcIntersectDist(vertices_[1], vertVects_[1]));
                    const Vec3 pVB(vertices_[2], vertVects_[2], fluxPlane.calcIntersectDist(vertices_[2], vertVects_[2]));
                    const Vec3 pVA(vertices_[3], vertVects_[3], fluxPlane.calcIntersectDist(vertices_[3], vertVects_[3]));
                    
                    const Vec3 normal_unnormalised=stitch::Vec3::cross(pVA, pVB, pVO);
                    const float A=normal_unnormalised.length() * 0.5f;
                    const float ASq=A*A;
                    o_specFluxArea=A;
                    
                    //Barycentric test...
                    const Vec3 AB=pVB-pVA;
                    Vec3 orthAB=(normal_unnormalised ^ AB);
                    
                    const Vec3 BO=pVO-pVB;
                    Vec3 orthBO=(normal_unnormalised ^ BO);
                    
                    const Vec3 OA=pVA-pVO;
                    Vec3 orthOA=(normal_unnormalised ^ OA);
                    
                    const float a=0.25f*fabsf((point-pVA)*orthAB);
                    const float b=0.25f*fabsf((point-pVB)*orthBO);
                    const float c=0.25f*fabsf((point-pVO)*orthOA);
                    
                    o_specScatterDir=stitch::Vec3::sumScaleAndNormalise(vertVects_[1], a,
                                                                        vertVects_[2], c,
                                                                        vertVects_[3], b);
                    
                    if (((a+b)<=ASq)&&
                        ((a+c)<=ASq)&&
                        ((b+c)<=ASq))
                    {
                        inBeamSegment=true;
                    }
                } else
                {
                    const Vec3 pVO(vertices_[0] + vertVects_[0]*fluxPlane.calcIntersectDist(vertices_[0], vertVects_[0]));
                    const Vec3 pVOVect=vertVects_[0];
                    
                    Vec3 pVA(vertices_[numVertices-1] + vertVects_[numVertices-1]*fluxPlane.calcIntersectDist(vertices_[numVertices-1], vertVects_[numVertices-1]));
                    Vec3 pVAVect=vertVects_[numVertices-1];
                    
                    Vec3 pVB;
                    Vec3 pVBVect;
                    
                    for (size_t vertexNum=1; vertexNum<numVertices; ++vertexNum)
                    {
                        pVB=pVA;
                        pVBVect=pVAVect;
                        
                        pVA=Vec3(vertices_[vertexNum] + vertVects_[vertexNum]*fluxPlane.calcIntersectDist(vertices_[vertexNum], vertVects_[vertexNum]));
                        pVAVect=vertVects_[vertexNum];
                        
                        const Vec3 normal_unnormalised=stitch::Vec3::cross(pVA, pVB, pVO);
                        const float A=normal_unnormalised.length() * 0.5f;
                        o_specFluxArea+=A;
                        
                        if (!inBeamSegment)
                        {//Only test for in beam segment until it is known. The rest of the time only the area id updated!
                            //Barycentric test...
                            const Vec3 AB=pVB-pVA;
                            const Vec3 orthAB=stitch::Vec3::crossNormalised(normal_unnormalised, AB);
                            const float heightAB=fabsf((point-pVA)*orthAB);
                            
                            const Vec3 BO=pVO-pVB;
                            const Vec3 orthBO=stitch::Vec3::crossNormalised(normal_unnormalised, BO);
                            const float heightBO=fabsf((point-pVB)*orthBO);
                            
                            const Vec3 OA=pVA-pVO;
                            const Vec3 orthOA=stitch::Vec3::crossNormalised(normal_unnormalised, OA);
                            const float heightOA=fabsf((point-pVO)*orthOA);
                            
                            const float a=0.5f * AB.length() * heightAB;
                            const float b=0.5f * BO.length() * heightBO;
                            const float c=0.5f * OA.length() * heightOA;
                            
                            //Use last valid spec scatter vect.
                            o_specScatterDir=stitch::Vec3::sumScaleAndNormalise(pVOVect, a,
                                                                                pVBVect, c,
                                                                                pVAVect, b);
                            
                            if (((a+b)<=A)&&
                                ((a+c)<=A)&&
                                ((b+c)<=A))
                            {
                                inBeamSegment=true;
                            }
                        }
                    }
                }
            }
            
            return inBeamSegment;
        }
        
        //=======================================================================//
        /*! Test whether or not a point is within the spec beam segment geometry i.e. within the specular beam segment. */
        inline bool pointInSpecBeamSegment(const Vec3 &point) const
        {
            float specFluxArea;//temp storage for values to be ignored!
            stitch::Vec3 specScatterDir;//temp storage for values to be ignored!
            
            return pointInSpecBeamSegment(point, specFluxArea, specScatterDir);
        }
        
        //=======================================================================//
        /*! Calculate the flux density probability at a point within the glossy beam segment.*/
        inline float calcGlossyFluxProb(const Vec3 &point) const
        {
            float prob=0.0f;
            const size_t numVertices=vertices_.size();
            
            if ((numVertices>=4)&&((point*generator_.normal_) > (generator_.d_+0.001f))&& //+0.001f to cull the beam segment generator with everything behind it.
                ( (!hasEndCap_) || ((point*endCap_.normal_)>endCap_.d_)) )
            {
                const Plane fluxPlane(vertVects_[0], point*vertVects_[0]);
                
                //===
                const Vec3 fluxPlaneVertexA(vertices_[1], vertVects_[1], fluxPlane.calcIntersectDist(vertices_[1], vertVects_[1]));
                const Vec3 fluxPlaneVertexB(vertices_[2], vertVects_[2], fluxPlane.calcIntersectDist(vertices_[2], vertVects_[2]));
                const Vec3 fluxPlaneVertexC(vertices_[3], vertVects_[3], fluxPlane.calcIntersectDist(vertices_[3], vertVects_[3]));
                
                //===
                const float rGlossySDRad_=1.0f/glossySDRad_;
                
                Vec3 dPointTemp=point-vertices_[1];
                dPointTemp.normalise();
                
                const float fluxPlaneThetaVertexALength=acosf(stitch::MathUtil::clamp(vertVects_[1] * dPointTemp, -1.0f, 1.0f)) * rGlossySDRad_;
                Vec3 fluxPlaneThetaVertexA=(fluxPlaneVertexA-point);
                fluxPlaneThetaVertexA.normalise();
                fluxPlaneThetaVertexA*=fluxPlaneThetaVertexALength;
                
                
                dPointTemp=point-vertices_[2];
                dPointTemp.normalise();
                
                const float fluxPlaneThetaVertexBLength=acosf(stitch::MathUtil::clamp(vertVects_[2] * dPointTemp, -1.0f, 1.0f)) * rGlossySDRad_;
                Vec3 fluxPlaneThetaVertexB=(fluxPlaneVertexB-point);
                fluxPlaneThetaVertexB.normalise();
                fluxPlaneThetaVertexB*=fluxPlaneThetaVertexBLength;
                
                
                dPointTemp=point-vertices_[3];
                dPointTemp.normalise();
                
                const float fluxPlaneThetaVertexCLength=acosf(stitch::MathUtil::clamp(vertVects_[3] * dPointTemp, -1.0f, 1.0f)) * rGlossySDRad_;
                Vec3 fluxPlaneThetaVertexC=(fluxPlaneVertexC-point);
                fluxPlaneThetaVertexC.normalise();
                fluxPlaneThetaVertexC*=fluxPlaneThetaVertexCLength;
                
                //prob=gaussVolumeGaussMC(fluxPlaneThetaVertexA,
                //                      fluxPlaneThetaVertexB,
                //                      fluxPlaneThetaVertexC);
                //OR
                //prob=gaussVolumeGaussSubd(fluxPlaneThetaVertexA,
                //                           fluxPlaneThetaVertexB,
                //                           fluxPlaneThetaVertexC);
                //OR
                prob=gaussVolumeGaussRecur(fluxPlaneThetaVertexA,
                                           fluxPlaneThetaVertexALength,
                                           fluxPlaneThetaVertexB,
                                           fluxPlaneThetaVertexBLength,
                                           fluxPlaneThetaVertexC,
                                           fluxPlaneThetaVertexCLength,
                                           fluxPlane.normal_);
                //OR
                //prob=gaussVolumeRecurABCNormInit(fluxPlaneThetaVertexA,
                //fluxPlaneThetaVertexB,
                //fluxPlaneThetaVertexC);
                //OR
                //prob=gaussVolumeLookUpOptimisedABC(fluxPlaneThetaVertexA*glossySDRad_, fluxPlaneThetaVertexB*glossySDRad_, fluxPlaneThetaVertexC*glossySDRad_, glossySDRad_).length();
                //OR
                //prob=gaussVolumeBarycentricRandomABC(fluxPlaneThetaVertexA*glossySDRad_, fluxPlaneThetaVertexB*glossySDRad_, fluxPlaneThetaVertexC*glossySDRad_, glossySDRad_, 10000).length();
                
            }
            
            return prob;
        }
        
        //! Make sure that the generator and end-cap planes have the correct orientation.
        inline void sanitise()
        {
            if ((vertVects_[0] * generator_.normal_)<0.0f)
            {
                generator_.normal_*=-1.0f;
                generator_.d_*=-1.0f;
            }
            
            if ((vertVects_[0] * endCap_.normal_)>0.0f)
            {
                endCap_.normal_*=-1.0f;
                endCap_.d_*=-1.0f;
            }
        }
        
        /*! Static method to load or re-generate the volume lookup table for the gaussVolumeLookUp... methods. */
        static void generateVolumeTexture();
        
        /*! Update the bounding volume of the beam segment */
        void updateBV();
        
        /*! Test whether or not a point is within the glossy BeamSegment's bounding volume. */
        inline bool pointInBV(const Vec3 &point) const
        {
            return BVBrush_.pointInBrush(point);
        }
        
        /*! Get a const ref to the BVBrush */
        const stitch::Brush &getBVBrush() const
        {
            return BVBrush_;
        }
        
        
        
        //=== Use table to lookup the probability volume ===//
        inline static float getVolImageValue(const float indexA, const float indexB, const float indexTheta)
        {
            size_t intIndexA0=indexA;
            size_t intIndexB0=indexB;
            size_t intIndexTheta0=indexTheta;
            
            size_t intIndexA1=intIndexA0+1;
            size_t intIndexB1=intIndexB0+1;
            size_t intIndexTheta1=intIndexTheta0+1;
            
            if (intIndexA0>511) intIndexA0=511;
            if (intIndexB0>511) intIndexB0=511;
            if (intIndexTheta0>255) intIndexTheta0=255;
            
            if (intIndexA1>511) intIndexA1=511;
            if (intIndexB1>511) intIndexB1=511;
            if (intIndexTheta1>255) intIndexTheta1=255;
            
            const float fracIndexA=indexA - intIndexA0;
            const float fracIndexB=indexB - intIndexB0;
            const float fracIndexTheta=indexTheta - intIndexTheta0;
            
            const size_t offset000=(intIndexA0 + (intIndexB0<<9) + (intIndexTheta0<<18));
            const size_t offset100=(intIndexA1 + (intIndexB0<<9) + (intIndexTheta0<<18));
            const size_t offset110=(intIndexA1 + (intIndexB1<<9) + (intIndexTheta0<<18));
            const size_t offset010=(intIndexA0 + (intIndexB1<<9) + (intIndexTheta0<<18));
            const size_t offset001=(intIndexA0 + (intIndexB0<<9) + (intIndexTheta1<<18));
            const size_t offset101=(intIndexA1 + (intIndexB0<<9) + (intIndexTheta1<<18));
            const size_t offset111=(intIndexA1 + (intIndexB1<<9) + (intIndexTheta1<<18));
            const size_t offset011=(intIndexA0 + (intIndexB1<<9) + (intIndexTheta1<<18));
            
            float data000=*(volImageData_+offset000);
            float data100=*(volImageData_+offset100);
            float data110=*(volImageData_+offset110);
            float data010=*(volImageData_+offset010);
            float data001=*(volImageData_+offset001);
            float data101=*(volImageData_+offset101);
            float data111=*(volImageData_+offset111);
            float data011=*(volImageData_+offset011);
            
            float dataX00=data000*(1.0f-fracIndexA) + data100*fracIndexA;
            float dataX10=data010*(1.0f-fracIndexA) + data110*fracIndexA;
            float dataX01=data001*(1.0f-fracIndexA) + data101*fracIndexA;
            float dataX11=data011*(1.0f-fracIndexA) + data111*fracIndexA;
            
            float dataXX0=dataX00*(1.0f-fracIndexB) + dataX10*fracIndexB;
            float dataXX1=dataX01*(1.0f-fracIndexB) + dataX11*fracIndexB;
            
            float dataXXX=dataXX0*(1.0f-fracIndexTheta) + dataXX1*fracIndexTheta;
            
            return dataXXX;
        }
        
        inline static Vec3 gaussVolumeLookUpOptimisedAB(const Vec3 &A, const Vec3 &B, const float sdRad)
        {
            stitch::Vec3 volume=(A ^ B);
            
            if ((volImageData_!=nullptr)&&(volume.lengthSq()>0.0f))
            {
                const float lengthA=A.length();
                const float lengthB=B.length();
                
                const float cosTheta=(A*B)/(lengthA*lengthB);
                const float theta=acosf(stitch::MathUtil::clamp(cosTheta, -1.0f, 1.0f));//Do a clamp on behalf of acos in case of numerical inaccuracies which will cause cosTheta to be outside of [-1,1].
                
                float prob=0.0f;
                
                if (theta<((float)M_PI_2))
                {//90 deg or smaller wedge.
                    const float recipSD=1.0f/(SD_TEX_MAX*sdRad);
                    const float indexA=(stitch::MathUtil::min(lengthA*recipSD, 1.0f));
                    const float indexB=(stitch::MathUtil::min(lengthB*recipSD, 1.0f));
                    const float indexTheta=theta*((float)M_2_PI);
                    
                    //===//
                    /*
                     const size_t intIndexA=indexA * 511.0f+0.5f;
                     const size_t intIndexB=indexB * 511.0f+0.5f;
                     const size_t intIndexTheta=indexTheta * 255.0f+0.5f;
                     
                     const size_t offset=(intIndexA + (intIndexB<<9) + (intIndexTheta<<18));
                     prob=*(volImageData_+offset);
                     */
                    prob=getVolImageValue(indexA*512.0f, indexB*512.0f, indexTheta*255.0f);
                } else
                {//Bigger than 90 deg wedge.
                    
                    const stitch::Vec3 C=(B-A)*(lengthA/(lengthA+lengthB))+A;
                    const float lengthC=C.length();
                    const float halfTheta=theta*0.5f;
                    
                    const float recipSD=1.0f/(SD_TEX_MAX*sdRad);
                    const float indexA=(stitch::MathUtil::min(lengthA*recipSD, 1.0f));
                    const float indexB=(stitch::MathUtil::min(lengthB*recipSD, 1.0f));
                    const float indexC=(stitch::MathUtil::min(lengthC*recipSD, 1.0f));
                    const float indexHalfTheta=halfTheta*((float)M_2_PI);
                    
                    //===//
                    
                    //                    const size_t intIndexA=indexA * 511.0f+0.5f;
                    //                    const size_t intIndexB=indexB * 511.0f+0.5f;
                    //                    const size_t intIndexC=indexC * 511.0f+0.5f;
                    //                    const size_t intIndexHalfTheta=indexHalfTheta * 255.0f+0.5f;
                    //
                    //                    const size_t offsetA=(intIndexA + (intIndexC<<9) + (intIndexHalfTheta<<18));
                    //                    prob=*(volImageData_+offsetA);
                    //
                    //                    const size_t offsetB=(intIndexB + (intIndexC<<9) + (intIndexHalfTheta<<18));
                    //                    prob+=*(volImageData_+offsetB);
                    
                    prob=getVolImageValue(indexA*512.0f, indexC*512.0f, indexHalfTheta*255.0f);
                    prob+=getVolImageValue(indexC*512.0f, indexB*512.0f, indexHalfTheta*255.0f);
                }
                //===//
                
                volume.normalise();
                volume*=prob;
                
                return  volume;
            } else
            {
                return stitch::Vec3(0.0f, 0.0f, 0.0f);
            }
        }
        
        
        inline static Vec3 gaussVolumeLookUpOptimisedABC(const Vec3 &A, const Vec3 &B, const Vec3 &C, const float sdRad)
        {
            return stitch::Vec3::sum(gaussVolumeLookUpOptimisedAB(A,B,sdRad),
                                     gaussVolumeLookUpOptimisedAB(B,C,sdRad),
                                     gaussVolumeLookUpOptimisedAB(C,A,sdRad));
        }
        //=== ===
        
        
        //=== Monte Carlo calculation of the probability volume ===
        static Vec3 gaussVolumeBarycentricRandomAB(const Vec3 &A, const Vec3 &B, const float sdRad);
        static float gaussVolumeBarycentricRandomAB(const float ANorm, const float BNorm, const float theta);
        static Vec3 gaussVolumeBarycentricRandomABC(const Vec3 &A, const Vec3 &B, const Vec3 &C, const float sdRad, const size_t numSamples);
        //=== ===
        
        static inline float gaussProbVol(const float rSq)
        {
            return expf(-rSq*0.5f);
        }
        
        //=== Recursive devide and conquer caculation of the probability volume ===
        static float gaussVolumeRecurABCNormInit(const Vec3 &ANorm, const Vec3 &BNorm, const Vec3 &CNorm)
        {
            const float A0Sq=ANorm.lengthSq();
            const float B0Sq=BNorm.lengthSq();
            const float C0Sq=CNorm.lengthSq();
            
            return  (0.5f*((float)M_1_PI)) * gaussVolumeRecurABCNorm(ANorm, A0Sq, gaussProbVol(A0Sq),
                                                                     BNorm, B0Sq, gaussProbVol(B0Sq),
                                                                     CNorm, C0Sq, gaussProbVol(C0Sq),
                                                                     stitch::Vec3::calcDistToPointSq(BNorm, ANorm),
                                                                     stitch::Vec3::calcDistToPointSq(CNorm, BNorm),
                                                                     stitch::Vec3::calcDistToPointSq(ANorm, CNorm),
                                                                     stitch::Vec3::crossLengthSq(ANorm, BNorm, CNorm)*0.25f);
        }
        
        static float gaussVolumeRecurABCNorm(const Vec3 &ANorm, const float A0Sq, const float probA,
                                             const Vec3 &BNorm, const float B0Sq, const float probB,
                                             const Vec3 &CNorm, const float C0Sq, const float probC,
                                             const float ABSq,
                                             const float BCSq,
                                             const float CASq,
                                             const float areaABCSq);
        //=== ===
        
        
        //!Cumulative probability distribution of approx. 2D Gaussian. It's a Bundt! Well... almost.
        inline static float CDF(const float phi)
        {
#define SINECDFAPPROX
#ifdef SINECDFAPPROX
            //A whole lot of constant folding going on!
            
            const float B=float(M_PI/2.5)*phi;
            constexpr float rA=float(2.5/M_PI);
            
            return (phi<2.5f) ? ((phi*phi)*0.25f + (rA*rA*0.5f)*cosf(B)+(rA*0.5f)*phi*sinf(B) - (rA*rA*0.5f)) : ((float)(1.5625 - 6.25/(M_PI*M_PI)));//0.9292426
#else//Numerical solution.
            const float dr=r/25.0f;
            float cdf = 0.0f;
            
            for (float ir=dr*0.5f; ir<r; ir+=dr)
            {
                //#define COSINEPDFAPPROX
#ifdef COSINEPDFAPPROX
                const float p=ir<2.5f ? (1.0/(4.0*M_PI))*(1.0f + cosf(ir * (M_PI/2.5f))): 0.0f;
#else
                const float p=(1.0f/((float)(2.0*M_PI))) * exp(-0.5f*ir*ir);
#endif
                cdf += p * dr * ((float)(2.0*M_PI)) * ir;
            }
            
            return cdf;
#endif
        }
        
        
        struct GaussLine
        {
        public:
            GaussLine(const Vec3 P, const Vec3 Q, const float scale) :
            P_(P), Q_(Q), scale_(scale)
            {}
            
            inline void invert()
            {
                std::swap(P_,Q_);
                scale_*=-1.0f;
            }
            
            Vec3 P_;
            Vec3 Q_;
            float scale_;
        };
        
        //===========================================================================//
        //===========================================================================//
        /*!
         \brief Calculate the volume over ABC using a boundary line integral and Gauss's divergence theorem.
         */
        static float gaussVolumeGaussMC(const Vec3 &A, const Vec3 &B, const Vec3 &C)
        {
            float prob=0.0f;
            
            const size_t numSamplesPerUnit=1000;
            
            const Vec3 polyNormal=stitch::Vec3::crossNormalised(A, B, C);//Assume that triangle ABC has some area.
            
            std::vector<GaussLine> lineVector;
            lineVector.reserve(3);
            
            lineVector.emplace_back(A, B, 1.0f);
            lineVector.emplace_back(B, C, 1.0f);
            lineVector.emplace_back(C, A, 1.0f);
            
            for (const auto & line : lineVector)
            {
                //S:=||PQ||
                //T point on line PQ.
                //R:=||T||
                
                const stitch::Vec3 PQ(line.P_, line.Q_);
                Vec3 lineNormalTimesS(PQ^polyNormal);//PQ orthogonal to polyNormal -> sin(theta)=1.0.
                lineNormalTimesS*=line.scale_;
                
                if ((line.P_ * lineNormalTimesS)!=0.0f)
                {
                    const size_t numSamples=numSamplesPerUnit * PQ.length();
                    const float dt=1.0f / ((float)numSamples);
                    
                    for (size_t i=0; i<numSamples; ++i)
                    {
                        const float t=stitch::GlobalRand::uniformSampler();
                        
                        stitch::Vec3 T(line.P_, line.Q_, 1.0f-t, t);
                        const float R=T.length();
                        const float dotTimesSTimesR=T*lineNormalTimesS;
                        const float dProb=dotTimesSTimesR * ( CDF(R) / (((float)(2.0*M_PI))*(R*R)) ) * dt;
                        prob+=dProb;
                    }
                }
            }
            
            return fabsf(prob);
        }
        //===========================================================================//
        //===========================================================================//
        
        
        //===========================================================================//
        //===========================================================================//
        static inline float calcFdotN(const Vec3 &P, const float lengthP, const Vec3 &borderLineNormal)
        {
            const float dotTimesRAtP=P*borderLineNormal;
            
            const float RAtP=lengthP+0.000001f;//+0.000001 to avoid numerical errors making the below calculation NAN/INF due to 1.0f/(r0*r0).
            return dotTimesRAtP * ( CDF(RAtP) / (2.0f*float(M_PI)*(RAtP*RAtP)) );
        }
        
        static float gaussLineRecur(const Vec3 &P, const float fdotnP,
                                    const Vec3 &Q, const float fdotnQ,
                                    const float lengthPQ,
                                    const Vec3 &borderLineNormal)
        {
            float prob;
            
            if (lengthPQ<0.05f)//early termination threshold... Should be expressed as an error bound and tunable!
            {
                prob=(fdotnP+fdotnQ)*0.5f*lengthPQ;
            } else
            {
                const stitch::Vec3 C(P, Q, 0.5f, 0.5f);
                const float fdotnC=calcFdotN(C, C.length(), borderLineNormal);
                
                const float fdotnPQ=(fdotnP+fdotnQ)*0.5f;
                
                if (fabsf(fdotnC-fdotnPQ)<0.01f)//accuracy threshold... Should be expressed as an error bound and tunable!
                {
                    prob=(fdotnP+fdotnC+fdotnC+fdotnQ)*0.25f*lengthPQ;
                }
                else
                {
                    prob=gaussLineRecur(P, fdotnP, C, fdotnC, lengthPQ*0.5f, borderLineNormal);
                    prob+=gaussLineRecur(C, fdotnC, Q, fdotnQ, lengthPQ*0.5f, borderLineNormal);
                }
            }
            
            return prob;
        }
        
        static float gaussVolumeGaussRecur(const Vec3 &A,
                                           const float lengthA,
                                           const Vec3 &B,
                                           const float lengthB,
                                           const Vec3 &C,
                                           const float lengthC,
                                           const Vec3 &polyNormal)
        {
            float prob=0.0f;
            
            //            const Vec3 polyNormal=stitch::Vec3::crossNormalised(A, B, C);//Assume that triangle ABC has some area.
            //            const float lengthA=A.length();
            //            const float lengthB=B.length();
            //            const float lengthC=C.length();
            
            {//AB
                const Vec3 &P=A;
                const Vec3 &Q=B;
                const float &lengthP=lengthA;
                const float &lengthQ=lengthB;
                
                const stitch::Vec3 PQ(P, Q);
                const float lengthSqPQ=PQ.lengthSq();
                const float lengthPQ=sqrtf(lengthSqPQ);
                Vec3 borderNormalPQ(PQ^polyNormal);//PQ orthogonal to polyNormal -> sin(theta)=1.0.
                borderNormalPQ*=1.0f/lengthPQ;
                
                const float fdotnP=calcFdotN(P, lengthP, borderNormalPQ);
                const float fdotnQ=calcFdotN(Q, lengthQ, borderNormalPQ);
                
                const float tO=stitch::Vec3::dotorigin(P, Q) / lengthSqPQ;
                
                if ((tO>0.0f)&&(tO<1.0f))
                {
                    stitch::Vec3 O(P, Q, 1.0f-tO, tO);
                    const float fdotnO=calcFdotN(O, O.length(), borderNormalPQ);
                    
                    prob+=gaussLineRecur(P, fdotnP, O, fdotnO, lengthPQ*tO, borderNormalPQ);
                    prob+=gaussLineRecur(O, fdotnO, Q, fdotnQ, lengthPQ*(1.0f-tO), borderNormalPQ);
                } else
                {
                    prob+=gaussLineRecur(P, fdotnP, Q, fdotnQ, lengthPQ, borderNormalPQ);
                }
            }
            
            {//BC
                const Vec3 &P=B;
                const Vec3 &Q=C;
                const float &lengthP=lengthB;
                const float &lengthQ=lengthC;
                
                const stitch::Vec3 PQ(P, Q);
                const float lengthSqPQ=PQ.lengthSq();
                const float lengthPQ=sqrtf(lengthSqPQ);
                Vec3 borderNormalPQ(PQ^polyNormal);//PQ orthogonal to polyNormal -> sin(theta)=1.0.
                borderNormalPQ*=1.0f/lengthPQ;
                
                const float fdotnP=calcFdotN(P, lengthP, borderNormalPQ);
                const float fdotnQ=calcFdotN(Q, lengthQ, borderNormalPQ);
                
                const float tO=stitch::Vec3::dotorigin(P, Q) / lengthSqPQ;
                
                if ((tO>0.0f)&&(tO<1.0f))
                {
                    stitch::Vec3 O(P, Q, 1.0f-tO, tO);
                    const float fdotnO=calcFdotN(O, O.length(), borderNormalPQ);
                    
                    prob+=gaussLineRecur(P, fdotnP, O, fdotnO, lengthPQ*tO, borderNormalPQ);
                    prob+=gaussLineRecur(O, fdotnO, Q, fdotnQ, lengthPQ*(1.0f-tO), borderNormalPQ);
                } else
                {
                    prob+=gaussLineRecur(P, fdotnP, Q, fdotnQ, lengthPQ, borderNormalPQ);
                }
            }
            
            {//CA
                const Vec3 &P=C;
                const Vec3 &Q=A;
                const float &lengthP=lengthC;
                const float &lengthQ=lengthA;
                
                const stitch::Vec3 PQ(P, Q);
                const float lengthSqPQ=PQ.lengthSq();
                const float lengthPQ=sqrtf(lengthSqPQ);
                Vec3 borderNormalPQ(PQ^polyNormal);//PQ orthogonal to polyNormal -> sin(theta)=1.0.
                borderNormalPQ*=1.0f/lengthPQ;
                
                const float fdotnP=calcFdotN(P, lengthP, borderNormalPQ);
                const float fdotnQ=calcFdotN(Q, lengthQ, borderNormalPQ);
                
                const float tO=stitch::Vec3::dotorigin(P, Q) / lengthSqPQ;
                
                if ((tO>0.0f)&&(tO<1.0f))
                {
                    stitch::Vec3 O(P, Q, 1.0f-tO, tO);
                    const float fdotnO=calcFdotN(O, O.length(), borderNormalPQ);
                    
                    prob+=gaussLineRecur(P, fdotnP, O, fdotnO, lengthPQ*tO, borderNormalPQ);
                    prob+=gaussLineRecur(O, fdotnO, Q, fdotnQ, lengthPQ*(1.0f-tO), borderNormalPQ);
                } else
                {
                    prob+=gaussLineRecur(P, fdotnP, Q, fdotnQ, lengthPQ, borderNormalPQ);
                }
            }
            
            return fabsf(prob);
        }
        //===========================================================================//
        //===========================================================================//
        
        
        //===========================================================================//
        //===========================================================================//
        /*!
         \brief Calculate the volume over ABC using a boundary line integral and Gauss's divergence theorem.
         
         Calculate the volume over ABC using boundary line integral and Gauss's divergence theorem. In one
         dimension Gauss's theorem is equivalent to the fundamental theorem of calculus; in two dimensions
         it is equivalent to Green's theorem which is a special case of Stokes' theorem.
         
         Each of the boundary lines is split in two if it contains an F minima.
         Trapezoidal integration is then used on the up to six resulting boundary lines.
         Often only two trapezoidal steps are required per side, AB, BC or CA.
         */
        static float gaussVolumeGaussSubd(const Vec3 &A, const Vec3 &B, const Vec3 &C)
        {
            float prob=0.0f;
            
            const float a=0.5f;//The minimum step size in Gaussian space.
            const float b=1.0f;//Step increase per unit range.
            
            std::vector<GaussLine> lineVector;
            lineVector.reserve(6);
            
            lineVector.emplace_back(A, B, 1.0f);
            lineVector.emplace_back(B, C, 1.0f);
            lineVector.emplace_back(C, A, 1.0f);
            
            
            //Invert and split the lines so that the integration can always start on a peak and proceed down hill.
            for (size_t i=0; i<3; ++i)
            {
                const float tO=stitch::Vec3::dotorigin(lineVector[i].P_, lineVector[i].Q_) / stitch::Vec3::calcDistToPointSq(lineVector[i].P_, lineVector[i].Q_);
                
                if (tO>=1.0f)
                {
                    lineVector[i].invert();
                } else
                    if (tO>=0.0f)
                    {
                        stitch::Vec3 O(lineVector[i].P_, lineVector[i].Q_, 1.0f-tO, tO);
                        lineVector.emplace_back(O, lineVector[i].P_, -1.0f);
                        lineVector[i].P_=O;
                    }
            }
            
            
            const Vec3 polyNormal=stitch::Vec3::crossNormalised(A, B, C);//Assume that triangle ABC has some area.
            
            for (const auto & line : lineVector)
            {
                //S:=||PQ||
                //T point on line PQ.
                //R:=||T||
                
                const stitch::Vec3 PQ(line.P_, line.Q_);
                const Vec3 borderNormalTimesS(PQ^polyNormal);
                float dotTimesSTimesR=(line.P_ * borderNormalTimesS) * line.scale_;
                
                if (dotTimesSTimesR!=0.0f)
                {
                    stitch::Vec3 T=line.P_;
                    const float r0=T.length()+0.000001f;//+0.000001 to avoid numerical errors making the below calculation NAN/INF due to 1.0f/(r0*r0).
                    float p0=dotTimesSTimesR * ( CDF(r0) / (((float)(2.0*M_PI))*(r0*r0)) );
                    float t0=0.0f;
                    
                    const float recipPQLength=1.0f/PQ.length();
                    float dt=(a+r0*b) * recipPQLength;
                    
                    while ((t0+dt) < 1.0f)
                    {
                        T.addScaled(PQ, dt);
                        
                        dotTimesSTimesR=(T * borderNormalTimesS) * line.scale_;
                        const float R=T.length()+0.000001f;//+0.000001 to avoid numerical errors making the below calculation NAN/INF due to 1.0f/(r0*r0).
                        
                        const float p=dotTimesSTimesR * ( CDF(R) / (((float)(2.0*M_PI))*(R*R)) );
                        prob+=(p+p0)*0.5f*dt;
                        
                        p0=p;
                        t0+=dt;
                        
                        dt=(a+R*b) * recipPQLength;
                    }
                    
                    {
                        dotTimesSTimesR=(line.Q_ * borderNormalTimesS) * line.scale_;
                        const float r1=line.Q_.length()+0.000001f;//+0.000001 to avoid numerical errors making the below calculation NAN/INF due to 1.0f/(r0*r0).
                        
                        const float p1=dotTimesSTimesR * ( CDF(r1) / (((float)(2.0*M_PI))*(r1*r1)) );
                        prob+=(p1+p0)*0.5f*(1.0f-t0);
                    }
                    
                }
            }
            
            return fabsf(prob);
        }
        //===========================================================================//
        //===========================================================================//
        
        
        
        
    private:
        stitch::Brush BVBrush_;
        
    public:
        Plane generator_;
        
        //!The standard deviation of the scatter distribution in radians (rad).
        float glossySDRad_;
        
        Colour_t fluxSPD_;
        
        std::vector<Vec3> vertices_;
        std::vector<Vec3> vertVects_;
        
        bool hasEndCap_;
        Plane endCap_;
        std::vector<Vec3> endCapVertices_;
        
        /*! Volume lookup table for the gaussVolumeLookUp...() methods.
         @todo delete the memory for this table before app exits!*/
        static float *volImageData_;
        
        //!Accessor method for the Gaussian probability lookup table.
        static float gaussProbLookUpOptimisedAB(const float lengthANorm, const float lengthBNorm, const float theta);
        
    private:
        bool BVUpdated_;
        
    };
    
}


#endif// STITCH_BEAM_SEGMENT_H
