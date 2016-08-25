#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>

#ifndef CLEANUP
#define CLEANUP 0
#endif

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
	bool short_user, help;
} opts = {
	.name = "tildifer",
	.short_user = false,
	.intermediate_width = 0,
	.help = false
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
				} else if(strcmp("intermediate-width", arg + 2) == 0) {
					if(++from >= *argc) return false;
					char* end = NULL;
					errno = 0;
					long width = strtol(argv[from], &end, 10);
					if(errno || !end || *end) return false;
					if(width <= 0 || width > INT_MAX) return false;
					opts.intermediate_width = (int)width;
				} else if(strcmp("short", arg + 2) == 0) {
					opts.short_user = true;
				} else return false;
			} else if(arg[1] == 'h') {
				if(arg[2] == '\0') {
					opts.help = true;
				} else return false;
			} else return false;
		} else argv[to++] = arg;
	}
	if(!to) argv[to++] = ".";
	*argc = to;
	return true;
}

static void usage(FILE* f) {
	fprintf(f, "Usage: %s [options] [--] [path ...]\n", opts.name);
	fprintf(f, "Prints a normalized version of each path. If none is given, then \".\" is used.\n");
	fprintf(f, "\n");
	fprintf(f, "Options:\n");
	fprintf(f, "  --                         Stop parsing Arguments\n");
	fprintf(f, "  -h --help                  Show this help, then exit\n");
	fprintf(f, "     --intermediate-width N  Shorten all intermediate directories to N characters\n");
	fprintf(f, "     --short                 Shorten the home directory of the current user to just '~'\n");
}

static void tildify(char const* path) {
	setpwent();
	for(struct passwd* pwent; (pwent = getpwent()); ) {
		char const* home = pwent->pw_dir;
		assert(home[0] == '/' && "home directories must be absolute paths");
		if(home[1]) {
			size_t i = 1;
			while(home[i] && path[i] && path[i] == home[i]) ++i;
			if(home[i] == '\0' || (home[i] == '/' && home[i + 1] == '\0' && path[i] == '\0')) {
				if(home[i] == '\0' && home[i - 1] == '/') --i;
				if(opts.intermediate_width > 0) {
					printf("~%s", opts.short_user && opts.uid == pwent->pw_uid ? "" : pwent->pw_name);
					size_t j = i;
					while(path[j]) {
						printf("/%.*s", opts.intermediate_width, path + j + 1);
					}
					printf("%s\n", path);
				} else printf("%s\n", path + i);
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
	}

	for(int i = 0; i < argc; ++i) {
		if(argv[i][0] != '/') {
			char* buff = realpath(argv[i], NULL);
			if(!buff) {
				#if CLEANUP
				endpwent();
				#endif
				return RC_REALPATH_FAILURE;
			}
			tildify(buff);
			free(buff);
		} else tildify(argv[i]);
	}

	#if CLEANUP
	endpwent();
	#endif
	return RC_SUCCESS;
}
