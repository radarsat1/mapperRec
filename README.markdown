
mapperRec
=========

This is a small libmapper program that creates a monitor and checks if
there are any interesting signals on the network.  When it finds one,
it creates a corresponding signal on its device, and requests a direct
mapping.  It then records any received information to one of several
back-ends.

The back-ends currently implemented are:

- A text file writer.
- A binary file writer.
- A writer that forwards the data to [Andy Schemder's OSCStreamDB][1], which writes it to a Postgresql database.

Currently, "interesting signals" are defined as any output signal that
matches a given device name.  So it can be used to record all output
signals of a device.  In the future this might be better defined using
a more complex matching strategy, a command line, or a GUI-based
selection.

Recording to SDIF/GDIF would be a nice enhancement too.

This software is licensed with the GPLv3; see the attached file
COPYING for details, which should be included in this download.

Stephen Sinclair 2011
[Input Devices and Music Interaction Laboratory][2], McGill University.

[1]: http://cnmat.berkeley.edu/system/files/attachments/oscstreamdb-final.pdf
[2]: http://idmil.org/software/libmapper
