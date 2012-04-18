#!/usr/bin/env VERSIONER_PYTHON_PREFER_32_BIT=yes python
 
import wx
import mapperRec

app = wx.App()

frame = wx.Frame(None, -1, 'test')
frame.Show()

app.MainLoop()

mapperRec.set_backend(mapperRec.BACKEND_FILE)
mapperRec.set_device_name("testsend")
mapperRec.set_output_filename("test.txt")
mapperRec.start()
