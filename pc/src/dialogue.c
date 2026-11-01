/**
 * Hotel Dusk: Room 215 (USA) - PC Port Interactive Dialogue & Script Engine
 */

#include "dialogue.h"
#include <string.h>
#include <stdio.h>

static DialogueState s_State;

void Dialogue_Init(void) {
    memset(&s_State, 0, sizeof(DialogueState));
    s_State.current_node = 0;
    s_State.char_speed_ms = 22; // Quick, typewriter cadence
    s_State.revealed_chars = 0;
    s_State.char_timer_ms = 0;
    s_State.in_choice_mode = false;
    s_State.selected_choice = 0;

    // Node 0: Ed's call
    DialogueNode *n0 = &s_State.nodes[0];
    strcpy(n0->speaker, "EDWARD");
    n0->name_color = 0xFFFFAA00;
    strcpy(n0->text, "Kyle, you there? It's Ed. Get your butt down to Hotel Dusk on Route 66. We got a client looking for something that doesn't want to be found.");
    n0->anim = ANIM_ED_PHONE;
    n0->sfx = SFX_DIALOGUE_NEXT;
    n0->choice_count = 0;
    n0->next_node = 1;

    // Node 1: Kyle's reply
    DialogueNode *n1 = &s_State.nodes[1];
    strcpy(n1->speaker, "KYLE HYDE");
    n1->name_color = 0xFF00E5FF;
    strcpy(n1->text, "Hotel Dusk? Never heard of it. What's the job, Ed? And don't give me the runaround like last time.");
    n1->anim = ANIM_KYLE_INSPECTION;
    n1->sfx = SFX_DIALOGUE_NEXT;
    n1->choice_count = 0;
    n1->next_node = 2;

    // Node 2: Ed's briefing (Branching Decision!)
    DialogueNode *n2 = &s_State.nodes[2];
    strcpy(n2->speaker, "EDWARD");
    n2->name_color = 0xFFFFAA00;
    strcpy(n2->text, "Client wants two items: a small red box and an old order sheet. Check into Room 215. Owner's name is Dunning Smith. What do you wanna know first?");
    n2->anim = ANIM_ED_PHONE;
    n2->sfx = SFX_DIALOGUE_NEXT;
    n2->choice_count = 3;
    strcpy(n2->choices[0].text, "1. Ask about Room 215's legend");
    n2->choices[0].next_node = 3;
    strcpy(n2->choices[1].text, "2. Ask about the sealed red box");
    n2->choices[1].next_node = 4;
    strcpy(n2->choices[2].text, "3. Ask about Bradley's connection");
    n2->choices[2].next_node = 5;

    // Node 3: Room 215
    DialogueNode *n3 = &s_State.nodes[3];
    strcpy(n3->speaker, "EDWARD");
    n3->name_color = 0xFFFFAA00;
    strcpy(n3->text, "Dunning claims Room 215 is the 'Wish Room'. Folks say whatever you wish for in that room comes true. Sounds like bunk to me, but keep your ears open.");
    n3->anim = ANIM_ED_PHONE;
    n3->sfx = SFX_DIALOGUE_NEXT;
    n3->choice_count = 0;
    n3->next_node = 6;

    // Node 4: Red Box
    DialogueNode *n4 = &s_State.nodes[4];
    strcpy(n4->speaker, "EDWARD");
    n4->name_color = 0xFFFFAA00;
    strcpy(n4->text, "The red box is sealed tight. Client says it was left behind in Room 215 three years ago. The exact same year Bradley vanished into thin air.");
    n4->anim = ANIM_ED_PHONE;
    n4->sfx = SFX_DIALOGUE_NEXT;
    n4->choice_count = 0;
    n4->next_node = 6;

    // Node 5: Bradley
    DialogueNode *n5 = &s_State.nodes[5];
    strcpy(n5->speaker, "EDWARD");
    n5->name_color = 0xFFFFAA00;
    strcpy(n5->text, "Bradley? Kyle, you can't keep chasing a ghost. You put a bullet in him on the Hudson River... but rumors say Nile's syndicate had ties to that very hotel.");
    n5->anim = ANIM_ED_PHONE;
    n5->sfx = SFX_JINGLE_MYSTERY;
    n5->choice_count = 0;
    n5->next_node = 6;

    // Node 6: Kyle Departure
    DialogueNode *n6 = &s_State.nodes[6];
    strcpy(n6->speaker, "KYLE HYDE");
    n6->name_color = 0xFF00E5FF;
    strcpy(n6->text, "Three years of silence... and now Hotel Dusk. I pack my case, hop in the sedan, and drive straight into the Nevada dusk.");
    n6->anim = ANIM_OPENING_CUTSCENE;
    n6->sfx = SFX_DIALOGUE_NEXT;
    n6->choice_count = 0;
    n6->next_node = 7;

    // Node 7: Mila
    DialogueNode *n7 = &s_State.nodes[7];
    strcpy(n7->speaker, "MILA");
    n7->name_color = 0xFFCCAAFF;
    strcpy(n7->text, "... (She stands quietly by the road, staring into the desert distance. Around her wrist glints Brian Bradley's silver bracelet.)");
    n7->anim = ANIM_MILA_SILENCE;
    n7->sfx = SFX_JINGLE_EVENT;
    n7->choice_count = 0;
    n7->next_node = 8;

    // Node 8: Dunning at front desk
    DialogueNode *n8 = &s_State.nodes[8];
    strcpy(n8->speaker, "DUNNING SMITH");
    n8->name_color = 0xFFFF8844;
    strcpy(n8->text, "Name's Dunning Smith, welcome to Hotel Dusk. You got a reservation, mister? Key to 215 is ready, but stay out of the staff rooms if you know what's good for ya.");
    n8->anim = ANIM_KYLE_INSPECTION;
    n8->sfx = SFX_BELL;
    n8->choice_count = 0;
    n8->next_node = 0; // Loop back for interactive testing

    s_State.total_nodes = 9;
    Anim_SetCurrent(n0->anim);
}

