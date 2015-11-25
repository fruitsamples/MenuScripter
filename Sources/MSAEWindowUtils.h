// MSWindowUtils.h//// Original version by Jon Lansdell and Nigel Humphreys.// 4.0 and 3.1 updates by Greg Sutton.// �Apple Computer Inc 1996, all rights reserved.#pragma once#include "MSToken.h"#include <Windows.h>OSErr		GetDescOfNamedWindow(StringPtr nameStr, AEDesc* result);OSErr		GetDescOfNamedDocument( StringPtr nameStr, AEDesc* result );OSErr		GetDescOfNthWindow( short index, AEDesc* result );OSErr		GetDescOfNthDocument( short index, AEDesc* result );long		GetWindowIndex( WindowPtr theWindow );OSErr		SetWindowIndex( WindowPtr theWindow, long theIndex );long		GetDocumentIndex( WindowPtr theWindow );OSErr		SetDocumentIndex( WindowPtr theWindow, long theIndex );Boolean		IsDocumentWindow( WindowPtr theWindow );Boolean		IsResultsWindow( WindowPtr theWindow );short		CountWindows(void);short 		CountDocuments( void );WindowPtr	GetNthWindow( long index );WindowPtr	GetNthDocument( long index );WindowPtr	GetNamedWindow( StringPtr theName );WindowPtr	GetNamedDocument( StringPtr theName );Boolean		IsVisible( WindowPtr theWindow );OSErr		GetWindowBounds( WindowPtr theWindow, Rect* theRect );