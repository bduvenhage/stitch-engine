// obj.h: interface for the obj mesh class.
//Adapted from: http://cggmwww.csie.nctu.edu.tw/courses/cgu/2002/prog1/mesh.h
//////////////////////////////////////////////////////////////////////

#ifndef __OBJ_H__
#define __OBJ_H__

#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <vector>
#include <string>

#include "../Math/Vec3.h"


class ObjMaterial
{
public:
	float Ka_[4];
	float Kd_[4];
	float Ks_[4];
	float Ns_;

	ObjMaterial()
	{ 
		for (int i=0; i<4; i++)
        {
			Ka_[i] = Kd_[i] = Ks_[i] = 1.0f;
        }
        
		Ns_ = 0.0f;
	}
};


class ObjVertex
{
public:
    int v_;		// vertex (index of vList)
    int n_;		// normal (index of nList)
    int t_;		// texture (index of tList)
    int m_;		// material (index of material)
    
    ObjVertex() :
        v_(0), n_(0),
        t_(0), m_(0)
    {}
    
    ObjVertex(const int v_index, const int n_index, const int t_index=0, const int m_index=0)
    {
        v_ = v_index;
        n_ = n_index;
        t_ = t_index;
        m_ = m_index;
    }
};


class ObjFace
{
public:
    std::vector<ObjVertex> v_;
    
    ObjFace () 
    {
    }
};


class ObjMesh  
{
public:
    std::map<std::string, int> matIDMap_;
    std::vector<ObjMaterial> matVector_;

    std::vector<stitch::Vec3>	vList_;		// Vertex List
	std::vector<stitch::Vec3>	nList_;		// Normal List
	std::vector<stitch::Vec3>	tList_;		// Texture List
	std::vector<ObjFace>	    faceList_;	// Face List

	ObjMesh();
	virtual ~ObjMesh();

	void init(const std::string sceneFileName);
    
private:
	FILE *sceneFILE_;
    std::string sceneFileName_;
    
	void	LoadMesh();
	void	LoadMaterial(const std::string &materialFileName);    
};

#endif //__OBJ_H__
