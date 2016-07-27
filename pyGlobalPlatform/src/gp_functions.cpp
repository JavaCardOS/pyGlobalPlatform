#include "gp_functions.h"

#ifdef WIN32
#define UNICODE
#define _UNICODE
#include <Windows.h>
#include <tchar.h>
#else
#include <stdlib.h>
#include <wchar.h>
#endif

#include "globalplatform/globalplatform.h"
#include "globalplatform/connectionplugin.h"


/* Macro to get array element count; */
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

/* Macro for checking function arguments count; */
#define CHECK_FUNCTION_ARGUMENTS_COUNT(c) {\
    unsigned int uiArgumentCount = PyTuple_GET_SIZE(args);\
    if (uiArgumentCount != c) {\
        PyErr_Format(PyExc_TypeError, "%s() takes %u arguments. (%u given)", __FUNCTION__, c, uiArgumentCount);\
        return NULL;\
    }\
}

/* Macro for checking gp call result; */
#define CHECK_GP_CALL_RESULT(r) {\
    if (r.errorStatus != OPGP_ERROR_STATUS_SUCCESS) {\
        _RaiseError(PyExc_Exception, __FUNCTION__, r.errorMessage);\
        return NULL;\
    }\
}

/* Function to raise an error; */
static void _RaiseError(PyObject* pobjError, const char *pcFunctionName, const TCHAR *ptcMessage)
{
#ifdef UNICODE
    char pcMessage[0x400] = { 0 };
    sprintf(pcMessage, "%s(): %S", pcFunctionName, ptcMessage);
    PyErr_SetString(pobjError, pcMessage);
#else
    PyErr_SetString(pobjError, ptcMessage);
#endif
}

PyObject* establishContext(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(0);

    OPGP_CARD_CONTEXT stCardContext = { 0 };
    memset(&stCardContext.connectionFunctions, 0x00, sizeof(stCardContext.connectionFunctions));
    stCardContext.libraryHandle = NULL;
    stCardContext.librarySpecific = NULL;
#ifdef WIN32
    _tcsncpy_s(stCardContext.libraryName, ARRAY_SIZE(stCardContext.libraryName), _T("gppcscconnectionplugin"), _tcslen(_T("gppcscconnectionplugin")));
    _tcsncpy_s(stCardContext.libraryVersion, ARRAY_SIZE(stCardContext.libraryVersion), _T("1"), _tcslen( _T("1")));
#else
    strncpy(stCardContext.libraryName, _T("gppcscconnectionplugin"), _tcslen(_T("gppcscconnectionplugin")));
    strncpy(stCardContext.libraryVersion, _T("1"), _tcslen( _T("1")));
#endif
    OPGP_ERROR_STATUS errorStatus = OPGP_establish_context(&stCardContext);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyString_FromStringAndSize((const char *)&stCardContext, sizeof(stCardContext));
}

PyObject* releaseContext(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(1);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));

    OPGP_ERROR_STATUS errorStatus = OPGP_release_context(&stCardContext);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* listReaders(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(1);
    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));

    TCHAR tcaReaderNames[0x100 * 0x100];
    DWORD dwReaderNamesLength = sizeof(tcaReaderNames) / sizeof(TCHAR);
    OPGP_ERROR_STATUS errorStatus = OPGP_list_readers(stCardContext, (OPGP_STRING)tcaReaderNames, &dwReaderNamesLength);
    CHECK_GP_CALL_RESULT(errorStatus);

    int iReaderCount = 0;
    TCHAR* ptcaReadersName[0x100];
    DWORD dwReaderNameOffset = 0;
    while (true) {
        if (tcaReaderNames[dwReaderNameOffset] == '\0') {
            break;
        }

        TCHAR *ptcReaderName = &tcaReaderNames[dwReaderNameOffset];
        ptcaReadersName[iReaderCount] = ptcReaderName;
        ++iReaderCount;
        dwReaderNameOffset += _tcslen(ptcReaderName) + 1;
    }

    PyObject* pobjReaderNames = PyTuple_New(iReaderCount);
    for (int i = 0; i < iReaderCount; ++i) {
#ifdef UNICODE
        PyObject* pobjReaderName = PyUnicode_AsASCIIString(PyUnicode_FromWideChar(ptcaReadersName[i], _tcslen(ptcaReadersName[i])));
#else
        PyObject* pobjReaderName = PyString_FromStringAndSize(ptcaReadersName[i], strlen(ptcaReadersName[i]));
#endif
        PyTuple_SetItem(pobjReaderNames, i, pobjReaderName);
    }
    return pobjReaderNames;
}

PyObject* connectCard(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(3);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));
    PyObject* pobjReaderName = PyTuple_GetItem(args, 1);
    PyObject* pobjProtocol = PyTuple_GetItem(args, 2);

    long lProtocol = PyLong_AsLong(pobjProtocol);
    if ((!(OPGP_CARD_PROTOCOL_T0 & lProtocol)) && (!(OPGP_CARD_PROTOCOL_T1 & lProtocol))) {
        PyErr_SetFromErrno(PyExc_ValueError);
    }

    if (!PyString_Check(pobjReaderName)) {
        PyErr_SetFromErrno(PyExc_ValueError);
    }

    // Input shall be a string (not a unicode);
    if (!PyString_Check(pobjReaderName)) {
        PyErr_Format(PyExc_TypeError, "Please input reader name as a sting (not a unicode).");
    }

    TCHAR *ptcReaderName = NULL;
#ifdef UNICODE
    // String to const wchar_t *;
    ptcReaderName = PyUnicode_AsUnicode(PyUnicode_FromString(PyString_AsString(pobjReaderName)));
#else
    // String to const char *;
    ptcReaderName = PyString_AsString(pobjReaderName);
#endif

    OPGP_CARD_INFO stCardInfo = { 0 };
    OPGP_ERROR_STATUS errorStatus = OPGP_card_connect(stCardContext, ptcReaderName, &stCardInfo, lProtocol);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyString_FromStringAndSize((const char *)&stCardInfo, sizeof(OPGP_CARD_INFO));
}

PyObject* disconnectCard(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(2);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));
    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)PyString_AsString(PyTuple_GetItem(args, 1));

    OPGP_ERROR_STATUS errorStatus = OPGP_card_disconnect(stCardContext, &stCardInfo);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* pyOPGP_select_application(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(3);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));
    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)PyString_AsString(PyTuple_GetItem(args, 1));
    PyObject* pobjAID = PyTuple_GetItem(args, 2);

    PBYTE pbAID = (PBYTE)PyString_AsString(pobjAID);
    DWORD dwAIDLength = (DWORD)PyString_GET_SIZE(pobjAID);

    OPGP_ERROR_STATUS errorStatus = OPGP_select_application(stCardContext, stCardInfo, pbAID, dwAIDLength);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* pyGP211_get_status(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(4);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));
    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)PyString_AsString(PyTuple_GetItem(args, 1));
    GP211_SECURITY_INFO stGP211SecurityInfo = *(GP211_SECURITY_INFO *)PyString_AsString(PyTuple_GetItem(args, 2));
    PyObject* pobjElement= PyTuple_GetItem(args, 3);

#define NUM_APPLICATIONS 64
    DWORD dwDataCount = NUM_APPLICATIONS;
    GP211_APPLICATION_DATA astAppletData[NUM_APPLICATIONS];
    GP211_EXECUTABLE_MODULES_DATA astExecutableData[NUM_APPLICATIONS];
    BYTE bCardElement = (BYTE)PyLong_AsLong(pobjElement);

    OPGP_ERROR_STATUS errorStatus = GP211_get_status(stCardContext, stCardInfo, &stGP211SecurityInfo, bCardElement, astAppletData, astExecutableData, &dwDataCount);
    CHECK_GP_CALL_RESULT(errorStatus);

    if ((bCardElement & 0xE0) != 0) {
        PyObject* pobjAppletData = PyTuple_New(dwDataCount);
        for (DWORD d = 0; d < dwDataCount; ++d) {
            PyObject* pobjOneAppletData = PyDict_New();
            GP211_APPLICATION_DATA* pstOneAppletData = &astAppletData[d];
            PyDict_SetItem(pobjOneAppletData, PyString_FromString("aid"), PyString_FromStringAndSize((const char *)pstOneAppletData->AID, pstOneAppletData->AIDLength));
            PyDict_SetItem(pobjOneAppletData, PyString_FromString("lifeCycleState"), PyLong_FromLong(pstOneAppletData->lifeCycleState));
            PyDict_SetItem(pobjOneAppletData, PyString_FromString("privileges"), PyLong_FromLong(pstOneAppletData->privileges));

            PyTuple_SetItem(pobjAppletData, d, pobjOneAppletData);
        }

        PyObject* pobjRet = PyTuple_New(2);
        PyTuple_SetItem(pobjRet, 0, pobjAppletData);
        PyTuple_SetItem(pobjRet, 1, PyTuple_New(0));
        return pobjRet;
    } else if ((bCardElement & 0x10) != 0) {
        PyObject* pobjExecuableModulesData = PyTuple_New(dwDataCount);
        for (DWORD dwDataIndex = 0; dwDataIndex < dwDataCount; ++dwDataIndex) {
            PyObject* pobjOneExecuableData = PyDict_New();
            GP211_EXECUTABLE_MODULES_DATA* pstOneExecuableModulesData = &astExecutableData[dwDataIndex];
            PyDict_SetItem(pobjOneExecuableData, PyString_FromString("aid"), PyString_FromStringAndSize((const char *)pstOneExecuableModulesData->AID, pstOneExecuableModulesData->AIDLength));
            PyDict_SetItem(pobjOneExecuableData, PyString_FromString("lifeCycleState"), PyLong_FromLong(pstOneExecuableModulesData->lifeCycleState));

            BYTE bNumExecutableModules = pstOneExecuableModulesData->numExecutableModules;
            PyObject* pobjExecutableModuleData = PyTuple_New(bNumExecutableModules);
            for (BYTE bExecutableModuleIndex = 0; bExecutableModuleIndex < bNumExecutableModules; ++bExecutableModuleIndex) {
                OPGP_AID* pstExecutableModuleAID = &pstOneExecuableModulesData->executableModules[bExecutableModuleIndex];
                PyTuple_SetItem(pobjExecutableModuleData, bExecutableModuleIndex, PyString_FromStringAndSize((const char *)pstExecutableModuleAID->AID, pstExecutableModuleAID->AIDLength));
            }
            PyDict_SetItem(pobjOneExecuableData, PyString_FromString("executableModules"), pobjExecutableModuleData);

            PyTuple_SetItem(pobjExecuableModulesData, dwDataIndex, pobjOneExecuableData);
        }
        PyObject* pobjRet = PyTuple_New(2);
        PyTuple_SetItem(pobjRet, 0, PyTuple_New(0));
        PyTuple_SetItem(pobjRet, 1, pobjExecuableModulesData);
        return pobjRet;
    } else {
    }

    return PyLong_FromLong(-1);
}

