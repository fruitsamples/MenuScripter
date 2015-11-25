// MSAEMenuUtils.c//// Original version by Jon Lansdell and Nigel Humphreys.// 4.0 and 3.1 updates by Greg Sutton.// �Apple Computer Inc 1996, all rights reserved.#include "MSAEMenuUtils.h"#include <TextUtils.h>#ifdef THINK_C	#include "PLStrs.h"#else	#include <PLStringFuncs.h>#endif#include <AEPackObject.h>#include "MSGlobals.h"#include "MSAEWindowUtils.h"OSErr	MenuNameToMenuToken( StringPtr theName, MenuToken *theToken ){	short   index,			numMenus = CountMenus( );		for ( index = appleM; index <= numMenus ; index++ )	{		if ( IdenticalString( theName, (**(myMenus[index])).menuData, NULL ) == 0 )		{			theToken->tokenMenu = myMenus[index];			theToken->tokenID = index + appleID;			return noErr;		}	}	return errAENoSuchObject;}	OSErr	GetDescOfNamedMenu( StringPtr theName, AEDesc* result ){	MenuToken		theToken;	OSErr			err = noErr;		err = MenuNameToMenuToken( theName, &theToken );	if ( noErr != err ) goto done;		err = AECreateDesc( typeMyMenu, (Ptr)&theToken, sizeof( theToken ), result );done:	return err;}OSErr	GetDescOfNthMenu( short theIndex, AEDesc* result ){	MenuToken		theToken;	OSErr			err = noErr;		if ( theIndex <= CountMenus( ) )	{		theToken.tokenMenu = myMenus[theIndex];		theToken.tokenID = theIndex + appleID;		err = AECreateDesc( typeMyMenu, (Ptr)&theToken, sizeof( theToken ), result );	}	else		err = errAEIllegalIndex;	return err;}short	CountMenus( void ){	if ( CountDocuments( ) > 0 )		return kLastMenu + 1;	else		return editM + 1;}void	GetMenuName( MenuToken* theToken, StringPtr theResult ){	if ( theResult )		PLstrcpy( theResult, (**theToken->tokenMenu).menuData );	// Destination first}OSErr	MenuItemNameToMenuItemToken( MenuToken* containerToken, StringPtr theName, MenuItemToken *theToken ){	short   index,			numItems = CountMenuTokenItems( containerToken );	Str255	pStr;		for ( index = 1; index <= numItems ; index++ )	{		GetMenuItemText( containerToken->tokenMenu, index, pStr );			if ( IdenticalString( theName, pStr, NULL ) == 0 )		{			theToken->tokenMenuToken = *containerToken;			theToken->tokenItem = index;			return noErr;		}	}	return errAENoSuchObject;}	OSErr	GetDescOfNamedMenuItem( MenuToken* containerToken, StringPtr theName, AEDesc* result ){	MenuItemToken	theToken;	OSErr			err;		err = MenuItemNameToMenuItemToken( containerToken, theName, &theToken );	if ( noErr != err ) goto done;		err = AECreateDesc( typeMyMenuItem, (Ptr)&theToken, sizeof( theToken ), result );done:	return err;}OSErr	GetDescOfNthMenuItem( MenuToken* containerToken, short theIndex, AEDesc* result ){	MenuItemToken	aToken;	OSErr			err = noErr;		if ( theIndex <= CountMenuTokenItems( containerToken ) )	{		aToken.tokenMenuToken = *containerToken;		aToken.tokenItem = theIndex;		err = AECreateDesc( typeMyMenuItem, (Ptr)&aToken, sizeof( aToken ), result );	}	else		err = errAEIllegalIndex;	return err;}short	CountMenuTokenItems( MenuToken* containerToken ){	return CountMItems( containerToken->tokenMenu );}void	GetMenuItemName( MenuItemToken* theToken, StringPtr theResult ){	GetMenuItemText( theToken->tokenMenuToken.tokenMenu,										theToken->tokenItem, theResult );}void	SetMenuItemName( MenuItemToken* theToken, StringPtr theResult ){	SetMenuItemText( theToken->tokenMenuToken.tokenMenu,										theToken->tokenItem, theResult );}OSErr	MakeMenuSpecifier( MenuToken* theToken, AEDesc* theResult ){	AEDesc		nullSpec = {typeNull, NULL},				aDesc = {typeNull, NULL};	Str255		aPStr;	OSErr		anErr;		GetMenuName( theToken, aPStr );		anErr = AECreateDesc( typeChar, (Ptr)&aPStr[1], aPStr[0], &aDesc );	if ( noErr != anErr ) goto done;		anErr = CreateObjSpecifier( cMenu, &nullSpec, formName,											&aDesc, false, theResult );	done:	(void)AEDisposeDesc( &aDesc );	return anErr;}OSErr	MakeMenuItemSpecifier( MenuItemToken* theToken, AEDesc* theResult ){	AEDesc		menuSpec = {typeNull, NULL},				aDesc = {typeNull, NULL};	Str255		aPStr;	OSErr		anErr;		anErr = MakeMenuSpecifier( &theToken->tokenMenuToken, &menuSpec );	if ( noErr != anErr ) goto done;		GetMenuItemName( theToken, aPStr );		anErr = AECreateDesc( typeChar, (Ptr)&aPStr[1], aPStr[0], &aDesc );	if ( noErr != anErr ) goto done;		anErr = CreateObjSpecifier( cMenuItem, &menuSpec, formName,													&aDesc, false, theResult );done:	(void)AEDisposeDesc( &aDesc );	(void)AEDisposeDesc( &menuSpec );		return anErr;}MenuHandle	MenuHandleFromMenuID( short theMenuID ){	long		anIndex;	MenuHandle	aResult = NULL;	anIndex = theMenuID - appleID;	if ( anIndex >= 0 && anIndex <= kLastMenu )		aResult = myMenus[anIndex];		return aResult;	}// Scripts are stored in a resource with the ID determined by the  menu and item//  (see top of MSScript.c) this routine converts this resource ID back to a token.void	MenuTokenFromResID( short theResID, MenuToken* theToken ){	theToken->tokenID = theResID / 32;	theToken->tokenMenu = MenuHandleFromMenuID( theToken->tokenID );}void	MenuItemTokenFromResID( short theResID, MenuItemToken* theToken ){	MenuTokenFromResID( theResID, &theToken->tokenMenuToken );	theToken->tokenItem = theResID % 32;}