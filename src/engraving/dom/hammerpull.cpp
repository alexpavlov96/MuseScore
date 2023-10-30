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

#include "hammerpull.h"

#include "segment.h"

using namespace mu;

namespace mu::engraving {

//---------------------------------------------------------
//   hammerPullStyle
//---------------------------------------------------------

static const ElementStyle hammerPullStyle {
    { Sid::staffTextPlacement, Pid::PLACEMENT },
    { Sid::staffTextMinDistance, Pid::MIN_DISTANCE },
};

//---------------------------------------------------------
//   HammerPull
//---------------------------------------------------------

HammerPull::HammerPull(Segment* parent, TextStyleType tid)
    : TextBase(ElementType::HAMMER_PULL, parent, tid, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    initElementStyle(&hammerPullStyle);
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

engraving::PropertyValue HammerPull::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::TEXT_STYLE:
        return TextStyleType::STAFF;
    default:
        return TextBase::propertyDefault(id);
    }
}

void HammerPull::addTechnic(Technic technic)
{
    switch (m_displayableTechnicName) {
    case DisplayableTechnicName::None:
        m_displayableTechnicName = (technic == Technic::HammerOn ? DisplayableTechnicName::HammerOn : DisplayableTechnicName::PullOff);
        break;
    case DisplayableTechnicName::HammerOn:
        if (technic == Technic::PullOff) {
            m_displayableTechnicName = DisplayableTechnicName::Both;
        }

        break;
    case DisplayableTechnicName::PullOff:
        if (technic == Technic::HammerOn) {
            m_displayableTechnicName = DisplayableTechnicName::Both;
        }

        break;
    default:
        break;
    }

    static std::unordered_map<DisplayableTechnicName, String> technicNames =
    {
        { DisplayableTechnicName::HammerOn, u"H"   },
        { DisplayableTechnicName::PullOff,  u"P"   },
        { DisplayableTechnicName::Both,     u"H/P" },
    };

    if (technicNames.find(m_displayableTechnicName) != technicNames.end()) {
        setPlainText(technicNames.at(m_displayableTechnicName));
    }
}
}
