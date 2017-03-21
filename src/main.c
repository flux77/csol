/* yuk
 * Copyright (c) 2017 Niels Sonnich Poulsen (http://nielssp.dk)
 * Licensed under the MIT license.
 * See the LICENSE file or http://opensource.org/licenses/MIT for more information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "config.h"
#include "rc.h"
#include "theme.h"
#include "game.h"
#include "ui.h"

const char *short_options = "hvlt:Tms:";

const struct option long_options[] = {
  {"help", no_argument, NULL, 'h'},
  {"version", no_argument, NULL, 'v'},
  {"list", no_argument, NULL, 'l'},
  {"theme", required_argument, NULL, 't'},
  {"themes", no_argument, NULL, 'T'},
  {"mono", no_argument, NULL, 'm'},
  {"seed", required_argument, NULL, 's'},
  {0, 0, 0, 0}
};

enum action { PLAY, LIST_GAMES, LIST_THEMES };

void describe_option(const char *short_option, const char *long_option, const char *description) {
  printf("  -%-14s --%-18s %s\n", short_option, long_option, description);
}

char *combine_paths(const char *path1, const char *path2) {
  size_t length1 = strlen(path1);
  size_t length2 = strlen(path2);
  size_t combined_length = length1 + length2 + 2;
  char *combined_path = malloc(combined_length);
  strcpy(combined_path, path1);
  if (path1[length1 - 1] != '/') {
    strcat(combined_path, "/");
  }
  strcat(combined_path, path2);
  return combined_path;

}

FILE *find_csolrc() {
  FILE *f;
  char *config_dir = getenv("XDG_CONFIG_HOME");
  char *config_file = NULL;
  if (config_dir) {
    config_file = combine_paths(config_dir, "csol/csolrc");
  } else {
    config_dir = getenv("HOME");
    if (config_dir) {
      config_file = combine_paths(config_dir, ".config/csol/csolrc");
    }
  }
  if (config_file) {
    f = fopen(config_file, "r");
    free(config_file);
    if (f) {
      return f;
    }
  }
  config_dir = getenv("XDG_CONFIG_DIRS");
  if (config_dir) {
    int i = 0;
    while (1) {
      if (!config_dir[i] || config_dir[i] == ':') {
        char *dir = malloc(i + 1);
        strncpy(dir, config_dir, i);
        dir[i] = '\0';
        config_file = combine_paths(dir, "csol/csolrc");
        f = fopen(config_file, "r");
        free(config_file);
        free(dir);
        if (f) {
          return f;
        }
        if (!config_dir[i]) {
          break;
        }
        config_dir = config_dir + i + 1;
        i = 0;
      } else {
        i++;
      }
    }
  }
  f = fopen("./csolrc", "r");
  return f;
}

int main(int argc, char *argv[]) {
  int opt;
  int option_index = 0;
  int colors = 1;
  unsigned int seed = time(NULL);
  enum action action = PLAY;
  char *game_name = "yukon";
  char *theme_name = "default";
  while ((opt = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
    switch (opt) {
      case 'h':
        printf("usage: %s [options] [game]\n", argv[0]);
        puts("options:");
        describe_option("h", "help", "Show help.");
        describe_option("v", "version", "Show version information.");
        describe_option("l", "list", "List available games.");
        describe_option("t <name>", "theme <name>", "Select a theme.");
        describe_option("T", "themes", "List available themes.");
        describe_option("m", "mono", "Disable colors.");
        describe_option("s <seed>", "seed <seed>", "Select seed.");
        return 0;
      case 'v':
        puts("csol " CSOL_VERSION);
        return 0;
      case 'l':
        action = LIST_GAMES;
        break;
      case 't':
        theme_name = optarg;
        break;
      case 'T':
        action = LIST_THEMES;
        break;
      case 'm':
        colors = 0;
        break;
      case 's':
        seed = atol(optarg);
        break;
    }
  }
  if (optind < argc) {
    game_name = argv[optind];
  }
  FILE *rc_file = find_csolrc();
  int error = 0;
  if (!rc_file) {
    printf("csolrc: %s\n", strerror(errno));
    error = 1;
  } else {
    error = !execute_file(rc_file);
    fclose(rc_file);
  }
  if (error) {
    printf("Errors encountered, press enter to continue\n");
    getchar();
  }
  Theme *theme;
  Game *game;
  switch (action) {
    case LIST_GAMES:
      for (GameList *list = list_games(); list; list = list->next) {
        printf("%s - %s\n", list->game->name, list->game->title);
      }
      break;
    case LIST_THEMES:
      for (ThemeList *list = list_themes(); list; list = list->next) {
        printf("%s - %s\n", list->theme->name, list->theme->title);
      }
      break;
    case PLAY:
      theme = get_theme(theme_name);
      if (!theme) {
        printf("theme not found: %s\n", theme_name);
        return 1;
      }
      game = get_game(game_name);
      if (!game) {
        printf("game not found: %s\n", game_name);
        return 1;
      }
      ui_main(game, theme, colors, seed);
      break;
  }
  return 0;
}
