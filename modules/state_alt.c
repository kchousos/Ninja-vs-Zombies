#include <stdlib.h>
#include <time.h>

#include "ADTVector.h"
#include "ADTSet.h"
#include "ADTList.h"
#include "common_types.h"
#include "raylib.h"
#include "state.h"


// Οι ολοκληρωμένες πληροφορίες της κατάστασης του παιχνιδιού.
// Ο τύπος State είναι pointer σε αυτό το struct, αλλά το ίδιο το struct
// δεν είναι ορατό στον χρήστη.

struct state {
	Vector objects;			// περιέχει στοιχεία Object (Εμπόδια / Εχθροί / Πύλες)
	List portal_pairs;		// περιέχει PortaPair (ζευγάρια πυλών, είσοδος/έξοδος)
	Set objects_set;

	struct state_info info;
};

// Ζευγάρια πυλών

typedef struct portal_pair {
	Object entrance;		// η πύλη entrance
	Object exit;			// οδηγεί στην exit
}* PortalPair;


int compare (Pointer a, Pointer b) {

	Object obj1 = a, obj2 = b;
	if (obj1->rect.x - obj2->rect.x != 0)
		return (int)obj1->rect.x - (int)obj2->rect.x; 
	else
		return (int)(&a - &b);
}


// Δημιουργεί και επιστρέφει την αρχική κατάσταση του παιχνιδιού

State state_create() {
	// Δημιουργία του state
	State state = malloc(sizeof(*state));

	// Γενικές πληροφορίες
	state->info.current_portal = 0;			// Δεν έχουμε περάσει καμία πύλη
	state->info.wins = 0;					// Δεν έχουμε νίκες ακόμα
	state->info.playing = true;				// Το παιχνίδι ξεκινάει αμέσως
	state->info.paused = false;				// Χωρίς να είναι paused.

	// Πληροφορίες για το χαρακτήρα.
	Object character = state->info.character = malloc(sizeof(*character));
	character->type = CHARACTER;
	character->forward = true;
	character->jumping = false;

	// Ο χαρακτήρας (όπως και όλα τα αντικείμενα) έχουν συντεταγμένες x,y σε ένα
	// καρτεσιανό επίπεδο.
	// - Στο άξονα x το 0 είναι η αρχή στης πίστας και οι συντεταγμένες
	//   μεγαλώνουν προς τα δεξιά.
	// - Στον άξονα y το 0 είναι το "δάπεδο" της πίστας, και οι
	//   συντεταγμένες μεγαλώνουν προς τα _κάτω_.
	// Πέρα από τις συντεταγμένες, αποθηκεύουμε και τις διαστάσεις width,height
	// κάθε αντικειμένου. Τα x,y,width,height ορίζουν ένα παραλληλόγραμμο, οπότε
	// μπορούν να αποθηκευτούν όλα μαζί στο obj->rect τύπου Rectangle (ορίζεται
	// στο include/raylib.h).
	//
	// Προσοχή: τα x,y αναφέρονται στην πάνω-αριστερά γωνία του Rectangle, και
	// τα y μεγαλώνουν προς τα κάτω, οπότε πχ ο χαρακτήρας που έχει height=38,
	// αν θέλουμε να "κάθεται" πάνω στο δάπεδο, θα πρέπει να έχει y=-38.

	character->rect.width = 100;
	character->rect.height = 200;
	character->rect.x = 0.0;
	character->rect.y = - character->rect.height;

	// Δημιουργία των objects (πύλες / εμπόδια / εχθροί) και προσθήκη στο vector
	// state->objects. Η πίστα περιέχει συνολικά 4*PORTAL_NUM αντικείμενα, από
	// τα οποία τα PORTAL_NUM είναι πύλες, και τα υπόλοια εμπόδια και εχθροί.

	state->objects = vector_create(0, free);		// Δημιουργία του vector
	state->objects_set = set_create(compare, free);

	for (int i = 0; i < 4*PORTAL_NUM; i++) {
		// Δημιουργία του Object και προσθήκη στο vector
		Object obj = malloc(sizeof(*obj));
		vector_insert_last(state->objects, obj);

		// Κάθε 4 αντικείμενα υπάρχει μια πύλη. Τα υπόλοιπα αντικείμενα
		// επιλέγονται τυχαία.
		if (i == 0) {
		obj->type = OBSTACLE;					
		obj->rect.width = 140;
		obj->rect.height = 180;
		}
		else if(i % 4 == 3) {							// Το 4ο, 8ο, 12ο κλπ αντικείμενο
			obj->type = PORTAL;						// είναι πύλη.
			obj->rect.width = 200;
			obj->rect.height = 10;

		} else if(rand() % 2 == 0) {				// Για τα υπόλοιπα, με πιθανότητα 50%
			obj->type = OBSTACLE;					// επιλέγουμε εμπόδιο.
			obj->rect.width = 140;
			obj->rect.height = 180;

		} else {
			obj->type = ENEMY;						// Και τα υπόλοιπα είναι εχθροί.
			obj->rect.width = 120;
			obj->rect.height = 200;
			obj->forward = false;					// Οι εχθροί αρχικά κινούνται προς τα αριστερά.
		}

		// Τα αντικείμενα είναι ομοιόμορφα τοποθετημένα σε απόσταση SPACING
		// μεταξύ τους, και "κάθονται" πάνω στο δάπεδο.

		obj->rect.x = (i+1) * SPACING;
		obj->rect.y = - obj->rect.height;


		set_insert(state->objects_set, vector_get_at(state->objects, i));
	}

	// Δημιουργία της λίστας portal_pairs
	state->portal_pairs = list_create(free);

	srand((unsigned int) time(NULL));

	// Αρχικοποίηση του πίνακα που μας δείχνει αν
	// μια πύλη είναι ήδη exit για κάποια άλλη ή όχι
	int taken_exit[PORTAL_NUM];
	for (int i = 0; i < PORTAL_NUM; i++) {
		taken_exit[i] = 0;
	}

	// Για κάθε portal
	for (int i = 3; i < vector_size(state->objects); i += 4) {

		PortalPair pair = malloc(sizeof(*pair));

		pair->entrance = vector_get_at(state->objects, i);

		int exit_pos = ((rand() % (PORTAL_NUM + 1)) * 4) - 1;  // Τυχαία exit πύλη
		if (exit_pos == -1) exit_pos = 3;

		pair->exit = vector_get_at(state->objects, exit_pos);

		// Έλεγχος για πιασμένο exit
		ListNode node = list_first(state->portal_pairs);
		int taken_flag = 0;
		for(int j = (exit_pos + 1) / 4; taken_flag == 0; j++) {

			if (!taken_exit[j]) {

				list_insert_next(state->portal_pairs, node, pair);
				taken_exit[j] = 1;
				taken_flag = 1;
			}

			if (j == 100) j = -1;
		}

		list_insert_next(state->portal_pairs, node, pair);
	}

	return state;
}

