-- Globals provided by Tilemap.
--
-- This file can be used by toyboypy (https://toyboxpy.io) to import into a project's luacheck config.
--
-- Just add this to your project's .luacheckrc:
--    require "toyboxes/luacheck" (stds, files)
--
-- and then add 'toyboxes' to your std:
--    std = "lua54+playdate+toyboxes"

return {
    globals = {
        dm = {
            fields = {
                Tilemap = {
                    fields = {
                        super = {
                            fields = {
                                className = {},
                                init = {},
                            }
                        },
                        className = {},
                        init = {},
                        setImageTable = {},
                        setTiles = {},
                        getTiles = {},
                        draw = {},
                        setTileAtPosition = {},
                        getTileAtPosition = {},
                        setSize = {},
                        getSize = {},
                        getPixelSize = {},
                        getTileSize = {}
                    }
                }
            }
        }
    }
}