#!/usr/bin/env VERSIONER_PYTHON_PREFER_32_BIT=yes python
 
import wx
import mapperRec

class RecFrame(wx.Frame):
    def __init__(self, parent, id, title):
        wx.Frame.__init__(self, parent, id, title,
                          wx.DefaultPosition, wx.Size(500, 500))
        vbox = wx.BoxSizer(wx.VERTICAL)
        panel1 = wx.Panel(self, -1)
        panel2 = wx.Panel(self, -1)
        box = wx.BoxSizer(wx.HORIZONTAL)
        self.devtext = wx.TextCtrl(panel1, -1, '<device>')
        self.sigtext = wx.TextCtrl(panel1, -1, '<signal>')
        box.Add(wx.StaticText(panel1, -1, "Match:"), 0, wx.ALIGN_CENTER)
        box.Add(self.devtext, 1, wx.TOP | wx.BOTTOM)
        box.Add(self.sigtext, 1, wx.TOP | wx.BOTTOM)
        panel1.SetSizer(box)
        box = wx.BoxSizer(wx.HORIZONTAL)
        self.devlist = wx.ListCtrl(panel2, -1, style=wx.LC_REPORT)
        self.siglist = wx.ListCtrl(panel2, -1, style=wx.LC_REPORT)
        self.devlist.InsertColumn(0, "Device")
        self.siglist.InsertColumn(0, "Signal")
        self.devlist.SetColumnWidth(0, 2048)
        self.siglist.SetColumnWidth(0, 2048)
        box.Add(self.devlist, 1, wx.EXPAND)
        box.Add(self.siglist, 1, wx.EXPAND)
        panel2.SetSizer(box)
        vbox.Add(panel1, 0, wx.EXPAND | wx.ALL)
        vbox.Add(panel2, 1, wx.EXPAND | wx.ALL)
        self.SetSizer(vbox)

        self.statusbar = self.CreateStatusBar(2)
        self.statusbar.SetStatusWidths([-1,100])
        self.statusbar.SetStatusText("Please specify a device to match.",0)
        self.statusbar.SetStatusText("Idle",1)

        self.devlist.InsertStringItem(0, "ExampleDevice")
        self.siglist.InsertStringItem(0, "ExampleSig")

class RecApp(wx.App):
    def OnInit(self):
        frame = RecFrame(None, -1, 'mapperRecorder')
        frame.Show(True)
        return True

app = RecApp(0)
app.MainLoop()

# mapperRec.set_backend(mapperRec.BACKEND_FILE)
# mapperRec.set_device_name("testsend")
# mapperRec.set_output_filename("test.txt")
# mapperRec.start()
