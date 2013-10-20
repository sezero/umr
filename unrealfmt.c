#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "umr.h"
#include "urf.h"


static upkg_hdr *hdr;
static upkg_exports *exports;
static upkg_imports *imports;
static upkg_names *names;

static FILE *file;	/* we store the file pointer globally around here */

static int pkg_opened = 0;		/* sanity check */
static int indent_level;

static char header[64];		/* we load the header into this buffer */


static char *print_flags(signed int flags) {
    static char buf[256];

    memset((void *)buf, 0, 256);

    if (flags & RF_Transactional)
	strcat(buf, "Transactional");

    if (flags & RF_SourceModified)
	strcat(buf, " SrcModified");

    if (flags & RF_Public)
	strcat(buf, " Public");

    if (flags & RF_LoadForClient)
	strcat(buf, " LFClient");

    if (flags & RF_LoadForServer)
	strcat(buf, " LFServer");

    if (flags & RF_LoadForEdit)
	strcat(buf, " LFEdit");
	
    if (flags & RF_Standalone)
	strcat(buf, " Standalone");

    if (flags & RF_HasStack)
	strcat(buf, " HasStack");

    if (flags & RF_Intrinsic)
	strcat(buf, " Intrinsic");

    return buf;
}

static void print_pkg_hdr(void) {
    printf("tag             = 0x%08x\n"
	   "file_version    = %d\n"
	   "pkg_flags       = %s\n"
	   "name_count      = %d\n"
	   "name_offset     = 0x%08x\n"
	   "export_count    = %d\n"
	   "export_offset   = 0x%08x\n"
	   "import_count    = %d\n"
	   "import_offset   = 0x%08x\n"
	   "heritage_count  = %d\n"
	   "heritage_offset = 0x%08x\n",
	   hdr->tag,
	   hdr->file_version,
	   print_flags(hdr->pkg_flags),
	   hdr->name_count,
	   hdr->name_offset,
	   hdr->export_count,
	   hdr->export_offset,
	   hdr->import_count,
	   hdr->import_offset,
	   hdr->heritage_count,
	   hdr->heritage_offset
    );
}

static void print_name(int i) {
	printf("%d: %s\t%s", i, names[i].name, print_flags(names[i].flags));
}

static void indent(int level) {
    int i;

    for (i = -1; i < level; i++)
	printf("  ");
}

static void print_export(int idx) {
    int level, tmp;

    level = indent_level++;

    indent(level - 1);
    printf("EXPORT #%d\n", idx);

    indent(level);
    printf("class_index   = %d\n",  exports[idx].class_index);

    indent(level);
    printf("package_index = %d\n",  exports[idx].package_index);

    indent(level);
    printf("super_index   = %d\n",  exports[idx].super_index);

    indent(level);
    tmp = exports[idx].object_name;
    if (tmp < 0 || tmp > hdr->name_count) {
	tmp = hdr->name_count;
    }
    printf("object_name   = %s\n", names[tmp].name);

    indent(level);
    printf("object_flags  = %s\n", print_flags(exports[idx].object_flags));

    indent(level);
    printf("serial_size   = %d\n",  exports[idx].serial_size);

    indent(level);
    printf("serial_offset = 0x%08x\n",  exports[idx].serial_offset);

    indent(level);
    printf("class_name    = %s\n", names[exports[idx].class_name].name);

    indent(level);
    printf("package_name  = %s\n", names[exports[idx].package_name].name);

    indent(level);
    tmp = exports[idx].type_name;
    if (tmp < 0 || tmp > hdr->name_count) {
	tmp = hdr->name_count;
    }
    printf("type_name     = %s\n", names[tmp].name);

    indent(level);
    printf("object_size   = %d\n",  exports[idx].object_size);

    indent(level);
    printf("object_offset = 0x%08x\n",  exports[idx].object_offset);
}

static void print_import(int idx) {
    int level;

    level = indent_level++;

    indent(level - 1);
    printf("IMPORT #%d\n", idx);

    indent(level);
    printf("class_package = %s\n", names[imports[idx].class_package].name);

    indent(level);
    printf("class_name    = %s\n", names[imports[idx].class_name].name);

    indent(level);
    printf("package_index = %d\n",  imports[idx].package_index);

    indent(level);
    printf("object_name   = %s\n", names[imports[idx].object_name].name);
}

/* decode an FCompactIndex.
 * original documentation by Tim Sweeney was at
 * http://unreal.epicgames.com/Packages.htm
 */
