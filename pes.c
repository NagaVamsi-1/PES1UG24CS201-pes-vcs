#include "pes.h"
#include "index.h"
#include "commit.h"
#include "tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// ─── INIT ─────────────────────────────────────────────

void cmd_init(void) {
    mkdir(".pes", 0777);
    mkdir(".pes/objects", 0777);
    mkdir(".pes/refs", 0777);
    mkdir(".pes/refs/heads", 0777);

    FILE *f = fopen(".pes/HEAD", "w");
    fprintf(f, "ref: refs/heads/main\n");
    fclose(f);

    printf("Initialized empty PES repository in .pes/\n");
}

// ─── ADD ─────────────────────────────────────────────

void cmd_add(int argc, char *argv[]) {
    if (argc < 3) {
        printf("error: no files specified\n");
        return;
    }

    Index idx;
    if (index_load(&idx) != 0) {
        printf("error: failed to load index\n");
        return;
    }

    for (int i = 2; i < argc; i++) {
        if (index_add(&idx, argv[i]) != 0) {
            printf("error: failed to add %s\n", argv[i]);
            return;
        }
    }
}

// ─── STATUS ──────────────────────────────────────────

void cmd_status(void) {
    Index idx;

    if (index_load(&idx) != 0) {
        printf("error: failed to load index\n");
        return;
    }

    index_status(&idx);
}

// ─── COMMIT ──────────────────────────────────────────

void cmd_commit(int argc, char *argv[]) {
    if (argc < 4 || strcmp(argv[2], "-m") != 0) {
        printf("Usage: pes commit -m <message>\n");
        return;
    }

    ObjectID id;

    if (commit_create(argv[3], &id) != 0) {
        printf("error: commit failed\n");
        return;
    }

    char hex[HASH_HEX_SIZE + 1];
    hash_to_hex(&id, hex);

    printf("Committed as %s\n", hex);
}

// ─── LOG ─────────────────────────────────────────────

void print_commit(const ObjectID *id, const Commit *c, void *ctx) {
    (void)ctx;

    char hex[HASH_HEX_SIZE + 1];
    hash_to_hex(id, hex);

    printf("commit %s\n", hex);
    printf("Author: %s\n", c->author);
    printf("Message: %s\n\n", c->message);
}

void cmd_log(void) {
    if (commit_walk(print_commit, NULL) != 0) {
        printf("No commits yet\n");
    }
}

// ─── CAT-FILE ────────────────────────────────────────

void cmd_cat_file(const char *hash_str) {
    ObjectID id;

    if (hex_to_hash(hash_str, &id) != 0) {
        printf("Invalid hash\n");
        return;
    }

    ObjectType type;
    void *data;
    size_t len;

    if (object_read(&id, &type, &data, &len) != 0) {
        printf("Object not found\n");
        return;
    }

    fwrite(data, 1, len, stdout);
    printf("\n");

    free(data);
}

// ─── MAIN ────────────────────────────────────────────

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: pes <command>\n");
        return 1;
    }

    const char *cmd = argv[1];

    if      (strcmp(cmd, "init") == 0)     cmd_init();
    else if (strcmp(cmd, "add") == 0)      cmd_add(argc, argv);
    else if (strcmp(cmd, "status") == 0)   cmd_status();
    else if (strcmp(cmd, "commit") == 0)   cmd_commit(argc, argv);
    else if (strcmp(cmd, "log") == 0)      cmd_log();
    else if (strcmp(cmd, "cat-file") == 0) {
        if (argc < 3) {
            printf("Usage: pes cat-file <hash>\n");
            return 1;
        }
        cmd_cat_file(argv[2]);
    }
    else {
        printf("Unknown command\n");
        return 1;
    }

    return 0;
}
