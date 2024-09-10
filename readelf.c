///////////////////////////////////////////////////////////////////////////////
//  University of Hawaii, College of Engineering
//  Lab 5 - readelf - SRE - Spring 2024
//
/// readelf - reads the elf file format of linux files
///
/// @file    readelf.c
/// @author  Dustin Tomi <tomid@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> // Add this line to include uint8_t type
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <elf.h>
#include "readelf.h"

/// The program name
const char PROGRAM_NAME[] = "readelf" ;

/// The main entry point for wc
///
///   - Process command line parameters
///   - Orchistrate the file management
///   - Call a dedicated function to process files
int main( int argc, char* argv[] ) {

   //--------Marks code----------------------------------------
   if ( argc != 2 ) {
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

   void* fileBuffer = mmap( NULL, fileInfo.st_size, PROT_READ, MAP_PRIVATE, fd, 0 );

   if( fileBuffer == MAP_FAILED ) {
      // mmap() returns a pointer to the mapped area or MAP_FAILED
      printf( "%s: Can't open [%s] (mmap)\n", PROGRAM_NAME, fileName );
      exit( EXIT_FAILURE );

      /// @todo:  `mmap` fails on 0-length files, but `wc` should be able
      ///          to process them.  Make an exception.
   }
   //--------------------------------------------------------------

   elfHeader *header = (elfHeader *) fileBuffer;

   printELFHeader(header);

   #ifdef SYMBOL
   elfSymbolTable *symtab = NULL;
   char *strtab = NULL;

   // Locate symbol and string tables
   for (int i = 0; i < header->sh_entry_count; ++i) {
      // struct Elf64_Shdr {
      //    Elf64_Word sh_name;
      //    Elf64_Word sh_type;
      //    Elf64_Xword sh_flags;
      //    Elf64_Addr sh_addr;
      //    Elf64_Off sh_offset;
      //    Elf64_Xword sh_size;
      //    Elf64_Word sh_link;     
      //    Elf64_Word sh_info;
      //    Elf64_Xword sh_addralign;
      //    Elf64_Xword sh_entsize;
      // };

      // start of file + start of sections + i * sections size
      Elf64_Shdr* section = (Elf64_Shdr*) (
         (uint8_t*)fileBuffer + 
         header->sh_offset + 
         (i * header->sh_entry_size)
      );
      #ifdef DEBUG
      printf(" type : %ld\n",section->sh_type);
      #endif
      
      // check for .symtab and .dynsym
      if (section->sh_type == SHT_SYMTAB || section->sh_type == SHT_DYNSYM) {
         #ifdef DEBUG
         printf(" offset : %ld\n",section->sh_offset);
         printf(" count : %ld\n", section->sh_size / section->sh_entsize);
         #endif

         symtab = (elfSymbolTable*) ((uint8_t*)fileBuffer + section->sh_offset);

         // string table = start + offset
         // offset = start + header + link * size

         Elf64_Shdr* stringTable = (Elf64_Shdr *) ((uint8_t *)fileBuffer + header->sh_offset + (section->sh_link * header->sh_entry_size));
         strtab = (char *)((uint8_t *)fileBuffer + stringTable->sh_offset);

         printSymbolTable(symtab, strtab, section->sh_type, section->sh_size / section->sh_entsize);
      }
   }
   #endif

   //--------------------------------------------------------------
   munmap( fileBuffer, fileInfo.st_size );
	
   close( fd );

   exit( EXIT_SUCCESS );
}

/// Prints the elf header file similar to using readelf -h
//
/// @param elfHeader* pointer to the header to print
void printELFHeader(elfHeader* header) {
   #ifdef HEADER
   char str[20];

   printf("ELF Header:\n");
   printf("  Magic:   ");
   for (int i = 0; i < 16; ++i) {
      printf("%02x ", header->ident[i]);
   }

   printf("\n");
   printf("  Class:                             %s\n", header->ident[4] == 1 ? "ELF64" : "ELF32");
   printf("  Data:                              %s\n", header->ident[5] == 1 ? "2's complement, little endian" : "Unknown");
   printf("  Version:                           %d (current)\n", header->ident[6]);

   char* osABI;
   switch(header->ident[7]) {
      case 0:
         osABI = "UNIX System V\0";
         break;
      case 1:
         osABI = "HP-UX\0";
         break;
      case 2:
         osABI = "NetBSD\0";
         break;
      case 3:
         osABI = "Linux\0";
         break;
      case 4:
         osABI = "Solaris\0";
         break;
      case 5:
         osABI = "IRIX\0";
         break;
      case 6:
         osABI = "FreeBSD\0";
         break;
      case 7:
         osABI = "TRU64 UNIX\0";
         break;
      case 9:
         osABI = "ARM architecture\0";
         break;
      default:
         osABI = "Stand-alone (embedded)\0";
         break;
   }
   printf("  OS/ABI:                            %s\n", osABI);

   printf("  ABI Version:                       %d\n", header->ident[8]);

   char* typeString;
   switch(header->type) {
      case 0:
         typeString = "Unknown\0";
         break;
      case 1:
         typeString = "REL\0";
         break;
      case 2:
         typeString = "EXEC\0";
         break;
      case 3:
         typeString = "DYN (Position-Independent Executable file)\0";
         break;
      case 4:
         typeString = "CORE\0";
         break;
      default:
         sprintf(str, "%d", header->machine);   // NOLINT
         typeString = str;
         break;
   }
   printf("  Type:                              %s\n", typeString);

   char* machineString;
   switch(header->machine) {
      case 0:
         machineString = "Unknown\0";
         break;
      case 1:
         machineString = "AT&T WE 32100\0";
         break;
      case 2:
         machineString = "SPARC\0";
         break;
      case 3:
         machineString = "Intel 80386\0";
         break;
      case 4:
         machineString = "Motorola 68000\0";
         break;
      case 5:
         machineString = "Motorola 88000\0";
         break;
      case 6:
         machineString = "Intel 80860\0";
         break;
      case 7:
         machineString = "MIPS RS3000\0";
         break;
      case 62:
         machineString = "Advanced Micro Devices X86-64\0";
         break;
      default:
         sprintf(str, "%d", header->machine);   // NOLINT
         machineString = str;
         break;
   }
   printf("  Machine:                           %s\n", machineString);

   printf("  Version:                           0x%x\n", header->version);
   printf("  Entry point address:               0x%lx\n", header->entry_point);
   printf("  Start of program headers:          %lu (bytes into file)\n", header->ph_offset);
   printf("  Start of section headers:          %lu (bytes into file)\n", header->sh_offset);
   printf("  Flags:                             0x%x\n", header->flags);
   printf("  Size of this header:               %d (bytes)\n", header->eh_size);
   printf("  Size of program headers:           %d (bytes)\n", header->ph_entry_size);
   printf("  Number of program headers:         %d\n", header->ph_entry_count);
   printf("  Size of section headers:           %d (bytes)\n", header->sh_entry_size);
   printf("  Number of section headers:         %d\n", header->sh_entry_count);
   printf("  Section header string table index: %d\n", header->sh_str_index);
   #endif
}

/// Prints the symbol table similar to readelf -s
//
/// @param symbolTable the symbol table
/// @param strtab the string table to print the names
/// @param type the type of symbol table ie .symtab or .dynsym
/// @param entries the number of entires to print 
void printSymbolTable(elfSymbolTable *symbolTable, char *strtab, int type, int entries) {
   char str[20];

   printf("\nSymbol table '%s' contains %d entries:\n", ((type == 2) ? ".symtab" : ".dynsym"), entries);
   printf("   Num:    Value          Size Type    Bind   Vis      Ndx Name\n");

   for (int i = 0; i < entries; ++i) {
      printf("  %4d", i);
      printf(": %016lx", symbolTable[i].st_value);
      printf("%6lu", symbolTable[i].st_size);

      char* symbolTypeString;
      switch (ELF64_ST_TYPE(symbolTable[i].st_info)) {
        case STT_NOTYPE:
            symbolTypeString = "NOTYPE";
            break;
        case STT_OBJECT:
            symbolTypeString =  "OBJECT";
            break;
        case STT_FUNC:
            symbolTypeString =  "FUNC";
            break;
        case STT_SECTION:
            symbolTypeString = "SECTION";
            break;
        case STT_FILE:
            symbolTypeString = "FILE";
            break;
        case STT_COMMON:
            symbolTypeString = "COMMON";
            break;
        case STT_TLS:
            symbolTypeString = "TLS";
            break;
        default:
            symbolTypeString = "UNKNOWN";
            break;
      }
      printf(" %-7s", symbolTypeString);

      char* infoString;
      switch (ELF64_ST_BIND(symbolTable[i].st_info)) {
         case STB_LOCAL:
            infoString = "LOCAL";
            break;
         case STB_GLOBAL:
            infoString = "GLOBAL";
            break;
         case STB_WEAK:
            infoString = "WEAK";
            break;
         default:
            infoString = "UNKNOWN";
            break;
      }
      printf(" %-6s", infoString);

      char* visString;
      switch(symbolTable[i].st_other) {
         case 0:
            visString = "DEFAULT";
            break;
         case 1:
            visString = "INTERNAL";
            break;
         case 2:
            visString = "HIDDEN";
            break;
         case 3:
            visString = "PROTECTED";
            break;
         default:
            visString = "DEFAULT";
            break;
      }
      printf(" %-7s", visString);

      char* ndxString;
      switch(symbolTable[i].st_shndx) {
         case 0:
            ndxString = "UND";
            break;
         case 65521:
            ndxString = "ABS";
            break;
         default:
            sprintf(str, "%d", symbolTable[i].st_shndx); // NOLINT
            ndxString = str;
            break;
      }
      printf(" %4s", ndxString);

      printf(" %s\n", &strtab[symbolTable[i].st_name]);
   }
}