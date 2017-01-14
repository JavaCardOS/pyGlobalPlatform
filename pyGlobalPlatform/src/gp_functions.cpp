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

#if (PY_MAJOR_VERSION == 2)
// String => Array;
#define _PyArray_AsString(a)                PyString_AsString(a)
// Array => String;
#define _PyString_FromArrayAndSize(a, s)    PyString_FromStringAndSize(a, s)
// String => Array => size;
#define _PyArray_GetSize(a)                 PyString_Size(a)
// char * => String;
#define _PyString_FromString(s)             PyString_FromString(s)
#else
// String => Array;
#define _PyArray_AsString(a)                PyUnicode_1BYTE_DATA(a)
// Array => String;
#define _PyString_FromArrayAndSize(a, s)    PyUnicode_FromKindAndData(PyUnicode_1BYTE_KIND, a, s)
// String => Array => size;
#define _PyArray_GetSize(a)                 PyUnicode_GetSize(a)
// char * => String;
#define _PyString_FromString(s)             PyUnicode_FromString(s)
#endif


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

#define ARGS_GetBYTEArray(i, T, n) \
    PyObject* pobj##n = PyTuple_GetItem(args, i); \
    T *p##n = NULL; \
    DWORD dw##n##Length = 0; \
    if (pobj##n != Py_None) { \
        p##n = (T *)_PyArray_AsString(pobj##n); \
        dw##n##Length = (DWORD) _PyArray_GetSize(pobj##n); \
    }\
    \

#ifdef UNICODE
    #if (PY_MAJOR_VERSION == 2)
        #define ARGS_GetTCHARArray(i, n) \
            PyObject* pobj##n = PyTuple_GetItem(args, i); \
            TCHAR *p##n = NULL; \
            DWORD dw##n##Length = 0; \
            if (pobj##n != Py_None) { \
                p##n = (TCHAR *)PyUnicode_AsUnicode(pobj##n); \
                if (p##n == NULL) {\
                    pobj##n = PyUnicode_FromString(PyString_AsString(pobj##n));\
                    p##n = (TCHAR *) PyUnicode_AsUnicode(pobj##n); \
                    dw##n##Length = (DWORD) PyUnicode_GetSize(pobj##n); \
                } else { \
                    dw##n##Length = (DWORD)PyUnicode_GetSize(pobj##n); \
                } \
            }\
            \
            
    #else
        #define ARGS_GetTCHARArray(i, n) \
            PyObject* pobj##n = PyTuple_GetItem(args, i); \
            TCHAR *p##n = NULL; \
            DWORD dw##n##Length = 0; \
            if (pobj##n != Py_None) { \
                p##n = (TCHAR *)PyUnicode_AsUnicode(pobj##n); \
                dw##n##Length = (DWORD)PyUnicode_GetSize(pobj##n); \
            }\
            \
            
    #endif
#else
    #define ARGS_GetTCHARArray(i, n) \
        PyObject* pobj##n = PyTuple_GetItem(args, i); \
        TCHAR *p##n = NULL; \
        DWORD dw##n##Length = 0; \
        if (pobj##n != Py_None) { \
            p##n = PyBytes_AsString(PyUnicode_AsASCIIString(pobj##n)); \
            dw##n##Length = (DWORD)PyUnicode_GetSize(pobj##n); \
        }\
        \

#endif

#define ARGS_GetStruct(i, T, n) \
    PyObject *pobj##n = PyTuple_GetItem(args, i); \
    T st##n;\
    T *pst##n = NULL;\
    DWORD dwst##n##Length = 0; \
    if (pobj##n != Py_None) { \
        st##n = *(T *)_PyArray_AsString(pobj##n);\
        pst##n = &st##n;\
        dwst##n##Length = PyUnicode_GetSize(pobj##n); \
    }\
    \

#define ARGS_GetCardContext(i) \
    PyObject *pobjCardContext = PyTuple_GetItem(args, i);\
    if (pobjCardContext == Py_None) {\
        _RaiseError(PyExc_Exception, __FUNCTION__, _T("Card context can not be None!"));\
        return NULL;\
    }\
    OPGP_CARD_CONTEXT stCardContext = *(OPGP_CARD_CONTEXT *)_PyArray_AsString(pobjCardContext);\
    \

#define ARGS_GetCardInfo(i) \
    PyObject *pobjCardInfo = PyTuple_GetItem(args, i);\
    if (pobjCardInfo == Py_None) {\
        _RaiseError(PyExc_Exception, __FUNCTION__, _T("Cardinfo can not be None."));\
        return NULL;\
    }\
    OPGP_CARD_INFO stCardInfo = *(OPGP_CARD_INFO *)_PyArray_AsString(pobjCardInfo);\
    \


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
    _tcsncpy(stCardContext.libraryName, _T("gppcscconnectionplugin"), _tcslen(_T("gppcscconnectionplugin")));
    _tcsncpy(stCardContext.libraryVersion, _T("1"), _tcslen( _T("1")));
    OPGP_ERROR_STATUS errorStatus = OPGP_establish_context(&stCardContext);
    CHECK_GP_CALL_RESULT(errorStatus);

    return _PyString_FromArrayAndSize((const char *)&stCardContext, sizeof(stCardContext));
}

