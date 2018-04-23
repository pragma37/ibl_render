import bpy

class IBLWorld(bpy.types.Panel):
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'

    bl_context = "world"
    bl_label = "IBL Render"
    COMPAT_ENGINES = {'ibl_render'}

    @classmethod
    def poll(cls, context):
        return context.scene.render.engine == 'ibl_render'

    def draw(self, context):
        self.layout.prop(context.scene.world, "ibl_hdri")
        self.layout.prop(context.scene.world, "background_color")


def register():
    bpy.utils.register_class(IBLWorld)
    bpy.types.World.ibl_hdri = bpy.props.PointerProperty(type=bpy.types.Image, 
        name="HDRI", description="HDRI texture for image based lighting")
    bpy.types.World.background_color = bpy.props.FloatVectorProperty(name='Background Color', default=(0.5,0.5,0.5), subtype='COLOR')



def unregister():
    bpy.utils.unregister_class(IBLWorld)
    del bpy.types.World.ibl_hdri
    del bpy.types.World.background_color 