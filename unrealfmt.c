#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "umr.h"
#include "urf.h"
#include "unrealfmtdata.h"


static void print_flags(uint32_t flags)
{
    if (flags & RF_Transactional)
	printf("Transactional");

    if (flags & RF_SourceModified)
	printf(" SrcModified");

    if (flags & RF_Public)
	printf(" Public");

    if (flags & RF_LoadForClient)
	printf(" LFClient");

    if (flags & RF_LoadForServer)
	printf(" LFServer");

    if (flags & RF_LoadForEdit)
	printf(" LFEdit");

    if (flags & RF_Standalone)
	printf(" Standalone");

    if (flags & RF_HasStack)
	printf(" HasStack");

    if (flags & RF_Intrinsic)
	printf(" Intrinsic");
}

static void print_pkg_hdr(const struct upkg *pkg, int verbose)
{
    if (!verbose) return;

    printf("Package header:\n");

    printf("tag             = 0x%08x\n"
	   "file_version    = %d\n",
	   pkg->hdr->tag,
	   pkg->hdr->file_version);

    printf("pkg_flags       = ");
    print_flags(pkg->hdr->pkg_flags);
    printf("\n");

    printf("name_count      = %d\n"
	   "name_offset     = 0x%08x\n"
	   "export_count    = %d\n"
	   "export_offset   = 0x%08x\n"
	   "import_count    = %d\n"
	   "import_offset   = 0x%08x\n",
	   pkg->hdr->name_count,
	   pkg->hdr->name_offset,
	   pkg->hdr->export_count,
	   pkg->hdr->export_offset,
	   pkg->hdr->import_count,
	   pkg->hdr->import_offset);

    if (pkg->hdr->file_version < 68) {
	printf ("heritage_count  = %d\n"
		"heritage_offset = 0x%08x\n",
		pkg->hdr->heritage_count,
		pkg->hdr->heritage_offset);
    } else {
	printf ("guid            = 0x%08x %08x %08x %08x\n",
		pkg->hdr->guid[0], pkg->hdr->guid[1],
		pkg->hdr->guid[2], pkg->hdr->guid[3]);
	printf ("generation_count= %d\n",
		pkg->hdr->generation_count);
	if (pkg->hdr->generation_count > 0) {
	    int i;
	    for (i = 0; i < pkg->hdr->generation_count; i++) {
		printf ("generation #%d:\n"
			"   export_count = %d\n"
			"   name_count   = %d\n",
			i, pkg->hdr->gen[i].export_count,
			pkg->hdr->gen[i].name_count);
	    }
	}
    }
}

static void print_names(const struct upkg *pkg)
{
    int i;

    if (!pkg->verbose) return;
    printf("Name table:\n");
    for (i = 0; i < pkg->hdr->name_count; i++) {
	printf("%d: %s\t", i, pkg->names[i].name);
	print_flags(pkg->names[i].flags);
	printf("\n");
    }
    printf("\n");
}

static void indent(int level)
{
    int i;

    for (i = -1; i < level; i++)
	printf("  ");
}

static void print_export(struct upkg *pkg, int idx)
{
    int level, tmp;

    if (!pkg->verbose) return;

    level = pkg->indent_level++;

    indent(level - 1);
    printf("EXPORT #%d\n", idx);

    indent(level);
    printf("class_index   = %d\n",  pkg->exports[idx].class_index);

    indent(level);
    printf("package_index = %d\n",  pkg->exports[idx].package_index);

    indent(level);
    printf("super_index   = %d\n",  pkg->exports[idx].super_index);

    indent(level);
    tmp = pkg->exports[idx].object_name;
    if (tmp < 0 || tmp > pkg->hdr->name_count) {
	tmp = pkg->hdr->name_count;
    }
    printf("object_name   = %s\n", pkg->names[tmp].name);

    indent(level);
    printf("object_flags  = ");
    print_flags(pkg->exports[idx].object_flags);
    printf("\n");

    indent(level);
    printf("serial_size   = %d\n", pkg->exports[idx].serial_size);

    indent(level);
    printf("serial_offset = 0x%08x\n", pkg->exports[idx].serial_offset);

    indent(level);
    printf("class_name    = %s\n", pkg->names[pkg->exports[idx].class_name].name);

    indent(level);
    printf("package_name  = %s\n", pkg->names[pkg->exports[idx].package_name].name);

    indent(level);
    tmp = pkg->exports[idx].type_name;
    if (tmp < 0 || tmp > pkg->hdr->name_count) {
	tmp = pkg->hdr->name_count;
    }
    printf("type_name     = %s\n", pkg->names[tmp].name);

    indent(level);
    printf("object_size   = %d\n", pkg->exports[idx].object_size);

    indent(level);
    printf("object_offset = 0x%08x\n", pkg->exports[idx].object_offset);
}

