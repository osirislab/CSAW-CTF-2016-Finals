
#include "ed.h"

struct ed_properties ed_props;

void *scalloc(size_t size) {
    void *ptr = calloc(1, size);
    if (NULL == ptr) {
        print_err("Uh, double dee, we don't have no more butterd toast");
    }
    return ptr;
}

void *smalloc(size_t size) {
    void *ptr = malloc(size);
    if (NULL == ptr) {
        print_err("Uh, double dee, we don't have no more butterd toast");
    }
    return ptr;
}

void *srealloc(void *ptr, size_t size) {
    ptr = realloc(ptr, size);
    if (NULL == ptr) {
        print_err("Uh, double dee, we don't have no more butterd toast");
    }
    return ptr;
}

bool add_would_overflow(size_t x, size_t y) {
    return (y > 0) && (x > SIZE_MAX - y);
}

bool sub_would_overflow(size_t x, size_t y) {
    return x < y;
}

/*
 _____    _   _     _             ______                    
|  ___|  | | | |   (_)            |  ___|                   
| |__  __| | | |    _ _ __   ___  | |_ _   _ _ __   ___ ___ 
|  __|/ _` | | |   | | '_ \ / _ \ |  _| | | | '_ \ / __/ __|
| |__| (_| | | |___| | | | |  __/ | | | |_| | | | | (__\__ \
\____/\__,_| \_____/_|_| |_|\___| \_|  \__,_|_| |_|\___|___/
                                                            

*/

int ed_append(struct ed_command *cmd) {
    struct ed_str *line_buffer;

    if (cmd->src_start > vector_count(ed_props.line_buffer)) {
        return -1;
    }

    if (0 == vector_count(ed_props.line_buffer)) {
        ed_props.current_line = 0;
    } else {
        ed_props.current_line = cmd->src_start + 1;
    }

    for (;;) {
        // [1] Alloc made
        line_buffer = ed_read_line();

        if (NULL == line_buffer) {
            return -1;
        }
        // Stop reading on single '.'
        if (0 == strncmp(line_buffer->str, ".\n", 2)) {
            // [1] Alloc freed
            free(line_buffer->str);
            free(line_buffer);
            break;
        } else {
            // [1] Alloc stored
            vector_insert(ed_props.line_buffer, ed_props.current_line, line_buffer);
            ed_props.current_line += 1;
        }
    }
    ed_props.current_line -= 1;
    return 0;
}

int ed_change(struct ed_command *cmd) {
    struct ed_str *line_buffer;

    if (cmd->src_start > vector_count(ed_props.line_buffer)) {
        return -1;
    }

    ed_props.current_line = cmd->src_start;

    for (;;) {
        // [1] Alloc made
        line_buffer = ed_read_line();

        if (NULL == line_buffer) {
            return -1;
        }
        // Stop reading on single '.'
        if (0 == strncmp(line_buffer->str, ".\n", 2)) {
            // [1] Alloc freed
            free(line_buffer->str);
            free(line_buffer);
            break;
        } else {
            if (ed_props.current_line < vector_count(ed_props.line_buffer)) {
                // [1] Alloc stored
                vector_set(ed_props.line_buffer, ed_props.current_line, line_buffer);
            } else {
                // [1] Alloc stored
                vector_add(ed_props.line_buffer, line_buffer);
            }
            ed_props.current_line++;
        }
    }
    ed_props.current_line -= 1;
    return 0;
}

int ed_delete(struct ed_command *cmd) {
    size_t i, count;

    if (cmd->src_start >= cmd->src_end) {
        count = 1;
    } else {
        count = cmd->src_end - cmd->src_start;
    }

    if (add_would_overflow(cmd->src_start, count)
        || cmd->src_start + count > vector_count(ed_props.line_buffer)) {
        return -1;
    }

    for (i = 0; i < count; i++) {
        vector_delete(ed_props.line_buffer, cmd->src_start);
    }

    ed_props.current_line = cmd->src_start;
    return 0;
}

