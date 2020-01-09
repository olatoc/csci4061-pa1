/*
* test machine: csel-kh1250-01.cselabs.umn.edu
* date due: October 9th, 2019
* name: Oliver Latocki, Jonathan Sulisz
* x500: latoc004, sulis005
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <unistd.h>

#include "graph.h"
#include "dfs_stack.h"

//Parse the input makefile to determine targets, dependencies, and recipes
int process_file(char *fname)
{
	FILE* fp = fopen(fname, "r");
	char line[LINE_SIZE];
	int i = 0;

	if (!fp) {
		printf("Failed to open the file: %s \n", fname);
		return -1;
	}

	//Read the contents and store in lines
	while (fgets(line, LINE_SIZE, fp)) 
		strncpy(lines[i++], line, strlen(line));
		//fills up 2d array liines with lines of fp


	fclose(fp);

	return 0;
}

//Validate the input arguments, bullet proof the program

//Array of target * to keep track of targets in Makefile
target *targets[MAX_TARGETS];
//Number of targets
int t_count = 0;

/*
 * This function fills the dependency array of a target at inded t_target of the targets[] array
*/
void find_dep(int t_index, int row){
	char* token; 
	char* temp_row = lines[row];
	int i = 0;
	int temp_dep_count = 0;
	//separate dependencies from target via the ":" delimiter
	token = __strtok_r(temp_row, ":", &temp_row);
	while ((token = __strtok_r(temp_row, " ", &temp_row))){
		//check if target has 0 dependencies
		if (token[0] == '\0' || token[0] == '\n'){
			targets[t_index]->dep_count = 0;
			return;
		}
		//fill dependency array
		targets[t_index]->depend[i] = token;
		temp_dep_count += 1;
		i++;
	}
	//Remove newline from last dependency
	targets[t_index]->depend[temp_dep_count - 1][strcspn(targets[t_index]->depend[temp_dep_count - 1], "\n")] = 0;
	targets[t_index]->dep_count = temp_dep_count;
	return;
}

/*
 * This function fills the recipe array for a target at t_index of the targets[] array
*/
void find_recipe(int t_index, int row){
	targets[t_index]->recipe_count = 0;
	int row_offset = 1;
	int recipe_i = 0;
	//fill targets[t_index]->recipe with recipes from file
	while (lines[row + row_offset][0] == '\t'){
		char recipe_line[LINE_SIZE];
		strcpy(recipe_line, lines[row + row_offset]);
		//remove \t char from beginning of recipe
		if (recipe_line[0] == '\t'){
    		memmove(recipe_line, recipe_line+1, strlen(recipe_line));
		}
		//remove \n from end of recipe
		recipe_line[strcspn(recipe_line, "\n")] = 0;
		targets[t_index]->recipe[recipe_i] = (char*)malloc(sizeof(char)*LINE_SIZE);
		strcpy(targets[t_index]->recipe[recipe_i], recipe_line);

		row_offset++;
		recipe_i++;
		targets[t_index]->recipe_count++;
	}
}

void fill_targets(){
	int row = 0;
	int t_index = 0;
	//find targets by iterating through rows of lines
	while (lines[row][0] != '\0'){
		//Parse this row; find target and dependencies
		if (lines[row][0] != '\t' && lines[row][0] != '\n'){
			//This line has a target; allocate memory for new target
			targets[t_index] = (target *)malloc(sizeof(target));
			//find targets[t_index]->name
			char line[LINE_SIZE];
			strcpy(line, lines[row]);
			strtok(line, ":");
			//remove extra spaces around name
			line[strcspn(line, " ")] = 0;
			targets[t_index]->name = (char*)malloc(sizeof(char)*128);
			strcpy(targets[t_index]->name, line);

			//find dependencies for target
			find_dep(t_index, row);

			//find recipes for target
			find_recipe(t_index, row);
			t_index++; //increment target index
			t_count++; //increment target count
		}
		row++;
	}

}
/* This function prints the contents of targets[], which includes 
 * the name, dependencies, and recipes of a target
 */