static void print_import(struct upkg *pkg, int idx)
{
    int level;

    if (!pkg->verbose) return;

    level = pkg->indent_level++;

    indent(level - 1);
    printf("IMPORT #%d\n", idx);

    indent(level);
    printf("class_package = %s\n", pkg->names[pkg->imports[idx].class_package].name);

    indent(level);
    printf("class_name    = %s\n", pkg->names[pkg->imports[idx].class_name].name);

    indent(level);
    printf("package_index = %d\n", pkg->imports[idx].package_index);

    indent(level);
    printf("object_name   = %s\n", pkg->names[pkg->imports[idx].object_name].name);
}

/* decode an FCompactIndex.
 * original documentation by Tim Sweeney was at
 * http://unreal.epicgames.com/Packages.htm
 * also see Unreal Wiki:
 * http://wiki.beyondunreal.com/Legacy:Package_File_Format/Data_Details
 */
static fci_t get_fci(const char *in, int *pos)
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

static uint32_t get_u32(const void *addr, int *pos)
{
	const unsigned char *p = (const unsigned char *)addr;
	*pos += 4;
	return (uint32_t)READ_INT32(p);
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

static void get_string(const char *addr, int count, int *pos, char *buf)
{
	if (count > UPKG_MAX_NAME_SIZE || count == UPKG_NAME_NOCOUNT)
		count = UPKG_MAX_NAME_SIZE;

	strncpy(buf, addr, count);	/* the string stops at count chars, or is ASCIIZ */
	buf[UPKG_MAX_NAME_SIZE - 1] = '\0';
	*pos += strlen(buf) + 1;
}

static int export_index(struct upkg *pkg, int i)
{
	if (i > 0) {
		int j = i - 1;
		if (j < pkg->hdr->export_count)
			return j;
		/* else: shouldn't happen.. */
	}

	return -1;
}

static int import_index(struct upkg *pkg, int i)
{
	if (i < 0) {
		int j = -i - 1;
		if (j < pkg->hdr->import_count)
			return j;
		/* else: shouldn't happen.. */
	}

	return -1;
}

/* idx == exports[idx], c_idx == index to the next element from idx */
static int set_classname(struct upkg *pkg, int idx, int c_idx) {
    int i, next;

    i = c_idx;

    do {
	if (i < 0) {
	    i = import_index(pkg, i);
	    if (i < 0) break;/* shouldn't happen.. */
	    print_import(pkg, i);
	    if (!strcmp(pkg->names[pkg->imports[i].class_name].name, "Class")) {
		pkg->exports[idx].class_name = pkg->imports[i].object_name;
		return pkg->imports[i].package_index;
	    }

	    next = pkg->imports[i].package_index;
	    if (i == -next - 1) break;/* shouldn't happen.. */
	}

	if (i > 0) {
	    i = export_index(pkg, i);
	    if (i < 0) break;/* shouldn't happen.. */
	    print_export(pkg, i);
	    next = pkg->exports[i].class_index;
	    if (i ==  next - 1) break;/* shouldn't happen.. */
	} else {
	    break;
	}

	i = next;
    } while (i >= -pkg->hdr->import_count && i < pkg->hdr->export_count);

    pkg->exports[idx].class_name = pkg->hdr->name_count;
    return c_idx;
}

static int set_pkgname(struct upkg *pkg, int idx, int c_idx) {
    int i, next;

    i = c_idx;

    do {
	if (i < 0) {
	    i = import_index(pkg, i);
	    if (i < 0) break;/* shouldn't happen.. */
	    print_import(pkg, i);
	    if (!strcmp(pkg->names[pkg->imports[i].class_name].name, "Package")) {
		pkg->exports[idx].package_name = pkg->imports[i].object_name;
		return pkg->imports[i].package_index;
	    }

	    next = pkg->imports[i].package_index;
	    if (i == -next - 1) break;/* shouldn't happen.. */
	}

	if (i > 0) {
	    i = export_index(pkg, i);
	    if (i < 0) break;/* shouldn't happen.. */
	    print_export(pkg, i);

	    next = pkg->exports[i].class_index;
	    if (i ==  next - 1) break;/* shouldn't happen.. */
	} else {
	    break;
	}

	i = next;
    } while (i >= -pkg->hdr->import_count && i < pkg->hdr->export_count);

    pkg->exports[idx].package_name = pkg->hdr->name_count;
    return c_idx;
}

/* load in the header, AWA allocating the needed memory for the tables */
static int load_upkg(struct upkg *pkg)
{
	unsigned char *p;
	uint32_t *swp;
	int i;

	/* byte swap the header (all members are 32 bit LE values) */
	p = (unsigned char *) pkg->hdr;
	swp = (uint32_t *) pkg->hdr;
	for (i = 0; i < UPKG_HDR_SIZE/4; i++, p += 4) {
		swp[i] = READ_INT32(p);
	}

	if (pkg->hdr->tag != UPKG_HDR_TAG)
		return -1;
	if (pkg->hdr->name_count	< 0  ||
	    pkg->hdr->export_count	< 0  ||
	    pkg->hdr->import_count	< 0  ||
	    pkg->hdr->name_offset	< 36 ||
	    pkg->hdr->export_offset	< 36 ||
	    pkg->hdr->import_offset	< 36) {
		printf("Illegal values in header:\n");
		print_pkg_hdr(pkg, 1);
		return -1;
	}

	if (pkg->hdr->file_version >= 68) {
		/* move data to correct members */
		memmove(&pkg->hdr->guid[0], &pkg->hdr->heritage_count, 20);
		/* read the generation history */
		if (pkg->hdr->generation_count > 0) {
			pkg->hdr->gen = (struct _genhist *)
			    calloc(pkg->hdr->generation_count, sizeof(struct _genhist));
			if (pkg->hdr->gen == NULL)
				return -1;
			fseek(pkg->file, UPKG_HDR_SIZE - 8, SEEK_SET);
			p = (unsigned char *) &pkg->hdr->heritage_count;
			for (i = 0; i < pkg->hdr->generation_count; i++) {
				fread(p, 4, 1, pkg->file);
				pkg->hdr->gen[i].export_count = READ_INT32(p);
				fread(p, 4, 1, pkg->file);
				pkg->hdr->gen[i].name_count = READ_INT32(p);
			}
		}
		pkg->hdr->heritage_count = 0;
		pkg->hdr->heritage_offset = 0;
	}

	print_pkg_hdr(pkg, pkg->verbose);

	for (i = 0; export_desc[i].version; i++) {
		if (pkg->hdr->file_version == export_desc[i].version) {
			break;
		}
	}

	if (export_desc[i].version == 0)
		return -1;

	pkg->names =
	    (struct upkg_name *) calloc((pkg->hdr->name_count + 1), sizeof(struct upkg_name));
	if (pkg->names == NULL)
		return -1;

	pkg->exports =
	    (struct upkg_export *) calloc(pkg->hdr->export_count, sizeof(struct upkg_export));
	if (pkg->exports == NULL)
		return -1;

	pkg->imports =
	    (struct upkg_import *) calloc(pkg->hdr->import_count, sizeof(struct upkg_import));
	if (pkg->imports == NULL)
		return -1;

	return 0;
}

/* load the name table */
static void get_names(struct upkg *pkg)
{
	int i, idx, c;
	long nofs;
	char readbuf[80];

	nofs = pkg->hdr->name_offset;
	idx = 0;

	for (i = 0; i < pkg->hdr->name_count; i++) {
		nofs += idx;
		idx = 0;
		memset(readbuf, 0, 80);
		fseek(pkg->file, nofs, SEEK_SET);
		fread(readbuf, 1, 80, pkg->file);

		if (pkg->hdr->file_version >= 64) {
			c = get_s8(&readbuf[idx], &idx);
			get_string(&readbuf[idx], c, &idx, pkg->names[i].name);
		} else {
			get_string(&readbuf[idx], UPKG_NAME_NOCOUNT, &idx, pkg->names[i].name);
		}

		pkg->names[i].flags = get_u32(&readbuf[idx], &idx);
	}

/* hdr->name_count + 1 names total, this one's last */
	strncpy(pkg->names[i].name, "(NULL)", UPKG_MAX_NAME_SIZE);
	pkg->names[i].flags = 0;

	print_names(pkg);
}

/* load the export table (which is at the end of the file) */
static void get_exports_cpnames(struct upkg *pkg, int idx) {
    int x;

    if (idx < 0 || idx >= pkg->hdr->export_count)
	return;
    if (pkg->verbose) printf("%d\n", idx);

    pkg->indent_level = 0;

    print_export(pkg, idx);

    x = pkg->exports[idx].class_index;

    x = set_classname(pkg, idx, x);

    set_pkgname(pkg, idx, x);
}

static void get_exports(struct upkg *pkg)
{
	int i, idx;
	long eofs;
	char readbuf[40];

	eofs = pkg->hdr->export_offset;
	idx = 0;

	for (i = 0; i < pkg->hdr->export_count; i++) {
		eofs += idx;
		idx = 0;
		memset(readbuf, 0, 40);
		fseek(pkg->file, eofs, SEEK_SET);
		fread(readbuf, 1, 40, pkg->file);

		pkg->exports[i].class_index = get_fci(&readbuf[idx], &idx);
		pkg->exports[i].super_index = get_fci(&readbuf[idx], &idx);
		if (pkg->hdr->file_version >= 60)
			pkg->exports[i].package_index = get_s32(&readbuf[idx], &idx);
		else	pkg->exports[i].package_index = 0;
		pkg->exports[i].object_name = get_fci(&readbuf[idx], &idx);
		pkg->exports[i].object_flags = get_u32(&readbuf[idx], &idx);
		pkg->exports[i].serial_size = get_fci(&readbuf[idx], &idx);

		if (pkg->exports[i].serial_size > 0) {
			pkg->exports[i].serial_offset =
			    get_fci(&readbuf[idx], &idx);
		} else {
			pkg->exports[i].serial_offset = -1;
		}

		get_exports_cpnames(pkg, i); /* go grab the class & package names */
	}
}

/* load the import table.  same story as get_exports() */
static void get_imports(struct upkg *pkg)
{
	int i, idx;
	long iofs;
	char readbuf[40];

	iofs = pkg->hdr->import_offset;
	idx = 0;

	for (i = 0; i < pkg->hdr->import_count; i++) {
		iofs += idx;
		idx = 0;
		memset(readbuf, 0, 40);
		fseek(pkg->file, iofs, SEEK_SET);
		fread(readbuf, 1, 40, pkg->file);

		pkg->imports[i].class_package = get_fci(&readbuf[idx], &idx);
		pkg->imports[i].class_name = get_fci(&readbuf[idx], &idx);
		if (pkg->hdr->file_version >= 60)
			pkg->imports[i].package_index = get_s32(&readbuf[idx], &idx);
		else {
			pkg->imports[i].package_index = 0;
			get_fci(&readbuf[idx], &idx);/* ?? */
		}
		pkg->imports[i].object_name = get_fci(&readbuf[idx], &idx);
	}
}

/* load the type_names */
static void get_type(struct upkg *pkg, const char *buf, int e, int d)
{
	int i, idx, c;
	int32_t tmp = 0;
	char str[UPKG_MAX_NAME_SIZE];

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
			get_string(&buf[idx], c, &idx, str);
			break;
		case UPKG_DATA_ASCIZ:
			get_string(&buf[idx], UPKG_NAME_NOCOUNT, &idx, str);
			break;
		case UPKG_OBJ_JUNK:	/* do nothing */
			break;
		case UPKG_OBJ_NAME:
			pkg->exports[e].type_name = tmp;
			break;
		case UPKG_EXP_SIZE:	/* maybe we'll do something later on */
			break;
		case UPKG_OBJ_SIZE:
			pkg->exports[e].object_size = tmp;
			break;
		default:
			fprintf(stderr, "Unknown datatype/operation listed for export #%d\n", e);
			pkg->exports[e].type_name = -1;
			return;
		}
	}

	pkg->exports[e].object_offset = pkg->exports[e].serial_offset + idx;
}

