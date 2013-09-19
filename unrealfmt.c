#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "umr.h"
#include "urf.h"



upkg_hdr *hdr;			// read the urf.h for these 4...
upkg_exports *exports;
upkg_imports *imports;
upkg_names *names;

FILE *file;			// we store the file pointer globally around here

int data_size,			// a way to standardize some freaky parts of the format
 pkg_opened = 0;		// sanity check
int indent_level;

char header[4096],		// we load the header into this buffer
 buf[256];			// temp buf for get_string()


char *print_flags(signed int flags) {
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

void print_pkg_hdr(void) {
    printf("tag             = 0x%08lx\n"
	   "file_version    = %ld\n"
	   "pkg_flags       = %s\n"
	   "name_count      = %ld\n"
	   "name_offset     = 0x%08lx\n"
	   "export_count    = %ld\n"
	   "export_offset   = 0x%08lx\n"
	   "import_count    = %ld\n"
	   "import_offset   = 0x%08lx\n"
	   "heritage_count  = %ld\n"
	   "heritage_offset = 0x%08lx\n",
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

void print_name(int i) {
	printf("%d: %s\t%s", i, names[i].name, print_flags(names[i].flags));
}

void indent(int level) {
    int i;
    
    for (i = -1; i < level; i++)
	printf("  ");
}

void print_export(int idx) {
    int level, tmp;
    
    level = indent_level++;

    indent(level - 1);
    printf("EXPORT #%d\n", idx);

    indent(level);
    printf("class_index   = %ld\n", exports[idx].class_index);
    
    indent(level);
    printf("package_index = %ld\n", exports[idx].package_index);

    indent(level);
    printf("super_index   = %ld\n", exports[idx].super_index);

    indent(level);
    tmp = exports[idx].object_name;
    if (tmp < 0 || tmp > hdr->name_count) {
	tmp = hdr->name_count;
    }
    printf("object_name   = %s\n", names[tmp].name);

    indent(level);
    printf("object_flags  = %s\n", print_flags(exports[idx].object_flags));

    indent(level);
    printf("serial_size   = %ld\n", exports[idx].serial_size);

    indent(level);
    printf("serial_offset = 0x%08lx\n", exports[idx].serial_offset);

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
    printf("object_size   = %ld\n", exports[idx].object_size);

    indent(level);
    printf("object_offset = 0x%08lx\n", exports[idx].object_offset);
}

void print_import(int idx) {
    int level;
    
    level = indent_level++;

    indent(level - 1);
    printf("IMPORT #%d\n", idx);

    indent(level);
    printf("class_package = %s\n", names[imports[idx].class_package].name);
    
    indent(level);
    printf("class_name    = %s\n", names[imports[idx].class_name].name);
    
    indent(level);
    printf("package_index = %ld\n", imports[idx].package_index);
    
    indent(level);
    printf("object_name   = %s\n", names[imports[idx].object_name].name);
}

// this function decodes the encoded indices in the upkg files
signed long get_fci(char *in)
{
	signed long a;
	int size;

	a = 0;
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

	data_size = size;

	return a;
}

signed long get_s32(void *addr)
{
	data_size = sizeof(signed long);
	return *(signed long *) addr;
}

signed long get_s16(void *addr)
{
	data_size = sizeof(signed short);
	return *(signed short *) addr;
}

signed long get_s8(void *addr)
{
	data_size = sizeof(signed char);
	return *(signed char *) addr;
}

char *get_string(char *addr, int count)
{
	if (count > UPKG_MAX_NAME_SIZE || count == UPKG_NAME_NOCOUNT)
		count = UPKG_MAX_NAME_SIZE;

	strncpy(buf, addr, count);	// the string stops at count chars, or is ASCIIZ

	data_size = strlen(buf) + 1;

	return buf;
}

signed int export_index(signed int i)
{
	if (i > 0) {
		return i - 1;
	}

	return -1;
}

signed int import_index(signed int i)
{
	if (i < 0) {
		return -i - 1;
	}

	return -1;
}

// idx == exports[idx], c_idx == index to the next element from idx
int set_classname(int idx, int c_idx) {
    int i, next;
    
    i = c_idx;
    
    do {
	if (i < 0) {
	    i = import_index(i);
	    print_import(i);
	    if (!strcmp(names[imports[i].class_name].name, "Class")) {
		exports[idx].class_name = imports[i].object_name;
		return imports[i].package_index;
	    }
	    
	    next = imports[i].package_index;
	}
	
	if (i > 0) {
	    i = export_index(i);
	    print_export(i);
	    next = exports[i].class_index;
	} else {
	    break;
	}
	
	i = next;
    } while (i >= -hdr->import_count && i < hdr->export_count);
    
    exports[idx].class_name = hdr->name_count;
    return c_idx;
}

int set_pkgname(int idx, int c_idx) {
    int i, next;
    
    i = c_idx;
    
    do {
	if (i < 0) {
	    i = import_index(i);
	    print_import(i);
	    if (!strcmp(names[imports[i].class_name].name, "Package")) {
		exports[idx].package_name = imports[i].object_name;
		return imports[i].package_index;
	    }
	    
	    next = imports[i].package_index;
	}
	
	if (i > 0) {
	    i = export_index(i);
	    print_export(i);

	    next = exports[i].class_index;
	} else {
	    break;
	}
	
	i = next;
    } while (i >= -hdr->import_count && i < hdr->export_count);
    
    exports[idx].package_name = hdr->name_count;
    return c_idx;
}

// load in the header, AWA allocating the needed memory for the tables
int load_upkg(void)
{
	int index, i;

	index = 0;

	hdr = (upkg_hdr *) header;

	if (hdr->tag != UPKG_HDR_TAG)
		return -1;

	for (i = 0; export_desc[i].version; i++) {
		if (hdr->file_version == export_desc[i].version) {
			break;
		}
	}

	if (export_desc[i].version == 0)
		return -1;

	names =
	    (upkg_names *) malloc(sizeof(upkg_names) * (hdr->name_count + 1));
	if (names == NULL)
		return -1;

	exports =
	    (upkg_exports *) malloc(sizeof(upkg_exports) *
				    hdr->export_count);
	if (exports == NULL) {
		free(names);
		return -1;
	}

	imports =
	    (upkg_imports *) malloc(sizeof(upkg_imports) *
				    hdr->import_count);
	if (imports == NULL) {
		free(exports);
		free(names);
		return -1;
	}
	
	print_pkg_hdr();

	return 0;
}

// load the name table
void get_names(void)
{
	int i, index;

	index = hdr->name_offset;

	for (i = 0; i < hdr->name_count; i++) {
		if (hdr->file_version >= 64) {
			get_string(&header[index + 1],
				   get_s8(&header[index]));
			index++;
		} else {
			get_string(&header[index], UPKG_NAME_NOCOUNT);
		}
		index += data_size;

		strncpy(names[i].name, buf, UPKG_MAX_NAME_SIZE);

		names[i].flags = get_s32(&header[index]);
		index += data_size;
		
		print_name(i);
		printf("\n");
	}
	
// hdr->name_count + 1 names total, this one's last
	strncpy(names[i].name, "(NULL)", UPKG_MAX_NAME_SIZE);
	names[i].flags = 0;
	
	print_name(i);
	printf("\n");
}

// load the export table (which is at the end of the file... go figure)
void get_exports_cpnames(int idx) {
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

void get_exports(void)
{
	int i, index;
	char readbuf[1024];

	fseek(file, hdr->export_offset, SEEK_SET);

	fread((void *) readbuf, 1, 1024, file);

	index = 0;

	for (i = 0; i < hdr->export_count; i++) {
		exports[i].class_index = get_fci(&readbuf[index]);
		index += data_size;

		exports[i].package_index = get_s32(&readbuf[index]);
		index += data_size;

		exports[i].super_index = get_fci(&readbuf[index]);
		index += data_size;

		exports[i].object_name = get_fci(&readbuf[index]);
		index += data_size;

		exports[i].object_flags = get_s32(&readbuf[index]);
		index += data_size;

		exports[i].serial_size = get_fci(&readbuf[index]);
		index += data_size;

		if (exports[i].serial_size > 0) {
			exports[i].serial_offset =
			    get_fci(&readbuf[index]);
			index += data_size;
		} else {
			exports[i].serial_offset = -1;
		}

		get_exports_cpnames(i);	// go grab the class & package names
	}
}

// load the import table (notice a trend?).  same story as get_exports()
void get_imports(void)
{
	int i, index;
	char readbuf[1024];

	fseek(file, hdr->import_offset, SEEK_SET);

	fread((void *) readbuf, 1, 1024, file);

	index = 0;

	for (i = 0; i < hdr->import_count; i++) {
		imports[i].class_package = get_fci(&readbuf[index]);
		index += data_size;

		imports[i].class_name = get_fci(&readbuf[index]);
		index += data_size;

		imports[i].package_index = get_s32(&readbuf[index]);
		index += data_size;

		imports[i].object_name = get_fci(&readbuf[index]);
		index += data_size;
	}
}	

// load the type_names
void get_type(char *buf, int e, int d)
{
	int i, index;
	signed long tmp;
	char *chtmp;

	index = 0;

	for (i = 0; i < strlen(export_desc[d].order); i++) {
		switch (export_desc[d].order[i]) {
		case UPKG_DATA_FCI:
			tmp = get_fci(&buf[index]);
			index += data_size;
			break;
		case UPKG_DATA_32:
			tmp = get_s32(&buf[index]);
			index += data_size;
			break;
		case UPKG_DATA_16:
			tmp = get_s16(&buf[index]);
			index += data_size;
			break;
		case UPKG_DATA_8:
			tmp = get_s8(&buf[index]);
			index += data_size;
			break;
		case UPKG_DATA_ASCIC:
			chtmp =
			    get_string(&buf[index + 1],
				       get_s8(&buf[index++]));
			index += data_size;
			break;
		case UPKG_DATA_ASCIZ:
			chtmp = get_string(&buf[index], UPKG_NAME_NOCOUNT);
			index += data_size;
			break;
		case UPKG_OBJ_JUNK:	// do nothing!!!
			break;
		case UPKG_OBJ_NAME:
			exports[e].type_name = tmp;
			break;
		case UPKG_EXP_SIZE:	// maybe we'll do something later on
			break;
		case UPKG_OBJ_SIZE:
			exports[e].object_size = tmp;
			break;
		default:
			printf
			    ("Unknown datatype/operation listed for export #%d!\n",
			     e);
			exports[e].type_name = -1;
			return;
		}
	}

	exports[e].object_offset = exports[e].serial_offset + index;
}

int get_types_isgood(int idx)
{
	int i;

	for (i = 0; export_desc[i].version; i++) {
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

void check_type(int e, int d)
{
	int i;
	char readbuf[101], s, l;

	fseek(file, exports[e].object_offset, SEEK_SET);

	fread((void *) readbuf, 100, 1, file);

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


void get_types(void)
{
	int i, j;
	char readbuf[UPKG_MAX_ORDERS * 4];

	for (i = 0; i < hdr->export_count; i++) {
		if ((j = get_types_isgood(i)) != -1) {
			fseek(file, exports[i].serial_offset, SEEK_SET);

			fread((void *) readbuf, 4, UPKG_MAX_ORDERS, file);

			get_type(readbuf, i, j);

			check_type(i, j);
		} else {
			exports[i].type_name = -1;
		}
	}
}


//**************  GLOBALS


// open that puppy!!!  gets the file opened and the data structs read for use
int upkg_open(char *filename)
{
	if (pkg_opened)		// is there a pkg opened already?
		return -4;	// if so, don't try to open another one!

	file = fopen(filename, "rb");

	if (file == NULL)
		return -1;

	if (fread((void *) header, 1, 4096, file) < 4096) {
		fclose(file);
		return -2;
	}

	if (load_upkg() != 0) {
		fclose(file);
		return -3;
	}

	pkg_opened = 1;

	get_names();		// this order is important.
	get_imports();
	get_exports();
	get_types();

	return 0;
}

// close everything out
void upkg_close(void)
{
	if (pkg_opened == 0)
		return;

	free(imports);
	free(exports);
	free(names);
	hdr = (upkg_hdr *) 0;

	fclose(file);

	pkg_opened = 0;
}

// API stuff...  should be self-explainatory (upkg_o* == unreal package object *)
signed int upkg_ocount(void)
{
	if (pkg_opened == 0)
		return -1;

	return hdr->export_count;
}


char *upkg_oname(signed int idx)
{
	idx = export_index(idx);
	if (idx == -1 || pkg_opened == 0)
		return NULL;

	return names[exports[idx].object_name].name;
}

char *upkg_oclassname(signed int idx)
{
	idx = export_index(idx);
	if (idx == -1 || pkg_opened == 0)
		return NULL;

	return names[exports[idx].class_name].name;
}

char *upkg_opackagename(signed int idx)
{
	idx = export_index(idx);
	if (idx == -1 || pkg_opened == 0)
		return NULL;

	return names[exports[idx].package_name].name;
}

char *upkg_otype(signed int idx)
{
	idx = export_index(idx);
	if (idx == -1 || pkg_opened == 0)
		return NULL;

	if (exports[idx].type_name == -1)
		return NULL;

	return names[exports[idx].type_name].name;
}

signed int upkg_export_size(signed int idx)
{
	idx = export_index(idx);
	if (idx == -1 || pkg_opened == 0)
		return 0;

	return exports[idx].serial_size;
}

signed int upkg_object_size(signed int idx)
{
	idx = export_index(idx);
	if (idx == -1 || pkg_opened == 0)
		return 0;

	return exports[idx].object_size;
}

signed int upkg_export_offset(signed int idx)
{
	idx = export_index(idx);
	if (idx == -1 || pkg_opened == 0)
		return 0;

	return exports[idx].serial_offset;
}

signed int upkg_object_offset(signed int idx)
{
	idx = export_index(idx);
	if (idx == -1 || pkg_opened == 0)
		return 0;

	return exports[idx].object_offset;
}


int upkg_read(void *readbuf, signed int bytes, signed int offset)
{
	if (pkg_opened == 0)
		return -1;

	fseek(file, offset, SEEK_SET);

	return fread(readbuf, 1, bytes, file);
}

int upkg_export_dump(char *filename, signed int idx)
{
	int count, diff;
	void *buffer;
	FILE *out;

	idx = export_index(idx);
	if (idx == -1 || pkg_opened == 0)
		return -1;

	buffer = malloc(4096);
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
		fwrite(buffer, 1, diff, out);
		count -= diff;
	} while (count > 0);

	free(buffer);

	return 0;
}

int upkg_object_dump(char *filename, signed int idx)
{
	int count, diff;
	void *buffer;
	FILE *out;

	idx = export_index(idx);
	if (idx == -1 || pkg_opened == 0)
		return -1;

	buffer = malloc(4096);
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
		fwrite(buffer, 1, diff, out);
		count -= diff;
	} while (count > 0);

	free(buffer);

	return 0;
}