PyObject* pyGP211_set_status(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(6);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));
    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)PyString_AsString(PyTuple_GetItem(args, 1));
    GP211_SECURITY_INFO stGP211SecurityInfo = *(GP211_SECURITY_INFO *)PyString_AsString(PyTuple_GetItem(args, 2));
    PyObject* pobjCardElement = PyTuple_GetItem(args, 3);
    PyObject* pobjAID = PyTuple_GetItem(args, 4);
    PyObject* pobjLiftCycleState= PyTuple_GetItem(args, 5);

    BYTE bCardElement = (BYTE)PyLong_AsLong(pobjCardElement);
    PBYTE pbAID = (PBYTE)PyString_AsString(pobjAID);
    DWORD dwAIDLength = (DWORD)PyString_GET_SIZE(pobjAID);  
    BYTE bLifeCycleState = (BYTE)PyLong_AsLong(pobjLiftCycleState);

    OPGP_ERROR_STATUS errorStatus = GP211_set_status(stCardContext, stCardInfo, &stGP211SecurityInfo, bCardElement, pbAID, dwAIDLength, bLifeCycleState);
    CHECK_GP_CALL_RESULT(errorStatus);
    
    return PyLong_FromLong(0);
}

PyObject* pyGP211_mutual_authentication(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(12);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));
    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)PyString_AsString(PyTuple_GetItem(args, 1));
    PBYTE pbBaseKey = NULL;
    PyObject* pobjBaseKey = PyTuple_GetItem(args, 2);
    if (pobjBaseKey != Py_None) {
        pbBaseKey = (PBYTE)PyString_AsString(pobjBaseKey);
    }
    PBYTE pbS_ENC = (PBYTE)PyString_AsString(PyTuple_GetItem(args, 3));
    PBYTE pbS_MAC = (PBYTE)PyString_AsString(PyTuple_GetItem(args, 4));
    PBYTE pbDEK = (PBYTE)PyString_AsString(PyTuple_GetItem(args, 5));
    BYTE bKeySetVersion = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 6));
    BYTE bKeyIndex = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 7));
    BYTE bSecureChannelProtocol = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 8));
    BYTE bSecureChannelProtocolImpl = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 9));
    BYTE bSecurityLevel = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 10));
    BYTE bDerivationMethod = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 11));

    GP211_SECURITY_INFO stGP211SecurityInfo = { 0 };
    OPGP_ERROR_STATUS errorStatus = GP211_mutual_authentication(stCardContext, stCardInfo, pbBaseKey, pbS_ENC, pbS_MAC, pbDEK, bKeySetVersion, bKeyIndex, bSecureChannelProtocol, bSecureChannelProtocolImpl, bSecurityLevel, bDerivationMethod, &stGP211SecurityInfo);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyString_FromStringAndSize((const char *)&stGP211SecurityInfo, sizeof(GP211_SECURITY_INFO));
}

PyObject* pyGP211_init_implicit_secure_channel(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(7);

    PyObject* pobjAID = PyTuple_GetItem(args, 0);
    PBYTE pbAID = (PBYTE)PyString_AsString(pobjAID);
    DWORD dwAIDLength = (DWORD)PyString_GET_SIZE(pobjAID);
    PBYTE pbBaseKey = (PBYTE)PyString_AsString(PyTuple_GetItem(args, 1));
    PBYTE pbS_ENC = (PBYTE)PyString_AsString(PyTuple_GetItem(args, 2));
    PBYTE pbS_MAC = (PBYTE)PyString_AsString(PyTuple_GetItem(args, 3));
    PBYTE pbDEK = (PBYTE)PyString_AsString(PyTuple_GetItem(args, 4));
    BYTE bSecureChannelProtocolImpl = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 5));
    PBYTE pbSequenceCounter = (PBYTE)PyString_AsString(PyTuple_GetItem(args, 6));

    GP211_SECURITY_INFO stGP211SecurityInfo = { 0 };
    OPGP_ERROR_STATUS errorStatus = GP211_init_implicit_secure_channel(pbAID, dwAIDLength, pbBaseKey, pbS_ENC, pbS_MAC, pbDEK, bSecureChannelProtocolImpl, pbSequenceCounter, &stGP211SecurityInfo);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyString_FromStringAndSize((const char *)&stGP211SecurityInfo, sizeof(GP211_SECURITY_INFO));
}

PyObject* pyGP211_close_implicit_secure_channel(PyObject* self, PyObject* args)
{
    //OPGP_ERROR_STATUS errorStatus = close_implicit_secure_channel(&stGP211SecurityInfo);
    //if (errorStatus.errorStatus != OPGP_ERROR_STATUS_SUCCESS) {
    //    return PyLong_FromLong(-1);
    //}

    return PyLong_FromLong(0);
}

PyObject* pyGP211_get_data(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(4);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));
    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)PyString_AsString(PyTuple_GetItem(args, 1));
    GP211_SECURITY_INFO stGP211SecurityInfo = *(GP211_SECURITY_INFO *)PyString_AsString(PyTuple_GetItem(args, 2));
    PyObject* pobjIdentifier = PyTuple_GetItem(args, 3);
    BYTE baIdentifier[2] = { 0 };
    if (PyString_GET_SIZE(pobjIdentifier) < 2) {
        baIdentifier[1] = PyString_AsString(pobjIdentifier)[0];
    } else {
        memcpy(baIdentifier, PyString_AsString(pobjIdentifier), 2);
//        memcpy_s(baIdentifier, 2, PyString_AsString(pobjIdentifier), 2);
    }
    BYTE baRecvBuffer[0x100] = { 0 };
    DWORD dwRecvBufferLength = sizeof(baRecvBuffer);

    OPGP_ERROR_STATUS errorStatus = GP211_get_data(stCardContext, stCardInfo, &stGP211SecurityInfo, baIdentifier, baRecvBuffer, &dwRecvBufferLength);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyString_FromStringAndSize((const char *)baRecvBuffer, dwRecvBufferLength);
}

PyObject* pyGP211_get_data_iso7816_4(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(3);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));
    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)PyString_AsString(PyTuple_GetItem(args, 1));
    PBYTE pbIdentifier = (PBYTE)PyString_AsString(PyTuple_GetItem(args, 2));

    BYTE baRecvBuffer[0x100] = { 0 };
    DWORD dwRecvBufferLength = sizeof(baRecvBuffer) / sizeof(BYTE);
    OPGP_ERROR_STATUS errorStatus = GP211_get_data_iso7816_4(stCardContext, stCardInfo, pbIdentifier, baRecvBuffer, &dwRecvBufferLength);
    CHECK_GP_CALL_RESULT(errorStatus);

    PyObject* pobjRet = PyString_FromStringAndSize((const char *)baRecvBuffer, dwRecvBufferLength);

    return pobjRet;
}

PyObject* pyGP211_get_secure_channel_protocol_details(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(2);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));
    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)PyString_AsString(PyTuple_GetItem(args, 1));

    BYTE bSecureChannelProtocol;
    BYTE bSecureChannelProtocolImpl;

    OPGP_ERROR_STATUS errorStatus = GP211_get_secure_channel_protocol_details(stCardContext, stCardInfo, &bSecureChannelProtocol, &bSecureChannelProtocolImpl);
    CHECK_GP_CALL_RESULT(errorStatus);

    PyObject* pobjRet = PyTuple_New(2);
    PyTuple_SetItem(pobjRet, 0, PyLong_FromLong(bSecureChannelProtocol));
    PyTuple_SetItem(pobjRet, 1, PyLong_FromLong(bSecureChannelProtocolImpl));
    return pobjRet;
}

