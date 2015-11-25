#include "../../../common.h"

/*
 * Describes a single result of stat() invocation, along with some supplemental information.
 * Used to decouple information collection and printout steps, which is in turn needed
 * (aside from better composition in source code) to table-format the output.
 * 
 * All `char *` fields own data (i. e. are freed together with the struct instance itself).
 */
struct item {
	struct item *next; /* poor man's linked list */

	char type;
	char perms[10];
	char *link_count;
	char *owner;
	char *group;
	char *size_from_blocks;
	char *size;
	char *mtime;
	char *name;
	char *full_path;
	bool is_dir;
};

/*
 * Holds maximal column widths encountered for given item set during printout. 
 */
struct field_widths {
	int link_count;
	int owner;
	int group;
	int size_from_blocks;
	int size;
	int mtime;
};

/*
 * Allocates a new `struct item`.
 * 
 * @last: previous allocated item; used to arrange items into a single-linked list.
 */
struct item *item_new(struct item *last)
{
	struct item *item = calloc(1, sizeof(struct item));
	assert(item);

	if (last) {
		last->next = item;
	}

	return item;
}

/*
 * Deallocates a `struct item` together with all strings contained in it.
 * `item->next` must be NULL (i. e. it does not deallocate items further down the list).
 */
void item_free(struct item *item)
{
	if (item != NULL) {
		assert(item->next == NULL);
		free(item->link_count);
		free(item->owner);
		free(item->group);
		free(item->size_from_blocks);
		free(item->size);
		free(item->mtime);
		free(item->name);
		free(item->full_path);
		free(item);
	}
}

/*
 * Deallocates a list of `struct item`, starting from its head.
 */
void item_free_list(struct item *head)
{
	while (head != NULL) {
		struct item *next = head->next;
		head->next = NULL;
		item_free(head);
		head = next;
	}
}

/*
 * RAII-style cleanup wrappers and definitions for `struct item`.
 * See common.h.
 */

DEFINE_TRIVIAL_CLEANUP_FUNC(struct item *, item_free_list)
#define _cleanup_item_list_ \
	_cleanup_(item_free_listp)

/*
 * Convert a size value into a human-readable form using binary SI suffixes
 * and fixed-point decimal arithmetic.
 *
 * The result string is allocated and returned in asprintf(3) style.
 */
void pretty_print_size(char **result, size_t size)
{
	assert(result);

	if (size < 1024) {
		asprintf(result, "%zu", size);
	} else {
		const char *suffixes = " KMGTPE";

		size *= 10; /* poor man's decimal fixed-point representation with precision of 1 */

		do {
			++suffixes;
			size /= 1024;
		} while (suffixes[1] && size >= 1024 * 10);

		asprintf(result, "%zu%s%zu%c",
		         size / 10,
		         localeconv()->decimal_point,
		         size % 10,
		         suffixes[0]);
	}
}

/*
 * Concatenates two pathes via a slash, if first is non-empty. Does not do any normalization.
 * 
 * The result path is allocated and returned.
 */
char *full_path(const char *dir_path, const char *file_name)
{
	assert(dir_path);
	assert(file_name);

	if (strempty(dir_path)) {
		return strdup(file_name);
	} else {
		char *result = NULL;
		asprintf(&result, "%s/%s", dir_path, file_name);
		assert(result);
		return result;
	}
}

/*
 * Performs stat() on one file and returns a new `struct item` containing information about the file.
 * 
 * @dir_fd: file descriptor opened for the file's directory, openat(3p) style; may be AT_FDCWD.
 * @dir_path: path to the directory pointed to by @dir_fd; used for output and diagnostic messages.
 * @file_name: target file name relative to the directory pointed to by @dir_fd.
 * @last: previously returned item, used to arrange items in a single linked list.
 */
struct item *fill_one_item(int dir_fd, const char *dir_path, const char *file_name, struct item *last)
{
	struct stat st;
	struct passwd *owner;
	struct group *group;
	struct item *item;
	_cleanup_free_ char *file_full_path = NULL;
	int r;

	assert(dir_path);
	assert(file_name);

	file_full_path = full_path(dir_path, file_name);

	/*
	 * Collect the file metadata first.
	 */
	
	r = fstatat(dir_fd, file_name, &st, AT_SYMLINK_NOFOLLOW);
	if (r < 0) {
		log ("Cannot stat '%s': %m", file_full_path);
		return NULL;
	}

	errno = 0;
	owner = getpwuid(st.st_uid);
	if (owner == NULL && errno != 0) {
		log ("Cannot get owner of '%s': %m", file_full_path);
		return NULL;
	}

