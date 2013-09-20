#include <stdio.h>
#include <stdlib.h>

#include "urf.h"


// version,  class_name, order
const upkg_export_hdr export_desc[] = {
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

	{65, "Sound", "FjFn3sFd"},	// UT: AmbCity.uax, Pan1.uax

	{66, "Music", "FjFn3sFd"},
	{66, "Sound", "FjFn3sFd"},
	{66, "Palette", ""},

	{68, "Music", "FjFn3sFd"},
	{68, "Sound", "FjFn3sFd"},
	{68, "Palette", ""},

	{69, "Sound", "FjFn3sFd"},	// UT: chaossounds.uax, chaossounds2.uax

	{71, "Sound", "FjFn3sFd"},	// DS9 The Fallen
	{72, "Sound", "FjFn3sFd"},	//
	{73, "Sound", "FjFn3sFd"},	//

	{0, "", ""}		// last entry must have version == 0
};

const upkg_object_hdr object_desc[] = {
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
