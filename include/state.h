#pragma once

#include "raylib.h"
#include "ADTList.h"

#define PORTAL_NUM 100		// πόσα portals υπάρχουν στην πίστα
#define SPACING 1200			// απόσταση ανάμεσα στα αντικείμενα της πίστας

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

typedef enum {
	OBSTACLE, PORTAL, ENEMY, CHARACTER
} ObjectType;

// Πληροφορίες για κάθε αντικείμενο
typedef struct object {
	ObjectType type;				// Τύπος (Χαρακτήρας / Εμπόδιο / Εχθρός / Πύλη)
	Rectangle rect;					// Θέση και μέγεθος του αντικειμένου. Το Rectangle ορίζεται στο include/raylib.h, line 213
	bool forward;					// true: το αντικείμενο κινείται προς τα δεξιά
	bool jumping;					// true: το αντικείμενο έχει ξεκινήσει άλμα
}* Object;

// Γενικές πληροφορίες για την κατάσταση του παιχνιδιού
typedef struct state_info {
	Object character;				// πληροφορίες για το χαρακτήρα
	int current_portal;				// σε ποιο αριθμό πύλης βρίσκεται ο χαρακτήρας (η τελευταία πύλη που βρίσκεται αριστερά του)
	int wins;						// αριθμός νικών (πόσες φορές φτάσαμε στην τελική πύλη)
	bool playing;					// true αν το παιχνίδι είναι ενεργό (false μετά από game over)
	bool paused;					// true αν το παιχνίδι είναι paused
}* StateInfo;

// Πλοροφορίες για το ποια πλήκτρα είναι πατημένα
typedef struct key_state {
	bool up;						// true αν το αντίστοιχο πλήκτρο είναι πατημένο
	bool left;
	bool right;
	bool enter;
	bool n;
	bool p;
}* KeyState;

// Η κατάσταση του παιχνιδιού (handle)
typedef struct state* State;


// Δημιουργεί και επιστρέφει την αρχική κατάσταση του παιχνιδιού

State state_create();

// Επιστρέφει τις βασικές πληροφορίες του παιχνιδιού στην κατάσταση state

StateInfo state_info(State state);

// Επιστρέφει μια λίστα με όλα τα αντικείμενα του παιχνιδιού στην κατάσταση state,
// των οποίων η συντεταγμένη x είναι ανάμεσα στο x_from και x_to.

List state_objects(State state, float x_from, float x_to);

// Ενημερώνει την κατάσταση state του παιχνιδιού μετά την πάροδο 1 frame.
// Το keys περιέχει τα πλήκτρα τα οποία ήταν πατημένα κατά το frame αυτό.

void state_update(State state, KeyState keys);

// Καταστρέφει την κατάσταση state ελευθερώνοντας τη δεσμευμένη μνήμη.

void state_destroy(State state);
