// Nitrogen/UnicodeConverter.hh
// ----------------------------
//
// Maintained by Marshall Clow

// Part of the Nitrogen project.
//
// Written 2004-2006 by Marshall Clow and Joshua Juran.
//
// This code was written entirely by the above contributors, who place it
// in the public domain.


#ifndef NITROGEN_UNICODECONVERTER_HH
#define NITROGEN_UNICODECONVERTER_HH


#ifndef __UNICODECONVERTER__
#include <UnicodeConverter.h>
#endif

// nucleus
#ifndef NUCLEUS_OWNED_HH
#include "nucleus/owned.hh"
#endif

// Nitrogen
#ifndef MAC_TOOLBOX_TYPES_OPTIONBITS_HH
#include "Mac/Toolbox/Types/OptionBits.hh"
#endif
#ifndef MAC_TOOLBOX_UTILITIES_THROWOSSTATUS_HH
#include "Mac/Toolbox/Utilities/ThrowOSStatus.hh"
#endif

#ifndef NITROGEN_TEXTCOMMON_HH
#include "Nitrogen/TextCommon.hh"
#endif


namespace Nitrogen {
	
//	We're skipping the UPP stuff for now.....
#if 0
/*
 *  NewUnicodeToTextFallbackUPP()
 *  
 *  Availability:
 *    Mac OS X:         in version 10.0 and later in CoreServices.framework
 *    CarbonLib:        in CarbonLib 1.0 and later
 *    Non-Carbon CFM:   available as macro/inline
 */
extern UnicodeToTextFallbackUPP
NewUnicodeToTextFallbackUPP(UnicodeToTextFallbackProcPtr userRoutine) AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;

/*
 *  DisposeUnicodeToTextFallbackUPP()
 *  
 *  Availability:
 *    Mac OS X:         in version 10.0 and later in CoreServices.framework
 *    CarbonLib:        in CarbonLib 1.0 and later
 *    Non-Carbon CFM:   available as macro/inline
 */
extern void
DisposeUnicodeToTextFallbackUPP(UnicodeToTextFallbackUPP userUPP) AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;

/*
 *  InvokeUnicodeToTextFallbackUPP()
 *  
 *  Availability:
 *    Mac OS X:         in version 10.0 and later in CoreServices.framework
 *    CarbonLib:        in CarbonLib 1.0 and later
 *    Non-Carbon CFM:   available as macro/inline
 */
extern OSStatus
InvokeUnicodeToTextFallbackUPP(
  UniChar *                 iSrcUniStr,
  ByteCount                 iSrcUniStrLen,
  ByteCount *               oSrcConvLen,
  TextPtr                   oDestStr,
  ByteCount                 iDestStrLen,
  ByteCount *               oDestConvLen,
  LogicalAddress            iInfoPtr,
  ConstUnicodeMappingPtr    iUnicodeMappingPtr,
  UnicodeToTextFallbackUPP  userUPP)                          AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;
#endif

  using ::TextToUnicodeInfo;
  using ::UnicodeToTextInfo;
  using ::UnicodeToTextRunInfo;
  }


namespace nucleus
  {
//	Do these need to take pointers???? [ No - they are already pointers ]
	template <> struct disposer< TextToUnicodeInfo >
		{
		typedef TextToUnicodeInfo  argument_type;
		typedef void               result_type;
		
		void operator()( TextToUnicodeInfo ttui ) const
			{
				(void) ::DisposeTextToUnicodeInfo ( &ttui );
			}
		};

	template <> struct disposer< UnicodeToTextInfo >
		{
		typedef UnicodeToTextInfo  argument_type;
		typedef void               result_type;
		
		void operator()( UnicodeToTextInfo utti ) const
			{
				(void) ::DisposeUnicodeToTextInfo ( &utti );
			}
		};

	template <> struct disposer< UnicodeToTextRunInfo >
		{
		typedef UnicodeToTextRunInfo  argument_type;
		typedef void                  result_type;
		
		void operator()( UnicodeToTextRunInfo uttri ) const
			{
				(void) ::DisposeUnicodeToTextRunInfo ( &uttri );
			}
		};
  }