static int32_t get_fci(const char *in, int *pos)
{
	int32_t a;
	int size;

	size = 1;
	a = in[0] & 0x3f;

	if (in[0] & 0x40) {
		size++;
		a |= (in[1] & 0x7f) << 6;

		if (in[1] & 0x80) {
			size++;
			a |= (in[2] & 0x7f) << 13;

			if (in[2] & 0x80) {
				size++;
				a |= (in[3] & 0x7f) << 20;

				if (in[3] & 0x80) {
					size++;
					a |= (in[4] & 0x3f) << 27;
				}
			}
		}
	}

	if (in[0] & 0x80)
		a = -a;

	*pos += size;

	return a;
}

/* read Little Endian data in an endian-neutral way */
#define READ_INT16(b) ((b)[0] | ((b)[1] << 8))
#define READ_INT32(b) ((b)[0] | ((b)[1] << 8) | ((b)[2] << 16) | ((b)[3] << 24))
static int32_t get_s32(const void *addr, int *pos)
{
	const unsigned char *p = (const unsigned char *)addr;
	*pos += 4;
	return (int32_t)READ_INT32(p);
}

static int32_t get_s16(const void *addr, int *pos)
{
	const unsigned char *p = (const unsigned char *)addr;
	*pos += 2;
	return (int16_t)READ_INT16(p);
}

static int32_t get_s8(const void *addr, int *pos)
{
	*pos += sizeof(signed char);
	return *(signed char *) addr;
}

static char *get_string(const char *addr, int count, int *pos)
{
	static char buf[256];

	if (count > UPKG_MAX_NAME_SIZE || count == UPKG_NAME_NOCOUNT)
		count = UPKG_MAX_NAME_SIZE;

	strncpy(buf, addr, count);	/* the string stops at count chars, or is ASCIIZ */

	*pos += strlen(buf) + 1;

	return buf;
}

static int export_index(int i)
{
	if (i > 0) {
		int j = i - 1;
		if (j < hdr->export_count)
			return j;
	}

	return -1;
}

static int import_index(int i)
{
	if (i < 0) {
		int j = -i - 1;
		if (j < hdr->import_count)
			return j;
	}

	return -1;
}

/* idx == exports[idx], c_idx == index to the next element from idx */
static int set_classname(int idx, int c_idx) {
    int i, next;

    i = c_idx;

    do {
	if (i < 0) {
	    i = import_index(i);
	    if (i < 0) break;
	    print_import(i);
	    if (!strcmp(names[imports[i].class_name].name, "Class")) {
		exports[idx].class_name = imports[i].object_name;
		return imports[i].package_index;
	    }

	    next = imports[i].package_index;
	    if (i == -next - 1) break;/* endless loop with UnrealShare.u */
	}

	if (i > 0) {
	    i = export_index(i);
	    if (i < 0) break;
	    print_export(i);
	    next = exports[i].class_index;
	    if (i ==  next - 1) break;/* endless loop with UnrealShare.u */
	} else {
	    break;
	}

	i = next;
    } while (i >= -hdr->import_count && i < hdr->export_count);

    exports[idx].class_name = hdr->name_count;
    return c_idx;
}

static int set_pkgname(int idx, int c_idx) {
    int i, next;

    i = c_idx;

    do {
	if (i < 0) {
	    i = import_index(i);
	    if (i < 0) break;
	    print_import(i);
	    if (!strcmp(names[imports[i].class_name].name, "Package")) {
		exports[idx].package_name = imports[i].object_name;
		return imports[i].package_index;
	    }

	    next = imports[i].package_index;
	    if (i == -next - 1) break;/* endless loop with UnrealShare.u */
	}

	if (i > 0) {
	    i = export_index(i);
	    if (i < 0) break;
	    print_export(i);

	    next = exports[i].class_index;
	    if (i ==  next - 1) break;/* endless loop with UnrealShare.u */
	} else {
	    break;
	}

	i = next;
    } while (i >= -hdr->import_count && i < hdr->export_count);

    exports[idx].package_name = hdr->name_count;
    return c_idx;
}