int ed_insert(struct ed_command *cmd) {
    struct ed_str *line_buffer;

    if (cmd->src_start > vector_count(ed_props.line_buffer)) {
        return -1;
    }

    ed_props.current_line = cmd->src_start;

    for (;;) {
        // [1] Alloc made
        line_buffer = ed_read_line();

        if (NULL == line_buffer) {
            return -1;
        }
        // Stop reading on single '.'
        if (0 == strncmp(line_buffer->str, ".\n", 2)) {
            // [1] Alloc freed
            free(line_buffer->str);
            free(line_buffer);
            break;
        } else {
            if (ed_props.current_line < vector_count(ed_props.line_buffer)) {
                // [1] Alloc stored
                vector_insert(ed_props.line_buffer, ed_props.current_line, line_buffer);
            } else {
                // [1] Alloc stored
                vector_add(ed_props.line_buffer, line_buffer);
            }
            ed_props.current_line++;
        }
    }
    ed_props.current_line -= 1;
    return 0;
}

int ed_join(struct ed_command *cmd) {
    size_t i, count, new_count;
    struct ed_str *line, *tmp_line;

    if (cmd->src_start >= cmd->src_end) {
        count = 1;
    } else {
        count = cmd->src_end - cmd->src_start;
    }

    if (add_would_overflow(cmd->src_start, count)
        || cmd->src_start + count > vector_count(ed_props.line_buffer)) {
        return -1;
    }

    if (2 > vector_count(ed_props.line_buffer)) {
        return 0;
    }

    new_count = vector_count(ed_props.line_buffer);
    line = NULL;
    tmp_line = NULL;
    for (i = 0; i < count - 1; i++) {
        tmp_line = vector_get(ed_props.line_buffer, cmd->src_start);

        // Get rid of newline
        tmp_line->str[tmp_line->len - 1] = '\0';

        if (NULL == line) {
            line = (struct ed_str *)scalloc(sizeof(struct ed_str));
            line->len = tmp_line->len;
            line->str = (char *)scalloc(line->len);
        } else {
	    if (add_would_overflow(line->len, tmp_line->len)) {
	        return -1;
	    }
            line->len += tmp_line->len;
            line->str = srealloc(line->str, line->len);
        }

        memcpy(line->str + line->len - tmp_line->len, tmp_line->str, tmp_line->len);
        vector_delete(ed_props.line_buffer, cmd->src_start);

        // Bad code!!!!
        // For every line joined, the count is subtracted twice
        // Cuasing the vars to become out of sync
        new_count -= 2;
    }

    // Add newline and null byte to end of new line
    line->str[line->len - 1] = '\n';

    vector_insert(ed_props.line_buffer, cmd->src_start, line);

    ed_props.current_line = cmd->src_start;
    ed_props.line_buffer->count = new_count;

    return 0;
}

int ed_mark(struct ed_command *cmd) {
    if ((uint8_t) cmd->src_start > (uint8_t) ed_props.line_buffer->count) {
        return -1;
    }

    // Ehhh... sure? lol
    // 
    // Leads to arbituary memory write
    // 
    // Keep adding marks until overflow into mark_count
    // then set it to negative to overwrite the can_save flag

    ed_props.marks[ed_props.mark_count].mark_char = cmd->mark_char;
    ed_props.marks[ed_props.mark_count].line = cmd->src_start;
    ed_props.mark_count += 1;
    return 0;
}

