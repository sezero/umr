#ifndef _UMR_H
#define _UMR_H

int upkg_open(char *);		// open a upkg format file.
void upkg_close(void);		// close a upkg format file, previously opened.

signed int upkg_ocount(void);	// returns the number of exports
char *upkg_oname(signed int);	// returns the name of the export
char *upkg_oclassname(signed int);	// returns the name of the export's class
char *upkg_opackagename(signed int);	// returns the name of the export's package
char *upkg_otype(signed int);	// returns the name of the type of object
signed int upkg_export_size(signed int);	// return the size of the export described
signed int upkg_object_size(signed int);	// return the size of the object described
signed int upkg_export_offset(signed int);	// return the offset to said export
signed int upkg_object_offset(signed int);	// same

int upkg_read(void *, signed int, signed int);	// read data from the upkg file
int upkg_export_dump(char *, signed int);	// dump an export
int upkg_object_dump(char *, signed int);	// dump an object

#endif				// _UMR_H
