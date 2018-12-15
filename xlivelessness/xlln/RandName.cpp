#include "windows.h"
#include "RandName.h"
#include <string>

static const DWORD MAX_RAND_NAME = 128;

// Get a random vowel
static WCHAR GetRandVowel()
{
	for (;;)
	{
		WCHAR c = WCHAR(L'a' + (rand() % 26));
		if (wcschr(L"aeiou", c) != NULL)
			return c;
	}
}

// Append vowel or vowel group to string
static VOID AppendVowel(WCHAR* strRandom)
{
	if ((rand() % 2 == 0))
	{
		WCHAR strChar[2] = { GetRandVowel(), 0 };
		lstrcatW(strRandom, strChar);
	}
	else
	{
		const WCHAR* strVowelGroup[10] =
		{
			L"ai", L"au", L"ay", L"ea", L"ee", L"ie", L"oa", L"oi", L"oo", L"ou"
		};
		lstrcatW(strRandom, strVowelGroup[rand() % 10]);
	}
}

// Get a random consonant
static WCHAR GetRandConsonant()
{
	for (;;)
	{
		WCHAR c = WCHAR(L'a' + (rand() % 26));
		if (wcschr(L"aeiou", c) == NULL)
			return c;
	}
}

// Append consonant or consonant group to string
static VOID AppendConsonant(WCHAR* strRandom, BOOL bLeading)
{
	if ((rand() % 2 == 0))
	{
		WCHAR strChar[2] = { GetRandConsonant(), 0 };
		lstrcatW(strRandom, strChar);
	}
	else
	{
		const WCHAR* strLeadConGroup[32] =
		{
			L"bl", L"br", L"cl", L"cr", L"dr", L"fl", L"fr", L"gh", L"gl", L"gn",
			L"gr", L"kl", L"kn", L"kr", L"ph", L"pl", L"pr", L"ps", L"qu", L"sc",
			L"sk", L"sl", L"sn", L"sp", L"st", L"sw", L"th", L"tr", L"vh", L"vl",
			L"wh", L"zh"
		};
		const WCHAR* strTrailConGroup[32] =
		{
			L"rt", L"ng", L"bs", L"cs", L"ds", L"gs", L"hs", L"sh", L"ss", L"ks",
			L"ms", L"ns", L"ps", L"rs", L"ts", L"gh", L"ph", L"sk", L"st", L"tt",
			L"nd", L"nk", L"nt", L"nx", L"pp", L"rd", L"rg", L"rk", L"rn", L"rv",
			L"th", L"ys"
		};
		if (bLeading)
			lstrcatW(strRandom, strLeadConGroup[rand() % 32]);
		else
			lstrcatW(strRandom, strTrailConGroup[rand() % 32]);
	}
}

// Generate a random name
VOID GetName(WCHAR* strName, DWORD dwSize)
{
	//assert(dwSize < MAX_RAND_NAME);

	// Name consists of two to five parts.
	//
	// 1) consonant or consonant group (e.g. th, qu, st) [optional]
	// 2) vowel or vowel group (e.g. ea, ee, au)
	// 3) consonant or consonant group
	// 4) vowel or vowel group [optional]
	// 5) consonant or consonant group [optional]

	WCHAR strRandom[MAX_RAND_NAME];
	strRandom[0] = 0;
	if ((rand() % 2 == 0))
		AppendConsonant(strRandom, TRUE);
	AppendVowel(strRandom);
	AppendConsonant(strRandom, FALSE);
	if ((rand() % 2 == 0))
	{
		AppendVowel(strRandom);
		if ((rand() % 2 == 0))
			AppendConsonant(strRandom, FALSE);
	}

	// Make the first letter upper case
	*strRandom = towupper(*strRandom);

	lstrcpynW(strName, strRandom, dwSize);
}
