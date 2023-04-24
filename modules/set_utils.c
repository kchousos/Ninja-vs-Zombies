#include <stdio.h>
#include "set_utils.h"
#include "ADTSet.h"

// Επιστρέφει την μοναδική τιμή του set που είναι ισοδύναμη με value,
// ή αν δεν υπάρχει, την μικρότερη τιμή του set που είναι μεγαλύτερη
// από value. Αν δεν υπάρχει καμία τότε επιστρέφει NULL.

Pointer set_find_eq_or_greater(Set set, Pointer value) {

	Pointer result = set_find(set, value);

	if (!result) {

		set_insert(set, value);
		SetNode node = set_find_node(set, value);
		SetNode next = set_next(set, node);
		if (next != SET_EOF)
			result = set_find(set, set_node_value(set, next));
		set_remove(set, value);
	}

	return result;
}

// Επιστρέφει την μοναδική τιμή του set που είναι ισοδύναμη με value,
// ή αν δεν υπάρχει, την μεγαλύτερη τιμή του set που είναι μικρότερη
// από value. Αν δεν υπάρχει καμία τότε επιστρέφει NULL.

Pointer set_find_eq_or_smaller(Set set, Pointer value) {

	Pointer result = set_find(set, value);

	if (!result) {

		set_insert(set, value);
		SetNode node = set_find_node(set, value);
		SetNode previous = set_previous(set, node);
		if (previous != SET_BOF)
			result = set_find(set, set_node_value(set, previous));
		set_remove(set, value);
	}

	return result;
}
