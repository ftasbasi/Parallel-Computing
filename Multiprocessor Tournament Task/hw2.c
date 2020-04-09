#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
int simulate_game() {
    usleep(10);// Simulation takes time
    return (rand() %2);// HOME_WIN or AWAY_WIN
}

void main(int argc, char **argv){
    int rank, world_size,countofTeams;
    countofTeams=atoi(argv[1]);
    countofTeams=pow(2,countofTeams);

    int *table;
    int round_max;
    round_max=log2(countofTeams);
    MPI_Status stat;
    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &world_size );
    srand(time(NULL) + rank);
    int *row_raw,*row_tmp;
    int round=1;
    int partialSize;
    double start_time;
    partialSize = countofTeams / world_size;
    /* If I'm the root (process 0), then fill out the big table */
    if (rank == 0) {
        table = malloc(sizeof(int) * countofTeams);
        for (int i = 0; i < countofTeams; i++) {
            *(table + i) = 0;
        }
    }
    row_raw = malloc(sizeof(int) * partialSize);
    MPI_Scatter(table, partialSize, MPI_INT, row_raw, partialSize, MPI_INT, 0, MPI_COMM_WORLD);
    start_time=MPI_Wtime();
    if (rank==0){
        free(table);
    }
    while (round_max > round - 1 && partialSize>1) {


        int k = 0;

        if (round == 1) {
            while (k < partialSize) {

                if (simulate_game()) {
                    *(row_raw + k) = (rank * partialSize) + k + 1;
                    *(row_raw + k + 1) = 0;
                } else {
                    *(row_raw + k) = 0;
                    *(row_raw + k + 1) = (rank * partialSize) + k + 2;
                }
                k += 2;
            }
        } else {
            while (k < partialSize) {

                if (simulate_game()) {
                    *(row_raw + k + 1) = 0;
                } else {
                    *(row_raw + k) = 0;
                }
                k += 2;
            }
        }

        k = 0;
        int l = 0;
        row_tmp = malloc(sizeof(int) * partialSize / 2);
        while (k < partialSize) {
            if (*(row_raw + k)) {
                *(row_tmp + l) = *(row_raw + k);
                l++;
            }
            k++;
        }
        free(row_raw);
        row_raw=row_tmp;

        partialSize/=2;
        round++;
    }


    int partner;
    int other;
    int mine;
    mine=*row_raw;
    free(row_raw);

    //printf("roundmax:%d , round:%d\n",round_max,round);
    round_max=round_max-round+2;
    round=1;
    while (round_max > round){
        if (rank % (int) pow(2, round) == 0) {
            partner = rank + (int) pow(2, round - 1);
        } else {
            partner = rank - (int) pow(2, round - 1);
        }

        if (rank % (int) pow(2, round) != 0) {
            MPI_Send(&mine, 1, MPI_INT, partner, partner, MPI_COMM_WORLD);
            break;
        } else {
            MPI_Recv(&other, 1, MPI_INT, partner, rank, MPI_COMM_WORLD, &stat);
            if (simulate_game()) {
            } else{
                mine=other;
            }
        }
        round++;
    }

    //printf("Process %d leaving\n",rank);

    if (rank==0){
        printf("The_Winner %d\nTotal_Time %lf\n",mine,MPI_Wtime()-start_time);
    }



    MPI_Finalize();

}