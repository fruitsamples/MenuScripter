#pragma once#include "MSToken.h"OSErr	MenuNameToMenuToken( StringPtr theName, MenuToken *theToken );OSErr	GetDescOfNamedMenu( StringPtr theName, AEDesc* result );OSErr	GetDescOfNthMenu( short theIndex, AEDesc* result );short	CountMenus( void );void	GetMenuName( MenuToken* theToken, StringPtr theResult );void	SetMenuItemName( MenuItemToken* theToken, StringPtr theResult );OSErr	MenuItemNameToMenuItemToken( MenuToken* containerToken,									StringPtr theName, MenuItemToken *theToken );OSErr	GetDescOfNamedMenuItem( MenuToken* containerToken, StringPtr theName, AEDesc* result );OSErr	GetDescOfNthMenuItem( MenuToken* containerToken, short theIndex, AEDesc* result );short	CountMenuTokenItems( MenuToken* containerToken );void	GetMenuItemName( MenuItemToken* theToken, StringPtr theResult );OSErr	MakeMenuSpecifier( MenuToken* theToken, AEDesc* theResult );OSErr	MakeMenuItemSpecifier( MenuItemToken* theToken, AEDesc* theResult );void	MenuTokenFromResID( short theResID, MenuToken* theToken );void	MenuItemTokenFromResID( short theResID, MenuItemToken* theToken );MenuHandle	MenuHandleFromMenuID( short theMenuID );