PyObject* releaseContext(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(1);

    ARGS_GetCardContext(0);

    OPGP_ERROR_STATUS errorStatus = OPGP_release_context(&stCardContext);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* listReaders(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(1);

    ARGS_GetCardContext(0);

    TCHAR tcaReaderNames[0x100 * 0x100];
    DWORD dwReaderNamesLength = sizeof(tcaReaderNames) / sizeof(TCHAR);
    OPGP_ERROR_STATUS errorStatus = OPGP_list_readers(stCardContext, (OPGP_STRING)tcaReaderNames, &dwReaderNamesLength);
    CHECK_GP_CALL_RESULT(errorStatus);

    int iReaderCount = 0;
    TCHAR* ptcaReadersName[0x100];
    DWORD dwReaderNameOffset = 0;
    while (true) {
        if (tcaReaderNames[dwReaderNameOffset] == _T('\0')) {
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
        PyObject* pobjReaderName = PyUnicode_FromWideChar((const TCHAR *) ptcaReadersName[i], _tcslen(ptcaReadersName[i]));
#else
        PyObject* pobjReaderName = _PyString_FromArrayAndSize(ptcaReadersName[i], strlen(ptcaReadersName[i]));
#endif
        PyTuple_SetItem(pobjReaderNames, i, pobjReaderName);
    }
    return pobjReaderNames;
}

PyObject* connectCard(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(3);

    ARGS_GetCardContext(0);
    ARGS_GetTCHARArray(1, ReaderName);

    long lProtocol = PyLong_AsLong(PyTuple_GetItem(args, 2));
    if ((!(OPGP_CARD_PROTOCOL_T0 & lProtocol)) && (!(OPGP_CARD_PROTOCOL_T1 & lProtocol))) {
        PyErr_SetFromErrno(PyExc_ValueError);
    }

    OPGP_CARD_INFO stCardInfo = { 0 };
    OPGP_ERROR_STATUS errorStatus = OPGP_card_connect(stCardContext, pReaderName, &stCardInfo, lProtocol);
    CHECK_GP_CALL_RESULT(errorStatus);

    return _PyString_FromArrayAndSize((const char *)&stCardInfo, sizeof(OPGP_CARD_INFO));
}

PyObject* disconnectCard(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(2);

    ARGS_GetCardContext(0);
    ARGS_GetCardInfo(1);

    OPGP_ERROR_STATUS errorStatus = OPGP_card_disconnect(stCardContext, &stCardInfo);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* pyOPGP_select_application(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(3);

    ARGS_GetCardContext(0);
    ARGS_GetCardInfo(1);
    ARGS_GetBYTEArray(2, BYTE, AID);

    OPGP_ERROR_STATUS errorStatus = OPGP_select_application(stCardContext, stCardInfo, pAID, dwAIDLength);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* pyGP211_get_status(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(4);

    ARGS_GetCardContext(0);
    ARGS_GetCardInfo(1);
    ARGS_GetStruct(2, GP211_SECURITY_INFO, SecurityInfo);
    BYTE bCardElement = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 3));

#define NUM_APPLICATIONS 64
    DWORD dwDataCount = NUM_APPLICATIONS;
    GP211_APPLICATION_DATA astAppletData[NUM_APPLICATIONS];
    GP211_EXECUTABLE_MODULES_DATA astExecutableData[NUM_APPLICATIONS];

    OPGP_ERROR_STATUS errorStatus = GP211_get_status(stCardContext, stCardInfo, pstSecurityInfo, bCardElement, astAppletData, astExecutableData, &dwDataCount);
    CHECK_GP_CALL_RESULT(errorStatus);

    if ((bCardElement & 0xE0) != 0) {
        PyObject* pobjAppletData = PyTuple_New(dwDataCount);
        for (DWORD d = 0; d < dwDataCount; ++d) {
            PyObject* pobjOneAppletData = PyDict_New();
            GP211_APPLICATION_DATA* pstOneAppletData = &astAppletData[d];
            PyDict_SetItem(pobjOneAppletData, _PyString_FromString("aid"), _PyString_FromArrayAndSize((const char *)pstOneAppletData->AID, pstOneAppletData->AIDLength));
            PyDict_SetItem(pobjOneAppletData, _PyString_FromString("lifeCycleState"), PyLong_FromLong(pstOneAppletData->lifeCycleState));
            PyDict_SetItem(pobjOneAppletData, _PyString_FromString("privileges"), PyLong_FromLong(pstOneAppletData->privileges));

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
            PyDict_SetItem(pobjOneExecuableData, _PyString_FromString("aid"), _PyString_FromArrayAndSize((const char *) pstOneExecuableModulesData->AID, pstOneExecuableModulesData->AIDLength));
            PyDict_SetItem(pobjOneExecuableData, _PyString_FromString("lifeCycleState"), PyLong_FromLong(pstOneExecuableModulesData->lifeCycleState));

            BYTE bNumExecutableModules = pstOneExecuableModulesData->numExecutableModules;
            PyObject* pobjExecutableModuleData = PyTuple_New(bNumExecutableModules);
            for (BYTE bExecutableModuleIndex = 0; bExecutableModuleIndex < bNumExecutableModules; ++bExecutableModuleIndex) {
                OPGP_AID* pstExecutableModuleAID = &pstOneExecuableModulesData->executableModules[bExecutableModuleIndex];
                PyTuple_SetItem(pobjExecutableModuleData, bExecutableModuleIndex, _PyString_FromArrayAndSize((const char *)pstExecutableModuleAID->AID, pstExecutableModuleAID->AIDLength));
            }
            PyDict_SetItem(pobjOneExecuableData, _PyString_FromString("executableModules"), pobjExecutableModuleData);

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

    ARGS_GetCardContext(0);
    ARGS_GetCardInfo(1);
    ARGS_GetStruct(2, GP211_SECURITY_INFO, SecurityInfo);
    BYTE bCardElement = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 3));
    ARGS_GetBYTEArray(4, BYTE, AID);
    BYTE bLifeCycleState = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 5));

    OPGP_ERROR_STATUS errorStatus = GP211_set_status(stCardContext, stCardInfo, pstSecurityInfo, bCardElement, pAID, dwAIDLength, bLifeCycleState);
    CHECK_GP_CALL_RESULT(errorStatus);
    
    return PyLong_FromLong(0);
}

PyObject* pyGP211_mutual_authentication(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(12);

    ARGS_GetCardContext(0);
    ARGS_GetCardInfo(1);
    ARGS_GetBYTEArray(2, BYTE, BaseKey);
    ARGS_GetBYTEArray(3, BYTE, ENCKey);
    ARGS_GetBYTEArray(4, BYTE, MACKey);
    ARGS_GetBYTEArray(5, BYTE, DEKKey);
    // Get Key informations;
    BYTE bKeySetVersion = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 6));
    BYTE bKeyIndex = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 7));
    // Get SecureChannel informations;
    BYTE bSecureChannelProtocol = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 8));
    BYTE bSecureChannelProtocolImpl = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 9));
    BYTE bSecurityLevel = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 10));
    BYTE bDerivationMethod = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 11));

    GP211_SECURITY_INFO stGP211SecurityInfo = { 0 };
    OPGP_ERROR_STATUS errorStatus = GP211_mutual_authentication(stCardContext, stCardInfo, pBaseKey, pENCKey, pMACKey, pDEKKey, bKeySetVersion, bKeyIndex, bSecureChannelProtocol, bSecureChannelProtocolImpl, bSecurityLevel, bDerivationMethod, &stGP211SecurityInfo);
    CHECK_GP_CALL_RESULT(errorStatus);

    return _PyString_FromArrayAndSize((const char *)&stGP211SecurityInfo, sizeof(GP211_SECURITY_INFO));
}

