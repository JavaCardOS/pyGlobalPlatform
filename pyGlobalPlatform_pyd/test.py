#!/usr/bin/python

import pyglobalplatform as gp

c = gp.establishContext()
readername = gp.listReaders(c)[0]
ci = gp.connectCard(c, readername, 1)
selectCommand = '\x00\xA4\x04\x00\x00'
print ">> " + "".join("%02X" %(ord(v)) for v in selectCommand)
print "<< " + "".join("%02X" %(ord(v)) for v in gp.GP211_send_APDU(c, ci, None, selectCommand))
gp.disconnectCard(c, ci)
gp.releaseContext(c)