static int get_types_isgood(struct upkg *pkg, int idx, int start)
{
	int i;

	for (i = start; export_desc[i].version; i++) {
		if (export_desc[i].version == pkg->hdr->file_version) {
			if (strncmp(export_desc[i].class_name,
				    pkg->names[pkg->exports[idx].class_name].name,
				    strlen(export_desc[i].class_name)
			    ) == 0) {
				return i;
			}
		}
	}

	return -1;
}

static void check_type(struct upkg *pkg, int e, int d)
{
	long i, s, l;
	char readbuf[96], c;

	fseek(pkg->file, pkg->exports[e].object_offset, SEEK_SET);
	fread(readbuf, 96, 1, pkg->file);

	i = pkg->exports[e].type_name;
	if (i < 0 || i >= pkg->hdr->name_count) {
		pkg->exports[e].type_name = -1;
		return;
	}

	/* !! FIXME !! -- Harry Potter and the Chamber of Secrets */
	if (!strcmp(pkg->names[i].name, "XA") && pkg->hdr->file_version == 79) {
		if (strcmp(export_desc[d].order, "FjFn3j3j3j3j3j3j3sFd") != 0)
			pkg->exports[e].type_name = -1;
		return;
	}

	if (!strcmp(pkg->names[i].name, "mp2") &&
	    (pkg->hdr->file_version == 75 || pkg->hdr->file_version == 76)) {
		unsigned char *p = (unsigned char *)readbuf;
		uint16_t u = ((p[0] << 8) | p[1]) & 0xFFFE;
		if (u == 0xFFFC || u == 0xFFF4)
			return;
		pkg->exports[e].type_name = -1;
		return;
	}

	for (i = 0; object_desc[i].sig_offset != -1; i++) {
		s = object_desc[i].sig_offset;
		l = strlen(object_desc[i].sig);
		c = readbuf[s + l];
		readbuf[s + l] = 0;

		if (!strcmp(&readbuf[s], object_desc[i].sig))
			return;

		readbuf[s + l] = c;
	}

	pkg->exports[e].type_name = -1;
}

