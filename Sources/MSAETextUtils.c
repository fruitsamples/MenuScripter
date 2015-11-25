// MSAETextUtils.c//// Original version by Jon Lansdell and Nigel Humphreys.// 4.0 and 3.1 updates by Greg Sutton.// �Apple Computer Inc 1996, all rights reserved./*	14-Nov-95 : GS : Removed reliance on compiler setting local variable to zero						in GetInsertDescFromInsertHere().*/#include "MSAETextUtils.h"#include "MSAEUtils.h"#include "MSWindow.h"		// for DPtrFromWindowPtr()#include "MSAERecording.h"#include <AEPackObject.h>// ----------------------------------------------------------------------------------//	Name:    PutStyledTextFromDescIntoTEHandle//	Purpose: Takes the text in an AEDesc containing typeIntlText and puts it in//	         a styled text edit record at the current insertion point.//					 Looks for typeIntlText, typeStyledText, typeChar in that order.// ----------------------------------------------------------------------------------OSErr	PutStyledTextFromDescIntoTEHandle(const AEDesc *sourceTextDesc, TEHandle theHTE){	AEDesc styledTextDesc = { typeNull, NULL };	AEDesc textStyleDesc = { typeNull, NULL };	AEDesc rawTextDesc = { typeNull, NULL };	OSErr  myErr;		//	Coerce to an AERecord and then extract the parts of the	//	styled text - works for typeIntlText, typeStyledText	myErr = AECoerceDesc(sourceTextDesc, typeAERecord, &styledTextDesc);		if (noErr == myErr)	{				myErr = AEGetKeyDesc(&styledTextDesc,  keyAEText,											typeChar, &rawTextDesc);												 		myErr = AEGetKeyDesc(&styledTextDesc,  keyAEStyles,											typeScrapStyles, &textStyleDesc);	}	else	{		myErr = AECoerceDesc(sourceTextDesc, typeChar, &rawTextDesc);				textStyleDesc.dataHandle = NULL; // so that TEStylInsert acts like TEInsert					}				HLock((Handle)rawTextDesc.dataHandle);		TEDelete(theHTE);					// Insert over current selection	TEStylInsert((const void *) (*rawTextDesc.dataHandle),							 			GetHandleSize(rawTextDesc.dataHandle),							 				(StScrpHandle) textStyleDesc.dataHandle,							 					theHTE);							 	HUnlock((Handle)rawTextDesc.dataHandle);		(void)AEDisposeDesc(&textStyleDesc);	(void)AEDisposeDesc(&rawTextDesc);	(void)AEDisposeDesc(&styledTextDesc);			return(myErr);}TEHandle	TEHandleFromWindow(WindowPtr theWindow){	DPtr		docPtr;	TEHandle	result = NULL;		if (! theWindow)		return(NULL);		docPtr = DPtrFromWindowPtr(theWindow);		if (docPtr)		result = docPtr->theText;			return(result);}TEHandle	TEHandleFromTextToken(TextToken* aToken){	if (! aToken)		return(NULL);			return(TEHandleFromWindow(aToken->tokenWindow));}OSErr	GetInsertDescFromInsertHere(AEDesc* insertHereDesc, AEDesc* insertDesc, DescType* insertType){	AEDesc		insertRec = {typeNull, NULL},				objectSpec = {typeNull, NULL};	DescType	returnedType;	Size		actualSize;	OSErr		err = noErr; 	switch (insertHereDesc->descriptorType) 	{ 		case typeInsertionLoc: 			err = AECoerceDesc(insertHereDesc, typeAERecord, &insertRec); 			if (noErr != err) goto done; 			 			err = AEGetKeyPtr(&insertRec, keyAEPosition, typeEnumeration, &returnedType, 										(Ptr)insertType, sizeof(insertType), &actualSize); 			if (noErr != err) goto done;			err = AEGetKeyDesc(&insertRec, keyAEObject, typeWildCard, &objectSpec);			if (objectSpec.descriptorType != typeNull)			{				err = AEResolve(&objectSpec, kAEIDoMinimum, insertDesc);				if (err != noErr) goto done;			} 			break; 	 		case typeObjectSpecifier: 			err = AEResolve(insertHereDesc, kAEIDoMinimum, insertDesc); 			if (noErr != err) goto done; 			*insertType = insertDesc->descriptorType; 			break; 			 		case typeNull:					// No insertion location given 			*insertType = typeNull; 			break; 			 		case typeType: 			*insertType = *(DescType *)*insertHereDesc->dataHandle; 			break; 			 		default:						// Just copy the descriptor 			err = AEDuplicateDesc(insertHereDesc, insertDesc); 			if (noErr != err) goto done; 			*insertType = insertDesc->descriptorType; 	} 	 done: 	if (insertRec.dataHandle) 		AEDisposeDesc(&insertRec); 	if (objectSpec.dataHandle) 		AEDisposeDesc(&objectSpec); 	 	return(err);}// This routine returns an enumerated type describing the relative position// of one TextToken to another.TokenWithinType	TokenWithinToken(TextToken* container, TextToken* token, short* numPartial){	TokenWithinType		result;	if (token->tokenOffset + token->tokenLength < container->tokenOffset)		result = kTokenBefore;	else if (container->tokenOffset + container->tokenLength < token->tokenOffset)		result = kTokenAfter;	else if (token->tokenOffset >= container->tokenOffset				&& token->tokenOffset + token->tokenLength <= container->tokenOffset + container->tokenLength)		result = kTokenWithin;	else if (token->tokenOffset < container->tokenOffset)	{		result = kTokenPartialBefore;		if (numPartial)			*numPartial = token->tokenOffset + token->tokenLength - container->tokenOffset;	}	else	{		result = kTokenPartialAfter;		if (numPartial)			*numPartial = container->tokenOffset + container->tokenLength - token->tokenOffset;	}		return(result);}OSErr	TextTokenFromDocumentToken(WindowToken* theWindowToken, TextToken* theTextToken){	DPtr		docPtr;	docPtr = DPtrFromWindowPtr(theWindowToken->tokenWindow);	if (! docPtr)		return(errAENoSuchObject);			// Create our text token	theTextToken->tokenWindow = theWindowToken->tokenWindow;		theTextToken->tokenOffset = 1;								// Start at 1	theTextToken->tokenLength = (**docPtr->theText).teLength;	// through whole length		return(noErr);}OSErr	TextTokenFromDocumentDesc(AEDesc* windowDesc, TextToken* theToken){	AEDesc			aDesc = {typeNull, NULL};	WindowToken		aWindowToken;	Size			actualSize;	OSErr			err;		err = AECoerceDesc(windowDesc, typeMyDocument, &aDesc);	if (noErr != err) goto done;			GetRawDataFromDescriptor(&aDesc, (Ptr)&aWindowToken,									sizeof(aWindowToken), &actualSize);	err = TextTokenFromDocumentToken(&aWindowToken, theToken);	done:		(void)AEDisposeDesc(&aDesc);	return(err);}OSErr	TextDescFromDocumentToken(WindowToken* theWindowToken, AEDesc* textDesc){	TextToken	aToken;	OSErr		err;		err = TextTokenFromDocumentToken(theWindowToken, &aToken);	if (noErr != err) goto done;		err = AECreateDesc(typeMyText, (Ptr)&aToken, sizeof(aToken), textDesc);done:	return(err);}OSErr	TextDescFromDocumentDesc(AEDesc* windowDesc, AEDesc* textDesc){	TextToken	aToken;	OSErr		err;		err = TextTokenFromDocumentDesc(windowDesc, &aToken);	if (noErr != err) goto done;		err = AECreateDesc(typeMyText, (Ptr)&aToken, sizeof(aToken), textDesc);done:	return(err);}void MoveToNonSpace(short *start, short limit, charsHandle myChars)	// Treats space, comma, full stop, ; and : as space chars{ 	short x;	while (*start <= limit) {	  x = (**myChars)[*start];		if (IsWhiteSpace(x))			(*start) +=1;		else			return;	}}	void	MoveToSpace(short *start, short limit, charsHandle myChars)	// Treats space,comma, full stop, ; and : as space chars{ 	short x;		while (*start <= limit)	{		x = (**myChars)[*start];		if (! IsWhiteSpace(x))			(*start)++;		else			return;	}}void	MoveToEndOfParagraph(short *start, short limit, charsHandle myChars)	//	Treats CR as end of paragraph{ 	short x;		while (*start <= limit)	{		x = (**myChars)[*start];		if (! IsParagraphDelimiter(x))		// had x != CR			(*start)++;		else			return;	}}// This routine counts the given elementType between startAt and OSErr	CountTextElements(TEHandle inTextHandle, short startAt,								short forHowManyChars, DescType elementType, short* result){	charsHandle	theChars;	short       limit,				start;	OSErr		err = noErr;	switch (elementType)	{		case cInsertionPoint:	// Always one more insertion location than characters			*result = forHowManyChars + 1;			break;					case cChar:			// Easy			*result = forHowManyChars;			break;					case cText:				*result = 1;			break;				case cWord:			// Cycle through - counting		case cParagraph:			theChars = (charsHandle)(**inTextHandle).hText;			start = startAt - 1;					// Convert to zero based			limit = start + forHowManyChars - 1;	// when passed one based			*result	= 0;			MoveToNonSpace(&start, limit, theChars);			while (start <= limit)			{				(*result)++;				switch (elementType)				{					case cWord:						MoveToSpace(&start, limit, theChars);						break;											case cParagraph:						MoveToEndOfParagraph(&start, limit, theChars);						break;				}				MoveToNonSpace(&start, limit, theChars);			}			break;			default:			*result = -1;			err = errAEBadKeyForm;	}		return(err);} // CountTextElementsOSErr	GetDescOfNthTextElement(short index, DescType elementType,										TextToken* containerToken, AEDesc* result){	DPtr        docPtr;	TextToken	theToken;	short		start,				maxChars,				elementCount,				limit,				elementStart;	charsHandle	theChars;	OSErr		err;		if (! containerToken)		return(errAEEmptyListContainer);	docPtr = DPtrFromWindowPtr(containerToken->tokenWindow);	start = containerToken->tokenOffset - 1;	// Zero based	maxChars = containerToken->tokenLength;	err = CountTextElements(docPtr->theText, containerToken->tokenOffset,										maxChars, elementType, &elementCount);	if (noErr != err) return(err);		if (index < 0)						// Change a negative index to positive		index = elementCount + index + 1;			if (index > elementCount)			// Got given an index out of range		return(errAEIllegalIndex);					// Set the window that the token relates to	theToken.tokenWindow = containerToken->tokenWindow;	switch (elementType)	{		case cInsertionPoint:			theToken.tokenOffset = start + index - 1;			theToken.tokenLength = 0;			break;					case cChar:		// Easy - just the start point + the index			theToken.tokenOffset = start + index;			theToken.tokenLength = 1;			break;					case cText:				theToken.tokenOffset = start + index;			theToken.tokenLength = maxChars;			break;					case cWord:		case cParagraph:			theChars = (charsHandle)(**(docPtr->theText)).hText;			limit = start + maxChars - 1;			MoveToNonSpace(&start, limit, theChars);			while ((start <= limit) && (index > 0))			{				index--;				elementStart = start;				switch (elementType)				{					case cWord:						MoveToSpace(&start, limit, theChars);						break;											case cParagraph:						MoveToEndOfParagraph(&start, limit, theChars);						break;				}				theToken.tokenLength = start - elementStart;				MoveToNonSpace(&start, limit, theChars);			}			theToken.tokenOffset = elementStart + 1;	// Convert to one based			break;	}	err = AECreateDesc(typeMyText, (Ptr)&theToken, sizeof(theToken), result);	return(err);}char	GetTEHChar(TEHandle aTEH, short offset){	char	result;		offset--;		// This is now 0 based	if (offset < 0 || offset >= (*aTEH)->teLength)		return('\0');			result = *(char *)((*(**aTEH).hText) + offset);		return(result);}Boolean		IsAtStart(TextToken* theToken){	Boolean	result;					// Is at start if offset is at 1	result = (theToken->tokenOffset == 1);		return(result);}Boolean		IsAtEnd(TextToken* theToken){	TEHandle	aTEH;	Boolean		result;		aTEH = TEHandleFromTextToken(theToken);					// Does it go to the end?	result = (theToken->tokenOffset + theToken->tokenLength >= (**aTEH).teLength);		return(result);}Boolean		IsWhiteSpace(short aChar){	Boolean	result;	result = (aChar == ' ' || aChar == ',' || aChar == '.'				|| aChar == ':' || aChar == LF || aChar == CR);		     	return(result);}Boolean		IsParagraphDelimiter(short aChar){	Boolean	result;	result = (aChar == CR);		     	return(result);}Boolean		IsContentsToken(TextToken* theToken){	return(IsAtStart(theToken) && IsAtEnd(theToken));}Boolean		IsParagraphToken(TextToken* theToken, short* start, short* end){	TEHandle	aTEH;	OSErr		err;	short		number;	Boolean		fStart,				fEnd,				result;		aTEH = TEHandleFromTextToken(theToken);			fStart = IsAtStart(theToken) || IsParagraphDelimiter(GetTEHChar(aTEH, theToken->tokenOffset - 1));	fEnd = IsAtEnd(theToken) || IsParagraphDelimiter(GetTEHChar(aTEH, theToken->tokenOffset + theToken->tokenLength));		if (fStart && fEnd)	{		// need to do a count of the paragraphs				err = CountTextElements(aTEH, theToken->tokenOffset,							theToken->tokenLength, cParagraph, &number);		// count text elements before it i.e. offset == 0 limit == theToken->tokenOffset				if (IsAtStart(theToken))			*start = 1;		else		{				// From beginning to charracter before start of paragraph			err = CountTextElements(aTEH, 1,theToken->tokenOffset - 1, cParagraph, start);			(*start)++;		}				*end = *start + number - 1;				result = true;	}	else		result = false;		return(result);}Boolean		IsWordToken(TextToken* theToken, short* start, short* end){	TEHandle	aTEH;	OSErr		err;	short		number;	Boolean		fStart,				fEnd,				result;			aTEH = TEHandleFromTextToken(theToken);			fStart = IsAtStart(theToken) || IsWhiteSpace(GetTEHChar(aTEH, theToken->tokenOffset - 1));	fEnd = IsAtEnd(theToken) || IsWhiteSpace(GetTEHChar(aTEH, theToken->tokenOffset + theToken->tokenLength));		if (fStart && fEnd)	{		// need to do a count of the words				err = CountTextElements(aTEH, theToken->tokenOffset,							theToken->tokenLength, cWord, &number);		// count text elements before it i.e. offset == 0 limit == theToken->tokenOffset				if (IsAtStart(theToken))			*start = 1;		else		{				// From beginning to charracter before start of word			err = CountTextElements(aTEH, 1, theToken->tokenOffset - 1, cWord, start);			(*start)++;		}				*end = *start + number - 1;				result = true;	}	else		result = false;		return(result);}DescType	GetTextTokenType(TextToken* theToken, short* start, short* end){	DescType	result;		*start = *end = -1;					// Just set to the same value	if (! theToken->tokenLength)	{		result = cInsertionPoint;	}	else if (IsContentsToken(theToken))	{		result = pContents;	}	else if (IsParagraphToken(theToken, start, end))	{		result = cParagraph;	}	else if (IsWordToken(theToken, start, end))	{		result = cWord;	}	else	{		result = cChar;		*start = theToken->tokenOffset;		*end = theToken->tokenOffset + theToken->tokenLength - 1;	}		return(result);}OSErr	MakeContentsSpecifier(TextToken* theToken, AEDesc* result){	AEDesc		docSpec = {typeNull, NULL},				contentsDesc = {typeNull, NULL};	DescType	propertyID;	OSErr		err;	err = MakeDocumentObj(theToken->tokenWindow, &docSpec);	if (noErr != err) goto done;		propertyID = pContents;	err = AECreateDesc(typeType, (Ptr)&propertyID, sizeof(DescType), &contentsDesc);	if (err != noErr) goto done;	err = CreateObjSpecifier(cProperty, &docSpec, formPropertyID, &contentsDesc, false, result);done:	(void)AEDisposeDesc(&docSpec);	(void)AEDisposeDesc(&contentsDesc);		return(err);}OSErr	MakeAbsoluteTextSpecifier(WindowPtr theWindow, DescType textType, long index, AEDesc* result){	AEDesc		docSpec = {typeNull, NULL},				absoluteDesc = {typeNull, NULL};	OSErr		err;		if (theWindow)	{		err = MakeDocumentObj(theWindow, &docSpec);		if (noErr != err) goto done;	}	// else just use the NULL'ed value	err = AECreateDesc(typeLongInteger, (Ptr)&index, sizeof(index), &absoluteDesc);	if (err != noErr) goto done;	err = CreateObjSpecifier(textType, &docSpec, formAbsolutePosition,													&absoluteDesc, false, result);done:	(void)AEDisposeDesc(&docSpec);	(void)AEDisposeDesc(&absoluteDesc);		return(err);}OSErr	MakeInsertionPointSpecifier(TextToken* theToken, AEDesc* result){	AEDesc		relativeToSpec = {typeNull, NULL},				relativeDesc = {typeNull, NULL};	DescType	relativeType;	OSErr		err;	if (IsAtStart(theToken))			// Before contents (whether there are any or not)	{		relativeType = kAEPrevious;		err = MakeContentsSpecifier(theToken, &relativeToSpec);	}	else if (IsAtEnd(theToken))			// After last character	{		relativeType = kAENext;		//err = MakeContentsSpecifier(theToken, &relativeToSpec);		err = MakeAbsoluteTextSpecifier(theToken->tokenWindow, cChar, -1, &relativeToSpec);	}	else								// Has a character it can go before	{		relativeType = kAEPrevious;		err = MakeAbsoluteTextSpecifier(theToken->tokenWindow, cChar, theToken->tokenOffset, &relativeToSpec);	}	if (noErr != err) goto done;	  	err = AECreateDesc(typeEnumerated, &relativeType, sizeof(relativeType), &relativeDesc);	if (noErr != err) goto done;	err = CreateObjSpecifier(cInsertionPoint, &relativeToSpec, formRelativePosition,	                           									&relativeDesc, false, result);done:	(void)AEDisposeDesc(&relativeToSpec);	(void)AEDisposeDesc(&relativeDesc);	return(err);}OSErr	GetIndexSpecifier(TextToken* theToken, DescType textType, long index, AEDesc* result){	OSErr	err;	switch (textType)	{		case cInsertionPoint:			err = MakeInsertionPointSpecifier(theToken, result);			break;					case pContents:			err = MakeContentsSpecifier(theToken, result);			break;					case cParagraph:		case cWord:		case cChar:			err = MakeAbsoluteTextSpecifier(theToken->tokenWindow, textType, index, result);			break;				default:			err = errAETypeError;	}		return(err);}OSErr	GetTextTokenObjectSpecifier(TextToken* theToken, AEDesc* result){	AEDesc		docSpec = {typeNull, NULL},				startSpec = {typeNull, NULL},				endSpec = {typeNull, NULL},				rangeDesc = {typeNull, NULL};	DescType	textType;	short		start,				end;	OSErr		err;		textType = GetTextTokenType(theToken, &start, &end);		err = GetIndexSpecifier(theToken, textType, start, &startSpec);	if (noErr != err) goto done;	if (start != end)		// Sort out rest of range specifier	{		err = GetIndexSpecifier(theToken, textType, end, &endSpec);		if (noErr != err) goto done;				err = CreateRangeDescriptor(&startSpec, &endSpec, false, &rangeDesc);		if (noErr != err) goto done;		err = MakeDocumentObj(theToken->tokenWindow, &docSpec);		if (noErr != err) goto done;		err = CreateObjSpecifier(cText, &docSpec, formRange, &rangeDesc, false, result);	}	else		err = AEDuplicateDesc(&startSpec, result);	done:	(void)AEDisposeDesc(&docSpec);	(void)AEDisposeDesc(&startSpec);	(void)AEDisposeDesc(&endSpec);	(void)AEDisposeDesc(&rangeDesc);	return(err);}