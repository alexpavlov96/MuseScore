/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "global/io/file.h"
#include "modularity/ioc.h"

#include "engraving/iengravingfontsprovider.h"
#include "engraving/types/symid.h"

using namespace mu::engraving;

class Engraving_EngravingFontTests : public ::testing::Test
{
public:
    muse::GlobalInject<IEngravingFontsProvider> engravingFonts;
};

TEST_F(Engraving_EngravingFontTests, FontFromBytesMatchesFontFromFile)
{
    muse::ByteArray font;
    muse::ByteArray metadata;
    ASSERT_TRUE(muse::io::File::readFile(":/fonts/leland/Leland.otf", font));
    ASSERT_TRUE(muse::io::File::readFile(":/fonts/leland/metadata.json", metadata));

    IEngravingFontPtr fromFile = engravingFonts()->fontByName("Leland");
    ASSERT_TRUE(fromFile);

    engravingFonts()->addExternalFontFromData("LelandBytes", fromFile->family(), font, metadata);

    IEngravingFontPtr fromBytes = engravingFonts()->fontByName("LelandBytes");
    ASSERT_TRUE(fromBytes);

    EXPECT_EQ(fromBytes->name(), "LelandBytes");
    EXPECT_EQ(fromBytes->symCode(SymId::noteheadBlack), fromFile->symCode(SymId::noteheadBlack));
    EXPECT_EQ(fromBytes->bbox(SymId::noteheadBlack, 1.0), fromFile->bbox(SymId::noteheadBlack, 1.0));

    EXPECT_TRUE(fromBytes->engravingDefaults() == fromFile->engravingDefaults());
}
