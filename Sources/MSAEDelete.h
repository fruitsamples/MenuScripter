// MSAEDelete.h//// Original version by Jon Lansdell and Nigel Humphreys.// 4.0 and 3.1 updates by Greg Sutton.// �Apple Computer Inc 1996, all rights reserved.#ifndef __MSAEDELETE__#define __MSAEDELETE__#include <AppleEvents.h>#include <AEObjects.h>#include <AERegistry.h>#include "MSToken.h"pascal OSErr	DoDelete(const AppleEvent *theAppleEvent, AppleEvent *reply, long refcon);OSErr			DeleteTextToken(TextToken* theToken);OSErr			DeleteTextDesc(AEDesc* textDesc);OSErr			DeleteDesc(AEDesc* aDesc);#endif