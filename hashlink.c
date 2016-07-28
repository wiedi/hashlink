/*-
 * Copyright (c) 2016 Sebastian Wiedenroth <wiedi@frubar.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

#include "uthash.h"


typedef struct {
	char *key;
	char *path;
	UT_hash_handle hh;
} INDEX_ENTRY;

INDEX_ENTRY *source_index = NULL;

char *source_dir = NULL;
char *target_dir = NULL;


int add_source_entry(char *key, char *path) {
	INDEX_ENTRY *e = NULL;

	HASH_FIND_STR(source_index, key, e);
	if(e != NULL)
		return 0;
	
	e = malloc(sizeof(INDEX_ENTRY));
	if(e == NULL)
		return -1;
	
	e->key  = key;
	e->path = path;
	
	HASH_ADD_KEYPTR(hh, source_index, e->key, strlen(e->key), e);
	return 0;
}


int read_index(char *fn, int (*handle_entry)(char *key, char*path)) {
	char *line = NULL, *part;
	size_t len = 0;
	
	char *key  = NULL;
	char *path = NULL;
	
	FILE *f = fopen(fn, "r");
	if(!f) {
		return -1;
	}

	while(getline(&line, &len, f) >= 0) {
		part = line;
		
		key = strsep(&part, " ");
		if(key == NULL)
			continue;
		path = strsep(&part, "\n");
		if(path == NULL)
			continue;

		key  = strdup(key);
		path = strdup(path);
		if(key == NULL || path == NULL) {
			goto err;
		}

		if(handle_entry(key, path)) {
			goto err;
		}

		free(line);
		line = NULL;
	}
	return 0;
err:
	free(key);
	free(path);
	free(line);
	return -1;
}


int mkdirp(char *fn) {
	if(*fn == '\0')
		return 0;

	char *p = strchr(fn + 1, '/');
	while(p) {
		*p = '\0';
		if(mkdir(fn, 0755)) {
			if (errno != EEXIST) {
				*p = '/';
				return -1;
			}
		}
		*p = '/';
		p = strchr(p + 1, '/');
	}
	return 0;
}


int create_link(char *key, char *path) {
	char *source, *target;
	INDEX_ENTRY *e = NULL;

	HASH_FIND_STR(source_index, key, e);
	if(e == NULL)
		return 0;
	
	if(asprintf(&source, "%s/%s", source_dir, e->path) < 0) {
		return -1;
	}
	if(asprintf(&target, "%s/%s", target_dir, path) < 0) {
		return -1;
	}

	if(mkdirp(target)) {
		printf("failed to create path for %s\n", target);
		return 0;
	}

	if(link(source, target)) {
		printf("failed to link: %s -> %s\n", source, target);
	}
	return 0;
}

int main(int argc, char **argv) {
	if(argc != 5) {
		printf("Usage\n\t%s <source-index> <source-dir> <target-index> <target-dir>\n\n", argv[0]);
		return 0;
	}
	
	source_dir = argv[2];
	target_dir = argv[4];
	
	if(read_index(argv[1], add_source_entry)) {
		perror("hashlink: read source index");
		return -1;
	}
		
	if(read_index(argv[3], create_link)) {
		perror("hashlink");
		return -1;
	}
	return 0;
}
