#include <elf.h>

/// elf header structure gotten from elf 64
typedef struct {
   uint8_t  ident[16];      /// contains magic and other things
   uint16_t type;           /// things like dyn
   uint16_t machine;        /// machine
   uint32_t version;        /// version
   uint64_t entry_point;    /// entry point
   uint64_t ph_offset;      /// program headers
   uint64_t sh_offset;      /// section headers
   uint32_t flags;          /// flags
   uint16_t eh_size;        /// size of header
   uint16_t ph_entry_size;  /// size of entry
   uint16_t ph_entry_count; /// count
   uint16_t sh_entry_size;  /// section size
   uint16_t sh_entry_count; /// section count
   uint16_t sh_str_index;   /// string index
} elfHeader;

/// symbol table from elf 64
typedef struct {
    uint32_t    st_name;    /// name
    uint8_t     st_info;    /// info   
    uint8_t     st_other;   /// other
    uint16_t    st_shndx;   /// ndx
    Elf64_Addr  st_value;   /// value
    uint64_t    st_size;    /// size
} elfSymbolTable;

void printELFHeader(elfHeader* header);
void printSymbolTable(elfSymbolTable *symbolTable, char *strtab, int type, int symtab_entries);