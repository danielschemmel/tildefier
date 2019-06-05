#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#ifndef CLEANUP
#define CLEANUP 0
#endif

#define TILDEFIER_VERSION_STRING "tildefier version " VERSION

enum rc {
	RC_SUCCESS,
	RC_ARGUMENT_PARSING,
	RC_REALPATH_FAILURE,
	RC_MALLOC_FAILURE,
	RC_PWD_FAILURE
};

static struct {
	char const* name;
	uid_t uid;
	int intermediate_width;
	bool newline, short_user, help, version;
} opts = {
	.name = "tildefier",
	.newline = true,
	.short_user = false,
	.intermediate_width = 0,
	.help = false,
	.version = false,
};

static void set_default_opts(void) {
	opts.uid = geteuid();
}

static bool parse_opts(int* argc, char** argv) {
	set_default_opts();
	if(*argc > 0 && argv[0][0]) opts.name = argv[0];
	int to = 0;
	bool stop = false;
	for(int from = 1; from < *argc; ++from) {
		char* arg = argv[from];
		if(!stop && arg[0] == '-') {
			if(arg[1] == '-') {
				if(arg[2] == '\0') {
					stop = true;
				} else if(strcmp("help", arg + 2) == 0) {
					opts.help = true;
				} else if(strcmp("version", arg + 2) == 0) {
					opts.version = true;
				} else if(strcmp("intermediate-width", arg + 2) == 0) {
					if(++from >= *argc) return false;
					char* end = NULL;
					errno = 0;
					long width = strtol(argv[from], &end, 10);
					if(errno || !end || *end) return false;
					if(width <= 0 || width > INT_MAX) return false;
					opts.intermediate_width = (int)width;
				} else if(strcmp("no-newline", arg + 2) == 0) {
					opts.newline = false;
				} else if(strcmp("short", arg + 2) == 0) {
					opts.short_user = true;
				} else return false;
			} else if(arg[1] == 'h') {
				if(arg[2] == '\0') {
					opts.help = true;
				} else return false;
			} else if(arg[1] == 'n') {
				if(arg[2] == '\0') {
					opts.newline = false;
				} else return false;
			} else if(arg[1] == 'v') {
				if(arg[2] == '\0') {
					opts.version = true;
				} else return false;
			} else return false;
		} else argv[to++] = arg;
	}
	if(!to) argv[to++] = ".";
	*argc = to;
	return true;
}

static void usage(FILE* f) {
	fprintf(f,
		"Usage: %s [options] [--] [path ...]\n"
		"Prints a normalized, absolute version of each path where the prefix is given as an user's home directory if possible. If no paths are given on the command line, the current working directory is used.\n"
		"\n"
		"Options:\n"
		"  --                         Stop parsing Arguments\n"
		"  -h --help                  Show this help, then exit\n"
		"     --intermediate-width N  Shorten all intermediate directories to N characters\n"
		"  -n --no-newline            Do not output a newline\n"
		"     --short                 Shorten the home directory of the current user to just '~'\n"
		"  -v --version               Print \"" TILDEFIER_VERSION_STRING "\"\n"
	, opts.name);
}

static void tildefy(char const* path) {
	setpwent();
	for(struct passwd* pwent; (pwent = getpwent()); ) {
		char const* home = pwent->pw_dir;
		assert(home[0] == '/' && "home directories must be absolute paths");
		if(home[1]) {
			size_t i = 1;
			while(home[i] && path[i] && path[i] == home[i]) ++i;
			if(home[i] == '\0' || (home[i] == '/' && home[i + 1] == '\0' && path[i] == '\0')) {
				if(home[i] == '\0' && home[i - 1] == '/') --i;
				printf("~%s", opts.short_user && opts.uid == pwent->pw_uid ? "" : pwent->pw_name);
				if(opts.intermediate_width > 0) {
					for(;;) {
						size_t j = i;
						do {
							++i;
						} while(path[i] && path[i] != '/');
						if(path[i]) {
							int width = (int)(i - j - 1) < opts.intermediate_width ? (int)(i - j - 1) : opts.intermediate_width;
							printf("/%.*s", width, path + j + 1);
						} else {
							printf("/%s%s", path + j + 1, opts.newline ? "\n" : "");
							break;
						}
					}
				} else printf("%s%s", path + i, opts.newline ? "\n" : "");
				return;
			}
		}
	}
	puts(path);
}

int main(int argc, char** argv) {
	if(!parse_opts(&argc, argv)) {
		usage(stderr);
		return RC_ARGUMENT_PARSING;
	} else if(opts.help) {
		usage(stdout);
		return RC_SUCCESS;
	} else if(opts.version) {
		puts(TILDEFIER_VERSION_STRING);
		return RC_SUCCESS;
	}

	for(int i = 0; i < argc; ++i) {
		char* buff = realpath(argv[i], NULL);
		if(!buff) {
			#if CLEANUP
			endpwent();
			#endif
			int err = errno;
			switch(err) {
				case EACCES:
					fprintf(stderr, "Cannot open %s (permission denied)\n", argv[i]);
					break;

				case ENOTDIR:
				case ENOENT:
					fprintf(stderr, "Cannot open %s (path does not exist)\n", argv[i]);
					break;

				default:
					fprintf(stderr, "Cannot open %s (error %d)\n", argv[i], err);
					break;
			}
			return RC_REALPATH_FAILURE;
		}
		tildefy(buff);
		free(buff);
	}

	#if CLEANUP
	endpwent();
	#endif
	return RC_SUCCESS;
}
