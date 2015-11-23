# -*- coding:utf-8 -*-

'''
Created on 2015-11-20

@author: javacardos@gmail.com
'''

import globalplatform as gp


def establishContext():
    return gp.establishContext()

def releaseContext(context):
    return gp.releaseContext(context)
    
def listReaders(context):
    return gp.listReaders(context)


'''
Issuer security domain AID;
'''
AID_ISD = '\xA0\x00\x00\x00\x03\x00\x00\x00'

'''
Protocol values;
'''
SCARD_PROTOCOL_UNDEFINED    = 0x00000000
SCARD_PROTOCOL_T0           = 0x00000001
SCARD_PROTOCOL_T1           = 0x00000002
SCARD_PROTOCOL_RAW          = 0x00010000
SCARD_PROTOCOL_Tx           = (SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1)

def connectCard(context, readerName, protocol):
    return gp.connectCard(context, readerName, protocol)

def disconnectCard(context, cardInfo):
    return gp.disconnectCard(context, cardInfo)

def selectApplication(context, cardInfo, aid):
    return gp.OPGP_select_application(context, cardInfo, aid)

'''
GET STATUS p1 values;
'''
GET_STATUS_P1_ISD = 0x80
GET_STATUS_P1_APP_SSD = 0x40
GET_STATUS_P1_EXECUTABLE_LOAD_FILES = 0x20
GET_STATUS_P1_EXECUTABLE_LOAD_FILES_MODULES = 0x10

def getStatus(context, cardInfo, securityInfo, cardElement):
    return gp.GP211_get_status(context, cardInfo, securityInfo, cardElement)

def setStatus(context, cardInfo, securityInfo, cardElement, aid, lifeCycleState):
    return gp.GP211_set_status(context, cardInfo, securityInfo, cardElement, aid, lifeCycleState)

'''
Default SCP key value;
'''
DEFAULT_KEY = '\x40\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4A\x4B\x4C\x4D\x4E\x4F'

def mutualAuthentication(context, cardInfo, baseKey, sencKey, smacKey, dekKey, keySetVersion, keyIndex, scp, scpi, securityLevel, derivationMethod):
    return gp.GP211_mutual_authentication(context, cardInfo, baseKey, sencKey, smacKey, dekKey, keySetVersion, keyIndex, scp, scpi, securityLevel, derivationMethod)

def initImplicitSecureChannel(aid, baseKey, sencKey, smacKey, dekKey, scpi, sequenceCounter):
    return gp.GP211_init_implicit_secure_channel(aid, baseKey, sencKey, smacKey, dekKey, scpi, sequenceCounter)

def closeImplicitSecureChannel(securityInfo):
    return gp.GP211_close_implicit_secure_channel(securityInfo)

'''
GET DATA tag values;
'''
TAG_DATA_IIN = '\x42'
TAG_DATA_CIN = '\x45'
TAG_DATA_CARD_DATA = '\x66'
TAG_DATA_KEY_INFORMATION_TEMPLATE = '\xE0'
TAG_DATA_SECURITY_LEVEL = '\xD3'
TAG_DATA_APPLICATIONS = '\x2F00'
TAG_DATA_EXTENDED_CARD_RESOURCE_INFORMATION = '\xFF\x21'
TAG_CONFIRMATION_COUNTER = '\xC2'
TAG_SEQUENCE_COUNTER = '\xC1'

def getData(context, cardInfo, securityInfo, identifier):
    return gp.GP211_get_data(context, cardInfo, securityInfo, identifier)

def getData_ISO7816_4(context, cardInfo, identifier):
    return gp.GP211_get_data_iso7816_4(context, cardInfo, identifier)

def getSCPDetails(context, cardInfo):
    return gp.GP211_get_secure_channel_protocol_details(context, cardInfo)

def getSequenceCounter(context, cardInfo):
    return gp.GP211_get_sequence_counter(context, cardInfo)

def putData(context, cardInfo, securityInfo, identifier, data):
    return gp.GP211_put_data(context, cardInfo, securityInfo, identifier, data)

def changePIN(context, cardInfo, securityInfo, tryLimit, newPIN):
    return gp.GP211_pin_change(context, cardInfo, securityInfo, tryLimit, newPIN)

