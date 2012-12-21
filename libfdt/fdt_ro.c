/*
 * libfdt - Flat Device Tree manipulation
 * Copyright (C) 2006 David Gibson, IBM Corporation.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */
#include "config.h"
#if CONFIG_OF_LIBFDT

#include "libfdt_env.h"

#include <fdt.h>
#include <libfdt.h>

#include "libfdt_internal.h"

#define CHECK_HEADER(fdt)	{ \
	int err; \
	if ((err = fdt_check_header(fdt)) != 0) \
		return err; \
}

static int offset_streq(const void *fdt, int offset,
			const char *s, int len)
{
	const char *p = fdt_offset_ptr(fdt, offset, len+1);

	if (! p)
		/* short match */
		return 0;

	if (memcmp(p, s, len) != 0)
		return 0;

	if (p[len] != '\0')
		return 0;

	return 1;
}

/*
 * Checks if the property name matches.
 */
static int prop_name_eq(const void *fdt, int offset, const char *name,
			struct fdt_property **prop, int *lenp)
{
	int namestroff, len;

	*prop = fdt_offset_ptr_typed(fdt, offset, *prop);
	if (! *prop)
		return -FDT_ERR_BADSTRUCTURE;

	namestroff = fdt32_to_cpu((*prop)->nameoff);
	if (streq(fdt_string(fdt, namestroff), name)) {
		len = fdt32_to_cpu((*prop)->len);
		*prop = fdt_offset_ptr(fdt, offset,
				       sizeof(**prop) + len);
		if (*prop) {
			if (lenp)
				*lenp = len;
			return 1;
		} else
			return -FDT_ERR_BADSTRUCTURE;
	}
	return 0;
}

/*
 * Return a pointer to the string at the given string offset.
 */
char *fdt_string(const void *fdt, int stroffset)
{
	return (char *)fdt + fdt_off_dt_strings(fdt) + stroffset;
}

/*
 * Check if the specified node is compatible by comparing the tokens
 * in its "compatible" property with the specified string:
 *
 *   nodeoffset - starting place of the node
 *   compat     - the string to match to one of the tokens in the
 *                "compatible" list.
 */
int fdt_node_is_compatible(const void *fdt, int nodeoffset,
			   const char *compat)
{
	const char* cp;
	int cplen, len;

	cp = fdt_getprop(fdt, nodeoffset, "compatible", &cplen);
	if (cp == NULL)
		return 0;
	while (cplen > 0) {
		if (strncmp(cp, compat, strlen(compat)) == 0)
			return 1;
		len = strlen(cp) + 1;
		cp += len;
		cplen -= len;
	}

	return 0;
}

/*
 * Find a node by its device type property. On success, the offset of that
 * node is returned or an error code otherwise:
 *
 *   nodeoffset - the node to start searching from or 0, the node you pass
 *                will not be searched, only the next one will; typically,
 *                you pass 0 to start the search and then what the previous
 *                call returned.
 *   type       - the device type string to match against.
 */
int fdt_find_node_by_type(const void *fdt, int nodeoffset, const char *type)
{
	int offset, nextoffset;
	struct fdt_property *prop;
	uint32_t tag;
	int len, ret;

	CHECK_HEADER(fdt);

	tag = fdt_next_tag(fdt, nodeoffset, &nextoffset, NULL);
	if (tag != FDT_BEGIN_NODE)
		return -FDT_ERR_BADOFFSET;
	if (nodeoffset)
		nodeoffset = 0;	/* start searching with next node */

	while (1) {
		offset = nextoffset;
		tag = fdt_next_tag(fdt, offset, &nextoffset, NULL);

		switch (tag) {
		case FDT_BEGIN_NODE:
			nodeoffset = offset;
			break;

		case FDT_PROP:
			if (nodeoffset == 0)
				break;
			ret = prop_name_eq(fdt, offset, "device_type",
					   &prop, &len);
			if (ret < 0)
				return ret;
			else if (ret > 0 &&
				 strncmp(prop->data, type, len - 1) == 0)
			    return nodeoffset;
			break;

		case FDT_END_NODE:
		case FDT_NOP:
			break;

		case FDT_END:
			return -FDT_ERR_NOTFOUND;

		default:
			return -FDT_ERR_BADSTRUCTURE;
		}
	}
}

/*
 * Find a node based on its device type and one of the tokens in its its
 * "compatible" property. On success, the offset of that node is returned
 * or an error code otherwise:
 *
 *   nodeoffset - the node to start searching from or 0, the node you pass
 *                will not be searched, only the next one will; typically,
 *                you pass 0 to start the search and then what the previous
 *                call returned.
 *   type       - the device type string to match against.
 *   compat     - the string to match to one of the tokens in the
 *                "compatible" list.
 */