PyObject* pyGP211_get_sequence_counter(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(2);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));
    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)PyString_AsString(PyTuple_GetItem(args, 1));

    BYTE baSequenceCounter[2] = { 0 };
    OPGP_ERROR_STATUS errorStatus = GP211_get_sequence_counter(stCardContext, stCardInfo, baSequenceCounter);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong((baSequenceCounter[1] << 8) | baSequenceCounter[0]);
}

PyObject* pyGP211_put_data(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(5);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));
    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)PyString_AsString(PyTuple_GetItem(args, 1));
    GP211_SECURITY_INFO stGP211SecurityInfo = *(GP211_SECURITY_INFO *)PyString_AsString(PyTuple_GetItem(args, 2));
    PBYTE pbIdentifier = (PBYTE)PyString_AsString(PyTuple_GetItem(args, 3));
    PyObject* pobjDataObject = PyTuple_GetItem(args, 4);
    PBYTE pbDataObject = (PBYTE)PyString_AsString(pobjDataObject);
    DWORD dwDataObjectLength = (DWORD)PyString_GET_SIZE(pobjDataObject);

    OPGP_ERROR_STATUS errorStatus = GP211_put_data(stCardContext, stCardInfo, &stGP211SecurityInfo, pbIdentifier, pbDataObject, dwDataObjectLength);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* pyGP211_pin_change(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(5);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));
    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)PyString_AsString(PyTuple_GetItem(args, 1));
    GP211_SECURITY_INFO stGP211SecurityInfo = *(GP211_SECURITY_INFO *)PyString_AsString(PyTuple_GetItem(args, 2));
    BYTE bTryLimit = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 3));
    PyObject* pobjPIN = PyTuple_GetItem(args, 4);
    PBYTE pbNewPIN = (PBYTE)PyString_AsString(pobjPIN);
    DWORD dwNewPINLength = (DWORD)PyString_GET_SIZE(pobjPIN);

    OPGP_ERROR_STATUS errorStatus = GP211_pin_change(stCardContext, stCardInfo, &stGP211SecurityInfo, bTryLimit, pbNewPIN, dwNewPINLength);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* pyGP211_put_3des_key(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(7);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));
    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)PyString_AsString(PyTuple_GetItem(args, 1));
    GP211_SECURITY_INFO stGP211SecurityInfo = *(GP211_SECURITY_INFO *)PyString_AsString(PyTuple_GetItem(args, 2));
    BYTE bKeySetVersion = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 3));
    BYTE bKeyIndex = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 4));
    BYTE bNewKeySetVersion = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 5));
    PBYTE pb3DESKey = (PBYTE)PyString_AsString(PyTuple_GetItem(args, 6));

    OPGP_ERROR_STATUS errorStatus = GP211_put_3des_key(stCardContext, stCardInfo, &stGP211SecurityInfo, bKeySetVersion, bKeyIndex, bNewKeySetVersion, pb3DESKey);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* pyGP211_put_rsa_key(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(8);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));
    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)PyString_AsString(PyTuple_GetItem(args, 1));
    GP211_SECURITY_INFO stGP211SecurityInfo = *(GP211_SECURITY_INFO *)PyString_AsString(PyTuple_GetItem(args, 2));
    BYTE bKeySetVersion = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 3));
    BYTE bKeyIndex = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 4));
    BYTE bNewKeySetVersion = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 5));
    PyObject* pobjPEMKeyFileName = PyTuple_GetItem(args, 6);
#ifdef UNICODE
    TCHAR *ptcPEMKeyFileName = PyUnicode_AsUnicode(PyUnicode_FromString(PyString_AsString(pobjPEMKeyFileName)));
#else
    TCHAR *ptcPEMKeyFileName = PyString_AsString(pobjPEMKeyFileName);
#endif
    char* pcPassPhrase = PyString_AsString(PyTuple_GetItem(args, 7));

    OPGP_ERROR_STATUS errorStatus = GP211_put_rsa_key(stCardContext, stCardInfo, &stGP211SecurityInfo, bKeySetVersion, bKeyIndex, bNewKeySetVersion, (OPGP_STRING)ptcPEMKeyFileName, pcPassPhrase);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* pyGP211_put_secure_channel_keys(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(9);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));
    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)PyString_AsString(PyTuple_GetItem(args, 1));
    GP211_SECURITY_INFO stGP211SecurityInfo = *(GP211_SECURITY_INFO *)PyString_AsString(PyTuple_GetItem(args, 2));
    BYTE bKeySetVersion = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 3));
    BYTE bNewKeySetVersion = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 4));
    PBYTE pbNewBaseKey = NULL;
    PyObject* pobjNewBaseKey = PyTuple_GetItem(args, 5);
    if (pobjNewBaseKey != Py_None) {
        pbNewBaseKey = (PBYTE)PyString_AsString(pobjNewBaseKey);
    }
    PBYTE pbNewS_ENC = (PBYTE)PyString_AsString(PyTuple_GetItem(args, 6));
    PBYTE pbNewS_MAC = (PBYTE)PyString_AsString(PyTuple_GetItem(args, 7));
    PBYTE pbNewDEK = (PBYTE)PyString_AsString(PyTuple_GetItem(args, 8));

    OPGP_ERROR_STATUS errorStatus = GP211_put_secure_channel_keys(stCardContext, stCardInfo, &stGP211SecurityInfo, bKeySetVersion, bNewKeySetVersion, pbNewBaseKey, pbNewS_ENC, pbNewS_MAC, pbNewDEK);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* pyGP211_delete_key(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(5);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));
    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)PyString_AsString(PyTuple_GetItem(args, 1));
    GP211_SECURITY_INFO stGP211SecurityInfo = *(GP211_SECURITY_INFO *)PyString_AsString(PyTuple_GetItem(args, 2));
    BYTE bKeySetVersion = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 3));
    BYTE bKeyIndex = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 4));

    OPGP_ERROR_STATUS errorStatus = GP211_delete_key(stCardContext, stCardInfo, &stGP211SecurityInfo, bKeySetVersion, bKeyIndex);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* pyGP211_get_key_information_templates(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(4);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));
    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)PyString_AsString(PyTuple_GetItem(args, 1));
    GP211_SECURITY_INFO *pstGP211SecurityInfo = NULL;
    PyObject* pobjSecurityInfo = PyTuple_GetItem(args, 2);
    if (pobjSecurityInfo != Py_None) {
        pstGP211SecurityInfo = (GP211_SECURITY_INFO *)PyString_AsString(pobjSecurityInfo);
    }
    BYTE  bKeyInformationTemplate = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 3));
    GP211_KEY_INFORMATION pstKeyInformation[0x100];
    DWORD dwKeyInformationCount = sizeof(pstKeyInformation) / sizeof(GP211_KEY_INFORMATION);

    OPGP_ERROR_STATUS errorStatus = GP211_get_key_information_templates(stCardContext, stCardInfo, pstGP211SecurityInfo, bKeyInformationTemplate, pstKeyInformation, &dwKeyInformationCount);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyString_FromStringAndSize((const char *)&pstKeyInformation, dwKeyInformationCount * sizeof(GP211_KEY_INFORMATION));
}

PyObject* pyGP211_delete_application(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(4);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));
    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)PyString_AsString(PyTuple_GetItem(args, 1));
    GP211_SECURITY_INFO stGP211SecurityInfo = *(GP211_SECURITY_INFO *)PyString_AsString(PyTuple_GetItem(args, 2));
    PyObject* pobjAIDs = PyTuple_GetItem(args, 3);

    DWORD dwAIDCount = PyTuple_GET_SIZE(pobjAIDs);
    if (dwAIDCount == 0) {
        return PyLong_FromLong(0);
    }

    OPGP_AID staAIDs[NUM_APPLICATIONS];
    for (DWORD dw = 0; dw < dwAIDCount; ++dw) {
        PyObject* pobjAID = PyTuple_GetItem(pobjAIDs, dw);
        BYTE bAidLength = (BYTE)PyString_GET_SIZE(pobjAID);
        staAIDs[dw].AIDLength = bAidLength;
//        memcpy_s(staAIDs[dw].AID, 16, PyString_AsString(pobjAID), bAidLength);
        memcpy(staAIDs[dw].AID, PyString_AsString(pobjAID), bAidLength);
    }

    GP211_RECEIPT_DATA bstReceiptData[NUM_APPLICATIONS] = { 0 };
    DWORD dwReceiptDataCount = NUM_APPLICATIONS;
    OPGP_ERROR_STATUS errorStatus = GP211_delete_application(stCardContext, stCardInfo, &stGP211SecurityInfo, staAIDs, dwAIDCount, bstReceiptData, &dwReceiptDataCount);
    //CHECK_GP_CALL_RESULT(errorStatus);
    return PyLong_FromLong(-1);

    // TODO: Test this result; How to return the receipt data?
    return PyString_FromStringAndSize((char *)&bstReceiptData, sizeof(GP211_RECEIPT_DATA) * dwReceiptDataCount);
}