static void get_types(struct upkg *pkg)
{
	int i, j, next;
	char readbuf[UPKG_MAX_ORDERS * 4];

	for (i = 0, next = 0; i < pkg->hdr->export_count; next = 0, i++) {
		_retry:
		j = get_types_isgood(pkg, i, next);
		if (j != -1) {
			fseek(pkg->file, pkg->exports[i].serial_offset, SEEK_SET);
			fread(readbuf, 4, UPKG_MAX_ORDERS, pkg->file);
			get_type(pkg, readbuf, i, j);
			check_type(pkg, i, j);

			if (pkg->exports[i].type_name == -1 &&
			    pkg->hdr->file_version >= 79) {
			/* Undying / Mobile Forces order difference?
			 * see if there is an alternative order for
			 * the same file version/class combination */
				next = j + 1;
				goto _retry;
			}
		} else {
			pkg->exports[i].type_name = -1;
		}
	}
	if (pkg->verbose) printf("\n");
}


/* PUBLIC API */

struct upkg *upkg_open(const char *filename, int verbose)
{
	struct upkg *pkg;
	FILE *file;

	file = fopen(filename, "rb");
	if (file == NULL)
		return NULL;

	pkg = (struct upkg *) calloc(1, sizeof(struct upkg));
	if (!pkg) {
		fclose(file);
		return NULL;
	}

