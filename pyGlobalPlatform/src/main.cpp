#include <Python.h>

#include "gp_functions.h"

static PyObject* version(PyObject *self, PyObject *args);

static PyMethodDef GPMethods[] = {
    { "version", version, METH_VARARGS, "Get version of pyGlobalPlatform." },
    { "OPGP_enable_trace_mode", pyOPGP_enable_trace_mode, METH_VARARGS, "Enables the trace mode." },

    { "establishContext", establishContext, METH_VARARGS, "This function establishes a context to connection layer." },
    { "releaseContext", releaseContext, METH_VARARGS, "This function releases the context to the connection layer established by establishContext()." },
    { "listReaders", listReaders , METH_VARARGS, "This function returns a list of currently available readers." },
    { "connectCard", connectCard, METH_VARARGS, "This function connects to a reader." },
    { "disconnectCard", disconnectCard, METH_VARARGS, "This function disconnects a reader." },

    { "OPGP_select_application", pyOPGP_select_application, METH_VARARGS, "GlobalPlatform2.1.1: Selects an application on a card by AID."},
    { "GP211_get_status", pyGP211_get_status, METH_VARARGS, "GlobalPlatform2.1.1: Gets the life cycle status of Applications, the Issuer Security Domains, Security Domains and Executable Load Files and their privileges or information about Executable Modules of the Executable Load Files."},
    { "GP211_set_status", pyGP211_set_status, METH_VARARGS, "GlobalPlatform2.1.1: Sets the life cycle status of Applications, Security Domains or the Card Manager."},
    { "GP211_mutual_authentication", pyGP211_mutual_authentication, METH_VARARGS, "GlobalPlatform2.1.1: Mutual authentication."},
    { "GP211_init_implicit_secure_channel", pyGP211_init_implicit_secure_channel, METH_VARARGS, "GlobalPlatform2.1.1: Inits a Secure Channel implicitly."},
    { "GP211_close_implicit_secure_channel", pyGP211_close_implicit_secure_channel, METH_VARARGS, "GlobalPlatform2.1.1: Closes a Secure Channel implicitly."},
    { "GP211_get_data", pyGP211_get_data, METH_VARARGS, "GlobalPlatform2.1.1: Retrieve card data."},
    { "GP211_get_data_iso7816_4", pyGP211_get_data_iso7816_4, METH_VARARGS, "Retrieve card data according ISO/IEC 7816-4 command not within a secure channel."},
    { "GP211_get_secure_channel_protocol_details", pyGP211_get_secure_channel_protocol_details, METH_VARARGS, "GlobalPlatform2.1.1: This returns the Secure Channel Protocol and the Secure Channel Protocol implementation."},
    { "GP211_get_sequence_counter", pyGP211_get_sequence_counter, METH_VARARGS, "GlobalPlatform2.1.1: This returns the current Sequence Counter."},
    { "GP211_put_data", pyGP211_put_data, METH_VARARGS, "GlobalPlatform2.1.1: Put card data."},
    { "GP211_pin_change", pyGP211_pin_change, METH_VARARGS, "GlobalPlatform2.1.1: Changes or unblocks the global PIN."},
    { "GP211_put_3des_key", pyGP211_put_3des_key, METH_VARARGS, "GlobalPlatform2.1.1: replaces a single 3DES key in a key set or adds a new 3DES key."},
    { "GP211_put_rsa_key", pyGP211_put_rsa_key, METH_VARARGS, "GlobalPlatform2.1.1: replaces a single public RSA key in a key set or adds a new public RSA key."},
    { "GP211_put_secure_channel_keys", pyGP211_put_secure_channel_keys, METH_VARARGS, "GlobalPlatform2.1.1: replaces or adds a secure channel key set consisting of S-ENC, S-MAC and DEK."},
    { "GP211_delete_key", pyGP211_delete_key, METH_VARARGS, "GlobalPlatform2.1.1: deletes a key or multiple keys."},
    { "GP211_get_key_information_templates", pyGP211_get_key_information_templates, METH_VARARGS, "GlobalPlatform2.1.1: Retrieves key information of keys on the card."},
    { "GP211_delete_application", pyGP211_delete_application, METH_VARARGS, "GlobalPlatform2.1.1: Deletes a Executable Load File or an application."},
    { "GP211_install_for_load", pyGP211_install_for_load, METH_VARARGS, "GlobalPlatform2.1.1: Prepares the card for loading an application."},
    { "GP211_get_extradition_token_signature_data", pyGP211_get_extradition_token_signature_data, METH_VARARGS, "GlobalPlatform2.1.1: Function to retrieve the data to sign by the Card Issuer in an Extradition Token."},
    { "GP211_get_load_token_signature_data", pyGP211_get_load_token_signature_data, METH_VARARGS, "GlobalPlatform2.1.1: Function to retrieve the data to sign by the Card Issuer in a Load Token."},
    { "GP211_get_install_token_signature_data", pyGP211_get_install_token_signature_data, METH_VARARGS, "GlobalPlatform2.1.1: Function to retrieve the data to sign by the Card Issuer in an Install Token."},
    { "GP211_calculate_load_token", pyGP211_calculate_load_token, METH_VARARGS, "GlobalPlatform2.1.1: Calculates a Load Token using PKCS#1."},
    { "GP211_calculate_install_token", pyGP211_calculate_install_token, METH_VARARGS, "GlobalPlatform2.1.1: Calculates an Install Token using PKCS#1."},
    { "GP211_calculate_load_file_data_block_hash", pyGP211_calculate_load_file_data_block_hash, METH_VARARGS, "GlobalPlatform2.1.1: Calculates a Load File Data Block Hash."},
    { "GP211_load", pyGP211_load, METH_VARARGS, "GlobalPlatform2.1.1: Loads a Executable Load File ,"},
    { "GP211_load_from_buffer", pyGP211_load_from_buffer, METH_VARARGS, "GlobalPlatform2.1.1: Loads a Executable Load File ,"},
    { "GP211_install_for_install", pyGP211_install_for_install, METH_VARARGS, "GlobalPlatform2.1.1: Installs an application on the card."},
    { "GP211_install_for_make_selectable", pyGP211_install_for_make_selectable, METH_VARARGS, "GlobalPlatform2.1.1: Makes an installed application selectable."},
    { "GP211_install_for_install_and_make_selectable", pyGP211_install_for_install_and_make_selectable, METH_VARARGS, "GlobalPlatform2.1.1: Installs and makes an installed application selectable."},
    { "GP211_install_for_personalization", pyGP211_install_for_personalization, METH_VARARGS, "GlobalPlatform2.1.1: Informs a Security Domain that a associated application will retrieve personalization data."},
    { "GP211_install_for_extradition", pyGP211_install_for_extradition, METH_VARARGS, "GlobalPlatform2.1.1: Associates an application with another Security Domain."},
    { "GP211_put_delegated_management_keys", pyGP211_put_delegated_management_keys, METH_VARARGS, "GlobalPlatform2.1.1: Adds a key set for Delegated Management."},
    { "GP211_send_APDU", pyGP211_send_APDU, METH_VARARGS, "Sends an application protocol data unit."},
    { "GP211_calculate_3des_DAP", pyGP211_calculate_3des_DAP, METH_VARARGS, "GlobalPlatform2.1.1: Calculates a Load File Data Block Signature using 3DES."},
    { "GP211_calculate_rsa_DAP", pyGP211_calculate_rsa_DAP, METH_VARARGS, "GlobalPlatform2.1.1: Calculates a Load File Data Block Signature using SHA-1 and PKCS#1 ,"},
    { "GP211_validate_delete_receipt", pyGP211_validate_delete_receipt, METH_VARARGS, "GlobalPlatform2.1.1: Validates a Load Receipt."},
    { "GP211_validate_install_receipt", pyGP211_validate_install_receipt, METH_VARARGS, "GlobalPlatform2.1.1: Validates an Install Receipt."},
    { "GP211_validate_load_receipt", pyGP211_validate_load_receipt, METH_VARARGS, "GlobalPlatform2.1.1: Validates a Load Receipt."},
    { "GP211_validate_extradition_receipt", pyGP211_validate_extradition_receipt, METH_VARARGS, "GlobalPlatform2.1.1: Validates an Extradition Receipt."},
    { "OPGP_manage_channel", pyOPGP_manage_channel, METH_VARARGS, "ISO 7816-4 / GlobalPlatform2.1.1: Opens or closes a Logical Channel."},
    { "OPGP_select_channel", pyOPGP_select_channel, METH_VARARGS, "ISO 7816-4 / GlobalPlatform2.1.1: If multiple Logical Channels are open or a new Logical Channel is opened with select_application,"},
    { "GP211_store_data", pyGP211_store_data, METH_VARARGS, "GlobalPlatform2.1.1: The STORE DATA command is used to transfer data to an Application or the Security Domain processing the command."},
    { "GP211_begin_R_MAC", pyGP211_begin_R_MAC, METH_VARARGS, "Initiates a R-MAC session."},
    { "GP211_end_R_MAC", pyGP211_end_R_MAC, METH_VARARGS, "Terminates a R-MAC session."},
    { "OPGP_read_executable_load_file_parameters", pyOPGP_read_executable_load_file_parameters, METH_VARARGS, "Reads the parameters of an Executable Load File."},
    { "OPGP_VISA2_derive_keys", pyOPGP_VISA2_derive_keys, METH_VARARGS, "Derives the static keys from a master key according the VISA 2 key derivation scheme."},
    { "OPGP_cap_to_ijc", pyOPGP_cap_to_ijc, METH_VARARGS, "Converts a CAP file to an IJC file ,"},
    { "OPGP_extract_cap_file", pyOPGP_extract_cap_file, METH_VARARGS, "Extracts a CAP file into a buffer."},
    { "OPGP_read_executable_load_file_parameters_from_buffer", pyOPGP_read_executable_load_file_parameters_from_buffer, METH_VARARGS, "Receives Executable Load File as a buffer instead of a FILE."},
    { "OPGP_EMV_CPS11_derive_keys", pyOPGP_EMV_CPS11_derive_keys, METH_VARARGS, "Derives the static keys from a master key according the EMV CPS 1.1 key derivation scheme."},

    { NULL, NULL, 0, NULL }        /* Sentinel */
};

static PyModuleDef g_pyModuleDef = {
    PyModuleDef_HEAD_INIT
    , "pyGlobalPlatform"
    , "pyGlobalPlatform Doc."
    , -1
    , GPMethods
    , NULL
    , NULL
    , NULL
    , NULL
};

PyMODINIT_FUNC PyInit_pyGlobalPlatform(void)
{
    return PyModule_Create(&g_pyModuleDef);
}

static PyObject* version(PyObject *self, PyObject *args)
{
    return Py_BuildValue("s", "1.4");
}