int ed_copy(struct ed_command *cmd) {
    struct vector tmp_vec;
    size_t i, count;

    if (cmd->src_start >= cmd->src_end) {
        count = 1;
    } else {
        count = cmd->src_end - cmd->src_start;
    }

    if (add_would_overflow(cmd->src_start, count)
        || cmd->src_start + count > vector_count(ed_props.line_buffer)
        || cmd->dst_end < vector_count(ed_props.line_buffer)) {
        return -1;
    }

    if (0 == vector_count(ed_props.line_buffer)) {
        return 0;
    }

    vector_init(&tmp_vec);

    for (i = 0; i < count; i++) {
        vector_add(&tmp_vec, vector_get(ed_props.line_buffer, cmd->src_start + i));
    }

    ed_props.current_line = cmd->dst_start;
    for (i = 0; i < count; i++) {
        vector_insert(ed_props.line_buffer, ed_props.current_line,
            vector_get(&tmp_vec, i));
        ed_props.current_line++;
    }

    // Cleanup
    free(tmp_vec.data);

    return 0;
}

int ed_move(struct ed_command *cmd) {
    struct vector tmp_vec;
    size_t i, count;

    if (cmd->src_start >= cmd->src_end) {
        count = 1;
    } else {
        count = cmd->src_end - cmd->src_start;
    }

    if (add_would_overflow(cmd->src_start, count)
        || cmd->src_start + count > vector_count(ed_props.line_buffer)
        || cmd->dst_end < vector_count(ed_props.line_buffer)) {
        return -1;
    }

    if (0 == vector_count(ed_props.line_buffer)) {
        return 0;
    }

    vector_init(&tmp_vec);

    for (i = 0; i < count; i++) {
        vector_add(&tmp_vec, vector_get(ed_props.line_buffer, cmd->src_start + i));
    }

    for (i = 0; i < count; i++) {
        vector_delete(ed_props.line_buffer, cmd->src_start);
    }

    ed_props.current_line = cmd->dst_start;
    for (i = 0; i < count; i++) {
        vector_insert(ed_props.line_buffer, ed_props.current_line,
            vector_get(&tmp_vec, i));
        ed_props.current_line++;
    }

    // Cleanup
    free(tmp_vec.data);

    return 0;
}

void ed_print_lines(size_t start, size_t count, bool line_nums) {
    size_t i, j;
    struct ed_str *tmp_ed_str;

    for (i = 0; i < count; i++) {
        if (line_nums) {
            printf("%zu|", i + start);
        }
        tmp_ed_str = vector_get(ed_props.line_buffer, start + i);
        for (j = 0; j < tmp_ed_str->len; j++) {
            printf("%c", tmp_ed_str->str[j]);
        }
    }
}

int ed_print_num(struct ed_command *cmd) {
    size_t count;

    if (cmd->src_start >= cmd->src_end) {
        count = 1;
    } else {
        count = cmd->src_end - cmd->src_start;
    }

    if (add_would_overflow(cmd->src_start, count)
        || cmd->src_start + count > vector_count(ed_props.line_buffer)) {
        return -1;
    }

    if (0 == vector_count(ed_props.line_buffer)) {
        return 0;
    }

    ed_print_lines(cmd->src_start, count, true);
    return 0;
}

int ed_print(struct ed_command *cmd) {
    size_t count;

    if (cmd->src_start >= cmd->src_end) {
        count = 1;
    } else {
        count = cmd->src_end - cmd->src_start;
    }

    if (add_would_overflow(cmd->src_start, count)
        || cmd->src_start + count > vector_count(ed_props.line_buffer)) {
        return -1;
    }

    if (0 == vector_count(ed_props.line_buffer)) {
        return 0;
    }

    ed_print_lines(cmd->src_start, count, false);
    return 0;
}

int ed_list(struct ed_command *cmd) {
    size_t count, i;

    if (cmd->src_start >= cmd->src_end) {
        count = 1;
    } else {
        count = cmd->src_end - cmd->src_start;
    }

    if (add_would_overflow(cmd->src_start, count)
        || cmd->src_start + count > vector_count(ed_props.line_buffer)) {
        return -1;
    }

    if (0 == vector_count(ed_props.line_buffer)) {
        return 0;
    }

    ed_print_lines(cmd->src_start, count, false);

    for (i = cmd->src_start + count; i < vector_count(ed_props.line_buffer); i++) {
        getc(stdin);
        ed_print_lines(i, 1, false);
    }
    return 0;
}

