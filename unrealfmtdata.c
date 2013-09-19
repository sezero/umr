#include <stdio.h>
#include <stdlib.h>

#include "urf.h"


// version,  class_name, order
upkg_export_hdr export_desc[] = {
	{61, "Music",   "FjFnFd"},
	{61, "Sound",   "FjFnFd"},
	{61, "Palette", ""},

	{62, "Music", "FjFn3sFd"},
	{62, "Sound", "FjFn3sFd"},
	{62, "Palette", ""},

	{63, "Music", "FjFn3sFd"},	// 63: Return to NaPali
	{63, "Sound", "FjFn3sFd"},
	{63, "Palette", ""},

	{64, "Music", "FjFn3sFd"},
	{64, "Music", "FjFn3sFd"},
	{64, "Palette", ""},

	{66, "Music", "FjFn3sFd"},
	{66, "Sound", "FjFn3sFd"},
	{66, "Palette", ""},

	{68, "Music", "FjFn3sFd"},
	{68, "Sound", "FjFn3sFd"},
	{68, "Palette", ""},

	{0, "", ""}		// last entry must have version == 0
};

upkg_object_hdr object_desc[] = {
	{"s3m", "SCRM", 44, "ScreamTracker 3"}
	,
	{"it", "IMPM", 0, "Impluse Tracker"}
	,
	{"xm", "Fast", 38, "FastTracker 2.0"}
	,
	{"WAV", "WAVE", 8, "MS PCM Sound"}
	,

	{"", "", -1, ""}	// last entry must have sig_offset == 0
};
