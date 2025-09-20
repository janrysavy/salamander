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
// Password storage. When the user enables "Use a master password" the
// configuration saves passwords encrypted with AES; otherwise they are only
// scrambled using the method originally introduced in the FTP client.
//
// Password Manager methods may be called only from Salamander's main thread.
// Planned access points are: FTP connect, WinSCP connect, Salamander
// configuration, and saving/loading Salamander configuration. Everything
// currently runs in the main thread, so there is no need to address
// concurrency or manager locking.

#pragma pack(push)
#pragma pack(1)
struct CMasterPasswordVerifier
{
    BYTE Salt[16];  // random salt, mode == 3
    BYTE Dummy[16]; // random encrypted data
    BYTE MAC[10];   // control record used to verify the master password
};
#pragma pack(pop)

class CPasswordManager
{
private:
    BOOL UseMasterPassword;                          // the user previously entered a master password that was used for encryption; the actual 'MasterPassword' may be NULL and requested later
    char* PlainMasterPassword;                       // allocated password in plain form terminated by zero; NULL if the user did not enter it this session; not stored in the registry
    char* OldPlainMasterPassword;                    // temporarily holds the old password during Plugins.PasswordManagerEvent() so plugins can decrypt passwords
    CMasterPasswordVerifier* MasterPasswordVerifier; // verifies the master password; stored in the registry and may be NULL

    CSalamanderCryptAbstract* SalamanderCrypt; // interface for working with the Crypt library

public:
    CPasswordManager();
    ~CPasswordManager();

    BOOL IsPasswordSecure(const char* password); // evaluates password strength, returns TRUE if strong enough, otherwise FALSE

    // sets the master password; if 'password' is NULL or empty, turns master password off
    void SetMasterPassword(HWND hParent, const char* password);

    // used to supply the master password when the plain form is currently unknown
    BOOL EnterMasterPassword(const char* password);

    BOOL ChangeMasterPassword(HWND hParent);
    BOOL IsUsingMasterPassword() { return UseMasterPassword; }         // are passwords protected via AES/Master Password?
    BOOL IsMasterPasswordSet() { return PlainMasterPassword != NULL; } // did the user enter the Master Password in this session?

    // if master password usage is enabled and it hasn't been entered in this session,
    // displays a dialog to enter it. Returns FALSE when a valid password cannot be obtained,
    // otherwise TRUE. Always call this before EncryptPassword/DecryptPassword when encrypt/encrypted == TRUE.
    // For convenience it can be called even when master password use is disabled (quietly returns TRUE).
    BOOL AskForMasterPassword(HWND hParent);

    void NotifyAboutMasterPasswordChange(HWND hParent);

    BOOL Save(HKEY hKey); // saves stored passwords to the Registry
    BOOL Load(HKEY hKey); // loads passwords from the Registry

    // 'encryptedPasswordSize' specifies the size of the buffer to hold the encrypted password; it must be 50 characters larger than 'plainPassword'

    // encrypts a plain text password into binary form using the strong AES cipher
    // before AES encryption a scramble with padding is applied (strengthens short passwords)
    // when AES encryption is requested ('encrypt' == TRUE) AskForMasterPassword() must be called before and succeed
    // 'plainPassword' points to the null-terminated text password
    // 'encryptedPassword' returns a pointer to a Salamander-allocated binary buffer with the encrypted password; free it with CSalamanderGeneralAbstract::Free
    // 'encryptedPasswordSize' returns the size of the 'encryptedPassword' buffer in bytes
    // if 'encrypt' is TRUE the function encrypts using AES (protected with master password); if FALSE the password is only scrambled
    BOOL EncryptPassword(const char* plainPassword, BYTE** encryptedPassword, int* encryptedPasswordSize, BOOL encrypt);
    // 'plainPassword' must be freed with CSalamanderGeneralAbstract::Free
    // if 'plainPassword' is NULL it only checks whether the password can be decrypted
    BOOL DecryptPassword(const BYTE* encryptedPassword, int encryptedPasswordSize, char** plainPassword);
    // returns TRUE for an AES-encrypted password, otherwise FALSE; decided by the signature in the first byte of the password
    BOOL IsPasswordEncrypted(const BYTE* encyptedPassword, int encyptedPasswordSize);

    // adds a new password to the Passwords array; returns TRUE on success (and fills 'passwordID'
    // with a value greater than zero and less than 0xffffffff), otherwise FALSE
    // 'pluginDLLName' must be NULL when the password belongs to the Salamander core, otherwise CPluginData is filled
    // 'password' is the password in plain form
    //BOOL StorePassword(const char *pluginDLLName, const char *password, DWORD *passwordID); // the call must be preceded by a successful AskForMasterPassword()
    //BOOL SetPassword(const char *pluginDLLName, DWORD passwordID, const char *password); // the call must be preceded by a successful AskForMasterPassword()
    //BOOL GetPassword(const char *pluginDLLName, DWORD passwordID, char *password, int bufferLen); // the call must be preceded by a successful AskForMasterPassword()
    //BOOL DeletePassword(const char *pluginDLLName, DWORD passwordID);

    // checks whether 'password' matches the one stored in 'MasterPasswordVerifier'; returns TRUE on match, otherwise FALSE
    BOOL VerifyMasterPassword(const char* password);

protected:
    // allocates and computes 'MasterPasswordVerifier' which is stored in the registry for later verification
    void CreateMasterPasswordVerifier(const char* password);
};

extern CPasswordManager PasswordManager;
