/*
 * BlueALSA - cmd-softvol.c
 * Copyright (c) 2016-2023 Arkadiusz Bokowy
 *
 * This file is a part of bluez-alsa.
 *
 * This project is licensed under the terms of the MIT license.
 *
 */

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <dbus/dbus.h>

#include "cli.h"
#include "shared/dbus-client.h"

static void usage(const char *command) {
	printf("Get or set the SoftVolume property of the given PCM.\n\n");
	cli_print_usage("%s [OPTION]... PCM-PATH [STATE]", command);
	printf("\nOptions:\n"
			"  -h, --help\t\tShow this message and exit\n"
			"\nPositional arguments:\n"
			"  PCM-PATH\tBlueALSA PCM D-Bus object path\n"
			"  STATE\t\tEnable or disable SoftVolume property\n"
	);
}

static int cmd_softvol_func(int argc, char *argv[]) {

	int opt;
	const char *opts = "h";
	const struct option longopts[] = {
		{ "help", no_argument, NULL, 'h' },
		{ 0 },
	};

	opterr = 0;
	while ((opt = getopt_long(argc, argv, opts, longopts, NULL)) != -1)
		switch (opt) {
		case 'h' /* --help */ :
			usage(argv[0]);
			return EXIT_SUCCESS;
		default:
			cmd_print_error("Invalid argument '%s'", argv[optind - 1]);
			return EXIT_FAILURE;
		}

	if (argc - optind < 1) {
		cmd_print_error("Missing BlueALSA PCM path argument");
		return EXIT_FAILURE;
	}
	if (argc - optind > 2) {
		cmd_print_error("Invalid number of arguments");
		return EXIT_FAILURE;
	}

	DBusError err = DBUS_ERROR_INIT;
	const char *path = argv[optind];

	struct ba_pcm pcm;
	if (!cli_get_ba_pcm(path, &pcm, &err)) {
		cmd_print_error("Couldn't get BlueALSA PCM: %s", err.message);
		return EXIT_FAILURE;
	}

	if (argc - optind == 1) {
		cli_print_pcm_soft_volume(&pcm);
		return EXIT_SUCCESS;
	}

	bool state;
	const char *value = argv[optind + 1];
	if (!cli_parse_value_on_off(value, &state)) {
		cmd_print_error("Invalid argument: %s", value);
		return EXIT_FAILURE;
	}

	pcm.soft_volume = state;

	if (!bluealsa_dbus_pcm_update(&config.dbus, &pcm, BLUEALSA_PCM_SOFT_VOLUME, &err)) {
		cmd_print_error("SoftVolume update failed: %s", err.message);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

const struct cli_command cmd_softvol = {
	"soft-volume",
	"Get or set PCM SoftVolume property",
	cmd_softvol_func,
};
