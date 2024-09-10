///////////////////////////////////////////////////////////////////////////////
//  University of Hawaii, College of Engineering
//  Lab 3 - wc - SRE - Spring 2024
//
/// wc - print line, word, and byte counts for each file
///
/// @see     https://linuxcommand.org/lc3_man_pages/wc1.html
///
/// @file    wc.c
/// @author  Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>     // For printf()
#include <stdlib.h>    // For exit(), EXIT_SUCCESS, EXIT_FAILURE
#include <sys/stat.h>  // For stat()
#include <unistd.h>    // For close()
#include <sys/mman.h>  // For mmap(), munmap()
#include <fcntl.h>     // For open()
#include <string.h>    // For strchr()


const char PROGRAM_NAME[]    = "wc" ;
const char PRINTABLE_CHARS[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789`~!@#$%^&*()-_=+[{]}\\|;:'\",<.>/?" ;


void processFile( char fileName[], char* fileBuffer, size_t fileSize ) {
   size_t offset   = 0;
   size_t numChars = 0;
   size_t numWords = 0;
   size_t numLines = 0;

   /// This will be used by our state machine to determine when we are in
   /// a word or not.  Words have printable characters.  Anything that's
   /// not printable is not part of a word.
   ///
   /// wc will count a word when we make the transition from NOT_IN_WORD
   /// to IN_WORD.
   enum WORD_STATE { IN_WORD, NOT_IN_WORD } wordState = NOT_IN_WORD;

   while( offset < fileSize ) {
      numChars++;  // For now, everything's a character

      // strchr() returns a pointer to the matched character or NULL if the
      // character is not found
      if( strchr( PRINTABLE_CHARS, fileBuffer[offset] ) != NULL ) {
         // We found a printable character
         if( wordState == IN_WORD ) {
            // Do nothing... we're in a word and we just found another character.
         } else {
            // This is the first character of a new word...
            wordState = IN_WORD;
            numWords++;
         }
      } else {
         // No printable character here
         if( wordState == IN_WORD ) {
            // We were in a word, now we're not
            wordState = NOT_IN_WORD;
         } else {
            // Do nothing... we weren't in a word and we're still not in a word
         }
      }

      // Counting lines is tricky.  Linux systems count lines with LF (0x0A).
      // Windows systems count lines with CR LF (0x0D 0x0A).
      // We will count a line every time we find a LF (0x0A).
      if( fileBuffer[offset] == 0x0A ) {
         numLines++;
      }

      offset++; // Done with this character... move onto the next character
   }

   printf( "%zu\t%zu\t%zu\t%s\n", numLines, numWords, numChars, fileName );
}


/// The main entry point for wc
///
///   - Process command line parameters
///   - Orchistrate the file management
///   - Call a dedicated function to process files
int main( int argc, char* argv[] ) {

   if( argc != 2 ) {
      printf( "Usage: %s FILE\n", PROGRAM_NAME );
      exit( EXIT_FAILURE );
   }

	char* fileName = argv[1];  ///< The first argument on the command line is the fileName

   struct stat fileInfo;      ///< Holds information about the file

   if( stat( fileName, &fileInfo ) != 0 ) {
      // stat() returns 0 on success and -1 if there's a problem.
      printf( "%s: Can't open [%s] (stat)\n", PROGRAM_NAME, fileName );
      exit( EXIT_FAILURE );
   }

   int fd; ///< The file descriptor for the file
   fd = open ( fileName, O_RDONLY );
   if( fd < 0 ) {
      // open() returns -1 on error; otherwise, it returns a nonnegative file descriptor
      printf( "%s: Can't open [%s] (open)\n", PROGRAM_NAME, fileName );
      exit( EXIT_FAILURE );
   }

   void* fileBuffer;
   fileBuffer = mmap( NULL, fileInfo.st_size, PROT_READ, MAP_PRIVATE, fd, 0 );
   if( fileBuffer == MAP_FAILED ) {
      // mmap() returns a pointer to the mapped area or MAP_FAILED
      printf( "%s: Can't open [%s] (mmap)\n", PROGRAM_NAME, fileName );
      exit( EXIT_FAILURE );

      /// @todo:  `mmap` fails on 0-length files, but `wc` should be able
      ///          to process them.  Make an exception.
   }

   processFile( fileName, fileBuffer, fileInfo.st_size );

   munmap( fileBuffer, fileInfo.st_size );
	/// @todo Check the exit status
	
   close( fd );
	/// @todo Check the exit status

   exit( EXIT_SUCCESS );
}
