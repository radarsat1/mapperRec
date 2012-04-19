#!/usr/bin/env python

import ctypes, time
import _ctypes
import os

def func_address(adll, name):
    if 'dlsym' in dir(_ctypes):
        return _ctypes.dlsym(adll._handle, name)
    else:
        return _ctypes.GetProcAddress(adll._handle, name)

if os.path.exists("./libmapperrec.dylib"):
    rec = ctypes.cdll.LoadLibrary("./libmapperrec.dylib")
elif os.path.exists("./libmapperrec.so"):
    rec = ctypes.cdll.LoadLibrary("./libmapperrec.so")

rec.oscstreamdb_defaults()

backend_strings = (ctypes.c_char_p*3).in_dll(rec, "backend_strings")

backend = ctypes.c_int.in_dll(rec, "backend")

BACKEND_FILE = 0
BACKEND_BINARY = 1
BACKEND_OSCSTREAMDB = 2

class backend_text_options_t(ctypes.Structure):
    _fields_ = [("file_path", ctypes.c_char_p)]

class backend_binary_options_t(ctypes.Structure):
    _fields_ = [("file_path", ctypes.c_char_p)]

backend_text_options = backend_text_options_t.in_dll(rec, "backend_text_options")
backend_binary_options = backend_binary_options_t.in_dll(rec, "backend_binary_options")

def set_output_filename(fn):
    backend_text_options.file_path = fn
    backend_binary_options.file_path = fn

backend_patterns = {
    BACKEND_FILE: BACKEND_FILE,
    BACKEND_BINARY: BACKEND_BINARY,
    BACKEND_OSCSTREAMDB: BACKEND_OSCSTREAMDB,
    'file': BACKEND_FILE,
    'text': BACKEND_FILE,
    'binary': BACKEND_BINARY,
    'oscstreamdb': BACKEND_OSCSTREAMDB,
}
    
def set_device_name(name):
    rec.recmonitor_add_device_string(name)

def set_path_name(name):
    rec.recmonitor_add_signal_string(name)

def set_backend(b):
    backend.value = backend_patterns[b]

def start():
    if ctypes.c_int.in_dll(rec, "n_device_strings").value < 1:
        raise Exception("Must set device or path name.")

    backend_start = ctypes.c_void_p.in_dll(rec, "backend_start")
    backend_poll = ctypes.c_void_p.in_dll(rec, "backend_poll")
    backend_stop = ctypes.c_void_p.in_dll(rec, "backend_stop")
    backend_write_value = ctypes.c_void_p.in_dll(rec, "backend_write_value")

    if backend.value == BACKEND_FILE:
        backend_start.value = func_address(rec, "text_start")
        backend_stop.value = func_address(rec, "text_stop")
        backend_poll.value = func_address(rec, "text_poll")
        backend_write_value.value = func_address(rec, "text_write_value")
    elif backend.value == BACKEND_BINARY:
        backend_start.value = func_address(rec, "binary_start")
        backend_stop.value = func_address(rec, "binary_stop")
        backend_poll.value = func_address(rec, "binary_poll")
        backend_write_value.value = func_address(rec, "binary_write_value")
    elif backend.value == BACKEND_OSCSTREAMDB:
        # TODO OSCStreamDB options
        raise Exception("Still need to implement OSCStreamDB options.")
        backend_start.value = func_address(rec, "oscstreamdb_start")
        backend_stop.value = func_address(rec, "oscstreamdb_stop")
        backend_poll.value = func_address(rec, "oscstreamdb_poll")
        backend_write_value.value = func_address(rec, "oscstreamdb_write_value")

    backend_start = ctypes.CFUNCTYPE(None).in_dll(rec, "backend_start")
    backend_poll = ctypes.CFUNCTYPE(None).in_dll(rec, "backend_poll")
    backend_stop = ctypes.CFUNCTYPE(None).in_dll(rec, "backend_stop")

    # Instruct library to send us names via a memory FIFO
    ctypes.c_int.in_dll(rec, "send_device_names").value = 1
    ctypes.c_int.in_dll(rec, "send_signal_names").value = 1

    if backend_start():
        raise Exception("Error starting backend.")

    try:
        if rec.recmonitor_start():
            raise Exception("Error starting monitor.")

        try:
            if rec.recdevice_start():
                raise Exception("Error starting device.")
        except:
            rec.recmonitor_stop()
            raise

    except:
        backend_stop()
        raise

def stop():
    rec.recdevice_stop()
    rec.recmonitor_stop()
    if ctypes.c_int.in_dll(rec, "backend_stop")!=0:
        backend_stop = ctypes.CFUNCTYPE(None).in_dll(rec, "backend_stop")
        backend_stop()

def poll():
    backend_poll = ctypes.CFUNCTYPE(None).in_dll(rec, "backend_poll")
    if backend_poll() or rec.command_poll():
        return True
    rec.recmonitor_poll()
    rec.recdevice_poll()

_get_device_name = rec.get_device_name
_get_device_name.argtypes = None
_get_device_name.restype = ctypes.c_char_p

_get_signal_name = rec.get_signal_name
_get_signal_name.argtypes = None
_get_signal_name.restype = ctypes.c_char_p

def get_device_name():
    s = _get_device_name()
    if s!=None and s!='':
        return ord(s[0]), s[1:]
    return None, None

def get_signal_name():
    s = _get_signal_name()
    if s!=None and s!='':
        return ord(s[0]), s[1:]
    return None, None

if __name__=="__main__":
    set_backend(BACKEND_FILE)
    set_device_name("testsend")
    set_output_filename("test.txt")
    start()
