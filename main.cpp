#include <bits/stdc++.h>
#include <cstdlib>
#include <ctime>
#include <unistd.h>

using namespace std;

/* Class:- Robot
 * Defines an entity, Robot, which stores 16 genes, 4 sensors, and some
 * energy. It needs energy to remain alive and can move around the map
 * to search for batteries. 
 *
 * The 4 sensors are pointing in North, South, East and West directions
 * and detect what is present in the adjacent squares of the robot.
 * Sensor readings:
 * 0 - No object in square
 * 1 - Wall object
 * 2 - Battery object
 * 3 - Don’t care if anything is there/Sensor gives corrupt reading
 *
 * Each gene is an array/vector containing five codes, 4 corresponding to each
 * sensors and the fifth denoting the robot's next action.
 * Robot Actions:
 * 0 - Move 1 north
 * 1 - Move 1 south
 * 2 - Move 1 east
 * 3 - Move 1 west
 * 4 - Move 1 random direction
 */

const double mutation_rate = 0.05;	// Rate of mutation during individual gene swapping
const double corruption_rate = 0.1;	// Rate of corruption in sensor reading, i.e. when a sensor will give reading 3

class Robot
{
public:
	vector<vector<int> > genes;	// Vector to store 16 genes, where each gene will be a vector of size 5
	int energy;					// To store the robot's current energy, which starts with a value of 5
	int time_alive;				// To store how many moves the robot was able to stay alive
	int energy_harvested;		// To store the energy harvested from batteries
	int generations_survived;	// To store the number of generations a particular Robot instance survived
	pair<int, int> pos;			// To store the current position of the robot on the map (stored as a pair of row and column numbers)
	vector<int> sensor;			// Vector to store the current sensor readings of the 4 sensors

	// Default constructor, where all member variables except the gene sensor readings are set to 0 or default values.
	Robot():	
		genes(16, vector<int>(5, 0)),
		energy(5),
		time_alive(0),
		energy_harvested(0),
		generations_survived(0),
		pos(0, 0),
		sensor(4, 0)
	{
		// Iterate over all genes and put 4 random sensor readings to serve as gene codes
		for(int i = 0; i < 16; i++)
			for(int j = 0; j < 4; j++)
				// Storing a random sensor reading between 0-3
				genes[i][j] = rand() % 4;
	}

	// Parameterized constructor to construct an instance with a specific inital position
	Robot(pair<int, int> p):
		genes(16, vector<int>(5, 0)),
		energy(5),
		time_alive(0),
		energy_harvested(0),
		generations_survived(0),
		pos(p),
		sensor(4, 0)
	{
		// Iterate over all genes and put 4 random sensor readings to serve as gene codes
		for(int i = 0; i < 16; i++)
			for(int j = 0; j < 4; j++)
				genes[i][j] = rand() % 4;
	}

	// Parameterized constructor to construct a child of two parent nodes by mixing their genes
	// All member variables are initialized by default constructors, then genes are created by mixing the genes of parents.
	Robot(Robot &bot1, Robot &bot2):
		genes(16, vector<int>(5, 0)),
		energy(5),
		time_alive(0),
		energy_harvested(0),
		generations_survived(0),
		pos(0, 0),
		sensor(4, 0)
	{
		// Iterate over the genes vector
		for(int i = 0; i < 16; i++)
		{
			// Use bot1's genes for even indices, and bot2's genes for odd indices.
			// This means that the last (15th) gene is inherited from bot2
			if(i%2 == 0)
				genes[i] = bot1.genes[i];
			else
				genes[i] = bot2.genes[i];

			// Generate a random number between 0-1
			double temp = ((double) rand() / (RAND_MAX)) + 1;

			// temp will be less than mutation_rate with the same probability
			if(temp < mutation_rate)
			{
				// In this case one of the gene character's are mutated.
				int y = rand()%5;	// Selects a random character in the i'th gene to mutate
				// The last character on a gene can be 0-4, while others can be 0-3
				if(y == 4)
					genes[i][y] = rand()%5;
				else
					genes[i][y] = rand()%4;
			}
		}
	}

	// Defining < operator for the Robot class in order to use std::sort() function
	bool operator< (const Robot &other) const {
		// Returning this.energy_harvested > other.energy_harvested so that they can be sorted in descending order of energy_harvested
        return energy_harvested > other.energy_harvested;
    }

    // Function to reset necessary variables of a robot before simulating a generation,
    // so that previous generation's values don't get carried over to new generations.
    void resetParameters()
    {
    	time_alive = 0;
		energy = 5;
		energy_harvested = 0;
		allocate_random_position();
    }

