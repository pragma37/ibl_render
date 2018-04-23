bl_info = {
    "name": "IBL Render",
    "category": "Render",
}

if "bpy" in locals():
    import imp
    imp.reload(material)
    imp.reload(render)
    imp.reload(scene)
    imp.reload(world)
    imp.reload(sockets)

import bpy
import bl_ui
import bmesh
import bgl

from . import material
from . import render
from . import scene
from . import world
from . import sockets

from struct import pack, unpack
import itertools
import time

__socket = None

def get_socket():
    global __socket
    return __socket

def renderer_call(function,*args):
    global __socket
    function = (function+'\0').encode('ascii')
    __socket.sendall(function)
    for arg in args:
        __socket.sendall(arg)

class IBLRenderEngine(bpy.types.RenderEngine):
    bl_idname = "ibl_render"
    bl_label = "IBL Render"
    bl_use_preview = False
    bl_use_shading_nodes = False

    hdri = None

    def view_update(self, context):
        self.do_update(context.scene)

    def update(self, data, scene):
        self.do_update(scene)


    def do_update(self, scene):
        scene.world.ambient_color = [0.5, 0.5, 0.5]
        renderer_call('clean_resources')

        if scene.world.ibl_hdri and scene.world.ibl_hdri != self.hdri:
            hdri = scene.world.ibl_hdri
            renderer_call("load_hdri",
                          pack('iii', hdri.size[0], hdri.size[1], hdri.channels),
                          pack('f'*len(hdri.pixels), *hdri.pixels))
            self.hdri = hdri

        for object in scene.objects:
            if object.type == 'MESH' and object.hide_render == False:
                mesh_copy = object.to_mesh(scene, True, 'PREVIEW', False)
                bm = bmesh.new()
                bm.from_mesh(mesh_copy)
                bmesh.ops.triangulate(bm, faces=bm.faces)
                bm.to_mesh(mesh_copy)
                bm.free()
        
                vertices = []
                for vertex in mesh_copy.vertices:
                    vertices.extend(vertex.co)
                    vertices.extend(vertex.normal)
                    #uvs TODO
                    vertices.extend([0.0,0.0])
        
                indices = []
                for tri in mesh_copy.polygons:
                    indices.extend(tri.vertices)

                renderer_call('load_mesh', 
                              (object.data.name+'\0').encode('ascii'), 
                              pack('i',len(mesh_copy.vertices)), 
                              pack('f'*len(vertices),*vertices), 
                              pack('i',len(indices)),
                              pack('I'*(len(indices)),*indices))

                bpy.data.meshes.remove(mesh_copy)


    def view_draw(self, context):
        region = context.region
        view_3d = context.region_data

        pixels = self.do_render(context.scene, region.width, region.height, 
                                view_3d.view_matrix, view_3d.window_matrix,
                                view_3d.view_matrix.inverted().transposed()[3][:3])

        pixel_count = region.width * region.height
        bitmap = bgl.Buffer(bgl.GL_FLOAT, [pixel_count * 3], pixels)

        bgl.glRasterPos2i(0, 0)
        bgl.glDrawPixels(region.width, region.height, bgl.GL_RGB, bgl.GL_FLOAT, bitmap)


    def render(self, scene):
        scale = scene.render.resolution_percentage / 100.0
        self.size_x = int(scene.render.resolution_x * scale)
        self.size_y = int(scene.render.resolution_y * scale)

        view_matrix = scene.camera.matrix_world.inverted()
        projection_matrix = scene.camera.calc_matrix_camera(1,1,1,self.size_y/self.size_x)
        camera_position = scene.camera.location

        pixels = self.do_render(scene,self.size_x, self.size_y, 
                                view_matrix, projection_matrix, camera_position)

        pixel_count = self.size_x * self.size_y
        render_result = []
        for i in range(pixel_count):
            pixel = [pixels[i*3],pixels[i*3+1],pixels[i*3+2],1.0]
            render_result.append(pixel)
        
        result = self.begin_result(0, 0, self.size_x, self.size_y)
        layer = result.layers[0].passes["Combined"]
        layer.rect = render_result
        self.end_result(result)


    def do_render(self, scene, width, height, view_matrix, projection_matrix, camera_position):
        renderer_call('resize', pack('iii',width, height, 4))

        def b_matrix_to_list(matrix):
            matrix = matrix.transposed()
            list = []
            for vector in matrix:
                list.extend(vector)
            return list

        view_matrix = b_matrix_to_list(view_matrix)
        projection_matrix = b_matrix_to_list(projection_matrix)

        renderer_call('render_begin', 
                      pack('f'*3,*camera_position),
                      pack('f'*16,*view_matrix), 
                      pack('f'*16,*projection_matrix),
                      pack('f'*3,*scene.world.background_color))

        for object in scene.objects:
            if object.type == 'MESH' and object.hide_render == False:
                matrix = b_matrix_to_list(object.matrix_world)
                color = [1.0,0.0,1.0]
                if len(object.material_slots) > 0 and object.material_slots[0].material:
                    color = object.material_slots[0].material.ibl.color
                renderer_call('draw_mesh', 
                              pack('f'*16,*matrix),
                              (object.data.name+'\0').encode('ascii'),
                              pack('f'*3,*color))
        
        renderer_call('render_end')
        global __socket
        pixel_count = width * height
        pixels = sockets.receive_array(get_socket(), pixel_count*3, 'f')
        return pixels
            
        

def register():
    bpy.utils.register_class(IBLRenderEngine)
    bl_ui.properties_render.RENDER_PT_render.COMPAT_ENGINES.add(IBLRenderEngine.bl_idname)

    material.register()
    render.register()
    scene.register()
    world.register()

    import os
    import subprocess
    import socket as socket_lib

    socket = socket_lib.socket(socket_lib.AF_INET, socket_lib.SOCK_STREAM)
    socket.bind(('localhost',0))
    adress, port = socket.getsockname()
    script_file = os.path.realpath(__file__)
    directory = os.path.dirname(script_file)
    subprocess.Popen([bpy.app.binary_path_python, os.path.join(directory, 'renderer.py'), str(port)])
    socket.listen()
    connection, adress = socket.accept()
    global __socket
    __socket = connection


def unregister():
    bpy.utils.unregister_class(IBLRenderEngine)
    bl_ui.properties_render.RENDER_PT_render.COMPAT_ENGINES.remove(IBLRenderEngine.bl_idname)

    material.unregister()
    render.unregister()
    scene.unregister()
    world.unregister()

    renderer_call('clean_resources')
    renderer_call('END')

import atexit
@atexit.register
def on_exit():
    renderer_call('clean_resources')
    renderer_call('END')

if __name__ == "__main__":
    register()