/* load in the header, AWA allocating the needed memory for the tables */
static int load_upkg(void)
{
	unsigned char *p;
	uint32_t *swp;
	int i;

	/* byte swap the header (all members are 32 bit LE values) */
	p = (unsigned char *) header;
	swp = (uint32_t *) header;
	for (i = 0; i < (int)sizeof(upkg_hdr)/4; i++, p += 4) {
		swp[i] = READ_INT32(p);
	}

	hdr = (upkg_hdr *) header;

	print_pkg_hdr();

	if (hdr->tag != UPKG_HDR_TAG)
		return -1;
	if (hdr->name_count	< 0	||
	    hdr->name_offset	< 0	||
	    hdr->export_count	< 0	||
	    hdr->export_offset	< 0	||
	    hdr->import_count	< 0	||
	    hdr->import_offset	< 0	)
		return -1;

	for (i = 0; export_desc[i].version; i++) {
		if (hdr->file_version == export_desc[i].version) {
			break;
		}
	}

	if (export_desc[i].version == 0)
		return -1;

	names =
	    (upkg_names *) calloc((hdr->name_count + 1), sizeof(upkg_names));
	if (names == NULL)
		return -1;

	exports =
	    (upkg_exports *) calloc(hdr->export_count, sizeof(upkg_exports));
	if (exports == NULL) {
		free(names);
		names = NULL;
		return -1;
	}

	imports =
	    (upkg_imports *) calloc(hdr->import_count, sizeof(upkg_imports));
	if (imports == NULL) {
		free(exports);
		free(names);
		exports = NULL;
		names = NULL;
		return -1;
	}

	return 0;
}

/* load the name table */
static void get_names(void)
{
	int i, idx, c;
	long nofs;
	char readbuf[80];
	const char *str;

	nofs = hdr->name_offset;
	idx = 0;

	for (i = 0; i < hdr->name_count; i++) {
		nofs += idx;
		idx = 0;
		memset(readbuf, 0, 80);
		fseek(file, nofs, SEEK_SET);
		fread(readbuf, 1, 80, file);

		if (hdr->file_version >= 64) {
			c = get_s8(&readbuf[idx], &idx);
			str = get_string(&readbuf[idx], c, &idx);
		} else {
			str = get_string(&readbuf[idx], UPKG_NAME_NOCOUNT, &idx);
		}

		strncpy(names[i].name, str, UPKG_MAX_NAME_SIZE);
		names[i].name[UPKG_MAX_NAME_SIZE - 1] = '\0';

		names[i].flags = get_s32(&readbuf[idx], &idx);

		print_name(i);
		printf("\n");
	}

/* hdr->name_count + 1 names total, this one's last */
	strncpy(names[i].name, "(NULL)", UPKG_MAX_NAME_SIZE);
	names[i].flags = 0;
	printf("\n");
}

/* load the export table (which is at the end of the file... go figure) */
static void get_exports_cpnames(int idx) {
    int x;

    if (idx < 0 || idx >= hdr->export_count)
	return;
    printf("%d\n", idx);

    indent_level = 0;

    print_export(idx);

    x = exports[idx].class_index;

    x = set_classname(idx, x);

    set_pkgname(idx, x);
}

static void get_exports(void)
{
	int i, idx;
	long eofs;
	char readbuf[40];

	eofs = hdr->export_offset;
	idx = 0;

	for (i = 0; i < hdr->export_count; i++) {
		eofs += idx;
		idx = 0;
		memset(readbuf, 0, 40);
		fseek(file, eofs, SEEK_SET);
		fread(readbuf, 1, 40, file);

		exports[i].class_index = get_fci(&readbuf[idx], &idx);
		exports[i].package_index = get_s32(&readbuf[idx], &idx);
		exports[i].super_index = get_fci(&readbuf[idx], &idx);
		exports[i].object_name = get_fci(&readbuf[idx], &idx);
		exports[i].object_flags = get_s32(&readbuf[idx], &idx);
		exports[i].serial_size = get_fci(&readbuf[idx], &idx);

		if (exports[i].serial_size > 0) {
			exports[i].serial_offset =
			    get_fci(&readbuf[idx], &idx);
		} else {
			exports[i].serial_offset = -1;
		}

		get_exports_cpnames(i); /* go grab the class & package names */
	}
}

/* load the import table.  same story as get_exports() */
static void get_imports(void)
{
	int i, idx;
	long iofs;
	char readbuf[40];

	iofs = hdr->import_offset;
	idx = 0;

	for (i = 0; i < hdr->import_count; i++) {
		iofs += idx;
		idx = 0;
		memset(readbuf, 0, 40);
		fseek(file, iofs, SEEK_SET);
		fread(readbuf, 1, 40, file);

		imports[i].class_package = get_fci(&readbuf[idx], &idx);
		imports[i].class_name = get_fci(&readbuf[idx], &idx);
		imports[i].package_index = get_s32(&readbuf[idx], &idx);
		imports[i].object_name = get_fci(&readbuf[idx], &idx);
	}
}