PyObject* pyGP211_init_implicit_secure_channel(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(7);

    ARGS_GetBYTEArray(0, BYTE, AID);
    ARGS_GetBYTEArray(1, BYTE, BaseKey);
    ARGS_GetBYTEArray(2, BYTE, ENCKey);
    ARGS_GetBYTEArray(3, BYTE, MACKey);
    ARGS_GetBYTEArray(4, BYTE, DEKKey);
    BYTE bSecureChannelProtocolImpl = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 5));
    PBYTE pbSequenceCounter = (PBYTE)_PyArray_AsString(PyTuple_GetItem(args, 6));

    GP211_SECURITY_INFO stGP211SecurityInfo = { 0 };
    OPGP_ERROR_STATUS errorStatus = GP211_init_implicit_secure_channel(pAID, dwAIDLength, pBaseKey, pENCKey, pMACKey, pDEKKey, bSecureChannelProtocolImpl, pbSequenceCounter, &stGP211SecurityInfo);
    CHECK_GP_CALL_RESULT(errorStatus);

    return _PyString_FromArrayAndSize((const char *)&stGP211SecurityInfo, sizeof(GP211_SECURITY_INFO));
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

    ARGS_GetCardContext(0);
    ARGS_GetCardInfo(1);
    ARGS_GetStruct(2, GP211_SECURITY_INFO, SecurityInfo);
    // Get data identifier;
    PyObject* pobjIdentifier = PyTuple_GetItem(args, 3);
    BYTE baIdentifier[2] = { 0 };
    if (pobjIdentifier != Py_None) {
        if (PyUnicode_GetSize(pobjIdentifier) < 2) {
            baIdentifier[1] = _PyArray_AsString(pobjIdentifier)[0];
        }
        else {
            memcpy(baIdentifier, _PyArray_AsString(pobjIdentifier), 2);
        }
    }

    BYTE baRecvBuffer[0x100] = { 0 };
    DWORD dwRecvBufferLength = sizeof(baRecvBuffer);
    OPGP_ERROR_STATUS errorStatus = GP211_get_data(stCardContext, stCardInfo, pstSecurityInfo, baIdentifier, baRecvBuffer, &dwRecvBufferLength);
    CHECK_GP_CALL_RESULT(errorStatus);

    return _PyString_FromArrayAndSize((const char *)baRecvBuffer, dwRecvBufferLength);
}

PyObject* pyGP211_get_data_iso7816_4(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(3);

    ARGS_GetCardContext(0);
    ARGS_GetCardInfo(1);
    ARGS_GetBYTEArray(2, BYTE, Identifier);

    BYTE baRecvBuffer[0x100] = { 0 };
    DWORD dwRecvBufferLength = sizeof(baRecvBuffer) / sizeof(BYTE);
    OPGP_ERROR_STATUS errorStatus = GP211_get_data_iso7816_4(stCardContext, stCardInfo, pIdentifier, baRecvBuffer, &dwRecvBufferLength);
    CHECK_GP_CALL_RESULT(errorStatus);

    PyObject* pobjRet = _PyString_FromArrayAndSize((const char *)baRecvBuffer, dwRecvBufferLength);

    return pobjRet;
}

PyObject* pyGP211_get_secure_channel_protocol_details(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(2);

    ARGS_GetCardContext(0);
    ARGS_GetCardInfo(1);

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

    ARGS_GetCardContext(0);
    ARGS_GetCardInfo(1);

    BYTE baSequenceCounter[2] = { 0 };
    OPGP_ERROR_STATUS errorStatus = GP211_get_sequence_counter(stCardContext, stCardInfo, baSequenceCounter);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong((baSequenceCounter[1] << 8) | baSequenceCounter[0]);
}

PyObject* pyGP211_put_data(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(5);

    ARGS_GetCardContext(0);
    ARGS_GetCardInfo(1);
    ARGS_GetStruct(2, GP211_SECURITY_INFO, SecurityInfo);
    ARGS_GetBYTEArray(3, BYTE, Identifier);
    ARGS_GetBYTEArray(4, BYTE, DataObject);

    OPGP_ERROR_STATUS errorStatus = GP211_put_data(stCardContext, stCardInfo, pstSecurityInfo, pIdentifier, pDataObject, dwDataObjectLength);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* pyGP211_pin_change(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(5);

    ARGS_GetCardContext(0);
    ARGS_GetCardInfo(1);
    ARGS_GetStruct(2, GP211_SECURITY_INFO, SecurityInfo);
    BYTE bTryLimit = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 3));
    ARGS_GetBYTEArray(4, BYTE, NewPIN);

    OPGP_ERROR_STATUS errorStatus = GP211_pin_change(stCardContext, stCardInfo, pstSecurityInfo, bTryLimit, pNewPIN, dwNewPINLength);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* pyGP211_put_3des_key(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(7);

    ARGS_GetCardContext(0);
    ARGS_GetCardInfo(1);
    ARGS_GetStruct(2, GP211_SECURITY_INFO, SecurityInfo);
    BYTE bKeySetVersion = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 3));
    BYTE bKeyIndex = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 4));
    BYTE bNewKeySetVersion = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 5));
    ARGS_GetBYTEArray(6, BYTE, 3DESKey);

    OPGP_ERROR_STATUS errorStatus = GP211_put_3des_key(stCardContext, stCardInfo, pstSecurityInfo, bKeySetVersion, bKeyIndex, bNewKeySetVersion, p3DESKey);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* pyGP211_put_rsa_key(PyObject* self, PyObject* args) {
    CHECK_FUNCTION_ARGUMENTS_COUNT(8);

    ARGS_GetCardContext(0);
    ARGS_GetCardInfo(1);
    ARGS_GetStruct(2, GP211_SECURITY_INFO, SecurityInfo);
    BYTE bKeySetVersion = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 3));
    BYTE bKeyIndex = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 4));
    BYTE bNewKeySetVersion = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 5));
    ARGS_GetTCHARArray(6, PEMKeyFileName);
    ARGS_GetBYTEArray(7, char, PassPhrase);

    OPGP_ERROR_STATUS errorStatus = GP211_put_rsa_key(stCardContext, stCardInfo, pstSecurityInfo, bKeySetVersion, bKeyIndex, bNewKeySetVersion, (OPGP_STRING)pPEMKeyFileName, pPassPhrase);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* pyGP211_put_secure_channel_keys(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(9);

    ARGS_GetCardContext(0);
    ARGS_GetCardInfo(1);
    ARGS_GetStruct(2, GP211_SECURITY_INFO, SecurityInfo);
    BYTE bKeySetVersion = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 3));
    BYTE bNewKeySetVersion = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 4));
    ARGS_GetBYTEArray(5, BYTE, NewBaseKey);
    ARGS_GetBYTEArray(6, BYTE, NewENCKey);
    ARGS_GetBYTEArray(7, BYTE, NewMACKey);
    ARGS_GetBYTEArray(8, BYTE, NewDEKKey);

    OPGP_ERROR_STATUS errorStatus = GP211_put_secure_channel_keys(stCardContext, stCardInfo, pstSecurityInfo, bKeySetVersion, bNewKeySetVersion, pNewBaseKey, pNewENCKey, pNewMACKey, pNewDEKKey);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* pyGP211_delete_key(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(5);

    ARGS_GetCardContext(0);
    ARGS_GetCardInfo(1);
    ARGS_GetStruct(2, GP211_SECURITY_INFO, SecurityInfo);
    BYTE bKeySetVersion = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 3));
    BYTE bKeyIndex = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 4));

    OPGP_ERROR_STATUS errorStatus = GP211_delete_key(stCardContext, stCardInfo, pstSecurityInfo, bKeySetVersion, bKeyIndex);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* pyGP211_get_key_information_templates(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(4);

    ARGS_GetCardContext(0);
    ARGS_GetCardInfo(1);
    ARGS_GetStruct(2, GP211_SECURITY_INFO, SecurityInfo);
    BYTE  bKeyInformationTemplate = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 3));

    GP211_KEY_INFORMATION pstKeyInformation[0x100];
    DWORD dwKeyInformationCount = sizeof(pstKeyInformation) / sizeof(GP211_KEY_INFORMATION);
    OPGP_ERROR_STATUS errorStatus = GP211_get_key_information_templates(stCardContext, stCardInfo, pstSecurityInfo, bKeyInformationTemplate, pstKeyInformation, &dwKeyInformationCount);
    CHECK_GP_CALL_RESULT(errorStatus);

    return _PyString_FromArrayAndSize((const char *)&pstKeyInformation, dwKeyInformationCount * sizeof(GP211_KEY_INFORMATION));
}

