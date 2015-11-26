# -*- coding:utf-8 -*-
'''
Created on 2015-11-20

@author: zhenkui
'''

import globalplatformlib as gp

def testListReaders():
    print "***** testListReaders()"
    context = gp.establishContext()
    readers = gp.listReaders(context)
    for reader in readers:
        print reader
    gp.releaseContext(context)
    print "-----end\n"

def testSCP():
    print "***** testSCP()"
    context = gp.establishContext()
    readerName = selectReader(context)
    cardInfo = gp.connectCard(context, readerName, gp.SCARD_PROTOCOL_Tx)
    scp, scpi = gp.getSCPDetails(context, cardInfo)
    securityInfo = gp.mutualAuthentication(context, cardInfo, gp.DEFAULT_KEY, gp.DEFAULT_KEY, gp.DEFAULT_KEY, gp.DEFAULT_KEY, 0, 0, scp, scpi, 0, 0)
    gp.releaseContext(context)
    print "-----end\n"
    
def testGetData():
    print "***** testGetData()"
    context = gp.establishContext()
    readerName = selectReader(context)
    cardInfo = gp.connectCard(context, readerName, gp.SCARD_PROTOCOL_Tx)
    scp, scpi = gp.getSCPDetails(context, cardInfo)
    securityInfo = gp.mutualAuthentication(context, cardInfo, gp.DEFAULT_KEY, gp.DEFAULT_KEY, gp.DEFAULT_KEY, gp.DEFAULT_KEY, 0, 0, scp, scpi, 0, 0)
    data1 = gp.getData(context, cardInfo, securityInfo, gp.TAG_DATA_IIN)
    data2 = gp.getData(context, cardInfo, securityInfo, gp.TAG_DATA_CIN)
    data3 = gp.getData(context, cardInfo, securityInfo, gp.TAG_DATA_CARD_DATA)
    data4 = gp.getData(context, cardInfo, securityInfo, gp.TAG_DATA_KEY_INFORMATION_TEMPLATE)
    data5 = gp.getData(context, cardInfo, securityInfo, gp.TAG_DATA_SECURITY_LEVEL)
    data6 = gp.getData(context, cardInfo, securityInfo, gp.TAG_DATA_APPLICATIONS)
    data7 = gp.getData(context, cardInfo, securityInfo, gp.TAG_DATA_EXTENDED_CARD_RESOURCE_INFORMATION)
    data8 = gp.getData(context, cardInfo, securityInfo, gp.TAG_CONFIRMATION_COUNTER)
    data9 = gp.getData(context, cardInfo, securityInfo, gp.TAG_SEQUENCE_COUNTER)
    gp.releaseContext(context)
    
    if data1 != -1:
        print "1 IIN: " + "".join("%02X" %(ord(b)) for b in data1)
    if data2 != -1:
        print "2 CIN: " + "".join("%02X" %(ord(b)) for b in data2)
    if data3 != -1:
        print "3 CARD DATA: " + "".join("%02X" %(ord(b)) for b in data3)
    if data4 != -1:
        print "4 KEY INFORMATION TEMPLATE: " + "".join("%02X" %(ord(b)) for b in data4)
    if data5 != -1:
        print "5 SECURITY LEVEL: " + "".join("%02X" %(ord(b)) for b in data5)
    if data6 != -1:
        print "6 APPLICATIONS: " + "".join("%02X" %(ord(b)) for b in data6)
    if data7 != -1:
        print "7 EXTENDED CARD RESOURCE INFORMATION: " + "".join("%02X" %(ord(b)) for b in data7)
    if data8 != -1:
        print "8 CONFIRMATION COUNTER: " + "".join("%02X" %(ord(b)) for b in data8)
    if data9 != -1:
        print "9 SEQUENCE COUNTER: " + "".join("%02X" %(ord(b)) for b in data9)
    print "-----end\n"
    