/* load the type_names */
static void get_type(const char *buf, int e, int d)
{
	int i, idx, c;
	int32_t tmp = 0;

	idx = 0;

	for (i = 0; i < (int) strlen(export_desc[d].order); i++) {
		switch (export_desc[d].order[i]) {
		case UPKG_DATA_FCI:
			tmp = get_fci(&buf[idx], &idx);
			break;
		case UPKG_DATA_32:
			tmp = get_s32(&buf[idx], &idx);
			break;
		case UPKG_DATA_16:
			tmp = get_s16(&buf[idx], &idx);
			break;
		case UPKG_DATA_8:
			tmp = get_s8(&buf[idx], &idx);
			break;
		case UPKG_DATA_ASCIC:
			c = get_s8(&buf[idx], &idx);
			get_string(&buf[idx], c, &idx);
			break;
		case UPKG_DATA_ASCIZ:
			get_string(&buf[idx], UPKG_NAME_NOCOUNT, &idx);
			break;
		case UPKG_OBJ_JUNK:	/* do nothing */
			break;
		case UPKG_OBJ_NAME:
			exports[e].type_name = tmp;
			break;
		case UPKG_EXP_SIZE:	/* maybe we'll do something later on */
			break;
		case UPKG_OBJ_SIZE:
			exports[e].object_size = tmp;
			break;
		default:
			fprintf(stderr, "Unknown datatype/operation listed for export #%d\n", e);
			exports[e].type_name = -1;
			return;
		}
	}

	exports[e].object_offset = exports[e].serial_offset + idx;
}

static int get_types_isgood(int idx, int start)
{
	int i;

	for (i = start; export_desc[i].version; i++) {
		if (export_desc[i].version == hdr->file_version) {
			if (strncmp(export_desc[i].class_name,
				    names[exports[idx].class_name].name,
				    strlen(export_desc[i].class_name)
			    ) == 0) {
				return i;
			}
		}
	}

	return -1;
}

static void check_type(int e, int d)
{
	int i, s, l;
	char readbuf[101];

	fseek(file, exports[e].object_offset, SEEK_SET);
	fread((void *) readbuf, 100, 1, file);

	i = exports[e].type_name;
	if (i < 0 || i >= hdr->name_count) {
		exports[e].type_name = -1;
		return;
	}

	/* !! FIXME !! -- Harry Potter and the Chamber of Secrets */
	if (!strcmp(names[i].name, "XA") && hdr->file_version == 79) {
		if (strcmp(export_desc[d].order, "FjFn3j3j3j3j3j3j3sFd") != 0)
			exports[e].type_name = -1;
		return;
	}

	if (!strcmp(names[i].name, "mp2") &&
	    (hdr->file_version == 75 || hdr->file_version == 76)) {
		unsigned char *p = (unsigned char *)readbuf;
		uint16_t u = ((p[0] << 8) | p[1]) & 0xFFFE;
		if (u == 0xFFFC || u == 0xFFF4)
			return;
		exports[e].type_name = -1;
		return;
	}

	for (i = 0; object_desc[i].sig_offset != -1; i++) {
		s = object_desc[i].sig_offset;
		l = strlen(object_desc[i].object_sig);
		readbuf[100] = readbuf[s + l];
		readbuf[s + l] = 0;

		if (!strcmp(&readbuf[s], object_desc[i].object_sig)) {
			return;
		}

		readbuf[s + l] = readbuf[100];
	}

	exports[e].type_name = -1;
}

static void get_types(void)
{
	int i, j, next;
	char readbuf[UPKG_MAX_ORDERS * 4];

	for (i = 0, next = 0; i < hdr->export_count; next = 0, i++) {
		_retry:
		j = get_types_isgood(i, next);
		if (j != -1) {
			fseek(file, exports[i].serial_offset, SEEK_SET);
			fread((void *) readbuf, 4, UPKG_MAX_ORDERS, file);
			get_type(readbuf, i, j);
			check_type(i, j);

			if (exports[i].type_name == -1 &&
			    hdr->file_version >= 79) {
			/* Undying / Mobile Forces order difference?
			 * see if there is an alternative order for
			 * the same file version/class combination */
				next = j + 1;
				goto _retry;
			}
		} else {
			exports[i].type_name = -1;
		}
	}
}


/* PUBLIC API */