def put3DESKey(context, cardInfo, securityInfo, keySetVersion, keyIndex, newKeySetVersion, keyData):
    return gp.GP211_put_3des_key(context, cardInfo, securityInfo, keySetVersion, keyIndex, newKeySetVersion, keyData)

def putRSAKey(context, cardInfo, securityInfo, keysetVersion, keyIndex, newKeySetVersion, pemKeyFileName, passPhrase):
    return gp.GP211_put_rsa_key(context, cardInfo, securityInfo, keysetVersion, keyIndex, newKeySetVersion, pemKeyFileName, passPhrase)

def putSCKey(context, cardInfo, securityInfo, keysetVersion, newKeySetVersion, newBaseKey, newSEncKey, newSMacKey, newDEKKey):
    return gp.GP211_put_secure_channel_keys(context, cardInfo, securityInfo, keysetVersion, newKeySetVersion, newBaseKey, newSEncKey, newSMacKey, newDEKKey)

def deleteKey(context, cardInfo, securityInfo, keysetVersion, keyIndex):
    return gp.GP211_delete_key(context, cardInfo, securityInfo, keysetVersion, keyIndex)

def getKeyInformationTemplates(context, cardInfo, securityInfo, num):
    return gp.GP211_get_key_information_templates(context, cardInfo, securityInfo, num)

def deleteApplication(context, cardInfo, securityInfo, aids):
    return gp.GP211_delete_application(context, cardInfo, securityInfo, aids)

def installForLoad(context, cardInfo, securityInfo, packageAID, sdAID, dataBlockHash, loadToken, nonVolatileCodeSpaceLimit, volatileDataSpaceLimit, nonVolatileDataSpaceLimit):
    return gp.GP211_install_for_load(context, cardInfo, securityInfo, packageAID, sdAID, dataBlockHash, loadToken, nonVolatileCodeSpaceLimit, volatileDataSpaceLimit, nonVolatileDataSpaceLimit)

def getExtraditionTokenSignatureData(sdAID, aid, tokenSignData):
    return gp.GP211_get_extradition_token_signature_data(sdAID, aid, tokenSignData)

def getLoadTokenSignatureData(aid, sdAID, loadFileDataBlockHash, nonVolatileCodeSpaceLimit, volatileDataSpaceLimit, noVolatileDataSpaceLimit):
    return gp.GP211_get_load_token_signature_data(aid, sdAID, loadFileDataBlockHash, nonVolatileCodeSpaceLimit, volatileDataSpaceLimit, noVolatileDataSpaceLimit)

def getInstallTokenSignatureData(p1, packageAID, moduleAID, appletAID, privileges, volatileDataSpaceLimit, nonVolatileDataSpaceLimit, installParameters):
    return gp.GP211_get_install_token_signature_data(p1, packageAID, moduleAID, appletAID, privileges, volatileDataSpaceLimit, nonVolatileDataSpaceLimit, installParameters)

def calculateLoadToken(packageAID, sdAID, loadFileDataBlockHash, nonVolatileCodeSpaceLimit, volatileDataSpaceLimit, nonVolatileDataSpaceLimit, pemKeyFileName, passPhrase):
    return gp.GP211_calculate_load_token(packageAID, sdAID, loadFileDataBlockHash, nonVolatileCodeSpaceLimit, volatileDataSpaceLimit, nonVolatileDataSpaceLimit, pemKeyFileName, passPhrase)

def calculateInstallToken(packageAID, moduleAID, appletAID, privileges, volatileDataSpaceLimit, nonVolatileDataSpaceLimit, installParameters, pemKeyFileName, passPhrase):
    return gp.GP211_calculate_install_token(packageAID, moduleAID, appletAID, privileges, volatileDataSpaceLimit, nonVolatileDataSpaceLimit, installParameters, pemKeyFileName, passPhrase)

def calculateLoadFileDataBlockHash(pemKeyFileName):
    return gp.GP211_calculate_load_file_data_block_hash(pemKeyFileName)

def load(context, cardInfo, securityInfo, dapBlock, packageFileName):
    return gp.GP211_load(context, cardInfo, securityInfo, dapBlock, packageFileName)

def loadFromBuffer(context, cardInfo, securityInfo, dapBlock, packageBuf):
    return gp.GP211_load_from_buffer(context, cardInfo, securityInfo, dapBlock, packageBuf)

