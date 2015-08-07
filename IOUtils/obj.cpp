// mesh.cpp: implementation of the obj mesh class.
//Adapted from: http://cggmwww.csie.nctu.edu.tw/courses/cgu/2002/prog1/mesh.cpp
//////////////////////////////////////////////////////////////////////

#include "obj.h"
#include <iostream>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ObjMesh::ObjMesh()
{
    sceneFILE_=0;
    sceneFileName_="";
}

ObjMesh::~ObjMesh()
{
}

void ObjMesh::init(const std::string sceneFileName)
{
    //Remove previous loaded mesh.
    matIDMap_.clear();
    matVector_.clear();
    vList_.clear();
    nList_.clear();
    tList_.clear();
    faceList_.clear();
    
    ObjMaterial material;
    
	material.Ka_[0] = 0.0; material.Ka_[1] = 0.0; material.Ka_[2] = 0.0; material.Ka_[3] = 1.0; 
	material.Kd_[0] = 1.0; material.Kd_[1] = 1.0; material.Kd_[2] = 1.0; material.Kd_[3] = 1.0; 
	material.Ks_[0] = 0.8; material.Ks_[1] = 0.8; material.Ks_[2] = 0.8; material.Ks_[3] = 1.0;
	material.Ns_ = 32;
    
    matVector_.push_back(material);
    matIDMap_["(null)"]=0;
    
    sceneFileName_=sceneFileName;
	LoadMesh();
}


void ObjMesh::LoadMesh()
{
	char    keyword[512];
	int	    cur_mat = 0;

	sceneFILE_ = fopen(sceneFileName_.c_str(),"r");

	if (!sceneFILE_) 
	{
        std::cout<< std::string("Can not open object File \"") << sceneFileName_ << "\" !" << std::endl;
		return;
	}


    vList_.push_back(stitch::Vec3(0.0f,0.0f,0.0f));
    nList_.push_back(stitch::Vec3(0.0f,0.0f,0.0f));
    tList_.push_back(stitch::Vec3(0.0f,0.0f,0.0f));
    
	while(!feof(sceneFILE_))
	{
		keyword[0] = 0;
		fscanf(sceneFILE_,"%s", keyword);

		if (!strcmp(keyword,"mtllib"))
		{
            char mat_file[512];		// material file name
  			fscanf(sceneFILE_,"%s", mat_file);
			LoadMaterial(std::string(mat_file));
		}

		else if (!strcmp(keyword,"usemtl"))
		{
            char matName[512];
			fscanf(sceneFILE_,"%s",matName);
			cur_mat = matIDMap_[sceneFileName_+std::string("_")+std::string(matName)];
		}

		else if (!strcmp(keyword,"v"))
		{
            float	x=0.0f, y=0.0f, z=0.0f;
			fscanf(sceneFILE_,"%f %f %f",&x,&y,&z);
			vList_.push_back(stitch::Vec3(x,y,z));
		}

		else if (!strcmp(keyword,"vn"))
		{
            float	x=0.0f, y=0.0f, z=0.0f;
			fscanf(sceneFILE_,"%f %f %f",&x,&y,&z);
			nList_.push_back(stitch::Vec3(x,y,z));
		}
        
		else if (!strcmp(keyword,"vt"))
		{
            float	x=0.0f, y=0.0f, z=0.0f;
			fscanf(sceneFILE_,"%f %f %f",&x,&y,&z);
			tList_.push_back(stitch::Vec3(x,y,z));
		}

		else if (!strcmp(keyword,"f"))
		{
			ObjFace	face;
            char data[512];

			fgets(data, 512, sceneFILE_);//Get entire face description line.
            
            char *fDesc=data;
            char *vDesc=0;
            
            do
            {
                vDesc=strsep(&fDesc, " \t\n");
                
                if ((*vDesc)!=0)
                {//There is a vertex to process.
                    ObjVertex vertex;
                    
                    char *ptr;
                    vertex.v_ = strtol(vDesc, &ptr, 10);
                    vertex.t_ = (*ptr==0) ? 0 : strtol(ptr+1, &ptr, 10);
                    vertex.n_ = (*ptr==0) ? 0 : strtol(ptr+1, &ptr, 10);

                    if (vertex.v_<0) vertex.v_=vList_.size()+vertex.v_;
                    if (vertex.t_<0) vertex.t_=tList_.size()+vertex.t_;
                    if (vertex.n_<0) vertex.n_=nList_.size()+vertex.n_;
                                        
                    vertex.m_ = cur_mat;
                    
                    face.v_.push_back(vertex);
                }
			} while (fDesc!=0);

			faceList_.push_back(face);
		}

		else if (!strcmp(keyword,"#"))
        {
            char data[512];
			fgets(data,512,sceneFILE_);
        }
        
		else
        {
            char data[512];
			fgets(data,512,sceneFILE_);
        }

	}

	if (sceneFILE_) fclose(sceneFILE_);

	printf("vetex: %d, normal: %d, texture: %d, triangles: %d\n",(int)vList_.size(), (int)nList_.size(), (int)tList_.size(), (int)faceList_.size());
}


