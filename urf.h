#ifndef _URF_H
#define _URF_H

#include <stdint.h>
#include "umr.h"


/* orig. documentation by Tim Sweeney was at
 * http://unreal.epicgames.com/Packages.htm  */

#define UPKG_MAX_NAME_SIZE	64
#define UPKG_MAX_ORDERS		10

/* all data types in upkg files are signed */
#define UPKG_DATA_FCI	'F'	/* FCompactIndex */
#define UPKG_DATA_32	'3'		/* int32 */
#define UPKG_DATA_16	'1'		/* int16 */
#define UPKG_DATA_8	'8'		/* int8  */
#define UPKG_DATA_ASCIC	'C'	/* <byte numchars><string>'\0' */
#define UPKG_DATA_ASCIZ	'Z'	/* simply a usual <string>'\0' */

#define UPKG_OBJ_JUNK	'j'
#define UPKG_OBJ_NAME	'n'
#define UPKG_EXP_SIZE	's'
#define UPKG_OBJ_SIZE	'd'

#define UPKG_NAME_NOCOUNT	-1

#define UPKG_HDR_TAG	0x9e2a83c1

enum upkg_flags {
    RF_Transactional	= 0x00000001,
    RF_SourceModified	= 0x00000002,
    RF_Public		= 0x00000004,
    RF_LoadForClient	= 0x00010000,
    RF_LoadForServer	= 0x00020000,
    RF_LoadForEdit	= 0x00040000,
    RF_Standalone	= 0x00080000,
    RF_HasStack		= 0x02000000,
    RF_Intrinsic	= 0x04000000
};

#pragma pack(1)
struct unreal_pkg_hdr {
	uint32_t tag;	// hdr tag, should == UPKG_HDR_TAG
	int32_t file_version,	// should be 61 for unreal.  > for UT
	 pkg_flags,		// bitflags..  we don't need them
	 name_count,		// number of names in name table (>= 0)
	 name_offset,		// offset to name table          (>= 0)
	 export_count,		// num. exports in export table  (>= 0)
	 export_offset,		// offset to export table        (>= 0)
	 import_count,		//  -- seeing a trend yet?       (>= 0)
	 import_offset;		//                               (>= 0)

	/* number of GUIDs in heritage table (>= 1) and table's offset:
	 * with version < 68. */
	int32_t heritage_count,
	 heritage_offset;

	/* with versions >= 68:  a GUID, a dword for generation count
	   and export_count and name_count dwords for each generation:
	uint32_t guid[4];
	int32_t generation_count;
	struct _genhist {
		int32_t export_count,
			name_count;
	} genhist[generation_count];
	*/
};
#pragma pack()


/* indices have 2 types.  type 1 is harder, so I'll describe type 2 first. =)

  type 2 is an index into the name table (upkg_name_table).  pure and simple.
  
  type 1 is an index into either the imports table, or the exports table, or NULL.
   if index == 0, you can ignore it
   if index < 0, use imports[-index - 1]
   if index > 0, use exports[index - 1]
   
  type 1 is used for dependency/inheritancy info
*/
struct unreal_pkg_export_tbl {
	int32_t class_index,	// index, type 1
	 package_index,		// index, type 1
	 super_index,		// index, type 1
	 object_name,		// index, type 2
	 object_flags,		// flags for the object (will be supported when I decide to code it ;-)
	 serial_size,		// size of export described
	 serial_offset,		// start of the export in the the package file (offset from beginning of file)
	 class_name,		// index, type 2 (the name of the object class)
	 package_name,		// index, type 2 (the name of the object package)
	 type_name,		// index, type 2 (the name of the object type)
	 object_size,		// bytes of data in object
	 object_offset;		// offset into package file that object starts
};

struct unreal_pkg_import_tbl {
	int32_t class_package,	// index, type 2
	 class_name,		// index, type 2
	 package_index,		// index, type 1
	 object_name;		// index, type 2
};

struct unreal_pkg_name_tbl {
	char name[UPKG_MAX_NAME_SIZE];	// a name
	int32_t flags;	// flags for the name
};

struct unreal_pkg_export_hdr {
	int32_t version;	// version of pkg header this supports
	char class_name[UPKG_MAX_NAME_SIZE];	// unreal class
	char order[UPKG_MAX_ORDERS * 10];	// order of the header
};

struct unreal_pkg_object_hdr {
	char type_str[4];	// type string of the object type
	char object_sig[5];	// sig of the object data (if exists)
	int sig_offset;		// offset in object that object_sig occurs
	char desc[33];		// description of the object
};

typedef struct unreal_pkg_hdr upkg_hdr;
typedef struct unreal_pkg_export_tbl upkg_exports;
typedef struct unreal_pkg_import_tbl upkg_imports;
typedef struct unreal_pkg_name_tbl upkg_names;
typedef struct unreal_pkg_export_hdr upkg_export_hdr;
typedef struct unreal_pkg_object_hdr upkg_object_hdr;


extern const upkg_export_hdr export_desc[];
extern const upkg_object_hdr object_desc[];

#endif	/* _URF_H */
