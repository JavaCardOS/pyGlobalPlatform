# pyGlobalPlatform
pyGlobalPlatform is a open source python **globalplatform** client library. It is depend on the [GlobalPlatform](http://sourceforge.net/projects/globalplatform/) project. Using this library, you can use all features of GlobalPlatform project use Python programming language.

# Dependencies
Open source project: [GlobalPlatform](http://sourceforge.net/projects/globalplatform/)

# Usage:
## Code

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

## Result
![Usage](pyGlobalPlatform-usage.png)

For more api usage examples, please visit [pyGlobalPlatform Developer's Guide](http://javacardos.com/javacardforum/viewforum.php?f=41).

See [GlobalPlatform Library API reference](http://globalplatform.sourceforge.net/apidocs/index.html) for more details about API.

# Structure
![Structure](pyGlobalPlatform.png)

- **Opensource project [GlobalPlatform](http://sourceforge.net/projects/globalplatform/)**: The implementation of GlobalPlatform functions.
- **pyGlobalPlatform-pyd**: python objects to C data types conversion.
- **pyGlobalPlatform-py(globalplatformlib)**: Define python API prototype, constants.
- **GlobalPlatform Client Applications**: Implemention of some GP tools using python programming language.


# Build

Provide cmake script, this project can be compiled for use on both windows and linux platform. For more details, please visit our forum [JavacardOS pyGlobalPlatform Discussions](http://javacardos.com/javacardforum/viewforum.php?f=41).

# Developer's Guide
Introduce the usage of library APIs. Visit: [pyGlobalPlatform Developer's Guide](http://javacardos.com/javacardforum/viewforum.php?f=41)
# Discussions
If you have any questions, please visit: [JavacardOS pyGlobalPlatform Discussions](http://javacardos.com/javacardforum/viewforum.php?f=41)

# Website
[JavacardOS](http://www.javacardos.com)
