#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <math.h>

main(int argc, char **argv)  {
    int numtasks, rank, source=0, dest, tag=1, i,partialSIZE;


    MPI_Status stat;
    MPI_Datatype rowtype;   // required variable

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    partialSIZE=atoi(argv[1])/numtasks;
    char *end;
    char c[1000];
    float a[numtasks][partialSIZE];
    float b[partialSIZE];

    MPI_Type_contiguous(partialSIZE, MPI_FLOAT, &rowtype);
    MPI_Type_commit(&rowtype);
    float max=0;
    double start_time;

    if (rank == 0) {
        FILE *fp;
        char *filename=argv[2];
        fp=fopen(filename,"r");
        if (fp==NULL){
            printf("could not open file");
            return 1;
        }
        int ii=0,jj=0;
        while (fgets(c, sizeof(c),fp)!=NULL && ii<numtasks){
            if (jj==partialSIZE){
                jj=0;
                ii++;
            }

            a[ii][jj]=strtod(c, &end);

            jj++;
        }
        fclose(fp);
        if (numtasks>1){
            for (i=1; i<numtasks; i++){
                MPI_Send(&a[i], 1, rowtype, i, tag, MPI_COMM_WORLD);
            }
        }
        start_time=MPI_Wtime();




        for (int j = 0; j < sizeof(a[0])/ sizeof(float); ++j) {
            if(max<a[0][j]){
                max=a[0][j];
            }
        }

        // Now collecting individual results on process 0


    }
    else {

        // all tasks receive rowtype data from task 0
        MPI_Recv(b, partialSIZE, MPI_FLOAT, source, tag, MPI_COMM_WORLD, &stat);

        for (int j = 0; j < sizeof(b)/ sizeof(float); ++j) {
            if(max<b[j]){
                max=b[j];
            }
        }
    }

    double task_case=0.0;
    float partnermax=0;
    int partner;
    task_case=numtasks;
    double round=1;
    while (log2(numtasks)>round-1) {
        if (rank % (int) pow(2, round) == 0) {
            partner = rank + (int) pow(2, round - 1);
        } else {
            partner = rank - (int) pow(2, round - 1);
        }

        if (rank % (int) pow(2, round) != 0) {
            MPI_Send(&max, 1, MPI_FLOAT, partner, partner, MPI_COMM_WORLD);

            //printf("task %d (%f) --> to %d\n", rank, max, partner);
            break;
        } else {

            MPI_Recv(&partnermax, 1, MPI_FLOAT, partner, rank, MPI_COMM_WORLD, &stat);
            //printf("task %d (%f) <-- from %d (%fc)\n", rank, max, partner, partnermax);
            if (partnermax > max) {
                max = partnermax;
            }
        }
    round++;
    }

    if (rank==0){

        printf("Max_Value %lf\nTotal_Time %lf\n",max,MPI_Wtime()-start_time);
    }

    // free datatype when done using it
    //printf("TASK %d is exited\n",rank);
    MPI_Type_free(&rowtype);
    //MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
}