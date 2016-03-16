
#include <err.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>

static const char *config_default_filename = SYSCONFDIR "/openbmc-console.conf";

struct config_item {
	char			*name;
	char			*value;
	struct config_item	*next;
};

struct config {
	struct config_item	*items;
};

const char *config_get_value(struct config *config, const char *name)
{
	struct config_item *item;

	for (item = config->items; item; item = item->next)
		if (!strcasecmp(item->name, name))
			return item->value;

	return NULL;
}

static void config_parse(struct config *config, char *buf)
{
	struct config_item *item;
	char *name, *value;
	char *p, *line;
	int rc;

	for (p = NULL, line = strtok_r(buf, "\n", &p); line;
			line = strtok_r(NULL, "\n", &p)) {

		/* trim leading space */
		for (;*line == ' ' || *line == '\t'; line++)
			;

		/* skip comments */
		if (*line == '#')
			continue;

		name = value = NULL;

		rc = sscanf(line, "%m[^ =] = %ms ", &name, &value);
		if (rc != 2 || !strlen(name) || !strlen(value)) {
			free(name);
			free(value);
			continue;
		}

		/* create a new item and add to our list */
		item = malloc(sizeof(*item));
		item->name = name;
		item->value = value;
		item->next = config->items;
		config->items = item;
	}
}

struct config *config_init(const char *filename)
{
	struct config *config;
	struct stat statbuf;
	size_t len, i;
	char *buf;
	int fd, rc;

	if (!filename)
		filename = config_default_filename;

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		warn("Can't open configuration file %s", filename);
		return NULL;
	}

	rc = fstat(fd, &statbuf);
	if (rc) {
		warn("Can't stat %s", filename);
		goto err_close;
	}

	len = statbuf.st_size;

	buf = malloc(len + 1);
	for (i = 0; i < len;) {
		rc = read(fd, buf + i, len - i);
		if (rc < 0) {
			warn("Can't read configuration file %s", filename);
			goto err_free;

		} else if (!rc) {
			break;
		}
		i += rc;

	}
	buf[len] = '\0';

	close(fd);

	config = malloc(sizeof(*config));
	config->items = NULL;

	config_parse(config, buf);

	free(buf);

	return config;

err_free:
	free(buf);
err_close:
	close(fd);
	return NULL;
}

void config_fini(struct config *config)
{
	struct config_item *item, *next;

	for (item = config->items; item; item = next) {
		next = item->next;
		free(item->name);
		free(item->value);
		free(item);
	}

	free(config);
}
