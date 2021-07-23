#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

int LOWEST_ADDRESS = 0;
int HIGHEST_ADDRESS = 256;
int BYTES_PER_ADDRESS = 4;
int INITIAL_ADDRESS = 0;

struct Node {
	bool	available; // true: hole, false: process
	int	address;
	int	length;
	struct Node*	next;
	struct Node*	prev;
};

struct ContiguousMemory {
	int capacity;
	struct Node* head;
	int lowest;
	int highest;
};

struct Node* get_node(bool type, int len) {
	struct Node* n = (struct Node*)malloc(sizeof(struct Node));
	n->available = type;
	n->length = len;
	n->next = NULL;
	n->prev = NULL;
	return n;
}

struct ContiguousMemory* init_mem(int MEMORY_SIZE) {
	struct ContiguousMemory* mem = (struct ContiguousMemory*) malloc(sizeof(struct ContiguousMemory));
	mem->capacity = MEMORY_SIZE;
	mem->lowest = LOWEST_ADDRESS;
	mem->highest = HIGHEST_ADDRESS;
	mem->head = get_node(true, mem->capacity);
	(mem->head)->address = INITIAL_ADDRESS;
	printf("%s", "Contiguous memory created with capacity: ");
	printf("%d", mem->capacity);
	printf("%s", "\n\n");
	return mem;
}

void list_print(struct Node* head) {
	struct Node* temp = head;
	while(temp != NULL) {
		printf("%s", "[ Address: ");
		printf("%d", temp->address);
		printf("%s", ", Type: ");
		printf("%s", temp->available ? "hole" : "process");
		printf("%s", ", Length: ");
		printf("%d", temp->length);
		printf("%s", "] -> \n");
		temp = temp->next;
	}
	printf("%s", "\n");
}

struct Node *split(struct Node *head)
{
    struct Node *fast = head,*slow = head;
    while (fast->next && fast->next->next)
    {
        fast = fast->next->next;
        slow = slow->next;
    }
    struct Node *temp = slow->next;
    slow->next = NULL;
    return temp;
}
 
struct Node *merge(struct Node *first, struct Node *second)
{
    if (!first)
        return second;
 
    if (!second)
        return first;
 
    if (first->address < second->address)
    {
        first->next = merge(first->next,second);
        first->next->prev = first;
        first->prev = NULL;
        return first;
    }
    else
    {
        second->next = merge(first,second->next);
        second->next->prev = second;
        second->prev = NULL;
        return second;
    }
}

struct Node *merge_sort(struct Node *head)
{
    if (!head || !head->next)
        return head;
    struct Node *second = split(head);
 
    head = merge_sort(head);
    second = merge_sort(second);
 
    return merge(head,second);
}

void first_fit(struct ContiguousMemory* mem, struct Node* elem) {
	struct Node* curr;
	struct Node** head_p = &(mem->head);

	if (elem->length > mem->capacity) {
		printf("%s", "ERROR: process is too big for memory.\n\n");
		return;
	}

	if (elem->length <= (*head_p)->length // it fits
	&& (*head_p)->available == true // it's a hole
	) {
		
		//split node
		(*head_p)->length = (*head_p)->length - elem->length;
		elem->address = (*head_p)->address;
		(*head_p)->address = (*head_p)->address + (elem->length/BYTES_PER_ADDRESS);
		
		//insert node
		elem->next = *head_p;
		elem->next->prev = elem;
		*head_p = elem;

		printf("%s", "Process successfully inserted in head!\n");
	}

	else {
		curr = *head_p;

		while(curr->next != NULL // end has not been reached
		) {
			if(curr->available == true // if a hole is found
			&& elem->length <= curr->length //and the process fits
			) {
				break; // choose this hole
			}
			curr = curr->next;
		}

		if (curr->next == NULL && curr->available == false) {
			printf("%s", "ERROR: No space left for a new process in memory.\n\n");
			return;
		}

		//split
		curr->length = curr->length - elem->length;
		elem->address = curr->address;
		curr->address = curr->address + (elem->length/BYTES_PER_ADDRESS);

		//insert node
		elem->next = curr->next;

		if (curr->next != NULL)
			elem->next->prev = elem;
		curr->next = elem;
		elem->prev = curr;

		merge_sort(mem->head);

		printf("%s", "Process successfully inserted!\n");
	}
}

