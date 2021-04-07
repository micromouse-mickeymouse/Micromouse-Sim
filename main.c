#include <stdio.h>
#include <string.h>

#include "API.h"

#define QUEUE_MAX 100

char debug_str[80];

// Queue for flood fill
int flood_fill_queue[QUEUE_MAX][2] = { [ 0 ... (QUEUE_MAX-1) ] = {-1, -1} };
int queue_front = 0;
int queue_back = -1;
int item_count = 0;

// Constants
int goal_pos[4][2] = {{3,2},{3,3},{4,2},{4,3}};

// Maze and robot gloabls
int current_coords[2] = {0,0};
int current_direction = 0;

int maze_map[8][6] = {
    {5, 1, 1, 1, 1, 8},
    {4, 0, 0, 0, 0, 2},
    {4, 0, 0, 0, 0, 2},
    {4, 0, 0, 0, 0, 2},
    {4, 0, 0, 0, 0, 2},
    {4, 0, 0, 0, 0, 2},
    {4, 0, 0, 0, 0, 2},
    {6, 3, 3, 3, 3, 7},
};

int flood_map[8][6] = {
    {10, 9, 8, 7, 6, 6},
    {9, 8, 7, 6, 5, 5},
    {8, 7, 6, 5, 4, 4},
    {7, 6, 5, 4, 3, 3},
    {6, 5, 4, 3, 2, 2},
    {5, 4, 3, 2, 1, 1},
    {4, 3, 2, 1, 0, 0},
    {4, 3, 2, 1, 0, 0},
};

int blank_flood_map[8][6] = {
    {-1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1},
};

void log(char* text) {
    fprintf(stderr, "%s\n", text);
    fflush(stderr);
}

// ========================================================
// Wall discovery
// ========================================================

void discover_walls(){
	int left = API_wallLeft();
    int right = API_wallRight();
    int front = API_wallFront();
    
    int x = current_coords[0];
    int y = current_coords[1];

    int wall_values[4] = {0,0,0,0};

    // If walls are on left, right, and front
    if (left && right && front){
        int potential_wall_values[4] = {13, 12, 11, 14};
        memcpy(wall_values, potential_wall_values, sizeof(potential_wall_values));
    }

    // If walls are on left and right
    else if (left && right && !front){
        int potential_wall_values[4] = {9, 10, 9, 10};
        memcpy(wall_values, potential_wall_values, sizeof(potential_wall_values));
    }

    // If walls are on left and front
    else if (left && !right && front){
    	int potential_wall_values[4] = {8, 7, 6, 5};
        memcpy(wall_values, potential_wall_values, sizeof(potential_wall_values));
    }

    // If walls are on right, and front
    else if (!left && right && front){
    	int potential_wall_values[4] = {7, 6, 5, 8};
        memcpy(wall_values, potential_wall_values, sizeof(potential_wall_values));
    }

    // If a wall is just in front
    else if (front){
        int potential_wall_values[4] = {2, 3, 4, 1};
        memcpy(wall_values, potential_wall_values, sizeof(potential_wall_values));
    }

    // If a wall is just on the left
    else if (left){
        int potential_wall_values[4] = {1, 2, 3, 4};
        memcpy(wall_values, potential_wall_values, sizeof(potential_wall_values));
    }

    // If a wall is just on the right
    else if (right){
        int potential_wall_values[4] = {3, 4, 1, 2};
        memcpy(wall_values, potential_wall_values, sizeof(potential_wall_values));
    }

    // update maze map with new wall infomation for debug
	maze_map[x][y] = wall_values[current_direction];
}

// ========================================================
// Flood fill
// ========================================================

