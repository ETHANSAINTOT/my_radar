/*
** EPITECH PROJECT, 2025
** B-MUL-100:my_radar
** File description:
** my_radar.h
** creator : ethan.saintot@epitech.eu
*/

#ifndef MY_RADAR_H
#define MY_RADAR_H

#include <SFML/Graphics.h>
#include <SFML/System.h>
#include <SFML/Window.h>
#include <SFML/Audio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define AIRCRAFT_SIZE 20
#define EXIT_ERROR 84
#define EXPLOSION_DURATION 0.5f
#define EXPLOSION_SIZE 25

typedef enum entity_type_e {
    AIRCRAFT,
    BOMB
} entity_type_t;

typedef struct aircraft_s {
    sfVector2f departure;
    sfVector2f arrival;
    sfVector2f position;
    sfVector2f direction;
    float speed;
    int delay;
    int active;
    int destroyed;
    int exploding;
    float explosion_time;
    entity_type_t type;
    sfSprite *sprite;
    sfRectangleShape *hitbox;
    struct aircraft_s *next;
} aircraft_t;

typedef struct tower_s {
    sfVector2f position;
    float radius;
    sfSprite *sprite;
    sfCircleShape *area;
    struct tower_s *next;
} tower_t;

typedef struct simulation_s {
    sfRenderWindow *window;
    sfEvent event;
    sfClock *clock;
    sfClock *timer_clock;
    sfClock *fps_clock;
    sfText *timer_text;
    sfText *fps_text;
    sfFont *font;
    sfSprite *background;
    sfSprite *exit_button;
    sfTexture *bg_texture;
    sfTexture *exit_texture;
    sfTexture *plane_texture;
    sfTexture *bomb_texture;
    sfTexture *explosion_texture;
    sfTexture *tower_texture;
    aircraft_t *aircrafts;
    tower_t *towers;
    int show_hitboxes;
    int show_sprites;
    float elapsed_time;
    int active_aircrafts;
    float fps;
} simulation_t;

int run_simulation(const char *script_path);
int parse_script(const char *filepath, simulation_t *sim);
simulation_t *init_simulation(void);
void free_simulation(simulation_t *sim);
void process_events(simulation_t *sim);
void update_simulation(simulation_t *sim, float delta_time);
void render_simulation(simulation_t *sim);
aircraft_t *create_aircraft(sfVector2f departure, sfVector2f arrival, 
                           float speed, int delay, sfTexture *texture, entity_type_t type);
tower_t *create_tower(sfVector2f position, float radius, sfTexture *texture);
void start_explosion(aircraft_t *entity);
int check_collision(aircraft_t *a1, aircraft_t *a2);
int is_in_control_area(aircraft_t *aircraft, tower_t *towers);
int has_reached_destination(aircraft_t *aircraft);
float distance(sfVector2f p1, sfVector2f p2);
sfVector2f normalize(sfVector2f vector);
sfVector2f vector_sub(sfVector2f v1, sfVector2f v2);
sfVector2f vector_mul(sfVector2f v, float scalar);
sfVector2f vector_add(sfVector2f v1, sfVector2f v2);
float vector_length(sfVector2f v);

#endif