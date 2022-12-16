#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

// A structure to represent a deque
struct Deque {
    int front, rear, size;
    unsigned capacity;
    float *array;
};

// Function to create a deque of given capacity.
// It initializes size of deque as 0
struct Deque *createDeque(unsigned capacity) {
    struct Deque *deque = (struct Deque *) malloc(sizeof(struct Deque));
    deque->capacity = capacity;
    deque->front = deque->size = 0;

    // This is important, see the enqueue
    deque->rear = capacity - 1;
    deque->array = (float *) malloc(deque->capacity * sizeof(float));
    return deque;
}

// Deque is full when size becomes equal to the capacity
int isFull(struct Deque *deque) {
    return (deque->size == deque->capacity);
}

// Deque is empty when size is 0
int isEmpty(struct Deque *deque) {
    return (deque->size == 0);
}

// Function to add an item at the front of deque.
// It changes rear and size
void pushFront(struct Deque *deque, float *item) {
    if (isFull(deque))
        return;
    deque->front = (deque->front - 1) % deque->capacity;
    deque->array[deque->front] = *item;
    deque->size = deque->size + 1;
}

// Function to add an item at the rear of deque.
// It changes rear and size
void pushBack(struct Deque *deque, float *item) {
    if (isFull(deque))
        return;
    deque->rear = (deque->rear + 1) % deque->capacity;
    deque->array[deque->rear] = *item;
    deque->size = deque->size + 1;
}

// Function to remove an item from front of deque.
// It changes front and size
float *popFront(struct Deque *deque) {
    if (isEmpty(deque))
        return NULL;
    float *item = (float *) malloc(sizeof(float));
    *item = deque->array[deque->front];
    deque->front = (deque->front + 1) % deque->capacity;
    deque->size = deque->size - 1;
    return item;
}

// Function to remove an item from rear of deque.
// It changes rear and size
float *popBack(struct Deque *deque) {
    if (isEmpty(deque))
        return NULL;
    float *item = (float *) malloc(sizeof(float));
    *item = deque->array[deque->rear];
    deque->rear = (deque->rear - 1) % deque->capacity;
    deque->size = deque->size - 1;
    return item;
}

// Function to get front of deque
float *getFront(struct Deque *deque) {
    if (isEmpty(deque))
        return NULL;
    return &deque->array[deque->front];
}

// Function to get rear of deque
float *getBack(struct Deque *deque) {
    if (isEmpty(deque))
        return NULL;
    return &deque->array[deque->rear];
}

// Function to peek deck
float *peek(struct Deque *deque, int index) {
    if (isEmpty(deque))
        return NULL;
    return &deque->array[(deque->front + index) % deque->capacity];
}

// Function to get size of deque
int getSize(struct Deque *deque) {
    return deque->size;
}

// Function to get capacity of deque
int getCapacity(struct Deque *deque) {
    return deque->capacity;
}

// Function to get mean of deque
float getMean(struct Deque *deque) {
    float sum = 0;
    for (int i = 0; i < getSize(deque); i++)
        sum += *peek(deque, i);
    return sum / getSize(deque);
}

double calculate_signal_freq(char *first_timestamp, char *last_timestamp, int n_periods) {
    // Parsing timestamps
    int h1, m1, s1, ms1, h2, m2, s2, ms2;
    sscanf(first_timestamp, "%d:%d:%d.%d", &h1, &m1, &s1, &ms1);
    sscanf(last_timestamp, "%d:%d:%d.%d", &h2, &m2, &s2, &ms2);

    // Calculating time difference, in seconds
    float time_diff = (h2 - h1) * 3600 + (m2 - m1) * 60 + (s2 - s1) + (float) (ms2 - ms1) / 1000;

    // Calculating signal frequency
    float signal_freq = 1 / (time_diff / n_periods);

    return signal_freq;
}

float last_reference = 0.;
int n_switches = 0,
    n_periods = 4,
    v_length = 0;
bool is_ready = false;
char first_timestamp[50];
char last_timestamp[50];

// Deques to store reference and measure signals
struct Deque *ref_deque, *mea_deque;

void setup(char *TIMESTAMP, float REFERENCE, float MEASURE) {
    // Truncating reference signal
    REFERENCE = REFERENCE > 1500 ? 1 : -1;

    // Using first 20 periods to get the signal frequency
    if (!is_ready) {
        // Checking if reference signal changed
        if (REFERENCE == -last_reference)
            n_switches++;

        // Storing first timestamp
        if (first_timestamp[0] == '\0')
            strcpy(first_timestamp, TIMESTAMP);

        // Storing last timestamp
        if (n_switches == n_periods * 2) 
            strcpy(last_timestamp, TIMESTAMP);

        // Once n_periods are detected, the system is ready
        if (n_switches == n_periods * 2) {
            is_ready = true;
            return;
        }

        v_length++;
        last_reference = REFERENCE;
        return;
    }

    // Calculating signal frequency
    // ! Frequency is not been used yet
    float signal_freq = calculate_signal_freq(first_timestamp, last_timestamp, n_periods);

    // Creating deque
    if (!ref_deque) {
        ref_deque = createDeque(v_length);
        mea_deque = createDeque(v_length);
    }

    // Filling deque
    if (getSize(mea_deque) < v_length) {
        float mul = REFERENCE * MEASURE;
        pushBack(ref_deque, &REFERENCE);
        pushBack(mea_deque, &mul);
        return;
    }

    // Removing oldest value and adding new one
    float mul = MEASURE * REFERENCE;
    popFront(ref_deque);
    pushBack(ref_deque, &REFERENCE);
    popFront(mea_deque);
    pushBack(mea_deque, &mul);


    // printf("%f\n", *peek(mea_deque, 700));
    // Calculating mean
    float mean = getMean(mea_deque);

    printf("%f\n", mean);
};

int main() {
    // Reading data file, line by line
    FILE *fp;
    char *line = NULL;
    size_t len = 0, read;

    fp = fopen("data/simulados.txt", "r");
    while ((read = getline(&line, &len, fp)) != -1) {  
        // Array to store the tokens
        char *tokens[100];
        int j = 0;
        
        // Splitting line into tokens
        char *token = strtok(line, " ");
        while (token != NULL) {
            tokens[j] = token;
            token = strtok(NULL, " ");
            j++;
        }
        setup(tokens[0], atof(tokens[1]), atof(tokens[2]));
    }

    // Closing file
    fclose(fp);
    if (line)
        free(line);
    
    return 0;
}