namespace Nitrogen
  {

//	Shouldn't these pointers be const &s ??
	inline nucleus::owned<TextToUnicodeInfo> CreateTextToUnicodeInfo ( const UnicodeMapping &iUnicodeMapping ) {
		(void) TextEncodingConversionManagerErrorsRegistrationDependency();
		TextToUnicodeInfo result;
		Mac::ThrowOSStatus ( ::CreateTextToUnicodeInfo ( &iUnicodeMapping, &result ));
		return nucleus::owned<TextToUnicodeInfo>::seize ( result );
		}
	
	inline nucleus::owned<TextToUnicodeInfo> CreateTextToUnicodeInfoByEncoding ( TextEncoding iEncoding ) {
		(void) TextEncodingConversionManagerErrorsRegistrationDependency();
		TextToUnicodeInfo result;
		Mac::ThrowOSStatus ( ::CreateTextToUnicodeInfoByEncoding ( iEncoding, &result ));
		return nucleus::owned<TextToUnicodeInfo>::seize ( result );
		}

	inline nucleus::owned<UnicodeToTextInfo> CreateUnicodeToTextInfo ( const UnicodeMapping &iUnicodeMapping ) {
		(void) TextEncodingConversionManagerErrorsRegistrationDependency();
		UnicodeToTextInfo result;
		Mac::ThrowOSStatus ( ::CreateUnicodeToTextInfo ( &iUnicodeMapping, &result ));
		return nucleus::owned<UnicodeToTextInfo>::seize ( result );
		}
	
	inline nucleus::owned<UnicodeToTextInfo> CreateUnicodeToTextInfoByEncoding ( TextEncoding iEncoding ) {
		(void) TextEncodingConversionManagerErrorsRegistrationDependency();
		UnicodeToTextInfo result;
		Mac::ThrowOSStatus ( ::CreateUnicodeToTextInfoByEncoding ( iEncoding, &result ));
		return nucleus::owned<UnicodeToTextInfo>::seize ( result );
		}

	inline nucleus::owned<UnicodeToTextRunInfo> CreateUnicodeToTextRunInfo ( ItemCount iNumberOfMappings, const UnicodeMapping iUnicodeMappings[] ) {
		(void) TextEncodingConversionManagerErrorsRegistrationDependency();
		UnicodeToTextRunInfo result;
		Mac::ThrowOSStatus ( ::CreateUnicodeToTextRunInfo ( iNumberOfMappings, iUnicodeMappings, &result ));
		return nucleus::owned<UnicodeToTextRunInfo>::seize ( result );
		}
	
	inline nucleus::owned<UnicodeToTextRunInfo> CreateUnicodeToTextRunInfoByEncoding ( ItemCount iNumberOfEncodings, const ::TextEncoding iEncodings[] ) {
		(void) TextEncodingConversionManagerErrorsRegistrationDependency();
		UnicodeToTextRunInfo result;
		Mac::ThrowOSStatus ( ::CreateUnicodeToTextRunInfoByEncoding ( iNumberOfEncodings, iEncodings, &result ));
		return nucleus::owned<UnicodeToTextRunInfo>::seize ( result );
		}

	inline nucleus::owned<UnicodeToTextRunInfo> CreateUnicodeToTextRunInfoByScriptCode ( ItemCount iNumberOfScriptCodes, const ::ScriptCode iScripts[] ) {
		(void) TextEncodingConversionManagerErrorsRegistrationDependency();
		UnicodeToTextRunInfo result;
		Mac::ThrowOSStatus ( ::CreateUnicodeToTextRunInfoByScriptCode ( iNumberOfScriptCodes, iScripts, &result ));
		return nucleus::owned<UnicodeToTextRunInfo>::seize ( result );
		}

//	I think that these two  should possibly return a new info, rather than changing the input
	inline void ChangeTextToUnicodeInfo ( TextToUnicodeInfo ioTextToUnicodeInfo, const UnicodeMapping &iUnicodeMapping ) {
		(void) TextEncodingConversionManagerErrorsRegistrationDependency();
		Mac::ThrowOSStatus ( ::ChangeTextToUnicodeInfo ( ioTextToUnicodeInfo, &iUnicodeMapping ));
		}

	inline void ChangeUnicodeToTextInfo ( UnicodeToTextInfo ioUnicodeToTextInfo, const UnicodeMapping &iUnicodeMapping ) {
		(void) TextEncodingConversionManagerErrorsRegistrationDependency();
		Mac::ThrowOSStatus ( ::ChangeUnicodeToTextInfo ( ioUnicodeToTextInfo, &iUnicodeMapping ));
		}

	inline void DisposeTextToUnicodeInfo ( TextToUnicodeInfo *ioTextToUnicodeInfo ) {
		(void) TextEncodingConversionManagerErrorsRegistrationDependency();
		Mac::ThrowOSStatus ( ::DisposeTextToUnicodeInfo ( ioTextToUnicodeInfo ));
		}
	
	inline void DisposeUnicodeToTextInfo ( UnicodeToTextInfo *ioUnicodeToTextInfo ) {
		(void) TextEncodingConversionManagerErrorsRegistrationDependency();
		Mac::ThrowOSStatus ( ::DisposeUnicodeToTextInfo ( ioUnicodeToTextInfo ));
		}
	
	inline void DisposeUnicodeToTextRunInfo ( UnicodeToTextRunInfo *ioUnicodeToTextRunInfo ) {
		(void) TextEncodingConversionManagerErrorsRegistrationDependency();
		Mac::ThrowOSStatus ( ::DisposeUnicodeToTextRunInfo ( ioUnicodeToTextRunInfo ));
		}


//	Returns the length of the converted string (in Bytes)
	ByteCount ConvertFromTextToUnicode (
//	Inputs
			TextToUnicodeInfo iTextToUnicodeInfo, ByteCount iSourceLen, ConstLogicalAddress iSourceStr, Mac::OptionBits iControlFlags,
//	Offsets
			ItemCount iOffsetCount, const ByteOffset iOffsetArray[], ItemCount *oOffsetCount, ByteOffset oOffsetArray[],
//	Outputs
			ByteCount iOutputBufLen, ByteCount * oSourceRead, UniChar oUnicodeStr[] );
			
//	A simple case without the offsets
	ByteCount ConvertFromTextToUnicode (
//	Inputs
			TextToUnicodeInfo iTextToUnicodeInfo, ByteCount iSourceLen, ConstLogicalAddress iSourceStr, Mac::OptionBits iControlFlags,
//	Outputs
			ByteCount iOutputBufLen, ByteCount * oSourceRead, UniChar oUnicodeStr[] );


//	Returns the length of the converted string (in Bytes)

	ByteCount ConvertFromUnicodeToText ( 
//	Unicode inputs
			UnicodeToTextInfo iUnicodeToTextInfo, ByteCount iUnicodeLen, const UniChar iUnicodeStr[], Mac::OptionBits iControlFlags,
//	Offsets
			ItemCount iOffsetCount, const ByteOffset iOffsetArray[], ItemCount * oOffsetCount, ByteOffset oOffsetArray[],
//	Outputs
			ByteCount iOutputBufLen, ByteCount * oInputRead, LogicalAddress oOutputStr );

//	A simple case without the offsets			
	ByteCount ConvertFromUnicodeToText ( 
//	Unicode inputs
			UnicodeToTextInfo iUnicodeToTextInfo, ByteCount iUnicodeLen, const UniChar iUnicodeStr[], Mac::OptionBits iControlFlags,
//	Outputs
			ByteCount iOutputBufLen, ByteCount *oInputRead, LogicalAddress oOutputStr );


#if 0
/*
 *  ConvertFromUnicodeToTextRun()
 *  
 *  Availability:
 *    Mac OS X:         in version 10.0 and later in CoreServices.framework
 *    CarbonLib:        in CarbonLib 1.0 and later
 *    Non-Carbon CFM:   in UnicodeConverter 1.1 and later
 */
extern OSStatus 
ConvertFromUnicodeToTextRun(
  UnicodeToTextRunInfo   iUnicodeToTextInfo,
  ByteCount              iUnicodeLen,
  const UniChar          iUnicodeStr[],
  OptionBits             iControlFlags,
  ItemCount              iOffsetCount,
  const ByteOffset       iOffsetArray[],           /* can be NULL */
  ItemCount *            oOffsetCount,             /* can be NULL */
  ByteOffset             oOffsetArray[],           /* can be NULL */
  ByteCount              iOutputBufLen,
  ByteCount *            oInputRead,
  ByteCount *            oOutputLen,
  LogicalAddress         oOutputStr,
  ItemCount              iEncodingRunBufLen,
  ItemCount *            oEncodingRunOutLen,
  TextEncodingRun        oEncodingRuns[])                     AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;


/*
 *  ConvertFromUnicodeToScriptCodeRun()
 *  
 *  Availability:
 *    Mac OS X:         in version 10.0 and later in CoreServices.framework
 *    CarbonLib:        in CarbonLib 1.0 and later
 *    Non-Carbon CFM:   in UnicodeConverter 1.1 and later
 */
extern OSStatus 
ConvertFromUnicodeToScriptCodeRun(
  UnicodeToTextRunInfo   iUnicodeToTextInfo,
  ByteCount              iUnicodeLen,
  const UniChar          iUnicodeStr[],
  OptionBits             iControlFlags,
  ItemCount              iOffsetCount,
  const ByteOffset       iOffsetArray[],           /* can be NULL */
  ItemCount *            oOffsetCount,             /* can be NULL */
  ByteOffset             oOffsetArray[],           /* can be NULL */
  ByteCount              iOutputBufLen,
  ByteCount *            oInputRead,
  ByteCount *            oOutputLen,
  LogicalAddress         oOutputStr,
  ItemCount              iScriptRunBufLen,
  ItemCount *            oScriptRunOutLen,
  ScriptCodeRun          oScriptCodeRuns[])                   AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;


/* Truncate a multibyte string at a safe place. */
/*
 *  TruncateForTextToUnicode()
 *  
 *  Availability:
 *    Mac OS X:         in version 10.0 and later in CoreServices.framework
 *    CarbonLib:        in CarbonLib 1.0 and later
 *    Non-Carbon CFM:   in UnicodeConverter 1.1 and later
 */
extern OSStatus 
TruncateForTextToUnicode(
  ConstTextToUnicodeInfo   iTextToUnicodeInfo,
  ByteCount                iSourceLen,
  ConstLogicalAddress      iSourceStr,
  ByteCount                iMaxLen,
  ByteCount *              oTruncatedLen)                     AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;


/* Truncate a Unicode string at a safe place. */
/*
 *  TruncateForUnicodeToText()
 *  
 *  Availability:
 *    Mac OS X:         in version 10.0 and later in CoreServices.framework
 *    CarbonLib:        in CarbonLib 1.0 and later
 *    Non-Carbon CFM:   in UnicodeConverter 1.1 and later
 */
extern OSStatus 
TruncateForUnicodeToText(
  ConstUnicodeToTextInfo   iUnicodeToTextInfo,
  ByteCount                iSourceLen,
  const UniChar            iSourceStr[],
  OptionBits               iControlFlags,
  ByteCount                iMaxLen,
  ByteCount *              oTruncatedLen)                     AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;


/* Convert a Pascal string to Unicode string. */
/*
 *  ConvertFromPStringToUnicode()
 *  
 *  Availability:
 *    Mac OS X:         in version 10.0 and later in CoreServices.framework
 *    CarbonLib:        in CarbonLib 1.0 and later
 *    Non-Carbon CFM:   in UnicodeConverter 1.1 and later
 */
extern OSStatus 
ConvertFromPStringToUnicode(
  TextToUnicodeInfo   iTextToUnicodeInfo,
  ConstStr255Param    iPascalStr,
  ByteCount           iOutputBufLen,
  ByteCount *         oUnicodeLen,
  UniChar             oUnicodeStr[])                          AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;


/* Convert a Unicode string to Pascal string. */
/*
 *  ConvertFromUnicodeToPString()
 *  
 *  Availability:
 *    Mac OS X:         in version 10.0 and later in CoreServices.framework
 *    CarbonLib:        in CarbonLib 1.0 and later
 *    Non-Carbon CFM:   in UnicodeConverter 1.1 and later
 */
extern OSStatus 
ConvertFromUnicodeToPString(
  UnicodeToTextInfo   iUnicodeToTextInfo,
  ByteCount           iUnicodeLen,
  const UniChar       iUnicodeStr[],
  Str255              oPascalStr)                             AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;
#endif


	inline ItemCount CountUnicodeMappings ( Mac::OptionBits iFilter, ConstUnicodeMappingPtr iFindMapping ) {
		(void) TextEncodingConversionManagerErrorsRegistrationDependency();
		::ItemCount result;
		Mac::ThrowOSStatus ( ::CountUnicodeMappings ( iFilter, iFindMapping, &result ));
		return result;
		}
	
#if 0
/* Get a list of the available conversion mappings. */
/*
 *  QueryUnicodeMappings()
 *  
 *  Availability:
 *    Mac OS X:         in version 10.0 and later in CoreServices.framework
 *    CarbonLib:        in CarbonLib 1.0 and later
 *    Non-Carbon CFM:   in UnicodeConverter 1.1 and later
 */
extern OSStatus 
QueryUnicodeMappings(
  OptionBits               iFilter,
  ConstUnicodeMappingPtr   iFindMapping,
  ItemCount                iMaxCount,
  ItemCount *              oActualCount,
  UnicodeMapping           oReturnedMappings[])               AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;
#endif


#if 0
/* Setup the fallback handler for converting Unicode To Text. */
/*
 *  SetFallbackUnicodeToText()
 *  
 *  Availability:
 *    Mac OS X:         in version 10.0 and later in CoreServices.framework
 *    CarbonLib:        in CarbonLib 1.0 and later
 *    Non-Carbon CFM:   in UnicodeConverter 1.1 and later
 */
extern OSStatus 
SetFallbackUnicodeToText(
  UnicodeToTextInfo          iUnicodeToTextInfo,
  UnicodeToTextFallbackUPP   iFallback,
  OptionBits                 iControlFlags,
  LogicalAddress             iInfoPtr)                        AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;


/* Setup the fallback handler for converting Unicode To TextRuns. */
/*
 *  SetFallbackUnicodeToTextRun()
 *  
 *  Availability:
 *    Mac OS X:         in version 10.0 and later in CoreServices.framework
 *    CarbonLib:        in CarbonLib 1.0 and later
 *    Non-Carbon CFM:   in UnicodeConverter 1.1 and later
 */
extern OSStatus 
SetFallbackUnicodeToTextRun(
  UnicodeToTextRunInfo       iUnicodeToTextRunInfo,
  UnicodeToTextFallbackUPP   iFallback,
  OptionBits                 iControlFlags,
  LogicalAddress             iInfoPtr)                        AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;
#endif


	inline void ResetTextToUnicodeInfo ( TextToUnicodeInfo ioTextToUnicodeInfo ) {
		(void) TextEncodingConversionManagerErrorsRegistrationDependency();
		Mac::ThrowOSStatus ( ::ResetTextToUnicodeInfo ( ioTextToUnicodeInfo ));
		}
	
	inline void ResetUnicodeToTextInfo ( UnicodeToTextInfo ioUnicodeToTextInfo ) {
		(void) TextEncodingConversionManagerErrorsRegistrationDependency();
		Mac::ThrowOSStatus ( ::ResetUnicodeToTextInfo ( ioUnicodeToTextInfo ));
		}

	inline void ResetUnicodeToTextRunInfo ( UnicodeToTextRunInfo ioUnicodeToTextRunInfo ) {
		(void) TextEncodingConversionManagerErrorsRegistrationDependency();
		Mac::ThrowOSStatus ( ::ResetUnicodeToTextRunInfo ( ioUnicodeToTextRunInfo ));
		}

  	}

#endif
