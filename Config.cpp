/**
 * @file
 * @author  James Poe   <>, (C) 2008, 2009
 * @date    08/28/08
 * @brief   This is the implementation for the Config object.
 *
 * @section LICENSE
 * Copyright: See COPYING file that comes with this distribution
 *
 * @section DESCRIPTION
 * C++ Implementation: Config
 */
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

#include "Config.h"


/**
 * @ingroup Config
 *
 * @param filename
 * @param delimiter
 * @param comment
 * @param eof
 * @param region
 * @return 
 * Standard constructor
 */
Config::Config ( string filename,
                 string delimiter,
                 string comment,
                 string eof,
                 string region,
                 string reserved )
                : configDelimiter (delimiter), configComment (comment), 
                  configEOF (eof), configRegion (region), configReservedRegions(reserved)
{
  std::cout << "Reading configuration file..." << std::endl;

  std::ifstream in( filename.c_str() );

  if ( !in ) throw file_not_found( filename );

  in >> (*this);
}

/**
 * @ingroup Config
 *
 * @return 
 * Empty constructor
 */
Config::Config ( )
{
  configDelimiter = "=";
  configComment = "#";
  configEOF = "EOF";
  configRegion = "%";
  configReservedRegions = "derived|calculated";

  std::cout << "Creating empty configuration file..." << std::endl;

}


/**
 * @ingroup Config
 *
 * @param region
 * @param key
 * @return 
 *
 * Remove an entry from the configuration
 */
void Config::remove ( const string& region, const string& key )
{
  // Remove key and its value
  configContents.erase ( configContents.find( pair < string, string > ( region, key ) ) );
  return;
}


/**
 * @ingroup Config
 *
 * @param region
 * @param key
 * @return bool
 *
 * Return the existance of a region/key pair within the configuration
 */
bool Config::keyExists ( const string& region, const string& key ) const
{
  mapci p = configContents.find ( pair < string, string > ( region, key) );
  return ( p != configContents.end() );
}


/**
 * @ingroup Config
 *
 * @param s
 * @return 
 *
 * Remove string leading/trailing whitespace
 */
void Config::trim ( string& s )
{
  // Remove leading and trailing whitespace
  static const char whitespace[] = " \n\t\v\r\f";
  s.erase( 0 , s.find_first_not_of ( whitespace ) );
  s.erase( s.find_last_not_of ( whitespace ) + 1U );
}


/**
 * @ingroup Config
 *
 * @param os
 * @param cf
 * @return os
 *
 * Write an internal configuration file to output stream
 */
std::ostream& operator<<( std::ostream& os, const Config& cf )
{
  string curReg="";
  // Save a ConfigFile to os
  for( Config::mapci p = cf.configContents.begin();
       p != cf.configContents.end();
     ++p )
  {
    if ( curReg != p->first.first )
    {
      os << cf.configRegion << " " << p->first.first << std::endl;
      curReg = p->first.first;
    }

    os << p->first.second << " " << cf.configDelimiter << " ";
    os << p->second << std::endl;
  }
  return os;
}


/**
 * @ingroup Config
 *
 * @param is
 * @param cf
 * @return os
 *
 * Process an istream and load it into the configuration
 */
