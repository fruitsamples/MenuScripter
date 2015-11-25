// MSGXPrinting.c//// Original version by Jon Lansdell and Nigel Humphreys.// 4.0 and 3.1 updates by Greg Sutton.// GX printing by Don Swatman// �Apple Computer Inc 1996, all rights reserved.// Adds GX printing functionality to MenuScripter/*		Changes for 4.0	29-Feb-96 : GS : Changed <Graphics routines.h> to <GXGraphics.h>										Changed <graphics toolbox.h> to <GXEnvironment.h>*/#include <Gestalt.h>#ifdef __MWERKS__	#include <Graphics routines.h>	#include <graphics toolbox.h>#else	#include <GXGraphics.h>	#include <GXPrinting.h>#endif#include <GXEnvironment.h>#include "QDLibrary.h"#include "MSGXPrinting.h"#include "MSWindow.h"#include "MSGlobals.h"#include "MSMain.h"#include "MSAEWindowUtils.h"// Constants#define		kGraphicsHeapSize	((long) 300 * 1024)// GlobalsgxGraphicsClient gClient;//-------------------------------------------------------//       InitGXIfPresent//// This uses Gestalt() to see if QuickDraw GX is present.// If it is, it then initialises the GX managers.// If it isn't, then it removes the GX file menu item(s).//-------------------------------------------------------#pragma segment Mainvoid InitGXIfPresent(void){	long gxVersion;	long gxPrintVersion;	gGXIsPresent = false;// Check to see whether QuickDraw GX is available.	if (Gestalt(gestaltGXVersion, &gxVersion) == noErr)		if (Gestalt(gestaltGXPrintingMgrVersion, &gxPrintVersion) == noErr)			gGXIsPresent = true;	if (gGXIsPresent)	{// Initialize QuickDraw GX.		gClient = GXNewGraphicsClient(nil, kGraphicsHeapSize, (gxClientAttribute) 0);		GXEnterGraphics();		GXInitPrinting();	}	else	{// No QuickDraw GX, so remove extra file menu item(s)		DelMenuItem ( myMenus[fileM], fmPrintOne);	}}//-------------------------------------------------------//       CleanUpGXIfPresent//// If GX was installed (i.e. gGXIsPresent) then remove// the GX managers.//-------------------------------------------------------#pragma segment Mainvoid CleanUpGXIfPresent(void){	if (gGXIsPresent)	{		GXExitPrinting();		GXDisposeGraphicsClient(gClient);		GXExitGraphics();	}}//-------------------------------------------------------//       ConvertMenuActualToGXMenu//// This converts an item number as returned by MenuKey()// or MenuSelect() to the value used internally. The reason// you have to this is because a non gx application has// less items in the file menu.//-------------------------------------------------------#pragma segment Mainshort ConvertMenuActualToGXMenu ( short theItem ){	short menuItem;		menuItem = theItem;		if (!gGXIsPresent)	{		if (theItem == fmNoGXPrint)			menuItem = fmPrint;		else			if (theItem == fmNoGXQuit)				menuItem = fmQuit;	}	return menuItem;}//-------------------------------------------------------//       GetRectOfPage//// This returns a QuickDraw rect for the current printer page.// If gx is not present if uses rPage from the document's THPrint.// If gx is present then we get a GXRectangle from the requested// page's format. It then converts this to a Rect. This could be// extended to handle custom formated pages.//-------------------------------------------------------#pragma segment Mainvoid GetRectOfPage( DPtr  theDoc,										Rect  *pageRect ){	gxFormat		pageFormat;	gxRectangle pageSize;	gxRectangle paperSize;	if (gGXIsPresent)	{			pageFormat = GXGetJobFormat(theDoc->documentJob, 1);			GXGetFormatDimensions(pageFormat, &pageSize, &paperSize);	// Convert to a QD Rect			FixedRectToShort( &pageSize, pageRect );	}	else		*pageRect = (*(theDoc->thePrintSetup))->prInfo.rPage;	OffsetRect( pageRect, -pageRect->left, -pageRect->top);}//-------------------------------------------------------//       AdjustMenusForGXPrintDialogs//// This enables or disables menu's when the GX print dialog// is being displayed.//-------------------------------------------------------#pragma segment GXPrintSegvoid AdjustMenusForGXPrintDialogs( Boolean dialogGoingUp ){    Boolean redrawMenuBar = false;        if (dialogGoingUp)    {        SetMenuItemState ( false, myMenus[appleM], aboutItem);        redrawMenuBar |= SetMenuItemState ( false, myMenus[fileM], kMenuTitle );        if ( CountDocuments( ) )        {            redrawMenuBar |= SetMenuItemState ( false, myMenus[fontM],  kMenuTitle);            redrawMenuBar |= SetMenuItemState ( false, myMenus[sizeM],  kMenuTitle);            redrawMenuBar |= SetMenuItemState ( false, myMenus[styleM], kMenuTitle);           	redrawMenuBar |= SetMenuItemState ( false, myMenus[scriptM],     kMenuTitle);            redrawMenuBar |= SetMenuItemState ( false, myMenus[subroutineM], kMenuTitle);            SetMenuItemState ( false, myMenus[editM], selectAllCommand);        }        HiliteMenu(0);    }    else    {				SetMenuItemState ( true, myMenus[appleM], aboutItem);				redrawMenuBar |= SetMenuItemState ( true, myMenus[fileM], kMenuTitle);				redrawMenuBar |= SetMenuItemState ( true, myMenus[editM], kMenuTitle);				if ( CountDocuments( ) )				{			      redrawMenuBar |= SetMenuItemState ( true, myMenus[fontM],  kMenuTitle);			      redrawMenuBar |= SetMenuItemState ( true, myMenus[sizeM],  kMenuTitle);			      redrawMenuBar |= SetMenuItemState ( true, myMenus[styleM], kMenuTitle);			           	redrawMenuBar |= SetMenuItemState ( true, myMenus[scriptM],     kMenuTitle);            redrawMenuBar |= SetMenuItemState ( true, myMenus[subroutineM], kMenuTitle);			      SetMenuItemState ( true, myMenus[editM], selectAllCommand);				}				MaintainMenus( &redrawMenuBar );    }  	if (redrawMenuBar)			DrawMenuBar();}//-------------------------------------------------------//       SetupGXEditMenuRec//// Sets up the gxEditMenuRecord. This is used by the// GX dialogs so that they know where the items in the// Edit Menu are//-------------------------------------------------------#pragma segment GXPrintSegvoid SetupGXEditMenuRec(gxEditMenuRecord *editMenuRec)	{		editMenuRec->editMenuID = editID;		editMenuRec->cutItem    =   cutCommand;		editMenuRec->copyItem   =   copyCommand;		editMenuRec->pasteItem  =   pasteCommand;		editMenuRec->clearItem  =   clearCommand;		editMenuRec->undoItem   =   undoCommand;	}//-------------------------------------------------------//       DoGXPageSetup////  Puts up a GX Page Setup dialog//-------------------------------------------------------#pragma segment GXPrintSegBoolean DoGXPageSetup ( DPtr theDoc )	{	    Boolean          result = false;			OSErr            theErr;			gxDialogResult   dialogResult;			gxEditMenuRecord editMenuRec;						if (gGXIsPresent)				if (theDoc)				{					AdjustMenusForGXPrintDialogs(true);					SetupGXEditMenuRec( &editMenuRec );// Display the Page Setup dialog box.					dialogResult = GXJobDefaultFormatDialog ( theDoc->documentJob, &editMenuRec);					theErr = GXGetJobError(theDoc->documentJob);					AdjustMenusForGXPrintDialogs(false);// Return True, if there are no Errors and the OK button was clicked					result = ((theErr == noErr) && (dialogResult == gxOKSelected));				}		return(result);	}//-------------------------------------------------------//       struct GXPrintSpoolDataRec////  Used to pass information during printing//-------------------------------------------------------typedef struct GXPrintSpoolDataRec{	gxRectangle   pageArea;				// Page rectangle.	gxViewPort    printViewPort;  // View port we're printing in.} GXPrintSpoolDataRec, *GXPrintSpoolDataPtr;//-------------------------------------------------------//       PrintAShape////  Called after shape translation. It checks to see if any shape// it recieves are drawable, the draws it in the GX view port.// Information in the form of a pointer to a GXPrintSpoolDataRec// is passed in the refCon.//-------------------------------------------------------#pragma segment GXPrintSegOSErr PrintAShape(gxShape currentShape, long refCon){	GXPrintSpoolDataPtr spoolData;	gxShapeType         theShapeType;// Get the spool data from the refCon	spoolData = (GXPrintSpoolDataPtr) refCon;// Don't waste time spooling the shape if it's being drawn off the page.	theShapeType = GXGetShapeType(currentShape);	if (   (theShapeType == gxEmptyType)			|| (theShapeType == gxFullType)			|| (theShapeType == gxPictureType)			|| GXTouchesBoundsShape(&spoolData->pageArea, currentShape) )	{// Set the ports and draw it		GXSetShapeViewPorts(currentShape, 1, &spoolData->printViewPort);		GXDrawShape(currentShape);	}	return (OSErr) GXGetGraphicsError(nil);}//-------------------------------------------------------//       DuplicateStyleTERec//// This copies the styled TERec in theDoc. It puts it into// the destPort//-------------------------------------------------------#pragma segment Mainvoid DuplicateStyleTERec( TEHandle  hSourceTE,													TEHandle *hDestTE,													Rect     *destRect,													GrafPtr   destPort ){	GrafPtr oldPort;	short   oldSelStart;	short   oldSelEnd;	StScrpHandle printerTextStyles;	// Set up the ports	GetPort(&oldPort);	SetPort(destPort);	// Create a temporary Text Edit and copy the windows text edit into it	*hDestTE = TEStyleNew(destRect, destRect);// Select all the text (preserving the previous settings) so that we can use // GetTylScrap (or TEGetStyleScrapHandle ) to get the style of the whole TERec	oldSelStart = (*hSourceTE)->selStart;	oldSelEnd   = (*hSourceTE)->selEnd;	TESetSelect(0,(*hSourceTE)->teLength, hSourceTE);// Get the style	printerTextStyles = GetStylScrap(hSourceTE);// Revert the selection range	TESetSelect(oldSelStart, oldSelEnd, hSourceTE);// Move the text from the documents TERec and add the style (got above) to it	HLock((Handle)((*hSourceTE)->hText));	TEStyleInsert ( (Ptr)*((*hSourceTE)->hText),									(*hSourceTE)->teLength,									printerTextStyles,									*hDestTE);			HUnlock((Handle)((*hSourceTE)->hText));// Deactivat the temporary TERec	TEDeactivate(*hDestTE);	// Reset the port	SetPort(oldPort);}//-------------------------------------------------------//       GXPrintLoop//// This does the actual printing. It creates a invisible window// to draw into. It duplicates the Text Edit record in the // document to a temporary record. It puts up the progress// dialog then it steps through the pages. At each page, it// clips the text edit rec so it doesn't draw lines of text// over two pages. It then uses TEUpdate to draw each page and// finally scroll's the text edit rec ready for the next page.//-------------------------------------------------------#pragma segment GXPrintSegOSErr GXPrintLoop ( DPtr theDoc){	OSErr      theErr = noErr; 	WindowPtr  imagingWind;              // Temp window we're going to draw into	gxViewPort printViewPort;            // Printer view port	Rect       tempWindRect = {0,0,0,0}; // Bounds for temp window  TEHandle   tempTE;                   // Copy of the documents text edit record 	PageEndsArray pageEnds;           	short      numDocPages;	long       firstPage;	long       lastPage;	long       numPagesToPrint;	long       pageCounter;	Point      patStretch = {1,1};	gxFormat   pageFormat;	Rect       everywhereRect;	Str255     windTitle;	GXPrintSpoolDataRec spoolData;	Rect       printerPage;	Rect       rectToClip;// Create a window to draw into. It's bounds are small(0,0,0,0)// and it's not shown so it doesn't appear to the user 	imagingWind = NewWindow ( nil, &tempWindRect, "\p",														false, documentProc, (WindowPtr)-1,														false, 0);// Set the port to the imaging port	SetPort((GrafPtr)imagingWind);	// Get the size of the printers page 	GetRectOfPage ( theDoc, &printerPage );// Duplicate the text edit record from the document	DuplicateStyleTERec( theDoc->theText, &tempTE, &printerPage, (GrafPtr)imagingWind );// Work out the size of each page using GetPageEnds. This stops text//  drawing over two pages	(*tempTE)->destRect = printerPage;	GetPageEnds(printerPage.bottom-printerPage.top,							tempTE,							pageEnds,							&numDocPages);	// Determine which pages the user selected to print, and print// only those pages that are actually in the document. 	GXGetJobPageRange(theDoc->documentJob, &firstPage, &lastPage);	if (lastPage > numDocPages)		lastPage = numDocPages;// Calculate the number of pages to print and begin printing. 	numPagesToPrint = lastPage - firstPage + 1;	theErr = GXGetJobError(theDoc->documentJob);	if (!theErr)	{// Get the title of the window as we'll use this for the name we call the print job		GetWTitle ( theDoc->theWindow, windTitle);// Put up the print progress dialog		GXStartJob( theDoc->documentJob,								windTitle, 								numPagesToPrint);		theErr = GXGetJobError(theDoc->documentJob);		if (!theErr)		{// Create a new view port for printing and set our translator//	rects to "wide open" so that they include all data we're//	drawing. For each page we print, call GXStartPage, draw,//	and call GXFinishPage. 			SetRect(&everywhereRect, 0, 0, 32767, 32767);			printViewPort = GXNewViewPort(gxScreenViewDevices);					for (pageCounter = firstPage; (theErr == noErr) && (pageCounter <= lastPage); pageCounter++)			{// Get the page's format and start printing the page.				pageFormat = GXGetJobFormat(theDoc->documentJob, 1);						GXStartPage(theDoc->documentJob, pageCounter, pageFormat, 1, &printViewPort);												theErr = GXGetJobError(theDoc->documentJob);		// If there were no errors, set up the translator, draw// the QuickDraw data for current page, and remove the// translator.				if (!theErr)				{	// Set up spool data					spoolData.printViewPort = printViewPort;					GXGetFormatDimensions( pageFormat, &spoolData.pageArea, nil);  // Install the translator					GXInstallQDTranslator((GrafPtr)imagingWind,																gxDefaultOptionsTranslation,																&everywhereRect,																&everywhereRect,																patStretch,																NewgxShapeSpoolProc ( PrintAShape ),																&spoolData);// Make sure it draws into the correct QuickDraw window					SetPort((GrafPtr)imagingWind);// Clip the page to the size of the current page					rectToClip = printerPage;					if (pageCounter == 1)						rectToClip.bottom = rectToClip.top + pageEnds[pageCounter-1];					else						rectToClip.bottom = rectToClip.top																+ ( pageEnds[pageCounter-1] - pageEnds[pageCounter-2] );					ClipRect(&rectToClip);// Use TEUpdate to do the drawing					TEUpdate(&printerPage, tempTE);// Remove the Translator					GXRemoveQDTranslator((GrafPtr)imagingWind, nil);// Finish off this page					GXFinishPage(theDoc->documentJob);// Scroll the text edit down to the next page					if (pageCounter < lastPage)						TEScroll(0,rectToClip.top-rectToClip.bottom, tempTE);				}			}// All done, so finish the printer job and dispose of the GXView port			GXFinishJob(theDoc->documentJob);			theErr = GXGetJobError(theDoc->documentJob);			GXDisposeViewPort(printViewPort);		}	}// Dispose of temporary TERec and imaging window	TEDispose ( tempTE );	DisposeWindow ( imagingWind );	return theErr;}	//-------------------------------------------------------//       GXPrintDocument//// This puts up the print dialog box, then calls GXPrintLoop()// to do the actual printing.//-------------------------------------------------------#pragma segment GXPrintSegOSErr GXPrintDocument ( DPtr    theDoc,												Boolean askUser ){	OSErr					    err = noErr;	gxEditMenuRecord	editMenuRec;	gxDialogResult		dialogResult;// Put up the print dialog if askUser is set to true	if (!askUser)		dialogResult = gxOKSelected;	else	{// Set up the edit menu record so GX knows how to gray it		SetupGXEditMenuRec( &editMenuRec );			AdjustMenusForGXPrintDialogs(true);		dialogResult = GXJobPrintDialog(theDoc->documentJob, 															&editMenuRec);		AdjustMenusForGXPrintDialogs(false);	}// if ok selected, then print the pages	if (dialogResult == gxOKSelected)		err = GXPrintLoop(theDoc);	else		err = userCanceledErr;	return err;}