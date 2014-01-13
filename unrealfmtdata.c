#include <stdio.h>
#include <stdlib.h>

#include "urf.h"

/* version,  class_name, order */
const upkg_export_hdr export_desc[] = {
/* unreal beta 1998 -- two leading unknown dwords (00 00 00 00 00 00 00 00) in v35/v37.
 * four unknown dwords (81 00 00 00 00 00 FF FF FF FF FF FF FF FF 00 00) in all v35-41. */
	{35, "Music",   "3j3j3j3j3j3jFjFnFd"},
	{37, "Music",   "3j3j3j3j3j3jFjFnFd"},
	{35, "Sound",   "3j3j3j3j3j3jFjFnFd"},
	{37, "Sound",   "3j3j3j3j3j3jFjFnFd"},
	{40, "Music",       "3j3j3j3jFjFnFd"},
	{41, "Music",       "3j3j3j3jFjFnFd"},
	{42, "Sound",       "3j3j3j3jFjFnFd"},

/* unreal retail (pkg ver.60/61) */
	{61, "Music",   "FjFnFd"},
	{61, "Sound",   "FjFnFd"},

	{62, "Music", "FjFn3sFd"},
	{62, "Sound", "FjFn3sFd"},

	{63, "Music", "FjFn3sFd"},	/* Return to NaPali */
	{63, "Sound", "FjFn3sFd"},	/* Wheel of Time */

	{64, "Music", "FjFn3sFd"},
	{64, "Sound", "FjFn3sFd"},

	{65, "Sound", "FjFn3sFd"},	/* Tactical Ops, UT */

	{66, "Music", "FjFn3sFd"},
	{66, "Sound", "FjFn3sFd"},

	{68, "Music", "FjFn3sFd"},
	{68, "Sound", "FjFn3sFd"},

/* Tactical Ops, UT, ... */
	{69, "Sound", "FjFn3sFd"},
	{69, "Music", "FjFn3sFd"},

/* DS9 The Fallen */
	{71, "Sound", "FjFn3sFd"},
	{72, "Sound", "FjFn3sFd"},
	{73, "Sound", "FjFn3sFd"},

/* Harry Potter and the Philosopher's Stone */
	{75, "Music", "FjFn3sFd"},
	{75, "Sound", "FjFn3sFd"},
	{76, "Music", "FjFn3sFd"},
	{76, "Sound", "FjFn3sFd"},

/* Undying */
/*  (extra dwords are unknown,  marked as junk for now..) */
	{79, "Sound", "FjFn3j3j3j3j3j3sFd"},
	{80, "Sound", "FjFn3j3j3j3j3j3j3sFd"},
	{81, "Sound", "FjFn3j3j3j3j3j3j3sFd"},
	{83, "Sound", "FjFn3j3j3j3j3j3j3j3sFd"},
	{85, "Sound", "FjFn3j3j3j3j3j3j3j3sFd"},

/* Harry Potter and the Chamber of Secrets */
/*  (extra dwords are unknown,  marked as junk for now..) */
/*  FIXME: only WAVE. what about those "XA" stuff there?? */
	{79, "Sound", "FjFn3j3j3j3j3j3j3sFd"},

/* Mobile Forces */
	{80, "Sound", "FjFn3sFd"},
	{81, "Sound", "FjFn3sFd"},
	{82, "Sound", "FjFn3sFd"},
	{83, "Sound", "FjFn3sFd"},
	{83, "Music", "FjFn3sFd"},

/* last entry must have version == 0 */
	{0, "", ""}
};

const upkg_object_hdr object_desc[] = {
	{"s3m", "SCRM", 44, "ScreamTracker 3"},
	{"it", "IMPM", 0, "Impulse Tracker"},
	{"xm", "Fast", 38, "FastTracker 2.0"},
	{"WAV", "WAVE", 8, "MS PCM Sound"},

/* last entry must have sig_offset == -1 */
	{"", "", -1, ""}
};
