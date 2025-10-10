// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

class CPasswordManager;

//****************************************************************************
//
// CChangeMasterPassword
//

class CChangeMasterPassword : public CCommonDialog
{
private:
    CPasswordManager* PwdManager;

public:
    CChangeMasterPassword(HWND hParent, CPasswordManager* pwdManager);

protected:
    virtual void Validate(CTransferInfo& ti);
    virtual void Transfer(CTransferInfo& ti);

    void EnableControls();

    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

//****************************************************************************
//
// CEnterMasterPassword
//

class CEnterMasterPassword : public CCommonDialog
{
private:
    CPasswordManager* PwdManager;

public:
    CEnterMasterPassword(HWND hParent, CPasswordManager* pwdManager);

protected:
    virtual void Validate(CTransferInfo& ti);
    virtual void Transfer(CTransferInfo& ti);

    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

//****************************************************************************
//
// CRemoveMasterPassword
//

class CRemoveMasterPassword : public CCommonDialog
{
private:
    CPasswordManager* PwdManager;

public:
    CRemoveMasterPassword(HWND hParent, CPasswordManager* pwdManager);

protected:
    virtual void Validate(CTransferInfo& ti);
    virtual void Transfer(CTransferInfo& ti);

    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

//****************************************************************************
//
// CPasswordManager
//
// Password storage. When the user enables the "Use Master Password" option,
// configuration passwords are encrypted with AES; otherwise they are only
// scrambled using Petr's original FTP client method.
//
// Password manager methods may be called only from Salamander's main thread.
// Planned call sites include FTP and WinSCP connection dialogs and the
// configuration save/load workflows. Because all of them currently run on the
// main thread, no explicit locking is required.

#pragma pack(push)
#pragma pack(1)
struct CMasterPasswordVerifier
{
    BYTE Salt[16];  // random salt, mode == 3
    BYTE Dummy[16]; // random encrypted data
    BYTE MAC[10];   // verifier used to check the master password
};
#pragma pack(pop)

class CPasswordManager
{
private:
    BOOL UseMasterPassword;                          // TRUE once the user has provided the master password; the plaintext value may later be NULL and must then be requested
    char* PlainMasterPassword;                       // plaintext master password allocated with a trailing zero; NULL until entered during this session; not stored in the registry
    char* OldPlainMasterPassword;                    // temporarily holds the previous PlainMasterPassword while Plugins.PasswordManagerEvent() runs so plugins can decrypt their data
    CMasterPasswordVerifier* MasterPasswordVerifier; // verifier stored in the registry for subsequent master password checks; may be NULL

    CSalamanderCryptAbstract* SalamanderCrypt; // interface to the cryptographic library

public:
    CPasswordManager();
    ~CPasswordManager();

    BOOL IsPasswordSecure(const char* password); // returns TRUE when the password meets strength requirements, otherwise FALSE

    // sets the master password; if 'password' is NULL or empty, master password protection is turned off
    void SetMasterPassword(HWND hParent, const char* password);

    // supplies the master password when it is not currently known in plaintext
    BOOL EnterMasterPassword(const char* password);

    BOOL ChangeMasterPassword(HWND hParent);
    BOOL IsUsingMasterPassword() { return UseMasterPassword; }         // TRUE when master password protection is active
    BOOL IsMasterPasswordSet() { return PlainMasterPassword != NULL; } // TRUE if the user entered the master password in this session

    // when master password usage is enabled and the password has not been entered
    // in this session, displays a dialog for entering it
    // returns FALSE if the correct master password cannot be entered in that situation; returns TRUE otherwise
    // call this before EncryptPassword/DecryptPassword when encrypt/encrypted == TRUE
    // callers may invoke it even when master password usage is turned off (quietly returns TRUE)
    BOOL AskForMasterPassword(HWND hParent);

    void NotifyAboutMasterPasswordChange(HWND hParent);

    BOOL Save(HKEY hKey); // save password-manager data to the registry
    BOOL Load(HKEY hKey); // load password-manager data from the registry

    // 'encryptedPasswordSize' specifies the buffer size for the encrypted password; it must be 50 characters longer than 'plainPassword'

    // encrypts the plaintext password into binary form using AES
    // before AES encryption it performs an additional scramble that adds padding (hardening short passwords)
    // if the caller requires AES password encryption ('encrypt' == TRUE), AskForMasterPassword() must succeed beforehand
    // 'plainPassword' points to the zero-terminated password in text form
    // 'encryptedPassword' receives a pointer to a binary buffer allocated by Salamander with the encrypted password; release it with CSalamanderGeneralAbstract::Free
    // 'encryptedPasswordSize' receives the size of the 'encryptedPassword' buffer in bytes
    // if 'encrypt' is TRUE, the function encrypts the password with AES (protected by the master password); if FALSE, the password is only scrambled
    BOOL EncryptPassword(const char* plainPassword, BYTE** encryptedPassword, int* encryptedPasswordSize, BOOL encrypt);
    // 'plainPassword' must be deallocated with CSalamanderGeneralAbstract::Free
    // if 'plainPassword' is NULL, only checks whether the password can be decrypted
    BOOL DecryptPassword(const BYTE* encryptedPassword, int encryptedPasswordSize, char** plainPassword);
    // returns TRUE for an AES-encrypted password, otherwise returns FALSE; the signature in the first byte determines it
    BOOL IsPasswordEncrypted(const BYTE* encyptedPassword, int encyptedPasswordSize);

    // adds a new password to the Passwords array; returns TRUE if successful (also fills 'passwordID' with a value greater than zero and less than 0xffffffff), otherwise FALSE
    // 'pluginDLLName' must be NULL if the password belongs to Salamander's core, otherwise it is filled by CPluginData
    // 'password' is the password in plain form
    //BOOL StorePassword(const char *pluginDLLName, const char *password, DWORD *passwordID); // the call must be preceded by a successful AskForMasterPassword()
    //BOOL SetPassword(const char *pluginDLLName, DWORD passwordID, const char *password); // the call must be preceded by a successful AskForMasterPassword()
    //BOOL GetPassword(const char *pluginDLLName, DWORD passwordID, char *password, int bufferLen); // the call must be preceded by a successful AskForMasterPassword()
    //BOOL DeletePassword(const char *pluginDLLName, DWORD passwordID);

    // verifies that 'password' matches the value stored in MasterPasswordVerifier; returns TRUE on success, otherwise FALSE
    BOOL VerifyMasterPassword(const char* password);

protected:
    // allocates and computes MasterPasswordVerifier, which is stored in the registry for subsequent verification
    void CreateMasterPasswordVerifier(const char* password);
};

extern CPasswordManager PasswordManager;
