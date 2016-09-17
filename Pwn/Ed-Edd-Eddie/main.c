//
//  main.c
//  Ed-Edd-Eddie
//
//  Created by Christopher Thompson on 8/23/16.
//  Copyright © 2016 breadchris. All rights reserved.
//

#include "ed.h"
#include "vector.h"

int main() {
	// TODO Randomly generate filename

	// len("/tmp/") + 16 random bytes + NULL byte
    ed_props.filename = scalloc(5 + (16 * 2) + 1);

    FILE *fp = fopen("/dev/urandom", "r");
    if (fp == NULL) {
        return -1;
    }

    uint8_t rand_name[16];
   	if (16 != fread(rand_name, sizeof(char), 16, fp)) {
   		exit(-1);
   	}
    fclose(fp);

	strcpy(ed_props.filename, "/tmp/");

	size_t i;
	for (i = 0; i < 16; i++) {
		sprintf(ed_props.filename, "%s%02x", ed_props.filename, rand_name[i]);
	}

    ed_props.line_buffer = scalloc(sizeof(struct vector));
    ed_props.current_line = 0;
    ed_props.can_save = false;

    vector_init(ed_props.line_buffer);

    ed_prompt();

    free(ed_props.line_buffer);
}