void print_targets(){
	for (int i = 0; i < t_count; i++){
		printf("target '%s' has %d dependencies and %d recipes\n",
							targets[i]->name, targets[i]->dep_count, targets[i]->recipe_count);
		for (int k = 0; k < targets[i]->dep_count; k++){
			printf("Dependency %d: %s\n", k, targets[i]->depend[k]);
		}
		for (int k = 0; k < targets[i]->recipe_count; k++){
			printf("Recipe %d: %s\n", k, targets[i]->recipe[k]);
		}
		if (i + 1 < t_count){
			printf("\n");
		}
	}
}

/* 
	Functions for printing and executing recipes of targets[]
*/
/*
	This function executes the recipe given as an argument
	by calling fork() and execvp() on the child process
*/
void fork_exec(char *recipe){
	//fork process for execution
	int c_pid = fork();
	if (c_pid < 0){
		printf("Error creating process\n");
	} else if (c_pid == 0){
		/* Child process; execute recipe */
		char *args[MAX_PARM];	
    	char* token;
    	char* rest = recipe;
		int i = 0;
		//fill arguments of execvp with recipe command and flags
    	while ((token = __strtok_r(rest, " ", &rest))){
			args[i] = token;
			i++;
		}
		//terminate args with null
		args[i] = NULL;
		execvp(args[0], args);
	}
}

/* Find target in targets[] array for DFS if it exists */
target *find_target_by_name(char *name){
	for (int i = 0; i < t_count; i++){
		if (!strcmp(targets[i]->name, name)){
			return targets[i];
		}
	}
	return NULL;
}

/*
	Use DFS to traverse the dependency tree starting at target "t"
	and print the recipes in the correct order of dependencies
	Used in ./mymake -r Makefile
*/
void print_tree_recipes(target *t){
	//check for NULL target pass
	if (t == NULL){
		return;
	}
	/* Print dependencies of leaf target */
	if (t->dep_count == 0){
		for (int i = 0; i < t->recipe_count; i++){
			printf("%s\n", t->recipe[i]);
		}
		return;
	}
	/* Recursively call print_tree_recipes as long as target has unsatisfied dependencies */
	else if (t->dep_count > 0){
		for (int i = 0; i < t->dep_count; i++){
			if (find_target_by_name(t->depend[i]) != NULL){
				print_tree_recipes(find_target_by_name(t->depend[i]));
			}
		}
		/* Print recipes once all dependencies have printed theirs */
		for (int i = 0; i < t->recipe_count; i++){
			printf("%s\n", t->recipe[i]);
		}
	}
}

/* 
	Same as print_tree_recipes, but executes rather than prints recipes 
	in the correct order of dependencies
*/
void exec_tree(target *t){
	if (t == NULL){
		return;
	}
	/* Execute dependencies of leaf target */
	if (t->dep_count == 0){
		for (int i = 0; i < t->recipe_count; i++){
			fork_exec(t->recipe[i]);
			wait(NULL);
		}
		return;
	}
	/* Recursively call print_tree_recipes as long as target has unsatisfied dependencies */
	else if (t->dep_count > 0){
		for (int i = 0; i < t->dep_count; i++){
			if (find_target_by_name(t->depend[i]) != NULL){
				exec_tree(find_target_by_name(t->depend[i]));
			}
		}
		/* Execute recipes once all dependencies have executed theirs */
		for (int i = 0; i < t->recipe_count; i++){
			fork_exec(t->recipe[i]);
			wait(NULL);
		}
	}
}


