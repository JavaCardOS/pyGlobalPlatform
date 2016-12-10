#!/usr/bin/python

import globalplatformlib as gp
if not gp:
    from pyGlobalPlatform import globalplatformlib as gp

def INFO(info):
    print("INFO " + info)

def INFO_ARRAY(a):
    INFO(''.join('%02X' %(ord(c)) for c in a))

def str_bin(s):
    for i in len(s) >> 1:
        pass
    
if __name__ == "__main__":
    gp.enableTraceMode(True)
    
    c = gp.establishContext()
    
    readernames = gp.listReaders(c)
    for i in range(len(readernames)):
        print('%d - %s' %(i, readernames[i]))
    i = int(input("Please select one reader: "))
    cc = gp.connectCard(c, readernames[i], gp.SCARD_PROTOCOL_T1)
    
    gp.sendApdu(c, cc, None, '\x00\xB0\x00\x00\x00')
    gp.selectApplication(c, cc, '')
    sc = gp.mutualAuthentication(c, cc, gp.DEFAULT_KEY, gp.DEFAULT_KEY, gp.DEFAULT_KEY, gp.DEFAULT_KEY, 0x00, 0x00, 0x02, 0x15, 0, 0)
    
    INFO("Get status:")
    try:
        status10 = gp.getStatus(c, cc, None, gp.GET_STATUS_P1_EXECUTABLE_LOAD_FILES_MODULES)
        INFO("    Status 0x10: " + str(status10))
    except:
        pass
    try:
        status20 = gp.getStatus(c, cc, None, gp.GET_STATUS_P1_EXECUTABLE_LOAD_FILES)
        INFO("    Status 0x20: " + str(status20))
    except:
        pass
    try:
        status40 = gp.getStatus(c, cc, None, gp.GET_STATUS_P1_APP_SSD)
        INFO("    Status 0x40: " + str(status40))
    except:
        pass
    try:
        status80 = gp.getStatus(c, cc, None, gp.GET_STATUS_P1_ISD)
        INFO("    Status 0x80: " + str(status80))
    except:
        pass
    
    INFO("Get data:")
    try:
        data_iin = gp.getData(c, cc, sc, gp.TAG_DATA_IIN)
        INFO_ARRAY(data_iin, "    Data IIN: ", )
    except:
        pass
    try:
        data_cin = gp.getData(c, cc, sc, gp.TAG_DATA_CIN)
        INFO_ARRAY(data_cin, "    Data CIN: ", )
    except:
        pass
    try:
        card_data = gp.getData(c, cc, sc, gp.TAG_DATA_CARD_DATA)
        INFO_ARRAY(card_data, "    Data CARD_DATA: ", )
    except:
        pass
    try:
        data_key_info = gp.getData(c, cc, sc, gp.TAG_DATA_KEY_INFORMATION_TEMPLATE)
        INFO_ARRAY(data_key_info, "    Data KEY INFROMATION TEMPLATE: ", )
    except:
        pass
    try:
        data_security_level = gp.getData(c, cc, sc, gp.TAG_DATA_SECURITY_LEVEL)
        INFO_ARRAY(data_security_level, "    Data SECURITY LEVEL: ", )
    except:
        pass
    try:
        data_apps = gp.getData(c, cc, sc, gp.TAG_DATA_APPLICATIONS)
        INFO_ARRAY(data_apps, "    Data APPLICATIONS: ", )
    except:
        pass
    try:
        data_card_resource = gp.getData(c, cc, sc, gp.TAG_DATA_EXTENDED_CARD_RESOURCE_INFORMATION)
        INFO_ARRAY(data_card_resource, "    Data CARD RESOURCE INFORMATION: ", )
    except:
        pass
    try:
        data_confirmation_counter = gp.getData(c, cc, sc, gp.TAG_CONFIRMATION_COUNTER)
        INFO_ARRAY(data_confirmation_counter, "    Data CONFIRMATION COUNTER: ")
    except:
        pass
    try:
        data_sequence_counter = gp.getData(c, cc, sc, gp.TAG_SEQUENCE_COUNTER)
        INFO_ARRAY(data_sequence_counter, "    Data SEQUENCE COUNTER: ", )
    except:
        pass
    
    packageAID = '\x11\x22\x33\x44\x55'
    moduleAID = '\x11\x22\x33\x44\x55\x00'
    appletAID = '\x11\x22\x33\x44\x55\x00'
    INFO("Delete Applet:")
    try:
        gp.deleteApplication(c, cc, sc, [packageAID]);
    except:
        pass
    
    INFO("Load CAP:")
    gp.installForLoad(c, cc, sc, packageAID, None, None, None, 0, 0, 0)
    gp.load(c, cc, sc, None, 'javacardos.cap')
    INFO("Install Applet:")
    gp.installForInstallAndMakeSelectable(c, cc, sc, packageAID, moduleAID, appletAID, 0x00, 0, 0, None, None)
    INFO("Process Applet:")
    gp.selectApplication(c, cc, appletAID);
    gp.sendApdu(c, cc, sc, '\x00\x00\x00\x00\x00')
    
    gp.disconnectCard(c, cc)
    gp.releaseContext(c)