std::istream& operator>>( std::istream& is, Config& cf )
{
  // Load a ConfigFile from is
  // Read in keys and values, keeping internal whitespace
  typedef string::size_type pos;
  const string& delim   = cf.configDelimiter;       // separator
  const string& comm    = cf.configComment;         // comment
  const string& eof     = cf.configEOF;             // end of file sentry
  const string& region  = cf.configRegion;          // region indicator
  const string& rsvdreg = cf.configReservedRegions; // reserved regions
  const pos skip        = delim.length();           // length of separator
  const pos skipRegion  = region.length();          // length of region indicator

  string curRegion = "DEFAULT";

  string nextline = "";  // might need to read ahead to see where value ends

  while ( is || nextline.length ( ) > 0 )
  {
    // Read an entire line at a time
    string line;
    if ( nextline.length ( ) > 0 )
    {
      line = nextline;  // we read ahead; use it now
      nextline = "";
    }
    else
    {
      std::getline ( is , line );
    }

    // Ignore comments
    line = line.substr ( 0 , line.find ( comm ) );

    // Check for end of file sentry
    if ( eof != "" && line.find ( eof ) != string::npos ) return is;

    // Check to see if there is a region indicator, if there is
    // update the current region
    pos regionPos = line.find ( region );
    if ( regionPos < string::npos )
    {
      line.replace ( 0 , regionPos+skipRegion , "");
      Config::trim ( line );
      curRegion = line;
      if ( rsvdreg.find(line) < string::npos ) throw Config::region_not_allowed ( line );
      continue;
    }

    // Parse the line if it contains a delimiter
    pos delimPos = line.find ( delim );
    if ( delimPos < string::npos )
    {
      // Extract the key
      string key = line.substr ( 0 , delimPos );
      line.replace ( 0 , delimPos+skip , "" );

      // See if value continues on the next line
      // Stop at blank line, next line with a key, end of stream,
      // or end of file sentry
      bool terminate = false;
      while( !terminate && is )
      {
        std::getline ( is , nextline );
        terminate = true;

        string nlcopy = nextline;

        // Blank Line Test
        Config::trim ( nlcopy );
        if( nlcopy == "" ) continue;

        // Grab anything before a comment
        nextline = nextline.substr( 0 , nextline.find ( comm ) );

        // Look for a delimiter
        if( nextline.find ( delim ) != string::npos )
          continue;

        // Look for a region indicator
        if ( nextline.find ( region ) != string::npos )
          continue;

        // Look for the EOF indicator
        if( eof != "" && nextline.find ( eof ) != string::npos )
          continue;

        nlcopy = nextline;
        Config::trim ( nlcopy );
        if( nlcopy != "" ) line += "\n";
        line += nextline;
        terminate = false;
      }

      // Store key and value
      Config::trim ( key );
      Config::trim ( line );
      // overwrites if key is repeated
      cf.configContents[ pair < string, string > ( curRegion , key ) ] = line;  
    }
  }

  return is;
}


/**
  *
  *   Validation Helper Functions
  *
**/


/**
 * @ingroup Config
 *
 * @param region
 * @param key
 * @return bool
 *
 * Return whether an option contains alphabetical characters
 */
bool Config::isOptionAlphabetical ( const string& region, const string& key ) const
{
  unsigned int x = 0;

  string alphabet = "abcdefghijklmnopqrstuvwxyz";
  string option = read<string> ( region, key );

  for ( x = 0; x < option.length() ; x++)
    option[ x ] = tolower ( option[x] );

  for ( x = 0 ; x < alphabet.length() ; x++)
  {
    if ( option.find ( alphabet[x] ) < string::npos )
    {
      return true;
    }
  }
  return false;
}

/**
 * @ingroup Config
 *
 * @param region
 * @param key
 * @return bool
 *
 * Return whether an option is a sum of more than one
 * float that is equal to 1.0
 */
bool Config::isOptionNormalizedHistogram ( const string& region, const string& key ) const
{
  unsigned int x;
  float sum = 0;
  deque<float> q = readDeque<float>( region, key );

  for ( x = 0; x < q.size() ; x++)
  {
    sum += q[x];
  }

   return ( ( sum == 1 ) && ( q.size() > 1 ) );
}


/**
 * @ingroup Config
 *
 * @param region
 * @param key
 * @return bool
 *
 * Return whether an option is a list 
 * (and thus not a histogram or a single entry)
 */
bool Config::isOptionList ( const string& region, const string& key ) const
{
  deque<float> q = readDeque<float>( region, key );
  return ( ( q.size() > 1 ) && ( !isOptionNormalizedHistogram ( region, key ) ));
}


/**
 * @ingroup Config
 *
 * @param region
 * @param key
 * @return OptionType
 *
 * Return the option type
 */
OptionType Config::getOptionType ( const string& region, const string& key ) const
{
  if ( isOptionNormalizedHistogram ( region, key ) )
    return HISTOGRAM;
  else if ( isOptionList ( region, key ) )
  {
    if ( isOptionAlphabetical ( region, key ) )
    {
      return ENUMERATOR_LIST;
    }
    else
    {
      return NUMERIC_LIST;
    }
  }
  else if ( isOptionAlphabetical ( region, key ) )
    return ENUMERATOR;
  else
    return NUMERIC;

}