	errno = 0;
	group = getgrgid(st.st_gid);
	if (group == NULL && errno != 0) {
		log ("Cannot get group of '%s': %m", file_full_path);
		return NULL;
	}

	/*
	 * Then fill out the symbolic fields.
	 * We do this instead of printf()'ing right away in order to make a neat aligned table-formatted output.
	 */

	item = item_new(last);

#define FORMAT_TYPE(checker, symbol) if (checker(st.st_mode)) { item->type = symbol; break; }
	item->type = '?';
	do {
		FORMAT_TYPE(S_ISREG, '-')
		FORMAT_TYPE(S_ISDIR, 'd')
		FORMAT_TYPE(S_ISLNK, 'l')
		FORMAT_TYPE(S_ISBLK, 'b')
		FORMAT_TYPE(S_ISCHR, 'c')
		FORMAT_TYPE(S_ISFIFO, 'p')
		FORMAT_TYPE(S_ISSOCK, 's')
	} while (0);
#undef FORMAT_TYPE

#define FORMAT_PERM(perm, symbol)                                              \
	(st.st_mode & perm)                                                    \
	? symbol                                                               \
	: '-'
#define FORMAT_TWO_PERMS(perm1, perm2, symbol1, symbol2, symbol12)             \
	(st.st_mode & perm1)                                                   \
	? (st.st_mode & perm2)                                                 \
	  ? symbol12                                                           \
	  : symbol1                                                            \
	: (st.st_mode & perm2)                                                 \
	  ? symbol2                                                            \
	  : '-'
	snprintf(item->perms, sizeof(item->perms),
	         "%c%c%c%c%c%c%c%c%c",
	         FORMAT_PERM(S_IRUSR, 'r'),
	         FORMAT_PERM(S_IWUSR, 'w'),
	         FORMAT_TWO_PERMS(S_IXUSR, S_ISUID, 'x', 'S', 's'),
	         FORMAT_PERM(S_IRGRP, 'r'),
	         FORMAT_PERM(S_IWGRP, 'w'),
	         FORMAT_TWO_PERMS(S_IXGRP, S_ISGID, 'x', 'S', 's'),
	         FORMAT_PERM(S_IROTH, 'r'),
	         FORMAT_PERM(S_IWOTH, 'w'),
	         FORMAT_TWO_PERMS(S_IXOTH, S_ISVTX, 'x', 'T', 't'));
#undef FORMAT_PERM
#undef FORMAT_TWO_PERMS

	asprintf(&item->link_count, "%lu", st.st_nlink);

	if (owner != NULL) {
		asprintf(&item->owner, "%s", owner->pw_name);
	} else {
		asprintf(&item->owner, "%d", st.st_uid);
	}

	if (group != NULL) {
		asprintf(&item->group, "%s", group->gr_name);
	} else {
		asprintf(&item->group, "%d", st.st_gid);
	}

	pretty_print_size(&item->size_from_blocks, 512 * st.st_blocks);

	if (S_ISDIR(st.st_mode)) {
		asprintf(&item->size, "%zu", (size_t)st.st_size);
	} else {
		pretty_print_size(&item->size, st.st_size);
	}

	astrftime(&item->mtime, "%c", st.st_mtim.tv_sec);

	item->name = strdup(file_name);
	assert(item->name);

	item->full_path = file_full_path;
	file_full_path = NULL;

	item->is_dir = S_ISDIR(st.st_mode);

	return item;
}

/*
 * Computes the maximal field width during printout.
 * 
 * @max_field_width: maximal field width encountered so far.
 * @field: that field of current item.
 */
void max_field_width(size_t *max_field_width, const char *field)
{
	assert(max_field_width);
	assert(field);

	size_t field_width = strlen(field);
	if (field_width > *max_field_width) {
		*max_field_width = field_width;
	}
}

/*
 * Performs printout for given set (list) of items.
 * 
 * @item_list: item list head.
 */
void print_items(struct item *item_list)
{
	struct field_widths widths;

	memzero(widths);

	for (struct item *item = item_list; item != NULL; item = item->next) {
		int w;

#define PROCESS_FIELD(field) do { w = (int)strlen(item->field); if (w > widths.field) { widths.field = w; } } while (0)
		PROCESS_FIELD(link_count);
		PROCESS_FIELD(owner);
		PROCESS_FIELD(group);
		PROCESS_FIELD(size_from_blocks);
		PROCESS_FIELD(size);
		PROCESS_FIELD(mtime);
#undef PROCESS_FIELD
	}

	for (struct item *item = item_list; item != NULL; item = item->next) {
		printf("%c%9s %-*s %-*s %-*s %*s (%*s on disk) %-*s %s\n",
		       item->type,
		       item->perms,
		       widths.link_count, item->link_count,
		       widths.owner, item->owner,
		       widths.group, item->group,
		       widths.size, item->size,
		       widths.size_from_blocks, item->size_from_blocks,
		       widths.mtime, item->mtime,
		       item->name);
	}

	putchar('\n');
}

