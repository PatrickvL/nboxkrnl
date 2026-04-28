/*
 * PatrickvL              Copyright (c) 2026
 */

#include "xc.hpp"
#include "rtl.hpp"


EXPORTNUM(338) VOID XBOXAPI XcRC4Key
(
	PVOID KeyState,
	ULONG KeyLength,
	PVOID Key
)
{
	// RC4 key schedule
	PUCHAR S = (PUCHAR)KeyState;
	PUCHAR K = (PUCHAR)Key;

	for (ULONG i = 0; i < 256; i++) {
		S[i] = (UCHAR)i;
	}
	// Store index bytes at S[256] and S[257]
	S[256] = 0;
	S[257] = 0;

	UCHAR j = 0;
	for (ULONG i = 0; i < 256; i++) {
		j = j + S[i] + K[i % KeyLength];
		UCHAR temp = S[i];
		S[i] = S[j];
		S[j] = temp;
	}
}

EXPORTNUM(339) VOID XBOXAPI XcRC4Crypt
(
	PVOID KeyState,
	ULONG DataLength,
	PVOID Data
)
{
	PUCHAR S = (PUCHAR)KeyState;
	PUCHAR D = (PUCHAR)Data;
	UCHAR i = S[256];
	UCHAR j = S[257];

	for (ULONG n = 0; n < DataLength; n++) {
		i++;
		j += S[i];
		UCHAR temp = S[i];
		S[i] = S[j];
		S[j] = temp;
		D[n] ^= S[(UCHAR)(S[i] + S[j])];
	}

	S[256] = i;
	S[257] = j;
}

EXPORTNUM(340) VOID XBOXAPI XcHMAC
(
	PVOID KeyMaterial,
	ULONG KeyMaterialLength,
	PVOID Data1,
	ULONG Data1Length,
	PVOID Data2,
	ULONG Data2Length,
	PVOID Digest
)
{
	// HMAC-SHA1 implementation using existing XcSHA* functions
	UCHAR KeyBuf[64];
	UCHAR iPad[64];
	UCHAR oPad[64];

	RtlZeroMemory(KeyBuf, sizeof(KeyBuf));
	if (KeyMaterialLength <= 64) {
		RtlMoveMemory(KeyBuf, KeyMaterial, KeyMaterialLength);
	}
	else {
		// Hash key if longer than block size
		UCHAR ShaContext[116]; // SHA context
		XcSHAInit(ShaContext);
		XcSHAUpdate(ShaContext, (PUCHAR)KeyMaterial, KeyMaterialLength);
		XcSHAFinal(ShaContext, KeyBuf);
	}

	for (ULONG i = 0; i < 64; i++) {
		iPad[i] = KeyBuf[i] ^ 0x36;
		oPad[i] = KeyBuf[i] ^ 0x5C;
	}

	// Inner hash: SHA1(iPad || Data1 || Data2)
	UCHAR InnerContext[116];
	XcSHAInit(InnerContext);
	XcSHAUpdate(InnerContext, iPad, 64);
	if (Data1 && Data1Length) {
		XcSHAUpdate(InnerContext, (PUCHAR)Data1, Data1Length);
	}
	if (Data2 && Data2Length) {
		XcSHAUpdate(InnerContext, (PUCHAR)Data2, Data2Length);
	}
	UCHAR InnerHash[20];
	XcSHAFinal(InnerContext, InnerHash);

	// Outer hash: SHA1(oPad || InnerHash)
	UCHAR OuterContext[116];
	XcSHAInit(OuterContext);
	XcSHAUpdate(OuterContext, oPad, 64);
	XcSHAUpdate(OuterContext, InnerHash, 20);
	XcSHAFinal(OuterContext, (PUCHAR)Digest);
}

EXPORTNUM(344) BOOLEAN XBOXAPI XcVerifyPKCS1Signature
(
	PVOID Signature,
	PVOID PublicKey,
	PVOID Data
)
{
	// Stub: signature verification always succeeds in emulation
	return TRUE;
}

EXPORTNUM(345) ULONG XBOXAPI XcModExp
(
	PULONG Result,
	PULONG Base,
	PULONG Exponent,
	PULONG Modulus,
	ULONG ModulusLength
)
{
	// Stub: modular exponentiation
	RtlZeroMemory(Result, ModulusLength);
	return 0;
}

EXPORTNUM(346) VOID XBOXAPI XcDESKeyParity
(
	PUCHAR pbKey,
	ULONG dwKeyLength
)
{
	// Stub: DES key parity
}

EXPORTNUM(347) VOID XBOXAPI XcKeyTable
(
	ULONG dwCipher,
	PVOID pKeyTable,
	PUCHAR pbKey
)
{
	// Stub: DES key table generation
}

EXPORTNUM(349) VOID XBOXAPI XcBlockCryptCBC
(
	ULONG dwCipher,
	ULONG dwInputLength,
	PUCHAR pbOutput,
	PUCHAR pbInput,
	PVOID pKeyTable,
	ULONG dwOp,
	PUCHAR pbFeedback
)
{
	// Stub: block cipher CBC mode
	RtlMoveMemory(pbOutput, pbInput, dwInputLength);
}
