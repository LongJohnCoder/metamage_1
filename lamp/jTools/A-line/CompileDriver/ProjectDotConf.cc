/*	=================
 *	ProjectDotConf.cc
 *	=================
 */

#include "CompileDriver/ProjectDotConf.hh"

// POSeven
#include "POSeven/FileDescriptor.hh"

// Io
#include "Io/TextInput.hh"

// BitsAndBytes
#include "StringPredicates.hh"

// CompileDriver
#include "CompileDriver/ProjectCatalog.hh"


namespace CompileDriver
{
	
	namespace NN = Nucleus;
	namespace P7 = POSeven;
	
	using BitsAndBytes::eos;
	
	
	static std::pair< std::string, std::string > ParseKeyValue( const std::string& line )
	{
		//if (line.size() == 0)  return;  // Redundant
		const char* ws = " \t";
		std::size_t posWhitespace = line.find_first_of( ws );
		// FIXME:  This will break when leading whitespace precedes key
		std::string key = line.substr( 0, posWhitespace );
		std::string value;
		
		if ( !eos( posWhitespace ) )
		{
			// Whitespace follows the key
			std::size_t posValue = line.find_first_not_of( ws, posWhitespace );
			
			if ( !eos( posValue ) )
			{
				// A value is present
				std::size_t posLastPrintableChar = line.find_last_not_of( ws );
				std::size_t posEndOfValue = posLastPrintableChar + 1;
				value = line.substr( posValue, posEndOfValue );
			}
		}
		
		return std::make_pair( key, value );
	}
	
	static void Tokenize( const std::string& line,
	                      std::back_insert_iterator< std::vector< std::string > > tokens )
	{
		typedef std::size_t size_type;
		
		const char* ws = " \t";
		size_type iLastPrintableChar = line.find_last_not_of( ws );
		
		if ( eos( iLastPrintableChar ) )
		{
			return;
		}
		
		size_type endOfData = iLastPrintableChar + 1;
		
		for ( size_type i = 0;  i < endOfData;  )
		{
			size_type tokenStart = line.find_first_not_of( ws, i          );
			size_type tokenEnd   = line.find_first_of    ( ws, tokenStart );
			
			*tokens++ = line.substr( tokenStart, tokenEnd - tokenStart );
			
			i = tokenEnd;
		}
	}
	
	void ReadProjectDotConf( const std::string& pathname, DotConfData& data )
	{
		// Open the config file, and read as a text input stream.
		Io::TextInputAdapter< NN::Owned< P7::FileDescriptor > > input( io::open_for_reading( pathname ) );
		
		std::size_t lineCount = 0;
		
		while ( input.Ready() )
		{
			// Read a line
			std::string text = input.Read();
			++lineCount;
			// Skip blank lines
			if ( text.size() == 0 )  continue;
			// Skip comments
			if ( text[ 0 ] == '#' )  continue;
			
			data.push_back( DotConfLine() );
			DotConfLine& line( data.back() );
			line.lineNumber = lineCount;
			
			// Parse a line into key and value
			//G::ref( line.key, value ) = ParseKeyValue( text );
			std::pair< std::string, std::string > kv = ParseKeyValue( text );
			line.key          = kv.first;
			std::string value = kv.second;
			
			// Check for double-quotes
			std::size_t size = value.size();
			
			if ( value[ 0 ] == '"'  &&  value[ size - 1 ] == '"' )
			{
				// Strip off double-quotes and push single value
				value = value.substr( 1, (size - 1) - 1 );
				
				line.values.push_back( value );
			}
			else
			{
				// Copy multiple tokens
				Tokenize( value, std::back_inserter( line.values ) );
			}
		}
	}
	
}

