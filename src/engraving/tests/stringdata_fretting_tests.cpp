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

#include <gtest/gtest.h>

#include "engraving/dom/chord.h"
#include "engraving/dom/interval.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/segment.h"
#include "engraving/editing/transpose.h"

#include "mocks/engravingconfigurationmock.h"

#include "utils/scorerw.h"

#include "global/modularity/ioc.h"

using namespace mu::engraving;
using ECMock = ::testing::NiceMock<EngravingConfigurationMock>;

static const String DATA_DIR("stringdata_fretting_data/");

struct TabNote {
    int string = -1;
    int fret = -1;
    int pitch = -1;
};

static std::vector<TabNote> collectTabNotes(MasterScore* score)
{
    std::vector<TabNote> result;
    Measure* m = score->firstMeasure();
    if (!m) {
        return result;
    }
    for (Segment& seg : m->segments()) {
        if (!seg.isChordRestType()) {
            continue;
        }
        EngravingItem* el = seg.element(0);
        if (!el || !el->isChord()) {
            continue;
        }
        Chord* chord = toChord(el);
        for (Note* note : chord->notes()) {
            result.push_back({ note->string(), note->fret(), note->pitch() });
        }
    }
    return result;
}

static void transposeScore(MasterScore* score, int semitones)
{
    score->cmdSelectAll();
    score->startCmd(TranslatableString::untranslatable("fretting test"));

    const bool down = semitones < 0;
    const int absSemitones = std::abs(semitones);
    const int remainder = absSemitones % 12;
    int fullOctaves = absSemitones / 12;
    const TransposeDirection dir = down ? TransposeDirection::DOWN : TransposeDirection::UP;

    if (remainder != 0) {
        const int diatonic = Interval::chromatic2diatonic(down ? -remainder : remainder);
        const int absDiatonic = std::abs(diatonic);
        int intervalIdx = -1;
        for (int i = 0; i < static_cast<int>(Interval::allIntervals.size()); ++i) {
            if (Interval::allIntervals[i].diatonic == absDiatonic
                && Interval::allIntervals[i].chromatic == remainder) {
                intervalIdx = i;
                break;
            }
        }
        ASSERT_GE(intervalIdx, 0) << "no interval for " << remainder << " semitones";
        Transpose::transpose(score, TransposeMode::BY_INTERVAL, dir, Key::C,
                             intervalIdx, true, true, true);
    }

    while (fullOctaves-- > 0) {
        constexpr int perfectOctaveIdx = 25;
        Transpose::transpose(score, TransposeMode::BY_INTERVAL, dir, Key::C,
                             perfectOctaveIdx, true, true, true);
    }

    score->endCmd();
    score->doLayout();
}

class Engraving_StringDataFrettingTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        auto mock = muse::modularity::globalIoc()->resolve<IEngravingConfiguration>("utests");
        m_mock = dynamic_cast<ECMock*>(mock.get());
        ASSERT_NE(m_mock, nullptr);
        ON_CALL(*m_mock, preferSameStringForTranspose()).WillByDefault(::testing::Return(true));
        ON_CALL(*m_mock, negativeFretsAllowed()).WillByDefault(::testing::Return(true));
    }

    void TearDown() override
    {
        if (m_mock) {
            ON_CALL(*m_mock, preferSameStringForTranspose()).WillByDefault(::testing::Return(false));
            ON_CALL(*m_mock, negativeFretsAllowed()).WillByDefault(::testing::Return(false));
        }
    }

    ECMock* m_mock = nullptr;
};

// Transpose +1: notes on strings 2+3 (fret 5 each) should stay on their
// original strings with fret bumped to 6.
TEST_F(Engraving_StringDataFrettingTests, preferSameStringKeepsOriginalStrings)
{
    MasterScore* score = ScoreRW::readScore(DATA_DIR + "same_string.mscx");
    ASSERT_TRUE(score);

    transposeScore(score, 1);

    auto notes = collectTabNotes(score);
    ASSERT_EQ(notes.size(), 2u);

    // Notes come back highest-pitch first (chord->notes() order)
    EXPECT_EQ(notes[0].string, 2);
    EXPECT_EQ(notes[0].fret, 6);
    EXPECT_EQ(notes[1].string, 3);
    EXPECT_EQ(notes[1].fret, 6);

    delete score;
}

// Transpose -1: string 3 fret 9 → fret 8 (valid), string 5 fret 0 → fret -1.
// The valid-fret note on string 3 must NOT be displaced to another string.
TEST_F(Engraving_StringDataFrettingTests, validFretNotDisplacedByNegativeFretInChord)
{
    MasterScore* score = ScoreRW::readScore(DATA_DIR + "valid_fret_not_displaced.mscx");
    ASSERT_TRUE(score);

    transposeScore(score, -1);

    auto notes = collectTabNotes(score);
    ASSERT_EQ(notes.size(), 2u);

    // Find note on string 3 — it should stay at fret 8
    const TabNote* str3note = nullptr;
    const TabNote* str5note = nullptr;
    for (const auto& n : notes) {
        if (n.string == 3) {
            str3note = &n;
        }
        if (n.string == 5) {
            str5note = &n;
        }
    }

    ASSERT_NE(str3note, nullptr) << "note on string 3 should remain on string 3";
    EXPECT_EQ(str3note->fret, 8);

    ASSERT_NE(str5note, nullptr) << "note on string 5 should remain on string 5";
    EXPECT_EQ(str5note->fret, -1);

    delete score;
}
