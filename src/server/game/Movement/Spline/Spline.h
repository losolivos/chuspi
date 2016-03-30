/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef TRINITYSERVER_SPLINE_H
#define TRINITYSERVER_SPLINE_H

#include "MovementTypedefs.h"
#include "Errors.h"

#include <G3D/Vector3.h>
#include <limits>

namespace Movement {

class SplineBase
{
public:
    typedef size_t index_type;
    typedef std::vector<G3D::Vector3> ControlArray;

    enum EvaluationMode
    {
        ModeLinear,
        ModeCatmullrom,
        ModeBezier3_Unused,
        UninitializedMode,
        ModesEnd
    };

protected:
    ControlArray points;

    index_type index_lo;
    index_type index_hi;

    uint8 m_mode;
    bool cyclic;

    enum
    {
        // could be modified, affects segment length evaluation precision
        // lesser value saves more performance in cost of lower precision
        // minimal value is 1
        // client's value is 20, blizzs use 2-3 steps to compute length
        STEPS_PER_SEGMENT = 3
    };

    static_assert(STEPS_PER_SEGMENT > 0, "shouldn't be lesser than 1");

protected:
    void EvaluateLinear(index_type, float, G3D::Vector3&) const;
    void EvaluateCatmullRom(index_type, float, G3D::Vector3&) const;
    void EvaluateBezier3(index_type, float, G3D::Vector3&) const;
    typedef void (SplineBase::*EvaluationMethtod)(index_type, float, G3D::Vector3&) const;
    static EvaluationMethtod evaluators[ModesEnd];

    void EvaluateDerivativeLinear(index_type, float, G3D::Vector3&) const;
    void EvaluateDerivativeCatmullRom(index_type, float, G3D::Vector3&) const;
    void EvaluateDerivativeBezier3(index_type, float, G3D::Vector3&) const;
    static EvaluationMethtod derivative_evaluators[ModesEnd];

    float SegLengthLinear(index_type) const;
    float SegLengthCatmullRom(index_type) const;
    float SegLengthBezier3(index_type) const;
    typedef float (SplineBase::*SegLenghtMethtod)(index_type) const;
    static SegLenghtMethtod seglengths[ModesEnd];

    void InitLinear(const G3D::Vector3*, index_type, bool, index_type);
    void InitCatmullRom(const G3D::Vector3*, index_type, bool, index_type);
    void InitBezier3(const G3D::Vector3*, index_type, bool, index_type);
    typedef void (SplineBase::*InitMethtod)(const G3D::Vector3*, index_type, bool, index_type);
    static InitMethtod initializers[ModesEnd];

public:

    explicit SplineBase() : index_lo(0), index_hi(0), m_mode(UninitializedMode), cyclic(false) { }

    /** Caclulates the position for given segment Idx, and percent of segment length t
        @param t - percent of segment length, assumes that t in range [0, 1]
        @param Idx - spline segment index, should be in range [first, last)
     */
    void evaluate_percent(index_type Idx, float u, G3D::Vector3& c) const {(this->*evaluators[m_mode])(Idx, u, c);}

    /** Caclulates derivation in index Idx, and percent of segment length t
        @param Idx - spline segment index, should be in range [first, last)
        @param t  - percent of spline segment length, assumes that t in range [0, 1]
     */
    void evaluate_derivative(index_type Idx, float u, G3D::Vector3& hermite) const {(this->*derivative_evaluators[m_mode])(Idx, u, hermite);}

    /**  Bounds for spline indexes. All indexes should be in range [first, last). */
    index_type first() const { return index_lo;}
    index_type last()  const { return index_hi;}

    bool empty() const { return index_lo == index_hi;}
    EvaluationMode mode() const { return (EvaluationMode)m_mode;}
    bool isCyclic() const { return cyclic;}

    const ControlArray& getPoints() const { return points;}
    index_type getPointCount() const { return points.size();}
    const G3D::Vector3& getPoint(index_type i) const { return points[i];}

    /** Initializes spline. Don't call other methods while spline not initialized. */
    void init_spline(const G3D::Vector3* controls, index_type count, EvaluationMode m);
    void init_cyclic_spline(const G3D::Vector3* controls, index_type count, EvaluationMode m, index_type cyclic_point);

    /** As i can see there are a lot of ways how spline can be initialized
        would be no harm to have some custom initializers. */
    template<class Init> inline void init_spline(Init& initializer)
    {
        initializer(m_mode, cyclic, points, index_lo, index_hi);
    }

    void clear();

    /** Calculates distance between [i; i+1] points, assumes that index i is in bounds. */
    float SegLength(index_type i) const { return (this->*seglengths[m_mode])(i);}

    std::string ToString() const;
};

template<typename length_type>
class Spline : public SplineBase
{
public:
    typedef length_type LengthType;
    typedef std::vector<length_type> LengthArray;
protected:

    LengthArray lengths;

    index_type computeIndexInBounds(length_type length) const;
public:

    explicit Spline(){ }

    /** Calculates the position for given t
        @param t - percent of spline's length, assumes that t in range [0, 1]. */
    void evaluate_percent(float t, G3D::Vector3& c) const;

    using SplineBase::evaluate_percent;

    /** Calculates derivation for given t
        @param t - percent of spline's length, assumes that t in range [0, 1]. */
    void evaluate_derivative(float t, G3D::Vector3& hermite) const;

    using SplineBase::evaluate_derivative;

    // Assumes that t in range [0, 1]
    index_type computeIndexInBounds(float t) const;
    void computeIndex(float t, index_type& out_idx, float& out_u) const;

    /**  Initializes lengths with SplineBase::SegLength method. */
    void initLengths();

    /** Initializes lengths in some custom way
        Note that value returned by cacher must be greater or equal to previous value. */
    template<class T>
    void initLengths(T& cacher)
    {
        index_type i = index_lo;
        lengths.resize(index_hi + 1);
        length_type prevLength = 0, newLength = 0;
        while (i < index_hi)
        {
            newLength = cacher(*this, i);
            // length overflowed, assign to max positive value
            if (newLength < 0)
                newLength = std::numeric_limits<length_type>::max();
            lengths[++i] = newLength;

            ASSERT(prevLength <= newLength);
            prevLength = newLength;
        }
    }

    /** Returns length of the whole spline. */
    length_type length() const { return lengths[index_hi];}
    /** Returns length between given nodes. */
    length_type length(index_type first, index_type last) const { return lengths[last]-lengths[first];}
    length_type length(index_type Idx) const { return lengths[Idx];}

    void set_length(index_type i, length_type length) { lengths[i] = length;}
    void clear();
};

}

#include "SplineImpl.h"

#endif // TRINITYSERVER_SPLINE_H