int main(int argc, char *argv[]) 
{
	//$./mymake Makefile
	//Similarly account for -r flag
	if (argc == 2 && strncmp(argv[1], "-p", 2) && strncmp(argv[1], "-r", 2)) {
		process_file(argv[1]);
		//TODO
		fill_targets();
		exec_tree(targets[0]);
	}
	if (argc == 3) {
		//$./mymake Makefile target
		if ((strncmp(argv[1], "-r", 2) && strncmp(argv[1], "-p", 2)) && !process_file(argv[1])) {
			//TODO
			//NO -p and NO -r
			//Find specified target in argv[2]
			char *temp_target_name = argv[2];
			if (!strncmp(temp_target_name,"-p",2) || !strncmp(temp_target_name,"-r",2)){
				printf("Failure: Incorrect argument order\n");
			}else{
				target *temp_target;
				fill_targets();
				temp_target = find_target_by_name(temp_target_name);
				if (temp_target != NULL){
					exec_tree(temp_target);
				} else{
					printf("Failure: target does not exist\n");
				}
			}
		} else if (!strncmp(argv[1], "-p", 2)) {
			//$./mymake -p Makefile
			// PRINT ALL TARGETS
			if (!process_file(argv[2])) {
				//TODO
				/* Print all targets, their dependencies and recipes in order they appear in Makefile */
				fill_targets();
				print_targets();
			}
		} else if (!strncmp(argv[1], "-r", 2)) {
			//$./mymake -r Makefile 
			// PRINT RECIPES IN CORRECT ORDER
			if (!process_file(argv[2])) {
				//TODO
				/* Print order of all recipes to produce first target in Makefile */
				fill_targets();
				print_tree_recipes(targets[0]);
			}
		}
	}
	if (argc > 3){
		printf("Failure: Too many arguments\n");
	}

	exit(EXIT_SUCCESS);
}


/* UNUSED CODE THAT DID NOT MAKE FINAL PROGRAM*/
//to be removed
// void print_specified_target(char *t){
// 	//printf("target: '%s'\n", t);
// 	for (int i = 0; i < t_count; i++){
// 		//printf("%d : %s\n", i, targets[i]->name);
// 		if (!strcmp(targets[i]->name, t)){
// 			printf("target '%s' has %d dependencies and %d recipes\n",
// 								targets[i]->name, targets[i]->dep_count, targets[i]->recipe_count);
// 			for (int k = 0; k < targets[i]->dep_count; k++){
// 				printf("Dependency %d: %s\n", k, targets[i]->depend[k]);
// 			}
// 			for (int k = 0; k < targets[i]->recipe_count; k++){
// 				printf("Recipe %d: %s\n", k, targets[i]->recipe[k]);
// 			}
// 			//printf("\n");
// 			return;
// 		}
// 	}
// }

/*
 * This function reads the row 'row' of 'lines' and returns it
 * Used for filling the recipe array of any target
*/
// char *parse_line(int row, int start_col){

