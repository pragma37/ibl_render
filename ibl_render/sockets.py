import socket as socket_lib
import struct
import ctypes

def send_array(socket, array, format):
    array = struct.pack(format*len(array),*array)
    socket.sendall(array)

def receive_string(socket):
    string = b''
    while True:
        char = socket.recv(1)
        if char == b'\0':
            return string.decode('ascii')
        else:
            string += char

def receive_data(socket, size):
    data = b''
    max_size = 4096
    remaining_size = size
    while remaining_size > 0:
        #recv_size = min(remaining_size, max_size)
        new_data = socket.recv(remaining_size)
        data += new_data
        remaining_size -= len(new_data)
    return data

def receive_array(socket,length,format,format_size=4):
    data = receive_data(socket, length * format_size)
    return struct.unpack(format*length, data)

def receive_vector(socket):
    vector = receive_data(socket,4*3)
    return vector

def receive_matrix(socket):
    vector = receive_data(socket,4*16)
    return vector