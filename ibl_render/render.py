import bpy
import bl_ui

def register():
    bl_ui.properties_render.RENDER_PT_dimensions.COMPAT_ENGINES.add("ibl_render")

def unregister():
    bl_ui.properties_render.RENDER_PT_dimensions.COMPAT_ENGINES.remove("ibl_render")
