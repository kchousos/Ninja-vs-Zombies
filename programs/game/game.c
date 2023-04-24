#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"

#include "interface.h"
#include "state.h"

State state;
KeyState keys;

void update_and_draw() {

	keys->left  = IsKeyDown(KEY_LEFT);
	keys->right = IsKeyDown(KEY_RIGHT);
	keys->up    = IsKeyDown(KEY_UP);
	keys->enter = IsKeyPressed(KEY_ENTER);
	keys->n     = IsKeyDown(KEY_N);
	keys->p     = IsKeyPressed(KEY_P);

  state_update(state, keys);
	interface_draw_frame(state);
}

int main() {
	state = state_create();
	interface_init();

	keys = malloc(sizeof(*keys));
	// Η κλήση αυτή καλεί συνεχόμενα την update_and_draw μέχρι ο χρήστης να κλείσει το παράθυρο
	start_main_loop(update_and_draw);

	interface_close();

	free(keys);

	state_destroy(state);
	
	return 0;
}
