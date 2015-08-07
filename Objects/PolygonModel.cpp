/*
 * $Id: PolygonModel.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  PolygonModel.cpp
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2011/02/22.
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

#include "PolygonModel.h"
#include "Math/MathUtil.h"
//#include "Beam.h"
#include "Math/Mat4.h"

#include "IOUtils/ply.h"
#include "IOUtils/obj.h"

#include <iostream>
#include <fstream>

#include "OSGUtils/StitchOSG.h"

#ifdef USE_OSG
osg::ref_ptr<osg::Node> stitch::Polygon::constructOSGNode(const bool createOSGLineGeometry, const bool createOSGNormalGeometry, const bool wireframe, const uintptr_t key) const
{
    Vec3 colour(stitch::Vec3::uniqueValue(key==0 ? ((uintptr_t)this) : key));
    colour.positivise();
    
    osg::ref_ptr<osg::Geode> osgGeode=new osg::Geode();
    
    osg::ref_ptr<osg::Vec3Array> osgTriangleVertices=new osg::Vec3Array();
    osg::ref_ptr<osg::Vec3Array> osgTriangleNormals=new osg::Vec3Array();
    
    
    //=== Construct triangle geometry ===
    
    {
        osgTriangleVertices->push_back(osg::Vec3f(v0_.x(), v0_.y(), v0_.z()));
        osgTriangleVertices->push_back(osg::Vec3f(v1_.x(), v1_.y(), v1_.z()));
        osgTriangleVertices->push_back(osg::Vec3f(v2_.x(), v2_.y(), v2_.z()));
        
        osgTriangleNormals->push_back(osg::Vec3f(n0_.x(), n0_.y(), n0_.z()));
        osgTriangleNormals->push_back(osg::Vec3f(n1_.x(), n1_.y(), n1_.z()));
        osgTriangleNormals->push_back(osg::Vec3f(n2_.x(), n2_.y(), n2_.z()));
    }
    
    osg::ref_ptr<osg::Geometry> osgTriangleGeometry=new osg::Geometry();
    osgTriangleGeometry->setVertexArray(osgTriangleVertices.get());
    osgTriangleGeometry->setNormalArray(osgTriangleNormals.get());
    osgTriangleGeometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    //osgTriangleGeometry->setUseVertexBufferObjects(true);
    osgTriangleGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES,0,osgTriangleVertices->size()));
    
    osg::ref_ptr<osg::StateSet> osgTriangleStateset=osgTriangleGeometry->getOrCreateStateSet();
    osgTriangleStateset->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
    //osgTriangleStateset->setMode(GL_RESCALE_NORMAL, osg::StateAttribute::ON);//Re-normalise scaled normal vectors.
    osgTriangleStateset->setMode(GL_LIGHTING, osg::StateAttribute::ON);
    //osgTriangleStateset->setMode(GL_LIGHT_MODEL_TWO_SIDE, osg::StateAttribute::ON);
    
    {//Solid or wireframe?
        osg::PolygonMode* pm = dynamic_cast<osg::PolygonMode*>(osgTriangleStateset->getAttribute(osg::StateAttribute::POLYGONMODE));
        if (!pm)
        {
            pm = new osg::PolygonMode;
        }
        
        if (wireframe)
        {
            pm->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
        } else
        {
            pm->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL);
        }
        
        osgTriangleStateset->setAttribute(pm);
    }
    
    osg::Depth* depth = new osg::Depth();
    osgTriangleStateset->setAttributeAndModes(depth, osg::StateAttribute::ON);
    
    osg::Material *material = new osg::Material();
    material->setColorMode(osg::Material::DIFFUSE);
    
    material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(colour.x(), colour.y(), colour.z(), 1.0));
    osgTriangleStateset->setAttributeAndModes(material, osg::StateAttribute::ON);
    
    osgGeode->addDrawable(osgTriangleGeometry.get());
    
    //=======
    
    return osgGeode;
}
#endif// USE_OSG


//=======================================================================//
void stitch::Polygon::calcIntersection(const Ray &ray, Intersection &intersect) const
{
    //if (BVIntersected(orig, normDir)) This is currently executed by the calling code!
    {//Moller and Trumbore - 1997, "Fast, minimum storage ray-triangle intersection.", Journal of Graphics Tools 2(1), 21-28.
        //Coded from PBRT Book, Second Ed, p140.
        const Vec3 e1(v0_,v1_);//v1_ - v0_
        const Vec3 e2(v0_,v2_);//v2_ - v0_
        const Vec3 s1=stitch::Vec3::cross(ray.direction_, e2);
        
        const float divisor=stitch::Vec3::dot(s1, e1);
        
        if (divisor!=0.0f)
        {
            const float recipDivisor=1.0f/divisor;
            
            const Vec3 d(v0_, ray.origin_);//ray.origin_ - v0_
            
            const float b1 = stitch::Vec3::dotscale(d, s1, recipDivisor);
            
            if ((b1>=0.0f) && (b1<=1.0f))
            {
                const Vec3 s2 = stitch::Vec3::cross(d, e1);
                const float b2 = stitch::Vec3::dotscale(ray.direction_, s2, recipDivisor);
                
                if ((b2>=0.0f) && ((b1+b2)<=1.0f))
                {
                    const float intersectDist=stitch::Vec3::dotscale(e2, s2, recipDivisor);
                    
                    if ((intersectDist>0.0f)&&(intersectDist<intersect.distance_))
                    {
                        intersect.distance_=intersectDist;
                        intersect.normal_.setToSumScaleAndNormalise(n0_, 1.0f-b1-b2, n1_, b1, n2_, b2);
                        intersect.itemID_=this->itemID_ | ((intersect.normal_*ray.direction_>0.0f)?0:1);//back surface gets even ID, front surface gets odd ID.
                        intersect.itemPtr_=this;
                    }
                }
            }
        }
    }
}





//=======================================================================//
void stitch::PolygonModel::calculateVertexNormals()
{
    smoothSurface_=true;
    
    std::vector<stitch::Vec3>::iterator vertexNormalIter=vertNormals_.begin();
    for (; vertexNormalIter!=vertNormals_.end(); ++vertexNormalIter)
    {
        (*vertexNormalIter)=stitch::Vec3(0.0f, 0.0f, 0.0f);
    }
    
    std::vector<size_t>::const_iterator faceIndexIter=indices_.begin();
    for (; faceIndexIter!=indices_.end(); faceIndexIter+=3)
    {
        stitch::Vec3 v0(vertCoords_[*(faceIndexIter+0)]);
        stitch::Vec3 v1(vertCoords_[*(faceIndexIter+1)]);
        stitch::Vec3 v2(vertCoords_[*(faceIndexIter+2)]);
        
        stitch::Vec3 planeNormal;
        
        if ( ((v2-v0).lengthSq()!=0.0f) && ((v1-v0).lengthSq()!=0.0f) && ((v1-v2).lengthSq()!=0.0f) )
        {//Protect against needle or point polygons.
            planeNormal=stitch::Vec3::crossNormalised(v2, v1, v0);
            
            vertNormals_[*(faceIndexIter+0)]+=planeNormal;
            vertNormals_[*(faceIndexIter+1)]+=planeNormal;
            vertNormals_[*(faceIndexIter+2)]+=planeNormal;
        }
    }
    
    vertexNormalIter=vertNormals_.begin();
    for (; vertexNormalIter!=vertNormals_.end(); ++vertexNormalIter)
    {
        vertexNormalIter->normalise();
    }
}


//=======================================================================//
#ifdef USE_OSG
osg::ref_ptr<osg::Node> stitch::PolygonModel::constructOSGNode(const bool createOSGLineGeometry, const bool createOSGNormalGeometry, const bool wireframe, const uintptr_t key) const
{
    Vec3 colour(stitch::Vec3::uniqueValue(key==0 ? ((uintptr_t)this) : key));
    colour.positivise();
    
    return ballTree_->constructOSGNode(createOSGLineGeometry, createOSGNormalGeometry, wireframe, key==0 ? (smoothSurface_ ? ((uintptr_t)this) : ((uintptr_t)0)) : key); //This method generates way too many drawables for OSG!
    
    osg::ref_ptr<osg::Geode> osgGeode=new osg::Geode();
    
    osg::ref_ptr<osg::Vec3Array> osgTriangleVertices=new osg::Vec3Array();
    osg::ref_ptr<osg::Vec3Array> osgTriangleNormals=new osg::Vec3Array();
    
    
    //=== Construct triangle geometry ===
    
    std::vector<size_t>::const_iterator faceIndexIter=indices_.begin();
    for (; faceIndexIter!=indices_.end(); faceIndexIter+=3)
    {
        stitch::VecN v0(vertCoords_[*(faceIndexIter+0)]);
        stitch::VecN v1(vertCoords_[*(faceIndexIter+1)]);
        stitch::VecN v2(vertCoords_[*(faceIndexIter+2)]);
        
        stitch::VecN n0(vertNormals_[*(faceIndexIter+0)]);
        stitch::VecN n1(vertNormals_[*(faceIndexIter+1)]);
        stitch::VecN n2(vertNormals_[*(faceIndexIter+2)]);
        
        osgTriangleVertices->push_back(osg::Vec3f(v0.x(), v0.y(), v0.z()));
        osgTriangleVertices->push_back(osg::Vec3f(v1.x(), v1.y(), v1.z()));
        osgTriangleVertices->push_back(osg::Vec3f(v2.x(), v2.y(), v2.z()));
        
        osgTriangleNormals->push_back(osg::Vec3f(n0.x(), n0.y(), n0.z()));
        osgTriangleNormals->push_back(osg::Vec3f(n1.x(), n1.y(), n1.z()));
        osgTriangleNormals->push_back(osg::Vec3f(n2.x(), n2.y(), n2.z()));
    }
    
    osg::ref_ptr<osg::Geometry> osgTriangleGeometry=new osg::Geometry();
    osgTriangleGeometry->setVertexArray(osgTriangleVertices.get());
    osgTriangleGeometry->setNormalArray(osgTriangleNormals.get());
    osgTriangleGeometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    //osgTriangleGeometry->setUseVertexBufferObjects(true);
    osgTriangleGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES,0,osgTriangleVertices->size()));
    
    osg::ref_ptr<osg::StateSet> osgTriangleStateset=osgTriangleGeometry->getOrCreateStateSet();
    osgTriangleStateset->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
    //osgTriangleStateset->setMode(GL_RESCALE_NORMAL, osg::StateAttribute::ON);//Re-normalise scaled normal vectors.
    osgTriangleStateset->setMode(GL_LIGHTING, osg::StateAttribute::ON);
    //osgTriangleStateset->setMode(GL_LIGHT_MODEL_TWO_SIDE, osg::StateAttribute::ON);
    
    {//Solid or wireframe?
        osg::PolygonMode* pm = dynamic_cast<osg::PolygonMode*>(osgTriangleStateset->getAttribute(osg::StateAttribute::POLYGONMODE));
        if (!pm)
        {
            pm = new osg::PolygonMode;
        }
        
        if (wireframe)
        {
            pm->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
        } else
        {
            pm->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL);
        }
        
        osgTriangleStateset->setAttribute(pm);
    }
    
    osg::Depth* depth = new osg::Depth();
    osgTriangleStateset->setAttributeAndModes(depth, osg::StateAttribute::ON);
    
    osg::Material *material = new osg::Material();
    material->setColorMode(osg::Material::DIFFUSE);
    material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(colour.x(), colour.y(), colour.z(), 1.0));
    osgTriangleStateset->setAttributeAndModes(material, osg::StateAttribute::ON);
    
    osgGeode->addDrawable(osgTriangleGeometry.get());
    
    //=======
    
    return osgGeode;
}
#endif// USE_OSG


//=======================================================================//
typedef struct Vertex {
    float x,y,z;             /* the usual 3-space position of a vertex */
} Vertex;