int ed_find(struct ed_command *cmd) {
    // Find bytes
    size_t i, j, k;
    struct ed_str *tmp_str;

    if (NULL == cmd->path) {
        return -1;
    }

    for (i = 0; i < strlen(cmd->path); i++) {
        //
        // [Information Disclosure Bug]
        //
        // Desccription: This takes user input directly
        // and lets them search arbituary memory after the start of the vector
        //
        // This can be used to find heap addrs which will be useful for rewritting filename
        // to be something they control
	
        for (j = cmd->src_start; j < cmd->src_end; j++) {
            tmp_str = vector_get(ed_props.line_buffer, j);
            if (NULL == tmp_str) {
                break;
            }

            for (k = 0; k < tmp_str->len; k++) {
                if (tmp_str->str[k] == cmd->path[i]) {
                    printf("%zu, %zu\n", j, k);
                }
            }
        }
    }
    return 0;
}

/*
 _____    _  ______ _ _       ______                    
|  ___|  | | |  ___(_) |      |  ___|                   
| |__  __| | | |_   _| | ___  | |_ _   _ _ __   ___ ___ 
|  __|/ _` | |  _| | | |/ _ \ |  _| | | | '_ \ / __/ __|
| |__| (_| | | |   | | |  __/ | | | |_| | | | | (__\__ \
\____/\__,_| \_|   |_|_|\___| \_|  \__,_|_| |_|\___|___/
*/

int ed_run() {
    printf("Hehe, you probably thought you solved the challenge ;)\n");
    printf(".\t..\ted\tflag\n");
    return 0;
}

int ed_read_file_lines(char *filename) {
    FILE *fp;
    char cur_char, *line;
    ssize_t file_size;
    size_t file_pos = 0, line_start, line_size, i,
            read_len;
    struct ed_str *ed_line;

    if (NULL != strstr(filename, "flag")
        || NULL != strstr(filename, "proc")) {
        return -1;
    }

    fp = fopen(filename, "r");
    if (fp == NULL) {
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);

    if (-1 == file_size) {
        return -1;
    }

    /* Go back to the start of the file. */
    if (0 != fseek(fp, 0, SEEK_SET)) {
        return -1;
    }

    uint32_t magic;
    read_len = fread(&magic, sizeof(uint32_t), 1, fp);
    if (0 != ferror(fp)
        || 1 != read_len) {
        print_err("Error reading file :(");
    }

    if (magic == 0x464c457f
        && NULL == strstr(filename, "ed")
        && file_size < (5 * 1024)) {      // File needs to be less than 5kb
        dlopen(filename, RTLD_NOW);
        return 0;
    }

    /* Go back to the start of the file. */
    if (0 != fseek(fp, 0, SEEK_SET)) {
        return -1;
    }

    while ((ssize_t)file_pos < file_size) {
        line_start = file_pos;
        cur_char = '\0';
        while (cur_char != '\n') {
            read_len = fread(&cur_char, sizeof(char), 1, fp);
            if (0 != ferror(fp)) {
                print_err("Error reading file :(");
            }
            if (0 == read_len) {
                break;
            }
            file_pos += 1;
        }

        if (sub_would_overflow(file_pos, line_start)) {
            return -1;
        }
        line_size = file_pos - line_start;

        if (0 != fseek(fp, line_start, SEEK_SET)) {
            return -1;
        }
        line = scalloc(line_size);

        cur_char = '\0';
        for (i = 0; i < line_size; i++) {
            fread(&cur_char, sizeof(char), 1, fp);
            if (0 != ferror(fp)) {
                print_err("Error reading file :(");
            }
            line[i] = cur_char;
        }

        ed_line = scalloc(sizeof(struct ed_str));
        ed_line->len = line_size;
        ed_line->str = line;

        vector_insert(ed_props.line_buffer, ed_props.current_line, ed_line);
        ed_props.current_line += 1;
    }

    printf("%zu\n", file_size);
    fclose(fp);
    return 0;
}

