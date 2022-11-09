// SPDX-FileCopyrightText: 2022-present Didier Malenfant <coding@malenfant.net>
//
// SPDX-License-Identifier: MIT

#include "Tilemap/OldCTilemap.h"

#define DM_LOG_ENABLE
#include "pdbase/pdbase.h"

// -- Forward declaration
static const lua_reg oldCTilemapClass[];

// -- Constants
#define CLASSNAME_OLDCTILEMAP "dm.OldCTilemap"

// -- tile_index class
typedef struct {
    LCDBitmapTable* image_table;

    int height;
    int width;

    int tile_width;
    int tile_height;

    int nb_of_tiles;
    LCDBitmap** tiles;
    
    uint16_t* map;
} OldCTilemap;

// -- Get an argument as a OldCTilemap class
#define GET_TILEMAP_ARG(index)    pd->lua->getArgObject(index, CLASSNAME_OLDCTILEMAP, NULL);

// -- Register the class
extern void register_OldCTilemap(PlaydateAPI* api)
{
    const char* err = NULL;
    
    // -- Register OldCTilemap
    if (!pd->lua->registerClass(CLASSNAME_OLDCTILEMAP, oldCTilemapClass, NULL, 0, &err))
    {
        DM_LOG("dm.OldCTilemap: Failed to register the OldCTilemap class (%s).", err);
        return;
    }
}

// -- Allocate a new tilemap
int oldCTilemapNew(lua_State* L)
{
    OldCTilemap* this = dmMemoryCalloc(1, sizeof(OldCTilemap));
    if (this == NULL) {
        return 0;
    }

    const char* path = pd->lua->getArgString(1);
    if (path == NULL) {
        DM_LOG("OldCTilemap: Error getting tilemap path argument.");
        return 0;
    }

    const char* err = NULL;
    this->image_table = pd->graphics->loadBitmapTable(path, &err);
    if (this->image_table == NULL) {
        DM_LOG("OldCTilemap: Error loading image table '%s' (%s).", path, (err != NULL) ? err : "Unknown Error");
        return 0;
    }

    this->nb_of_tiles = 0;
    for (; ;++this->nb_of_tiles) {
        LCDBitmap* bitmap = pd->graphics->getTableBitmap(this->image_table, this->nb_of_tiles);
        if (bitmap == NULL) {
            break;
        }
    }

    this->tiles = dmMemoryCalloc(this->nb_of_tiles, sizeof(LCDBitmap*));

    for (int index = 0; index < this->nb_of_tiles;++index) {
        this->tiles[index] = pd->graphics->getTableBitmap(this->image_table, index);
    }
    
    this->height = 0;
    this->width = 0;

    LCDBitmap* bitmap = pd->graphics->getTableBitmap(this->image_table, 1);
    if (bitmap == NULL) {
        DM_LOG("OldCTilemap: Error getting bitmap from image table '%s'.", path);
        return 0;
    }

    int width, height;
    pd->graphics->getBitmapData(bitmap, &width, &height, NULL, NULL, NULL);

    this->tile_height = width;
    this->tile_width = height;
    
    pd->graphics->freeBitmap(bitmap);

    this->map = NULL;

    pd->lua->pushObject(this, CLASSNAME_OLDCTILEMAP, 0);

    return 1;
}
 
// -- Delete the tilemap
int oldCTilemapDelete(lua_State* L)
{
    OldCTilemap* this = GET_TILEMAP_ARG(1);
    if(this == NULL) {
        DM_LOG("OldCTilemap: Error getting 'self' argument.");
        return 0;
    }
    
    if (this->image_table != NULL) {
        pd->graphics->freeBitmapTable(this->image_table);
        this->image_table = NULL;
    }
    
    if (this->tiles != NULL) {
        dmMemoryFree(this->tiles);
        this->image_table = NULL;
        this->nb_of_tiles = 0;
    }
    
    if (this->map != NULL) {
        dmMemoryFree(this->map);
        this->map = NULL;
    }
    
    dmMemoryFree(this);
    
    return 0;
}