typedef struct Face {
    unsigned char intensity; /* this user attaches intensity to faces */
    unsigned char nverts;    /* number of vertex indices in list */
    int *verts;              /* vertex index list */
} Face;

PlyProperty vert_props[] = { /* list of property information for a vertex */
    {"x", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,x), 0, 0, 0, 0},
    {"y", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,y), 0, 0, 0, 0},
    {"z", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,z), 0, 0, 0, 0},
};

PlyProperty face_props[] = { /* list of property information for a vertex */
    {"intensity", PLY_UCHAR, PLY_UCHAR, offsetof(Face,intensity), 0, 0, 0, 0},
    {"vertex_indices", PLY_INT, PLY_INT, offsetof(Face,verts),
        1, PLY_UCHAR, PLY_UCHAR, offsetof(Face,nverts)},
};

bool stitch::PolygonModel::loadPLYVertices(const std::string fileName, const stitch::Vec3 &centre, const float scale, const bool invertNormals)
{
    vertNormals_.clear();
    vertCoords_.clear();
    indices_.clear();
    
    std::cout << "Loading PLY file" << fileName << "...\n";
    std::cout.flush();
    
    //=== Read PLY file ===
    
    /* open a PLY file for reading */
    int file_type=0;
    float version=0;
    int nelems=0;
    char **elist=nullptr;
    
    PlyFile *ply = ply_open_for_reading(fileName.c_str(), &nelems, &elist, &file_type, &version);
    std::cout << "   PLY version " << version << "\n";
    std::cout << "   PLY type " << file_type << "\n";
    std::cout.flush();
    
    /* go through each kind of element that we learned is in the file */
    /* and read them */
    
    for (int i = 0; i < nelems; i++)
    {
        /* get the description of the first element */
        int num_elems;
        char *elem_name = elist[i];
        int nprops;
        PlyProperty **plist = ply_get_element_description (ply, elem_name, &num_elems, &nprops);
        
        /* print the name of the element, for debugging */
        std::cout << "   Element " << elem_name << " " << num_elems << "\n";
        std::cout.flush();
        
        /* if we're on vertex elements, read them in */
        if (equal_strings ("vertex", elem_name))
        {
            ply_get_property (ply, elem_name, &vert_props[0]);
            ply_get_property (ply, elem_name, &vert_props[1]);
            ply_get_property (ply, elem_name, &vert_props[2]);
            
            for (int j = 0; j < num_elems; j++)
            {
                Vertex vertex;
                ply_get_element (ply, (void *) &vertex);
                
                vertCoords_.push_back(stitch::Vec3(vertex.x, vertex.y, vertex.z)*scale+centre);
                
#ifdef USE_CXX11
                vertNormals_.emplace_back(0.0, 0.0, 1.0);
#else
                vertNormals_.push_back(stitch::Vec3(0.0, 0.0, 1.0));
#endif
            }
        }
        
        
        /* if we're on face elements, read them in */
        if (equal_strings("face", elem_name)) {
            
            ply_get_property (ply, elem_name, &face_props[0]);
            ply_get_property (ply, elem_name, &face_props[1]);
            
            for (int j = 0; j < num_elems; j++) {
                Face face;
                ply_get_element (ply, (void *) &face);
                
                if (face.nverts>=3)
                {
                    for (int k = 2; k < face.nverts; k++)
                    {
                        indices_.push_back(face.verts[0]);
                        
                        if (invertNormals)
                        {
                            indices_.push_back(face.verts[k]);
                            indices_.push_back(face.verts[k-1]);
                        } else
                        {
                            indices_.push_back(face.verts[k-1]);
                            indices_.push_back(face.verts[k]);
                        }
                    }
                }
                
            }
        }
        
        /* print out the properties we got, for debugging */
        for (int j = 0; j < nprops; j++)
        {
            std::cout << "   Property " << plist[j]->name << "\n";
            std::cout.flush();
        }
    }
    
    /* grab and print out the comments in the file */
    int num_comments;
    char **comments = ply_get_comments (ply, &num_comments);
    for (int i = 0; i < num_comments; i++)
    {
        std::cout << "   Comment = " << comments[i] << "\n";
        std::cout.flush();
    }
    
    /* grab and print out the object information */
    int num_obj_info;
    char **obj_info = ply_get_obj_info (ply, &num_obj_info);
    for (int i = 0; i < num_obj_info; i++)
    {
        std::cout << "   Obj_info " << obj_info[i] << "\n";
        std::cout.flush();
    }
    
    /* close the PLY file */
    ply_close (ply);
    //==============================
    
    
    
    updateBoundingVolume();
    
    std::cout << "done.\n";
    std::cout.flush();
    return true;
}