void Dialogue_Update(uint32_t delta_ms) {
    DialogueNode *cur = &s_State.nodes[s_State.current_node];
    size_t full_len = strlen(cur->text);

    if (s_State.revealed_chars < (int)full_len) {
        s_State.char_timer_ms += delta_ms;
        if (s_State.char_timer_ms >= s_State.char_speed_ms) {
            s_State.char_timer_ms -= s_State.char_speed_ms;
            s_State.revealed_chars++;

            // Play typewriter click every 3 characters
            if (s_State.revealed_chars % 3 == 0) {
                Audio_PlaySFX(SFX_TEXT_CLICK);
            }
        }
        s_State.is_finished_revealing = false;
        s_State.in_choice_mode = false;
    } else {
        s_State.is_finished_revealing = true;
        if (cur->choice_count > 0) {
            s_State.in_choice_mode = true;
        }
    }
}

void Dialogue_Advance(void) {
    DialogueNode *cur = &s_State.nodes[s_State.current_node];
    size_t full_len = strlen(cur->text);

    // If still revealing text, instant finish
    if (s_State.revealed_chars < (int)full_len) {
        s_State.revealed_chars = (int)full_len;
        s_State.is_finished_revealing = true;
        if (cur->choice_count > 0) {
            s_State.in_choice_mode = true;
        }
        return;
    }

    // In choice mode, advance is handled by choice selection
    if (s_State.in_choice_mode) return;

    // Advance to next node
    int next = cur->next_node;
    if (next >= 0 && next < s_State.total_nodes) {
        s_State.current_node = next;
        s_State.revealed_chars = 0;
        s_State.char_timer_ms = 0;
        s_State.is_finished_revealing = false;
        s_State.in_choice_mode = false;

        DialogueNode *new_node = &s_State.nodes[next];
        Anim_SetCurrent(new_node->anim);
        if (new_node->sfx != SFX_TEXT_CLICK) {
            Audio_PlaySFX(new_node->sfx);
        }
    }
}

void Dialogue_SelectChoice(int choice_index) {
    DialogueNode *cur = &s_State.nodes[s_State.current_node];
    if (!s_State.in_choice_mode || choice_index < 0 || choice_index >= cur->choice_count) return;

    Audio_PlaySFX(SFX_OK);
    int next = cur->choices[choice_index].next_node;
    if (next >= 0 && next < s_State.total_nodes) {
        s_State.current_node = next;
        s_State.revealed_chars = 0;
        s_State.char_timer_ms = 0;
        s_State.is_finished_revealing = false;
        s_State.in_choice_mode = false;

        DialogueNode *new_node = &s_State.nodes[next];
        Anim_SetCurrent(new_node->anim);
        if (new_node->sfx != SFX_TEXT_CLICK) {
            Audio_PlaySFX(new_node->sfx);
        }
    }
}

DialogueState* Dialogue_GetState(void) {
    return &s_State;
}