// -- Draws the tile map at screen coordinate (x, y).
// -- sourceRect, if specified, will cause only the part of the tilemap within sourceRect to be drawn.
// function Tilemap:draw(x, y, _sourceRect)
int oldCTilemapDraw(lua_State* L)
{
    OldCTilemap* this = GET_TILEMAP_ARG(1);
    if(this == NULL) {
        DM_LOG("OldCTilemap: Error getting 'self' argument.");
        return 0;
    }

    if (this->map == NULL) {
        DM_LOG("OldCTilemap: Size of tilemap not set before draw().");
        return 0;
    }
    
    int image_width = this->tile_width;
    int image_height = this->tile_height;

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

    if (tile_x >= this->width) {
        return 0;
    }

    if (draw_y < -image_height) {
        draw_y = -(-draw_y % image_height);
        tile_y = (int)((draw_y - y) / image_height);
    }

    if (tile_y >= this->height) {
        return 0;
    }

    int current_tile_x = tile_x;
    int current_tile_y = tile_y;

    int current_draw_x = draw_x;
    int current_draw_y = draw_y;

    int width = this->width;
    int height = this->height;
    uint16_t* tilemap = this->map;
    LCDBitmapTable* table = this->image_table;

    int display_width = pd->display->getWidth();
    int display_height = pd->display->getHeight();

    //LCDBitmap* otherBitmap = pd->graphics->getTableBitmap(this->image_table, 1);

    int tilemap_index = (current_tile_y * width) + current_tile_x;
    while((current_draw_y < display_height) && (current_tile_y < height)) {
        int next_tilemap_index_offset = width;

        while((current_draw_x < display_width) && (current_tile_x < width)) {
            int tile_index = tilemap[tilemap_index] - 1;
            if (tile_index != 0) {
                if (tile_index >= this->nb_of_tiles) {
                    DM_LOG("OldCTilemap: Invalid tile index %d.", tile_index);
                    return 0;
                }
                
                LCDBitmap* bitmap = this->tiles[tile_index];
                if (bitmap != NULL) {
                    pd->graphics->drawBitmap(bitmap, current_draw_x, current_draw_y, kBitmapUnflipped);
                    //pd->graphics->drawBitmap(otherBitmap, current_draw_x, current_draw_y, kBitmapUnflipped);
                }
            }

            next_tilemap_index_offset -= 1;
            tilemap_index += 1;

            current_draw_x += image_width;
            current_tile_x += 1;
        }

        tilemap_index += next_tilemap_index_offset;

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
int oldCTilemapSetTileAtPosition(lua_State* L)
{
    OldCTilemap* this = GET_TILEMAP_ARG(1);
    if(this == NULL) {
        DM_LOG("OldCTilemap: Error getting 'self' argument.");
        return 0;
    }
    
    if (this->map == NULL) {
        DM_LOG("OldCTilemap: Size of tilemap not set before setTileAtPosition().");
        return 0;
    }
    
    int x = pd->lua->getArgInt(2);
    int y = pd->lua->getArgInt(3);
    
    if ((x < 1) || (x > this->width) || (y < 1) || (y > this->height)) {
        DM_LOG("OldCTilemap: Out of bounds values %d,%s for getTileAtPosition.", x, y);
        return 0;
    }

    int tilemap_index = pd->lua->getArgInt(4);
    if (tilemap_index < 0) {
        DM_LOG("OldCTilemap: Out of bounds tile index %d for getTileAtPosition.", tilemap_index);
        return 0;
    }
    
    this->map[((y - 1) * this->width) + (x - 1)] = tilemap_index;

    return 0;
}

// -- Returns the image index of the tile at the given x and y coordinate. If x or y is out of bounds, returns nil.
// function Tilemap:getTileAtPosition(x, y)
int oldCTilemapGetTileAtPosition(lua_State* L)
{
    OldCTilemap* this = GET_TILEMAP_ARG(1);
    if(this == NULL) {
        DM_LOG("OldCTilemap: Error getting 'self' argument.");
        return 0;
    }

    if (this->map == NULL) {
        DM_LOG("OldCTilemap: Size of tilemap not set before getTileAtPosition().");
        return 0;
    }

    int x = pd->lua->getArgInt(2);
    int y = pd->lua->getArgInt(3);

    if ((x < 1) || (x > this->width) || (y < 1) || (y > this->height)) {
        DM_LOG("OldCTilemap: Out of bounds values %d,%s for getTileAtPosition.", x, y);
        return 0;
    }

    pd->lua->pushInt(this->map[((y - 1) * this->width) + (x - 1)]);

    return 1;
}

// -- Sets the tilemap’s width and height, in number of tiles.
// function Tilemap:setSize(width, height)
int oldCTilemapSetSize(lua_State* L)
{
    OldCTilemap* this = GET_TILEMAP_ARG(1);
    if(this == NULL) {
        DM_LOG("OldCTilemap: Error getting 'self' argument.");
        return 0;
    }

    if (this->map != NULL) {
        dmMemoryFree(this->map);
        this->map = NULL;
    }
    
    this->width = pd->lua->getArgInt(2);
    this->height = pd->lua->getArgInt(3);

    if ((this->width == 0) || (this->width > 2048) || (this->height == 0) || (this->height > 2048)) {
        DM_LOG("OldCTilemap: Trying to set an invalid size of %dx%d.", this->width, this->height);
        return 0;
    }

    this->map = dmMemoryCalloc(this->width * this->height, sizeof(uint16_t));

    return 0;
}

// -- Returns the size of the tile map, in tiles, as a pair, (width, height).
// function Tilemap:getSize()
int oldCTilemapGetSize(lua_State* L)
{
    OldCTilemap* this = GET_TILEMAP_ARG(1);
    if(this == NULL) {
        DM_LOG("OldCTilemap: Error getting 'self' argument.");
        return 0;
    }
    
    pd->lua->pushInt(this->width);
    pd->lua->pushInt(this->height);
    
    return 2;
}

// -- Returns the size of the tilemap in pixels; that is, the size of the image multiplied by the number of
// -- rows and columns in the map. Returns multiple values (width, height).
// function Tilemap:getPixelSize()
int oldCTilemapGetPixelSize(lua_State* L)
{
    OldCTilemap* this = GET_TILEMAP_ARG(1);
    if(this == NULL) {
        DM_LOG("OldCTilemap: Error getting 'self' argument.");
        return 0;
    }

    pd->lua->pushInt(this->width * this->tile_width);
    pd->lua->pushInt(this->height * this->tile_height);

    return 2;
}

// -- Returns multiple values (width, height), the pixel width and height of an individual tile.
// function Tilemap:getTileSize()
int oldCTilemapGetTileSize(lua_State* L)
{
    OldCTilemap* this = GET_TILEMAP_ARG(1);
    if(this == NULL) {
        DM_LOG("OldCTilemap: Error getting 'self' argument.");
        return 0;
    }
    
    pd->lua->pushInt(this->tile_width);
    pd->lua->pushInt(this->tile_height);
    
    return 2;
}

static const lua_reg oldCTilemapClass[] = {
    { "new", oldCTilemapNew },
    { "__gc", oldCTilemapDelete },
    
    { "draw", oldCTilemapDraw },
    { "setTileAtPosition", oldCTilemapSetTileAtPosition },
    { "getTileAtPosition", oldCTilemapGetTileAtPosition },
    { "setSize", oldCTilemapSetSize },
    { "getSize", oldCTilemapGetSize },
    { "getPixelSize", oldCTilemapGetPixelSize },
    { "getTileSize", oldCTilemapGetTileSize },
    
    { NULL, NULL }
};
