#ifndef _URF_H
#define _URF_H

#include <stddef.h>
#include <stdint.h>
#include "umr.h"


/* orig. documentation by Tim Sweeney was at
 * http://unreal.epicgames.com/Packages.htm
 * also see Unreal Wiki at
 * http://wiki.beyondunreal.com/Legacy:Package_File_Format
 */

typedef int32_t fci_t;		/* FCompactIndex */

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

#define UPKG_HDR_TAG		0x9e2a83c1

/* upkg_flags: */
#define RF_Transactional	0x00000001
#define RF_SourceModified	0x00000002
#define RF_Public		0x00000004
#define RF_LoadForClient	0x00010000
#define RF_LoadForServer	0x00020000
#define RF_LoadForEdit		0x00040000
#define RF_Standalone		0x00080000
#define RF_HasStack		0x02000000
#define RF_Intrinsic		0x04000000

struct _genhist {	/* for upkg versions >= 68 */
	int32_t export_count;
	int32_t name_count;
};

struct upkg_hdr {
	uint32_t tag;	/* UPKG_HDR_TAG */
	int32_t file_version;
	uint32_t pkg_flags;
	int32_t name_count;	/* number of names in name table (>= 0) */
	int32_t name_offset;		/* offset to name table  (>= 0) */
	int32_t export_count;	/* num. exports in export table  (>= 0) */
	int32_t export_offset;		/* offset to export table (>= 0) */
	int32_t import_count;	/* num. imports in export table  (>= 0) */
	int32_t import_offset;		/* offset to import table (>= 0) */

	/* number of GUIDs in heritage table (>= 1) and table's offset:
	 * only with versions < 68. */
	int32_t heritage_count;
	int32_t heritage_offset;
	/* with versions >= 68:  a GUID, a dword for generation count
	 * and export_count and name_count dwords for each generation: */
	uint32_t guid[4];
	int32_t generation_count;
#define UPKG_HDR_SIZE 64			/* 64 bytes up until here */
	struct _genhist *gen;
};
/* compile time assert for upkg_hdr size */
typedef int _check_hdrsize[2 * (offsetof(struct upkg_hdr, gen) == UPKG_HDR_SIZE) - 1];


/* ObjectReference is like a pointer.
 *   0 : ignore (NULL)
 * < 0 : imports[-index - 1]
 * > 0 : exports[ index - 1]
 */
struct upkg_export {
	fci_t class_index;	/* ObjectReference */
	fci_t super_index;	/* ObjectReference */
	int32_t package_index;	/* ObjectReference (not in unreal beta, i.e. file_version < 60) */
	fci_t object_name;
	uint32_t object_flags;
	fci_t serial_size;
	fci_t serial_offset;	/* if serial_size > 0 */

	/* the rest not stored in pkg but generated at runtime */
	int32_t class_name;
	int32_t package_name;
	int32_t type_name;
	int32_t object_size;
	int32_t object_offset;
};

struct upkg_import {
	fci_t class_package;
	fci_t class_name;
	int32_t package_index;	/* ObjectReference (unreal beta has an FCompactIndex here???) */
	fci_t object_name;
};

#define UPKG_NAME_NOCOUNT	-1
struct upkg_name {
	char name[UPKG_MAX_NAME_SIZE];
	uint32_t flags;
};

/* opaque upkg object that upkg_open() returns */
struct upkg {
	FILE *file;
	struct upkg_hdr *hdr;
	struct upkg_export *exports;
	struct upkg_import *imports;
	struct upkg_name *names;
	int indent_level;
};

#endif	/* _URF_H */
