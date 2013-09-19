#ifndef _UMR_H
#define _UMR_H

#include <stdint.h>

int upkg_open(const char *);	// open a upkg format file.
void upkg_close(void);		// close a upkg format file, previously opened.

signed int upkg_ocount(void);	// returns the number of exports
char *upkg_oname(int);		// returns the name of the export
char *upkg_oclassname(int);	// returns the name of the export's class
char *upkg_opackagename(int);	// returns the name of the export's package
char *upkg_otype(int);		// returns the name of the type of object
int32_t upkg_export_size(int);	// return the size of the export described
int32_t upkg_object_size(int);	// return the size of the object described
int32_t upkg_export_offset(int);	// return the offset to said export
int32_t upkg_object_offset(int);	// same

int upkg_read(void *, size_t, long);		// read data from the upkg file
int upkg_export_dump(const char *, int);	// dump an export
int upkg_object_dump(const char *, int);	// dump an object

#endif				// _UMR_H
