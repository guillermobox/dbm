#include <gdbm.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

struct st_command {
	char *name;
	char *description;
	int (*function)(int, char*[], GDBM_FILE);
};

void error(const char *msg, ...)
{
	va_list args;

	va_start(args, msg);
	vfprintf(stderr, msg, args);
	va_end(args);

	fputc('\n', stderr);
}

int com_set(int argc, char *argv[], GDBM_FILE db)
{
	datum key, value;

	if (argc < 2) {
		error("Please provide the key and value to set as arguments");
		return 1;
	}

	key.dptr = argv[0];
	key.dsize = strlen(argv[0]) + 1;

	value.dptr = argv[1];
	value.dsize = strlen(argv[1]) + 1;

	return gdbm_store(db, key, value, GDBM_REPLACE);
}

int com_remove(int argc, char *argv[], GDBM_FILE db)
{
	datum key;
	int err;

	if (argc < 1) {
		error("Please provide the key to remove");
		return 1;
	}

	key.dptr = argv[0];
	key.dsize = strlen(argv[0]) + 1;

	err = gdbm_delete(db, key);
	if (err) {
		error("Key not found");
	}
	return err;
}

int com_get(int argc, char *argv[], GDBM_FILE db)
{
	datum key, value;

	if (argc < 1) {
		error("Please provide the key to get");
		return 1;
	}

	key.dptr = argv[0];
	key.dsize = strlen(argv[0]) + 1;

	value = gdbm_fetch(db, key);
	if (value.dptr) {
		printf("%s", value.dptr);
		return 0;
	} else {
		error("Key not found");
		return 1;
	}
}

int com_list(int argc, char *argv[], GDBM_FILE db)
{
	datum key, value;

	key = gdbm_firstkey(db);
	if (key.dptr == NULL) {
		error("Empty database");
		return 1;
	}
	while (key.dptr) {
		value = gdbm_fetch(db, key);
		printf("%s\t%s\n", key.dptr, value.dptr);
		key = gdbm_nextkey(db, key);
	}
	return 0;
}

int com_inc(int argc, char *argv[], GDBM_FILE db)
{
	datum key, value;
	unsigned int n;
	char *dptr;

	if (argc < 1) {
		error("Please provide the key to increment");
		return 1;
	}

	key.dptr = argv[0];
	key.dsize = strlen(argv[0]) + 1;

	value = gdbm_fetch(db, key);
	if (value.dptr == NULL) {
		value.dptr = "1";
		value.dsize = 2;
		return gdbm_store(db, key, value, GDBM_REPLACE);
	}

	n = strtol(value.dptr, &dptr, 10);

	if (*value.dptr != '\0' && *dptr != '\0') {
		error("Value is not an integer: %s", value.dptr);
		return 1;
	}

	n = n + 1;

	value.dsize = snprintf(NULL, 0, "%u", n) + 1;
	dptr = malloc(value.dsize);
	snprintf(dptr, value.dsize, "%u", n);
	value.dptr = dptr;

	printf("%s", value.dptr);
	gdbm_store(db, key, value, GDBM_REPLACE);
	free(dptr);
	return 0;
}

struct st_command commands[] = {
	{"get", "Get the literal content of a value pair", com_get},
	{"set", "Set a new key/value pair, replacing any existing value for that key", com_set},
	{"list", "List all the key/value pairs, separated by TAB with extra CR", com_list},
	{"remove", "Remove a given key", com_remove},
	{"inc", "Increment in one unit the value number of a given key", com_inc},
	{NULL, NULL, NULL},
};

int main(int argc, char *argv[])
{
	struct st_command * command;
	const char * cmdname;
	char * dbpath;
	GDBM_FILE db;
	int err;

	cmdname = argv[1];
	argv += 2;
	argc -= 2;

	if (cmdname == NULL) {
		fprintf(stderr, "Usage: dbm <command> [key] [value]\n\nWhere command can be:\n\n");
		for (command = commands; command->name != NULL; command++)
			fprintf(stderr, "%10s  %s\n", command->name, command->description);
		fprintf(stderr,"\n");
		return (EXIT_FAILURE);
	}

	for (command = commands; command->name != NULL; command++)
		if (strcmp(command->name, cmdname) == 0)
			break;

	if (command->name == NULL) {
		error("Unknown command: %s", cmdname);
		return (EXIT_FAILURE);
	}

	dbpath = getenv("DBM_PATH");
	if (dbpath == NULL) {
		error("Please provide a database path in environment variable DBM_PATH");
		return (EXIT_FAILURE);
	}

	db = gdbm_open(dbpath, 512, GDBM_WRCREAT, 0777, NULL);
	if (db == NULL) {
		error("Imposible to open database");
		return (EXIT_FAILURE);
	}

	err = command->function(argc, argv, db);
	gdbm_close(db);

	return err;
}