PyObject* pyGP211_install_for_load(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(10);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));
    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)PyString_AsString(PyTuple_GetItem(args, 1));
    GP211_SECURITY_INFO stGP211SecurityInfo = *(GP211_SECURITY_INFO *)PyString_AsString(PyTuple_GetItem(args, 2));
    PyObject* pobjExecutableLoadFileAID = PyTuple_GetItem(args, 3);
    PBYTE pbExecutableLoadFileAID = (PBYTE)PyString_AsString(pobjExecutableLoadFileAID);
    DWORD dwExecutableLoadFileAIDLength = (DWORD)PyString_GET_SIZE(pobjExecutableLoadFileAID);
    PyObject* pobjSecurityDomainAID = PyTuple_GetItem(args, 4);
    PBYTE pbSecurityDomainAID = (PBYTE)PyString_AsString(pobjSecurityDomainAID);
    DWORD dwSecurityDomainAIDLength = (DWORD)PyString_GET_SIZE(pobjSecurityDomainAID);
    PyObject* pobjLoadFileDataBlockHash = PyTuple_GetItem(args, 5);
    PBYTE pbLoadFileDataBlockHash = NULL;
    if (PyString_GET_SIZE(pobjLoadFileDataBlockHash) > 0) {
        pbLoadFileDataBlockHash = (PBYTE)PyString_AsString(pobjLoadFileDataBlockHash);
    }
    PyObject* pobjLoadToken = PyTuple_GetItem(args, 6);
    PBYTE pbLoadToken = NULL;
    if (PyString_GET_SIZE(pobjLoadToken) > 0) {
        pbLoadToken = (PBYTE)PyString_AsString(pobjLoadToken);
    }
    DWORD dwNonVolatileCodeSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 7));
    DWORD dwVolatileDataSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 8));
    DWORD dwNonVolatileDataSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 9));

    OPGP_ERROR_STATUS errorStatus = GP211_install_for_load(stCardContext, stCardInfo, &stGP211SecurityInfo, pbExecutableLoadFileAID, dwExecutableLoadFileAIDLength, pbSecurityDomainAID, dwSecurityDomainAIDLength, pbLoadFileDataBlockHash, pbLoadToken, dwNonVolatileCodeSpaceLimit, dwVolatileDataSpaceLimit, dwNonVolatileDataSpaceLimit);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* pyGP211_get_extradition_token_signature_data(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(3);

    PyObject* pobjSecurityDomainAID = PyTuple_GetItem(args, 0);
    PBYTE pbSecurityDomainAID = (PBYTE)PyString_AsString(pobjSecurityDomainAID);
    DWORD dwSecurityDomainAIDLength = (DWORD)PyString_GET_SIZE(pobjSecurityDomainAID);
    PyObject* pobjApplicationAID = PyTuple_GetItem(args, 1);
    PBYTE pbApplicationAID = (PBYTE)PyString_AsString(pobjApplicationAID);
    DWORD dwApplicationAIDLength = (DWORD)PyString_GET_SIZE(pobjApplicationAID);
    PyObject* pobjExtraditionTokenSignatureData = PyTuple_GetItem(args, 2);
    BYTE baExtraditionTokenSignatureData[0x100];
    DWORD dwExtraditionTokenSignatureDataLength = sizeof(baExtraditionTokenSignatureData) / sizeof(BYTE);

    OPGP_ERROR_STATUS errorStatus = GP211_get_extradition_token_signature_data(pbSecurityDomainAID, dwSecurityDomainAIDLength, pbApplicationAID, dwApplicationAIDLength, baExtraditionTokenSignatureData, &dwExtraditionTokenSignatureDataLength);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyString_FromStringAndSize((const char *)baExtraditionTokenSignatureData, dwExtraditionTokenSignatureDataLength);
}

PyObject* pyGP211_get_load_token_signature_data(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(6);

    PyObject* pobjExecutableLoadFileAID = PyTuple_GetItem(args, 0);
    PBYTE pbExecutableLoadFileAID = (PBYTE)PyString_AsString(pobjExecutableLoadFileAID);
    DWORD dwExecutableLoadFileAIDLength = PyString_GET_SIZE(pobjExecutableLoadFileAID);
    PyObject* pobjSecurityDomainAID = PyTuple_GetItem(args, 1);
    PBYTE pbSecurityDomainAID = (PBYTE)PyString_AsString(pobjSecurityDomainAID);
    DWORD dwSecurityDomainAIDLength = PyString_GET_SIZE(pobjSecurityDomainAID);
    PBYTE pbLoadFileDataBlockHash = (PBYTE)PyString_AsString(PyTuple_GetItem(args, 2));
    DWORD dwNonVolatileCodeSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 3));
    DWORD dwVolatileDataSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 4));
    DWORD dwNonVolatileDataSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 5));
    BYTE baLoadTokenSignatureData[0x100] = { 0 };
    DWORD dwLoadTokenSignatureDataLength = ARRAY_SIZE(baLoadTokenSignatureData); 

    OPGP_ERROR_STATUS errorStatus = GP211_get_load_token_signature_data(pbExecutableLoadFileAID, dwExecutableLoadFileAIDLength, pbSecurityDomainAID, dwSecurityDomainAIDLength, pbLoadFileDataBlockHash, dwNonVolatileCodeSpaceLimit, dwVolatileDataSpaceLimit, dwNonVolatileDataSpaceLimit, baLoadTokenSignatureData, &dwLoadTokenSignatureDataLength);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyString_FromStringAndSize((const char *)baLoadTokenSignatureData, dwLoadTokenSignatureDataLength);
}

PyObject* pyGP211_get_install_token_signature_data(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(8);

    BYTE bP1 = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 0));
    PyObject* pobjExecutableLoadFileAID = PyTuple_GetItem(args, 1);
    PBYTE  pbExecutableLoadFileAID = (PBYTE)PyString_AsString(pobjExecutableLoadFileAID);
    DWORD  dwExecutableLoadFileAIDLength = PyString_GET_SIZE(pobjExecutableLoadFileAID);
    PyObject* pobjExecutableModuleAID = PyTuple_GetItem(args, 2);
    PBYTE  pbExecutableModuleAID = (PBYTE)PyString_AsString(pobjExecutableModuleAID);
    DWORD  dwExecutableModuleAIDLength = PyString_GET_SIZE(pobjExecutableModuleAID);
    PyObject* pobjApplicationAID = PyTuple_GetItem(args, 3);
    PBYTE  pbApplicationAID = (PBYTE)PyString_AsString(pobjApplicationAID);
    DWORD  dwApplicationAIDLength = PyString_GET_SIZE(pobjApplicationAID);
    BYTE  bApplicationPrivileges = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 4));
    DWORD  dwVolatileDataSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 5));
    DWORD  dwNonVolatileDataSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 6));
    PyObject* pobjInstallParameters = PyTuple_GetItem(args, 7);
    PBYTE  pbInstallParameters = (PBYTE)PyString_AsString(pobjInstallParameters);
    DWORD  dwInstallParametersLength = PyString_GET_SIZE(pobjInstallParameters);
    BYTE baInstallTokenSignatureData[0x100] = { 0 };
    DWORD  dwInstallTokenSignatureDataLength = sizeof(baInstallTokenSignatureData);

    OPGP_ERROR_STATUS errorStatus = GP211_get_install_token_signature_data(bP1, pbExecutableLoadFileAID, dwExecutableLoadFileAIDLength, pbExecutableModuleAID, dwExecutableModuleAIDLength, pbApplicationAID, dwApplicationAIDLength, bApplicationPrivileges, dwVolatileDataSpaceLimit, dwNonVolatileDataSpaceLimit, pbInstallParameters, dwInstallParametersLength, baInstallTokenSignatureData, &dwInstallTokenSignatureDataLength);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyString_FromStringAndSize((const char *)&baInstallTokenSignatureData, dwInstallTokenSignatureDataLength);
}

PyObject* pyGP211_calculate_load_token(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(8);

    PyObject* pobjExecutableLoadFileAID = PyTuple_GetItem(args, 0);
    PBYTE pbExecutableLoadFileAID = (PBYTE)PyString_AsString(pobjExecutableLoadFileAID);
    DWORD dwExecutableLoadFileAIDLength = (DWORD)PyString_GET_SIZE(pobjExecutableLoadFileAID);
    PyObject* pobjSecurityDomainAID = PyTuple_GetItem(args, 1);
    PBYTE pbSecurityDomainAID = (PBYTE)PyString_AsString(pobjSecurityDomainAID);
    DWORD dwSecurityDomainAIDLength = (DWORD)PyString_GET_SIZE(pobjSecurityDomainAID);
    PBYTE pbLoadFileDataBlockHash = (PBYTE)PyString_AsString(PyTuple_GetItem(args, 2));
    DWORD dwNonVolatileCodeSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 3));
    DWORD dwVolatileDataSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 4));
    DWORD dwNonVolatileDataSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 5));
    BYTE baLoadToken[128] = { 0 };
    PyObject* pobjPEMKeyFileName = PyTuple_GetItem(args, 6);
