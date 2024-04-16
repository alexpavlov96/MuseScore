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
#include "updatemodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"

#include "framework/ui/iinteractiveuriregister.h"
#include "framework/ui/iuiengine.h"
#include "framework/ui/iuiactionsregister.h"

#include "internal/updateconfiguration.h"
#include "internal/updatescenario.h"
#include "internal/updateactioncontroller.h"
#include "internal/updateuiactions.h"
#include "internal/appupdateservice.h"

#include "internal/musesoundscheckupdatescenario.h"
#include "internal/musesoundscheckupdateservice.h"

#include "view/updatemodel.h"

#include "diagnostics/idiagnosticspathsregister.h"

using namespace mu::update;
using namespace mu::modularity;
using namespace mu::ui;

static void update_init_qrc()
{
    Q_INIT_RESOURCE(update);
}

std::string UpdateModule::moduleName() const
{
    return "update";
}

void UpdateModule::registerExports()
{
    m_scenario = std::make_shared<UpdateScenario>();
    m_configuration = std::make_shared<UpdateConfiguration>();
    m_actionController = std::make_shared<UpdateActionController>();
    m_appUpdateService = std::make_shared<AppUpdateService>();

    m_museSoundsCheckUpdateScenario = std::make_shared<MuseSoundsCheckUpdateScenario>();
    m_museSamplerUpdateService = std::make_shared<MuseSoundsCheckUpdateService>();

    ioc()->registerExport<IUpdateScenario>(moduleName(), m_scenario);
    ioc()->registerExport<IUpdateConfiguration>(moduleName(), m_configuration);
    ioc()->registerExport<IAppUpdateService>(moduleName(), m_appUpdateService);

    ioc()->registerExport<IMuseSoundsCheckUpdateScenario>(moduleName(), m_museSoundsCheckUpdateScenario);
    ioc()->registerExport<IMuseSoundsCheckUpdateService>(moduleName(), m_museSamplerUpdateService);
}

void UpdateModule::resolveImports()
{
    auto ar = ioc()->resolve<IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<UpdateUiActions>(m_actionController));
    }

    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerQmlUri(Uri("musescore://update/releaseinfo"), "MuseScore/Update/AppReleaseInfoDialog.qml");
        ir->registerQmlUri(Uri("musescore://update"), "MuseScore/Update/UpdateProgressDialog.qml");
        ir->registerQmlUri(Uri("musescore://update/musesoundsreleaseinfo"), "MuseScore/Update/MuseSoundsReleaseInfoDialog.qml");
    }
}

void UpdateModule::registerResources()
{
    update_init_qrc();
}

void UpdateModule::registerUiTypes()
{
    qmlRegisterType<UpdateModel>("MuseScore.Update", 1, 0, "UpdateModel");

    ioc()->resolve<IUiEngine>(moduleName())->addSourceImportPath(update_QML_IMPORT);
}

void UpdateModule::onInit(const framework::IApplication::RunMode& mode)
{
    if (mode != framework::IApplication::RunMode::GuiApp) {
        return;
    }

    m_configuration->init();
    m_actionController->init();
}

void UpdateModule::onDelayedInit()
{
    m_scenario->delayedInit();
    m_museSoundsCheckUpdateScenario->delayedInit();
}