int list_dir(int dir_fd, const char *dir_path);

/*
 * Lists the given set of items (i. e. performs printout and recurses into directories).
 * 
 * @dir_fd: file descriptor opened for the items' directory, openat(3p) style; may be AT_FDCWD.
 * @item_list: item list head.
 */
int list_items(int dir_fd, struct item *item_list)
{
	int r, had_errors = 0;

	print_items(item_list);

	for (struct item *item = item_list; item != NULL; item = item->next) {
		if (item->is_dir) {
			_cleanup_close_ int fd = openat(dir_fd, item->name, O_RDONLY|O_DIRECTORY);

			if (fd < 0) {
				log("Failed to openat() directory '%s': %m", item->full_path);
				had_errors = -1;
				continue;
			}

			r = list_dir(fd, item->full_path);
			if (r < 0) {
				log("Failed to list directory '%s': %m", item->full_path);
				had_errors = -1;
			}
		}
	}

	return had_errors;
}

/*
 * Lists all files in the given directory.
 * 
 * @dir_fd: file descriptor opened for the target directory.
 * @dir_path: path to the directory pointed to by @dir_fd; used for output and diagnostic messages.
 */
int list_dir(int dir_fd, const char *dir_path)
{
	_cleanup_free_ struct dirent *dirent = NULL;
	_cleanup_item_list_ struct item *item_list_head = NULL;
	struct item *item_list_tail = NULL;
	DIR* dir;
	struct dirent *direntp;
	long name_max;
	int r;

	assert(dir_path);

	dir = fdopendir(dir_fd);
	if (dir == NULL) {
		log("Failed to fdopendir() directory '%s': %m", dir_path);
		return -1;
	}

	errno = 0;
	name_max = fpathconf(dir_fd, _PC_NAME_MAX);
	if (name_max < 0) {
		name_max = NAME_MAX;
		log("Failed to call fpathconf() to get path name length limits for directory '%s', assuming %ld: %m", dir_path, name_max);
	}

	dirent = malloc(offsetof(struct dirent, d_name) + name_max + 1);
	assert(dirent);

	for (;;) {
		r = readdir_r(dir, dirent, &direntp);
		if (r != 0) {
			errno = r;
			log("Failed to readdir_r() directory '%s': %m", dir_path);
			return -1;
		}
		if (direntp == NULL) {
			break;
		}

		if (streq(dirent->d_name, ".") ||
		    streq(dirent->d_name, "..")) {
			continue;
		}

		item_list_tail = fill_one_item(dir_fd, dir_path, dirent->d_name, item_list_tail);
		if (item_list_tail == NULL) {
			return -1;
		}
		if (item_list_head == NULL) {
			item_list_head = item_list_tail;
		}
	}

	if (!strempty(dir_path)) {
		printf("Listing of directory %s:\n", dir_path);
	}

	return list_items(dir_fd, item_list_head);
}

/*
 * Lists the given file names.
 * 
 * @dir_fd: file descriptor opened for the files' directory, openat(3p) style; may be AT_FDCWD.
 * @dir_path: path to the directory pointed to by @dir_fd; used for output and diagnostic messages.
 * @args: array of file names.
 * @args_nr: count of file names in the array pointed to by @args.
 */
int list_args(int dir_fd, const char *dir_path, const char **args, size_t args_nr)
{
	_cleanup_item_list_ struct item *item_list_head = NULL;
	struct item *item_list_tail = NULL;

	assert(args);

	for (size_t i = 0; i < args_nr; ++i) {
		item_list_tail = fill_one_item(dir_fd, dir_path, args[i], item_list_tail);
		if (item_list_tail == NULL) {
			return -1;
		}
		if (item_list_head == NULL) {
			item_list_head = item_list_tail;
		}
	}

	return list_items(dir_fd, item_list_head);
}

int main(int argc, char **argv)
{
	/* init the locale subsystems */
	setlocale(LC_ALL, "");
	tzset();

	if (argc > 1) {
		return list_args(AT_FDCWD, "", (const char**)argv + 1, argc - 1);
	} else {
		_cleanup_close_ int cwd_fd = open(".", O_RDONLY|O_DIRECTORY);
		if (cwd_fd < 0) {
			log("Failed to open() current working directory: %m");
			return 1;
		}
		return list_dir(cwd_fd, "");		
	}
}
