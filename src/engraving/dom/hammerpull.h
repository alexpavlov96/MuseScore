/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __HAMMERPULL_H__
#define __HAMMERPULL_H__

#include "textbase.h"

namespace mu::engraving {
//---------------------------------------------------------
//   HammerPull
//---------------------------------------------------------

class HammerPull final : public TextBase
{
    OBJECT_ALLOCATOR(engraving, HammerPull)
    DECLARE_CLASSOF(ElementType::HAMMER_PULL)

public:

    enum class Technic {
        HammerOn = 0,
        PullOff
    };

    HammerPull(Segment* parent = 0, TextStyleType = TextStyleType::STAFF);

    HammerPull* clone() const override { return new HammerPull(*this); }

    void setSlur(Slur* slur) { m_slur = slur; }
    Slur* slur() const { return m_slur; }

    /// DEBUG
    void setNormalVector(PointF vec) { m_normalVector = vec; }
    PointF normalVector() const { return m_normalVector; }
    void setShoulderCoord(PointF vec) { m_shoulder = vec; }
    PointF shoulderCoord() const { return m_shoulder; }
    void setDistance(double dist) { m_distance = dist; }
    double distance() const { return m_distance; }
    ///

    void addTechnic(Technic technic);

private:
    enum class DisplayableTechnicName {
        None = -1,
        HammerOn,
        PullOff,
        Both
    } m_displayableTechnicName = DisplayableTechnicName::None;

    Slur* m_slur = nullptr;
    PointF m_normalVector;
    PointF m_shoulder;
    double m_distance;

    PropertyValue propertyDefault(Pid id) const override;
};
} // namespace mu::engraving
#endif
