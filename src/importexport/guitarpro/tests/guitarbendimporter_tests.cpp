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

#include "engraving/dom/masterscore.h"
#include "engraving/dom/excerpt.h"

using namespace mu;
using namespace mu::engraving;

static const String GUITARPRO_DIR(u"guitarbendimporter_data/");

namespace mu::iex::guitarpro {
extern Err importGTP(MasterScore*, muse::io::IODevice* io, const muse::modularity::ContextPtr& iocCtx, bool createLinkedTabForce = false,
                     bool experimental = false);
class GuitarBendImporter_Tests : public ::testing::Test
{
public:
    void gpReadTest(const String& folderName, const String& extension);
};

void GuitarBendImporter_Tests::gpReadTest(const String& fileName, const String& extension)
{
    String gpFileName = GUITARPRO_DIR + fileName + u"." + extension;
    String refFileName = GUITARPRO_DIR + fileName + u"-" + extension + u".mscx";

    auto importFunc = [](MasterScore* score, const muse::io::path_t& path) -> Err {
        muse::io::File file(path);
        return importGTP(score, &file, muse::modularity::globalCtx());
    };

    MasterScore* score = ScoreRW::readScore(gpFileName, false, importFunc);
    EXPECT_TRUE(score);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"guitarbend-" + fileName + u"-" + extension + u".mscx", refFileName));

    delete score;
}

TEST_F(GuitarBendImporter_Tests, gp1) {
    LOGE() << "@# --- gp 1 ---";
    gpReadTest(u"1", u"gp");
}
}
