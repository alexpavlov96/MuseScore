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

#include "io/file.h"

#include "engraving/tests/utils/scorerw.h"
#include "engraving/tests/utils/scorecomp.h"
#include "engraving/compat/midi/compatmidirender.h"

#include "engraving/dom/masterscore.h"
#include "engraving/dom/excerpt.h"

using namespace mu;
using namespace mu::engraving;

static const String GUITARPRO_DIR(u"data/guitarbendimporter/");
static const String SRC_GP_FILENAME = u"src";
static const String REF_MSCX_FILENAME = u"ref";
static const String MIDI_TXT_FILENME = u"midi";

namespace mu::iex::guitarpro {
extern Err importGTP(MasterScore*, muse::io::IODevice* io, const muse::modularity::ContextPtr& iocCtx, bool createLinkedTabForce = false,
                     bool experimental = false);
class GuitarBendImporter_Tests : public ::testing::Test
{
public:
    void gpReadTest(const String& folderName, const String& extension, bool checkMidi = false);
};

static EventsHolder renderMidiEvents(MasterScore* score);

EventsHolder renderMidiEvents(MasterScore* score)
{
    EventsHolder events;
    CompatMidiRendererInternal::Context ctx;

    CompatMidiRender::renderScore(score, events, ctx, true);

    return events;
}

EventsHolder midiEventsFromFile(const String& fileName)
{
    EventsHolder eventHolder;

    String fullName = String::fromUtf8(iex_guitarpro_tests_DATA_ROOT) + u"/" + GUITARPRO_DIR + fileName + u"-midi.txt";

    std::ifstream inputFile(fullName.toStdString());
    int currentChannel = 0;

    if (!inputFile.is_open()) {
        LOGE() << "@# Error: Could not open the file!";
        return eventHolder;
    }

    std::string line;

    while (std::getline(inputFile, line)) {
        std::stringstream ss(line);

        int number = 0;
        if (ss >> number) {
            if (ss.eof()) {
                currentChannel = number;
            } else {
                int tick = number;
                char eventType = 0;
                int dataA = 0;
                int dataB = 0;

                ss >> eventType;
                EventType evType = (eventType == 'p') ? EventType::ME_PITCHBEND : EventType::ME_NOTEON;

                if (evType == EventType::ME_PITCHBEND) {
                    int pitchBendVal = 0;
                    ss >> pitchBendVal;
                    dataA = pitchBendVal % 128;
                    dataB = pitchBendVal / 128;
                } else {
                    ss >> dataA >> dataB;
                }

                NPlayEvent event(evType, currentChannel, dataA, dataB);
                eventHolder[currentChannel].insert({tick, event});
            }
        }
    }

    inputFile.close();

    return eventHolder;
}

void GuitarBendImporter_Tests::gpReadTest(const String& folderName, const String& extension, bool checkMidi)
{
    String gpFileName = GUITARPRO_DIR + folderName + u"/" + SRC_GP_FILENAME + u"." + extension;
    String refFileName = GUITARPRO_DIR + folderName + u"/" + REF_MSCX_FILENAME + u"-" + extension + u".mscx";

    auto importFunc = [](MasterScore* score, const muse::io::path_t& path) -> Err {
        muse::io::File file(path);
        return importGTP(score, &file, muse::modularity::globalCtx());
    };

    MasterScore* score = ScoreRW::readScore(gpFileName, false, importFunc);
    EXPECT_TRUE(score);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"guitarbend-" + folderName + u"-" + extension + "u.mscx", refFileName));

    EventsHolder events;
    if (checkMidi) {
        events = renderMidiEvents(score);
    }

    delete score;

    LOGE() << "@# --- events from score ---";
    for (size_t i = 0; i < events.size(); ++i) {
        LOGE() << "@# chan = " << i;
        for (const auto& ev: events[i]) {
            if (ev.second.type() == ME_PITCHBEND) {
                LOGE() << "@# tick = " << ev.first << ", pb = " << ev.second.dataA() + ev.second.dataB() * 128;
            } else {
                LOGE() << "@# tick = " << ev.first << ", type = " << (int)ev.second.type() << ", a = " << ev.second.dataA() << ", b = " << ev.second.dataB();
            }
        }
    }

    if (!checkMidi) {
        return;
    }

    String midiTxtFileName = folderName + u"/" + MIDI_TXT_FILENME + u"-" + extension + u".txt";
    auto eventsFromFile = midiEventsFromFile(midiTxtFileName);

    EXPECT_TRUE(events.size() == eventsFromFile.size());

    // assuming that channels numbers grow from 0 upwards
    for (size_t i = 0; i < events.size(); i++) {
        EXPECT_TRUE(events[i] == eventsFromFile[i]);
    }
}

// TEST_F(GuitarBendImporter_Tests, gp1) {
//     LOGE() << "@# --- gp 1 ---";
//     gpReadTest(u"1", u"gp");
// }

// TEST_F(GuitarBendImporter_Tests, gp2) {
//     gpReadTest("2", "gp");
// }

// TEST_F(GuitarBendImporter_Tests, gp3) {
//     gpReadTest("3", "gp");
// }

// TEST_F(GuitarBendImporter_Tests, gp4) {
//     gpReadTest("4", "gp");
// }

TEST_F(GuitarBendImporter_Tests, gp5) {
    LOGE() << "@# --- gp 5 ---";
    gpReadTest(u"5", u"gp");
}

// TEST_F(GuitarBendImporter_Tests, gp6) {
//     gpReadTest("6", "gp");
// }

// TEST_F(GuitarBendImporter_Tests, gp7) {
//     gpReadTest("7", "gp");
// }

// TEST_F(GuitarBendImporter_Tests, gp5_1) {
//     gpReadTest("1", "gp5");
// }

// TEST_F(GuitarBendImporter_Tests, gp5_2) {
//     gpReadTest("2", "gp5");
// }

// TEST_F(GuitarBendImporter_Tests, gp5_3) {
//     gpReadTest("3", "gp5");
// }

// TEST_F(GuitarBendImporter_Tests, gp5_4) {
//     gpReadTest("4", "gp5");
// }

TEST_F(GuitarBendImporter_Tests, gp5_5) {
    LOGE() << "@# --- gp 5 (5) ---";
    gpReadTest(u"5", u"gp5");
}

// TEST_F(GuitarBendImporter_Tests, gp5_6) {
//     gpReadTest("6", "gp5");
// }

// TEST_F(GuitarBendImporter_Tests, gp5_7) {
//     gpReadTest("7", "gp5");
// }
}