#ifdef UNICODE
    // String to const wchar_t *;
    TCHAR *ptcPEMKeyFileName = PyUnicode_AsUnicode(PyUnicode_FromString(PyString_AsString(pobjPEMKeyFileName)));
#else
    // String to const char *;
    TCHAR *ptcPEMKeyFileName = PyString_AsString(pobjPEMKeyFileName);
#endif
    char* pcPassPhrase = PyString_AsString(PyTuple_GetItem(args, 7));

    OPGP_ERROR_STATUS errorStatus = GP211_calculate_load_token(pbExecutableLoadFileAID, dwExecutableLoadFileAIDLength, pbSecurityDomainAID, dwSecurityDomainAIDLength, pbLoadFileDataBlockHash, dwNonVolatileCodeSpaceLimit, dwVolatileDataSpaceLimit, dwNonVolatileDataSpaceLimit, pbLoadFileDataBlockHash, (OPGP_STRING)ptcPEMKeyFileName, pcPassPhrase);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyString_FromStringAndSize((const char *)&baLoadToken, sizeof(baLoadToken));
}

PyObject* pyGP211_calculate_install_token(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(10);

    BYTE bP1 = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 0));
    PyObject* pobjExecutableLoadFileAID = PyTuple_GetItem(args, 1);
    PBYTE  pbExecutableLoadFileAID = (PBYTE)PyString_AsString(pobjExecutableLoadFileAID);
    DWORD  dwExecutableLoadFileAIDLength = PyString_GET_SIZE(pobjExecutableLoadFileAID);
    PyObject* pobjExecutableModuleAID = PyTuple_GetItem(args, 2);
    PBYTE  pbExecutableModuleAID = (PBYTE)PyString_AsString(pobjExecutableModuleAID);
    DWORD  dwExecutableModuleAIDLength = PyString_GET_SIZE(pobjExecutableModuleAID);
    PyObject* pobjApplicationAID = PyTuple_GetItem(args, 3);
    PBYTE  pbApplicationAID = (PBYTE)PyString_AsString(pobjApplicationAID);
    DWORD  dwApplicationAIDLength = PyString_GET_SIZE(pobjApplicationAID);
    BYTE  bApplicationPrivileges = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 4));
    DWORD  dwVolatileDataSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 5));
    DWORD  dwNonVolatileDataSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 6));
    PyObject* pobjInstallParameters = PyTuple_GetItem(args, 7);
    PBYTE  pbInstallParameters = (PBYTE)PyString_AsString(pobjInstallParameters);
    DWORD  dwInstallParametersLength = PyString_GET_SIZE(pobjInstallParameters);
    BYTE baInstallToken[128] = { 0 };
    PyObject* pobjPEMKeyFileName = PyTuple_GetItem(args, 8);
#ifdef UNICODE
    // String to const wchar_t *;
    TCHAR *ptcPEMKeyFileName = PyUnicode_AsUnicode(PyUnicode_FromString(PyString_AsString(pobjPEMKeyFileName)));
#else
    // String to const char *;
    TCHAR *ptcPEMKeyFileName = PyString_AsString(pobjPEMKeyFileName);
#endif
    char* pcPassPhrase = PyString_AsString(PyTuple_GetItem(args, 9));

    OPGP_ERROR_STATUS errorStatus = GP211_calculate_install_token(bP1, pbExecutableLoadFileAID, dwExecutableLoadFileAIDLength, pbExecutableModuleAID, dwExecutableModuleAIDLength, pbApplicationAID, dwApplicationAIDLength, bApplicationPrivileges, dwVolatileDataSpaceLimit, dwNonVolatileDataSpaceLimit, pbInstallParameters, dwInstallParametersLength, baInstallToken, (OPGP_STRING)ptcPEMKeyFileName, pcPassPhrase);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyString_FromStringAndSize((const char *)&baInstallToken, sizeof(baInstallToken));
}

PyObject* pyGP211_calculate_load_file_data_block_hash(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(1);

    PyObject* pobjPEMKeyFileName = PyTuple_GetItem(args, 0);
#ifdef UNICODE
    // String to const wchar_t *;
    TCHAR *ptcPEMKeyFileName = PyUnicode_AsUnicode(PyUnicode_FromString(PyString_AsString(pobjPEMKeyFileName)));
#else
    // String to const char *;
    TCHAR *ptcPEMKeyFileName = PyString_AsString(pobjPEMKeyFileName);
#endif

    BYTE baHash[20] = { 0 };

    OPGP_ERROR_STATUS errorStatus = GP211_calculate_load_file_data_block_hash((OPGP_STRING)ptcPEMKeyFileName, baHash);
    if (errorStatus.errorStatus != OPGP_ERROR_STATUS_SUCCESS) {
        return PyLong_FromLong(-1);
    }

    return PyString_FromStringAndSize((const char *)&baHash, sizeof(baHash));
}

PyObject* pyGP211_load(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(5);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));
    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)PyString_AsString(PyTuple_GetItem(args, 1));
    GP211_SECURITY_INFO stGP211SecurityInfo = *(GP211_SECURITY_INFO *)PyString_AsString(PyTuple_GetItem(args, 2));
    PyObject* pobjDapBlock = PyTuple_GetItem(args, 3);
    GP211_DAP_BLOCK *pbDapBlock = (GP211_DAP_BLOCK *)PyString_AsString(pobjDapBlock);
    DWORD dwDapBlockLength = PyString_GET_SIZE(pobjDapBlock);

    PyObject* pobjExecutableLoadFileName = PyTuple_GetItem(args, 4);
    TCHAR *ptcExecutableLoadFileName = NULL;
#ifdef UNICODE
    ptcExecutableLoadFileName = PyUnicode_AsUnicode(PyUnicode_FromString(PyString_AsString(pobjExecutableLoadFileName)));
#else
    ptcExecutableLoadFileName = PyString_AsString(pobjExecutableLoadFileName);
#endif

    GP211_RECEIPT_DATA stReceiptData;
    DWORD dwReceiptDataAvailable = 0;

    OPGP_ERROR_STATUS errorStatus = GP211_load(stCardContext, stCardInfo, &stGP211SecurityInfo, pbDapBlock, dwDapBlockLength, (OPGP_STRING)ptcExecutableLoadFileName, &stReceiptData, &dwReceiptDataAvailable, NULL);
    CHECK_GP_CALL_RESULT(errorStatus);

    if (0 == dwReceiptDataAvailable) {
        return PyLong_FromLong(0);
    }

    return PyString_FromStringAndSize((const char *)&stReceiptData, sizeof(GP211_RECEIPT_DATA));
}

PyObject* pyGP211_load_from_buffer(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(5);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));
    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)PyString_AsString(PyTuple_GetItem(args, 1));
    GP211_SECURITY_INFO stGP211SecurityInfo = *(GP211_SECURITY_INFO *)PyString_AsString(PyTuple_GetItem(args, 2));
    PyObject* pobjDapBlock = PyTuple_GetItem(args, 3);
    GP211_DAP_BLOCK* pbDapBlock = (GP211_DAP_BLOCK *)PyString_AsString(pobjDapBlock);
    DWORD dwDapBlockLength = PyString_GET_SIZE(pobjDapBlock);
    PyObject* pobjLoadFileBuf = PyTuple_GetItem(args, 4);
    PBYTE pbLoadFileBuf = (PBYTE)PyString_AsString(pobjLoadFileBuf);
    DWORD dwLoadFileBufSize = PyString_GET_SIZE(pobjLoadFileBuf);
    GP211_RECEIPT_DATA stReceiptData;
    DWORD dwReceiptDataAvailable = 0;

    OPGP_ERROR_STATUS errorStatus = GP211_load_from_buffer(stCardContext, stCardInfo, &stGP211SecurityInfo, pbDapBlock, dwDapBlockLength, pbLoadFileBuf, dwLoadFileBufSize, &stReceiptData, &dwReceiptDataAvailable, NULL);
    CHECK_GP_CALL_RESULT(errorStatus);

    if (0 == dwReceiptDataAvailable) {
        return PyLong_FromLong(0);
    }

    return PyString_FromStringAndSize((const char *)&stReceiptData, sizeof(GP211_RECEIPT_DATA));
}

