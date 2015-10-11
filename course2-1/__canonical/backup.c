#define _DEFAULT_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>

#define log(fmt, ...) fprintf (stderr, fmt "\n", ## __VA_ARGS__)
#define die(fmt, ...) do { log(fmt, ## __VA_ARGS__); exit(EXIT_FAILURE); } while (0)
#define chk(expr, fmt, ...) do { if (!(expr)) die(fmt, ## __VA_ARGS__); } while (0)

void __attribute__((noreturn)) wait_and_exec(int delay, const char *path)
{
	sleep(delay);
	execlp(path, path, NULL);
	die("Failed to exec() the file \"%s\": %m", path);
}

struct backup_parameters
{
	/* an strv which has place for yet another argument */
	const char **archiver_argv;
	/* count of arguments already in the strv, without to-be-appended path */
	int archiver_argc;
	/* printf(3)-formatted template of output file name of the archiver */
	const char *archiver_out_template;
	
};

struct backup_state
{
	int in_fd;
	int out_fd;
};

const char *dbg_st_mode_to_string(int st_mode)
{
#define ENTRY(checker, description) if(checker(st_mode)) return description;
	ENTRY(S_ISBLK, "block special")
	ENTRY(S_ISCHR, "character special")
	ENTRY(S_ISFIFO, "FIFO special")
	ENTRY(S_ISREG, "regular file")
	ENTRY(S_ISDIR, "directory")
	ENTRY(S_ISLNK, "symbolic link (broken)")
	ENTRY(S_ISSOCK, "socket")
#undef ENTRY
	return "<unknown st_mode value>";
}

int timespec_less(const struct timespec *a, const struct timespec *b)
{
	return a->tv_sec < b->tv_sec ||
	       (a->tv_sec == b->tv_sec && a->tv_nsec < b->tv_nsec);
}

#define BUFFER_SIZE 1024

int cat_fd(int fd_in, int fd_out)
{
        char buffer[BUFFER_SIZE];
        ssize_t read_bytes, written_bytes;

        while((read_bytes = read(fd_in, buffer, BUFFER_SIZE)) > 0) {
                written_bytes = write(fd_out, buffer, read_bytes);

                if (written_bytes < 0) {
	                log("Could not write() %zd bytes to the destination file descriptor: %m", read_bytes);
                        return -1;
                }
        }

        if (read_bytes < 0) {
		log("Could not read() %zd bytes to the destination file descriptor: %m", BUFFER_SIZE);
                return -1;
        }

        return 0;
}

int backup_file(const struct backup_parameters *parameters, const struct backup_state *parent_state,
                const char *in_file, const char *out_file)
{
	int r;

	int in_fd = openat(parent_state->in_fd, in_file, O_RDONLY|O_CLOEXEC);
	chk(in_fd > 0, "Could not openat() input file \"%s\": %m", in_file);

	int out_fd = openat(parent_state->out_fd, out_file, O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC, 0666);
	chk(out_fd > 0, "Could not openat() output file \"%s\": %m", out_file);

	r = cat_fd(in_fd, out_fd);
	close(in_fd);
	close(out_fd);

	if (r < 0) {
		return -1;
	}

	const char **archiver_argv = parameters->archiver_argv;
	int archiver_argc = parameters->archiver_argc;

	archiver_argv[archiver_argc++] = out_file;

	int child_pid = fork();
	if (child_pid == 0) {
		r = fchdir(parent_state->out_fd);
		chk(r == 0, "Could not chdir() to the directory of the destination file \"%s\": %m", out_file);

		execvp(archiver_argv[0], (char**) archiver_argv);
		die("Could not execvp() archiver '%s': %m", archiver_argv[0]);
	}
	chk(child_pid > 0, "Could not fork() off the archiver process: %m");

	int child_exitcode;
	r = waitpid(child_pid, &child_exitcode, 0);
	chk(r > 0, "Could not waitpid() for child %d: %m", child_pid);
	chk(r == child_pid, "Unexpected return from waitpid() for child %d: %d", child_pid, r);

	if (WIFEXITED(child_exitcode) && WEXITSTATUS(child_exitcode) != 0) {
		log("Archiver for destination file \"%s\" exited with non-zero value %d",
		    out_file, WEXITSTATUS(child_exitcode));
		return -1;
	}

	if (WIFSIGNALED(child_exitcode)) {
		log("Archiver for destination file \"%s\" was terminated with signal %d",
		    out_file, WTERMSIG(child_exitcode));
		return -1;
	}

	return 0;
}

