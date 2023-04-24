//////////////////////////////////////////////////////////////////
//
// Test για το state.h module
//
//////////////////////////////////////////////////////////////////

#include "ADTSet.h"
#include "ADTList.h"
#include "acutest.h"			// Απλή βιβλιοθήκη για unit testing

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "common_types.h"
#include "state.h"
#include "set_utils.h"

void test_state_create() {

	State state = state_create();
	TEST_ASSERT(state != NULL);

	StateInfo info = state_info(state);
	TEST_ASSERT(info != NULL);

	TEST_ASSERT(info->current_portal == 0);
	TEST_ASSERT(info->wins == 0);
	TEST_ASSERT(info->playing);
	TEST_ASSERT(!info->paused);
	TEST_ASSERT(info->character->type = CHARACTER);
	TEST_ASSERT(info->character->forward = true);
	TEST_ASSERT(info->character->jumping == false);
	TEST_ASSERT(info->character->rect.width > 0);
	TEST_ASSERT(info->character->rect.height > 0);
	TEST_ASSERT(info->character->rect.x == 0);
	TEST_ASSERT(info->character->rect.y == - info->character->rect.height);

	float x_from = 0, x_to = 4 * PORTAL_NUM * SPACING;
	List list = state_objects(state, x_from, x_to);
	TEST_ASSERT(list_size(list) == 4*PORTAL_NUM);

	x_from = 234;
	x_to = x_from + SPACING;
	list = state_objects(state, x_from, x_to);
	TEST_ASSERT(list_size(list) == 1);

	x_from = 0;
	x_to = SPACING - 1;
	list = state_objects(state, x_from, x_to);
	TEST_ASSERT(list_size(list) == 0);

	state_destroy(state);
}

void test_state_update() {
	State state = state_create();
	TEST_ASSERT(state != NULL && state_info(state) != NULL);

	// Πληροφορίες για τα πλήκτρα (αρχικά κανένα δεν είναι πατημένο)
	struct key_state keys = { false, false, false, false, false, false };
	
	// Χωρίς κανένα πλήκτρο, ο χαρακτήρας μετακινείται 7 pixels μπροστά
	Rectangle old_rect = state_info(state)->character->rect;
	state_update(state, &keys);
	Rectangle new_rect = state_info(state)->character->rect;

	TEST_ASSERT( new_rect.x == old_rect.x + 20 && new_rect.y == old_rect.y );

	// Με πατημένο το δεξί βέλος, ο χαρακτήρας μετακινείται 12 pixes μπροστά
	keys.right = true;
	old_rect = state_info(state)->character->rect;
	state_update(state, &keys);
	new_rect = state_info(state)->character->rect;

	TEST_ASSERT( new_rect.x == old_rect.x + 40 && new_rect.y == old_rect.y );

	// Με πατημένο το αριστερό βέλος, ο χαρακτήρας μετακινείται 12 pixes πίσω
	keys.right = false;
	keys.left = true;
	old_rect = state_info(state)->character->rect;
	state_update(state, &keys);
	new_rect = state_info(state)->character->rect;

	TEST_ASSERT( new_rect.x == old_rect.x - 40 && new_rect.y == old_rect.y );

	// Με πατημένο το πάνω βέλος, ο χαρακτήρας μετακινείται 15 pixels πάνω
	keys.up = true;
	old_rect = state_info(state)->character->rect;
	state_update(state, &keys);
	new_rect = state_info(state)->character->rect;

	TEST_ASSERT(new_rect.y == old_rect.y - 30);

	state_destroy(state);
}


//// Tests για το set_utils.h

int compare(Pointer num1, Pointer num2) {

    return *(int*) num1 - *(int*) num2;
}

CompareFunc compare_func = &compare;

void test_set_utils() {

	Set set = set_create(compare_func, NULL);

	int array[5]={2,3,5,7,9};

	for (int i = 0; i < 5; i++){

		set_insert(set, &array[i]);
	}

	int value = 4;
	TEST_ASSERT(*(int*)set_find_eq_or_greater(set, &value) > value);
	TEST_ASSERT(*(int*)set_find_eq_or_smaller(set, &value) < value);

	value = 5;
	TEST_ASSERT(set_find_eq_or_greater(set, &value) == set_find(set, &value));
	TEST_ASSERT(set_find_eq_or_smaller(set, &value) == set_find(set, &value));

	value = 10;
	TEST_ASSERT(set_find_eq_or_greater(set, &value) == NULL);

	value = 1;
	TEST_ASSERT(set_find_eq_or_smaller(set, &value) == NULL);

	set_destroy(set);
}

// Λίστα με όλα τα tests προς εκτέλεση
TEST_LIST = {
	{ "test_state_create", test_state_create },
	{ "test_state_update", test_state_update },
	{ "test_set_utils", test_set_utils },

	{ NULL, NULL } // τερματίζουμε τη λίστα με NULL
};