PyObject* pyGP211_delete_application(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(4);

    ARGS_GetCardContext(0);
    ARGS_GetCardInfo(1);
    ARGS_GetStruct(2, GP211_SECURITY_INFO, SecurityInfo);

    PyObject* pobjAIDs = PyTuple_GetItem(args, 3);
    DWORD dwAIDCount = PyTuple_GET_SIZE(pobjAIDs);
    if (dwAIDCount == 0) {
        return PyLong_FromLong(0);
    }
    OPGP_AID staAIDs[NUM_APPLICATIONS];
    for (DWORD dw = 0; dw < dwAIDCount; ++dw) {
        PyObject* pobjAID = PyTuple_GetItem(pobjAIDs, dw);;
        if (pobjAID == NULL) {
            pobjAID = PyList_GetItem(pobjAIDs, dw);
        }
        
        BYTE bAidLength = (BYTE)_PyArray_GetSize(pobjAID);
        staAIDs[dw].AIDLength = bAidLength;
        memcpy(staAIDs[dw].AID, _PyArray_AsString(pobjAID), bAidLength);
    }

    GP211_RECEIPT_DATA bstReceiptData[NUM_APPLICATIONS] = { 0 };
    DWORD dwReceiptDataCount = NUM_APPLICATIONS;
    OPGP_ERROR_STATUS errorStatus = GP211_delete_application(stCardContext, stCardInfo, pstSecurityInfo, staAIDs, dwAIDCount, bstReceiptData, &dwReceiptDataCount);
    CHECK_GP_CALL_RESULT(errorStatus);

    // TODO: Test this result; How to return the receipt data?
    return _PyString_FromArrayAndSize((char *)&bstReceiptData, sizeof(GP211_RECEIPT_DATA) * dwReceiptDataCount);
}

PyObject* pyGP211_install_for_load(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(10);

    ARGS_GetCardContext(0);
    ARGS_GetCardInfo(1);
    ARGS_GetStruct(2, GP211_SECURITY_INFO, SecurityInfo);
    ARGS_GetBYTEArray(3, BYTE, ExecutableLoadFileAID);
    ARGS_GetBYTEArray(4, BYTE, SecurityDomainAID);
    ARGS_GetBYTEArray(5, BYTE, LoadFileDataBlockHash);
    ARGS_GetBYTEArray(6, BYTE, LoadToken);
    // Get limits;
    DWORD dwNonVolatileCodeSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 7));
    DWORD dwVolatileDataSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 8));
    DWORD dwNonVolatileDataSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 9));

    OPGP_ERROR_STATUS errorStatus = GP211_install_for_load(stCardContext, stCardInfo, pstSecurityInfo, pExecutableLoadFileAID, dwExecutableLoadFileAIDLength, pSecurityDomainAID, dwSecurityDomainAIDLength, pLoadFileDataBlockHash, pLoadToken, dwNonVolatileCodeSpaceLimit, dwVolatileDataSpaceLimit, dwNonVolatileDataSpaceLimit);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* pyGP211_get_extradition_token_signature_data(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(3);

    ARGS_GetBYTEArray(0, BYTE, SecurityDomainAID);
    ARGS_GetBYTEArray(1, BYTE, ApplicationAID);

    BYTE baExtraditionTokenSignatureData[0x100];
    DWORD dwExtraditionTokenSignatureDataLength = sizeof(baExtraditionTokenSignatureData) / sizeof(BYTE);
    OPGP_ERROR_STATUS errorStatus = GP211_get_extradition_token_signature_data(pSecurityDomainAID, dwSecurityDomainAIDLength, pApplicationAID, dwApplicationAIDLength, baExtraditionTokenSignatureData, &dwExtraditionTokenSignatureDataLength);
    CHECK_GP_CALL_RESULT(errorStatus);

    return _PyString_FromArrayAndSize((const char *)baExtraditionTokenSignatureData, dwExtraditionTokenSignatureDataLength);
}

