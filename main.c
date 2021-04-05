#include <stdio.h>
#include <string.h>

#include "API.h"

int maze_map[8][6] = {
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0},
};

void log(char* text) {
    fprintf(stderr, "%s\n", text);
    fflush(stderr);
}

void discover_walls(int *maze_map, int *current_position, int current_direction){
	int left = API_wallLeft();
    int right = API_wallRight();
    int front = API_wallFront();
    
    int x = current_position[0];
    int y = current_position[1];

    int wall_values[4] = {0,0,0,0};

    char debug_str[80];

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
    sprintf(debug_str, "%d", wall_values[current_direction]);

    API_setText(x, y, debug_str);
	// maze_map[x][y] = potential_wall_values[current_direction]
}

int make_turn(int left_or_right, int current_direction){

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

    // Return the current direction
    return (current_direction);
}

void move_forward(int *current_position, int direction){
    // Move robot forward
    API_moveForward();

    // Update current coord
    switch (direction){
        case(0):
            current_position[1] += 1;
            break;
        case(1):
            current_position[0] += 1;
            break;
        case(2):
            current_position[1] -= 1;
            break;
        case(3):
            current_position[0] -= 1;
            break;
    }

}


int main(int argc, char* argv[]) {
    log("Running...");


    int current_coords[2] = {0,0};
    int direction = 0;
    
    while (1) {
        discover_walls(maze_map, current_coords, direction);
        if (!API_wallLeft()) {
            direction = make_turn(0, direction);
        }
        while (API_wallFront()) {
            direction = make_turn(1, direction);
        }

        move_forward(current_coords, direction);
    }
}
