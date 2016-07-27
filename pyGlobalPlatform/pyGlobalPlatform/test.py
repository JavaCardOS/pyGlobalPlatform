from pyGlobalPlatform import globalplatformlib as gp

context = gp.establishContext()
readers = gp.listReaders(context)
print 'List readers:'
for i in range(len(readers)):
    reader = readers[i]
    print '    %d - ' %(i) + reader

reader = 'Feitian R502 Contactless Reader 0'
print 'Connect to reader: ' + reader
connection = gp.connectCard(context, reader, gp.SCARD_PROTOCOL_T1)
cmd = '\x00\xA4\x04\x00\x00'
print '>> ' + ''.join('%02X' %(ord(b)) for b in cmd)
rsp = gp.sendApdu(context, connection, None, '\x00\xA4\x04\x00\x00')
print '<< ' + ''.join('%02X' %(ord(b)) for b in rsp)