PyObject* pyGP211_get_load_token_signature_data(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(6);

    ARGS_GetBYTEArray(0, BYTE, ExecutableLoadFileAID);
    ARGS_GetBYTEArray(1, BYTE, SecurityDomainAID);
    ARGS_GetBYTEArray(2, BYTE, LoadFileDataBlockHash);
    DWORD dwNonVolatileCodeSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 3));
    DWORD dwVolatileDataSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 4));
    DWORD dwNonVolatileDataSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 5));
    BYTE baLoadTokenSignatureData[0x100] = { 0 };
    DWORD dwLoadTokenSignatureDataLength = ARRAY_SIZE(baLoadTokenSignatureData); 

    OPGP_ERROR_STATUS errorStatus = GP211_get_load_token_signature_data(pExecutableLoadFileAID, dwExecutableLoadFileAIDLength, pSecurityDomainAID, dwSecurityDomainAIDLength, pLoadFileDataBlockHash, dwNonVolatileCodeSpaceLimit, dwVolatileDataSpaceLimit, dwNonVolatileDataSpaceLimit, baLoadTokenSignatureData, &dwLoadTokenSignatureDataLength);
    CHECK_GP_CALL_RESULT(errorStatus);

    return _PyString_FromArrayAndSize((const char *)baLoadTokenSignatureData, dwLoadTokenSignatureDataLength);
}

PyObject* pyGP211_get_install_token_signature_data(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(8);

    BYTE bP1 = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 0));
    ARGS_GetBYTEArray(1, BYTE, ExecutableLoadFileAID);
    ARGS_GetBYTEArray(2, BYTE, ExecutableModuleAID);
    ARGS_GetBYTEArray(3, BYTE, ApplicationAID);
    BYTE  bApplicationPrivileges = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 4));
    DWORD  dwVolatileDataSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 5));
    DWORD  dwNonVolatileDataSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 6));
    ARGS_GetBYTEArray(7, BYTE, InstallParameters);

    BYTE baInstallTokenSignatureData[0x100] = { 0 };
    DWORD  dwInstallTokenSignatureDataLength = sizeof(baInstallTokenSignatureData);
    OPGP_ERROR_STATUS errorStatus = GP211_get_install_token_signature_data(bP1, pExecutableLoadFileAID, dwExecutableLoadFileAIDLength, pExecutableModuleAID, dwExecutableModuleAIDLength, pApplicationAID, dwApplicationAIDLength, bApplicationPrivileges, dwVolatileDataSpaceLimit, dwNonVolatileDataSpaceLimit, pInstallParameters, dwInstallParametersLength, baInstallTokenSignatureData, &dwInstallTokenSignatureDataLength);
    CHECK_GP_CALL_RESULT(errorStatus);

    return _PyString_FromArrayAndSize((const char *)&baInstallTokenSignatureData, dwInstallTokenSignatureDataLength);
}

PyObject* pyGP211_calculate_load_token(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(8);

    ARGS_GetBYTEArray(0, BYTE, ExecutableLoadFileAID);
    ARGS_GetBYTEArray(1, BYTE, SecurityDomainAID);
    ARGS_GetBYTEArray(2, BYTE, LoadFileDataBlockHash);
    DWORD dwNonVolatileCodeSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 3));
    DWORD dwVolatileDataSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 4));
    DWORD dwNonVolatileDataSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 5));
    ARGS_GetTCHARArray(6, PEMKeyFileName);
    ARGS_GetBYTEArray(7, char, PassPhrase);

    BYTE baLoadToken[128] = { 0 };
    OPGP_ERROR_STATUS errorStatus = GP211_calculate_load_token(pExecutableLoadFileAID, dwExecutableLoadFileAIDLength, pSecurityDomainAID, dwSecurityDomainAIDLength, pLoadFileDataBlockHash, dwNonVolatileCodeSpaceLimit, dwVolatileDataSpaceLimit, dwNonVolatileDataSpaceLimit, pLoadFileDataBlockHash, (OPGP_STRING)pPEMKeyFileName, pPassPhrase);
    CHECK_GP_CALL_RESULT(errorStatus);

    return _PyString_FromArrayAndSize((const char *)&baLoadToken, sizeof(baLoadToken));
}

PyObject* pyGP211_calculate_install_token(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(10);

    BYTE bP1 = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 0));
    ARGS_GetBYTEArray(1, BYTE, ExecutableLoadFileAID);
    ARGS_GetBYTEArray(2, BYTE, ExecutableModuleAID);
    ARGS_GetBYTEArray(3, BYTE, ApplicationAID);
    BYTE  bApplicationPrivileges = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 4));
    DWORD  dwVolatileDataSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 5));
    DWORD  dwNonVolatileDataSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 6));
    ARGS_GetBYTEArray(7, BYTE, InstallParameters);
    ARGS_GetTCHARArray(8, PEMKeyFileName);
    // Get PassPhrase;
    char* pcPassPhrase = NULL;
    PyObject *pobjPassPhrase = PyTuple_GetItem(args, 7);
    if (pobjPassPhrase != Py_None) {
        pcPassPhrase = (char *)_PyArray_AsString(pobjPassPhrase);
    }

    BYTE baInstallToken[128] = { 0 };
    OPGP_ERROR_STATUS errorStatus = GP211_calculate_install_token(bP1, pExecutableLoadFileAID, dwExecutableLoadFileAIDLength, pExecutableModuleAID, dwExecutableModuleAIDLength, pApplicationAID, dwApplicationAIDLength, bApplicationPrivileges, dwVolatileDataSpaceLimit, dwNonVolatileDataSpaceLimit, pInstallParameters, dwInstallParametersLength, baInstallToken, (OPGP_STRING)pPEMKeyFileName, pcPassPhrase);
    CHECK_GP_CALL_RESULT(errorStatus);

    return _PyString_FromArrayAndSize((const char *)&baInstallToken, sizeof(baInstallToken));
}

PyObject* pyGP211_calculate_load_file_data_block_hash(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(1);

    ARGS_GetTCHARArray(0, PEMKeyFileName);

    BYTE baHash[20] = { 0 };
    OPGP_ERROR_STATUS errorStatus = GP211_calculate_load_file_data_block_hash((OPGP_STRING)pPEMKeyFileName, baHash);
    if (errorStatus.errorStatus != OPGP_ERROR_STATUS_SUCCESS) {
        return PyLong_FromLong(-1);
    }

    return _PyString_FromArrayAndSize((const char *)&baHash, sizeof(baHash));
}

PyObject* pyGP211_load(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(5);

    ARGS_GetCardContext(0);
    ARGS_GetCardInfo(1);
    ARGS_GetStruct(2, GP211_SECURITY_INFO, SecurityInfo);
    ARGS_GetStruct(3, GP211_DAP_BLOCK, DapBlock);
    ARGS_GetTCHARArray(4, ExecutableLoadFileName);

    GP211_RECEIPT_DATA stReceiptData;
    DWORD dwReceiptDataAvailable = 0;
    OPGP_ERROR_STATUS errorStatus = GP211_load(stCardContext, stCardInfo, pstSecurityInfo, pstDapBlock, dwstDapBlockLength, (OPGP_STRING)pExecutableLoadFileName, &stReceiptData, &dwReceiptDataAvailable, NULL);
    CHECK_GP_CALL_RESULT(errorStatus);

    if (0 == dwReceiptDataAvailable) {
        return PyLong_FromLong(0);
    }

    return _PyString_FromArrayAndSize((const char *)&stReceiptData, sizeof(GP211_RECEIPT_DATA));
}

