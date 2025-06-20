/*
** EPITECH PROJECT, 2025
** B-MUL-100:my_radar
** File description:
** simulation.c
** creator : ethan.saintot@epitech.eu
*/

#include "my_radar.h"

int run_simulation(const char *script_path)
{
    simulation_t *sim = init_simulation();
    if (!sim)
        return EXIT_ERROR;

    if (parse_script(script_path, sim) != 0) {
        free_simulation(sim);
        return EXIT_ERROR;
    }

    sfClock_restart(sim->timer_clock);
    
    while (sfRenderWindow_isOpen(sim->window)) {
        float delta_time = sfTime_asSeconds(sfClock_restart(sim->clock));
        
        process_events(sim);
        update_simulation(sim, delta_time);
        render_simulation(sim);
        
        if (sim->active_aircrafts <= 0) {
            sfSleep(sfSeconds(2));
            sfRenderWindow_close(sim->window);
        }
    }
    
    free_simulation(sim);
    return 0;
}

simulation_t *init_simulation(void)
{
    simulation_t *sim = malloc(sizeof(simulation_t));
    if (!sim)
        return NULL;
    
    memset(sim, 0, sizeof(simulation_t));
    
    sfVideoMode mode = {WINDOW_WIDTH, WINDOW_HEIGHT, 32};
    sim->window = sfRenderWindow_create(mode, "My Radar - Air Traffic Control", 
                                        sfClose, NULL);
    if (!sim->window) {
        free(sim);
        return NULL;
    }
    
    sfRenderWindow_setVerticalSyncEnabled(sim->window, sfTrue);
    
    sim->clock = sfClock_create();
    sim->timer_clock = sfClock_create();
    sim->fps_clock = sfClock_create();
    sim->fps = 0.0f;
    
    sim->bg_texture = sfTexture_createFromFile("assets/images/world_map.jpg", NULL);
    sim->exit_texture = sfTexture_createFromFile("assets/images/exit.png", NULL);
    sim->plane_texture = sfTexture_createFromFile("assets/images/plane.png", NULL);
    sim->bomb_texture = sfTexture_createFromFile("assets/images/bomb.png", NULL);
    sim->explosion_texture = sfTexture_createFromFile("assets/images/explosion.png", NULL);
    sim->tower_texture = sfTexture_createFromFile("assets/images/tower.png", NULL);
    
    if (!sim->bg_texture || !sim->exit_texture || !sim->plane_texture || 
        !sim->bomb_texture || !sim->explosion_texture || !sim->tower_texture) {
        fprintf(stderr, "Error: Failed to load textures\n");
        free_simulation(sim);
        return NULL;
    }
    
    sim->background = sfSprite_create();
    sim->exit_button = sfSprite_create();
    
    sfSprite_setTexture(sim->background, sim->bg_texture, sfTrue);
    sfSprite_setTexture(sim->exit_button, sim->exit_texture, sfTrue);
    
    sfSprite_setPosition(sim->exit_button, (sfVector2f){10, 10});
    
    sim->font = sfFont_createFromFile("assets/fonts/nothing.ttf");
    if (!sim->font) {
        fprintf(stderr, "Error: Failed to load font\n");
        free_simulation(sim);
        return NULL;
    }
    
    sim->timer_text = sfText_create();
    sfText_setFont(sim->timer_text, sim->font);
    sfText_setCharacterSize(sim->timer_text, 30);
    sfText_setPosition(sim->timer_text, (sfVector2f){WINDOW_WIDTH - 150, 10});
    sfText_setColor(sim->timer_text, sfWhite);
    
    sim->fps_text = sfText_create();
    sfText_setFont(sim->fps_text, sim->font);
    sfText_setCharacterSize(sim->fps_text, 20);
    sfText_setPosition(sim->fps_text, (sfVector2f){WINDOW_WIDTH - 150, 50});
    sfText_setColor(sim->fps_text, sfWhite);
    
    sim->show_hitboxes = 1;
    sim->show_sprites = 1;
    sim->elapsed_time = 0.0f;
    sim->active_aircrafts = 0;
    
    return sim;
}

