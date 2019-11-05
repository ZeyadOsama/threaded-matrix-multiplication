#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <climits>
#include <pthread.h>

#define DEBUG

#define ELEMENT 1
#define ROW 2

#define TOKEN_BUFFER_SIZE 64
#define TOKEN_DELIMITERS " \t\r\n\a"

#define PAD_VALUE 5

const char *ERROR = "error";

using namespace std;

/**
 * @brief matrix definition.
 */
typedef struct {
    int *mat;
    int r;
    int c;
} matrix;

/**
 * @brief thread's data definition.
 */
typedef struct {
    matrix *matA;
    matrix *matB;
    matrix *matC;
    int i;
    int j;
} pthread_data_t;

/**
 * @return cwd: current working directory.
 */
char *getCWD();

/**
 * @param cwd
 * @return path for reading files
 */
char *getReadDirectory(char *cwd);

/**
 * @param cwd
 * @return path for writing files.
 */
char *getWriteDirectory(char *cwd);

/**
 * @brief reads a file from specified path.
 * @param path to read from.
 * @return string pointer containing file's chars.
 */
char *readFile(char *path);

/**
 * @brief write a file to specified path.
 * @param path to write in.
 * @param arr to be written.
 * @param length of arr.
 * @param time of execution.
 */
void writeFile(char *path, matrix *mat, int FLAG, double time);

/**
 * @brief crates empty output file or restarts the existing one.
 * @param path which output file should be.
 */
void generateOutputFile(char *path);

/**
 * @brief parses a given string.
 * @param file as a string.
 * @return pointer to tokenized string casted as integer.
 */
char **tokenizeFile(char *file);

/**
 * @brief extracts a matrix part from a given buffer.
 * @param buffer tokenized.
 * @return array.
 */
int *extractMatrix(int r, int c, int startIndex, char **);

/**
 * @param two matrices to be checked if they can be multiplied.
 * @return boolean upon check.
 */
bool multiplicationPossibility(matrix *matA, matrix *matB);

/**
 * @param buffer to be checked if allocated.
 * @return boolean upon check.
 */
bool allocation_check(const char *buffer);


#ifdef ROW

void *rowMultiplication(void *arg);

#endif

#ifdef ELEMENT

void *elementMultiplication(void *arg);

#endif

/// Driver Code
int main() {
    // setting up environment.
    generateOutputFile(getWriteDirectory(getCWD()));

    char **tokens = tokenizeFile(readFile(getReadDirectory(getCWD())));

    // assigning matrix A.
    matrix matA;
    matA.r = stoi(tokens[0]);
    matA.c = stoi(tokens[1]);
    matA.mat = extractMatrix(matA.r, matA.c, 2, tokens);

    // assigning matrix B.
    int offset = matA.r * matA.c + 2;
    matrix matB;
    matB.r = stoi(tokens[offset]);
    matB.c = stoi(tokens[offset + 1]);
    matB.mat = extractMatrix(matB.r, matB.c, offset + 2, tokens);

    if (!multiplicationPossibility(&matA, &matB))
        exit(EXIT_FAILURE);

    // assigning matrix C.
    matrix matC;
    matC.r = matA.r;
    matC.c = matB.c;

    int THREADS;

    // execution time
    clock_t start_t, end_t;
    double time;

#ifdef ROW
    /**
     * row by col multiplication.
     */
    matC.mat = (int *) malloc(matC.r * matC.c * sizeof(int));
    for (int i = 0; i < matC.r * matC.c; ++i)
        matC.mat[i] = 0;
    THREADS = matC.r;

    // declaring four threads
    pthread_t threadsRows[THREADS];

    // start time for row by col approach.
    start_t = clock();

    // creating threads.
    // each evaluating its own part.
    for (int i = 0; i < matC.r; i++) {
        auto *data = (pthread_data_t *) (malloc(sizeof(pthread_data_t)));
        data->matA = &matA;
        data->matB = &matB;
        data->matC = &matC;
        data->i = i;
        pthread_create(&threadsRows[i], nullptr, rowMultiplication, (void *) (data));
    }

    // joining and waiting for all threads to complete.
    for (int i = 0; i < THREADS; i++)
        pthread_join(threadsRows[i], nullptr);

    // end time for row by col approach.
    end_t = clock();
    time = (double) (end_t - start_t) / CLOCKS_PER_SEC;

    // displaying the result matrix
    cout << endl << "row by col:-" << endl;
    cout << "execution time:: " << time << endl;
    cout << "multiplication of A and B:-" << endl;
    for (int i = 0; i < matA.r; i++) {
        for (int j = 0; j < matB.c; j++)
            cout << setw(PAD_VALUE) << *(matC.mat + i * matC.c + j) << " ";
        cout << endl;
    }
    writeFile(getWriteDirectory(getCWD()), &matC, ROW, time);
    free(matC.mat);
#endif

#ifdef ELEMENT
    /**
     * element by element multiplication.
     */
    matC.mat = (int *) malloc(matC.r * matC.c * sizeof(int));
    for (int i = 0; i < matC.r * matC.c; ++i)
        matC.mat[i] = 0;
    THREADS = matC.r * matC.c;

    // declaring four threads
    pthread_t threadsElements[THREADS];

    // start time for row by col approach.
    start_t = clock();

    // creating threads.
    // each evaluating its own part.
    for (int i = 0; i < matC.r; i++)
        for (int j = 0; j < matC.c; j++) {
            auto *data = (pthread_data_t *) (malloc(sizeof(pthread_data_t)));
            data->matA = &matA;
            data->matB = &matB;
            data->matC = &matC;
            data->i = i;
            data->j = j;
            pthread_create(&threadsElements[i * j], nullptr, elementMultiplication, (void *) (data));
        }

    // joining and waiting for all threads to complete.
    for (int i = 0; i < THREADS; i++)
        pthread_join(threadsElements[i], nullptr);

    // end time for row by col approach.
    end_t = clock();
    time = (double) (end_t - start_t) / CLOCKS_PER_SEC;

    // displaying the result matrix
    cout << endl << "element by element:-" << endl;
    cout << "execution time:: " << time << endl;
    cout << "multiplication of A and B:-" << endl;
    for (int i = 0; i < matA.r; i++) {
        for (int j = 0; j < matB.c; j++)
            cout << setw(PAD_VALUE) << *(matC.mat + i * matC.c + j) << " ";
        cout << endl;
    }
    writeFile(getWriteDirectory(getCWD()), &matC, ELEMENT, time);
    free(matC.mat);
#endif

    // de-allocating.
    free(matA.mat);
    free(matB.mat);

    return EXIT_SUCCESS;
}