// Επιστρέφει τις βασικές πληροφορίες του παιχνιδιού στην κατάσταση state

StateInfo state_info(State state) {

	return &state->info;
}

// Επιστρέφει μια λίστα με όλα τα αντικείμενα του παιχνιδιού στην κατάσταση state,
// των οποίων η συντεταγμένη x είναι ανάμεσα στο x_from και x_to.

List state_objects(State state, float x_from, float x_to) {

	List state_objects_list = list_create(free);

	// Διατρέχει τον state->objects vector

	for(SetNode node = set_find_node(state->objects_set, &x_from);
			node != set_next(state->objects_set, set_find_node(state->objects_set, &x_to));
			node = set_next(state->objects_set, node)) {

		list_insert_next(state_objects_list, list_last(state_objects_list),
										 set_find(state->objects_set, node));
	}

	return state_objects_list;
}

// Ενημερώνει την κατάσταση state του παιχνιδιού μετά την πάροδο 1 frame.
// Το keys περιέχει τα πλήκτρα τα οποία ήταν πατημένα κατά το frame αυτό.

void state_update(State state, KeyState keys) {

	if (state->info.playing && (!state->info.paused || keys->n)) {

		//// Κίνηση χαρακτήρα ////
		int pixels = 20;

		if (keys->right && !keys->left) {
			pixels = 40;
			state->info.character->forward = true;
		} else if (!keys->right && keys->left) {
			pixels = 40;
			state->info.character->forward = false;
		}

		if (state->info.character->forward)
			state->info.character->rect.x += pixels;
		else
			state->info.character->rect.x -= pixels;

		//// Κατάσταση άλματος ////
		// Αν το ↑ είναι πατημένο και ο χαρακτήρας είναι στο έδαφος
		if (keys->up && state->info.character->rect.y == -state->info.character->rect.height)
			state->info.character->jumping = true;

		if (state->info.character->jumping)
			state->info.character->rect.y -= 30;

		// Αν έχει φτάσει στο ανώτατο ύψος, άρχισε την πτώση
		if (state->info.character->rect.y < - (state->info.character->rect.height + 550))
			state->info.character->jumping = false;

		// Αν είναι σε πτώση και δεν είναι στο έδαφος, αύξησε το y
		if (!state->info.character->jumping &&
				state->info.character->rect.y < -state->info.character->rect.height)
			state->info.character->rect.y += 30;

		//// Αντικείμενα του frame ////
		List all_objects = state_objects(state, 0, 4*PORTAL_NUM*SPACING);

		int portal_counter = 0;
		for (ListNode node = list_first(all_objects);
				 node != LIST_EOF;
				 node = list_next(all_objects, node)) {

			Object portal_obj = list_node_value(all_objects, node);

			if (portal_obj->type == PORTAL) {

				if (state->info.character->rect.x < portal_obj->rect.x)
					break;

				portal_counter++;
			}
		}
		state->info.current_portal = portal_counter;

		for (ListNode node = list_first(all_objects);
				 node != LIST_EOF;
				 node = list_next(all_objects, node)) {

			Object enemy_obj = list_node_value(all_objects, node);
			// Αν ο χαρακτήρας συγκρουστεί με κάτι
			if (CheckCollisionRecs(state->info.character->rect, enemy_obj->rect)) {

				// Αν δεν είναι πύλη, τότε σταματάει το παιχνίδι
				if (enemy_obj->type != PORTAL) {

				 state->info.playing = false;
				 return;
				} else {
					
					if (portal_counter == PORTAL_NUM - 1 ||
							portal_counter == PORTAL_NUM) {
						
						state->info.wins++;
						state->info.character->rect.x = 0.0;
						state->info.character->jumping = true;
					} else {
						
						if (state->info.character->forward) {
							
							for (ListNode node = list_first(state->portal_pairs);
									 node != LIST_EOF;
									 node = list_next(state->portal_pairs, node)) {
								
								PortalPair pair = list_node_value(state->portal_pairs, node);
								if (pair->entrance->rect.x == enemy_obj->rect.x) {
									state->info.character->rect.x = pair->exit->rect.x;
									break;
								}
							}
						} else {
							
							for (ListNode node = list_first(state->portal_pairs);
									 node != LIST_EOF;
									 node = list_next(state->portal_pairs, node)) {
								
								PortalPair pair = list_node_value(state->portal_pairs, node);
								if (pair->exit->rect.x == enemy_obj->rect.x) {
									state->info.character->rect.x = pair->entrance->rect.x;
									break;
								}
							}
						}
						
						state->info.character->jumping = true;
					}
					
				}
			}

			if (enemy_obj->type == ENEMY) {

				for (ListNode node = list_first(all_objects);
						 node != LIST_EOF;
						 node = list_next(all_objects, node)) {

					Object second_obj = list_node_value(all_objects, node);

					if (CheckCollisionRecs(enemy_obj->rect, second_obj->rect)) {
						if (second_obj->type == OBSTACLE) {
							
							if (enemy_obj->forward)
								enemy_obj->forward = false;
							else
								enemy_obj->forward = true;
						} else if (second_obj->type == PORTAL) {
							
							if (enemy_obj->forward) {
								
								for (ListNode node = list_first(state->portal_pairs);
										 node != LIST_EOF;
										 node = list_next(state->portal_pairs, node)) {
									
									PortalPair pair = list_node_value(state->portal_pairs, node);
									if (pair->entrance->rect.x == second_obj->rect.x) {
										enemy_obj->rect.x = pair->exit->rect.x + 250;
										break;
									}
								}
							} else {
							
								for (ListNode node = list_first(state->portal_pairs);
										 node != LIST_EOF;
										 node = list_next(state->portal_pairs, node)) {
									
									PortalPair pair = list_node_value(state->portal_pairs, node);
									if (pair->exit->rect.x == second_obj->rect.x) {
										enemy_obj->rect.x = pair->entrance->rect.x - 250;
										break;
									}
								}
							}
						}
					}
				}
				
				if (enemy_obj->forward) {
					enemy_obj->rect.x += 8;
				}else{
					enemy_obj->rect.x -= 8;
			  }
			}
		}

		/* list_destroy(all_objects); */

		if (keys->p) state->info.paused = true;

		// Όταν το παιχνίδι είναι σε παύση
	} else if (state->info.paused) {

		if (keys->enter || keys->p) state->info.paused = false;

	} else if (!state->info.playing) {

		state->info.wins = 0;
		/* state->info.current_portal = 0; */
		state->info.character->forward = true;
		/* if (keys->enter) state->info.playing = true; */
	}
}

// Καταστρέφει την κατάσταση state ελευθερώνοντας τη δεσμευμένη μνήμη.
void state_destroy(State state) {

	free(state->info.character);
	/* vector_destroy(state->objects); */
	/* list_destroy(state->portal_pairs); */
	free(state);
}
