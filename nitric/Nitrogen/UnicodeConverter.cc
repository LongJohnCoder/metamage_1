// Nitrogen/UnicodeConverter.cc
// ----------------------------
//
// Maintained by Marshall Clow

// Part of the Nitrogen project.
//
// Written 2004-2006 by Marshall Clow and Joshua Juran.
//
// This code was written entirely by the above contributors, who place it
// in the public domain.


#include "Nitrogen/UnicodeConverter.hh"


namespace Nitrogen
{
	
	using Mac::ThrowOSStatus;
	
	
//	RegisterTECManagerErrors is in TextCommon.cc
	
	static TextEncodingConversionManagerErrorsRegistrationDependency gTextEncodingConversionManagerErrorsRegistrationDependency;

//	Returns the length of the converted string (in Bytes)
	ByteCount ConvertFromTextToUnicode (
//	Inputs
			TextToUnicodeInfo iTextToUnicodeInfo, ByteCount iSourceLen, ConstLogicalAddress iSourceStr, Mac::OptionBits iControlFlags,
//	Offsets
			ItemCount iOffsetCount, const ByteOffset iOffsetArray[], ItemCount *oOffsetCount, ByteOffset oOffsetArray[],
//	Outputs
			ByteCount iOutputBufLen, ByteCount * oSourceRead, UniChar oUnicodeStr[] ) {
		ByteCount result;

//	The CFM headers define the iOffsetArray as "ByteOffset iOffsetArray []" (not const) - grumble.
#if TARGET_RT_MAC_MACHO
		ThrowOSStatus ( ::ConvertFromTextToUnicode ( iTextToUnicodeInfo, iSourceLen, iSourceStr, iControlFlags,
							iOffsetCount, iOffsetArray, oOffsetCount, oOffsetArray,
							iOutputBufLen, oSourceRead, &result, oUnicodeStr ));
#else
		ThrowOSStatus ( ::ConvertFromTextToUnicode ( iTextToUnicodeInfo, iSourceLen, iSourceStr, iControlFlags,
							iOffsetCount, const_cast<unsigned long *> ( iOffsetArray ), oOffsetCount, oOffsetArray,
							iOutputBufLen, oSourceRead, &result, oUnicodeStr ));
#endif
		return result;
		}

			
//	A simple case without the offsets
	ByteCount ConvertFromTextToUnicode (
//	Inputs
			TextToUnicodeInfo iTextToUnicodeInfo, ByteCount iSourceLen, ConstLogicalAddress iSourceStr, Mac::OptionBits iControlFlags,
//	Outputs
			ByteCount iOutputBufLen, ByteCount * oSourceRead, UniChar oUnicodeStr[] ) {
		ByteCount result;
		ThrowOSStatus ( ::ConvertFromTextToUnicode ( iTextToUnicodeInfo, iSourceLen, iSourceStr, iControlFlags,
							0, NULL, NULL, NULL,
							iOutputBufLen, oSourceRead, &result, oUnicodeStr ));
		return result;
		}


	ByteCount ConvertFromUnicodeToText ( 
//	Unicode inputs
			UnicodeToTextInfo iUnicodeToTextInfo, ByteCount iUnicodeLen, const UniChar iUnicodeStr[], Mac::OptionBits iControlFlags,
//	Offsets
			ItemCount iOffsetCount, const ByteOffset iOffsetArray[], ItemCount * oOffsetCount, ByteOffset oOffsetArray[],           /* can be NULL */
//	Outputs
			ByteCount iOutputBufLen, ByteCount * oInputRead, LogicalAddress oOutputStr ) {

		ByteCount result;
//	The CFM headers define the iOffsetArray as "ByteOffset iOffsetArray []" (not const) - grumble.
#if TARGET_RT_MAC_MACHO
		ThrowOSStatus ( ::ConvertFromUnicodeToText ( iUnicodeToTextInfo, iUnicodeLen, iUnicodeStr, iControlFlags,
							iOffsetCount, iOffsetArray, oOffsetCount, oOffsetArray,
							iOutputBufLen, oInputRead, &result, oOutputStr ));
#else
		ThrowOSStatus ( ::ConvertFromUnicodeToText ( iUnicodeToTextInfo, iUnicodeLen, iUnicodeStr, iControlFlags,
							iOffsetCount, const_cast<unsigned long *> ( iOffsetArray ), oOffsetCount, oOffsetArray,
							iOutputBufLen, oInputRead, &result, oOutputStr ));
#endif
		return result;
		}
	

//	A simple case without the offsets			
	ByteCount ConvertFromUnicodeToText ( 
//	Unicode inputs
			UnicodeToTextInfo iUnicodeToTextInfo, ByteCount iUnicodeLen, const UniChar iUnicodeStr[], Mac::OptionBits iControlFlags,
//	Outputs
			ByteCount iOutputBufLen, ByteCount *oInputRead, LogicalAddress oOutputStr ) {

		ByteCount result;
		ThrowOSStatus ( ::ConvertFromUnicodeToText ( iUnicodeToTextInfo, iUnicodeLen, iUnicodeStr, iControlFlags,
							0, NULL, NULL, NULL,
							iOutputBufLen, oInputRead, &result, oOutputStr ));
		return result;
		}
	

	}
