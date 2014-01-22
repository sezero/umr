#ifndef _UMR_H
#define _UMR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct upkg;

struct upkg *upkg_open (const char *);	/* open a upkg format file. */
void upkg_close (struct upkg *);	/* close a upkg format file, previously opened. */

int32_t upkg_ocount (struct upkg *);		/* returns the number of exports */
const char *upkg_oname (struct upkg *, int);		/* returns the name of the export */
const char *upkg_oclassname (struct upkg *, int);	/* returns the name of the export's class */
const char *upkg_opackagename (struct upkg *, int);	/* returns the name of the export's package */
const char *upkg_otype (struct upkg *, int);		/* returns the name of the type of object */
int32_t upkg_export_size (struct upkg *, int);		/* return the size of the export described */
int32_t upkg_object_size (struct upkg *, int);		/* return the size of the object described */
int32_t upkg_export_offset (struct upkg *, int);	/* return the offset to said export */
int32_t upkg_object_offset (struct upkg *, int);	/* same */

int upkg_read (struct upkg *, void *, size_t, long);		/* read data from the upkg */
int upkg_export_dump (struct upkg *, const char *, int);		/* dump an export */
int upkg_object_dump (struct upkg *, const char *, int);		/* dump an object */

#ifdef __cplusplus
}
#endif

#endif	/* _UMR_H */
