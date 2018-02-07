/**
 * @file
 * @author  James Poe   <>, (C) 2008, 2009, 2010
 * @date    08/28/2008
 * @brief   This is the interface for the Config object.
 *
 * @section LICENSE
 * Copyright: See COPYING file that comes with this distribution
 *
 * @section DESCRIPTION
 * C++ Interface: Config
 * 
 */
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <map>
#include <list>
#include <deque>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

using std::string;
using std::pair;
using std::map;
using std::deque;
using std::vector;
using std::cerr;
using std::cout;
using std::endl;

enum OptionType { NUMERIC, ENUMERATOR, HISTOGRAM, NUMERIC_LIST, ENUMERATOR_LIST };

/**
 * @ingroup Config
 * @brief   Configuration class
 *
 */
class Config {

/*
 *       DATA 
*/

private:
          string configDelimiter;         // key-value seperator
          string configComment;           // begin comment character
          string configEOF;               // end of config file Indicator
          string configRegion;            // region indicator
          string configReservedRegions;   // list of reserved regions

          map < pair < string , string > , string > configContents;    // Map of Values

          typedef map < pair < string , string > , string >::iterator mapi;          // Map interator
          typedef map < pair < string , string > , string >::const_iterator mapci;   // Map constant interator


/*
 *       METHODS
*/


public:

          Config();
          Config(string filename, 
                 string delimiter = "=",
                 string comment = "#",
                 string eof = "EOF",
                 string region = "%",
                 string reserved = "derived|calculated");


          // Standard read from configuration - call as read<T>(region,key)
          template<class T> T read ( const string& region , const string& key ) const;
          // Vector read from configuration - call as read<T>(region,key)
          template<class T> vector<T> readVector ( const string& region , const string& key ) const;
          // Deque read from configuration - call as read<T>(region,key)
          template<class T> deque<T> readDeque ( const string& region , const string& key ) const;



          // Standard add key to configuration - call as add<T>(region,key,value)
          template<class T> void add ( string region , string key, const T& value );
          template<class T> void addDeque ( string region, string key, const deque<T> &value );

          // Standard remove key from configuration - call as remove(region,key)
          void remove ( const string& region , const string& key );

          // Check for the existence of a Region/Key
          bool keyExists ( const string& region , const string& key ) const;

          friend std::ostream& operator<<( std::ostream& os, const Config& cf );
          friend std::istream& operator>>( std::istream& is, Config& cf );

          /*
           *   Validation Helper Functions
          */

          // Return whether an option is a normalized histogram
          bool isOptionNormalizedHistogram ( const string& region , const string& key ) const;
          // Return whether an option contains Alphabetical characters
          bool isOptionAlphabetical ( const string& region, const string& key ) const;
          // Return whether an option is a non-histogram list
          bool isOptionList ( const string& region, const string& key ) const;
          // Return Option Type
          OptionType getOptionType ( const string& region, const string& key ) const;


protected:

          // Convert To/From a string
          template <class T> static string T_as_string ( const T& t);
          template <class T> static T string_as_T ( const string& s);
          template <class T> static vector<T> string_as_TVector ( const string& s);
          template <class T> static deque<T> string_as_TDeque ( const string& s);
          static void trim( string& s );


/*
 *       EXCEPTIONS
*/

public:
          struct file_not_found 
          {
            string filename;
            file_not_found ( const string& filename_ = string() ) 
              : filename ( filename_ ) 
            {
              cerr << "File " << filename << " not found!" << endl;
            } 
          };

          struct key_not_found 
          {
            string region;
            string key;
            key_not_found ( const string& region_ = string(), const string& key_ = string() )
              : region ( region_ ) , key ( key_ ) 
            {
              cerr << "Region: " << region << " Key: " << key << " not present in configuration!" << endl;
            } 
          };

          struct region_not_allowed
          {
            string region;
            region_not_allowed ( const string& region_ = string() )
              : region ( region_ ) 
            {
              cerr << "Region '" << region << "' is reserved, please choose a new region name!" << endl;
            }
          };

};



/**
 * @name T_as_string
 *
 * @param t
 * @return string t
 *
 * Template class that converts from a T to a string
 * Type T must support the << operator
 */

template<class T>
string Config::T_as_string( const T& t )
{
  std::ostringstream ost;
  ost << t;
  return ost.str();
}


/**
 * @name string_as_T
 *
 * @param s
 * @return t
 *
 * Template class that converts from a string to type T
 * Type T must support the >> operator
 */

template<class T>
T Config::string_as_T ( const string& s )
{
  T t;
  std::istringstream ist ( s );
  ist >> t;
  return t;
}