PyObject* pyGP211_install_for_install(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(11);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));
    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)PyString_AsString(PyTuple_GetItem(args, 1));
    GP211_SECURITY_INFO stGP211SecurityInfo = *(GP211_SECURITY_INFO *)PyString_AsString(PyTuple_GetItem(args, 2));
    PyObject* pobjExecutableLoadFileAID = PyTuple_GetItem(args, 3);
    PBYTE  pbExecutableLoadFileAID = (PBYTE)PyString_AsString(pobjExecutableLoadFileAID);
    DWORD  dwExecutableLoadFileAIDLength = PyString_GET_SIZE(pobjExecutableLoadFileAID);
    PyObject* pobjExecutableModuleAID = PyTuple_GetItem(args, 4);
    PBYTE  pbExecutableModuleAID = (PBYTE)PyString_AsString(pobjExecutableModuleAID);
    DWORD  dwExecutableModuleAIDLength = PyString_GET_SIZE(pobjExecutableModuleAID);
    PyObject* pobjApplicationAID = PyTuple_GetItem(args, 5);
    PBYTE  pbApplicationAID = (PBYTE)PyString_AsString(pobjApplicationAID);
    DWORD  dwApplicationAIDLength = PyString_GET_SIZE(pobjApplicationAID);
    BYTE  bApplicationPrivileges = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 6));
    DWORD  dwVolatileDataSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 7));
    DWORD  dwNonVolatileDataSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 8));
    PyObject* pobjInstallParameters = PyTuple_GetItem(args, 9);
    PBYTE  pbInstallParameters = (PBYTE)PyString_AsString(pobjInstallParameters);
    DWORD  dwInstallParametersLength = PyString_GET_SIZE(pobjInstallParameters);
    PyObject* pobjInstallToken = PyTuple_GetItem(args, 10);
    PBYTE pbInstallToken = NULL;
    if (PyString_GET_SIZE(pobjInstallToken) > 0) {
        pbInstallToken = (PBYTE)PyString_AsString(pobjInstallToken);
    }
    GP211_RECEIPT_DATA stReceiptData;
    DWORD dwReceiptDataAvailable = 0;

    OPGP_ERROR_STATUS errorStatus = GP211_install_for_install(stCardContext, stCardInfo, &stGP211SecurityInfo, pbExecutableLoadFileAID, dwExecutableLoadFileAIDLength, pbExecutableModuleAID, dwExecutableModuleAIDLength, pbApplicationAID, dwApplicationAIDLength, bApplicationPrivileges, dwVolatileDataSpaceLimit, dwNonVolatileDataSpaceLimit, pbInstallParameters, dwInstallParametersLength, pbInstallToken, &stReceiptData, &dwReceiptDataAvailable);
    CHECK_GP_CALL_RESULT(errorStatus);

    if (0 == dwReceiptDataAvailable) {
        return PyLong_FromLong(0);
    }

    return PyString_FromStringAndSize((const char *)&stReceiptData, sizeof(GP211_RECEIPT_DATA));
}

PyObject* pyGP211_install_for_make_selectable(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(6);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));
    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)PyString_AsString(PyTuple_GetItem(args, 1));
    GP211_SECURITY_INFO stGP211SecurityInfo = *(GP211_SECURITY_INFO *)PyString_AsString(PyTuple_GetItem(args, 2));
    PyObject* pobjApplicationAID = PyTuple_GetItem(args, 3);
    PBYTE  pbApplicationAID = (PBYTE)PyString_AsString(pobjApplicationAID);
    DWORD  dwApplicationAIDLength = PyString_GET_SIZE(pobjApplicationAID);
    BYTE  bApplicationPrivileges = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 4));
    PyObject* pobjInstallToken = PyTuple_GetItem(args, 5);
    PBYTE pbInstallToken = NULL;
    if (PyString_GET_SIZE(pobjInstallToken) > 0) {
        pbInstallToken = (PBYTE)PyString_AsString(pobjInstallToken);
    }
    GP211_RECEIPT_DATA stReceiptData;
    DWORD dwReceiptDataAvailable = 0;

    OPGP_ERROR_STATUS errorStatus = GP211_install_for_make_selectable(stCardContext, stCardInfo, &stGP211SecurityInfo, pbApplicationAID, dwApplicationAIDLength, bApplicationPrivileges, pbInstallToken, &stReceiptData, &dwReceiptDataAvailable);
    CHECK_GP_CALL_RESULT(errorStatus);

    if (0 == dwReceiptDataAvailable) {
        return PyLong_FromLong(0);
    }

    return PyString_FromStringAndSize((const char *)&stReceiptData, sizeof(GP211_RECEIPT_DATA));
}

PyObject* pyGP211_install_for_install_and_make_selectable(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(11);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));
    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)PyString_AsString(PyTuple_GetItem(args, 1));
    GP211_SECURITY_INFO stGP211SecurityInfo = *(GP211_SECURITY_INFO *)PyString_AsString(PyTuple_GetItem(args, 2));
    PyObject* pobjExecutableLoadFileAID = PyTuple_GetItem(args, 3);
    PBYTE  pbExecutableLoadFileAID = (PBYTE)PyString_AsString(pobjExecutableLoadFileAID);
    DWORD  dwExecutableLoadFileAIDLength = PyString_GET_SIZE(pobjExecutableLoadFileAID);
    PyObject* pobjExecutableModuleAID = PyTuple_GetItem(args, 4);
    PBYTE  pbExecutableModuleAID = (PBYTE)PyString_AsString(pobjExecutableModuleAID);
    DWORD  dwExecutableModuleAIDLength = PyString_GET_SIZE(pobjExecutableModuleAID);
    PyObject* pobjApplicationAID = PyTuple_GetItem(args, 5);
    PBYTE  pbApplicationAID = (PBYTE)PyString_AsString(pobjApplicationAID);
    DWORD  dwApplicationAIDLength = PyString_GET_SIZE(pobjApplicationAID);
    BYTE  bApplicationPrivileges = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 6));
    DWORD  dwVolatileDataSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 7));
    DWORD  dwNonVolatileDataSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 8));
    PyObject* pobjInstallParameters = PyTuple_GetItem(args, 9);
    PBYTE  pbInstallParameters = (PBYTE)PyString_AsString(pobjInstallParameters);
    DWORD  dwInstallParametersLength = PyString_GET_SIZE(pobjInstallParameters);
    PyObject* pobjInstallToken = PyTuple_GetItem(args, 10);
    PBYTE pbInstallToken = NULL;
    if (PyString_GET_SIZE(pobjInstallToken) > 0) {
        pbInstallToken = (PBYTE)PyString_AsString(pobjInstallToken);
    }
    GP211_RECEIPT_DATA stReceiptData;
    DWORD dwReceiptDataAvailable = 0;

    OPGP_ERROR_STATUS errorStatus = GP211_install_for_install_and_make_selectable(stCardContext, stCardInfo, &stGP211SecurityInfo, pbExecutableLoadFileAID, dwExecutableLoadFileAIDLength, pbExecutableModuleAID, dwExecutableModuleAIDLength, pbApplicationAID, dwApplicationAIDLength, bApplicationPrivileges, dwVolatileDataSpaceLimit, dwNonVolatileDataSpaceLimit, pbInstallParameters, dwInstallParametersLength, pbInstallToken, &stReceiptData, &dwReceiptDataAvailable);
    CHECK_GP_CALL_RESULT(errorStatus);

    if (0 == dwReceiptDataAvailable) {
        return PyLong_FromLong(0);
    }

    return PyString_FromStringAndSize((const char *)&stReceiptData, sizeof(GP211_RECEIPT_DATA));
}

PyObject* pyGP211_install_for_personalization(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(4);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));
    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)PyString_AsString(PyTuple_GetItem(args, 1));
    GP211_SECURITY_INFO stGP211SecurityInfo = *(GP211_SECURITY_INFO *)PyString_AsString(PyTuple_GetItem(args, 2));
    PyObject* pobjApplicationAID = PyTuple_GetItem(args, 3);
    PBYTE  pbApplicationAID = (PBYTE)PyString_AsString(pobjApplicationAID);
    DWORD  dwApplicationAIDLength = PyString_GET_SIZE(pobjApplicationAID);

    OPGP_ERROR_STATUS errorStatus = GP211_install_for_personalization(stCardContext, stCardInfo, &stGP211SecurityInfo, pbApplicationAID, dwApplicationAIDLength);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* pyGP211_install_for_extradition(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(6);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));
    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)PyString_AsString(PyTuple_GetItem(args, 1));
    GP211_SECURITY_INFO stGP211SecurityInfo = *(GP211_SECURITY_INFO *)PyString_AsString(PyTuple_GetItem(args, 2));
    PyObject* pobjSecurityDomainAID = PyTuple_GetItem(args, 3);
    PBYTE pbSecurityDomainAID = (PBYTE)PyString_AsString(pobjSecurityDomainAID);
    DWORD dwSecurityDomainAIDLength = (DWORD)PyString_GET_SIZE(pobjSecurityDomainAID);
    PyObject* pobjApplicationAID = PyTuple_GetItem(args, 4);
    PBYTE pbApplicationAID = (PBYTE)PyString_AsString(pobjApplicationAID);
    DWORD dwApplicationAID = (DWORD)PyString_GET_SIZE(pobjApplicationAID);
    PBYTE pbExtrationToken = (PBYTE)PyString_AsString(PyTuple_GetItem(args, 5));
    GP211_RECEIPT_DATA stReceiptData;
    DWORD dwReceiptDataAvailable = 0;

    OPGP_ERROR_STATUS errorStatus = GP211_install_for_extradition(stCardContext, stCardInfo, &stGP211SecurityInfo, pbSecurityDomainAID, dwSecurityDomainAIDLength, pbApplicationAID, dwApplicationAID, pbExtrationToken, &stReceiptData, &dwReceiptDataAvailable);
    CHECK_GP_CALL_RESULT(errorStatus);

    if (0 == dwReceiptDataAvailable) {
        return PyLong_FromLong(0);
    }

    return PyString_FromStringAndSize((const char *)&stReceiptData, sizeof(GP211_RECEIPT_DATA));
}