#ifdef ROW

void *rowMultiplication(void *arg) {
    // assigning information.
    auto *data = ((pthread_data_t *) arg);
    matrix *matA = data->matA;
    matrix *matB = data->matB;
    matrix *matC = data->matC;
    int i = data->i;

    // applying multiplication.
    for (int j = 0; j < matB->c; j++)
        for (int k = 0; k < matB->r; k++)
            *(matC->mat + i * matC->c + j) += *(matA->mat + i * matA->c + k) * *(matB->mat + k * matB->c + j);
    return nullptr;
}

#endif

#ifdef ELEMENT

void *elementMultiplication(void *arg) {
    // assigning information.
    auto *data = ((pthread_data_t *) arg);
    matrix *matA = data->matA;
    matrix *matB = data->matB;
    matrix *matC = data->matC;
    int i = data->i;
    int j = data->j;

    // applying multiplication.
    for (int k = 0; k < matB->r; k++)
        *(matC->mat + i * matC->c + j) += *(matA->mat + i * matA->c + k) * *(matB->mat + k * matB->c + j);
    free(arg);
    return nullptr;
}

#endif

char *getCWD() {
    static char cwd[PATH_MAX];
    // store program start working directory.
    if (getcwd(cwd, sizeof(cwd)) == nullptr) {
        perror(ERROR);
        exit(EXIT_FAILURE);
    }
#ifdef DEBUG
    cout << "\ncwd:: " << cwd << endl;
#endif
    return cwd;
}

char *getReadDirectory(char *cwd) {
    static char cwd_r[PATH_MAX];
    strcpy(cwd_r, cwd);
    strcat(cwd_r, "/input.txt");
#ifdef DEBUG
    cout << "cwd:: " << cwd_r << endl;
#endif
    return cwd_r;
}

char *getWriteDirectory(char *cwd) {
    static char cwd_w[PATH_MAX];
    strcpy(cwd_w, cwd);
    strcat(cwd_w, "/output.txt");
#ifdef DEBUG
    cout << "cwd:: " << cwd_w << endl;
#endif
    return cwd_w;
}

