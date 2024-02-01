/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#ifndef MU_IMPORTEXPORT_GUITARBENDIMPORTER_H
#define MU_IMPORTEXPORT_GUITARBENDIMPORTER_H

#include <engraving/types/fraction.h>
#include <engraving/types/pitchvalue.h>
#include <engraving/dom/types.h>

namespace mu::engraving {
class Note;
class Chord;
class Score;
}

namespace mu::iex::guitarpro {
class GuitarBendImporter
{
public:

    GuitarBendImporter(mu::engraving::Score* score);
    std::vector<mu::engraving::Fraction> chordsDurations(mu::engraving::track_idx_t track, const mu::engraving::Fraction& tick) const;
    void importBendForNote(mu::engraving::Note* note, const mu::engraving::PitchValues& pitchValues);
    void prepareForImport();
    void createGuitarBends(mu::engraving::Note* note);
    const std::unordered_set<mu::engraving::Chord*>& chordsWithBends() const;

private:

    struct BendSegment {
        int startTime = -1;
        int middleTime = -1;
        int endTime = -1;
        int pitchDiff = -1;
    };

    struct BendData {
        mu::engraving::Fraction tick; // начальная позиция
        mu::engraving::Fraction ticks; // длина
        double startFactor = 0.0;
        int quarterTones = 0;
        mu::engraving::GuitarBendType type = mu::engraving::GuitarBendType::BEND;
    };

    struct ImportedBendInfo {
        int initialOffset = 0;
        mu::engraving::Note* note = nullptr;
        std::vector<BendSegment> segments;
    };

    struct TiedImportedBendInfo {
        int tick;
        ImportedBendInfo info;
    };

    std::unordered_map<mu::engraving::track_idx_t, std::map<int, std::vector<TiedImportedBendInfo>>> fillBendInfoByTiedNotes() const;

    void fillBendData(const mu::engraving::Note* note);
    ImportedBendInfo fillBendInfo(mu::engraving::Note* note, const mu::engraving::PitchValues& pitchValues);
    mu::engraving::GuitarBendType calculateBendType(const BendSegment& seg);

    // todo: here storing 1 note for "track+tick". TODO: adapt for chord
    std::unordered_map<mu::engraving::track_idx_t, std::map<int, ImportedBendInfo>> m_bendInfoForNote;
    std::unordered_set<mu::engraving::Chord*> m_chordsWithBends;
    std::unordered_map<mu::engraving::track_idx_t, std::unordered_map<int, std::vector<mu::engraving::Fraction>>> m_bendChordDurations;
    std::unordered_map<mu::engraving::track_idx_t, std::unordered_map<int, BendData>> m_bendDataByEndTick;

    mu::engraving::Score* m_score = nullptr;
};
} // mu::iex::guitarpro
#endif // MU_IMPORTEXPORT_GUITARBENDIMPORTER_H