PyObject* pyGP211_load_from_buffer(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(5);

    ARGS_GetCardContext(0);
    ARGS_GetCardInfo(1);
    ARGS_GetStruct(2, GP211_SECURITY_INFO, SecurityInfo);
    ARGS_GetStruct(3, GP211_DAP_BLOCK, DapBlock);
    ARGS_GetBYTEArray(4, BYTE, LoadFileBuf);
    
    GP211_RECEIPT_DATA stReceiptData;
    DWORD dwReceiptDataAvailable = 0;
    OPGP_ERROR_STATUS errorStatus = GP211_load_from_buffer(stCardContext, stCardInfo, pstSecurityInfo, pstDapBlock, dwstDapBlockLength, pLoadFileBuf, dwLoadFileBufLength, &stReceiptData, &dwReceiptDataAvailable, NULL);
    CHECK_GP_CALL_RESULT(errorStatus);

    if (0 == dwReceiptDataAvailable) {
        return PyLong_FromLong(0);
    }
    return _PyString_FromArrayAndSize((const char *)&stReceiptData, sizeof(GP211_RECEIPT_DATA));
}

PyObject* pyGP211_install_for_install(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(11);

    ARGS_GetCardContext(0);
    ARGS_GetCardInfo(1);
    ARGS_GetStruct(2, GP211_SECURITY_INFO, SecurityInfo);
    ARGS_GetBYTEArray(3, BYTE, ExecutableLoadFileAID);
    ARGS_GetBYTEArray(4, BYTE, ExecutableModuleAID);
    ARGS_GetBYTEArray(5, BYTE, ApplicationAID);
    BYTE  bApplicationPrivileges = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 6));
    DWORD  dwVolatileDataSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 7));
    DWORD  dwNonVolatileDataSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 8));
    ARGS_GetBYTEArray(9, BYTE, InstallParameters);
    ARGS_GetBYTEArray(10, BYTE, InstallToken);

    GP211_RECEIPT_DATA stReceiptData;
    DWORD dwReceiptDataAvailable = 0;
    OPGP_ERROR_STATUS errorStatus = GP211_install_for_install(stCardContext, stCardInfo, pstSecurityInfo, pExecutableLoadFileAID, dwExecutableLoadFileAIDLength, pExecutableModuleAID, dwExecutableModuleAIDLength, pApplicationAID, dwApplicationAIDLength, bApplicationPrivileges, dwVolatileDataSpaceLimit, dwNonVolatileDataSpaceLimit, pInstallParameters, dwInstallParametersLength, pInstallToken, &stReceiptData, &dwReceiptDataAvailable);
    CHECK_GP_CALL_RESULT(errorStatus);

    if (0 == dwReceiptDataAvailable) {
        return PyLong_FromLong(0);
    }
    return _PyString_FromArrayAndSize((const char *)&stReceiptData, sizeof(GP211_RECEIPT_DATA));
}

PyObject* pyGP211_install_for_make_selectable(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(6);

    ARGS_GetCardContext(0);
    ARGS_GetCardInfo(1);
    ARGS_GetStruct(2, GP211_SECURITY_INFO, SecurityInfo);
    ARGS_GetBYTEArray(3, BYTE, ApplicationAID);
    BYTE  bApplicationPrivileges = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 4));
    ARGS_GetBYTEArray(5, BYTE, InstallToken);

    GP211_RECEIPT_DATA stReceiptData;
    DWORD dwReceiptDataAvailable = 0;
    OPGP_ERROR_STATUS errorStatus = GP211_install_for_make_selectable(stCardContext, stCardInfo, pstSecurityInfo, pApplicationAID, dwApplicationAIDLength, bApplicationPrivileges, pInstallToken, &stReceiptData, &dwReceiptDataAvailable);
    CHECK_GP_CALL_RESULT(errorStatus);

    if (0 == dwReceiptDataAvailable) {
        return PyLong_FromLong(0);
    }
    return _PyString_FromArrayAndSize((const char *)&stReceiptData, sizeof(GP211_RECEIPT_DATA));
}

PyObject* pyGP211_install_for_install_and_make_selectable(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(11);

    ARGS_GetCardContext(0);
    ARGS_GetCardInfo(1);
    ARGS_GetStruct(2, GP211_SECURITY_INFO, SecurityInfo);
    ARGS_GetBYTEArray(3, BYTE, ExecutableLoadFileAID);
    ARGS_GetBYTEArray(4, BYTE, ExecutableModuleAID);
    ARGS_GetBYTEArray(5, BYTE, ApplicationAID);
    BYTE  bApplicationPrivileges = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 6));
    DWORD  dwVolatileDataSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 7));
    DWORD  dwNonVolatileDataSpaceLimit = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 8));
    ARGS_GetBYTEArray(9, BYTE, InstallParameters);
    ARGS_GetBYTEArray(10, BYTE, InstallToken);

    GP211_RECEIPT_DATA stReceiptData;
    DWORD dwReceiptDataAvailable = 0;
    OPGP_ERROR_STATUS errorStatus = GP211_install_for_install_and_make_selectable(stCardContext, stCardInfo, pstSecurityInfo, pExecutableLoadFileAID, dwExecutableLoadFileAIDLength, pExecutableModuleAID, dwExecutableModuleAIDLength, pApplicationAID, dwApplicationAIDLength, bApplicationPrivileges, dwVolatileDataSpaceLimit, dwNonVolatileDataSpaceLimit, pInstallParameters, dwInstallParametersLength, pInstallToken, &stReceiptData, &dwReceiptDataAvailable);
    CHECK_GP_CALL_RESULT(errorStatus);

    if (0 == dwReceiptDataAvailable) {
        return PyLong_FromLong(0);
    }

    return _PyString_FromArrayAndSize((const char *)&stReceiptData, sizeof(GP211_RECEIPT_DATA));
}

