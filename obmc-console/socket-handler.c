/**
 * Copyright © 2016 IBM Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <assert.h>
#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <endian.h>

#include <sys/socket.h>
#include <sys/un.h>

#include "console-server.h"

struct client {
	struct poller	*poller;
	int		fd;
};

struct socket_handler {
	struct handler	handler;
	struct console	*console;
	struct poller	*poller;
	int		sd;

	struct client	**clients;
	int		n_clients;
};

static struct socket_handler *to_socket_handler(struct handler *handler)
{
	return container_of(handler, struct socket_handler, handler);
}

static void client_close(struct socket_handler *sh, struct client *client)
{
	int idx;

	close(client->fd);
	if (client->poller)
		console_unregister_poller(sh->console, client->poller);

	for (idx = 0; idx < sh->n_clients; idx++)
		if (sh->clients[idx] == client)
			break;

	assert(idx < sh->n_clients);

	free(client);
	client = NULL;

	sh->n_clients--;
	memmove(&sh->clients[idx], &sh->clients[idx+1],
			sizeof(*sh->clients) * (sh->n_clients - idx));
	sh->clients = realloc(sh->clients,
			sizeof(*sh->clients) * sh->n_clients);
}

static enum poller_ret client_poll(struct handler *handler,
		int events, void *data)
{
	struct socket_handler *sh = to_socket_handler(handler);
	struct client *client = data;
	uint8_t buf[4096];
	int rc;

	if (!(events & POLLIN))
		return POLLER_OK;

	rc = read(client->fd, buf, sizeof(buf));
	if (rc <= 0) {
		client->poller = NULL;
		client_close(sh, client);
		return POLLER_REMOVE;
	}

	console_data_out(sh->console, buf, rc);

	return POLLER_OK;
}

static void client_send_data(struct socket_handler *sh,
		struct client *client, uint8_t *buf, size_t len)
{
	int rc;

	rc = write_buf_to_fd(client->fd, buf, len);
	if (rc)
		client_close(sh, client);
}

static enum poller_ret socket_poll(struct handler *handler,
		int events, void __attribute__((unused)) *data)
{
	struct socket_handler *sh = to_socket_handler(handler);
	struct client *client;
	int fd, n;

	if (!(events & POLLIN))
		return POLLER_OK;

	fd = accept(sh->sd, NULL, NULL);
	if (fd < 0)
		return POLLER_OK;

	client = malloc(sizeof(*client));
	memset(client, 0, sizeof(*client));

	client->fd = fd;
	client->poller = console_register_poller(sh->console, handler,
			client_poll, client->fd, POLLIN, client);

	n = sh->n_clients++;
	sh->clients = realloc(sh->clients,
			sizeof(*sh->clients) * sh->n_clients);
	sh->clients[n] = client;

	return POLLER_OK;

}

static int socket_init(struct handler *handler, struct console *console,
		struct config *config __attribute__((unused)))
{
	struct socket_handler *sh = to_socket_handler(handler);
	struct sockaddr_un addr;
	int rc;

	sh->console = console;
	sh->clients = NULL;
	sh->n_clients = 0;

	sh->sd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(sh->sd < 0) {
		warn("Can't create socket");
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	memcpy(&addr.sun_path, &console_socket_path, console_socket_path_len);

	rc = bind(sh->sd, (struct sockaddr *)&addr, sizeof(addr));
	if (rc) {
		warn("Can't bind to socket path %s",
				console_socket_path_readable);
		return -1;
	}

	rc = listen(sh->sd, 1);
	if (rc) {
		warn("Can't listen for incoming connections");
		return -1;
	}

	sh->poller = console_register_poller(console, handler, socket_poll,
			sh->sd, POLLIN, NULL);

	return 0;
}

static int socket_data(struct handler *handler, uint8_t *buf, size_t len)
{
	struct socket_handler *sh = to_socket_handler(handler);
	int i;

	for (i = 0; i < sh->n_clients; i++) {
		struct client *client = sh->clients[i];
		client_send_data(sh, client, buf, len);
	}
	return 0;
}

static void socket_fini(struct handler *handler)
{
	struct socket_handler *sh = to_socket_handler(handler);
	int i;

	for (i = 0; i < sh->n_clients; i++)
		client_close(sh, sh->clients[i]);

	if (sh->poller)
		console_unregister_poller(sh->console, sh->poller);

	close(sh->sd);
}

static struct socket_handler socket_handler = {
	.handler = {
		.name		= "socket",
		.init		= socket_init,
		.data_in	= socket_data,
		.fini		= socket_fini,
	},
};

console_register_handler(&socket_handler.handler);

