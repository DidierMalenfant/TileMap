-- SPDX-FileCopyrightText: 2022-present Didier Malenfant <coding@malenfant.net>
--
-- SPDX-License-Identifier: MIT

import "CoreLibs/object"

dm = dm or {}

class('LuaTilemap', { }, dm).extends()

local LuaTilemap <const> = dm.LuaTilemap
local gfx <const> = playdate.graphics
local display_width <const>, display_height <const> = playdate.display.getSize()

function LuaTilemap:init()
    LuaTilemap.super.init(self)

    self.image_table = nil
    self.map = nil

    self.height = 0
    self.width = 0

    self.tile_width = 0
    self.tile_height = 0
end

-- Sets the tilemap’s playdate.graphics.imagetable to table, a playdate.graphics.imagetable.
function LuaTilemap:setImageTable(table)
    self.image_table = table

    local image <const> = self.image_table:getImage(1)
    self.tile_width = image.width
    self.tile_height = image.height
end

-- Sets the tilemap’s width to width, then populates the tilemap with data, which should be
-- a flat, one-dimensional array-like table containing index values to the tilemap’s imagetable.
function LuaTilemap:setTiles(data, width)
    self.map = data
    self.width = width
end

-- Returns data, width
-- data is a flat, one-dimensional array-like table containing index values to the tilemap’s imagetable.
-- width is the width of the tile map, in number of tiles.
function LuaTilemap:getTiles()
    return self.data, self.width
end

-- Draws the tile map at screen coordinate (x, y).
-- sourceRect, if specified, will cause only the part of the tilemap within sourceRect to be drawn.
function LuaTilemap:draw(x, y, _sourceRect)
    local image_width, image_height <const> = self:getTileSize()

    local draw_x = x
    local draw_y = y

    local tile_x = 0
    local tile_y = 0

    if draw_x < -16 then
        draw_x = -(-draw_x % image_width)
        tile_x = math.floor((draw_x - x) / image_width)
    end

    if tile_x >= self.width then
        return
    end

    if draw_y < -16 then
        draw_y = -(-draw_y % image_height)
        tile_y = math.floor((draw_y - y) / image_height)
    end

    if tile_y >= self.height then
        return
    end

    local current_tile_x = tile_x
    local current_tile_y = tile_y

    local current_draw_x = draw_x
    local current_draw_y = draw_y

    local width <const> = self.width
    local height <const> = self.height
    local map <const> = self.map
    local table <const> = self.image_table

    local tile_index = (current_tile_y * width) + current_tile_x
    while((current_draw_y < display_height) and (current_tile_y < height)) do
        local next_tile_index_offset = width

        while((current_draw_x < display_width) and (current_tile_x < width)) do
            local image_index = map[tile_index]
            if image_index ~= 0 then
                local image = table:getImage(image_index)
                if image ~= nil then
                    image:draw(current_draw_x, current_draw_y)
                end
            end

            next_tile_index_offset = next_tile_index_offset - 1
            tile_index = tile_index + 1

            current_draw_x = current_draw_x + image_width
            current_tile_x = current_tile_x + 1
        end

        tile_index = tile_index + next_tile_index_offset

        current_tile_x = tile_x
        current_tile_y = current_tile_y + 1

        current_draw_x = draw_x
        current_draw_y = current_draw_y + image_height
    end

    local debug = false
    if debug then
        local text = "xy: "..tostring(x)..' '..tostring(y)
        local text_width, text_height = gfx.getTextSize(text)
        gfx.fillRect(0, 30, text_width, text_height)
        gfx.setImageDrawMode("fillWhite")
        gfx.drawText(text, 0, 30)

        text = "draw: "..tostring(draw_x)..' '..tostring(draw_y)
        text_width, text_height = gfx.getTextSize(text)
        gfx.fillRect(0, 50, text_width, text_height)
        gfx.drawText(text, 0, 50)

        text = "tile: "..tostring(tile_x)..' '..tostring(tile_y)
        text_width, text_height = gfx.getTextSize(text)
        gfx.fillRect(0, 70, text_width, text_height)
        gfx.drawText(text, 0, 70)
    end
end

-- Sets the index of the tile at tilemap position (x, y). index is the (1-based) index of the image
-- in the tilemap’s playdate.graphics.imagetable.
function LuaTilemap:setTileAtPosition(x, y, index)
    self.map[(x - 1) + ((y - 1) * self.width)] = index
end

-- Returns the image index of the tile at the given x and y coordinate. If x or y is out of bounds, returns nil.
function LuaTilemap:getTileAtPosition(x, y)
    return self.map[x + ((y - 1) * self.width)]
end

-- Sets the tilemap’s width and height, in number of tiles.
function LuaTilemap:setSize(width, height)
    self.width = width
    self.height = height
    self.map = {}
end

-- Returns the size of the tile map, in tiles, as a pair, (width, height).
function LuaTilemap:getSize()
    return self.width, self.height
end

-- Returns the size of the tilemap in pixels; that is, the size of the image multiplied by the number of
-- rows and columns in the map. Returns multiple alues (width, height).
function LuaTilemap:getPixelSize()
    local image_width, image_height <const> = self:getTileSize()
    return self.width * image_width, self.height * image_height
end

-- Returns multiple values (width, height), the pixel width and height of an individual tile.
function LuaTilemap:getTileSize()
    return self.tile_width, self.tile_height
end