PyObject* pyGP211_put_delegated_management_keys(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(8);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));
    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)PyString_AsString(PyTuple_GetItem(args, 1));
    GP211_SECURITY_INFO stGP211SecurityInfo = *(GP211_SECURITY_INFO *)PyString_AsString(PyTuple_GetItem(args, 2));
    BYTE bKeySetVersion = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 3));
    BYTE bNewKeySetVersion = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 4));
    PyObject* pobjPEMKeyFileName = PyTuple_GetItem(args, 5);
#ifdef UNICODE
    // String to const wchar_t *;
    TCHAR *ptcPEMKeyFileName = PyUnicode_AsUnicode(PyUnicode_FromString(PyString_AsString(pobjPEMKeyFileName)));
#else
    // String to const char *;
    TCHAR *ptcPEMKeyFileName = PyString_AsString(pobjPEMKeyFileName);
#endif
    char* pcPassPhrase = PyString_AsString(PyTuple_GetItem(args, 6));
    PBYTE pcReceiptKey = (PBYTE)PyString_AsString(PyTuple_GetItem(args, 7));

    OPGP_ERROR_STATUS errorStatus = GP211_put_delegated_management_keys(stCardContext, stCardInfo, &stGP211SecurityInfo, bKeySetVersion, bNewKeySetVersion, (OPGP_STRING)ptcPEMKeyFileName, pcPassPhrase, pcReceiptKey);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* pyGP211_send_APDU(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(4);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));
    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)PyString_AsString(PyTuple_GetItem(args, 1));
    PyObject* pobjSecurityInfo = PyTuple_GetItem(args, 2);
    GP211_SECURITY_INFO *pstGP211SecurityInfo = NULL;
    if (pobjSecurityInfo != Py_None) {
        pstGP211SecurityInfo = (GP211_SECURITY_INFO *)PyString_AsString(pobjSecurityInfo);
    }
    PyObject* pobjCApdu = PyTuple_GetItem(args, 3);

    BYTE* pbCApdu = (BYTE *)PyString_AsString(pobjCApdu);
    DWORD dwCApduLength = PyString_GET_SIZE(pobjCApdu);
    BYTE baRApdu[0x100] = { 0 };
    DWORD dwRApduLength = sizeof(baRApdu) / sizeof(BYTE);

    OPGP_ERROR_STATUS errorStatus = GP211_send_APDU(stCardContext, stCardInfo, pstGP211SecurityInfo, pbCApdu, dwCApduLength, baRApdu, &dwRApduLength);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyString_FromStringAndSize((const char *)baRApdu, dwRApduLength);
}

PyObject* pyGP211_calculate_3des_DAP(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(3);

    PBYTE pbLoadFileDataBlockHash = (PBYTE)PyString_AsString(PyTuple_GetItem(args, 0));
    PyObject* pobjSecurityDomainAID = PyTuple_GetItem(args, 1);
    PBYTE pbSecurityDomainAID = (PBYTE)PyString_AsString(pobjSecurityDomainAID);
    DWORD dwSecurityDomainAIDLength = (DWORD)PyString_GET_SIZE(pobjSecurityDomainAID);
    PBYTE pbDAPCalculationKey = (PBYTE)PyString_AsString(PyTuple_GetItem(args, 2));
    GP211_DAP_BLOCK stLoadFileDataBlockSignature;

    OPGP_ERROR_STATUS errorStatus = GP211_calculate_3des_DAP(pbLoadFileDataBlockHash, pbSecurityDomainAID, dwSecurityDomainAIDLength, pbDAPCalculationKey, &stLoadFileDataBlockSignature);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyString_FromStringAndSize((char *)&stLoadFileDataBlockSignature, sizeof(GP211_DAP_BLOCK));
}

PyObject* pyGP211_calculate_rsa_DAP(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(4);

    PBYTE pbLoadFileDataBlockHash = (PBYTE)PyString_AsString(PyTuple_GetItem(args, 0));
    PyObject* pobjSecurityDomainAID = PyTuple_GetItem(args, 1);
    PBYTE pbSecurityDomainAID = (PBYTE)PyString_AsString(pobjSecurityDomainAID);
    DWORD dwSecurityDomainAIDLength = (DWORD)PyString_GET_SIZE(pobjSecurityDomainAID);
    PyObject* pobjPEMKeyFileName = PyTuple_GetItem(args, 2);
#ifdef UNICODE
    // String to const wchar_t *;
    TCHAR *ptcPEMKeyFileName = PyUnicode_AsUnicode(PyUnicode_FromString(PyString_AsString(pobjPEMKeyFileName)));
#else
    // String to const char *;
    TCHAR *ptcPEMKeyFileName = PyString_AsString(pobjPEMKeyFileName);
#endif
    char* pcPassPhrase = PyString_AsString(PyTuple_GetItem(args, 3));
    GP211_DAP_BLOCK stLoadFileDataBlockSignature;

    OPGP_ERROR_STATUS errorStatus = GP211_calculate_rsa_DAP(pbLoadFileDataBlockHash, pbSecurityDomainAID, dwSecurityDomainAIDLength, (OPGP_STRING)ptcPEMKeyFileName, pcPassPhrase, &stLoadFileDataBlockSignature);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyString_FromStringAndSize((char *)&stLoadFileDataBlockSignature, sizeof(GP211_DAP_BLOCK));
}

