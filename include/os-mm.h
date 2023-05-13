#ifndef OSMM_H
#define OSMM_H

//#define MM_PAGING
#define PAGING_MAX_MMSWP 4 /* max number of supported swapped space */
#define PAGING_MAX_SYMTBL_SZ 30

typedef char BYTE;
typedef uint32_t addr_t;
//typedef unsigned int uint32_t;

struct pgn_t{
   int pgn;
   struct pgn_t *pg_next; 
};

/*
 *  Memory region struct 
 */
struct vm_rg_struct {
   unsigned long rg_start; 
   unsigned long rg_end;

   struct vm_rg_struct *rg_next;
};

/*
 *  Memory area struct
 */
struct vm_area_struct {
   unsigned long vm_id;
   unsigned long vm_start;  
   unsigned long vm_end;

   unsigned long sbrk; // space increments after program break address
/*
 * Derived field
 * unsigned long vm_limit = vm_end - vm_start
 */
   struct mm_struct *vm_mm;   // associated mm_struct
   struct vm_rg_struct *vm_freerg_list;   // free region list
   struct vm_area_struct *vm_next;  // multiple vm_area_struct structs to be linked together.
};

/* 
 * Memory management struct
 */
struct mm_struct {
   /* Page table directory, contains all page table entries */
   uint32_t *pgd;

   struct vm_area_struct *mmap;  // list of vm_area_struct

   /* Currently we support a fixed number of symbol */
   struct vm_rg_struct symrgtbl[PAGING_MAX_SYMTBL_SZ];

   /* list of free page */
   struct pgn_t *fifo_pgn;
};

/*
 * FRAME/MEM PHY struct
 */
struct framephy_struct { 
   int fpn;    // frame number
   struct framephy_struct *fp_next; // next frame

   /* Resereed for tracking allocated framed */
   struct mm_struct* owner;   // owner of this frame
};

struct memphy_struct {
   /* Basic field of data and size */
   BYTE *storage;
   int maxsz;
   
   /* Sequential device fields */ 
   int rdmflg; // 1 if random access, 0 if sequential access
   int cursor; // current cursor position

   /* Management structure */
   struct framephy_struct *free_fp_list;
   struct framephy_struct *used_fp_list;
};

#endif