	pkg->file = file;
	pkg->hdr = (struct upkg_hdr *) calloc(1, sizeof(struct upkg_hdr));
	if (!pkg->hdr) goto err;

	if (fread(pkg->hdr, 1, UPKG_HDR_SIZE, file) < UPKG_HDR_SIZE)
		goto err;

	pkg->verbose = verbose;
	if (load_upkg(pkg) != 0)
		goto err;

	get_names(pkg);		/* this order is important. */
	get_imports(pkg);
	get_exports(pkg);
	get_types(pkg);

	return pkg;

err:	upkg_close(pkg);
	return NULL;
}

void upkg_close(struct upkg *pkg)
{
	if (!pkg) return;

	if (pkg->file) fclose(pkg->file);
	if (pkg->hdr->gen) free(pkg->hdr->gen);
	if (pkg->hdr) free(pkg->hdr);
	if (pkg->imports) free(pkg->imports);
	if (pkg->exports) free(pkg->exports);
	if (pkg->names) free(pkg->names);
	free(pkg);
}

int32_t upkg_ocount(struct upkg *pkg)
{
	if (!pkg) return -1;
	return pkg->hdr->export_count;
}

const char *upkg_oname(struct upkg *pkg, int idx)
{
	if (!pkg) return NULL;
	idx = export_index(pkg, idx);
	if (idx == -1) return NULL;
	return pkg->names[pkg->exports[idx].object_name].name;
}

