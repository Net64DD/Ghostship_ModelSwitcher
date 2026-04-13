#include "mod.h"

#include <stdio.h>
#include <stdlib.h>

#include "sm64.h"
#include "library.h"
#include "game/game_init.h"
#include "port/api/ui.h"
#include "actors/models.h"
#include "game/mario_misc.h"
#include "game/level_update.h"
#include "port/events/Events.h"

ListenerID gFrameUpdateListenerID;
ListenerID gRenderGamePostListenerID;
static ModelID gModelMapping[0x512];
static C_ComboboxOption gModelOptions[0x512];

void SetupModels() {
    CallUtil(RegisterModel, "Isabelle", isabelle_geo, MODEL_GEO_LAYOUT, MODEL_PUBLIC);
    CallUtil(RegisterModel, "Hat Kid", hat_kid_geo, MODEL_GEO_LAYOUT, MODEL_PUBLIC);
    CallUtil(RegisterModel, "Mario R96", mario_r96_geo, MODEL_GEO_LAYOUT, MODEL_PUBLIC);
    CallUtil(RegisterModel, "Samus", samus_geo, MODEL_GEO_LAYOUT, MODEL_PUBLIC);
}

void SetupUI() {
    uint32_t idx = 0;
    uint32_t count = CallUtil(GetModelCount);
    LoadedModel* models[count + 1];
    CallUtil(GetAllModels, models);

    C_WidgetConfig chk = {0};
    chk.type = C_WIDGET_CVAR_COMBOBOX;
    chk.cvar = "gMoreModels.SelectedModel";
    chk.opts.combo.default_index = 0;
    chk.opts.combo.map = gModelOptions;

    gModelOptions[0] = (C_ComboboxOption){ .id = idx++, .value = "None" };
    for (uint32_t i = 0; i < count; i++) {
        if (models[i]->visibility != MODEL_PUBLIC) {
            continue;
        }
        gModelOptions[idx] = (C_ComboboxOption){ .id = idx, .value = models[i]->name };
        gModelMapping[idx] = models[i]->id;
        idx++;
    }
    gModelOptions[idx] = (C_ComboboxOption){ .id = NULL, .value = NULL };

    C_AddSidebarEntry("More Models", 1);
    C_AddWidget("More Models", 1, "Selected Model", &chk);
}

void OnFrameUpdate(IEvent* event) {
    uint32_t selected = CVarGetInteger("gMoreModels.SelectedModel", 0);
    gMarioObject->header.gfx.sharedChild = gLoadedGraphNodes[selected > 0 ? gModelMapping[selected] : MODEL_MARIO];
}

void OnGameRenderHud(IEvent* event) {
    if (CVarGetInteger("gSkipIntro", 0) == 0) {
        return;
    }
}

MOD_INIT() {
    LUSLOG_INFO("Initializing More Models Mod");
    SetupModels();
    LUSLOG_INFO("Registered models");
    SetupUI();
    LUSLOG_INFO("Setup UI");
    gFrameUpdateListenerID = REGISTER_LISTENER(GameFrameUpdate, EVENT_PRIORITY_NORMAL, OnFrameUpdate);
    gRenderGamePostListenerID = REGISTER_LISTENER(RenderGamePost, EVENT_PRIORITY_NORMAL, OnGameRenderHud);
    LUSLOG_INFO("Registered event listeners");
}

MOD_EXIT() {
    C_RemoveSidebarEntry("More Models");
    UNREGISTER_LISTENER(GameFrameUpdate, gFrameUpdateListenerID);
    UNREGISTER_LISTENER(RenderGamePost, gRenderGamePostListenerID);
}