def installForInstall(context, cardInfo, securityInfo, packageAID, moduleAID, appletAID, privileges, volatileDataSpaceLimit, nonVolatileDataSpaceLimit, installParameters, installToken):
    return gp.GP211_install_for_install(context, cardInfo, securityInfo, packageAID, moduleAID, appletAID, privileges, volatileDataSpaceLimit, nonVolatileDataSpaceLimit, installParameters, installToken)

def installForMakeSelectable(context, cardInfo, securityInfo, appletAID, privileges, installToken):
    return gp.GP211_install_for_make_selectable(context, cardInfo, securityInfo, appletAID, privileges, installToken)

def installForInstallAndMakeSelectable(context, cardInfo, securityInfo, packageAID, moduleAID, appletAID, privileges, volatileDataSpaceLimit, nonVolatileDataSpaceLimit, installParameters, installToken):
    return gp.GP211_install_for_install_and_make_selectable(context, cardInfo, securityInfo, packageAID, moduleAID, appletAID, privileges, volatileDataSpaceLimit, nonVolatileDataSpaceLimit, installParameters, installToken)

def installForPersonalization(context, cardInfo, securityInfo, appAID):
    return gp.GP211_install_for_personalization(context, cardInfo, securityInfo, appAID)

def insatllForExtradition(context, cardInfo, securityInfo, sdAID, appletAID, extrationToken):
    return gp.GP211_install_for_extradition(context, cardInfo, securityInfo, sdAID, appletAID, extrationToken)

def putDelegatedManagementKeys(context, cardInfo, securityInfo, keysetVersion, newKeySetVersion, pemKeyFileName, passPhrase, receiptKey):
    return gp.GP211_put_delegated_management_keys(context, cardInfo, securityInfo, keysetVersion, newKeySetVersion, pemKeyFileName, passPhrase, receiptKey)

def sendApdu(context, cardInfo, securityInfo, capdu):
    return gp.GP211_send_APDU(context, cardInfo, securityInfo, capdu)

def calculate3DESDap(loadFileDataBlockHash, sdAID, dapCalculationKey):
    return gp.GP211_calculate_3des_DAP(loadFileDataBlockHash, sdAID, dapCalculationKey)

def calculateRSADap(loadFileDataBlockHash, sdAID, pemKeyFileName, passPhrase):
    return gp.GP211_calculate_rsa_DAP(loadFileDataBlockHash, sdAID, pemKeyFileName, passPhrase)

def validateDeleteReceipt(confirmationCounter, cardUniqueData, receiptKey, receiptData, aid):
    return gp.GP211_validate_delete_receipt(confirmationCounter, cardUniqueData, receiptKey, receiptData, aid)

def validateInstallReceipt(confirmationCounter, cardUniqueData, receiptKey, receiptData, packageAID, appletAID):
    return gp.GP211_validate_install_receipt(confirmationCounter, cardUniqueData, receiptKey, receiptData, packageAID, appletAID)

def validateLoadReceipt(confirmationCounter, cardUniqueData, receiptKey, receiptData, packageAID, sdAID):
    return gp.GP211_validate_load_receipt(confirmationCounter, cardUniqueData, receiptKey, receiptData, packageAID, sdAID)

def validateExtraditionReceipt(confirmationCounter, cardUniqueData, receiptKey, receiptData, oldSdAid, newSdAid, aid):
    return gp.GP211_validate_extradition_receipt(confirmationCounter, cardUniqueData, receiptKey, receiptData, oldSdAid, newSdAid, aid)

def manageChannel(context, cardInfo, securityInfo, openClose, channelToClose):
    return gp.OPGP_manage_channel(context, cardInfo, securityInfo, openClose, channelToClose)

def selectChannel(cardInfo, channelNUmber):
    return gp.OPGP_select_channel(channelNUmber)

def storeData(context, cardInfo, securityInfo, data):
    return gp.GP211_store_data(context, cardInfo, securityInfo, data)

def readLoadExecutableFileParameters(fileName):
    return gp.OPGP_read_executable_load_file_parameters(fileName)

def enableTraceMode(v):
    return gp.OPGP_enable_trace_mode(v)