int fdt_find_compatible_node(const void *fdt, int nodeoffset,
			     const char *type, const char *compat)
{
	int offset;

	offset = fdt_find_node_by_type(fdt, nodeoffset, type);
	if (offset < 0 || fdt_node_is_compatible(fdt, offset, compat))
		return offset;

	return -FDT_ERR_NOTFOUND;
}

/*
 * Return the node offset of the node specified by:
 *   parentoffset - starting place (0 to start at the root)
 *   name         - name being searched for
 *   namelen      - length of the name: typically strlen(name)
 *
 * Notes:
 *   If the start node has subnodes, the subnodes are _not_ searched for the
 *     requested name.
 */
int fdt_subnode_offset_namelen(const void *fdt, int parentoffset,
			       const char *name, int namelen)
{
	int level = 0;
	uint32_t tag;
	int offset, nextoffset;

	CHECK_HEADER(fdt);

	tag = fdt_next_tag(fdt, parentoffset, &nextoffset, NULL);
	if (tag != FDT_BEGIN_NODE)
		return -FDT_ERR_BADOFFSET;

	do {
		offset = nextoffset;
		tag = fdt_next_tag(fdt, offset, &nextoffset, NULL);

		switch (tag) {
		case FDT_END:
			return -FDT_ERR_TRUNCATED;

		case FDT_BEGIN_NODE:
			level++;
			/*
			 * If we are nested down levels, ignore the strings
			 * until we get back to the proper level.
			 */
			if (level != 1)
				continue;

			/* Return the offset if this is "our" string. */
			if (offset_streq(fdt, offset+FDT_TAGSIZE, name, namelen))
				return offset;
			break;

		case FDT_END_NODE:
			level--;
			break;

		case FDT_PROP:
		case FDT_NOP:
			break;

		default:
			return -FDT_ERR_BADSTRUCTURE;
		}
	} while (level >= 0);

	return -FDT_ERR_NOTFOUND;
}

/*
 * See fdt_subnode_offset_namelen()
 */
int fdt_subnode_offset(const void *fdt, int parentoffset,
		       const char *name)
{
	return fdt_subnode_offset_namelen(fdt, parentoffset, name, strlen(name));
}

/*
 * Searches for the node corresponding to the given path and returns the
 * offset of that node.
 */
int fdt_find_node_by_path(const void *fdt, const char *path)
{
	const char *end = path + strlen(path);
	const char *p = path;
	int offset = 0;

	CHECK_HEADER(fdt);

	/* Paths must be absolute */
	if (*path != '/')
		return -FDT_ERR_BADPATH;

	/* Handle the root path: root offset is 0 */
	if (strcmp(path, "/") == 0)
		return 0;

	while (*p) {
		const char *q;

		/* Skip path separator(s) */
		while (*p == '/')
			p++;
		if (! *p)
			return -FDT_ERR_BADPATH;

		/*
		 * Find the next path separator.  The characters between
		 * p and q are the next segment of the the path to find.
		 */
		q = strchr(p, '/');
		if (! q)
			q = end;

		/*
		 * Find the offset corresponding to the this path segment.
		 */
		offset = fdt_subnode_offset_namelen(fdt, offset, p, q-p);

		/* Oops, error, abort abort abort */
		if (offset < 0)
			return offset;

		p = q;
	}

	return offset;
}

/*
 * Given the offset of a node and a name of a property in that node, return
 * a pointer to the property struct.
 */
struct fdt_property *fdt_get_property(const void *fdt,
				      int nodeoffset,
				      const char *name, int *lenp)
{
	int level = 0;
	uint32_t tag;
	struct fdt_property *prop;
	int offset, nextoffset;
	int err;

	if ((err = fdt_check_header(fdt)) != 0)
		goto fail;

	err = -FDT_ERR_BADOFFSET;
	if (nodeoffset % FDT_TAGSIZE)
		goto fail;

	tag = fdt_next_tag(fdt, nodeoffset, &nextoffset, NULL);
	if (tag != FDT_BEGIN_NODE)
		goto fail;

	do {
		offset = nextoffset;

		tag = fdt_next_tag(fdt, offset, &nextoffset, NULL);
		switch (tag) {
		case FDT_END:
			err = -FDT_ERR_TRUNCATED;
			goto fail;

		case FDT_BEGIN_NODE:
			level++;
			break;

		case FDT_END_NODE:
			level--;
			break;

		case FDT_PROP:
			/*
			 * If we are nested down levels, ignore the strings
			 * until we get back to the proper level.
			 */
			if (level != 0)
				continue;

			err = prop_name_eq(fdt, offset, name, &prop, lenp);
			if (err > 0)
				return prop;
			else if (err < 0)
				goto fail;
			break;

		case FDT_NOP:
			break;

		default:
			err = -FDT_ERR_BADSTRUCTURE;
			goto fail;
		}
	} while (level >= 0);

	err = -FDT_ERR_NOTFOUND;
fail:
	if (lenp)
		*lenp = err;
	return NULL;
}