/**
 * @name string_as_T string
 *
 * @param s
 * @return t
 *
 * Template class that ensures a type T = string 
 * is handled correctly
 */

template<>
inline string Config::string_as_T<string> ( const string& s )
{
  return s;
}



/**
 * @name string_as_T bool
 *
 * @param s
 * @return b
 *
 * Template class that converts common bool strings 
 * to a bool properly
 */

template<>
inline bool Config::string_as_T<bool> ( const string& s )
{
  // Convert from a string to a bool
  // Interpret "false", "F", "no", "n", "0" as false
  // Interpret anything else as true
  bool b = true;
  string sup = s;
  for( string::iterator p = sup.begin(); p != sup.end(); ++p )
    *p = toupper(*p);  // make string all caps
  if( sup==string("FALSE") || sup==string("F") ||
      sup==string("NO") || sup==string("N") ||
      sup==string("0") || sup==string("NONE") )
    b = false;
  return b;
}



/**
 * @name string_as_TVector
 *
 * @param s
 * @return t
 *
 * Template class that converts from a string to a vector of 
 * Type T where each entry is separated by white-space
 */

template<class T>
vector<T> Config::string_as_TVector ( const string& s )
{
  vector<T> vec;
  std::stringstream ss ( s );
  string buf;
  while ( ss >> buf )
  {
    vec.push_back ( string_as_T<T> ( buf ) );
  }
  return vec;
}

/**
 * @name string_as_TDeque
 *
 * @param s
 * @return t
 *
 * Template class that converts from a string to a deque of 
 * Type T where each entry is separated by white-space
 */

template<class T>
deque<T> Config::string_as_TDeque ( const string& s )
{
  deque<T> dec;
  std::stringstream ss ( s );
  string buf;
  while ( ss >> buf )
  {
    dec.push_back ( string_as_T<T> ( buf ) );
  }
  return dec;
}


/**
 * @name read
 *
 * @param region
 * @param key
 * @return value
 *
 * Template class that reads from the configuration and returns the 
 * value corresponding to a particular region/key pair
 */


template<class T>
T Config::read ( const string& region, const string& key ) const
{
  // Read the value corresponding to key
  mapci p = configContents.find ( pair < string, string > (region, key) );
  if ( p == configContents.end() ) throw key_not_found ( region, key );
  return string_as_T<T>( p->second );
}


/**
 * @name readVector
 *
 * @param region
 * @param key
 * @return value
 *
 * Template class that reads from the configuration and returns a vector of the
 * values corresponding to a particular region/key pair
 */


template<class T>
vector<T> Config::readVector ( const string& region, const string& key ) const
{
  // Read the value corresponding to key
  mapci p = configContents.find ( pair < string, string > (region, key) );
  if ( p == configContents.end() ) throw key_not_found ( region, key );
  return string_as_TVector<T>( p->second );
}


/**
 * @name readDeque
 *
 * @param region
 * @param key
 * @return value
 *
 * Template class that reads from the configuration and returns a deque of the
 * values corresponding to a particular region/key pair
 */


template<class T>
deque<T> Config::readDeque ( const string& region, const string& key ) const
{
  // Read the value corresponding to key
  mapci p = configContents.find ( pair < string, string > (region, key) );
  if ( p == configContents.end() ) throw key_not_found ( region, key );
  return string_as_TDeque<T>( p->second );
}


/**
 * @name add
 *
 * @param region
 * @param key
 * @param value
 * @return
 *
 * Template class that adds a value to the configuration
 * corresponding to a particular region/key pair
 */

template<class T>
void Config::add ( string region, string key, const T& value )
{
  // Add a key with given value
  string v = T_as_string( value );
  trim ( key );
  trim ( region );
  trim ( v );
  configContents[ pair < string, string > ( region, key ) ] = v;
  return;
}


/**
 * @name addDeque
 *
 * @param region
 * @param key
 * @param value
 * @return
 *
 * Template class that adds a value to the configuration
 * corresponding to a particular region/key pair
 */

template<class T>
void Config::addDeque ( string region, string key, const deque<T> &value )
{
  std::ostringstream tmp;
  // Add a key with given value
  for ( unsigned int x = 0; x < value.size(); x++)
  {
    tmp << T_as_string ( value[x] );
    if ( x < (value.size( ) - 1) )
    {
      tmp << " ";
    }
  }
  trim ( key );
  trim ( region );
  configContents[ pair < string, string > ( region, key ) ] = tmp.str ( );
  return;
}

#endif