void ObjMesh::LoadMaterial(const std::string &materialFileName)
{
	FILE *materialFILE = fopen(materialFileName.c_str(),"r");
    
	if (!materialFILE) 
	{
        std::cout << "Can't open material file \"" << materialFileName << "\"!" << std::endl;
		return;
	}

    std::cout<<materialFileName<<std::endl;

    
	char	key[512];
    
	while(!feof(materialFILE))
	{
		key[0] = 0;
		fscanf(materialFILE,"%s", key);

		if (!strcmp(key,"newmtl"))
		{
            char data[512];
			fscanf(materialFILE,"%s",data);
            matVector_.push_back(matVector_[0]);
			matIDMap_[sceneFileName_+std::string("_")+std::string(data)] = matVector_.size()-1; 	// matMap["material_name"] = material_id;
		}

		else if (!strcmp(key,"Ka"))
		{
            float	r=0.0f,g=0.0f,b=0.0f;
			fscanf(materialFILE,"%f %f %f",&r,&g,&b);
			matVector_[matVector_.size()-1].Ka_[0] = r;
			matVector_[matVector_.size()-1].Ka_[1] = g;
			matVector_[matVector_.size()-1].Ka_[2] = b;
			matVector_[matVector_.size()-1].Ka_[3] = 1.0f;
		}

		else if (!strcmp(key,"Kd"))
		{
            float	r=0.0f,g=0.0f,b=0.0f;
			fscanf(materialFILE,"%f %f %f",&r,&g,&b);
			matVector_[matVector_.size()-1].Kd_[0] = r;
			matVector_[matVector_.size()-1].Kd_[1] = g;
			matVector_[matVector_.size()-1].Kd_[2] = b;
			matVector_[matVector_.size()-1].Kd_[3] = 1.0f;
		}

		else if (!strcmp(key,"Ks"))
		{
            float	r=0.0f,g=0.0f,b=0.0f;
			fscanf(materialFILE,"%f %f %f",&r,&g,&b);
			matVector_[matVector_.size()-1].Ks_[0] = r;
			matVector_[matVector_.size()-1].Ks_[1] = g;
			matVector_[matVector_.size()-1].Ks_[2] = b;
			matVector_[matVector_.size()-1].Ks_[3] = 1.0f;
		}

		else if (!strcmp(key,"Ns"))
		{
            float	x=0.0f;
			fscanf(materialFILE, "%f", &x);
			matVector_[matVector_.size()-1].Ns_ = x;
		}

		else if (!strcmp(key,"#"))
        {
            char data[512];
			fgets(data, 511, materialFILE);
        }
        
		else
        {
            char data[512];
			fgets(data,512,sceneFILE_);
        }

//		printf("[%s]\n",token);
	}

	printf("total material:%d\n", (int)matIDMap_.size());

	if (materialFILE) fclose(materialFILE);
}

