/*
** EPITECH PROJECT, 2025
** B-MUL-100:my_radar
** File description:
** update_render.c
** creator : ethan.saintot@epitech.eu
*/

#include "my_radar.h"

void update_simulation(simulation_t *sim, float delta_time)
{
    sim->elapsed_time = sfTime_asSeconds(sfClock_getElapsedTime(sim->timer_clock));
    char timer_str[32];
    sprintf(timer_str, "Timer: %.1f s", sim->elapsed_time);
    sfText_setString(sim->timer_text, timer_str);
    
    float fps_time = sfTime_asSeconds(sfClock_getElapsedTime(sim->fps_clock));
    if (fps_time >= 0.5f) {
        sim->fps = 1.0f / delta_time;
        char fps_str[32];
        sprintf(fps_str, "FPS: %.1f", sim->fps);
        sfText_setString(sim->fps_text, fps_str);
        sfClock_restart(sim->fps_clock);
    }
    
    int active_count = 0;
    
    aircraft_t *current = sim->aircrafts;
    while (current) {
        if (current->delay > sim->elapsed_time || current->destroyed) {
            if (current->exploding) {
                current->explosion_time += delta_time;
                if (current->explosion_time >= EXPLOSION_DURATION) {
                    current->exploding = 0;
                    current->destroyed = 1;
                    // Réinitialiser l'échelle pour le prochain usage
                    sfSprite_setScale(current->sprite, (sfVector2f){1.0f, 1.0f});
                }
            }
            
            current = current->next;
            continue;
        }
        
        if (!current->active) {
            current->active = 1;
            current->position = current->departure;
            sfSprite_setPosition(current->sprite, current->position);
            sfRectangleShape_setPosition(current->hitbox, current->position);
        }
        
        if (current->active && !has_reached_destination(current)) {
            sfVector2f movement = vector_mul(current->direction, current->speed * delta_time);
            current->position = vector_add(current->position, movement);
            
            if (current->position.x > WINDOW_WIDTH)
                current->position.x = 0;
            else if (current->position.x < 0)
                current->position.x = WINDOW_WIDTH;
            
            if (current->position.y > WINDOW_HEIGHT)
                current->position.y = 0;
            else if (current->position.y < 0)
                current->position.y = WINDOW_HEIGHT;
            
            sfSprite_setPosition(current->sprite, current->position);
            sfRectangleShape_setPosition(current->hitbox, current->position);
            
            active_count++;
        }
        
        current = current->next;
    }
    
    aircraft_t *a1 = sim->aircrafts;
    while (a1) {
        if (!a1->active || a1->destroyed || a1->exploding) {
            a1 = a1->next;
            continue;
        }
        
        aircraft_t *a2 = a1->next;
        while (a2) {
            if (!a2->active || a2->destroyed || a2->exploding) {
                a2 = a2->next;
                continue;
            }
            
            if (check_collision(a1, a2)) {
                if (a1->type == BOMB || a2->type == BOMB) {
                    start_explosion(a1);
                    start_explosion(a2);
                } else {
                    if (!is_in_control_area(a1, sim->towers) && 
                        !is_in_control_area(a2, sim->towers)) {
                        start_explosion(a1);
                        start_explosion(a2);
                    }
                }
            }
            
            a2 = a2->next;
        }
        
        a1 = a1->next;
    }
    
    sim->active_aircrafts = active_count;
}

void render_simulation(simulation_t *sim)
{
    sfRenderWindow_clear(sim->window, sfBlack);
    
    sfRenderWindow_drawSprite(sim->window, sim->background, NULL);
    
    tower_t *tower = sim->towers;
    while (tower) {
        if (sim->show_hitboxes)
            sfRenderWindow_drawCircleShape(sim->window, tower->area, NULL);
        
        if (sim->show_sprites)
            sfRenderWindow_drawSprite(sim->window, tower->sprite, NULL);
        
        tower = tower->next;
    }
    
    aircraft_t *entity = sim->aircrafts;
    while (entity) {
        if (entity->active) {
            if (entity->exploding) {
                if (sim->show_sprites) {
                    // Utiliser la texture d'explosion et l'agrandir
                    sfSprite_setTexture(entity->sprite, sim->explosion_texture, sfFalse);
                    
                    // Calcul du facteur d'échelle en fonction du temps d'explosion
                    float scale_factor = 1.0f + (entity->explosion_time / EXPLOSION_DURATION);
                    sfSprite_setScale(entity->sprite, (sfVector2f){scale_factor, scale_factor});
                    
                    // Faire "pulser" l'explosion avec une taille qui augmente
                    sfRenderWindow_drawSprite(sim->window, entity->sprite, NULL);
                }
            } else if (!entity->destroyed) {
                if (sim->show_hitboxes)
                    sfRenderWindow_drawRectangleShape(sim->window, entity->hitbox, NULL);
                
                if (sim->show_sprites) {
                    sfTexture* correct_texture = (entity->type == AIRCRAFT) ? 
                                                sim->plane_texture : sim->bomb_texture;
                    sfSprite_setTexture(entity->sprite, correct_texture, sfFalse);
                    sfRenderWindow_drawSprite(sim->window, entity->sprite, NULL);
                }
            }
        }
        
        entity = entity->next;
    }
    
    sfRenderWindow_drawSprite(sim->window, sim->exit_button, NULL);
    sfRenderWindow_drawText(sim->window, sim->timer_text, NULL);
    sfRenderWindow_drawText(sim->window, sim->fps_text, NULL);
    
    sfRenderWindow_display(sim->window);
}

int check_collision(aircraft_t *a1, aircraft_t *a2)
{
    sfFloatRect rect1 = sfRectangleShape_getGlobalBounds(a1->hitbox);
    sfFloatRect rect2 = sfRectangleShape_getGlobalBounds(a2->hitbox);
    
    return sfFloatRect_intersects(&rect1, &rect2, NULL);
}

int is_in_control_area(aircraft_t *aircraft, tower_t *towers)
{
    tower_t *tower = towers;
    sfVector2f aircraft_center = {
        aircraft->position.x + AIRCRAFT_SIZE / 2,
        aircraft->position.y + AIRCRAFT_SIZE / 2
    };
    
    while (tower) {
        float dist = distance(aircraft_center, tower->position);
        if (dist <= tower->radius)
            return 1;
        
        tower = tower->next;
    }
    
    return 0;
}

int has_reached_destination(aircraft_t *aircraft)
{
    float dist = distance(aircraft->position, aircraft->arrival);
    if (dist <= 5.0f) {
        aircraft->destroyed = 1;
        return 1;
    }
    return 0;
}

float distance(sfVector2f p1, sfVector2f p2)
{
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    return sqrtf(dx * dx + dy * dy);
}

sfVector2f normalize(sfVector2f vector)
{
    float length = vector_length(vector);
    if (length < 0.0001f)
        return (sfVector2f){0, 0};
    
    return (sfVector2f){vector.x / length, vector.y / length};
}

sfVector2f vector_sub(sfVector2f v1, sfVector2f v2)
{
    return (sfVector2f){v1.x - v2.x, v1.y - v2.y};
}

sfVector2f vector_mul(sfVector2f v, float scalar)
{
    return (sfVector2f){v.x * scalar, v.y * scalar};
}

sfVector2f vector_add(sfVector2f v1, sfVector2f v2)
{
    return (sfVector2f){v1.x + v2.x, v1.y + v2.y};
}

float vector_length(sfVector2f v)
{
    return sqrtf(v.x * v.x + v.y * v.y);
}