// 	char *final_str;
// 	int line_size = 0;
// 	char cur = lines[row][start_col];
// 	int i = 0;
// 	//count length of line
// 	while (cur != '\n' && cur != '\0'){
// 		line_size += 1;
// 		i++;
// 		cur = lines[row][i + start_col];
// 	}
// 	i = 0;
// 	cur = lines[row][start_col];
// 	//fill final_str with contents of lines[row]
// 	final_str = (char *)malloc(sizeof(char)*(line_size + 1));
// 	while (cur != '\n' && cur != '\0'){
// 		final_str[i] = cur;
// 		i++;
// 		cur = lines[row][i + start_col];
// 	}
// 	return final_str;
// }
/*
-Variation of Main.c file, not all functions have been copied into this and some of main() relies on older functions that may not be in the current main.c file. Variables in this won't be compatiable with the submitted code as global variables have not been added here.
void print_(char* argv)
{
	//printf("Test case 10");
	int row = 0;
	int count = 0;
	char line[LINE_SIZE];
	while( lines[row][0] != '\0')
	{
		if( lines[row][0] != '\t' && lines[row][0] != '\n')
		{
			targets[count] = ( target *)malloc( sizeof( target ));	
			strcpy(line, lines[row]);
			strtok(line, ": ");
			targets[count]->name = (char*)malloc(sizeof(char)*126);
			strcpy(targets[count]->name, line);
			//required to fill
			find_dep(count, row, 0);
			find_recipe(count, row);
			//add counters and global
			count++;global_counter++;
		}
		row++;
	}
	if(!strcmp(argv,"-p"))
	{
		for (int x = 0; x < count; x++)
		{
			printf("target %s has %d dependencies and %d recipes\n", targets[x]->name,targets[x]->dep_count,targets[x]->recipe_count);
			for (int y = 0; y < targets[x]->dep_count;y++)
			{
			printf("\tDependency %d %s\n",y, targets[x]->depend[y]);
			}
			for (int z = 0; z< targets[x]->recipe_count; z++)
			{
			printf("\t\tRecipe %d %s\n", z, targets[x]->recipe[z]);
			}
		}
	}
	else if(!strcmp(argv,"-r"))
	{
		int x = 0;
		int END = 0;
		//BUG: Code will not successfully traverse targets[]
		while(x != targets[0]->dep_count || END != 0)
		{
			//for (int y= 0;y < global_counter; y++)
			//{
			//	if (!strcmp(targets[y]->name, targets[0]->depend[x]))
//				{
//					//printf("%s\n", targets[y]->depend[x]);
//				}
//			}
			//x++;
			END++;
		//}
		x=0;
		while(x != targets[0]->recipe_count)
		{
			printf("%s\n", targets[0]->recipe[x]);
			x++;
		}
		return;
	}
	else
	{
	//from newer version, will not print successfully on old version of main.c
		target *temp_target;
		temp_target = find_target_by_name(temp_target_name);
		if (temp_target != NULL){
			exec_tree(temp_target);
		} else{
			printf("Failure: target does not exist\n");
		}
	}
}

int main(int argc, char *argv[]) 
{
	//$./mymake Makefile
	//Similarly account for -r flag

	if (argc <= 1)//not enough arg provided
	{
		printf("Not enough arguments provided: Provided with %i, expected atleast 1.\n", argc-1);
	}
	else if (argc == 2 && (strncmp(argv[1], "-p", 2) || strncmp(argv[1], "-r", 2))) 
		{
		//printf("Test Case 1");
		process_file(argv[1]);
		//TODO
		int count = 0;
		int row = 0;
		int col;
		//find targets
		while (lines[row][0] != '\0')
			{
			//Parse this row; find target and dependencies
			col = 0; // reset column position
			if (lines[row][0] != '\t' && lines[row][0] != '\n')//if we detect a tab or new line then we exec//
				{ 
				//This line has a target
				targets[count] = (target *)malloc(sizeof(target)); //first index of target allocated memory//
				targets[count]->name = parse_next(row, 0); //fetch string from select row and stores to 'name'//
				col += strlen(targets[count]->name) - 1; //column position extended to the length of the string//
				//find dependencies for target
				find_dep(count, row, col); // //
				//printing specific values//
				printf("Target: %s\n", targets[count]->name);
				for (int x = 0; x< targets[count]->dep_count; x++)//since there can be multiple dependencies looping through allows all to be printed from array
				{
					printf("\tDepend:\t%s\n", targets[count]->depend[x]);
				}
				//find recipe for target
				find_recipe(count, row);
				for (int x = 0; x< targets[count]->recipe_count; x++){
					printf("\tRecipe:\t%s\n", targets[count]-> recipe[x]);
				}
				count++; //increment target index
			}
			row++;
		}
	}

	else if (argc == 3) {
		//$./mymake Makefile target
		//printf("Test case 2"); 
		if ((strncmp(argv[1], "-r", 2) && strncmp(argv[1], "-p", 2))) 
		{
			if (!process_file(argv[1]))
			{

				print_(argv[2]);
			}
			else
			{
				printf("\nEither not a Makefile or an incorrect function. Please only use -r or -p.\n");
			}
			//TODO		
		} else if (!strncmp(argv[1], "-p", 2)) {
			//$./mymake -p Makefile 
			if (!process_file(argv[2])) 
			{
				//TODO
				print_(argv[1]);
			}
		}
		//$./mymake -r Makefile
		else if (!strncmp(argv[1], "-r", 2)) 
		{
		//insert code here
			if (!process_file(argv[2])) 
			{
				//TODO
				print_(argv[1]);
			}
		}
	}
	else // too many arguments provided
	{
		printf("Provided with too many arguments:\nPlease use: ./[Executing program name] [Optional -function] [Makefile Name] as a template\n");
	}

	exit(EXIT_SUCCESS);
}*/
