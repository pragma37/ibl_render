import bpy
import bl_ui

UI_PANELS=(
    bl_ui.properties_scene.SCENE_PT_scene,
    bl_ui.properties_scene.SCENE_PT_unit,
    bl_ui.properties_scene.SCENE_PT_keying_sets,
    bl_ui.properties_scene.SCENE_PT_keying_set_paths,
    bl_ui.properties_scene.SCENE_PT_color_management,
    bl_ui.properties_scene.SCENE_PT_audio,
    bl_ui.properties_scene.SCENE_PT_physics,
    bl_ui.properties_scene.SCENE_PT_rigid_body_world,
    bl_ui.properties_scene.SCENE_PT_rigid_body_cache,
    bl_ui.properties_scene.SCENE_PT_rigid_body_field_weights,
    bl_ui.properties_scene.SCENE_PT_simplify,
    bl_ui.properties_scene.SCENE_PT_custom_props)

def register():
    for panel in UI_PANELS:
        panel.COMPAT_ENGINES.add("ibl_render")

def unregister():
    for panel in UI_PANELS:
        panel.COMPAT_ENGINES.remove("ibl_render")