PyObject* pyGP211_install_for_personalization(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(4);

    ARGS_GetCardContext(0);
    ARGS_GetCardInfo(1);
    ARGS_GetStruct(2, GP211_SECURITY_INFO, SecurityInfo);
    ARGS_GetBYTEArray(3, BYTE, ApplicationAID);

    OPGP_ERROR_STATUS errorStatus = GP211_install_for_personalization(stCardContext, stCardInfo, pstSecurityInfo, pApplicationAID, dwApplicationAIDLength);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* pyGP211_install_for_extradition(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(6);

    ARGS_GetCardContext(0);
    ARGS_GetCardInfo(1);
    ARGS_GetStruct(2, GP211_SECURITY_INFO, SecurityInfo);
    ARGS_GetBYTEArray(3, BYTE, SecurityDomainAID);
    ARGS_GetBYTEArray(4, BYTE, ApplicationAID);
    ARGS_GetBYTEArray(5, BYTE, ExtrationToken);

    GP211_RECEIPT_DATA stReceiptData;
    DWORD dwReceiptDataAvailable = 0;
    OPGP_ERROR_STATUS errorStatus = GP211_install_for_extradition(stCardContext, stCardInfo, pstSecurityInfo, pSecurityDomainAID, dwSecurityDomainAIDLength, pApplicationAID, dwApplicationAIDLength, pExtrationToken, &stReceiptData, &dwReceiptDataAvailable);
    CHECK_GP_CALL_RESULT(errorStatus);

    if (0 == dwReceiptDataAvailable) {
        return PyLong_FromLong(0);
    }
    return _PyString_FromArrayAndSize((const char *)&stReceiptData, sizeof(GP211_RECEIPT_DATA));
}

PyObject* pyGP211_put_delegated_management_keys(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(8);

    ARGS_GetCardContext(0);
    ARGS_GetCardInfo(1);
    ARGS_GetStruct(2, GP211_SECURITY_INFO, SecurityInfo);
    BYTE bKeySetVersion = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 3));
    BYTE bNewKeySetVersion = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 4));
    ARGS_GetTCHARArray(5, PEMKeyFileName);
    ARGS_GetBYTEArray(6, char, PassPhrase);
    ARGS_GetBYTEArray(7, BYTE, ReceiptKey);

    OPGP_ERROR_STATUS errorStatus = GP211_put_delegated_management_keys(stCardContext, stCardInfo, pstSecurityInfo, bKeySetVersion, bNewKeySetVersion, (OPGP_STRING)pPEMKeyFileName, pPassPhrase, pReceiptKey);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* pyGP211_send_APDU(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(4);

    ARGS_GetCardContext(0);
    ARGS_GetCardInfo(1);
    ARGS_GetStruct(2, GP211_SECURITY_INFO, SecurityInfo);
    ARGS_GetBYTEArray(3, BYTE, CApdu);
    
    BYTE baRApdu[0x100] = { 0 };
    DWORD dwRApduLength = sizeof(baRApdu) / sizeof(BYTE);
    OPGP_ERROR_STATUS errorStatus = GP211_send_APDU(stCardContext, stCardInfo, pstSecurityInfo, pCApdu, dwCApduLength, baRApdu, &dwRApduLength);
    CHECK_GP_CALL_RESULT(errorStatus);

    return _PyString_FromArrayAndSize((const char *)baRApdu, dwRApduLength);
}

PyObject* pyGP211_calculate_3des_DAP(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(3);

    ARGS_GetBYTEArray(0, BYTE, LoadFileDataBlockHash);
    ARGS_GetBYTEArray(1, BYTE, SecurityDomainAID);
    ARGS_GetBYTEArray(2, BYTE, DAPCalculationKey);
    
    GP211_DAP_BLOCK stLoadFileDataBlockSignature;
    OPGP_ERROR_STATUS errorStatus = GP211_calculate_3des_DAP(pLoadFileDataBlockHash, pSecurityDomainAID, dwSecurityDomainAIDLength, pDAPCalculationKey, &stLoadFileDataBlockSignature);
    CHECK_GP_CALL_RESULT(errorStatus);

    return _PyString_FromArrayAndSize((char *)&stLoadFileDataBlockSignature, sizeof(GP211_DAP_BLOCK));
}

PyObject* pyGP211_calculate_rsa_DAP(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(4);

    ARGS_GetBYTEArray(0, BYTE, LoadFileDataBlockHash);
    ARGS_GetBYTEArray(1, BYTE, SecurityDomainAID);
    ARGS_GetTCHARArray(2, PEMKeyFileName);
    ARGS_GetBYTEArray(3, char, PassPhrase);

    GP211_DAP_BLOCK stLoadFileDataBlockSignature;
    OPGP_ERROR_STATUS errorStatus = GP211_calculate_rsa_DAP(pLoadFileDataBlockHash, pSecurityDomainAID, dwSecurityDomainAIDLength, (OPGP_STRING)pPEMKeyFileName, pPassPhrase, &stLoadFileDataBlockSignature);
    CHECK_GP_CALL_RESULT(errorStatus);

    return _PyString_FromArrayAndSize((char *)&stLoadFileDataBlockSignature, sizeof(GP211_DAP_BLOCK));
}