    // Function to move the robot in the direction on the map passed as parameteres.
    //
    // direction : 1 - North
    //             2 - South
    //             3 - East
    //             4 - West
    // 
    // map : map on which the robot is searching for batteries
	void move(int direction, vector<vector<char> > &map)
	{
		// The row or column number is updated depending on the direction if the current position is not at the boundary (i.e. against the wall)
		if(direction == 0 && pos.first > 0)
			pos.first--;
		else if(direction == 1 && pos.first < 9)
			pos.first++;
		else if(direction == 2 && pos.second < 9)
			pos.second++;
		else if(direction == 3 && pos.second > 0)
			pos.second--;

		// If the new position has a battery
		if(map[pos.first][pos.second] == 'B')
		{
			// Change the map character to 'x', to denote this battery has been picked up
			map[pos.first][pos.second] = 'x';

			// Increase the current energy available, and energy harvested by 5
			energy += 5;
			energy_harvested += 5;
		}
		else if(map[pos.first][pos.second] == ' ')
		{
			// To represent the movement of the robot, change map empty map characters to '*'
			map[pos.first][pos.second] = '*';
		}
	}

	// Function to allocate a random initial location for the robot 
	void allocate_random_position()
	{
		pos = pair<int, int>(rand()%10, rand()%10);
	}

	// Function to allocate random action values, i.e the put random values in the last index of each gene
	void allocate_random_actions()
	{
		for(int i = 0; i < 16; i++)
		{
			// Set a random value among the possible 0-4
			genes[i][4] = rand() % 5;
		}
	}

	// Function to get readings for the sensor pointing in the given direction
	void readSensor(int direction, vector<vector<char> > &map)
	{
		int x, y; // x and y store the position to be inspected

		// sensor vector stores the readings of different directions at different indices
		// 0 - North Sensor
		// 1 - South sensor
		// 2 - East sensor
		// 3 - West sensor
		if(direction == 0)
		{
			x = pos.first - 1;
			y = pos.second;
		}
		else if(direction == 1)
		{
			x = pos.first + 1;
			y = pos.second;
		}
		else if(direction == 2)
		{
			x = pos.first;
			y = pos.second + 1;
		}
		else
		{
			x = pos.first;
			y = pos.second - 1;
		}

		// Generate a random number between 0-1
		double temp = ((double) rand() / (RAND_MAX)) + 1;

		// temp will be less than corruption_rate with the same probability
		if(temp < corruption_rate)
		{
			// Sensor reading is corrupted
			sensor[direction] = 3;
		}
		else
		{
			if(x == -1 || x == 10 || y == -1 || y == 10)
			{
				// If the position to be inspected lies beyond the boundary, a wall object is detected
				sensor[direction] = 1;
			}
			else if(map[x][y] == 'B')
			{
				// A battery object is detected
				sensor[direction] = 2;
			}
			else
			{
				// Any other characters represents an empty location
				sensor[direction] = 0;
			}
		}
	}

	// Function to read all 4 sensors
	void readAllSeansors(vector<vector<char> > &map)
	{
		for(int i = 0; i < 4; i++)
			readSensor(i, map);
	}

	// Function to return the action corresponding to the sensor readings.
	// If no gene matches the sensor readings, the action corresponding to the last gene is returned.
	int compareGenes()
	{
		// Iterate through all 16 genes
		for(int i = 0; i < 16; i++)
		{
			// Assume the readings match the i'th gene.
			bool flag = true;
			for(int j = 0; j < 4; j++)
			{
				// If any of the 4 codes do not match the sensor readings, set flag to false
				if(sensor[j] != genes[i][j])
					flag = false;
			}
			// If the i'th gene matches the sensor readings, return it.
			if(flag) return genes[i][4];
		}
		// If no gene matches, return the last action value.
		return genes[15][4];
	}

	// Perform the action passed as a parameter, which will be moving in some direction on the passed map.
	void performAction(int action, vector<vector<char> > &map)
	{
		// Any action, valid or invalid, consumed energy.
		energy--;
		if(action < 4)
		{
			// Actions 0-3 correspond to moving in the denoted direction
			move(action, map);
		}
		else
		{
			// Action 4 corresponds to moving in a random direction
			move(rand()%4, map);
		}
	}
};

