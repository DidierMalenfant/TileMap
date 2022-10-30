// SPDX-FileCopyrightText: 2022-present Didier Malenfant <coding@malenfant.net>
//
// SPDX-License-Identifier: MIT

#include "Tilemap/Tilemap.h"

#define DM_LOG_ENABLE
#include "pdbase/pdbase.h"

// -- Forward declaration
static const lua_reg tilemapClass[];

// -- Constants
#define CLASSNAME_TILEMAP "dm.Tilemap"

// -- Tilemap class
typedef struct {
    LCDBitmapTable* image_table;

    int height;
    int width;

    int tile_width;
    int tile_height;
    
    uint16_t* map;
} Tilemap;

// -- Get an argument as a Tilemap class
#define GET_TILEMAP_ARG(index)    pd->lua->getArgObject(index, CLASSNAME_TILEMAP, NULL);

// -- Register the class
extern void register_Tilemap(PlaydateAPI* api)
{
    const char* err = NULL;
    
    // -- Register Tilemap
    if (!pd->lua->registerClass(CLASSNAME_TILEMAP, tilemapClass, NULL, 0, &err))
    {
        DM_LOG("dm.Tilemap: Failed to register the Tilemap class (%s).", err);
        return;
    }
}

// -- Allocate a new tilemap
int tilemapNew(lua_State* L)
{
    Tilemap* map = dmMemoryCalloc(1, sizeof(Tilemap));
    if (map == NULL) {
        return 0;
    }

    const char* path = pd->lua->getArgString(1);
    if (path == NULL) {
        DM_LOG("Tilemap: Error getting tilemap path argument.");
        return 0;
    }

    const char* err = NULL;
    map->image_table = pd->graphics->loadBitmapTable(path, &err);
    if (map->image_table == NULL) {
        DM_LOG("Tilemap: Error loading image table '%s' (%s).", path, (err != NULL) ? err : "Unknown Error");
        return 0;
    }

    map->height = 0;
    map->width = 0;

    LCDBitmap* bitmap = pd->graphics->getTableBitmap(map->image_table, 1);
    if (bitmap == NULL) {
        DM_LOG("Tilemap: Error getting bitmap from image table '%s'.", path);
        return 0;
    }

    int width, height;
    pd->graphics->getBitmapData(bitmap, &width, &height, NULL, NULL, NULL);

    map->tile_height = width;
    map->tile_width = height;
    
    pd->graphics->freeBitmap(bitmap);

    map->map = NULL;

    pd->lua->pushObject(map, CLASSNAME_TILEMAP, 0);

    return 1;
}
 
// -- Delete the tilemap
int tilemapDelete(lua_State* L)
{
    Tilemap* map = GET_TILEMAP_ARG(1);
    if(map == NULL) {
        DM_LOG("Tilemap: Error getting 'self' argument.");
        return 0;
    }
    
    if (map->image_table != NULL) {
        pd->graphics->freeBitmapTable(map->image_table);
        map->image_table = NULL;
    }
    
    if (map->map != NULL) {
        dmMemoryFree(map->map);
        map->map = NULL;
    }
    
    dmMemoryFree(map);
    
    return 0;
}

// -- Draws the tile map at screen coordinate (x, y).
// -- sourceRect, if specified, will cause only the part of the tilemap within sourceRect to be drawn.
// function Tilemap:draw(x, y, _sourceRect)
int tilemapDraw(lua_State* L)
{
    Tilemap* map = GET_TILEMAP_ARG(1);
    if(map == NULL) {
        DM_LOG("Tilemap: Error getting 'self' argument.");
        return 0;
    }

    if (map->map == NULL) {
        DM_LOG("Tilemap: Size of tilemap not set before draw().");
        return 0;
    }
    
    int image_width = map->tile_width;
    int image_height = map->tile_height;

    int x = pd->lua->getArgInt(2);
    int y = pd->lua->getArgInt(3);

    int draw_x = x;
    int draw_y = y;

    int tile_x = 0;
    int tile_y = 0;

    if (draw_x < -image_width) {
        draw_x = -(-draw_x % image_width);
        tile_x = (int)((draw_x - x) / image_width);
    }

    if (tile_x >= map->width) {
        return 0;
    }

    if (draw_y < -image_height) {
        draw_y = -(-draw_y % image_height);
        tile_y = (int)((draw_y - y) / image_height);
    }

    if (tile_y >= map->height) {
        return 0;
    }

    int current_tile_x = tile_x;
    int current_tile_y = tile_y;

    int current_draw_x = draw_x;
    int current_draw_y = draw_y;

    int width = map->width;
    int height = map->height;
    uint16_t* tilemap = map->map;
    LCDBitmapTable* table = map->image_table;

    int display_width = pd->display->getWidth();
    int display_height = pd->display->getHeight();

    int tile_index = (current_tile_y * width) + current_tile_x;
    while((current_draw_y < display_height) && (current_tile_y < height)) {
        int next_tile_index_offset = width;

        while((current_draw_x < display_width) && (current_tile_x < width)) {
            int image_index = tilemap[tile_index];
            if (image_index != 0) {
                LCDBitmap* bitmap = pd->graphics->getTableBitmap(map->image_table, image_index);
                if (bitmap != NULL) {
                    pd->graphics->drawBitmap(bitmap, current_draw_x, current_draw_y, kBitmapUnflipped);
                }
            }

            next_tile_index_offset -= 1;
            tile_index += 1;

            current_draw_x += image_width;
            current_tile_x += 1;
        }

        tile_index += next_tile_index_offset;

        current_tile_x = tile_x;
        current_tile_y += 1;

        current_draw_x = draw_x;
        current_draw_y += image_height;
    }

    return 0;
}

