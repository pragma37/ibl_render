import sys
import ctypes
from ctypes import *
from struct import pack, unpack
from sockets import *
import os
import gc

script_file = os.path.realpath(__file__)
directory = os.path.dirname(script_file)
dll = cdll.LoadLibrary(os.path.join(directory, 'ibl_render.dll'));

port = int(sys.argv[1])
socket = socket_lib.socket(socket_lib.AF_INET, socket_lib.SOCK_STREAM)
socket.connect(('localhost', port))

path = c_char_p(os.path.join(directory,'shaders','').encode('utf-8'))
width = 1024
height = 1024
dll.initialize(path, width, height, 4)

while True:
    function = receive_string(socket)

    if function == 'load_hdri':
        i_width, i_height, i_channels = receive_array(socket,3,'i')
        pixels = receive_data(socket, i_width*i_height*i_channels*4)
        dll.load_hdri(pixels, i_width, i_height, i_channels)
    if function == 'load_mesh':
        name = receive_string(socket).encode('ascii')
        vertex_count = receive_array(socket,1,'i')[0]
        vertices = receive_data(socket, 4*8*vertex_count)
        index_count = receive_array(socket,1,'i')[0]
        indices = receive_data(socket, 4*index_count)
        dll.load_mesh(c_char_p(name),
                      vertices,
                      vertex_count,
                      indices,
                      index_count)
    elif function == 'clean_resources':
        dll.clean_resources()
        gc.collect()
        ctypes._reset_cache()
    elif function == 'resize':
        _width, _height, _msaa = receive_array(socket,3,'i')
        if width != _width or height != _height:
            width, height, msaa = _width, _height, _msaa
            dll.resize(width, height, msaa)
    elif function == 'render_begin':
        camera_position = receive_vector(socket)
        view_matrix = receive_matrix(socket)
        projection_matrix = receive_matrix(socket)
        background_color = receive_vector(socket)
        dll.render_begin(camera_position, view_matrix, projection_matrix, background_color)
    elif function == 'draw_mesh':
        transform = receive_matrix(socket)
        mesh = receive_string(socket).encode('ascii')
        albedo = receive_vector(socket)
        metallic, roughness = receive_array(socket,2,'f')
        dll.draw_mesh(transform,c_char_p(mesh), albedo, c_float(metallic), c_float(roughness))
    elif function == 'render_end':
        pixel_count = width * height
        c_render_result = ctypes.POINTER(c_float * (pixel_count * 3))
        dll.render_end.restype = c_render_result
        pixels = dll.render_end()
        socket.sendall(pixels.contents)
    elif function == 'END':
        dll.terminate_context()
        break
