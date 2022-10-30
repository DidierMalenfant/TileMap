// SPDX-FileCopyrightText: 2022-present Didier Malenfant <coding@malenfant.net>
//
// SPDX-License-Identifier: MIT

#include "Tilemap/Tilemap.h"

int eventHandler(PlaydateAPI* playdate, PDSystemEvent event, uint32_t arg) {
    if(event == kEventInitLua) {
        register_Tilemap(playdate);
    }
    
    return 0;
}