//=======================================================================//
bool stitch::PolygonModel::loadOBJVertices(const std::string fileName, const stitch::Vec3 &centre, const float scale, const bool invertNormals)
{
    vertNormals_.clear();
    vertCoords_.clear();
    indices_.clear();
    
    std::cout << "Loading OBJ file" << fileName << "...\n";
    std::cout.flush();
    
    //=== Read OBJ file ===//
    std::ifstream fin(fileName.c_str(), std::ios::in);
    
    while (!fin.eof())
    {
        std::string keyword;
        
        //read the keyword and then the data.
        fin >> keyword;
        
        if (keyword=="v")
        {
            double x,y,z;
            fin >> x >> y >> z;
            //std::cout << "V: (" << x << ", " << y << ", " << z << ")\n";
            //std::cout.flush();
            
            Vec3 vert(x,y,z);
            vertCoords_.push_back(vert*scale + centre);
#ifdef USE_CXX11
            vertNormals_.emplace_back(1.0, 0.0, 0.0);
#else
            vertNormals_.push_back(stitch::Vec3(1.0, 0.0, 0.0));
#endif
        } else
            if (keyword=="vt")
            {
                char data[1024]; //Could be 2D.
                fin.getline(data,1024);
            } else
                if (keyword=="vn")
                {
                    char data[1024];
                    fin.getline(data,1024);
                } else
                    if (keyword=="f")
                    {
                        char data[1024];
                        fin.getline(data,1024);
                        
                        std::vector<size_t> vIndexVec;
                        
                        char *fDesc=data;
                        char *vDesc=nullptr;
                        
                        do
                        {//Read a vertex.
                            vDesc=strsep(&fDesc, " \t\n\r");
                            
                            if ((*vDesc)!=0)
                            {//There is a vertex to process.
                                char *ptr=nullptr;
                                long vertIndex=strtol(vDesc, &ptr, 10);
                                if (vertIndex<1) vertIndex=((int)vertCoords_.size())+vertIndex;
                                
                                vIndexVec.push_back(vertIndex-1);
                                
                                while (*ptr!=0)
                                {//Read the rest of the vertex attrbs.
                                    /*long attrb=*/strtol(ptr+1, &ptr, 10);
                                    
                                }
                            }
                            
                        } while (fDesc!=0);
                        
                        size_t v0=vIndexVec[0];
                        
                        for (size_t i=2; i<vIndexVec.size(); i++)
                        {
                            size_t v1=vIndexVec[i-1];
                            size_t v2=vIndexVec[i];
                            
                            indices_.push_back(v0);
                            if (!invertNormals)
                            {
                                indices_.push_back(v2);
                                indices_.push_back(v1);
                            } else
                            {
                                indices_.push_back(v1);
                                indices_.push_back(v2);
                            }
                        }
                        
                    } else
                    {
                        char data[1024];
                        fin.getline(data,1024);
                        
                        std::cout << keyword << " " << data << "\n";
                        std::cout.flush();
                    }
        
    }
    
    fin.close();
    //=====================//
    
    updateBoundingVolume();
    
    std::cout << "done.\n";
    std::cout.flush();
    return true;
}