int is_accsessable(int x, int y, int direction){
    int wall_layout = maze_map[x][y];

    sprintf(debug_str, "coord: %d, %d - Wall layout %d", x,y, wall_layout);
    log(debug_str);

    switch (direction)
    {
    // If the direction is up and the wall position value has a wall above it return false
    case (0):
        if (wall_layout == 2 ||
            wall_layout == 7 ||
            wall_layout == 8 ||
            wall_layout == 10 ||
            wall_layout == 12 ||
            wall_layout == 13 ||
            wall_layout == 14 ||
            wall_layout == 16){
                return 0;
            }
        break;

    // If the direction is right and the wall position value has a wall to the right return false
    case (1):
        if (wall_layout == 3 ||
            wall_layout == 6 ||
            wall_layout == 7 ||
            wall_layout == 9 ||
            wall_layout == 11 ||
            wall_layout == 12 ||
            wall_layout == 13 ||
            wall_layout == 16){
                return 0;
            }
        break;

    // If the direction is down and the wall position value has a wall below return false
    case (2):
        if (wall_layout == 4 ||
            wall_layout == 5 ||
            wall_layout == 6 ||
            wall_layout == 10 ||
            wall_layout == 11 ||
            wall_layout == 12 ||
            wall_layout == 14 ||
            wall_layout == 16){
                return 0;
            }
        break;

    // If the direction is left and the wall position value has a wall to the left return false
    case (3):
        if (wall_layout == 1 ||
            wall_layout == 5 ||
            wall_layout == 8 ||
            wall_layout == 9 ||
            wall_layout == 11 ||
            wall_layout == 13 ||
            wall_layout == 14 ||
            wall_layout == 16){
                return 0;
            }
        break;

    default:
        break;
    }

    // IF no wall detected then return true
    return 1;
}

int queue_empty(){
    return(item_count <= 0);
}

void queue_add(int x, int y) {
    // If the queue is not full
    if(item_count != QUEUE_MAX) {
        
        // resets end of the queue if the max is reached 
        if(queue_back == QUEUE_MAX-1) {
            queue_back = -1;          
        }
        // set the new index of last in queue
        queue_back++;

        // Add data
        flood_fill_queue[queue_back][0] = x;
        flood_fill_queue[queue_back][1] = y;

        // increase number in queue
        item_count++;
   }
}

void queue_pop(int *output_coords) {
    // Get the x and y data and saves it to the output coords array
    output_coords[0] = flood_fill_queue[queue_front][0];
    output_coords[1] = flood_fill_queue[queue_front][1];

    // Set new index of first in queue
    queue_front++;

    // Reset if front is the MAX value
    if(queue_front == QUEUE_MAX) {
        queue_front = 0;
    }
    
    // Decrease number in queue
    item_count--;
}

void update_simulator_map(int x,int y){
    sprintf(debug_str, "%d", flood_map[x][y]);
    API_setText(x, y, debug_str);
}

void fill_out(x, y){
    // If the top is accsessable from the bttom and the bottom is accsessable from the top fill in flood value
    if (is_accsessable(x, y, 0) && is_accsessable(x, y+1, 2)){
        if (flood_map[x][y+1] == -1){
            flood_map[x][y+1] = flood_map[x][y] + 1;
            // add to the flood fill queue
            queue_add(x, y+1);
        }
    }

    // If the right is accsessable from the left and left is accsessable from the right fill in flood value
    if (is_accsessable(x, y, 1) && is_accsessable(x+1, y, 3)){
        if (flood_map[x+1][y] == -1){
            flood_map[x+1][y] = flood_map[x][y] + 1;
            // add to the flood fill queue
            queue_add(x+1, y);
        }
    }

    // If the bottom is accsessable from the top and the top is accsessable from the bottom fill in flood value
    if (is_accsessable(x, y, 2) && is_accsessable(x, y-1, 0)){
        if (flood_map[x][y-1] == -1){
            flood_map[x][y-1] = flood_map[x][y] + 1;
            // add to the flood fill queue
            queue_add(x, y-1);
        }
    }

    // If the left is accsessable from the right and the right is accsessable from the left fill in flood value
    if (is_accsessable(x, y, 3) && is_accsessable(x-1, y, 1)){
        if (flood_map[x-1][y] == -1){
            flood_map[x-1][y] = flood_map[x][y] + 1;
            // add to the flood fill queue#
            queue_add(x-1, y);
        }
    }

    // Update map with the latest data
    update_simulator_map(x, y+1);
    update_simulator_map(x+1, y);
    update_simulator_map(x, y-1);
    update_simulator_map(x-1, y);
}