def selectReader(context):
    readers = gp.listReaders(context)
    for i in xrange(len(readers)):
        print "%d - %s" %(i, readers[i])
    readerIndex = len(readers) + 1
    while readerIndex not in range(len(readers)):
        readerIndex = raw_input("Please select the reader: ")
        readerIndex = int(readerIndex)
    return readers[readerIndex]

def testGetStatus():
    print "***** testGetData()"
    context = gp.establishContext()
    readerName = selectReader(context)
    cardInfo = gp.connectCard(context, readerName, gp.SCARD_PROTOCOL_Tx)
    scp, scpi = gp.getSCPDetails(context, cardInfo)
    securityInfo = gp.mutualAuthentication(context, cardInfo, gp.DEFAULT_KEY, gp.DEFAULT_KEY, gp.DEFAULT_KEY, gp.DEFAULT_KEY, 0, 0, scp, scpi, 0, 0)
    isd = gp.getStatus(context, cardInfo, securityInfo, gp.GET_STATUS_P1_ISD)
    appSSDs = gp.getStatus(context, cardInfo, securityInfo, gp.GET_STATUS_P1_APP_SSD)
    exeFiles = gp.getStatus(context, cardInfo, securityInfo, gp.GET_STATUS_P1_EXECUTABLE_LOAD_FILES)
    exeFilesModules = gp.getStatus(context, cardInfo, securityInfo, gp.GET_STATUS_P1_EXECUTABLE_LOAD_FILES_MODULES)
    gp.releaseContext(context)
    
    for status in (isd, appSSDs, exeFiles, exeFilesModules):
        if status != -1:
            appsInfo, fileModulesInfo = status
            for appInfo in appsInfo:
                print "AID: %s - PRIV: %02X - LIFE-CYCLE-STATE: %02X" %("".join("%02X" %(ord(b)) for b in appInfo['aid']), appInfo['privileges'], appInfo['lifeCycleState'])
            for fileModuleInfo in fileModulesInfo:
                print "AID: %s - PRIV: %02X" %("".join("%02X" %(ord(b)) for b in appInfo['aid']), fileModuleInfo['lifeCycleState'])
                for moduleInfo in fileModuleInfo['executableModules']:
                    print "    " + "".join('%02X' %(ord(b)) for b in moduleInfo)
    print "-----end\n"

def testLoadAndInstall():
    print "***** testLoadAndInstall()"
    context = gp.establishContext()
    readerName = selectReader(context)
    cardInfo = gp.connectCard(context, readerName, gp.SCARD_PROTOCOL_Tx)
    scp, scpi = gp.getSCPDetails(context, cardInfo)
    securityInfo = gp.mutualAuthentication(context, cardInfo, gp.DEFAULT_KEY, gp.DEFAULT_KEY, gp.DEFAULT_KEY, gp.DEFAULT_KEY, 0, 0, scp, scpi, 0, 0)
    
    capFile = 'C:\\Users\\zhenkui\\Desktop\\AlgTest.cap'
    loadFileInfo = gp.readLoadExecutableFileParameters(capFile)
    packageAID = loadFileInfo['loadFileAID']
    appletAID = loadFileInfo['applets'][0]
    
    try:
        ret = gp.deleteApplication(context, cardInfo, securityInfo, packageAID)
    except Exception, e:
        print e.message
        pass
    ret = gp.installForLoad(context, cardInfo, securityInfo, packageAID, gp.AID_ISD, '', '', 0, 0, 0)
    if ret == -1:
        raise Exception("Install for load failed.")
    ret = gp.load(context, cardInfo, securityInfo, '', capFile)
    if ret == -1:
        raise Exception("Load failed.")
    ret = gp.installForInstall(context, cardInfo, securityInfo, packageAID, appletAID, appletAID, 0, 0, 0, 0, '')
    if ret == -1:
        raise Exception("Insatll failed.")

    gp.releaseContext(context)

    print "-----end\n"

if __name__ == "__main__":
    gp.enableTraceMode(1)
    testListReaders()
    testSCP()
    testGetData()
    testGetStatus()
    testLoadAndInstall()
    testGetStatus()
    