bool stitch::PolygonModel::loadMDLVertices(const std::string fileName, const stitch::Vec3 &centre, const float scale, const bool invertNormals)
{
    vertNormals_.clear();
    vertCoords_.clear();
    indices_.clear();
    
    updateBoundingVolume();
    
    std::cout << "done.\n";
    std::cout.flush();
    return true;
}

bool stitch::PolygonModel::loadTetrahedron(const stitch::Vec3 &centre, const float scale)
{
    vertNormals_.clear();
    vertCoords_.clear();
    indices_.clear();
    
    vertCoords_.push_back(stitch::Vec3(0.0f, 0.0f, 1.0f)*scale+centre);
    vertCoords_.push_back(stitch::Vec3(cosf((0.0f/180.f) * ((float)M_PI))*cosf((-19.47f/180.f) * ((float)M_PI)),
                                       sinf((0.0f/180.f) * ((float)M_PI))*cosf((-19.47f/180.f) * ((float)M_PI)),
                                       (         1.0f          )*sinf((-19.47f/180.f) * ((float)M_PI)))*scale+centre);
    
    vertCoords_.push_back(stitch::Vec3(cosf((-120.0f/180.f) * ((float)M_PI))*cosf((-19.47f/180.f) * ((float)M_PI)),
                                       sinf((-120.0f/180.f) * ((float)M_PI))*cosf((-19.47f/180.f) * ((float)M_PI)),
                                       (           1.0f           )*sinf((-19.47f/180.f) * ((float)M_PI)))*scale+centre);
    
    vertCoords_.push_back(stitch::Vec3(cosf((-240.0f/180.f) * ((float)M_PI))*cosf((-19.47f/180.f) * ((float)M_PI)),
                                       sinf((-240.0f/180.f) * ((float)M_PI))*cosf((-19.47f/180.f) * ((float)M_PI)),
                                       (           1.0f           )*sinf((-19.47f/180.f) * ((float)M_PI)))*scale+centre);
    
    vertNormals_.push_back((vertCoords_[0]-centre).normalised());
    vertNormals_.push_back((vertCoords_[1]-centre).normalised());
    vertNormals_.push_back((vertCoords_[2]-centre).normalised());
    vertNormals_.push_back((vertCoords_[3]-centre).normalised());
    
    indices_.push_back(0);
    indices_.push_back(2);
    indices_.push_back(1);
    
    indices_.push_back(0);
    indices_.push_back(3);
    indices_.push_back(2);
    
    indices_.push_back(0);
    indices_.push_back(1);
    indices_.push_back(3);
    
    indices_.push_back(1);
    indices_.push_back(2);
    indices_.push_back(3);
    
    updateBoundingVolume();
    
    return true;
}


