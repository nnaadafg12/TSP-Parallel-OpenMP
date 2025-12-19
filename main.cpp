#include <stdio.h>
#include <direct.h>     /* find current location */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* clock_t, clock, CLOCKS_PER_SEC */
#include <string.h>     /* strcat */
#include <math.h>		/* math calc */

#include <omp.h>

#define INPUT_FILE_NAME "input.txt"
#define NUMCITY 15		//Random generated city no - in case of using andom geneation 'OR' MAX No of input City
#define LANDSIZE 100	//Max value during random value generation
#define square(A) ((A) * (A))
 
typedef int City[2];

void Input(City cities[]);									//Take input from file of city
void generate(City cities[]);								//generate random city
void print_cities(City cities[]);							//Print all the citys
float distance(City city1, City city2);						//Find Distance between 2 city
void copy_tour(City citiesDest[], City citiesSource[]);		//Copy toured cities
void copy_City(City dest, City source);						//Copy one city to another
void swap_cities(City city1, City city2);					//Swap 2 city
void circ_perm(City cities[], int numCities);				//Put all elements in pivot - before starting a new itaration
void scramble(City cities[], City *pivot, int numCities);
void parallel_scramble(City cities[], int numCities);		//Generate all combination by brute force approach
void target_function(City cities[]);						//Get final smallest value by comparison
float tour_length(City cities[]);	
float tour_length_reduction(City cities[]);						//Get total tour length
 
float shortestTourLength;
City shortestTour[NUMCITY];
int total_no_of_input=0;
 
int main()
{
	City cities[NUMCITY];
	Input(cities);
	//generate(cities);
	printf("Total No of Input = %d\n",total_no_of_input);

	//Time calculation Start
	clock_t t;
	t = clock();
	////////////////////////
	

	shortestTourLength = tour_length(cities);		//Generate initial value - not final
	copy_tour(shortestTour, cities);
	//scramble(cities, cities, total_no_of_input);
	parallel_scramble(cities, total_no_of_input);
	printf("found the shortest tour:\n");
	print_cities(shortestTour);
	printf("Length is: %f\n", shortestTourLength);

	//Time calculation End
	t = clock() - t;
  	printf ("It took %f second(s).\n",((float)t)/CLOCKS_PER_SEC);
	////////////////////////

	getchar();
	return 0;
}


void Input (City cities[])
{
	FILE *fp;
	//Get Current Directory
	char cwd[1024];
	if (getcwd(cwd, sizeof(cwd)) == NULL)	//Directory not found
	{
		perror("getcwd() error - Current directory not found !!");
		exit(0);
	}
	strcat(cwd, "\\");
	strcat(cwd, INPUT_FILE_NAME);
	fp = fopen(cwd , "r");

	int i=0,j=0;
	while(fscanf(fp,"%d",&cities[i][j])!=EOF)
	{
		j++;
		fscanf(fp,"%d",&cities[i][j]);
		i++;
		j=0;
		total_no_of_input++;
	}
}

void generate(City cities[])
{
    int i, j;
    total_no_of_input=NUMCITY;
    for (i = 0; i < NUMCITY; i++)
        for (j = 0; j < 2; j++)
            cities[i][j] = rand() % LANDSIZE;
}

float tour_length(City cities[])
{
	int i;
	float length = 0.0f;

	#pragma omp parallel for reduction(+:length)
	for(i = 0; i < total_no_of_input - 1; i++)
	{
		length += distance(cities[i], cities[i+1]);
	}
	length += distance(cities[total_no_of_input - 1], cities[0]);
	return length;
}

float tour_length_reduction(City cities[])
{
    int i;
    float length = 0.0f;

    #pragma omp parallel for reduction(+:length)
    for (i = 0; i < total_no_of_input; i++)
    {
        int next = (i + 1) % total_no_of_input;
        length += distance(cities[i], cities[next]);
    }
    return length;
}

void target_function(City cities[])
{
		float length = tour_length(cities);
		
		//#pragma omp critical
		{
			if (length < shortestTourLength)
		    {
			    shortestTourLength = length;
			    copy_tour(shortestTour, cities);
		    }
		}
    /*float length;
    length = tour_length(cities);

	if (length < shortestTourLength)
	{
		shortestTourLength = length;
		copy_tour(shortestTour, cities);
	}*/
}
 
/*pivot is the base address in the cities[NUMCITY] array
 *for the recursive scrambling; the only reason we also
 *pass the unchanged cities address is because we need it
 *to call the target function (which does *something* to
 *the scrambled array) at each recursive call to scramble
 */
void scramble(City cities[], City *pivot, int numCities)
{
	int i;
	//City *newPivot;
	if (numCities <= 1)
	{ //Scrambled! Call the target function
		target_function(cities);
		return;
	}

	for (i = 0; i < numCities; i++)
	{
		City *newPivot = &pivot[1];
		scramble(cities, newPivot, numCities - 1);
		circ_perm(pivot, numCities);
	}
}

void parallel_scramble(City cities[], int numCities)
{
    #pragma omp parallel
    {
        City local_cities[NUMCITY];   // private to each thread

        #pragma omp for schedule(static)
        for (int i = 0; i < numCities; i++)
        {
            // Start from the original order every time
            copy_tour(local_cities, cities);

            // Fix a different city in position 0 for each i
            swap_cities(local_cities[0], local_cities[i]);

            // Recursively generate permutations of positions 1..numCities-1
            scramble(local_cities, &local_cities[1], numCities - 1);
        }
    }
}


void circ_perm(City cities[], int numCities)
{
	int i;
	City tmp;
	copy_City(tmp, cities[0]);
	for (i = 0; i < numCities - 1; i++)
		copy_City(cities[i], cities[i + 1]);
	copy_City(cities[numCities - 1], tmp);
}

void copy_tour(City citiesDest[], City citiesSource[])
{
    int i;
    for (i = 0; i < NUMCITY; i++)
        copy_City(citiesDest[i], citiesSource[i]);
}
 
void copy_City(City dest, City source)
{
    dest[0] = source[0];
    dest[1] = source[1];
}
 
void swap_cities(City city1, City city2)
{
    City tmp;
    copy_City(tmp, city1);
    copy_City(city1, city2);
    copy_City(city2, tmp);
}
 
float distance(City city1, City city2)
{
    float result;
    result = sqrtf((float)square(city2[0] - city1[0]) +
                 (float)square(city2[1] - city1[1]));
    return result;
}
 
void print_cities(City cities[])
{
    int i;
    for (i = 0; i < total_no_of_input; i++) {
        printf("(%d,%d)", cities[i][0], cities[i][1]);
        if (i < total_no_of_input - 1)
            printf(" , ");
    }
    printf("\n");
}