void flood_fill(){
    int current_flood_coords[2];

    // Reset flood fill map
    memcpy(flood_map, blank_flood_map, sizeof(blank_flood_map));

    // Set the goal positions to 0
    for (int i=0;i<4;i++){
        flood_map[goal_pos[i][0]][goal_pos[i][1]] = 0;

        // Add the goal current position to the flood fill queue
        queue_add(goal_pos[i][0], goal_pos[i][1]);
    }

    // While the queue still contains coords to calculate
    while (!queue_empty()){
        queue_pop(current_flood_coords);
        fill_out(current_flood_coords[0], current_flood_coords[1]);
    }

}

// ========================================================
// Movement
// ========================================================

void make_turn(int left_or_right){
    // If the robot is turing left
    if (left_or_right == 0){

        // Update the direction indicator without going below 0
        if(current_direction == 0){
            current_direction = 3;
        } else{
            current_direction = current_direction - 1;
        }

        // Turn left
        API_turnLeft();
    }

    // IF the robot is turing right
    if (left_or_right == 1){
        // Update direction indicator without going above 3
        current_direction = (current_direction + 1) % 4;

        // turn right
        API_turnRight();
    }
}

void move_forward(){
    // Move robot forward
    API_moveForward();

    // Update current coord
    switch (current_direction){
        case(0):
            current_coords[1] += 1;
            break;
        case(1):
            current_coords[0] += 1;
            break;
        case(2):
            current_coords[1] -= 1;
            break;
        case(3):
            current_coords[0] -= 1;
            break;
    }

}

int move_to_next_square(){
    int x = current_coords[0];
    int y = current_coords[1];

    // Find the coords around the current coordinate
    int surrounding_coords[4][2] = {{x,y+1},{x+1,y},{x,y-1},{x-1,y}};

    // Set up varilbes for loop
    int smallest_flood_map_value = 99;
    int direction_to_move = -1;
    int current_flood_map_value = 999;

    // For each direction around the current coord
    for(int i=0;i<4;i++){
        // If coordinates are off the maze grid
        if (surrounding_coords[i][0] > 7 || surrounding_coords[i][0] < 0){
            continue;
        }
        if (surrounding_coords[i][1] > 5 || surrounding_coords[i][1] < 0){
            continue;
        }
        
        // Check that the surrounding coord is accsessable from current positon
        if (is_accsessable(x,y,i)){
            int current_x = surrounding_coords[i][0];
            int current_y = surrounding_coords[i][1];

            // Get the current value within the coord using the flood map
            current_flood_map_value = flood_map[current_x][current_y];
            if (current_flood_map_value < smallest_flood_map_value){
                // Set the new smallest value around the current coord
                smallest_flood_map_value = current_flood_map_value;

                // log the direction that we need to move in
                direction_to_move = i;
            }
        }
    }

    // While we are not facing the correct direction
    while(current_direction != direction_to_move){
        // If it takes one turn right to get there turn right
        if ((current_direction + 1) % 4 == direction_to_move){
            make_turn(1);
        }
        // Else just trun left until we have reached out desired direction
        else{
            make_turn(0);
        }
    }
    // Move forward
    move_forward();

    if(flood_map[current_coords[0]][current_coords[1]] == 0){
        return(1);
    } else {
        return(0);
    }
}

int main(int argc, char* argv[]) {
    int at_goal = 0;

    // WHile not at the finish repeat the main loop
    while (!at_goal) {
        discover_walls();
        flood_fill();
        at_goal = move_to_next_square();
    }

    // Reset mouse back to the start
    API_ackReset();

    // Reset all relivent mouse varibles while leaving the flood map the same
    at_goal = 0;
    current_coords[0] = 0;
    current_coords[1] = 0;
    current_direction = 0;

    // Move to the finish
    while(!at_goal){
        at_goal = move_to_next_square();
    }
}