bool stitch::PolygonModel::loadVectorsAndIndices(const std::vector<stitch::Vec3> &vectors,
                                                 const std::vector<size_t> &indices,
                                                 const stitch::Vec3 &centre, const float scale,
                                                 const stitch::Vec3 &upVector,
                                                 bool smoothNormals)
{
    vertNormals_.clear();
    vertCoords_.clear();
    indices_.clear();
    
    
    const stitch::Vec3 orthA=(upVector.orthVec()).normalised();
    const stitch::Vec3 orthB=stitch::Vec3::crossNormalised(upVector, orthA);
    
    const stitch::Mat4 rotMat(orthA.x(), orthB.x(), upVector.x(), 0.0f,
                              orthA.y(), orthB.y(), upVector.y(), 0.0f,
                              orthA.z(), orthB.z(), upVector.z(), 0.0f,
                              0.0f, 0.0f, 0.0f, 1.0f);
    
    if (smoothNormals) smoothSurface_=true;
    
    if (!smoothNormals)
    {
        const size_t numIndices=indices.size();
        
        for (size_t indexNum=0; indexNum<numIndices; indexNum+=3)
        {
            size_t i0=indices[indexNum+0];
            size_t i1=indices[indexNum+1];
            size_t i2=indices[indexNum+2];
            
            stitch::Vec3 vert0=vectors[i0];
            stitch::Vec3 vert1=vectors[i1];
            stitch::Vec3 vert2=vectors[i2];
            
            const stitch::Vec3 normal=stitch::Vec3::crossNormalised(vert1, vert2, vert0);
            
            vertCoords_.push_back(stitch::Vec3(rotMat * stitch::Vec4(vert0, 1.0f)) * scale + centre);
            vertNormals_.push_back(stitch::Vec3(rotMat * stitch::Vec4(normal, 0.0f)));
            indices_.push_back(vertCoords_.size()-1);
            
            vertCoords_.push_back(stitch::Vec3(rotMat * stitch::Vec4(vert1, 0.0f)) * scale + centre);
            vertNormals_.push_back(stitch::Vec3(rotMat * stitch::Vec4(normal, 0.0f)));
            indices_.push_back(vertCoords_.size()-1);
            
            vertCoords_.push_back(stitch::Vec3(rotMat * stitch::Vec4(vert2, 0.0f)) * scale + centre);
            vertNormals_.push_back(stitch::Vec3(rotMat * stitch::Vec4(normal, 0.0f)));
            indices_.push_back(vertCoords_.size()-1);
        }
    } else
    {
        const size_t numVertices=vectors.size();
        
        for (size_t vertexNum=0; vertexNum<numVertices; ++vertexNum)
        {
            vertCoords_.push_back(stitch::Vec3(rotMat * stitch::Vec4(vectors[vertexNum], 1.0f)) * scale + centre);
            vertNormals_.push_back(stitch::Vec3(rotMat * stitch::Vec4(vectors[vertexNum], 0.0f)));
        }
        
        const size_t numIndices=indices.size();
        for (size_t indexNum=0; indexNum<numIndices; ++indexNum)
        {
            indices_.push_back(indices[indexNum]);
        }
    }
    
    updateBoundingVolume();
    
    return true;
}