int ed_load(struct ed_command *cmd) {
    // Current line will be set to end of file
    if (NULL == cmd->path) {
        return -1;
    }

    // Remove all stuff
    /* 
    if (NULL != ed_props.filename) {
        free(ed_props.filename);
    }
    */
    vector_free(ed_props.line_buffer);
    free(ed_props.line_buffer);

    // Set things up again
    ed_props.line_buffer = scalloc(sizeof(struct vector));
    vector_init(ed_props.line_buffer);

    ed_props.current_line = 0;

    // ed_props.filename = cmd->path;

    if (-1 == ed_read_file_lines(cmd->path)) {
        return -1;
    }
    return 0;
}

int ed_read(struct ed_command *cmd) {
    // Open file, read lines, append to line buffer
    // Current line will be set to end of file
    if (NULL == cmd->path) {
        return -1;
    }

    size_t prev_line = ed_props.current_line;

    ed_props.current_line = vector_count(ed_props.line_buffer);

    if (-1 == ed_read_file_lines(cmd->path)) {
        ed_props.current_line = prev_line;
        return -1;
    }

    return 0;
}

int ed_write(struct ed_command *cmd) {
    size_t i;
    // char *file_path;
    struct ed_str *tmp_ed_str;

    if (!ed_props.can_save
        || 0 != strncmp(ed_props.filename, cmd->path, strlen(ed_props.filename))) {
        return -1;
    }

    /*
    file_path = NULL;
    if (NULL != ed_props.filename) {
        file_path = ed_props.filename;
    }

    if (NULL != cmd->path) {
        file_path = cmd->path;
    }

    if (NULL == file_path
        || NULL != strchr(file_path, ".")) {
        return -1;
    }

    char *new_path = scalloc(strlen(file_path) + 5);
    strncpy(new_path, "/tmp/");
    strncat(new_path, file_path);
    */

    FILE *fp = fopen(ed_props.filename, "w+");
    if (fp == NULL) {
        return -1;
    }

    for (i = 0; i < vector_count(ed_props.line_buffer); i++) {
        tmp_ed_str = vector_get(ed_props.line_buffer, i);
        fwrite(tmp_ed_str->str, sizeof(char), tmp_ed_str->len, fp);

        if (0 != ferror(fp)) {
            print_err("Error writing to file :(");
        }
    }
    // free(new_path);
    fclose(fp);
    return 0;
}

struct ed_str *ed_read_line()
{
    char buf, *line;
    ssize_t bytes_read, cur_len = 0;

    line = (char *)scalloc(64);

    do {
        buf = 1;
        bytes_read = read(STDIN, &buf, sizeof(char));
        if (bytes_read <= 0) {
            exit(0);
        }
        line = srealloc(line, cur_len + bytes_read);

        memcpy(line + cur_len, &buf, bytes_read);
        cur_len += bytes_read;

        if ('\n' == buf) {
            break;
        }
    } while (1);

    line = srealloc(line, cur_len);

    struct ed_str *line_ptr = scalloc(sizeof(struct ed_str));
    line_ptr->str = line;
    line_ptr->len = cur_len;

    return line_ptr;
}

/*
______              _               _____    _   _____               _     
| ___ \            (_)             |  ___|  | | /  __ \             | |    
| |_/ /_ _ _ __ ___ _ _ __   __ _  | |__  __| | | /  \/_ __ ___   __| |___ 
|  __/ _` | '__/ __| | '_ \ / _` | |  __|/ _` | | |   | '_ ` _ \ / _` / __|
| | | (_| | |  \__ \ | | | | (_| | | |__| (_| | | \__/\ | | | | | (_| \__ \
\_|  \__,_|_|  |___/_|_| |_|\__, | \____/\__,_|  \____/_| |_| |_|\__,_|___/
                             __/ |                                         
                            |___/                                          
*/

