/*
 * $Id: Vec3.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  Vec3.cpp
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

#include "Vec3.h"
#include "Vec4.h"
#include "VecN.h"
#include "Plane.h"
#include "Objects/BrushModel.h"
#include "Materials/DiffuseMaterial.h"
#include "../KDTree.h"

float stitch::Vec3::abyss_=0.0f;
std::map<uintptr_t, stitch::Vec3> stitch::Vec3::uniqueValueMap_;

stitch::Vec3 stitch::Vec3::allZeros()
{
    return Vec3();//Zeroed in constructor;
}

stitch::Vec3 stitch::Vec3::allOnes()
{
    return Vec3(1.0f);
}

stitch::Vec3 stitch::Vec3::rand()
{
    return Vec3(GlobalRand::uniformSampler()*2.0f-1.0f, GlobalRand::uniformSampler()*2.0f-1.0f, GlobalRand::uniformSampler()*2.0f-1.0f);
}

float stitch::Vec3::rand_pdf(const Vec3 &v)
{
    return 1.0f/8.0f;// 1/(2*2*2)
}


stitch::Vec3 stitch::Vec3::randBall()
{
    Vec3 randVec;
    
    do {
        randVec=Vec3(GlobalRand::uniformSampler()*2.0f-1.0f, GlobalRand::uniformSampler()*2.0f-1.0f, GlobalRand::uniformSampler()*2.0f-1.0f);
    } while (randVec.lengthSq()>1.0);
    
    return randVec;
}

float stitch::Vec3::randBall_pdf(const Vec3 &v)
{
    return 3.0f / (4.0f * ((float)M_PI));
}

stitch::Vec3 stitch::Vec3::randDisc()
{
    Vec3 randVec;
    
    do {
        randVec=Vec3(GlobalRand::uniformSampler()*2.0f-1.0f, GlobalRand::uniformSampler()*2.0f-1.0f, 0.0f);
    } while (randVec.lengthSq()>1.0);
    
    return randVec;
}

float stitch::Vec3::randDisc_pdf(const Vec3 &v)
{
    return 1.0f / ((float)M_PI);
}

stitch::Vec3 stitch::Vec3::randNorm()
{
    const float r1=GlobalRand::uniformSampler();
    const float r2=GlobalRand::uniformSampler();
    
    const float a=2.0f*((float)M_PI)*r1;
    const float b=2.0f*sqrtf(r2*(1.0f-r2));
    
    return Vec3(cosf(a)*b, sinf(a)*b, (1.0f-2.0f*r2));
}

float stitch::Vec3::randNorm_pdf(const Vec3 &v)
{
    return 1.0f / (4.0f * ((float)M_PI));
}

stitch::Vec3 stitch::Vec3::randGaussianLobe(const float sd)//sd is standard deviation in radians!
{
    //Box-Muller transform - adapted for Vec3 from 2007_Thomas_Luk_Leong_Villasenor.pdf
    const float u1=GlobalRand::uniformSampler();
    const float u2=GlobalRand::uniformSampler();
    
    const float R=sqrtf(-2.0f * logf(u1)) * sd; //log and logf return the natural logarithm i.e. ln
    const float theta=(2.0f * ((float)M_PI)) * u2;
    
    //Box-Muller produces TWO Gaussian random variables gx and gy from polar coordinates R and theta.
    //double gx=R * cosf(theta);
    //double gy=R * sinf(theta);
    
    //We'll use the polar coordinates directly, though to generate the Vec3!
    return Vec3(sinf(R)*cosf(theta), sinf(R)*sinf(theta), cosf(R));
    //Checked that x^2+y^2+z^2 = sin^2(R)*cos^2(theta) + sin^2(R)*sin^2(theta) + cos^2(R)
    //                         = sin^2(R)[cos^2(theta) + sin^2(theta)] + cos^2(R)
    //                         = sin^2(R)[1.0] + cos^2(R)
    //                         = 1.0
}

float stitch::Vec3::randGaussianLobe_pdf(const float sd, const Vec3 &v)
{
    const float a=acosf(stitch::MathUtil::clamp(v.z(), -1.0f, 1.0f));
    
    return (1.0f / ((2.0f * ((float)M_PI))* (sd*sd))) * expf( -(a*a) / (2.0f*(sd*sd)) );
}

stitch::Vec3 stitch::Vec3::randCosineLobe(const float exp)
{
    const float r1=GlobalRand::uniformSampler();
    const float r2=GlobalRand::uniformSampler();
    
    const float a=2.0 * ((float)M_PI) * r1;
    const float b=sqrtf(1.0f-powf(r2,2.0f/(exp+1.0f)));
    
    return Vec3(cosf(a)*b, sinf(a)*b, powf(r2,1.0f/(exp+1.0f)));
}


float stitch::Vec3::randCosineLobe_pdf(const float exp, const Vec3 &v)
{
    const float cosb=v.z();
    
    if (cosb>=0.0f)
    {
        return (exp+1.0f)/(2.0f*((float)M_PI)) * powf(cosb, exp);
    } else
    {
        return 0.0f;
    }
}

float stitch::Vec3::randCosineLobe_pdf_proj(const float exp, const Vec3 &v)
{
    const float cosb=v.z();
    
    if (cosb>=0.0f)
    {
        float randCosineLobePDF=randCosineLobe_pdf(exp, v);
        return randCosineLobePDF + randCosineLobePDF/(exp+1.0f);//((exp+2.0f)/(2.0f*((float)M_PI))) * powf(cosb, exp);//Note the +2.
    } else
    {
        return 0.0f;
    }
}


void stitch::Vec3::equidistantVectors_FibonacciSpiralSphere(const size_t minimumNumVectors, std::vector<stitch::Vec3> &vectors, std::vector<size_t> &binIndices)
{
    //std::cout << "stitch::Vec3::equidistantVectors_FibonacciSpiralSphere(...)...";
    //std::cout.flush();
    
    vectors.clear();
    binIndices.clear();
    
    //Generate the vectors based on the golden section spiral a.k.a. the Fibonacci Spiral sphere.
    const float phi=(sqrtf(5.0f)+1.0f)/2.0f-1.0f;
    const float ga=phi * ((float)M_PI);
    
    for (size_t vectorNum=0; vectorNum<minimumNumVectors; ++vectorNum)
    {
        const float lon=ga * vectorNum;
        const float lat=asinf(stitch::MathUtil::clamp(-1.0f + (2.0f * vectorNum)/minimumNumVectors, -1.0f, 1.0f));
        
#ifdef USE_CXX11
        vectors.emplace_back(cosf(lon)*cosf(lat), sinf(lon)*cosf(lat), sinf(lat));
#else
        vectors.push_back(Vec3(cosf(lon)*cosf(lat), sinf(lon)*cosf(lat), sinf(lat)));
#endif
    }
    
    //Build a brush that contains the sphere and use the updateVerticesAndLines method to create a new set of vertices and indices.
    //  It is assumed that a tesselation of the Fiboncci Spiral sphere would be strickly convex.
    //  Ducplicate brush vertices are not added.
    {
        stitch::Brush brush(new stitch::DiffuseMaterial(stitch::Colour_t(0.9f, 0.9f, 0.9f)));
        
        const size_t numVertices=vectors.size();
        
        //=== Create a permutation of vertices v0, v1, v2 ===
        for (size_t vertexNum0=0; vertexNum0<numVertices; ++vertexNum0)
        {
            for (size_t vertexNum1=vertexNum0+1; vertexNum1<numVertices; ++vertexNum1)
            {
                for (size_t vertexNum2=vertexNum1+1; vertexNum2<numVertices; ++vertexNum2)
                {
                    stitch::Vec3 v0=vectors[vertexNum0];
                    stitch::Vec3 v1=vectors[vertexNum1];
                    stitch::Vec3 v2=vectors[vertexNum2];
                    
                    stitch::Plane plane(v0, v1, v2);
                    bool verticesOnPositiveSide=false;
                    bool verticesOnNegativeSide=false;
                    
                    //Test if this permutation of vertices defines a brush face.
                    for (size_t vertexNum=0; vertexNum<numVertices; ++vertexNum)
                    {
                        if ((vertexNum!=vertexNum0) &&
                            (vertexNum!=vertexNum1) &&
                            (vertexNum!=vertexNum2))
                        {
                            stitch::Vec3 v=vectors[vertexNum];
                            
                            if (v * plane.normal_<plane.d_)
                            {
                                verticesOnNegativeSide=true;
                            } else
                            {
                                verticesOnPositiveSide=true;
                            }
                            
                            if (verticesOnPositiveSide && verticesOnNegativeSide)
                            {
                                break; //from for loop. Permutation of vertices v0, v1, v2 is not on a brush plane.
                            }
                        }
                    }
                    
                    if (verticesOnPositiveSide && (!verticesOnNegativeSide))
                    {//All vertices on postive side i.e. this is a brush face.
                        
                        //Flip brush face normal to have all other vertices on negative/in side of face.
                        plane.normal_=plane.normal_ * -1.0f;
                        plane.d_=plane.d_ * -1.0f;
                        
                        brush.addFace(stitch::BrushFace(plane, false));
                    }
                    
                    if ((!verticesOnPositiveSide) && verticesOnNegativeSide)
                    {//All vertices on negative side i.e. this is a brush face.
                        
                        brush.addFace(stitch::BrushFace(plane, false));
                    }
                }
            }
            //if ((vertexNum0%10)==0)
            //{
            //std::cout << 100.0f * (vertexNum0 / ((float)numVertices)) << "%...";
            //std::cout.flush();
            //}
        }
        //=== ===
        
        //=== Calculate the face vertices from the brush face planes ===
        brush.updateLinesVerticesAndBoundingVolume(true);
        //=== ===
        
        
        //=== Create fresh vectors and indices from the brush ===
        //  The vectors should be the same as those generated by the Fibonacci spiral,
        //  but possibly in a different order.
        vectors.clear();
        const size_t numFaces=brush.getNumFaces();
        
        for (size_t faceNum=0; faceNum<numFaces; ++faceNum)
        {
            const stitch::BrushFace &face=brush.getFaceByConstRef(faceNum);
            
            size_t numFaceVertices=face.vertexCoordVector_.size();
            
            if (numFaceVertices>=4)
            {
                //The first face vertex is the centre vertex so start at the second vertex.
                stitch::Vec3 v0=face.vertexCoordVector_[1];
                
                //If there are more than 3 perimeter vertices then v0 will be used as the common vertex to
                //  generate (numFaceVertices-2) triangle bins.
                for (size_t faceVertexNum=3; faceVertexNum<numFaceVertices; ++faceVertexNum)
                {
                    stitch::Vec3 v1=face.vertexCoordVector_[faceVertexNum-1l];
                    stitch::Vec3 v2=face.vertexCoordVector_[faceVertexNum];
                    
                    ssize_t i0=-1;
                    ssize_t i1=-1;
                    ssize_t i2=-1;
                    
                    //Check if any of the face vertices are already in the vectors list.
                    size_t numVectors=vectors.size();
                    for (size_t vectorNum=0; vectorNum<numVectors; ++vectorNum)
                    {
                        stitch::Vec3 v=vectors[vectorNum];
                        
                        if ((i0<0l)&&(Vec3::calcDistToPointSq(v, v0)<0.00001f)) i0=vectorNum;
                        if ((i1<0l)&&(Vec3::calcDistToPointSq(v, v1)<0.00001f)) i1=vectorNum;
                        if ((i2<0l)&&(Vec3::calcDistToPointSq(v, v2)<0.00001f)) i2=vectorNum;
                    }
                    
                    if (i0<0l)
                    {//This is a new vector.
                        vectors.push_back(v0);
                        i0=vectors.size()-1;
                    }
                    
                    if (i1<0l)
                    {//This is a new vector.
                        vectors.push_back(v1);
                        i1=vectors.size()-1;
                    }
                    
                    if (i2<0l)
                    {//This is a new vector.
                        vectors.push_back(v2);
                        i2=vectors.size()-1;
                    }
                    
                    binIndices.push_back(i0);
                    binIndices.push_back(i1);
                    binIndices.push_back(i2);
                }
            }
        }
        //=== ===//
        
    }
    
    //=== ===//
    //std::cout << "vectors.size()="<<vectors.size()<<" done.\n";
    //std::cout.flush();
}


void stitch::Vec3::equidistantVectors_IcosahedronBased_GetDetails_minNumVertices(const size_t minNumVertices,
                                                                            size_t &actualNumVertices,
                                                                            size_t &actualNumEdges,
                                                                            size_t &actualNumBins)
{
    //Start with icosahedron.
    //Note - For refinement of icosahedron:
    //  Faces_0 = 20, Edges_0 = 30, Vertices_0 = 12
    //  Faces_n = Faces_(n-1) * 4
    //  Edges_n = Edges_(n-1) * 2 + Faces_(n-1) * 3
    //  Vertices_n = Vertices_(n-1) + Edges_(n-1)
    
    actualNumVertices=12;
    actualNumEdges=30;
    actualNumBins=20;
    
    while (actualNumVertices<minNumVertices)
    {
        actualNumVertices=actualNumVertices + actualNumEdges;
        actualNumEdges=actualNumEdges * 2 + actualNumBins * 3;
        actualNumBins=actualNumBins*4;
    }
}

void stitch::Vec3::equidistantVectors_IcosahedronBased_GetDetails_minNumBins(const size_t minNumBins,
                                                                                            size_t &actualNumVertices,
                                                                                            size_t &actualNumEdges,
                                                                                            size_t &actualNumBins)
{
    //Start with icosahedron.
    //Note - For refinement of icosahedron:
    //  Faces_0 = 20, Edges_0 = 30, Vertices_0 = 12
    //  Faces_n = Faces_(n-1) * 4
    //  Edges_n = Edges_(n-1) * 2 + Faces_(n-1) * 3
    //  Vertices_n = Vertices_(n-1) + Edges_(n-1)
    
    actualNumVertices=12;
    actualNumEdges=30;
    actualNumBins=20;
    
    while (actualNumBins<minNumBins)
    {
        actualNumVertices=actualNumVertices + actualNumEdges;
        actualNumEdges=actualNumEdges * 2 + actualNumBins * 3;
        actualNumBins=actualNumBins*4;
    }
}

void stitch::Vec3::equidistantVectors_IcosahedronBased(const size_t minimumNumVectors, std::vector<stitch::Vec3> &vectors, std::vector<size_t> &binIndices)
{
    vectors.clear();
    binIndices.clear();
    stitch::KDTree kdTree;//Keep a tame kd-tree on the side for efficient searching of duplicate vertices to be merged while sub-dividing.
    
    //======================================================
    //Start with icosahedron.
    //Note - For refinement of icosahedron:
    //  Faces_0 = 20, Edges_0 = 30, Vertices_0 = 12
    //  Faces_n = Faces_(n-1) * 4
    //  Edges_n = Edges_(n-1) * 2 + Faces_(t-1) * 3
    //  Vertices_n = Vertices_(n-1) + Edges_(n-1)
    
    const float phi=atanf(0.5f);
    
#ifdef USE_CXX11
    vectors.emplace_back(0.0f, 0.0f, 1.0f);
#else
    vectors.push_back(Vec3(0.0f, 0.0f, 1.0f));
#endif
    
    for (size_t i=0; i<5; ++i)
    {
#ifdef USE_CXX11
        vectors.emplace_back(cosf((i*72.0f/180.f) * ((float)M_PI))*cosf(phi),
                             sinf((i*72.0f/180.f) * ((float)M_PI))*cosf(phi),
                             (         1.0f          )*sinf(phi));
#else
        vectors.push_back(Vec3(cosf((i*72.0f/180.f) * ((float)M_PI))*cosf(phi),
                               sinf((i*72.0f/180.f) * ((float)M_PI))*cosf(phi),
                               (         1.0f          )*sinf(phi)));
#endif
        binIndices.push_back(0);
        binIndices.push_back(((i+0)%5) + 1);
        binIndices.push_back(((i+1)%5) + 1);
    }
    
    for (size_t i=0; i<5; ++i)
    {
#ifdef USE_CXX11
        vectors.emplace_back(cosf(((i+0.5f)*72.0f/180.f) * ((float)M_PI))*cosf(-phi),
                             sinf(((i+0.5f)*72.0f/180.f) * ((float)M_PI))*cosf(-phi),
                             (         1.0f          )*sinf(-phi));
#else
        vectors.push_back(Vec3(cosf(((i+0.5f)*72.0f/180.f) * ((float)M_PI))*cosf(-phi),
                               sinf(((i+0.5f)*72.0f/180.f) * ((float)M_PI))*cosf(-phi),
                               (         1.0f          )*sinf(-phi)));
#endif
        binIndices.push_back(((i+0)%5) + 1);
        binIndices.push_back(((i+0)%5) + 6);
        binIndices.push_back(((i+1)%5) + 1);
        
        binIndices.push_back(((i+0)%5) + 6);
        binIndices.push_back(((i+1)%5) + 6);
        binIndices.push_back(((i+1)%5) + 1);
    }
    
#ifdef USE_CXX11
    vectors.emplace_back(0.0f, 0.0f, -1.0f);
#else
    vectors.push_back(Vec3(0.0f, 0.0f, -1.0f));
#endif
    
    for (size_t i=0; i<5; ++i)
    {
        binIndices.push_back(((i+1)%5) + 6);
        binIndices.push_back(((i+0)%5) + 6);
        binIndices.push_back(11);
    }
    //======================================================
    
    
    //Subdivide the icosahedron until number of vertices>=minimumNumVertices.
    while (vectors.size()<minimumNumVectors)
    {
        size_t numBinIndices=binIndices.size();
        
        for (size_t indexNum=0; indexNum<numBinIndices; indexNum+=3)
        {
            size_t i0=binIndices[indexNum+0];
            size_t i1=binIndices[indexNum+1];
            size_t i2=binIndices[indexNum+2];
            
            const stitch::Vec3 v0=vectors[i0];
            const stitch::Vec3 v1=vectors[i1];
            const stitch::Vec3 v2=vectors[i2];
            
            stitch::Vec3 v01=(v0+v1);
            v01.normalise();
            stitch::Vec3 v12=(v1+v2);
            v12.normalise();
            stitch::Vec3 v20=(v2+v0);
            v20.normalise();
            
            int32_t i01=-1;
            int32_t i12=-1;
            int32_t i20=-1;
            
            //=== Check if any of the new vertices already exist ===
            {//Need to have distance epsilon?
                const float similarity_epsilon=0.00001f;
                
                float radius01=1.0f;
                stitch::BoundingVolume *bv01=kdTree.getNearest(v01, radius01);
                
                float radius12=1.0f;
                stitch::BoundingVolume *bv12=kdTree.getNearest(v12, radius12);
                
                float radius20=1.0f;
                stitch::BoundingVolume *bv20=kdTree.getNearest(v20, radius20);
                
                if (bv01)
                {
                    const uint32_t vertexNum01=bv01->userIndex_;
                    if ( Vec3::calcDistToPointSq(vectors[vertexNum01], v01) < similarity_epsilon )
                    {
                        i01=vertexNum01;
                    }
                }
                
                if (bv12)
                {
                    const uint32_t vertexNum12=bv12->userIndex_;
                    if ( Vec3::calcDistToPointSq(vectors[vertexNum12], v12) < similarity_epsilon )
                    {
                        i12=vertexNum12;
                    }
                }
                
                if (bv20)
                {
                    const uint32_t vertexNum20=bv20->userIndex_;
                    if ( Vec3::calcDistToPointSq(vectors[vertexNum20], v20) < similarity_epsilon )
                    {
                        i20=vertexNum20;
                    }
                }
            }
            //=== ===//
            
            if (i01<0)
            {//This is a new vertex, so add it.
                vectors.push_back(v01);
                i01=vectors.size()-1;
                kdTree.addItem(new stitch::BoundingVolume(vectors[vectors.size()-1], 0.0f, vectors.size()-1));
            }
            
            if (i12<0)
            {//This is a new vertex, so add it.
                vectors.push_back(v12);
                i12=vectors.size()-1;
                kdTree.addItem(new stitch::BoundingVolume(vectors[vectors.size()-1], 0.0f, vectors.size()-1));
            }
            
            if (i20<0)
            {//This is a new vertex, so add it.
                vectors.push_back(v20);
                i20=vectors.size()-1;
                kdTree.addItem(new stitch::BoundingVolume(vectors[vectors.size()-1], 0.0f, vectors.size()-1));
            }
            
            std::vector<stitch::Vec3> splitAxisVec;
            splitAxisVec.push_back(Vec3(1.0f, 0.0f, 0.0f));
            splitAxisVec.push_back(Vec3(0.0f, 1.0f, 0.0f));
            splitAxisVec.push_back(Vec3(0.0f, 0.0f, 1.0f));
            
            kdTree.build(KDTREE_DEFAULT_CHUNK_SIZE, 0, 1000, splitAxisVec);
            
            {
                binIndices[indexNum+0]=i0;
                binIndices[indexNum+1]=i01;
                binIndices[indexNum+2]=i20;
            }
            
            {
                binIndices.push_back(i01);
                binIndices.push_back(i12);
                binIndices.push_back(i20);
            }
            
            {
                binIndices.push_back(i20);
                binIndices.push_back(i12);
                binIndices.push_back(i2);
            }
            
            {
                binIndices.push_back(i01);
                binIndices.push_back(i1);
                binIndices.push_back(i12);
            }
        }
    }
}


//=======================================================================//
void stitch::Vec3::relaxEquidistantVectors(std::vector<stitch::Vec3> &vectors, size_t numIterations)
{
    const size_t numVectors = vectors.size();
    
    for (size_t iterationsDone=0; iterationsDone<numIterations; ++iterationsDone)
    {
        float minResult=0.0f;
        float maxResult=0.0f;
        float sumResult=0.0f;
        
        for (size_t acteeVectorNum=0; acteeVectorNum<numVectors; ++acteeVectorNum)
        {
            stitch::Vec3 relaxDelta;//Initialised to zero in the constructor.
            float minActDistance=10.0;//Some large initial distance.
            const float hemicap=0.3f;//1.0=entire hemisphere; 0.1=10% top cap.
            const float hemicapRecip=1.0f/hemicap;
            
            for (size_t actingVectorNum=0; actingVectorNum<numVectors; ++actingVectorNum)
            {
                const float actWeight=(vectors[acteeVectorNum]*vectors[actingVectorNum]);
                
                const float effActWeight=actWeight*hemicapRecip - (hemicapRecip-1.0f);//Only use a cap of the hemisphere!
                
                if (effActWeight>0.0f)
                {
                    if (actingVectorNum!=acteeVectorNum)
                    {
                        stitch::Vec3 actNormal=(vectors[acteeVectorNum] - vectors[actingVectorNum]);
                        actNormal.normalise();
                        
                        relaxDelta+=actNormal * (effActWeight);
                        
                        const float actDistance=1.0f-actWeight;
                        if (actDistance < minActDistance) minActDistance=actDistance;
                    }
                }
            }
            
            vectors[acteeVectorNum] += relaxDelta*minActDistance+Vec3::randNorm()*0.005*minActDistance;
            vectors[acteeVectorNum].normalise();
            
            if (acteeVectorNum>0)
            {
                sumResult+=minActDistance;
                if (minActDistance<minResult) minResult=minActDistance;
                if (minActDistance>maxResult) maxResult=minActDistance;
            } else
            {
                sumResult=minResult=maxResult=minActDistance;
            }
        }
        
        //std::cout << "relaxActingDistance = ["  << minResult << "|" << (sumResult/numVectors) << "|" << maxResult << "]\n";
        //std::cout.flush();
    }
}

//=======================================================================//
void stitch::Vec3::relaxEquidistantVectorsII(std::vector<stitch::Vec3> &vectors, uint32_t numIterations)
{
    const uint32_t numVectors = vectors.size();
    
    for (uint32_t iterationsDone=0; iterationsDone<numIterations; ++iterationsDone)
    {
        float minResult=0.0f;
        float maxResult=0.0f;
        float sumResult=0.0f;
        
        KDTree kdTree;
        
        for (uint32_t acteeVectorNum=0; acteeVectorNum<numVectors; ++acteeVectorNum)
        {
            kdTree.addItem(new BoundingVolume(vectors[acteeVectorNum], 0.0f, acteeVectorNum));
        }
        
        std::vector<stitch::Vec3> splitAxisVec;
        splitAxisVec.push_back(Vec3(1.0f, 0.0f, 0.0f));
        splitAxisVec.push_back(Vec3(0.0f, 1.0f, 0.0f));
        splitAxisVec.push_back(Vec3(0.0f, 0.0f, 1.0f));
        
        kdTree.build(KDTREE_DEFAULT_CHUNK_SIZE, 0, 1000, splitAxisVec);
        
        
        for (uint32_t acteeVectorNum=0; acteeVectorNum<numVectors; ++acteeVectorNum)
        {
            stitch::Vec3 relaxDelta;//Initialised to zero in the constructor.
            float minActDistance=10.0;//Some large initial distance.
            
            KNearestItems kNearestItems(vectors[acteeVectorNum], 1.0f, numVectors/100);
            kdTree.getNearestK(&kNearestItems);
            
            const uint32_t numNearestItems=kNearestItems.numItems_;
            for (uint32_t i=0; i<numNearestItems; ++i)
            {
                uint32_t actingVectorNum=kNearestItems.heapArray_[i].second->userIndex_;
                
                if (actingVectorNum!=acteeVectorNum)
                {
                    const float actDistance=(kNearestItems.heapArray_[i].second->centre_-vectors[acteeVectorNum]).lengthSq() / kNearestItems.searchRadiusSq_;
                    const float actWeight=1.0f - actDistance;
                    stitch::Vec3 actNormal=(vectors[acteeVectorNum] - vectors[actingVectorNum]);
                    actNormal.normalise();
                    
                    relaxDelta+=actNormal * (actWeight);
                    
                    if (actDistance < minActDistance) minActDistance=actDistance;
                }
            }
            
            vectors[acteeVectorNum] += relaxDelta*sqrtf(minActDistance*kNearestItems.searchRadiusSq_)*0.1;//+Vec3::randNorm()*0.005*minActDistance;
            vectors[acteeVectorNum].normalise();
            
            if (acteeVectorNum>0)
            {
                sumResult+=minActDistance;
                if (minActDistance<minResult) minResult=minActDistance;
                if (minActDistance>maxResult) maxResult=minActDistance;
            } else
            {
                sumResult=minResult=maxResult=minActDistance;
            }
        }
        
        //std::cout << "relaxActingDistance = ["  << minResult << "|" << (sumResult/numVectors) << "|" << maxResult << "]\n";
        //std::cout.flush();
    }
}


//=======================================================================//
stitch::Vec3::Vec3(const stitch::Vec4 &lValue)
{
    v_[0]=lValue.x();
    v_[1]=lValue.y();
    v_[2]=lValue.z();
}

//=======================================================================//
stitch::Vec3 & stitch::Vec3::operator = (const stitch::Vec4 &lValue)
{
    v_[0]=lValue.x();
    v_[1]=lValue.y();
    v_[2]=lValue.z();
    return (*this);
}

//=======================================================================//
stitch::Vec3::Vec3(const stitch::VecN &lValue)
{
    v_[0]=lValue.x();
    v_[1]=lValue.y();
    v_[2]=lValue.z();
}

//=======================================================================//
stitch::Vec3 & stitch::Vec3::operator = (const stitch::VecN &lValue)
{
    v_[0]=lValue.x();
    v_[1]=lValue.y();
    v_[2]=lValue.z();
    return (*this);
}

//=======================================================================//
stitch::Vec3 stitch::Vec3::uniqueValue(uintptr_t key)
{
    std::map<uintptr_t, Vec3>::const_iterator iter=uniqueValueMap_.find(key);
    
    if (iter!=uniqueValueMap_.end())
    {
        return iter->second;
    } else
    {
        Vec3 newRandVec=randNorm();
        uniqueValueMap_[key]=newRandVec;
        return newRandVec;
    }
}

