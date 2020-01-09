# This was a project for the class CSCI 4061: Introduction to Operating Systems  
# csci4061-pa1

                /*
                * test machine: csel-kh1250-01.cselabs.umn.edu
                * date due: October 9th, 2019
                * name: Oliver Latocki, Jonathan Sulisz
                * x500: latoc004, sulis005  
                */

• **The purpose of our program**

The purpose of this program is to replicate functionalities of makefiles via the fork, exec, and wait system calls. This will allow us to gain a better understanding of the concept of operating system processes in C as well as the functionality of the make Unix command.

# • How to compile the program

Compiling main.c:

$ gcc -Wall -std=c99 -o mymake main.c


***ASSUMPTIONS***

We are assuming that recipes must begin with a '\t' character, and there are no '\t' without proceeding recipes.  
We are assuming all Makefiles are of the format of those given in the pa1/test folder.  
We are assuming the code is being compiled by gcc using the flags described above, especially that it is being compiled with the "-std=c99" flag.  
We are also assuming that the computer which compiles our program has installed the following libraries/headers:  

        #include <stdio.h>
        #include <stdlib.h>
        #include <string.h>
        #include <sys/types.h>
        #include <sys/wait.h> 
        #include <unistd.h>


# • What exactly your program does

**`void find_dep(int t_index, int row)`**

Inputs: t_index, row  
Outputs: void

**Overview:**
find_dep() parses the Makefile to tokenizes the dependencies of a given target at t_index of the global targets array. It then fills the dependency array for the given target.

**Specifics:**
Given a specific row, a token is set of the initial line to gather the dependencies. A while loop runs until we reach the of the line or either a null or new line is set, during the loop a token is set under a specific target index specified by t_index. After the loop is finished one of the counters is used to remove the newline from the last found dependency, if there is one.

**`void find_recipe(int t_index, int row)`**

Inputs: t_index, row  
Outputs: void

**Overview:**
`find_recipe()` parses the Makefile to fill the recipe array for the target at t_index of the global targets array. The argument "row" is the row in which target lies in the Makefile.

**Specifics:**
Given t_index and the row in which the target at t_index lies in the Makefile, the function iterates through the rows immediately under that of the specified target and copies the recipes at these rows into the target's recipe array. It does this until it finds a row which does not start with '\t', indicating the recipes have been exhausted.

**`void fill_targets()`**

Inputs: none  
Outputs: void

**Overview:**
This function parses the Makefile to allocate each target. It calls the previous functions `find_dep()` and `find_recipe()` for each target to fill the following fields of each target object contained in the global targets array:

    char *name; 
    char *depend[MAX_DEP];  
    char *recipe[MAX_RECIPES_PT];  
    unsigned char dep_count;  
    unsigned char recipe_count;


**Specifics:**
The function iterates through the rows of the Makefile. If it detects a row which does not start with a'\t' or a '\n' character, then it determines that said row contains a target. It then parses and allocates this target and inserts it into the global targets array. It then calls the functions `find_dep()` and `find_recipe()` to parse the dependencies and recipes for each target. At the end of each iteration, the target counter t_count is incremented and the the function iterates to the next target. The function does this until it reaches the end of the Makefile.

**`void print_targets()`**

Inputs: none  
Outputs: void

**Overview:**
This function is called for the "-p" command line flag and is used to print the contents of each target. These include the target's name, the number of dependencies and recipes it has, and the dependencies and recipes themselves. The function prints in the following format:

    target 'targetname' has [number of dependencies] dependencies and [number of recipes] recipes  
    Dependency 0: dependency0  
    Dependency 1: dependency1  
    Recipe 0: recipe0  
    Recipe 2: recipe1  

**`void fork_exec(char *recipe)`**

Inputs: char \*recipe  
Outputs: void

**Overview:**
This function takes a string representation of a recipe (such as "ls -lah"), forks it, executes it, and waits for the created child process to finish. This function is called for each recipe in the Makefile in the order specified by the dependency tree.

**Specifics**
The program calls `fork()`, creating a child process. Then it immediately checks to see if the child pid is less than 0, and throws an error if so. Otherwise, inside the child process, whose pid is 0, the recipe is parsed into an argv array to be passed to `execvp()`. The program then calls `execvp` inside the child process, executing the given recipe.

**`target *find_target_by_name(char *name)`**

Inputs: Char \*name
Outputs: target \*

**Overview:**
This function is used to find the target within the global targets array whose name matches the passed string "name". It returns a pointer to said target, or NULL if said target does not exist.

**`void print_tree_recipes(target *t)`**

Inputs: target \*t  
Outputs: void

**Overview:**
This function is used for the "-r" command line flag to print the recipes of the specified target.

**Specifics:**
This function prints the recipes of the passed target pointer "t" in the correct order they must be executed. It uses depth first search by recursively calling `print_tree_recipes` on each dependency of "t" as long as said dependency is also a target within the Makefile. Once it reaches a leaf target with no unsatisfied dependencies or each dependency of "t", it prints its recipes.

**`void exec_tree(target *t)`**

Inputs: target \*t  
Outputs: void

**Overview:**
This function is used to execute the recipes of a given target.

**Specifics:**
This function is very similar to `print_tree_recipes`, but instead of printing the recipes of "t" in the correct order, it executes them, then waits for them to finish before continuing. This function uses depth first search to execute the dependency tree.


**`int main(int argc, char \*argv[])`**

Inputs: int argc, char \*argv[]  
Outputs: int

**Overview:**
This is the main function which takes the command line arguments and determines how to print or execute the the Makefile.

**Specifics:**
At the beginning we run an if-statement to detect the number of arguments. If we have only two arguments, and the second one is neither '-p' nor '-r', then we call `process_file()` with the second input, which is the Makefile, then we execute the first target of the Makefile. This is the case of the type `./mymake Makefile`

If 3 inputs are entered then we run an check to determine which of the three possible cases was entered. These are of the types `./mymake Makefile target`, `./mymake -p Makefile`, and `./mymake -r Makefile`. For each case, we call `process_file()` and `fill_targets()` to parse the file and all the targets. For the first case, the specified target is executed, along with all of its dependencies. For the second case, the global target array is printed via the `print_targets()` function. For the third case, all the recipes are printed in the order that they must be executed to execute the first target in the Makefile.

If there are more than 3 arguments an error is thrown. If the "-p" or "-r" flag comes after the Makefile, an error is thrown. If the specified target does not exist in the Makefile, an error is thrown.


`void print_specified_target(char *t)` ***UNUSED; NOT PART OF PROGRAM***

Inputs: char \*t  
Outputs: void

**Overview:**
Thus function was used to print indivudial targets in the manner that `print_targets` does, but was rendered unnecessary.



# **• Your and your partner's individual contributions**  
Oliver: I wrote most of the code that is in the final version of main.c, including the functions described above. I debugged the code to fix seg faults and memory issues that arose from not using standard C libary functions. We both worked on the README.

Jonathan: I created a variation of the main.c that relied on older functions (pre-Oct 2nd), wrote alternate version of functions, and wrote descriptions for the functions on the Readme.

# • If you have attempted extra credit  
We did not attempt extra credit.