PyObject* pyGP211_validate_delete_receipt(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(5);

    DWORD dwConfirmationCounter = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 0));
    PyObject* pobjCardUniqueData = PyTuple_GetItem(args, 1);
    PBYTE pbCardUniqueData = (PBYTE)PyString_AsString(pobjCardUniqueData);
    DWORD dwCardUniqueDataLength = (DWORD)PyString_GET_SIZE(pobjCardUniqueData);
    PBYTE pbReceiptKey = (PBYTE)PyString_AsString(PyTuple_GetItem(args, 2));
    GP211_RECEIPT_DATA stReceiptData = *(GP211_RECEIPT_DATA *)PyString_AsString(PyTuple_GetItem(args, 3));
    PyObject* pobjAID = PyTuple_GetItem(args, 4);
    PBYTE pbAID = (PBYTE)PyString_AsString(pobjAID);
    DWORD dwAIDLength = (DWORD)PyString_GET_SIZE(pobjAID);

    OPGP_ERROR_STATUS errorStatus = GP211_validate_delete_receipt(dwConfirmationCounter, pbCardUniqueData, dwCardUniqueDataLength, pbReceiptKey, stReceiptData, pbAID, dwAIDLength);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* pyGP211_validate_install_receipt(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(6);

    DWORD dwConfirmationCounter = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 0));
    PyObject* pobjCardUniqueData = PyTuple_GetItem(args, 1);
    PBYTE pbCardUniqueData = (PBYTE)PyString_AsString(pobjCardUniqueData);
    DWORD dwCardUniqueDataLength = (DWORD)PyString_GET_SIZE(pobjCardUniqueData);
    PBYTE pbReceiptKey = (PBYTE)PyString_AsString(PyTuple_GetItem(args, 2));
    GP211_RECEIPT_DATA stReceiptData = *(GP211_RECEIPT_DATA *)PyString_AsString(PyTuple_GetItem(args, 3));
    PyObject* pobjExecutableLoadFileAID = PyTuple_GetItem(args, 4);
    PBYTE pbExecutableLoadFileAID = (PBYTE)PyString_AsString(pobjExecutableLoadFileAID);
    DWORD dwExecutableLoadFileAIDLength = (DWORD)PyString_GET_SIZE(pobjExecutableLoadFileAID);
    PyObject* pobjApplicationAID = PyTuple_GetItem(args, 5);
    PBYTE pbApplicationAID = (PBYTE)PyString_AsString(pobjApplicationAID);
    DWORD dwApplicationAIDLength = (DWORD)PyString_GET_SIZE(pobjApplicationAID);

    OPGP_ERROR_STATUS errorStatus = GP211_validate_install_receipt(dwConfirmationCounter, pbCardUniqueData, dwCardUniqueDataLength, pbReceiptKey, stReceiptData, pbExecutableLoadFileAID, dwExecutableLoadFileAIDLength, pbApplicationAID, dwApplicationAIDLength);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* pyGP211_validate_load_receipt(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(6);

    DWORD dwConfirmationCounter = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 0));
    PyObject* pobjCardUniqueData = PyTuple_GetItem(args, 1);
    PBYTE pbCardUniqueData = (PBYTE)PyString_AsString(pobjCardUniqueData);
    DWORD dwCardUniqueDataLength = (DWORD)PyString_GET_SIZE(pobjCardUniqueData);
    PBYTE pbReceiptKey = (PBYTE)PyString_AsString(PyTuple_GetItem(args, 2));
    GP211_RECEIPT_DATA stReceiptData = *(GP211_RECEIPT_DATA *)PyString_AsString(PyTuple_GetItem(args, 3));
    PyObject* pobjExecutableLoadFileAID = PyTuple_GetItem(args, 4);
    PBYTE pbExecutableLoadFileAID = (PBYTE)PyString_AsString(pobjExecutableLoadFileAID);
    DWORD dwExecutableLoadFileAIDLength = (DWORD)PyString_GET_SIZE(pobjExecutableLoadFileAID);
    PyObject* pobjSecurityDomainAID = PyTuple_GetItem(args, 5);
    PBYTE pbSecurityDomainAID = (PBYTE)PyString_AsString(pobjSecurityDomainAID);
    DWORD dwSecurityDomainAIDLength = (DWORD)PyString_GET_SIZE(pobjSecurityDomainAID);

    OPGP_ERROR_STATUS errorStatus = GP211_validate_load_receipt(dwConfirmationCounter, pbCardUniqueData, dwCardUniqueDataLength, pbReceiptKey, stReceiptData, pbExecutableLoadFileAID, dwExecutableLoadFileAIDLength, pbSecurityDomainAID, dwSecurityDomainAIDLength);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* pyGP211_validate_extradition_receipt(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
    //DWORD dwConfirmationCounter = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 0));
    //PyObject* pobjCardUniqueData = PyTuple_GetItem(args, 1);
    //PBYTE pbCardUniqueData = (PBYTE)PyString_AsString(pobjCardUniqueData);
    //DWORD dwCardUniqueDataLength = (DWORD)PyString_GET_SIZE(pobjCardUniqueData);
    //PBYTE pbReceiptKey = (PBYTE)PyString_AsString(PyTuple_GetItem(args, 2));
    //GP211_RECEIPT_DATA stReceiptData = *(GP211_RECEIPT_DATA *)PyString_AsString(PyTuple_GetItem(args, 3));
    //PyObject* pobjOldSecurityDomainAID = PyTuple_GetItem(args, 4);
    //PBYTE pbOldSecurityDomainAID = (PBYTE)PyString_AsString(pobjOldSecurityDomainAID);
    //DWORD dwOldSecurityDomainAID = (DWORD)PyString_GET_SIZE(pobjOldSecurityDomainAID);
    //PyObject* pobjNewSecurityDomainAID = PyTuple_GetItem(args, 5);
    //PBYTE pbNewSecurityDomainAID = (PBYTE)PyString_AsString(pobjNewSecurityDomainAID);
    //DWORD dwNewSecurityDomainAID = (DWORD)PyString_GET_SIZE(pobjNewSecurityDomainAID);
    //PyObject* pobjApplicationOrExecutableLoadFileAID = PyTuple_GetItem(args, 6);
    //PBYTE pbApplicationOrExecutableLoadFileAID = (PBYTE)PyString_AsString(pobjApplicationOrExecutableLoadFileAID);
    //DWORD dwApplicationOrExecutableLoadFileAIDLength = (DWORD)PyString_GET_SIZE(pobjApplicationOrExecutableLoadFileAID);

    //OPGP_ERROR_STATUS errorStatus = GP211_validate_extradition_receipt(dwConfirmationCounter, pbCardUniqueData, dwCardUniqueDataLength, pbReceiptKey, stReceiptData, pbOldSecurityDomainAID, dwOldSecurityDomainAID, pbNewSecurityDomainAID, dwNewSecurityDomainAID, pbApplicationOrExecutableLoadFileAID, dwApplicationOrExecutableLoadFileAIDLength);
    //if (errorStatus.errorStatus != OPGP_ERROR_STATUS_SUCCESS) {
    //    return PyLong_FromLong(-1);
    //}

    return PyLong_FromLong(0);
}

PyObject* pyOPGP_manage_channel(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(5);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));
    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)PyString_AsString(PyTuple_GetItem(args, 1));
    GP211_SECURITY_INFO stGP211SecurityInfo = *(GP211_SECURITY_INFO *)PyString_AsString(PyTuple_GetItem(args, 2));
    BYTE bOpenClose = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 3));
    BYTE bChannelNumberToClose = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 4));
    BYTE bChannelNumberOpened = 0;

    OPGP_ERROR_STATUS errorStatus = OPGP_manage_channel(stCardContext, &stCardInfo, &stGP211SecurityInfo, bOpenClose, bChannelNumberToClose, &bChannelNumberOpened);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(bChannelNumberOpened);
}

PyObject* pyOPGP_select_channel(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(2);

    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)PyString_AsString(PyTuple_GetItem(args, 0));
    BYTE bChannelNumber = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 1));

    OPGP_ERROR_STATUS errorStatus = OPGP_select_channel(&stCardInfo, bChannelNumber);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* pyGP211_store_data(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(4);

    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)PyString_AsString(PyTuple_GetItem(args, 0));
    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)PyString_AsString(PyTuple_GetItem(args, 1));
    GP211_SECURITY_INFO stGP211SecurityInfo = *(GP211_SECURITY_INFO *)PyString_AsString(PyTuple_GetItem(args, 2));
    PyObject* pobjData = PyTuple_GetItem(args, 3);
    PBYTE pbData = (PBYTE)PyString_AsString(pobjData);
    DWORD dwDataLength = (DWORD)PyString_GET_SIZE(pobjData);

    OPGP_ERROR_STATUS errorStatus = GP211_store_data(stCardContext, stCardInfo, &stGP211SecurityInfo, pbData, dwDataLength);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

/* Functions for OpenPlatform 201; */

PyObject* pyOP201_get_status(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOP201_set_status(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOP201_mutual_authentication(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOP201_get_data(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOP201_put_data(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOP201_pin_change(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOP201_put_3desKey(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOP201_put_rsa_key(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOP201_put_secure_channel_keys(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOP201_delete_key(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOP201_get_key_information_templates(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOP201_delete_application(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOP201_install_for_load(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOP201_get_load_token_signature_data(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOP201_get_install_token_signature_data(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOP201_calculate_load_token(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOP201_calculate_install_token(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOP201_calculate_load_file_DAP(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOP201_load(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOP201_load_from_buffer(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOP201_install_for_install(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOP201_install_for_make_selectable(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOP201_install_for_install_and_make_selectable(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOP201_put_delegated_management_keys(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOP201_send_APDU(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOP201_calculate_3des_DAP(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOP201_calculate_rsa_DAP(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOP201_validate_delete_receipt(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOP201_validate_install_receipt(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOP201_validate_load_receipt(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyGP211_begin_R_MAC(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyGP211_end_R_MAC(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOPGP_read_executable_load_file_parameters(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(1);

    PyObject* pobjLoadFileName = PyTuple_GetItem(args, 0);
    TCHAR *ptcLoadFileName = NULL;
#ifdef UNICODE
    if (PyUnicode_Check(pobjLoadFileName)) {
        ptcLoadFileName = PyUnicode_AsUnicode(pobjLoadFileName);
    } else {
        ptcLoadFileName = PyUnicode_AsUnicode(PyUnicode_FromString(PyString_AsString(pobjLoadFileName)));
    }
#else
#endif

    OPGP_LOAD_FILE_PARAMETERS stLoadFileParameters = { 0 };
    OPGP_ERROR_STATUS errorStatus = OPGP_read_executable_load_file_parameters(ptcLoadFileName, &stLoadFileParameters);
    CHECK_GP_CALL_RESULT(errorStatus);

    PyObject* pobjRet = PyDict_New();
    PyDict_SetItem(pobjRet, PyString_FromString("loadFileSize"), PyLong_FromLong(stLoadFileParameters.loadFileSize));
    PyDict_SetItem(pobjRet, PyString_FromString("loadFileAID"), PyString_FromStringAndSize((const char *)stLoadFileParameters.loadFileAID.AID, stLoadFileParameters.loadFileAID.AIDLength));
    PyObject* pobjAppletAIDs = PyTuple_New(stLoadFileParameters.numAppletAIDs);
    for (BYTE b = 0; b < stLoadFileParameters.numAppletAIDs; ++b) {
        PyTuple_SetItem(pobjAppletAIDs, b, PyString_FromStringAndSize((const char *)stLoadFileParameters.appletAIDs[b].AID, stLoadFileParameters.appletAIDs[b].AIDLength));
    }
    PyDict_SetItem(pobjRet, PyString_FromString("applets"), pobjAppletAIDs);
    return pobjRet;
}

PyObject* pyOPGP_VISA2_derive_keys(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOPGP_cap_to_ijc(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOPGP_extract_cap_file(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOPGP_read_executable_load_file_parameters_from_buffer(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOPGP_EMV_CPS11_derive_keys(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
}

PyObject* pyOPGP_enable_trace_mode(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(1);

    DWORD dwEnable = PyLong_AsLong(PyTuple_GetItem(args, 0));
    OPGP_enable_trace_mode(dwEnable, NULL);
    return PyLong_FromLong(0);
}

#ifndef WIN32
}
#endif
