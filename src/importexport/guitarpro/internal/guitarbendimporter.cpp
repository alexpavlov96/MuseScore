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
#include "guitarbendimporter.h"

#include "engraving/dom/chord.h"
#include "engraving/dom/guitarbend.h"
#include "engraving/dom/note.h"
#include "engraving/dom/score.h"
#include "engraving/dom/tie.h"
#include "log.h"

using namespace mu::engraving;

namespace mu::iex::guitarpro {
GuitarBendImporter::GuitarBendImporter(mu::engraving::Score* score) : m_score(score)
{
}

void GuitarBendImporter::importBendForNote(mu::engraving::Note* note, const mu::engraving::PitchValues& pitchValues)
{
    if (!pitchValues.empty()) {
        m_bendInfoForNote[note->track()][note->tick().ticks()] = fillBendInfo(note, pitchValues);
        m_chordsWithBends.insert(note->chord());
    }
}

// info about bends by notes - sorted according to tied notes
std::unordered_map<track_idx_t, std::map<int, std::vector<GuitarBendImporter::TiedImportedBendInfo>>> GuitarBendImporter::fillBendInfoByTiedNotes() const
{
    std::unordered_map<track_idx_t, std::map<int, std::vector<TiedImportedBendInfo>>> bendInfoByTiedNotes;
    std::unordered_set<Note*> visitedNotes; // TODO: заменить на тик+трак тоже, плюс разбить по нотам аккорд?

    for (const auto& [track, bendInfoForTrack] : m_bendInfoForNote) {
        for (const auto& [tick, bendInfo] : bendInfoForTrack) { // соблюдается порядок
            // const auto& bendSegments = bendInfo.segments;
            Note* note = bendInfo.note;

            std::vector<TiedImportedBendInfo> infoForTick; // TODO: name

            if (visitedNotes.find(note) != visitedNotes.end()) {
                LOGE() << "@# already visited";
                continue;
            }

            Tie* tie = nullptr;
            do {
                TiedImportedBendInfo tiedBendInfo;

                tiedBendInfo.tick = note->tick().ticks();

                if (m_bendInfoForNote.find(track) != m_bendInfoForNote.end() &&
                    m_bendInfoForNote.at(track).find(tiedBendInfo.tick) != m_bendInfoForNote.at(track).end()) {

                    tiedBendInfo.info = m_bendInfoForNote.at(track).at(tiedBendInfo.tick);
                } else {
                    LOGE() << "@# ! ... skipping info : no bends on note";
                }

                infoForTick.push_back(tiedBendInfo);
                visitedNotes.insert(note);
                tie = note->tieFor();
                if (tie) {
                    Note* tiedNote = tie->endNote();
                    if (!tiedNote) {
                        LOGE() << "@# !!! error";
                        break;
                    }

                    note = tiedNote;
                }
            } while (tie);

            // LOGE() << "@# visited notes size = " << visitedNotes.size();
            bendInfoByTiedNotes[track][tick] = infoForTick;
        }
    }

    return bendInfoByTiedNotes;
}

void GuitarBendImporter::prepareForImport()
{
    // разбиваем на 2 метода:
    // 1 - заполнить карту по залигованным нотам (вернуть её)
    auto bendInfoByTiedNotes = fillBendInfoByTiedNotes();
    // m_bendInfoForNote.clear(); // TODO: no usage should be later

    LOGE() << "@# --- bend info by tied notes ---";
    for (const auto& [track, trackInfo] : bendInfoByTiedNotes) {
        LOGE() << "@# track is " << track;
        for (const auto& [tick, tickInfo] : trackInfo) {

            LOGE() << "@# MAIN tick is " << tick << ", notes count = " << tickInfo.size();
            for (const auto& info : tickInfo) {
                LOGE() << "@# tick = " << info.tick << ", segments size = " << info.info.segments.size();
                size_t segmentsJumpsCount = 0;
                for (const auto& bs : info.info.segments) {
                    LOGE() << "@# time: " << bs.startTime << " ( -> " << bs.middleTime << " ) -> " << bs.endTime << ", pitch diff = " << bs.pitchDiff;
                    // всегда так или бывают кейсы холда?
                    if (bs.pitchDiff != 0) {
                        segmentsJumpsCount++;
                    }
                }

                LOGE() << "@# segment jumps = " << segmentsJumpsCount;
                fillBendData(info.info.note);
            }
        }
    }

    // 2 - добавить бенды в зависимости от карты

            // LOGE() << "@# --- preparing import for note " << note->pitch() << " on tick " << note->tick().ticks();
            // LOGE() << "@# //// lets' check bendSegments. size = " << bendSegments.size();
            // size_t segmentsJumpsCount = 0;
            // for (const auto& bs : bendSegments) {
            //     LOGE() << "@# time: " << bs.startTime << " ( -> " << bs.middleTime << " ) -> " << bs.endTime << ", pitch diff = " << bs.pitchDiff;
            //     // всегда так или бывают кейсы холда?
            //     if (bs.pitchDiff != 0) {
            //         segmentsJumpsCount++;
            //     }
            // }

            // find the duration of ties? find the first note?

            // сount the amount of tied notes
            // for each note check the pitchValues
            // see how amount of notes fits amount

            // size_t notesCount = note->tiedNotes().size();

            // LOGE() << "@# notes count = " << notesCount << ", segments = " << segmentsJumpsCount;
            // // merge bendSegments? - все данные помещать на 1 главную ноту ?

            // удалить лишние лиги - также не забыть про связанные стаффы

            // bool split = true;
            // if (notesCount > segmentsJumpsCount) {
            //     LOGE() << "@# no split";
            //     split = false;
            // }

            // int diff = notesCount - segmentsJumpsCount;
            // LOGE() << "@# diff = " << diff;
            // int curNoteNum = 0;
            // Note* curNote = note;

            // Tie* tie = curNote->tieFor();
            // while (tie && curNoteNum++ <= diff) {
            //     Note* tiedNote = tie->endNote();
            //     curNote->remove(tie);
            //     tie = tiedNote->tieFor();
            //     curNote = tiedNote;
            // }

            // fillBendData(note, split);
        // }
    // }

    // fillBendData(note);
}

void GuitarBendImporter::createGuitarBends(mu::engraving::Note* note)
{
    Chord* chord = toChord(note->parent());
    int chordTicks = chord->tick().ticks();
    LOGE() << "@# checking chord " << (int*)chord << "on tick " << chordTicks;

    // TODO: distinguish notes from same chord

    if (m_bendDataByEndTick.find(chord->track()) == m_bendDataByEndTick.end()) {
        LOGE() << "bend wasn't added: bends data on track " << chord->track() << " doesn't exist";
        return;
    }

    const auto& currentTrackData = m_bendDataByEndTick.at(chord->track());
    if (currentTrackData.find(chordTicks) == currentTrackData.end()) {
        LOGE() << "bend wasn't added: bends data on track " << chord->track() << " doesn't exist for tick " << chordTicks;
        return;
    }

    const BendData& bendData = currentTrackData.at(chordTicks);
    LOGE() << "@# should be bend! tones : " << bendData.quarterTones;

    if (bendData.type == GuitarBendType::PRE_BEND) {
        int pitch = bendData.quarterTones / 2;
        note->setPitch(note->pitch() + pitch);
        note->setTpcFromPitch();
        GuitarBend* bend = m_score->addGuitarBend(bendData.type, note);
        QuarterOffset quarterOff = bendData.quarterTones % 2 ? QuarterOffset::QUARTER_SHARP : QuarterOffset::NONE;
        bend->setEndNotePitch(note->pitch(), quarterOff);
        Note* startNote = bend->startNote();
        if (startNote) {
            startNote->setPitch(note->pitch() - pitch);
            startNote->setTpcFromPitch();
        }

        return;
    }

    if (bendData.type == GuitarBendType::SLIGHT_BEND) {
        GuitarBend* bend = m_score->addGuitarBend(bendData.type, note);
        bend->setStartTimeFactor(bendData.startFactor);
        return;
    }

    if (bendData.quarterTones == 0) {
        // TODO: пытаюсь не рисовать бенд между нотами в примере 2.gp
        // наверное в bendSegments лишние данные
        LOGE() << "@# skipping...";
        return;
    }

    Measure* startMeasure = m_score->tick2measure(bendData.tick);

    if (!startMeasure) {
        LOGE() << "@# ERROR 2";
        return;
    }

    Chord* startChord = startMeasure->findChord(bendData.tick, chord->track());

    if (!startChord) {
        LOGE() << "@# ERROR 3";
        return;
    }

    Note* startNote = startChord->upNote(); // TODO: разделять ноты
    int pitch = bendData.quarterTones / 2;

    GuitarBend* bend = m_score->addGuitarBend(bendData.type, startNote, note);
    if (!bend) {
        LOGE() << "bend wasn't created for track " << chord->track() << ", tick " << startChord->tick().ticks();
        return;
    }

    QuarterOffset quarterOff = bendData.quarterTones % 2 ? QuarterOffset::QUARTER_SHARP : QuarterOffset::NONE;
    bend->setEndNotePitch(startNote->pitch() + pitch, quarterOff);
    bend->setStartTimeFactor(bendData.startFactor);
}

static int findClosestNumerator(const Fraction& fraction, int requestedDenominator) {
    int closestNumerator = 0;
    double minDifference = std::numeric_limits<double>::max();
    int numerator = fraction.numerator();
    int denominator = fraction.denominator();

    for (int candidateNumerator = 1; candidateNumerator < requestedDenominator; candidateNumerator++) {
        double originalFraction = static_cast<double>(numerator) / denominator;
        double candidateFraction = static_cast<double>(candidateNumerator) / requestedDenominator;
        double difference = std::fabs(originalFraction - candidateFraction);

        if (difference < minDifference) {
            minDifference = difference;
            closestNumerator = candidateNumerator;
        }
    }

    return closestNumerator;
}

static Fraction ticksForBendSegment(int startTime, int endTime, const Fraction& totalDuration)
{
    constexpr int BEND_MAX_TIME = 60;
    constexpr int MAX_DENOMINATOR = 8;
    Fraction division = Fraction(endTime - startTime, BEND_MAX_TIME);
    int numerator = findClosestNumerator(totalDuration * division, MAX_DENOMINATOR);
    Fraction currentChordTicks = Fraction(numerator, MAX_DENOMINATOR);
    currentChordTicks = std::max(currentChordTicks, Fraction(1, MAX_DENOMINATOR));
    currentChordTicks = std::min(currentChordTicks, totalDuration - Fraction(1, MAX_DENOMINATOR));
    currentChordTicks.reduce();

    return currentChordTicks;
}

const std::unordered_set<mu::engraving::Chord*>& GuitarBendImporter::chordsWithBends() const
{
    return m_chordsWithBends;
}

GuitarBendType GuitarBendImporter::calculateBendType(const BendSegment& seg)
{
    if (seg.startTime == seg.endTime) {
        return GuitarBendType::PRE_BEND;
    } else if (seg.pitchDiff == 25) {
        return GuitarBendType::SLIGHT_BEND;
    }

    return GuitarBendType::BEND;
}

void GuitarBendImporter::fillBendData(const Note* note)
{
    // IF_ASSERT_FAILED(note) {
    if (!note) {
        LOGE() << "@# note is NULL";
        return;
    }

    if (m_bendInfoForNote.find(note->track()) == m_bendInfoForNote.end()) {
        LOGE() << "@# error 1: no bend segments filled for note " << note->pitch() << " on tick " << note->tick().ticks();
        return;
    }

    const auto& bendInfoForTrack = m_bendInfoForNote.at(note->track());
    if (bendInfoForTrack.find(note->tick().ticks()) == bendInfoForTrack.end()) {
        LOGE() << "@# error 2: no bend segments filled for note " << note->pitch() << " on tick " << note->tick().ticks();
        return;
    }

    const auto& bendInfo = bendInfoForTrack.at(note->tick().ticks());
    const auto& bendSegments = bendInfo.segments;

    Fraction noteTick = note->tick();
    Fraction totalDuration = note->chord()->ticks();

    // int pitchOffset = bendInfo.initialOffset;

    std::vector<Fraction> currentChordDurations;

    Fraction startTick = note->tick();

    // if there is only one "BEND" type, no need to split the duration
    int normalBendCount = std::count_if(bendSegments.begin(), bendSegments.end(), [this](const BendSegment& seg) {
        return calculateBendType(seg) == GuitarBendType::BEND;
    });

    LOGE() << "@# bend types:";
    for (size_t i = 0; i < bendSegments.size(); i++) {
        const auto& seg = bendSegments[i];
        BendData data;

        Fraction currentChordTicks;
        Fraction endNoteTick = startTick;

        data.type = calculateBendType(seg);
        data.tick = startTick;
        data.quarterTones = seg.pitchDiff / 25;

        if (data.type == GuitarBendType::BEND) {
            if (normalBendCount > 1) {
                currentChordTicks = ticksForBendSegment(seg.startTime, seg.endTime, totalDuration);
            } else {
                currentChordTicks = totalDuration;
            }

            endNoteTick = startTick + currentChordTicks;
            currentChordDurations.push_back(currentChordTicks);
            data.ticks = currentChordTicks;

            if (seg.middleTime != -1) {
                data.startFactor = (double)(seg.middleTime - seg.startTime) / (seg.endTime - seg.startTime);
                // LOGE() << "@# factor = " << data.startFactor;
            } else {
                // LOGE() << "@# skip factor";
            }

            startTick += currentChordTicks;
        } else {
            data.ticks = Fraction(0, 1);
        }

        m_bendDataByEndTick[note->track()][endNoteTick.ticks()] = std::move(data);
        LOGE() << "@# current chordticks = " << currentChordTicks.ticks();
        LOGE() << "@# storing data on tick : " << endNoteTick.ticks();
        LOGE() << "@# data.tick = " << data.tick.ticks() << ", ticks = " << data.ticks.ticks();

    }

    m_bendChordDurations[note->track()][noteTick.ticks()] = std::move(currentChordDurations);
}

std::vector<mu::engraving::Fraction> GuitarBendImporter::chordsDurations(mu::engraving::track_idx_t track, const mu::engraving::Fraction& tick) const
{
    if (m_bendChordDurations.find(track) == m_bendChordDurations.end()) {
        return {};
    }

    if (m_bendChordDurations.at(track).find(tick.ticks()) == m_bendChordDurations.at(track).end()) {
        return {};
    }

    return m_bendChordDurations.at(track).at(tick.ticks());
}

GuitarBendImporter::ImportedBendInfo GuitarBendImporter::fillBendInfo(Note* note, const PitchValues& pitchValues)
{
    PitchValues adaptedPitchValues;

    // здесь может подставить 0 0 - чтоб появился prebend segment
    if (pitchValues.front().time != 0) {
        PitchValue firstPv = pitchValues.front();
        firstPv.time = 0;
        LOGE() << "@# added adapted pitch in front";
        adaptedPitchValues.push_back(firstPv);
    }

    for (const auto& pv : pitchValues) {
        adaptedPitchValues.push_back(pv);
    }

    // зачем?
    if (pitchValues.back().time != 60) {
        PitchValue lastPv = pitchValues.back();
        lastPv.time = 60;
        adaptedPitchValues.push_back(lastPv);
        LOGE() << "@# added adapted pitch in back";
    }

    LOGE() << "@# generate chords";
    for (const auto& pv : adaptedPitchValues) {
        LOGE() << "@# time = " << pv.time << ", pitch = " << pv.pitch;
    }

    enum PitchDiff {
        NONE,
        SAME,
        UP,
        DOWN
    };

    std::vector<BendSegment> bendSegments;
    PitchDiff currentPitchDiff;
    PitchDiff previousPitchDiff = PitchDiff::NONE;

    auto pitchDiff = [](int prevPitch, int currentPitch) {
        if (prevPitch == currentPitch) {
            return PitchDiff::SAME;
        }

        return (prevPitch < currentPitch) ? PitchDiff::UP : PitchDiff::DOWN;
    };

    if (adaptedPitchValues.front().pitch != 0) {
        bendSegments.push_back(BendSegment());
        bendSegments.back().startTime = bendSegments.back().endTime = 0;
        bendSegments.back().pitchDiff = adaptedPitchValues.front().pitch;
    }

    for (size_t i = 0; i < adaptedPitchValues.size() - 1; i++) {
        currentPitchDiff = pitchDiff(adaptedPitchValues[i].pitch, adaptedPitchValues[i + 1].pitch);
        if (currentPitchDiff == previousPitchDiff) {
            if (!bendSegments.empty()) {
                bendSegments.back().endTime = adaptedPitchValues[i + 1].time;
                bendSegments.back().pitchDiff = adaptedPitchValues[i + 1].pitch - adaptedPitchValues[i].pitch;
            }

            continue;
        }

        if (currentPitchDiff == PitchDiff::SAME) {
            // кладем новый сегмент, когда прямая линия
            bendSegments.push_back(BendSegment());
            bendSegments.back().startTime = adaptedPitchValues[i].time;
            // на случай, если не будет вверх/вниз - заранее заполняем полное время
            bendSegments.back().endTime = adaptedPitchValues[i + 1].time;
            bendSegments.back().pitchDiff = adaptedPitchValues[i + 1].pitch - adaptedPitchValues[i].pitch;
        } else {
            // начинаем новый сегмент, если прямого отрезка не было
            if (previousPitchDiff != PitchDiff::SAME || bendSegments.empty()) {
                // скопировано с верхнего кода
                bendSegments.push_back(BendSegment());
                bendSegments.back().startTime = adaptedPitchValues[i].time;
                bendSegments.back().endTime = adaptedPitchValues[i + 1].time;
            } else {
                // иначе - корректируем middle
                bendSegments.back().middleTime = adaptedPitchValues[i].time;
                bendSegments.back().endTime = adaptedPitchValues[i + 1].time;
            }

            bendSegments.back().pitchDiff = adaptedPitchValues[i + 1].pitch - adaptedPitchValues[i].pitch;
        }

        previousPitchDiff = currentPitchDiff;
    }

    ImportedBendInfo info;
    info.segments = std::move(bendSegments);
    // info.initialOffset = adaptedPitchValues.front().pitch;
    info.note = note;

    return info;
}
} // namespace mu::iex::guitarpro