bool stitch::PolygonModel::loadFibonacciSpiralSphere(size_t numVertices, const stitch::Vec3 &centre, const float scale, bool smoothNormals)
{
    std::vector<size_t> binIndices;
    std::vector<stitch::Vec3> vectors;
    
    stitch::Vec3::equidistantVectors_FibonacciSpiralSphere(numVertices, vectors, binIndices);
    
    return loadVectorsAndIndices(vectors, binIndices, centre, scale, Vec3(0.0f, 1.0f, 0.0f), smoothNormals);
}

bool stitch::PolygonModel::loadIcosahedronBasedSphere(const size_t minimumNumVertices, const stitch::Vec3 &centre, const float scale, bool smoothNormals)
{
    std::vector<size_t> binIndices;
    std::vector<stitch::Vec3> vectors;
    
    stitch::Vec3::equidistantVectors_IcosahedronBased(minimumNumVertices, vectors, binIndices);
    
    return loadVectorsAndIndices(vectors, binIndices, centre, scale, Vec3(0.0f, 1.0f, 0.0f), smoothNormals);
}

void stitch::PolygonModel::updateBoundingVolume()
{
    //=== Update bounding volume ===
    if (vertCoords_.size()!=0)
    {
        centre_=Vec3(0.0f, 0.0f, 0.0f);
        radiusBV_=0.0f;
        std::vector<Vec3>::const_iterator vertexIter=vertCoords_.begin();
        for (; vertexIter!=vertCoords_.end(); ++vertexIter)
        {
            centre_+=*vertexIter;
        }
        centre_*=1.0f/vertCoords_.size();
        
        float maxRadiusSq=0.0f;
        for (vertexIter=vertCoords_.begin(); vertexIter!=vertCoords_.end(); ++vertexIter)
        {
            float radiusSq=Vec3::calcDistToPointSq((*vertexIter), centre_);
            
            if (radiusSq > maxRadiusSq)
            {
                maxRadiusSq=radiusSq;
            }
        }
        radiusBV_=sqrtf(maxRadiusSq);
    }
    //==============================
}