// Function to kill the 50% worst performing robots and reproduce new robots from the remaining robots
void reproduce(vector<Robot> &bots)
{
	// Sort the bots vector using the sort() function in the stl library.
	// It will use the operator '<' defined for the class Robots.
	sort(bots.begin(), bots.end());

	// Killing/Erasing the bots stored in indices 100 to the end
	//vector<Robot>::iterator it = bots.begin() + 100;
	bots.erase(bots.begin() + 100, bots.end());

	// Increase the generations_survived counter for each of the surviving robots
	for(int i = 0; i < bots.size(); i++)
		bots[i].generations_survived++;

	// Now we need to produce children robots to be stored in indices 100-199 of the bots vector.
	// We have 100 surviving robots that make 50 consecutive pairs.
	// For indices 100 to 149, consecutive pairs of parent robots are taken, i.e (1st+2th) and so on.

	int parent = 0; // Variable to help keep track of the parents to be referenced
	for(int i = 100; i < 150; i++)
	{
		// The better robot is kept as the second parameter because it will contribute the last gene of the child
		bots.push_back(Robot(bots[parent+1], bots[parent]));
		parent += 2;
	}

	// And, for indices 150 to 199, parents are matched with the scheme (1st+4th) and (2nd+3rd), and so on.
	parent = 0;
	for(int i = 150; i < 200; i += 2)
	{
		bots.push_back(Robot(bots[parent+3], bots[parent]));
		bots.push_back(Robot(bots[parent+2], bots[parent+1]));
		parent += 4;
	}
}

// Helper function to clear screen in Windows and Linux
void clear_screen()
{
	#ifdef _WIN32
	    system("cls");
	#else
	    system ("clear");
	#endif
}

// Function to print any map. It shall be used to show the maps of traversal of robots
void showMap(vector<vector<char> > &map)
{
	// The '_' and '|' characters are used to draw a boundary.
	cout << " _____________________ \n";
	for(int i = 0; i < 10; i++)
	{
		cout << "| ";
		for(int j = 0; j < 10; j++)
			cout << map[i][j] << " ";
		cout << "|\n";
	}
	cout << "|_____________________|\n";
}