const char *upkg_oclassname(struct upkg *pkg, int idx)
{
	if (!pkg) return NULL;
	idx = export_index(pkg, idx);
	if (idx == -1) return NULL;
	return pkg->names[pkg->exports[idx].class_name].name;
}

const char *upkg_opackagename(struct upkg *pkg, int idx)
{
	if (!pkg) return NULL;
	idx = export_index(pkg, idx);
	if (idx == -1) return NULL;
	return pkg->names[pkg->exports[idx].package_name].name;
}

const char *upkg_otype(struct upkg *pkg, int idx)
{
	if (!pkg) return NULL;
	idx = export_index(pkg, idx);
	if (idx == -1) return NULL;
	if (pkg->exports[idx].type_name == -1) return NULL;
	return pkg->names[pkg->exports[idx].type_name].name;
}

int32_t upkg_export_size(struct upkg *pkg, int idx)
{
	if (!pkg) return 0;
	idx = export_index(pkg, idx);
	if (idx == -1) return 0;
	return pkg->exports[idx].serial_size;
}

int32_t upkg_object_size(struct upkg *pkg, int idx)
{
	if (!pkg) return 0;
	idx = export_index(pkg, idx);
	if (idx == -1) return 0;
	return pkg->exports[idx].object_size;
}

int32_t upkg_export_offset(struct upkg *pkg, int idx)
{
	if (!pkg) return 0;
	idx = export_index(pkg, idx);
	if (idx == -1) return 0;
	return pkg->exports[idx].serial_offset;
}

int32_t upkg_object_offset(struct upkg *pkg, int idx)
{
	if (!pkg) return 0;
	idx = export_index(pkg, idx);
	if (idx == -1) return 0;
	return pkg->exports[idx].object_offset;
}

int upkg_read(struct upkg *pkg, void *readbuf, size_t bytes, long offset)
{
	if (!pkg || !readbuf) return -1;
	fseek(pkg->file, offset, SEEK_SET);
	return fread(readbuf, 1, bytes, pkg->file);
}

int upkg_export_dump(struct upkg *pkg, const char *filename, int idx)
{
	int cnt, diff;
	char buf[4096];
	FILE *out;

	if (!pkg) return -1;
	idx = export_index(pkg, idx);
	if (idx < 0) return -1;
	cnt = pkg->exports[idx].serial_size;
	if (cnt < 0) return -1;
	out = fopen(filename, "wb");
	if (!out) return -1;

	fseek(pkg->file, pkg->exports[idx].serial_offset, SEEK_SET);

	do {
		diff = fread(buf, 1, ((cnt > 4096)? 4096 : cnt), pkg->file);
		if (diff == 0) break;
		(void) fwrite(buf, 1, diff, out);
		cnt -= diff;
	} while (cnt > 0);

	fclose(out);
	if (cnt != 0) {
		cnt = pkg->exports[idx].serial_size - cnt;
		fprintf(stderr, "bad read: wrote %d, expected %d bytes\n",
			cnt, pkg->exports[idx].serial_size);
		return -1;
	}

	return 0;
}

int upkg_object_dump(struct upkg *pkg, const char *filename, int idx)
{
	int cnt, diff;
	char buf[4096];
	FILE *out;

	if (!pkg) return -1;
	idx = export_index(pkg, idx);
	if (idx < 0) return -1;
	cnt = pkg->exports[idx].object_size;
	if (cnt < 0) return -1;
	out = fopen(filename, "wb");
	if (!out) return -1;

	fseek(pkg->file, pkg->exports[idx].object_offset, SEEK_SET);

	do {
		diff = fread(buf, 1, ((cnt > 4096)? 4096 : cnt), pkg->file);
		if (diff == 0) break;
		(void) fwrite(buf, 1, diff, out);
		cnt -= diff;
	} while (cnt > 0);

	fclose(out);
	if (cnt != 0) {
		cnt = pkg->exports[idx].object_size - cnt;
		fprintf(stderr, "bad read: wrote %d, expected %d bytes\n",
			cnt, pkg->exports[idx].object_size);
		return -1;
	}

	return 0;
}