char *readFile(char *path) {
    static char *buffer = nullptr;
    long length;
    FILE *fptr = fopen(path, "rb");

    if (fptr) {
        fseek(fptr, 0, SEEK_END);
        length = ftell(fptr);
        fseek(fptr, 0, SEEK_SET);
        buffer = (char *) malloc((size_t) length);
        if (buffer)
            fread(buffer, sizeof(char), (size_t) length, fptr);
        fclose(fptr);
#ifdef DEBUG
        cout << "\ninput-file:: reading from file success.\n" << buffer << endl;
#endif
    }

    if (!buffer) {
        perror(ERROR);
        exit(EXIT_FAILURE);
    }
#ifdef DEBUG
    cout << "\nbuffer:-\n" << buffer << endl;
#endif
    return buffer;
}

void writeFile(char *path, matrix *mat, int FLAG, double time) {
    FILE *fptr;
    fptr = fopen(path, "a");
    for (int i = 0; i < mat->r; i++) {
        for (int j = 0; j < mat->c; j++)
            fprintf(fptr, "%5d ", *(mat->mat + i * mat->c + j));
        fprintf(fptr, "\n");
    }
    fprintf(fptr, "END%d\t", FLAG);
    fprintf(fptr, "[%f]\n", time);
    fclose(fptr);
#ifdef DEBUG
    cout << "output-file:: writing to file success." << endl;
#endif
}

void generateOutputFile(char *path) {
    FILE *fptr;
    fptr = fopen(path, "w");
    fprintf(fptr, "");
    fclose(fptr);
#ifdef DEBUG
    cout << "output-file:: resetting file success." << endl;
#endif
}

char **tokenizeFile(char *file) {
    // creating a buffer for storing tokens.
    // setting buffer size to be allocated.
    int buffer_size = TOKEN_BUFFER_SIZE;

    // positioning index for assigning tokens to their positions in buffer
    int index = 0;

    // allocating memory for tokens.
    static char **tokens = (char **) malloc(buffer_size * sizeof(char *));
    // pointers for each token string.
    char *token, **tokens_backup;

    // failure check.
    // check if buffer was allocated successfully.
    allocation_check((char *) tokens);

    token = strtok(file, TOKEN_DELIMITERS);
    while (token != nullptr) {
        // assigning tokens.
        tokens[index++] = token;

        // buffer size exceeded the pre-defined size.
        // reallocate.
        if (index >= buffer_size) {
            buffer_size += TOKEN_BUFFER_SIZE;
            tokens_backup = tokens;
            tokens = (char **) realloc(tokens, buffer_size * sizeof(char *));

            // failure check.
            // check if buffer was allocated successfully.
            if (!tokens) {
                free(tokens_backup);
                fprintf(stderr, "%s: allocation error\n", ERROR);
                exit(EXIT_FAILURE);
            }
        }
        // tokenize.
        token = strtok(nullptr, TOKEN_DELIMITERS);
    }
    // terminating tokens array by NULL value.
    tokens[index] = nullptr;
#ifdef DEBUG
    int r = stoi(tokens[0]);
    int c = stoi(tokens[1]);

    cout << "\nindex:: " << index << endl;
    cout << "\nr-mat-1:: " << r << endl;
    cout << "c-mat-1:: " << c << endl;
    cout << "r-mat-2:: " << tokens[(r * c) + 2] << endl;
    cout << "c-mat-2:: " << tokens[(r * c) + 3] << endl;

    extractMatrix(r, c, 2, tokens);

    int offset = r * c + 2;
    r = stoi(tokens[offset]);
    c = stoi(tokens[offset + 1]);
    extractMatrix(r, c, offset + 2, tokens);
#endif
    return tokens;
}

int *extractMatrix(int r, int c, int startIndex, char **tokens) {
#ifdef DEBUG
    cout << "\nr::" << r << endl;
    cout << "c::" << c << endl;
    cout << "start-index::" << startIndex << endl;
#endif

    int *arr = (int *) malloc(r * c * sizeof(int));
    int count = startIndex;
    for (int i = 0; i < r; i++)
        for (int j = 0; j < c; j++)
            *(arr + i * c + j) = stoi(tokens[count++]);
#ifdef DEBUG
    cout << "\nmatrix:-\n";
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++)
            cout << setw(PAD_VALUE) << *(arr + i * c + j) << " ";
        cout << endl;
    }
#endif
    return arr;
}

bool multiplicationPossibility(matrix *matA, matrix *matB) {
    return matA->c == matB->r;
}

bool allocation_check(const char *buffer) {
    // failure check.
    if (!buffer) {
        fprintf(stderr, "%s: allocation error\n", ERROR);
        // segment fault occurred.
        exit(EXIT_FAILURE);
    }
    return true;
}
