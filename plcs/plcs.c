#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "thread.h"
#include "thread-sync.h"

#define MAXN 10000
int T, N, M;
char A[MAXN + 1], B[MAXN + 1];
int dp[MAXN][MAXN];
int result;

#define DP(x, y) (((x) >= 0 && (y) >= 0) ? dp[x][y] : 0)
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MAX3(x, y, z) MAX(MAX(x, y), z)

// Mutex for protecting the dp array
mutex_t dp_mutex;

void ParallelLCS(int id) {
    // Calculate the range of subproblems for this thread
    int start = id * (N / T);
    int end = (id == T - 1) ? N - 1 : (id + 1) * (N / T) - 1;

    // Parallel computation of the dp array
    for (int i = start; i <= end; i++) {
        for (int j = 0; j < M; j++) {
            int skip_a = DP(i - 1, j);
            int skip_b = DP(i, j - 1);
            int take_both = DP(i - 1, j - 1) + (A[i] == B[j]);

            mutex_lock(&dp_mutex); // Acquire lock before updating the dp array
            dp[i][j] = MAX3(skip_a, skip_b, take_both);
            mutex_unlock(&dp_mutex); // Release lock after updating the dp array
        }
    }
}

int main(int argc, char *argv[]) {
    assert(scanf("%s%s", A, B) == 2);
    N = strlen(A);
    M = strlen(B);
    T = !argv[1] ? 1 : atoi(argv[1]);

    // Initialize mutex
    pthread_mutex_init(&dp_mutex, NULL);

    for (int i = 0; i < T; i++) {
        create(ParallelLCS);
    }
    join();  // Wait for all workers

    printf("%d\n", dp[N - 1][M - 1]);

    // Destroy mutex
    pthread_mutex_destroy(&dp_mutex);

    return 0;
}
