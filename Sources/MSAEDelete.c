// MSAEDelete.c//// Original version by Jon Lansdell and Nigel Humphreys.// 4.0 and 3.1 updates by Greg Sutton.// �Apple Computer Inc 1996, all rights reserved.#include "MSAEDelete.h"#include "MSAEUtils.h"#include "MSWindow.h"		// for DPtrFromWindowPtr()#include "MSAESelect.h"#pragma segment AppleEvent// -----------------------------------------------------------------------//		Name: 		DoDelete//		Purpose:	Performs a delete text operation on the direct object,//					if there is no direct object then delete the current//					selection// -----------------------------------------------------------------------	 pascal OSErr	DoDelete(const AppleEvent *theAppleEvent, AppleEvent *reply, long refcon){#ifdef __MWERKS__	#pragma unused (reply, refcon)#endif	AEDesc		directObj = {typeNull, NULL};	TextToken	aTextToken;	short		ignore;	OSErr		err;	err = AEGetParamDesc(theAppleEvent, keyDirectObject, typeWildCard, &directObj);	if (noErr != err) goto done;		if (directObj.descriptorType != typeNull)		err = DeleteDesc(&directObj);	else	{			// Just delete the selection of the front window		err = GetWindowSelection(FrontWindow(), &aTextToken, &ignore);		if (noErr != err) goto done;				err = DeleteTextToken(&aTextToken);	}done:		(void)AEDisposeDesc(&directObj);			return(err);} // DoDeleteEditOSErr	DeleteTextToken(TextToken* theToken){	WindowPtr		aWindow;	DPtr			docPtr;	TextToken		oldSelection;	short			oldLength,					insertLength;				// Should be zero	OSErr			err;		if (! theToken->tokenLength)		// Pointless deleting nothing		return(noErr);			aWindow = theToken->tokenWindow;	docPtr = DPtrFromWindowPtr(theToken->tokenWindow);		if (! aWindow || ! docPtr)		return(errAENoSuchObject);		err = GetWindowSelection(aWindow, &oldSelection, &oldLength);	if (noErr != err) goto done;	err = SelectTextToken(theToken);	if (noErr != err) goto done;	TEDelete(docPtr->theText); 					docPtr->dirty = true;	AdjustScrollbars(docPtr, false);	DrawPageExtras(docPtr);		err = UpdateSelectionToken(theToken, &oldSelection, oldLength, &insertLength);	done:	return(err);}OSErr	DeleteTextDesc(AEDesc* textDesc){	TextToken		aTextToken;	Size			actualSize;	OSErr			err;	if (typeMyText != textDesc->descriptorType)		return(errAETypeError);			GetRawDataFromDescriptor(textDesc, (Ptr)&aTextToken, sizeof(aTextToken), &actualSize);	err = DeleteTextToken(&aTextToken);		return(err);}OSErr	DeleteDesc(AEDesc* aDesc){	AEDesc		deleteDesc = {typeNull, NULL},				textDesc = {typeNull, NULL},				itemDesc = {typeNull, NULL};	long		index;	DescType	theAEKeyword;	OSErr		err;		if (typeObjectSpecifier == aDesc->descriptorType)		err = AEResolve(aDesc, kAEIDoMinimum, &deleteDesc);	else if (typeNull != aDesc->descriptorType)		err = AEDuplicateDesc(aDesc, &deleteDesc);			if (noErr != err) goto done;		switch (deleteDesc.descriptorType)	{		case typeAEList:			err = AECountItems(&deleteDesc, &index);			if (noErr != err) goto done;						for (; index > 0; index--)			// Do backwards or offsets will screw up.			{									// Perhaps we should sort this list first.												// Can a query double up on offsets?				err = AEGetNthDesc(&deleteDesc, index, typeWildCard, &theAEKeyword, &itemDesc);				if (noErr != err) goto done;								err = DeleteDesc(&itemDesc);	// Call recursively				if (noErr != err) goto done;				(void)AEDisposeDesc(&itemDesc);			}			break;					default:			err = AECoerceDesc(&deleteDesc, typeMyText, &textDesc);			if (noErr != err) goto done;			err = DeleteTextDesc(&textDesc);	}	done:	(void)AEDisposeDesc(&deleteDesc);	(void)AEDisposeDesc(&textDesc);	(void)AEDisposeDesc(&itemDesc);		return(err);}