PyObject* pyGP211_validate_delete_receipt(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(5);

    DWORD dwConfirmationCounter = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 0));
    ARGS_GetBYTEArray(1, BYTE, CardUniqueData);
    ARGS_GetBYTEArray(2, BYTE, ReceiptKey);
    ARGS_GetStruct(3, GP211_RECEIPT_DATA, ReceiptData);
    ARGS_GetBYTEArray(4, BYTE, AID);

    OPGP_ERROR_STATUS errorStatus = GP211_validate_delete_receipt(dwConfirmationCounter, pCardUniqueData, dwCardUniqueDataLength, pReceiptKey, stReceiptData, pAID, dwAIDLength);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* pyGP211_validate_install_receipt(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(6);

    DWORD dwConfirmationCounter = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 0));
    ARGS_GetBYTEArray(1, BYTE, CardUniqueData);
    ARGS_GetBYTEArray(2, BYTE, ReceiptKey);
    ARGS_GetStruct(3, GP211_RECEIPT_DATA, ReceiptData);
    ARGS_GetBYTEArray(4, BYTE, ExecutableLoadFileAID);
    ARGS_GetBYTEArray(5, BYTE, ApplicationAID);

    OPGP_ERROR_STATUS errorStatus = GP211_validate_install_receipt(dwConfirmationCounter, pCardUniqueData, dwCardUniqueDataLength, pReceiptKey, stReceiptData, pExecutableLoadFileAID, dwExecutableLoadFileAIDLength, pApplicationAID, dwApplicationAIDLength);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* pyGP211_validate_load_receipt(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(6);

    DWORD dwConfirmationCounter = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 0));
    ARGS_GetBYTEArray(1, BYTE, CardUniqueData);
    ARGS_GetBYTEArray(2, BYTE, ReceiptKey);
    ARGS_GetStruct(3, GP211_RECEIPT_DATA, ReceiptData);
    ARGS_GetBYTEArray(4, BYTE, ExecutableLoadFileAID);
    ARGS_GetBYTEArray(5, BYTE, SecurityDomainAID);

    OPGP_ERROR_STATUS errorStatus = GP211_validate_load_receipt(dwConfirmationCounter, pCardUniqueData, dwCardUniqueDataLength, pReceiptKey, stReceiptData, pExecutableLoadFileAID, dwExecutableLoadFileAIDLength, pSecurityDomainAID, dwSecurityDomainAIDLength);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* pyGP211_validate_extradition_receipt(PyObject* self, PyObject* args)
{
    PyErr_Format(PyExc_NotImplementedError, "%s() not implemented.", __FUNCTION__);
    return NULL;
    //DWORD dwConfirmationCounter = (DWORD)PyLong_AsLong(PyTuple_GetItem(args, 0));
    //PyObject* pobjCardUniqueData = PyTuple_GetItem(args, 1);
    //PBYTE pbCardUniqueData = (PBYTE)_PyArray_AsString(pobjCardUniqueData);
    //DWORD dwCardUniqueDataLength = (DWORD)PyUnicode_GetSize(pobjCardUniqueData);
    //PBYTE pbReceiptKey = (PBYTE)_PyArray_AsString(PyTuple_GetItem(args, 2));
    //GP211_RECEIPT_DATA stReceiptData = *(GP211_RECEIPT_DATA *)_PyArray_AsString(PyTuple_GetItem(args, 3));
    //PyObject* pobjOldSecurityDomainAID = PyTuple_GetItem(args, 4);
    //PBYTE pbOldSecurityDomainAID = (PBYTE)_PyArray_AsString(pobjOldSecurityDomainAID);
    //DWORD dwOldSecurityDomainAID = (DWORD)PyUnicode_GetSize(pobjOldSecurityDomainAID);
    //PyObject* pobjNewSecurityDomainAID = PyTuple_GetItem(args, 5);
    //PBYTE pbNewSecurityDomainAID = (PBYTE)_PyArray_AsString(pobjNewSecurityDomainAID);
    //DWORD dwNewSecurityDomainAID = (DWORD)PyUnicode_GetSize(pobjNewSecurityDomainAID);
    //PyObject* pobjApplicationOrExecutableLoadFileAID = PyTuple_GetItem(args, 6);
    //PBYTE pbApplicationOrExecutableLoadFileAID = (PBYTE)_PyArray_AsString(pobjApplicationOrExecutableLoadFileAID);
    //DWORD dwApplicationOrExecutableLoadFileAIDLength = (DWORD)PyUnicode_GetSize(pobjApplicationOrExecutableLoadFileAID);

    //OPGP_ERROR_STATUS errorStatus = GP211_validate_extradition_receipt(dwConfirmationCounter, pbCardUniqueData, dwCardUniqueDataLength, pbReceiptKey, stReceiptData, pbOldSecurityDomainAID, dwOldSecurityDomainAID, pbNewSecurityDomainAID, dwNewSecurityDomainAID, pbApplicationOrExecutableLoadFileAID, dwApplicationOrExecutableLoadFileAIDLength);
    //if (errorStatus.errorStatus != OPGP_ERROR_STATUS_SUCCESS) {
    //    return PyLong_FromLong(-1);
    //}

    return PyLong_FromLong(0);
}

PyObject* pyOPGP_manage_channel(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(5);

    ARGS_GetCardContext(0);
    ARGS_GetCardInfo(1);
    ARGS_GetStruct(2, GP211_SECURITY_INFO, SecurityInfo);
    BYTE bOpenClose = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 3));
    BYTE bChannelNumberToClose = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 4));

    BYTE bChannelNumberOpened = 0;
    OPGP_ERROR_STATUS errorStatus = OPGP_manage_channel(stCardContext, &stCardInfo, pstSecurityInfo, bOpenClose, bChannelNumberToClose, &bChannelNumberOpened);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(bChannelNumberOpened);
}

PyObject* pyOPGP_select_channel(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(2);

    ARGS_GetCardInfo(0);
    BYTE bChannelNumber = (BYTE)PyLong_AsLong(PyTuple_GetItem(args, 1));

    OPGP_ERROR_STATUS errorStatus = OPGP_select_channel(&stCardInfo, bChannelNumber);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
}

PyObject* pyGP211_store_data(PyObject* self, PyObject* args)
{
    CHECK_FUNCTION_ARGUMENTS_COUNT(4);

    ARGS_GetCardContext(0);
    ARGS_GetCardInfo(1);
    ARGS_GetStruct(2, GP211_SECURITY_INFO, SecurityInfo);
    ARGS_GetBYTEArray(3, BYTE, Data);

    OPGP_ERROR_STATUS errorStatus = GP211_store_data(stCardContext, stCardInfo, pstSecurityInfo, pData, dwDataLength);
    CHECK_GP_CALL_RESULT(errorStatus);

    return PyLong_FromLong(0);
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

    ARGS_GetTCHARArray(0, LoadFileName);

    OPGP_LOAD_FILE_PARAMETERS stLoadFileParameters = { 0 };
    OPGP_ERROR_STATUS errorStatus = OPGP_read_executable_load_file_parameters(pLoadFileName, &stLoadFileParameters);
    CHECK_GP_CALL_RESULT(errorStatus);

    PyObject* pobjRet = PyDict_New();
    PyDict_SetItem(pobjRet, _PyString_FromArrayAndSize("loadFileSize", strlen("loadFileSize")), PyLong_FromLong(stLoadFileParameters.loadFileSize));
    PyDict_SetItem(pobjRet, _PyString_FromArrayAndSize("loadFileAID", strlen("loadFileAID")), _PyString_FromArrayAndSize((const char *)stLoadFileParameters.loadFileAID.AID, stLoadFileParameters.loadFileAID.AIDLength));
    PyObject* pobjAppletAIDs = PyTuple_New(stLoadFileParameters.numAppletAIDs);
    for (BYTE b = 0; b < stLoadFileParameters.numAppletAIDs; ++b) {
        PyTuple_SetItem(pobjAppletAIDs, b, _PyString_FromArrayAndSize((const char *)stLoadFileParameters.appletAIDs[b].AID, stLoadFileParameters.appletAIDs[b].AIDLength));
    }
    PyDict_SetItem(pobjRet, _PyString_FromArrayAndSize("applets", strlen("applets")), pobjAppletAIDs);
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