int backup_dir(const struct backup_parameters *parameters, const struct backup_state *parent_state,
               const char *in_subdir, const char *out_subdir)
{
	int r, errors = 0;

	r = mkdirat(parent_state->out_fd, out_subdir, 0777);
	/* Ignore "File exists" here, because in order to fully handle that error we also need to check whether the existing entry
	 * is a directory (which is not an error), which means a TOCTTOU race condition. Instead, this is checked by
	 * openat(..., O_DIRECTORY) slightly below, which will fail if the existing entry is not a directory. */
	chk(r == 0 || errno == EEXIST, "Could not mkdirat() output subdirectory \"%s\": %m",
	    out_subdir);

	struct backup_state state = {
		.in_fd = -1,
		.out_fd = -1
	};

	state.in_fd = openat(parent_state->in_fd, in_subdir, O_RDONLY|O_DIRECTORY|O_CLOEXEC);
	chk(state.in_fd > 0, "Could not openat() input subdirectory \"%s\": %m", in_subdir);

	state.out_fd = openat(parent_state->out_fd, out_subdir, O_RDONLY|O_DIRECTORY|O_CLOEXEC);
	chk(state.out_fd > 0, "Could not openat() output subdirectory \"%s\": %m", out_subdir);

	DIR *in_dir = fdopendir(state.in_fd);
	chk(in_dir, "Could not fdopendir() input subdirectory \"%s\": %m", in_subdir);

	/* reserve space for the directory entry structure, as per dirent->h(0p) and readdir(3p) */
	struct dirent *dirent = alloca(sizeof(struct dirent) + NAME_MAX + 1);
	struct dirent *readdir_result = NULL;

	for (;;) {
		r = readdir_r(in_dir, dirent, &readdir_result);
		chk(r == 0, "Failed to readdir_r() input subdirectory \"%s\": %m", in_subdir);
		if (!readdir_result) {
			break;
		}

		if (!strcmp(dirent->d_name, ".") ||
		    !strcmp(dirent->d_name, "..")) {
			continue;
		}

		struct stat in_st;
		r = fstatat(state.in_fd, dirent->d_name, &in_st, 0);
		chk(r == 0, "Failed to fstatat() file \"%s\" in the input subdirectory \"%s\": %m",
		    dirent->d_name, in_subdir);

		if (S_ISDIR(in_st.st_mode)) {
			r = backup_dir(parameters, &state, dirent->d_name, dirent->d_name);
			if (r < 0) {
				++errors;
			}
		} else if (S_ISREG(in_st.st_mode)) {
			char *out_file = alloca(NAME_MAX + 1);
			snprintf(out_file, NAME_MAX + 1, parameters->archiver_out_template /* %s.gz */, dirent->d_name);

			struct stat out_st;
			r = fstatat(state.out_fd, out_file, &out_st, 0);
			chk(r == 0 || errno == ENOENT, "Could not fstatat() file \"%s\" in the output subdirectory \"%s\": %m",
			    out_file, out_subdir);

			if (r != 0 || timespec_less(&out_st.st_mtim, &in_st.st_mtim)) {
				r = backup_file(parameters, &state, dirent->d_name, dirent->d_name);
				if (r < 0) {
					++errors;
				}
			}
		} else {
			log("Don't know how to handle %s \"%s\" in the input subdirectory \"%s\"",
			    dbg_st_mode_to_string(in_st.st_mode), dirent->d_name, in_subdir);
			++errors;
		}
	}

	closedir(in_dir);
	close(state.in_fd);
	close(state.out_fd);

	return errors ? -1 : 0;
}

int main(int argc, char **argv)
{
	int r;

	if (argc != 3) {
		die("This program expects two arguments.");
	}

	const char *in_dir = argv[1], *out_dir = argv[2];

	const char *archiver_argv_template[] = {
		"gzip",
		"-f",
		"-9",
		NULL /* placeholder for the input file path ptr */,
		NULL
	};

	struct backup_parameters parameters = {
		.archiver_argv = archiver_argv_template,
		.archiver_argc = (sizeof(archiver_argv_template) / sizeof(*archiver_argv_template)) - 2,
		.archiver_out_template = "%s.gz"
	};

	/* initial values to open in/out directories with */
	struct backup_state state = {
		.in_fd = AT_FDCWD,
		.out_fd = AT_FDCWD
	};

	r = backup_dir(&parameters, &state, in_dir, out_dir);

	return r < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