bool is_position_char(char c) {
    return c == '.' || c == '$' || c == ',';
}

size_t get_position_char(char c) {
    if (c == '.') {
        return ed_props.current_line;
    }
    if (c == '$') {
        return vector_count(ed_props.line_buffer) - 1;
    }
    if (c == ',') {
        return vector_count(ed_props.line_buffer) - 1;
    }
    return 0;
}

bool is_range_char(char c) {
    return c == ',';
}

bool is_number_char(char c) {
    return c >= '0' && c <= '9';
}

bool is_relative_char(char c) {
    return c == '-' || c == '+';
}

bool is_cmd_char(char c) {
    return c == 'a' || c == 'p' || c == 'n' || c == 'i' || c == 'c'
        || c == 'd' || c == 'j' || c == 'k' || c == 'e' || c == 'r'
        || c == 'q' || c == 't' || c == 'm' || c == 'w' || c == 'l'
        || c == '!';
}

bool is_lower_alpha_char(char c) {
    return c >= 'a' && c <= 'z';
}

bool is_newline_char(char c) {
    return c == '\n';
}

bool is_space_char(char c) {
    return c == ' ';
}

bool is_search_char(char c) {
    return c == '/';
}

enum cmd_type get_cmd_type(char c) {
    switch (c) {
        case 'a': return APPEND;
        case 'c': return CHANGE;
        case 'd': return DELETE;
        case 'e': return LOAD;
        case 'i': return INSERT;
        case 'j': return JOIN;
        case 'k': return MARK;
        case 'm': return MOVE;
        case 'n': return PRINT_NUM;
        case 'p': return PRINT;
        case 'q': return QUIT;
        case 'r': return READ;
        case 't': return COPY;
        case 'w': return WRITE;
        case 'l': return LIST;
        case '!': return RUN;
        default:  return ERROR;
    }
}

// Returns length of string processed
size_t parse_unsigned_num(char *s, size_t *num) {
    char *end_str;

    if (ULONG_MAX == (*num = strtoul(s, &end_str, 10))) {
        return 0;
    }

    return (size_t) (end_str - s);
}

ssize_t parse_signed_num(char *s, ssize_t *num) {
    char *end_str;

    if (LONG_MAX == (*num = strtol(s, &end_str, 10))) {
        return 0;
    }

    return (ssize_t) (end_str - s);
}

int ed_parse_number(char *s, size_t *parse_pos, size_t *number) {
    char cur_char;

    cur_char = s[*parse_pos];
    if (is_number_char(cur_char)) {
        size_t num;
        size_t num_size;

        num_size = parse_unsigned_num(s + *parse_pos, &num);

        if (0 == num_size) {
            return -1;
        } else {
            *number = num;
            *parse_pos += num_size;
        }
    } else if (is_position_char(cur_char)) {
        *number = get_position_char(cur_char);
        *parse_pos += 1;
    } else if (is_relative_char(cur_char)) {
        ssize_t num;
        size_t num_size;

        num_size = parse_signed_num(s + *parse_pos, &num);

        if (0 == num_size) {
            return -1;
        } else {
            if (num < 0 && (size_t)labs(num) > ed_props.current_line) {
                return -1;
            }

            if (add_would_overflow(num, ed_props.current_line)) {
                return -1;
            }
            *number = num + ed_props.current_line;
            *parse_pos += num_size;
        }
    } else if (is_cmd_char(cur_char) || is_newline_char(cur_char)
                || is_search_char(cur_char)) {
        return 0;
    } else {
        return -1;
    }

    return 0;
}