void free_simulation(simulation_t *sim)
{
    if (!sim)
        return;
    
    aircraft_t *aircraft = sim->aircrafts;
    while (aircraft) {
        aircraft_t *next = aircraft->next;
        if (aircraft->sprite)
            sfSprite_destroy(aircraft->sprite);
        if (aircraft->hitbox)
            sfRectangleShape_destroy(aircraft->hitbox);
        free(aircraft);
        aircraft = next;
    }
    
    tower_t *tower = sim->towers;
    while (tower) {
        tower_t *next = tower->next;
        if (tower->sprite)
            sfSprite_destroy(tower->sprite);
        if (tower->area)
            sfCircleShape_destroy(tower->area);
        free(tower);
        tower = next;
    }
    
    if (sim->background)
        sfSprite_destroy(sim->background);
    if (sim->exit_button)
        sfSprite_destroy(sim->exit_button);
    
    if (sim->bg_texture)
        sfTexture_destroy(sim->bg_texture);
    if (sim->exit_texture)
        sfTexture_destroy(sim->exit_texture);
    if (sim->plane_texture)
        sfTexture_destroy(sim->plane_texture);
    if (sim->bomb_texture)
        sfTexture_destroy(sim->bomb_texture);
    if (sim->explosion_texture)
        sfTexture_destroy(sim->explosion_texture);
    if (sim->tower_texture)
        sfTexture_destroy(sim->tower_texture);
    
    if (sim->timer_text)
        sfText_destroy(sim->timer_text);
    if (sim->fps_text)
        sfText_destroy(sim->fps_text);
    if (sim->font)
        sfFont_destroy(sim->font);
    
    if (sim->clock)
        sfClock_destroy(sim->clock);
    if (sim->timer_clock)
        sfClock_destroy(sim->timer_clock);
    if (sim->fps_clock)
        sfClock_destroy(sim->fps_clock);
    
    if (sim->window)
        sfRenderWindow_destroy(sim->window);
    
    free(sim);
}

void process_events(simulation_t *sim)
{
    while (sfRenderWindow_pollEvent(sim->window, &sim->event)) {
        if (sim->event.type == sfEvtClosed)
            sfRenderWindow_close(sim->window);
        
        if (sim->event.type == sfEvtKeyPressed) {
            if (sim->event.key.code == sfKeyL)
                sim->show_hitboxes = !sim->show_hitboxes;
            
            if (sim->event.key.code == sfKeyS)
                sim->show_sprites = !sim->show_sprites;
            
            if (sim->event.key.code == sfKeyEscape || sim->event.key.code == sfKeyQ)
                sfRenderWindow_close(sim->window);
        }
        
        if (sim->event.type == sfEvtMouseButtonPressed) {
            sfVector2i mouse_pos = sfMouse_getPositionRenderWindow(sim->window);
            sfFloatRect exit_bounds = sfSprite_getGlobalBounds(sim->exit_button);
            
            if (sfFloatRect_contains(&exit_bounds, mouse_pos.x, mouse_pos.y)) {
                sfRenderWindow_close(sim->window);
            }
        }
    }
}

/*
void start_explosion(aircraft_t *entity)
{
    entity->exploding = 1;
    entity->explosion_time = 0.0f;
    
    // Remettre l'échelle à 1 pour éviter les problèmes avec des explosions successives
    sfSprite_setScale(entity->sprite, (sfVector2f){1.0f, 1.0f});
    
    // Centrer l'explosion sur la position actuelle
    sfFloatRect bounds = sfSprite_getLocalBounds(entity->sprite);
    sfSprite_setOrigin(entity->sprite, (sfVector2f){bounds.width / 2, bounds.height / 2});
}*/