// -- Sets the index of the tile at tilemap position (x, y). index is the (1-based) index of the image
// -- in the tilemap’s playdate.graphics.imagetable.
// function Tilemap:setTileAtPosition(x, y, index)
int tilemapSetTileAtPosition(lua_State* L)
{
    Tilemap* map = GET_TILEMAP_ARG(1);
    if(map == NULL) {
        DM_LOG("Tilemap: Error getting 'self' argument.");
        return 0;
    }
    
    if (map->map == NULL) {
        DM_LOG("Tilemap: Size of tilemap not set before setTileAtPosition().");
        return 0;
    }
    
    int x = pd->lua->getArgInt(2);
    int y = pd->lua->getArgInt(3);
    
    if ((x < 1) || (x > map->width) || (y < 1) || (y > map->height)) {
        DM_LOG("Tilemap: Out of bounds values %d,%s for getTileAtPosition.", x, y);
        return 0;
    }

    int tile_index = pd->lua->getArgInt(4);
    if (tile_index < 0) {
        DM_LOG("Tilemap: Out of bounds tile index %d for getTileAtPosition.", tile_index);
        return 0;
    }
    
    map->map[((y - 1) * map->width) + (x - 1)] = tile_index;

    return 0;
}

// -- Returns the image index of the tile at the given x and y coordinate. If x or y is out of bounds, returns nil.
// function Tilemap:getTileAtPosition(x, y)
int tilemapGetTileAtPosition(lua_State* L)
{
    Tilemap* map = GET_TILEMAP_ARG(1);
    if(map == NULL) {
        DM_LOG("Tilemap: Error getting 'self' argument.");
        return 0;
    }

    if (map->map == NULL) {
        DM_LOG("Tilemap: Size of tilemap not set before getTileAtPosition().");
        return 0;
    }

    int x = pd->lua->getArgInt(2);
    int y = pd->lua->getArgInt(3);

    if ((x < 1) || (x > map->width) || (y < 1) || (y > map->height)) {
        DM_LOG("Tilemap: Out of bounds values %d,%s for getTileAtPosition.", x, y);
        return 0;
    }

    pd->lua->pushInt(map->map[((y - 1) * map->width) + (x - 1)]);

    return 1;
}

// -- Sets the tilemap’s width and height, in number of tiles.
// function Tilemap:setSize(width, height)
int tilemapSetSize(lua_State* L)
{
    Tilemap* map = GET_TILEMAP_ARG(1);
    if(map == NULL) {
        DM_LOG("Tilemap: Error getting 'self' argument.");
        return 0;
    }

    if (map->map != NULL) {
        dmMemoryFree(map->map);
        map->map = NULL;
    }
    
    map->width = pd->lua->getArgInt(2);
    map->height = pd->lua->getArgInt(3);

    if ((map->width == 0) || (map->width > 2048) || (map->height == 0) || (map->height > 2048)) {
        DM_LOG("Tilemap: Trying to set an invalid size of %dx%d.", map->width, map->height);
        return 0;
    }

    map->map = dmMemoryCalloc(map->width * map->height, sizeof(uint16_t));

    return 0;
}

// -- Returns the size of the tile map, in tiles, as a pair, (width, height).
// function Tilemap:getSize()
int tilemapGetSize(lua_State* L)
{
    Tilemap* map = GET_TILEMAP_ARG(1);
    if(map == NULL) {
        DM_LOG("Tilemap: Error getting 'self' argument.");
        return 0;
    }
    
    pd->lua->pushInt(map->width);
    pd->lua->pushInt(map->height);
    
    return 2;
}

// -- Returns the size of the tilemap in pixels; that is, the size of the image multiplied by the number of
// -- rows and columns in the map. Returns multiple values (width, height).
// function Tilemap:getPixelSize()
int tilemapGetPixelSize(lua_State* L)
{
    Tilemap* map = GET_TILEMAP_ARG(1);
    if(map == NULL) {
        DM_LOG("Tilemap: Error getting 'self' argument.");
        return 0;
    }

    pd->lua->pushInt(map->width * map->tile_width);
    pd->lua->pushInt(map->height * map->tile_height);

    return 2;
}

// -- Returns multiple values (width, height), the pixel width and height of an individual tile.
// function Tilemap:getTileSize()
int tilemapGetTileSize(lua_State* L)
{
    Tilemap* map = GET_TILEMAP_ARG(1);
    if(map == NULL) {
        DM_LOG("Tilemap: Error getting 'self' argument.");
        return 0;
    }
    
    pd->lua->pushInt(map->tile_width);
    pd->lua->pushInt(map->tile_height);
    
    return 2;
}

static const lua_reg tilemapClass[] = {
    { "new", tilemapNew },
    { "__gc", tilemapDelete },
    
    { "draw", tilemapDraw },
    { "setTileAtPosition", tilemapSetTileAtPosition },
    { "getTileAtPosition", tilemapGetTileAtPosition },
    { "setSize", tilemapSetSize },
    { "getSize", tilemapGetSize },
    { "getPixelSize", tilemapGetPixelSize },
    { "getTileSize", tilemapGetTileSize },
    
    { NULL, NULL }
};