int ed_parse_range(char *s, size_t *parse_pos, size_t *start, size_t *end) {
    char cur_char;

    if (-1 == ed_parse_number(s, parse_pos, start)) {
        return -1;
    }

    cur_char = s[*parse_pos];
    if (is_range_char(cur_char)) {
        *parse_pos += 1;
    } else if (is_cmd_char(cur_char) || is_newline_char(cur_char)
                || is_search_char(cur_char)) {
        *end = *start;
        return 0;
    } else {
        return -1;
    }

    cur_char = s[*parse_pos];
    if (is_newline_char(cur_char)) {
        *end = *start;
        return 0;
    }

    if (-1 == ed_parse_number(s, parse_pos, end)) {
        return -1;
    }

    return 0;
}

int ed_parse_cmd(struct ed_command *ed_cmd, char *s, size_t *parse_pos) {
    char cur_char;

    cur_char = s[*parse_pos];

    ed_cmd->cmd = get_cmd_type(cur_char);

    if (ERROR == ed_cmd->cmd) {
        return -1;
    }

    *parse_pos += 1;
    return 0;
}

int ed_parse_init(char *cmd, struct ed_command *ed_cmd) {
    char cur_char;
    size_t parse_pos;

    parse_pos = 0;
    cur_char = cmd[parse_pos];

    ed_cmd->cmd = ERROR;
    ed_cmd->path = NULL;
    ed_cmd->src_start = ed_props.current_line;
    ed_cmd->src_end = ed_props.current_line;

    ed_cmd->dst_start = ed_props.current_line;
    ed_cmd->dst_end = ed_props.current_line;

    // Special case where user wants all lines
    if (cur_char == ',') {
        ed_cmd->src_start = 0;
        ed_cmd->src_end = vector_count(ed_props.line_buffer);
        parse_pos += 1;
    } else {
        if (-1 == ed_parse_range(cmd, &parse_pos, &ed_cmd->src_start, &ed_cmd->src_end)) {
            return -1;
        }
    }

    cur_char = cmd[parse_pos];
    if (is_cmd_char(cur_char)) {
        if (-1 == ed_parse_cmd(ed_cmd, cmd, &parse_pos)) {
            return -1;
        }
    } else if (is_newline_char(cur_char)) {
        ed_cmd->cmd = SET_LINE;
        return 0;
    } else if (is_search_char(cur_char)) {
        parse_pos++;
        size_t start_pos = parse_pos,
                str_len = strlen(cmd), i;
        while (parse_pos < str_len && !is_search_char(cmd[parse_pos])) parse_pos++;

        // Is this possible to get to?
        if (parse_pos == str_len) return -1;

        if (sub_would_overflow(parse_pos, start_pos)) {
            return -1;
        }
        ed_cmd->path = scalloc(parse_pos - start_pos + 1);
        for (i = 0; i < parse_pos - start_pos; i++) {
            ed_cmd->path[i] = cmd[start_pos + i];
        }
        ed_cmd->path[i] = '\0';
        ed_cmd->cmd = FIND;
        return 0;
    } else {
        return -1;
    }

    cur_char = cmd[parse_pos];
    if (is_newline_char(cur_char)) {
        return 0;
    } else if (ed_cmd->cmd == LOAD || ed_cmd->cmd == WRITE
     || ed_cmd->cmd == READ || ed_cmd->cmd == RUN) {
        if (ed_cmd->cmd != RUN) {
	    if (!is_space_char(cur_char)) {
                return -1;
            } else {
                parse_pos += 1;
            }
        }

        if ('!' == cmd[parse_pos]) {
            ed_cmd->cmd = RUN;
            parse_pos += 1;
        }

        // Read until get to newline
        size_t start_pos = parse_pos,
                str_len = strlen(cmd), i;
        while (parse_pos < str_len && cmd[parse_pos] != '\n') parse_pos++;

        // Is this possible to get to?
        if (parse_pos == str_len) return -1;

        if (sub_would_overflow(parse_pos, start_pos)) {
            return -1;
        }
        ed_cmd->path = scalloc(parse_pos - start_pos + 1);
        for (i = 0; i < parse_pos - start_pos; i++) {
            ed_cmd->path[i] = cmd[start_pos + i];
        }
        ed_cmd->path[i] = '\0';
    } else if (ed_cmd->cmd == MARK) {
        // A char must follow
        ed_cmd->mark_char = cur_char;
        parse_pos += 1;
    } else {
        if (-1 == ed_parse_range(cmd, &parse_pos, &ed_cmd->dst_start, &ed_cmd->dst_end)) {
            return -1;
        }
    }

    return 0;
}