/*
 * Given the offset of a node and a name of a property in that node, return
 * a pointer to the property data (ONLY).
 */
void *fdt_getprop(const void *fdt, int nodeoffset,
		  const char *name, int *lenp)
{
	const struct fdt_property *prop;

	prop = fdt_get_property(fdt, nodeoffset, name, lenp);
	if (! prop)
		return NULL;

	return (void *)prop->data;
}


uint32_t fdt_next_tag(const void *fdt, int offset, int *nextoffset, char **namep)
{
	const uint32_t *tagp, *lenp;
	uint32_t tag;
	const char *p;

	if (offset % FDT_TAGSIZE)
		return -1;

	tagp = fdt_offset_ptr(fdt, offset, FDT_TAGSIZE);
	if (! tagp)
		return FDT_END; /* premature end */
	tag = fdt32_to_cpu(*tagp);
	offset += FDT_TAGSIZE;

	switch (tag) {
	case FDT_BEGIN_NODE:
		if(namep)
			*namep = fdt_offset_ptr(fdt, offset, 1);

		/* skip name */
		do {
			p = fdt_offset_ptr(fdt, offset++, 1);
		} while (p && (*p != '\0'));
		if (! p)
			return FDT_END;
		break;
	case FDT_PROP:
		lenp = fdt_offset_ptr(fdt, offset, sizeof(*lenp));
		if (! lenp)
			return FDT_END;
		/*
		 * Get the property and set the namep to the name.
		 */
		if(namep) {
			struct fdt_property *prop;

			prop = fdt_offset_ptr_typed(fdt, offset - FDT_TAGSIZE, prop);
			if (! prop)
				return -FDT_ERR_BADSTRUCTURE;
			*namep = fdt_string(fdt, fdt32_to_cpu(prop->nameoff));
		}
		/* skip name offset, length and value */
		offset += 2*FDT_TAGSIZE + fdt32_to_cpu(*lenp);
		break;
	}

	if (nextoffset)
		*nextoffset = ALIGN(offset, FDT_TAGSIZE);

	return tag;
}

/*
 * Return the number of used reserve map entries and total slots available.
 */
int fdt_num_reservemap(void *fdt, int *used, int *total)
{
	struct fdt_reserve_entry *re;
	int  start;
	int  end;
	int  err = fdt_check_header(fdt);

	if (err != 0)
		return err;

	start = fdt_off_mem_rsvmap(fdt);

	/*
	 * Convention is that the reserve map is before the dt_struct,
	 * but it does not have to be.
	 */
	end = fdt_totalsize(fdt);
	if (end > fdt_off_dt_struct(fdt))
		end = fdt_off_dt_struct(fdt);
	if (end > fdt_off_dt_strings(fdt))
		end = fdt_off_dt_strings(fdt);

	/*
	 * Since the reserved area list is zero terminated, you get one fewer.
	 */
	if (total)
		*total = ((end - start) / sizeof(struct fdt_reserve_entry)) - 1;

	if (used) {
		*used = 0;
		while (start < end) {
			re = (struct fdt_reserve_entry *)(fdt + start);
			if (re->size == 0)
				return 0;	/* zero size terminates the list */

			*used += 1;
			start += sizeof(struct fdt_reserve_entry);
		}
		/*
		 * If we get here, there was no zero size termination.
		 */
		return -FDT_ERR_BADLAYOUT;
	}
	return 0;
}

/*
 * Return the nth reserve map entry.
 */
int fdt_get_reservemap(void *fdt, int n, struct fdt_reserve_entry *re)
{
	int  used;
	int  total;
	int  err;

	err = fdt_num_reservemap(fdt, &used, &total);
	if (err != 0)
		return err;

	if (n >= total)
		return -FDT_ERR_NOSPACE;
	if (re) {
		*re = *(struct fdt_reserve_entry *)
			_fdt_offset_ptr(fdt, n * sizeof(struct fdt_reserve_entry));
	}
	return 0;
}

#endif /* CONFIG_OF_LIBFDT */
