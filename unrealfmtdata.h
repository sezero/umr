struct _upkg_export_desc {
	int32_t version;	/* version of pkg header this supports */
	char class_name[UPKG_MAX_NAME_SIZE];	/* unreal class */
	char order[UPKG_MAX_ORDERS * 10];	/* order of the header */
};

struct _upkg_object_desc {
	char type_str[4];	/* type string of the object type */
	char object_sig[5];	/* sig of the object data (if exists) */
	int sig_offset;		/* offset in object that object_sig occurs */
	char desc[33];		/* description of the object */
};

/* version, class_name, order */
static const struct _upkg_export_desc export_desc[] = {
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

static const struct _upkg_object_desc object_desc[] = {
	{"s3m", "SCRM", 44, "ScreamTracker 3"},
	{"it", "IMPM", 0, "Impulse Tracker"},
	{"xm", "Fast", 38, "FastTracker 2.0"},
	{"WAV", "WAVE", 8, "MS PCM Sound"},

/* last entry must have sig_offset == -1 */
	{"", "", -1, ""}
};