// The main function to run this entire application and show appropriate outputs
int main()
{
	srand(time(0));

	vector<vector<char> > map(10, vector<char>(10, ' '));
	// map - Variable to store a template of an empty map, used to clear map after each robot simmulation.
	//       It is a vector of size 10, each containing a 10 sized character vector.

	// Printing the introduction and usage options
	clear_screen();
	cout << "Welcome,\nName - Omar\nDate - 05/14/2021\n\n"
			"This program simmulates a Genetic Algorithm where we observe\n"
			"the average enery harvested by a population of robots.\n\n"
			"The robots are placed on a 10x10 map one by one, with 40% places\n"
			"filled with batteries.\n\n"
			"Legend of the map representation:\n"
			"B - Batteries\n"
			"S - Start position of robot\n"
			"E - End position of robot\n"
			"* - Places visited by the robot\n"
			"x - Places where a battery was picked up\n"
			"s - Start position, if the robot ends at the same place\n\n"
			"Enter a choice 1-3:\n"
			"1 - Simmulate one generation at a time (Showing map of each robot)\n"
			"2 - Simmulate 100 generations (Showing only map of the best robot in each generation)\n"
			"3 - Exit\n";
	
	char start; 	// Stores the starting choice entered by the user.
	int gen_limit;	// Stores the number of generations to simmulate
	cin >> start;

	// For the first choice, gen_limit will be 1. And it will be 100 for the second choice
	if(start == '1')
		gen_limit = 1;
	else if(start == '2')
		gen_limit = 100;
	else
		exit(0);

	cout << "Starting simmulation....\n";

	vector<Robot> bots; // Vector to store a population of 200 robots
	for(int i = 0; i < 200; i++)
	{
		// Create a new Robot object and allocate random starting positions, as well as random gene actions
		bots.push_back(Robot());
		bots[i].allocate_random_position();
		bots[i].allocate_random_actions();
	}

	int generation = 0;			// Counter for generations being simmulated.
	int oldest_bot_age = 0;		// Stores the largest value of generations_survived among all the robots throughout the simmulation.
	vector<double> avg_scores;	// Vector to store the average fitness scores of each generation.
	char ch = 'N';				// Variable to read user choice on continuing the simmulation

	do
	{
		generation++;

		// If ch is 'Y', this is not the first generation and we need to generate the new population using reproduction
		if(ch == 'y' || ch == 'Y')
		{
			cout << "\nSummulating next generation......\n";

			// Use the helper function to kill 50% worst performing robots and produce the next population from the remaining.
			reproduce(bots);

			// Checking the population for the largest value of generations_survived.
			for(int i = 0; i < 200; i++)
			{
				if(bots[i].generations_survived > oldest_bot_age)
					oldest_bot_age = bots[i].generations_survived;
			}

			// Reset the necessary parameters of all the robots, which may have been changed in the previous generation.
			for(int i = 0; i < 200; i++)
				bots[i].resetParameters();
		}

		int most_energy_harvested = 0;	// To store the largest amount of energy_harvested within this generation
		double avg_fitness = 0;			// To stores the average fitness of this generation
		vector<vector<char> > best_map;	// To store away the map of the robot that harvests the most energy.

		// Iterate iver all robots
		for(int i = 0; i < 200; i++)
		{
			// Make a copy of the empty map for the i'th robot.
			vector<vector<char> > curr_map = map;
			
			// Randomly fill 40 locations with batteries
			for(int j = 0; j < 40; j++)
				curr_map[rand()%10][rand()%10] = 'B';

			// Mark the starting position of the robot.
			curr_map[bots[i].pos.first][bots[i].pos.second] = 'S';

			// The robot stays alive untill its energy reaces zero.
			while(bots[i].energy > 0)
			{
				bots[i].time_alive++;

				// Get readings from all the sensors and get the corresponding action to be performed according to the gene table
				bots[i].readAllSeansors(curr_map);
				int action = bots[i].compareGenes();

				// Perform the above determined action
				bots[i].performAction(action, curr_map);
			}

			//If the final position of the robot is same as the start, mark it with 's', otherwise mark it with 'E'.
			if(curr_map[bots[i].pos.first][bots[i].pos.second] == 'S')
				curr_map[bots[i].pos.first][bots[i].pos.second] = 's';
			else
				curr_map[bots[i].pos.first][bots[i].pos.second] = 'E';

			// If only one generation is being simmulated at a time, show the maps of each robots
			if(start == '1')
			{
				clear_screen();
				cout << "Generation - " << generation << " | Robot - " << i << "\n";
				cout << "=============================\n";
				showMap(curr_map);
			}	

			// Add the contribution of the current robot to the average fitness score of the current generation.
			avg_fitness += (double)(bots[i].energy_harvested) / 200.0;

			// If the current robot has harvested more energy than any robot till now, update most_energy_harvested and best_map
			if(bots[i].energy_harvested > most_energy_harvested)
			{
				most_energy_harvested = bots[i].energy_harvested;
				best_map = curr_map;
			}
		}

		avg_scores.push_back(avg_fitness); // Store the average fitness score of this generation

		// If only one generation is being simmulated at a time:
		if(start == '1')
		{
			// Wait for some time before clearing the screen, so that the maps shown are atleast visible .
			usleep(100000);
			clear_screen();

			// Show the map of the robot that harvested the most energy
			cout << "Generation - " << generation << " | Map of the Robot harvesting most energy:\n";
			cout << "========================================================\n";
			showMap(best_map);
			cout << "\nEnergy harvested by this robot = " << most_energy_harvested << "\n";

			cout << "\nAverage fitness of Generation " << generation << " => " << avg_fitness << "\n";

			// Ask the user if they want to simmulate another generation.
			cout << "\nSimulate next generation?\nEnter 'Y' to continue or 'N' to end simmulation.\n";
			cin >> ch;

			// Keep increasing the gen_limit to avoid loop termination even if the user enters 'Y'
			gen_limit++;
		}
		else
		{
			clear_screen();

			cout << "Generation - " << generation << " | Map of the Robot harvesting most energy:\n";
			cout << "========================================================\n";
			showMap(best_map);
			cout << "\nEnergy harvested by this robot = " << most_energy_harvested << "\n";

			cout << "\nAverage fitness of Generation " << generation << " => " << avg_fitness << "\n";

			// Change ch to 'Y' to avoid termination of loop before gen_limit is reached.
			ch = 'Y';
		}
	} while((ch == 'y' || ch == 'Y') && (generation < gen_limit));

	cout << "\nEnding simmulation.......\n";

	// Print the summary details of the whole simmulation
	cout << "\nLongest time a robot survived = " << oldest_bot_age << " generations\n";

	cout << "\nAverage fitness scores:\n";
	cout << "======================\n";
	for(int i = 0; i < generation; i++)
		cout << "Generation " << i+1 << " => " << avg_scores[i] << "\n";
}

/* Observations:
 * ============
 * 1. The initial random allocation of gene codes and their corresponding actions greatly affects the speed at which the
 *    average fitness score grows over generations.
 *    That is because if the gene pools are overall poorly made, but some robots performed well by chance, the subsequent
 *    generations will keep carrying those genes to a large extent as there is only 5% chance of mutation.
 * 2. In case of a good inital geen pool, the average fitness scores grow fairly quicky. But for some reason, this score
 *    kind of plateaus around 30-35 energy harvested. Sometimes this plateau is seen around 20 also.
 *    This may be related to the size of the map and the 40% distribution of batteries.
 * 3. On increasing the mutation rate, we observe a faster rate of growth in average fitness score, but the value fluctuates
 *    a lot more. It increses and decreases more dramatically between generations.
 * 4. On increasing the corruption rate of the sensor readings, the growth rate seems to consistently become slower.
 * 5. Finally, as we know that is infact a pseudo-random, whose seed we have passed as the time. But the randomness of this
 *    whole process for getting random numbers is very debatable. Thus, the randomness of this algorithm can also not be
 *    shown to a great extent.
 */