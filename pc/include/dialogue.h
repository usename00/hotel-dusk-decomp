/**
 * Hotel Dusk: Room 215 (USA) - PC Port Interactive Dialogue & Script Engine
 */

#ifndef DIALOGUE_H
#define DIALOGUE_H

#include <stdint.h>
#include <stdbool.h>
#include "animation.h"
#include "audio.h"

#define MAX_CHOICES 3
#define MAX_NODES 32

typedef struct {
    char text[64];
    int next_node;
} DialogueChoice;

typedef struct {
    char speaker[32];
    uint32_t name_color;
    char text[256];
    AnimID anim;
    SFX_ID sfx;
    int choice_count;
    DialogueChoice choices[MAX_CHOICES];
    int next_node; // Default next node if no choices
} DialogueNode;

typedef struct {
    int current_node;
    int revealed_chars;
    uint32_t char_timer_ms;
    uint32_t char_speed_ms;
    bool is_finished_revealing;
    bool in_choice_mode;
    int selected_choice;
    int total_nodes;
    DialogueNode nodes[MAX_NODES];
} DialogueState;

void Dialogue_Init(void);
void Dialogue_Update(uint32_t delta_ms);
void Dialogue_Advance(void);
void Dialogue_SelectChoice(int choice_index);
DialogueState* Dialogue_GetState(void);

#endif // DIALOGUE_H