int upkg_open(const char *filename)
{
	if (pkg_opened)		/* is there a pkg opened already? */
		return -4;	/* if so, don't try to open another one */

	file = fopen(filename, "rb");

	if (file == NULL)
		return -1;

	if (fread((void *) header, 1, 64, file) < 64) {
		fclose(file);
		return -2;
	}

	if (load_upkg() != 0) {
		fclose(file);
		return -3;
	}

	pkg_opened = 1;

	get_names();		/* this order is important. */
	get_imports();
	get_exports();
	get_types();

	return 0;
}

void upkg_close(void)
{
	if (pkg_opened == 0)
		return;

	pkg_opened = 0;

	fclose(file);
	file = NULL;
	free(imports);
	free(exports);
	free(names);
	imports = NULL;
	exports = NULL;
	names = NULL;
	hdr = NULL;
}

int32_t upkg_ocount(void)
{
	if (pkg_opened == 0)
		return -1;

	return hdr->export_count;
}

const char *upkg_oname(int idx)
{
	idx = export_index(idx);
	if (idx == -1 || pkg_opened == 0)
		return NULL;

	return names[exports[idx].object_name].name;
}

const char *upkg_oclassname(int idx)
{
	idx = export_index(idx);
	if (idx == -1 || pkg_opened == 0)
		return NULL;

	return names[exports[idx].class_name].name;
}

const char *upkg_opackagename(int idx)
{
	idx = export_index(idx);
	if (idx == -1 || pkg_opened == 0)
		return NULL;

	return names[exports[idx].package_name].name;
}

const char *upkg_otype(int idx)
{
	idx = export_index(idx);
	if (idx == -1 || pkg_opened == 0)
		return NULL;

	if (exports[idx].type_name == -1)
		return NULL;

	return names[exports[idx].type_name].name;
}

int32_t upkg_export_size(int idx)
{
	idx = export_index(idx);
	if (idx == -1 || pkg_opened == 0)
		return 0;

	return exports[idx].serial_size;
}

int32_t upkg_object_size(int idx)
{
	idx = export_index(idx);
	if (idx == -1 || pkg_opened == 0)
		return 0;

	return exports[idx].object_size;
}

int32_t upkg_export_offset(int idx)
{
	idx = export_index(idx);
	if (idx == -1 || pkg_opened == 0)
		return 0;

	return exports[idx].serial_offset;
}

int32_t upkg_object_offset(int idx)
{
	idx = export_index(idx);
	if (idx == -1 || pkg_opened == 0)
		return 0;

	return exports[idx].object_offset;
}

int upkg_read(void *readbuf, size_t bytes, long offset)
{
	if (pkg_opened == 0)
		return -1;

	fseek(file, offset, SEEK_SET);

	return fread(readbuf, 1, bytes, file);
}

int upkg_export_dump(const char *filename, int idx)
{
	int count, diff;
	void *buffer;
	FILE *out;

	idx = export_index(idx);
	if (idx < 0 || pkg_opened == 0)
		return -1;

	buffer = calloc(1, 4096);
	if (buffer == NULL)
		return -1;

	out = fopen(filename, "wb");
	if (out == NULL) {
		free(buffer);
		return -1;
	}

	fseek(file, exports[idx].serial_offset, SEEK_SET);

	count = exports[idx].serial_size;

	do {
		diff =
		    fread(buffer, 1, ((count > 4096) ? 4096 : count),
			  file);
		if (diff == 0) {
			count = exports[idx].serial_size - count;
			fprintf(stderr, "bad read: wrote %d, expected %d bytes\n",
				count, exports[idx].object_size);
			break;
		}
		fwrite(buffer, 1, diff, out);
		count -= diff;
	} while (count > 0);

	fclose(out);
	free(buffer);

	return 0;
}

int upkg_object_dump(const char *filename, int idx)
{
	int count, diff;
	void *buffer;
	FILE *out;

	idx = export_index(idx);
	if (idx < 0 || pkg_opened == 0)
		return -1;

	buffer = calloc(1, 4096);
	if (buffer == NULL)
		return -1;

	out = fopen(filename, "wb");
	if (out == NULL) {
		free(buffer);
		return -1;
	}

	fseek(file, exports[idx].object_offset, SEEK_SET);

	count = exports[idx].object_size;

	do {
		diff =
		    fread(buffer, 1, ((count > 4096) ? 4096 : count),
			  file);
		if (diff == 0) {
			count = exports[idx].object_size - count;
			fprintf(stderr, "bad read: wrote %d, expected %d bytes\n",
				count, exports[idx].object_size);
			break;
		}
		fwrite(buffer, 1, diff, out);
		count -= diff;
	} while (count > 0);

	fclose(out);
	free(buffer);

	return 0;
}