void stitch::PolygonModel::calcIntersection(const stitch::Ray &ray, Intersection &intersect) const
{
    uint32_t incomingObectID=intersect.itemID_; //Store the incoming ObjectID to ...
    
    if (ballTree_!=nullptr)
    {
        ballTree_->calcIntersection(ray, intersect);
    } else
    {
        //if (BVIntersected(orig, normDir)) This is currently executed by the calling code!
        {
            std::vector<size_t>::const_iterator faceIndexIter=indices_.begin();
            const std::vector<size_t>::const_iterator faceIndexIterEnd=indices_.end();
            
            for (; faceIndexIter!=faceIndexIterEnd; faceIndexIter+=3)
            {
                const Vec3 &v0=vertCoords_[*(faceIndexIter+0)];
                const Vec3 &v1=vertCoords_[*(faceIndexIter+1)];
                const Vec3 &v2=vertCoords_[*(faceIndexIter+2)];
                
                const Vec3 planeNormal_unnormalised=stitch::Vec3::cross(v2, v1, v0);
                const float d=v0 * planeNormal_unnormalised;
                
                const float dotp=(planeNormal_unnormalised*ray.direction_);
                
                if (dotp!=0.0f)
                {
                    const float intersectDist=(d-planeNormal_unnormalised*ray.origin_)/dotp;
                    
                    if ((intersectDist>0.0f)&&(intersectDist<intersect.distance_))
                    {
                        const Vec3 p=ray.origin_ + ray.direction_*intersectDist;
                        
                        const float lap=Vec3::calcDistToPoint(v2, v1);
                        const float lbp=Vec3::calcDistToPoint(v0, v2);
                        const float lcp=Vec3::calcDistToPoint(v1, v0);
                        
                        const float la_=((p-v1)*(v2-v1))/lap;
                        const float lb_=((p-v2)*(v0-v2))/lbp;
                        const float lc_=((p-v0)*(v1-v0))/lcp;
                        
                        const float la=sqrtf(Vec3::calcDistToPointSq(p, v1) - la_*la_);
                        const float lb=sqrtf(Vec3::calcDistToPointSq(p, v2) - lb_*lb_);
                        const float lc=sqrtf(Vec3::calcDistToPointSq(p, v0) - lc_*lc_);
                        
                        const float Aa=0.5f * la * lap;
                        const float Ab=0.5f * lb * lbp;
                        const float Ac=0.5f * lc * lcp;
                        
                        const float A=0.5f * (planeNormal_unnormalised).length();
                        
                        if ((Aa+Ab+Ac)<=(A*1.001f))
                        {
                            intersect.distance_=intersectDist;
                            
                            intersect.normal_.setToSumScaleAndNormalise(vertNormals_[*(faceIndexIter+0)], Aa, vertNormals_[*(faceIndexIter+1)], Ab, vertNormals_[*(faceIndexIter+2)], Ac);
                            
                            intersect.itemID_=this->itemID_ | ((intersect.normal_*ray.direction_>0.0f)?0:1);//back surface gets even ID, front surface gets odd ID.
                            intersect.itemPtr_=this;
                        }
                    }
                }
            }
        }
        
    }
    
    if ((smoothSurface_)&&(intersect.itemID_!=incomingObectID))
    {//Override the polygon's object id to that of the polygonModel if it is a smoothed surface and therefore one primitive!!!
        intersect.itemID_=this->itemID_|(intersect.itemID_&1);
        intersect.itemPtr_=this;
    }
}