void deallocate(struct ContiguousMemory* mem, int add) {
	struct Node* curr;
	struct Node** head_p = &(mem->head);
	int new_length;

	if(add >= mem->highest || add < mem->lowest) {
		printf("%s", "ERROR: Address unavailable\n\n");
		return;
	}

	curr = *head_p;

	while(curr->address < add
	      && curr->next != NULL // end has not been reached
	) {
		curr = curr->next;
	}

	//verification that there is a hole here
	if(curr->available == true) {
		printf("%s", "ERROR: There is not a program located here.\n\n");
		return;
	}

	//set location as available
	curr->available = true;

	/*==NO MERGING IS NECESSARY==*/

	if(
		//if there are no partitions
		(curr->prev == NULL
		&& curr->next == NULL
	) || (
		curr->prev == NULL //if node is the head
		&& (curr->next)->available == false //and is followed by a process
	) || (
		curr->next == NULL //if node is the tail
	  	&& (curr->prev)->available == false //and is preceeded by a process
	) || (
		//if surrounded by processes
		(curr->prev)->available == false 
		&& (curr->next)->available == false
		)
	) {
		printf("%s", "Memory set as hole\n");
		list_print(mem->head);
		return;
	}

	/*==MERGING IS NECESSARY==*/

	struct Node* merge_aux;

	/*MERGE WITH RIGHT*/
	if(
		//if previous is a process or current node is the head
		((curr->prev)->available == false || curr->prev == NULL)
    	&& (curr->next)->available == true //but next one is a hole
	) {
		new_length = curr->length + (curr->next)->length;
		merge_aux = curr->next;

		if(merge_aux->next != NULL) {
			(merge_aux->next)->prev = curr;
		}
		curr->next = merge_aux->next;
		
		free(merge_aux);
		curr->length = new_length;

		printf("%s", "Memory hole merged with right\n");
		list_print(mem->head);
		return;
	}

	/*MERGE WITH LEFT*/

	if(
		// if next one is a process or current node is the tail
		((curr->next)->available == false || curr->next == NULL)
		&& (curr->prev)->available == true // but previous is a hole
	) {
		new_length = (curr->prev)->length + curr->length;
		merge_aux = curr->prev;

		merge_aux->next = curr->next;
		(curr->next)->prev = curr->prev;

		free(curr);
		merge_aux->length = new_length;

		printf("%s", "Memory hole merged with left\n");
		list_print(mem->head);
		return;
	}

	/*DOUBLE MERGE*/
	// if surrounded by holes
	
	new_length = (curr->prev)->length +
				 (curr->length) +
				 (curr->next)->length;

	//merge with right hole
	merge_aux = curr->next;
	if(merge_aux->next != NULL) {
		(merge_aux->next)->prev = curr;
	}
	curr->next = merge_aux->next;
	free(merge_aux);

	//merge with left hole
	merge_aux = curr->prev;
	if(curr->next != NULL) {
		(curr->next)->prev = merge_aux;
	}
	(merge_aux)->next = curr->next;
	free(curr);

	//update hole size
	merge_aux->length = new_length;

	printf("%s", "Memory holes from both sides merged\n");	
	list_print(mem->head);
}

void insert(struct ContiguousMemory* mem, int size) {
	struct Node* n = get_node(false, size);
	first_fit(mem, n);
	list_print(mem->head);
}


void test() {
	struct ContiguousMemory* mem = init_mem(256);
	
	//bigger than memory
	insert(mem, 512);

	//filling memory
	insert(mem, 12);
	insert(mem, 20);
	deallocate(mem, 3);
	insert(mem, 48);
	insert(mem, 108);
	deallocate(mem, 15);
	insert(mem, 100);
	deallocate(mem, 3);
	deallocate(mem, 15);
}

int main() {
	test();
	return 0;
}