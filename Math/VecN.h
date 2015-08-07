/*
 * $Id: VecN.h 299 2015-08-07 14:57:10Z bernardt.duvenhage $
 */
/*
 *  VecN.h
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2011/04/29.
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

#ifndef STITCH_VECN_H
#define STITCH_VECN_H

namespace stitch {
	struct Vec3;
	class Vec4;
	class VecN;
}

#include "GlobalRand.h"

#include <cmath>
#include <vector>

//#define USE_STL_VEC_VECN 1
#define VECN_SIZE 4ul //must be bigger or equal to 4 to support the assumptions of the implementation!


//===	
namespace stitch {
    
    //! N-dimentional vector class.
	class VecN
	{
	public:
        
        //Note: This class was designed for (but is not limited to) the workflow of having a vector that is either:
        // * unitialised with zero length but zero values implied for all components or,
        // * of length n for n fixed throughout the program.
        
        //! Nullary constructor.
		VecN(void)
		{
#ifndef USE_STL_VEC_VECN
            c_=new float[VECN_SIZE];
            
            for (size_t i=0; i<VECN_SIZE; ++i)
            {
                c_[i]=0.0f;
            }
#endif
        }
        
    private:
		VecN(const size_t numComponents)
		{
#ifdef USE_STL_VEC_VECN        
            c_.assign(numComponents, 0.0f);
#else
            c_=new float[VECN_SIZE];
            
            for (size_t i=0; i<VECN_SIZE; ++i)
            {
                c_[i]=0.0f;
            }
#endif
        }
        
        //! Scalar constructor.
		VecN(const size_t numComponents, const float value)
		{
#ifdef USE_STL_VEC_VECN        
            c_.assign(numComponents, value);
#else
            c_=new float[VECN_SIZE];

            for (size_t i=0; i<VECN_SIZE; ++i)
            {
                c_[i]=value;
            }
#endif
        }
        
    public:
        //! Vector constructor.
		VecN(const float x, const float y, const float z)
		{
#ifdef USE_STL_VEC_VECN        
            c_.push_back(x);
            c_.push_back(y);
            c_.push_back(z);
#else
            c_=new float[VECN_SIZE];

            c_[0]=x;
            c_[1]=y;
            c_[2]=z;
            for (size_t i=3; i<VECN_SIZE; ++i)
            {
                c_[i]=0.0f;
            }
#endif
        }
		
        //! Vector constructor.
		VecN(const float x, const float y, const float z, const float w)
		{
#ifdef USE_STL_VEC_VECN        
            c_.push_back(x);
            c_.push_back(y);
            c_.push_back(z);
            c_.push_back(w);
#else
            c_=new float[VECN_SIZE];

            c_[0]=x;
            c_[1]=y;
            c_[2]=z;
            c_[3]=w;
            for (size_t i=4; i<VECN_SIZE; ++i)
            {
                c_[i]=0.0f;
            }
#endif
        }
        
        //! Copy constructor.
        VecN(const VecN &lValue)
		{
#ifdef USE_STL_VEC_VECN        
            c_=lValue.c_;
#else
            c_=new float[VECN_SIZE];

            for (size_t i=0; i<VECN_SIZE; ++i)
            {
                c_[i]=lValue.c_[i];
            }
#endif
        }
		
#ifdef USE_CXX11
        //! Move constructor.
        VecN(VecN &&rValue) noexcept
#ifdef USE_STL_VEC_VECN
        : c_(std::move(rValue.c_))
#else
        : c_(rValue.c_)
#endif
		{
#ifndef USE_STL_VEC_VECN
            rValue.c_=nullptr;
#endif
        }
#endif// USE_CXX11

        //! Conversion constructor.
		explicit VecN(const Vec3 &lValue);

        //! Conversion constructor.
		explicit VecN(const Vec4 &lValue);
        
        ~VecN()
        {
#ifndef USE_STL_VEC_VECN
            delete [] c_;
#endif
        }
		
        inline size_t getNumComponents() const
        {
#ifdef USE_STL_VEC_VECN        
            return c_.size();
#else
            return VECN_SIZE;
#endif
        }
        
        //! Equals overload.
		inline VecN & operator = (const VecN &lValue)
		{
#ifdef USE_STL_VEC_VECN        
            c_ = lValue.c_;
#else
            for (size_t i=0; i<VECN_SIZE; ++i)
            {
                c_[i]=lValue.c_[i];
            }
#endif
            
			return (*this);
		}
		
#ifdef USE_CXX11
        //! Equals overload.
		inline VecN & operator = (VecN &&rValue) noexcept
		{
#ifdef USE_STL_VEC_VECN
            c_ = std::move(rValue.c_);
#else
            delete [] c_;
            c_=rValue.c_;
            rValue.c_=nullptr;
#endif
            
			return (*this);
		}
#endif// USE_CXX11

        //! Equals conversion overload.
		VecN & operator = (const Vec3 &lValue);
        
        //! Equals conversion overload.
		VecN & operator = (const Vec4 &lValue);

		        
		inline void setScalar(const float s)
		{
#ifdef USE_STL_VEC_VECN
            const size_t size=c_.size();
            for (size_t i=0; i<size; ++i)
            {
                c_[i]=s;
            }
#else
            for (size_t i=0; i<VECN_SIZE; ++i)
            {
                c_[i]=s;
            }
#endif
		}
        
        inline void setZeros()
        {
#ifdef USE_STL_VEC_VECN        
            const size_t size=c_.size();
            for (size_t i=0; i<size; ++i)
            {
                c_[i]=0.0f;
            }
#else
            for (size_t i=0; i<VECN_SIZE; ++i)
            {
                c_[i]=0.0f;
            }
#endif
        }
        
        inline void setOnes()
        {
#ifdef USE_STL_VEC_VECN
            const size_t size=c_.size();
            for (size_t i=0; i<size; ++i)
            {
                c_[i]=1.0f;
            }
#else
            for (size_t i=0; i<VECN_SIZE; ++i)
            {
                c_[i]=1.0f;
            }
#endif
        }

        inline VecN oneMinus() const
        {
#ifdef USE_STL_VEC_VECN        
            VecN one(c_.size(), 1.0f);
#else
            VecN one(VECN_SIZE, 1.0f);
#endif
			return (one-(*this));
        }
        		
        inline float x() const
        {
#ifdef USE_STL_VEC_VECN        
            if (c_.size()>=1)
            {
                return c_[0];
            } else 
            {
                return 0.0f;
            }
#else
            return c_[0];
#endif
        }
        inline float & x()
        {
#ifdef USE_STL_VEC_VECN        
            if (c_.size()>=1)
            {
                return c_[0];
            } else 
            {
                return abyss_;
            }
#else
            return c_[0];
#endif
        }
        
        inline float y() const
        {
#ifdef USE_STL_VEC_VECN        
            if (c_.size()>=2)
            {
                return c_[1];
            } else 
            {
                return 0.0f;
            }
#else
            return c_[1];
#endif
        }
        inline float & y()
        {
#ifdef USE_STL_VEC_VECN        
            if (c_.size()>=2)
            {
                return c_[1];
            } else 
            {
                return abyss_;
            }
#else
            return c_[1];
#endif
        }
        
        inline float z() const
        {
#ifdef USE_STL_VEC_VECN        
            if (c_.size()>=3)
            {
                return c_[2];
            } else 
            {
                return 0.0f;
            }
#else
            return c_[2];
#endif
        }
        inline float & z()
        {
#ifdef USE_STL_VEC_VECN        
            if (c_.size()>=3)
            {
                return c_[2];
            } else 
            {
                return abyss_;
            }
#else
            return c_[2];
#endif
        }
        
        inline float w() const
        {
#ifdef USE_STL_VEC_VECN        
            if (c_.size()>=4)
            {
                return c_[3];
            } else 
            {
                return 0.0f;
            }
#else
            return c_[3];
#endif
        }
        inline float & w()
        {
#ifdef USE_STL_VEC_VECN        
            if (c_.size()>=4)
            {
                return c_[3];
            } else 
            {
                return abyss_;
            }
#else
            return c_[3];
#endif
        }
        
        //! Component access index operator.
        inline float operator [] (const size_t i) const
        {
#ifdef USE_STL_VEC_VECN        
            if (c_.size()>i)
            {
                return c_[i];
            } else 
            {
                return 0.0f;
            }
#else
            return c_[i];
#endif
        }

        //! Component reference access index operator.
        inline float & operator [] (const size_t i)
        {
#ifdef USE_STL_VEC_VECN        
            if (c_.size()>i)
            {
                return c_[i];
            } else 
            {
                return abyss_;
            }
#else
            return c_[i];
#endif
        }

        //===

		inline VecN operator * (const float lValue) const
		{
#ifdef USE_STL_VEC_VECN        
            const size_t size=c_.size();
#else
            const size_t size=VECN_SIZE;
#endif
            VecN returnVecN=*this;
            
            for (size_t i=0; i<size; ++i)
            {
                returnVecN.c_[i]*=lValue;
            }
            
			return returnVecN;
		}
        
		inline VecN &operator *= (const float lValue)
		{
#ifdef USE_STL_VEC_VECN        
            const size_t size=c_.size();
#else
            const size_t size=VECN_SIZE;
#endif
            
            for (size_t i=0; i<size; ++i)
            {
                c_[i]*=lValue;
            }
            
            return *this;
		}
        
		inline VecN operator / (const float lValue) const
		{
#ifdef USE_STL_VEC_VECN        
            const size_t size=c_.size();
#else
            const size_t size=VECN_SIZE;
#endif
            const float reciplValue=1.0f/lValue;
            VecN returnVecN=*this;
            
            for (size_t i=0; i<size; ++i)
            {
                returnVecN.c_[i]*=reciplValue;
            }
            
			return returnVecN;
		}
        
		inline VecN &operator /= (const float lValue)
		{
#ifdef USE_STL_VEC_VECN        
            const size_t size=c_.size();
#else
            const size_t size=VECN_SIZE;
#endif
            const float reciplValue=1.0f/lValue;
            
            for (size_t i=0; i<size; ++i)
            {
                c_[i]*=reciplValue;
            }
            
            return *this;
		}
        
        //===
        
        //! Dot product.
		inline float operator * (const VecN &lValue) const
		{
#ifdef USE_STL_VEC_VECN        
            const size_t computeSize=min(c_.size(), lValue.c_.size());
#else
            const size_t computeSize=VECN_SIZE;
#endif
            float result=0.0f;
            
            for (size_t i=0; i<computeSize; ++i)
            {
                result+=c_[i]*lValue.c_[i];
            }
            
			return result;
		}
        
        //! Component wise multiply.
		inline VecN cmult(const VecN &lValue) const //component wise multiply.
		{
#ifdef USE_STL_VEC_VECN        
            const size_t size=c_.size();
            const size_t lValueSize=lValue.c_.size();
            const size_t resultSize=max(size, lValueSize);
            const size_t computeSize=min(size, lValueSize);
            
            VecN returnVecN(resultSize);
#else
            const size_t computeSize=VECN_SIZE;
            
            VecN returnVecN;
#endif
            
            for (size_t i=0; i<computeSize; ++i)
            {
                returnVecN.c_[i]=c_[i]*lValue.c_[i];
            }
            
			return returnVecN;
		}
				
        //! Component wise multiply.
		inline VecN &operator *= (const VecN &lValue)
		{
#ifdef USE_STL_VEC_VECN
            const size_t size=c_.size();
            const size_t lValueSize=lValue.c_.size();
            const size_t computeSize=min(size, lValueSize);
#else
            const size_t computeSize=VECN_SIZE;
#endif
            
            for (size_t i=0; i<computeSize; ++i)
            {
                c_[i]*=lValue.c_[i];
            }
            
            return *this;
		}
        //===
        
		inline VecN operator + (const VecN &lValue) const
		{
#ifdef USE_STL_VEC_VECN        
            const size_t size=c_.size();
            const size_t lValueSize=lValue.c_.size();
            const size_t resultSize=max(size, lValueSize);
            
            VecN returnVecN(resultSize);
            
            for (size_t i=0; i<size; ++i)
            {
                returnVecN.c_[i]=c_[i];
            }
#else
            const size_t lValueSize=VECN_SIZE;
            VecN returnVecN=*this;
#endif
            for (size_t i=0; i<lValueSize; ++i)
            {
                returnVecN.c_[i]+=lValue.c_[i];
            }            
            
			return returnVecN;
		}
        
		inline VecN operator + (const float lValue) const
		{
#ifdef USE_STL_VEC_VECN        
            const size_t size=c_.size();
#else
            const size_t size=VECN_SIZE;
#endif
            VecN returnVecN=*this;
            for (size_t i=0; i<size; ++i)
            {
                returnVecN.c_[i]+=lValue;
            }
            
            return returnVecN;
		}
        
		inline VecN &operator += (const float lValue)
		{
#ifdef USE_STL_VEC_VECN        
            const size_t size=c_.size();
#else
            const size_t size=VECN_SIZE;
#endif
            
            for (size_t i=0; i<size; ++i)
            {
                c_[i]+=lValue;
            }
            
            return *this;
		}
        
		inline VecN &operator += (const VecN &lValue)
		{
#ifdef USE_STL_VEC_VECN        
            const size_t size=c_.size();
            const size_t lValueSize=lValue.c_.size();
            
            if (size>=lValueSize)
            {
                for (size_t i=0; i<lValueSize; ++i)
                {
                    c_[i]+=lValue[i];
                }
                    
            } else
            {
                (*this)=VecN(*this)+lValue;
            }
#else
            for (size_t i=0; i<VECN_SIZE; ++i)
            {
                c_[i]+=lValue[i];
            }
#endif
			return *this;
		}
                
		inline VecN &addScaled(const VecN &lValue, const float scale)
		{
#ifdef USE_STL_VEC_VECN
            size_t size=c_.size();
            size=min(size, lValue.c_.size());
            
                for (size_t i=0; i<size; ++i)
                {
                    c_[i]+=lValue[i]*scale;
                }
#else
            for (size_t i=0; i<VECN_SIZE; ++i)
            {
                c_[i]+=lValue[i]*scale;
            }
#endif
			return *this;
		}
        
		inline VecN &addScaled(const VecN &lValue, const VecN &scale)
		{
#ifdef USE_STL_VEC_VECN
            size_t size=c_.size();
            size=min(size, lValue.c_.size());
            size=min(size, scale.c_.size());
            
            for (size_t i=0; i<size; ++i)
            {
                c_[i]+=lValue[i]*scale[i];
            }
#else
            for (size_t i=0; i<VECN_SIZE; ++i)
            {
                c_[i]+=lValue[i]*scale[i];
            }
#endif
			return *this;
		}
        //===
        
		inline VecN operator - (const VecN &lValue) const
		{
#ifdef USE_STL_VEC_VECN        
            const size_t size=c_.size();
            const size_t lValueSize=lValue.c_.size();
            const size_t resultSize=max(size, lValueSize);
            
            VecN returnVecN(resultSize);
            
            for (size_t i=0; i<size; ++i)
            {
                returnVecN.c_[i]=c_[i];
            }
#else
            const size_t lValueSize=VECN_SIZE;
            VecN returnVecN=*this;
#endif
            for (size_t i=0; i<lValueSize; ++i)
            {
                returnVecN.c_[i]-=lValue.c_[i];
            }            
            
			return returnVecN;
		}
        
		inline VecN operator - (const float lValue) const
		{
#ifdef USE_STL_VEC_VECN        
            const size_t size=c_.size();
#else
            const size_t size=VECN_SIZE;
#endif
            VecN returnVecN=*this;
            for (size_t i=0; i<size; ++i)
            {
                returnVecN.c_[i]-=lValue;
            }
            
            return returnVecN;
		}
        
		inline VecN &operator -= (const float lValue)
		{
#ifdef USE_STL_VEC_VECN        
            const size_t size=c_.size();
#else
            const size_t size=VECN_SIZE;
#endif
            
            for (size_t i=0; i<size; ++i)
            {
                c_[i]-=lValue;
            }
            
            return *this;
		}
        
		inline VecN &operator -= (const VecN &lValue)
		{
#ifdef USE_STL_VEC_VECN        
            const size_t size=c_.size();
            const size_t lValueSize=lValue.c_.size();
            
            if (size>=lValueSize)
            {
                for (size_t i=0; i<lValueSize; ++i)
                {
                    c_[i]-=lValue[i];
                }
                
            } else
            {
                (*this)=VecN(*this)-lValue;
            }
#else
            for (size_t i=0; i<VECN_SIZE; ++i)
            {
                c_[i]-=lValue[i];
            }
#endif
			return *this;
		}
        
        //===
        
		inline float length() const
		{
#ifdef USE_STL_VEC_VECN        
            const size_t size=c_.size();
#else
            const size_t size=VECN_SIZE;
#endif
            float resultSq=0.0f;
            
            for (size_t i=0; i<size; ++i)
            {
                resultSq+=c_[i]*c_[i];
            }
            
			return sqrt(resultSq);
		}
        
		inline float lengthSq() const
		{
#ifdef USE_STL_VEC_VECN        
            const size_t size=c_.size();
#else
            const size_t size=VECN_SIZE;
#endif
            float resultSq=0.0f;
            
            for (size_t i=0; i<size; ++i)
            {
                resultSq+=c_[i]*c_[i];
            }
            
			return resultSq;
		}
        
        //! Component sum.
		inline float csum() const
		{
#ifdef USE_STL_VEC_VECN        
            const size_t size=c_.size();
#else
            const size_t size=VECN_SIZE;
#endif
            float result=0.0f;
            
            for (size_t i=0; i<size; ++i)
            {
                result+=c_[i];
            }
            
            return result;
		}
        
        //! Component average.
		inline float cavrg() const
		{
#ifdef USE_STL_VEC_VECN
            const size_t size=c_.size();
#else
            const size_t size=VECN_SIZE;
#endif
            float result=0.0f;
            
            for (size_t i=0; i<size; ++i)
            {
                result+=c_[i];
            }
            
            return result/size;
		}

        //! Normalise this vector and return what the length it WAS.
		inline float normalise()
		{
			const float vecLength=length();
			const float recipVecLength=1.0f/vecLength;
            
#ifdef USE_STL_VEC_VECN        
            const size_t size=c_.size();
#else
            const size_t size=VECN_SIZE;
#endif
            
            for (size_t i=0; i<size; ++i)
            {
                c_[i]*=recipVecLength;
            }
            
            return vecLength;
		}
        
        //! Return a normalised vector.
		inline VecN normalised() const
		{
            VecN returnVecN=*this;

            returnVecN.normalise();
            
            return returnVecN;
		}
		
        //! Make area under this vector components one and return what the component sum was.
		inline float cnormalise()
		{
			const float cSum=csum();
			const float recipCSum=1.0f/cSum;
            
#ifdef USE_STL_VEC_VECN        
            const size_t size=c_.size();
#else
            const size_t size=VECN_SIZE;
#endif
            
            for (size_t i=0; i<size; ++i)
            {
                c_[i]*=recipCSum;
            }
            
            return cSum;
		}
        
        //! Return a component normalised vector.
		inline VecN cnormalised() const
		{
            VecN returnVecN=*this;
            
            returnVecN.cnormalise();
            
            return returnVecN;
		}

        //! Raise each component to the power e.
        inline VecN toThePower(const float e) const
        {
            VecN returnVecN=*this;
            
#ifdef USE_STL_VEC_VECN        
            const size_t size=returnVecN.c_.size();
#else
            const size_t size=VECN_SIZE;
#endif
            for (size_t i=0; i<size; ++i)
            {
                returnVecN.c_[i]=powf(returnVecN.c_[i], e);
            }
            
            return returnVecN;
        }
        
        //! Raise each component to the power e.
        inline VecN operator ^ (const float e) const
        {
            VecN returnVecN=*this;
            
#ifdef USE_STL_VEC_VECN        
            const size_t size=returnVecN.c_.size();
#else
            const size_t size=VECN_SIZE;
#endif
            for (size_t i=0; i<size; ++i)
            {
                returnVecN.c_[i]=powf(returnVecN.c_[i], e);
            }
            
            return returnVecN;
        }
        
        //! cross product.
        inline VecN operator ^ (const VecN &lValue) const
        {
#ifdef USE_STL_VEC_VECN        
            const size_t size=min(c_.size(), lValue.c_.size());

            if (size==3)
            {
                return VecN(c_[1]*lValue.c_[2] - lValue.c_[1]*c_[2],
                            c_[2]*lValue.c_[0] - lValue.c_[2]*c_[0],
                            c_[0]*lValue.c_[1] - lValue.c_[0]*c_[1]);
            } else
            if (size==2)
            {
                return VecN(0.0f, 
                            0.0f, 
                            c_[0]*lValue.c_[1] - lValue.c_[0]*c_[1]);
            }
                        
            return VecN();//return a zero vector.
#else
            return VecN(c_[1]*lValue.c_[2] - lValue.c_[1]*c_[2],
                        c_[2]*lValue.c_[0] - lValue.c_[2]*c_[0],
                        c_[0]*lValue.c_[1] - lValue.c_[0]*c_[1]);                
#endif
        }
        
        //! Make all components of this vector positive.
        inline void positivise()
        {
#ifdef USE_STL_VEC_VECN        
            const size_t size=c_.size();
#else
            const size_t size=VECN_SIZE;
#endif
            
            for (size_t i=0; i<size; ++i)
            {
                c_[i]=fabsf(c_[i]);
            }
        }

        //! Return a vector with all positive components.
		inline VecN positivised() const
		{
            VecN returnVecN=*this;
            
            returnVecN.positivise();
            
            return returnVecN;
		}
        

        
        //! Clamp all components between low and high.
        inline void clamp(const float low, const float high)
        {
#ifdef USE_STL_VEC_VECN        
            const size_t size=c_.size();
#else
            const size_t size=VECN_SIZE;
#endif
            
            for (size_t i=0; i<size; ++i)
            {
                //c_[i]=max(low, min(high, c_[i]));
                c_[i]=(c_[i]<low) ? low : ((c_[i] > high) ? high : c_[i]);
            }
        }
        
        //! Generate a Russian Roulette mask using the vector components as probabilities.
        inline VecN rouletteMask(const float range) const
        {
            VecN returnVecN=*this;
            
#ifdef USE_STL_VEC_VECN        
            const size_t size=c_.size();
#else
            const size_t size=VECN_SIZE;
#endif
            
            for (size_t i=0; i<size; ++i)
            {
                returnVecN.c_[i]=((GlobalRand::uniformSampler()*range)<(returnVecN.c_[i])) ? 1.0f : 0.0f;
            }
            
            return returnVecN;
        }
        

        inline bool operator == (const VecN &lValue) const
        {
#ifdef USE_STL_VEC_VECN        
            const size_t size=c_.size();
            
            if (size!=lValue.c_.size()) return false;
#else
            const size_t size=VECN_SIZE;
#endif
            
            for (size_t i=0; i<size; ++i)
            {
                if (c_[i]!=lValue.c_[i]) return false;
            }
            
            return true;
        }
        
        inline bool operator != (const VecN &lValue) const
        {
            return !((*this)==lValue);
        }

        
        inline bool isZero() const
        {
#ifdef USE_STL_VEC_VECN
            const size_t size=c_.size();
#else
            const size_t size=VECN_SIZE;
#endif
            
            for (size_t i=0; i<size; ++i)
            {
                if (c_[i]!=0.0f) return false;
            }

            return true;
        }
        

        static VecN rand(const size_t numComponents=VECN_SIZE);
        static VecN randBall(const size_t numComponents=VECN_SIZE);
        static VecN randNorm(const size_t numComponents=VECN_SIZE);
        
	private:
        
#ifdef USE_STL_VEC_VECN        
        std::vector<float> c_;
#else
        float *c_;
#endif
        
        static float abyss_;
	};
}

#endif// STITCH_VECN_H
