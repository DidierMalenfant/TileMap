-- SPDX-FileCopyrightText: 2022-present Didier Malenfant <coding@malenfant.net>
--
-- SPDX-License-Identifier: MIT

import "CoreLibs/object"

local gfx <const> = playdate.graphics
local display_width <const>, display_height <const> = playdate.display.getSize()

dm = dm or {}

class('Tilemap', { }, dm).extends()

function dm.Tilemap:init()
    dm.Tilemap.super.init(self)

    self.image_table = nil
    self.map = nil
    self.width = 0
end

-- Sets the tilemap’s playdate.graphics.imagetable to table, a playdate.graphics.imagetable.
function dm.Tilemap:setImageTable(table)
    self.image_table = table
end

-- Sets the tilemap’s width to width, then populates the tilemap with data, which should be
-- a flat, one-dimensional array-like table containing index values to the tilemap’s imagetable.
function dm.Tilemap:setTiles(data, width)
    self.map = data
    self.width = width
end

-- Returns data, width
-- data is a flat, one-dimensional array-like table containing index values to the tilemap’s imagetable.
-- width is the width of the tile map, in number of tiles.
function dm.Tilemap:getTiles()
    return self.data, self.width
end

-- Draws the tile map at screen coordinate (x, y).
-- sourceRect, if specified, will cause only the part of the tilemap within sourceRect to be drawn.
function dm.Tilemap:draw(x, y, _sourceRect)
    local image_width, image_height = self:getTileSize()

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
    local current_draw_x = draw_x

    local current_tile_y = tile_y
    local current_draw_y = draw_y

    while((current_draw_y < display_height) and (current_tile_y < self.height)) do
        while((current_draw_x < display_width) and (current_tile_x < self.width)) do
            local tile_index = self.map[(current_tile_y * self.width) + current_tile_x]
            if tile_index ~= 0 then
                local image = self.image_table:getImage(tile_index)
                if image ~= nil then
                    image:draw(current_draw_x, current_draw_y)
                end
            end

            current_draw_x = current_draw_x + image_width
            current_tile_x = current_tile_x + 1
        end

        current_draw_y = current_draw_y + image_height
        current_tile_y = current_tile_y + 1

        current_tile_x = tile_x
        current_draw_x = draw_x
    end

    local debug = false
    if debug then
        local text = "xy: "..tostring(x)..' '..tostring(y)
        local width, height = gfx.getTextSize(text)
        gfx.fillRect(0, 30, width, height)
        gfx.setImageDrawMode("fillWhite")
        gfx.drawText(text, 0, 30)

        text = "draw: "..tostring(draw_x)..' '..tostring(draw_y)
        width, height = gfx.getTextSize(text)
        gfx.fillRect(0, 50, width, height)
        gfx.drawText(text, 0, 50)

        text = "tile: "..tostring(tile_x)..' '..tostring(tile_y)
        width, height = gfx.getTextSize(text)
        gfx.fillRect(0, 70, width, height)
        gfx.drawText(text, 0, 70)
    end
end

-- Sets the index of the tile at tilemap position (x, y). index is the (1-based) index of the image
-- in the tilemap’s playdate.graphics.imagetable.
function dm.Tilemap:setTileAtPosition(x, y, index)
    -- print(tostring(x + ((y - 1) * self.width))..': '..tostring(index))
    self.map[(x - 1) + ((y - 1) * self.width)] = index
end

-- Returns the image index of the tile at the given x and y coordinate. If x or y is out of bounds, returns nil.
function dm.Tilemap:getTileAtPosition(x, y)
    return self.map[x + ((y - 1) * self.width)]
end

-- Sets the tilemap’s width and height, in number of tiles.
function dm.Tilemap:setSize(width, height)
    self.width = width
    self.height = height
    self.map = {}
end

-- Returns the size of the tile map, in tiles, as a pair, (width, height).
function dm.Tilemap:getSize()
    return self.width, self.height
end

-- Returns the size of the tilemap in pixels; that is, the size of the image multiplied by the number of
-- rows and columns in the map. Returns multiple alues (width, height).
function dm.Tilemap:getPixelSize()
    local image_width, image_height = self:getTileSize()
    return self.width * image_width, self.height * image_height
end

-- Returns multiple values (width, height), the pixel width and height of an individual tile.
function dm.Tilemap:getTileSize()
    local image = self.image_table:getImage(1)
    return image.width, image.height
end
