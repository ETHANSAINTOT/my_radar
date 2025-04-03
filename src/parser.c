/*
** EPITECH PROJECT, 2025
** B-MUL-100:my_radar
** File description:
** parser.c
** creator : ethan.saintot@epitech.eu
*/

#include "my_radar.h"

aircraft_t *create_aircraft(sfVector2f departure, sfVector2f arrival, 
                           float speed, int delay, sfTexture *texture, entity_type_t type)
{
    aircraft_t *aircraft = malloc(sizeof(aircraft_t));
    if (!aircraft)
        return NULL;
    
    aircraft->departure = departure;
    aircraft->arrival = arrival;
    aircraft->position = departure;
    aircraft->speed = speed;
    aircraft->delay = delay;
    aircraft->active = 0;
    aircraft->destroyed = 0;
    aircraft->exploding = 0;
    aircraft->explosion_time = 0.0f;
    aircraft->type = type;
    aircraft->next = NULL;
    
    sfVector2f direction = vector_sub(arrival, departure);
    aircraft->direction = normalize(direction);
    
    aircraft->sprite = sfSprite_create();
    sfSprite_setTexture(aircraft->sprite, texture, sfTrue);
    
    sfFloatRect bounds = sfSprite_getLocalBounds(aircraft->sprite);
    sfSprite_setOrigin(aircraft->sprite, (sfVector2f){bounds.width / 2, bounds.height / 2});
    
    sfSprite_setPosition(aircraft->sprite, departure);
    
    float angle = atan2f(direction.y, direction.x) * 180 / M_PI;
    sfSprite_setRotation(aircraft->sprite, angle);
    
    aircraft->hitbox = sfRectangleShape_create();
    sfRectangleShape_setSize(aircraft->hitbox, (sfVector2f){AIRCRAFT_SIZE, AIRCRAFT_SIZE});
    sfRectangleShape_setOrigin(aircraft->hitbox, (sfVector2f){AIRCRAFT_SIZE / 2, AIRCRAFT_SIZE / 2});
    sfRectangleShape_setPosition(aircraft->hitbox, departure);
    sfRectangleShape_setFillColor(aircraft->hitbox, sfTransparent);
    
    if (type == BOMB) {
        sfRectangleShape_setOutlineColor(aircraft->hitbox, sfYellow);
    } else {
        sfRectangleShape_setOutlineColor(aircraft->hitbox, sfRed);
    }
    
    sfRectangleShape_setOutlineThickness(aircraft->hitbox, 1);
    
    return aircraft;
}

tower_t *create_tower(sfVector2f position, float radius, sfTexture *texture)
{
    tower_t *tower = malloc(sizeof(tower_t));
    if (!tower)
        return NULL;
    
    tower->position = position;
    tower->radius = radius;
    tower->next = NULL;
    
    tower->sprite = sfSprite_create();
    sfSprite_setTexture(tower->sprite, texture, sfTrue);
    
    sfFloatRect bounds = sfSprite_getLocalBounds(tower->sprite);
    sfSprite_setOrigin(tower->sprite, (sfVector2f){bounds.width / 2, bounds.height / 2});
    
    sfSprite_setPosition(tower->sprite, position);
    
    tower->area = sfCircleShape_create();
    sfCircleShape_setRadius(tower->area, radius);
    sfCircleShape_setOrigin(tower->area, (sfVector2f){radius, radius});
    sfCircleShape_setPosition(tower->area, position);
    sfCircleShape_setFillColor(tower->area, sfColor_fromRGBA(0, 255, 0, 50));
    sfCircleShape_setOutlineColor(tower->area, sfGreen);
    sfCircleShape_setOutlineThickness(tower->area, 1);
    
    return tower;
}

int parse_script(const char *filepath, simulation_t *sim)
{
    FILE *file = fopen(filepath, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open script file '%s'\n", filepath);
        return 1;
    }
    
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    
    while ((read = getline(&line, &len, file)) != -1) {
        if (read <= 1)
            continue;
        
        char type = line[0];
        
        if (type == 'A' || type == 'B') {
            sfVector2f departure, arrival;
            float speed;
            int delay;
            entity_type_t entity_type = (type == 'A') ? AIRCRAFT : BOMB;
            
            if (sscanf(line, "%*c %f %f %f %f %f %d", 
                       &departure.x, &departure.y, &arrival.x, &arrival.y, 
                       &speed, &delay) != 6) {
                fprintf(stderr, "Error: Invalid entity format in script\n");
                free(line);
                fclose(file);
                return 1;
            }
            
            if (entity_type == BOMB && speed < 10.0f) {
                speed *= 2.0f;
            }
            
            sfTexture *texture = (entity_type == AIRCRAFT) ? 
                                sim->plane_texture : sim->bomb_texture;
            
            aircraft_t *entity = create_aircraft(departure, arrival, speed, delay, 
                                               texture, entity_type);
            if (!entity) {
                fprintf(stderr, "Error: Failed to create entity\n");
                free(line);
                fclose(file);
                return 1;
            }
            
            entity->next = sim->aircrafts;
            sim->aircrafts = entity;
            sim->active_aircrafts++;
            
        } else if (type == 'T') {
            sfVector2f position;
            float radius;
            
            if (sscanf(line, "T %f %f %f", &position.x, &position.y, &radius) != 3) {
                fprintf(stderr, "Error: Invalid tower format in script\n");
                free(line);
                fclose(file);
                return 1;
            }
            
            tower_t *tower = create_tower(position, radius, sim->tower_texture);
            if (!tower) {
                fprintf(stderr, "Error: Failed to create tower\n");
                free(line);
                fclose(file);
                return 1;
            }
            
            tower->next = sim->towers;
            sim->towers = tower;
            
        } else {
            fprintf(stderr, "Error: Unknown entity type '%c' in script\n", type);
            free(line);
            fclose(file);
            return 1;
        }
    }
    
    free(line);
    fclose(file);
    return 0;
}