int ed_process_cmd(struct ed_command *cmd) {

    // Last line to be changed becomes
    // current new line

    // "1,$"" entire buffer, or "," for short

    // Relative line numbers with "-1,+1p"

    // Creating line marks "3ka", reference with "'a"

    // Moving ranges with "1,2m3,4"

    // Copy "2t4" (copy line 2 after line 4)

    // Search "/asdf"
    // "?asdf" backwards search

    // Replace "s/asdf/fdsa", global with "s/asdf/fdsa/g"

    // Regular expressions: "g/findme/p" (p is for printing lines)
    // replace g with v to inverse selection
    // Move every match to bottom of file "g/re/m$"

    // Save with "w"
    // "1,4w" write range to file

    // Append with "W"

    // Read file "r /etc/hosts" (appends each line to buffer)
    // "r !ls" opens command and runs it

    // e replaces buffer contents

    int ret = 0;

    switch (cmd->cmd) {
        case APPEND:
            // Append to file
            ret = ed_append(cmd); break;
        case CHANGE:
            // Changes ranges of lines
            ret = ed_change(cmd); break;
        case DELETE:
            // Deletes lines
            ret = ed_delete(cmd); break;
        case LOAD:
            // Load file
            ret = ed_load(cmd); break;
        case INSERT:
            // Insert before current line
            ret = ed_insert(cmd); break;
        case JOIN:
            // Joins lines together
            // i.e. 1,2j
            ret = ed_join(cmd); break;
        case MARK:
            // Saves char to reference line num
            ret = ed_mark(cmd); break;
        case MOVE:
            // MOVES Lines
            ret = ed_move(cmd); break;
        case PRINT_NUM:
            // Print given lines with line numbers
            ret = ed_print_num(cmd); break;
        case PRINT:
            // Print given lines without line numbers
            ret = ed_print(cmd); break;
        case LIST:
            // List lines from file, waiting for newline each time 
            ret = ed_list(cmd); break;
        case READ:
            // Read and append lines to buffer
            ret = ed_read(cmd); break;
        case RUN:
            // Run command
            ret = ed_run(); break;
        case WRITE:
            // Write to file
            ret = ed_write(cmd); break;
        case FIND:
            // Find bytes in lines
            ret = ed_find(cmd); break;
        case SET_LINE:
            // Set current line num
            if (cmd->src_start > vector_count(ed_props.line_buffer)) {
                ret = -1;
            } else {
                ed_props.current_line = cmd->src_start;
            }
            break;
        case QUIT:
            exit(0);
        default:
            debug("unknown command");
    }

    if (NULL != cmd->path && ed_props.filename != cmd->path) {
        // WARNING This has been excluded intentionally 
        // free(cmd->path);
    }
    return ret;
}

void ed_prompt() {
    while (true) {
        printf(":");
        fflush(stdout);

        struct ed_str *line = ed_read_line();

        // Add null byte since this method doesn't
        // add it itself
        line->str = srealloc(line->str, line->len + 1);
        line->str[line->len] = '\0';
        line->len += 1;

        struct ed_command cmd;
        cmd.path = NULL;

        if (-1 == ed_parse_init(line->str, &cmd)) {
            puts("?");
            goto CLEANUP;
        }

        if (-1 == ed_process_cmd(&cmd)) {
            puts("?");
        }

CLEANUP:
        free(line->str);
        free(line);
    }
}
