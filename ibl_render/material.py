import bpy

def update_material(self, context):
    material = self.id_data
    material.diffuse_color = self.albedo
    material.specular_intensity = 0

class IBLMaterialProperty(bpy.types.PropertyGroup):
    albedo = bpy.props.FloatVectorProperty(name='Albedo', default=(0.5,0.5,0.5), subtype='COLOR', update=update_material)
    metallic = bpy.props.FloatProperty(name='Metallic', default=0.0, min=0.0, max=1.0, update=update_material)
    roughness = bpy.props.FloatProperty(name='Roughness', default=0.5, min=0.0, max=1.0, update=update_material)

class IBLMaterial(bpy.types.Panel):
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'

    bl_context = "material"
    bl_label = "IBL Render"
    COMPAT_ENGINES = {'ibl_render'}

    @classmethod
    def poll(cls, context):
        return context.scene.render.engine == 'ibl_render' and context.object is not None

    def draw(self, context):
        layout = self.layout
        object = context.object
        material = object.active_material

        material = context.material
        slot = context.material_slot
        space = context.space_data

        if object:
            is_sortable = len(object.material_slots) > 1
            rows = 1
            if (is_sortable): rows = 4

            row = layout.row()

            row.template_list("MATERIAL_UL_matslots", "", object, "material_slots", object, "active_material_index", rows=rows)

            col = row.column(align=True)
            col.operator("object.material_slot_add", icon='ZOOMIN', text="")
            col.operator("object.material_slot_remove", icon='ZOOMOUT', text="")

            col.menu("MATERIAL_MT_specials", icon='DOWNARROW_HLT', text="")

            if is_sortable:
                col.separator()

                col.operator("object.material_slot_move", icon='TRIA_UP', text="").direction = 'UP'
                col.operator("object.material_slot_move", icon='TRIA_DOWN', text="").direction = 'DOWN'

            if object.mode == 'EDIT':
                row = layout.row(align=True)
                row.operator("object.material_slot_assign", text="Assign")
                row.operator("object.material_slot_select", text="Select")
                row.operator("object.material_slot_deselect", text="Deselect")

        split = layout.split(percentage=0.65)

        if object:
            split.template_ID(object, "active_material", new="material.new")
            row = split.row()

            if slot:
                row.prop(slot, "link", text="")
            else:
                row.label()
        elif material:
            split.template_ID(space, "pin_id")
            split.separator()

        layout.separator();
        layout.label("Material Settings:")
        
        if material:
            layout.prop(material.ibl, "albedo")
            layout.prop(material.ibl, "metallic")
            layout.prop(material.ibl, "roughness")


def register():
    bpy.utils.register_class(IBLMaterialProperty)
    bpy.types.Material.ibl = bpy.props.PointerProperty(type=IBLMaterialProperty)

    bpy.utils.register_class(IBLMaterial)


def unregister():
    bpy.utils.unregister_class(IBLMaterialProperty)
    del bpy.types.Material.ibl

    bpy.utils.unregister_class(IBLMaterial)