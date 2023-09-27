# Welcome to Homework 0!

For these questions you'll need the mini course and  "Linux-In-TheBrowser" virtual machine (yes it really does run in a web page using javascript) at -

http://cs-education.github.io/sys/

Let's take a look at some C code (with apologies to a well known song)-
```C
// An array to hold the following bytes. "q" will hold the address of where those bytes are.
// The [] mean set aside some space and copy these bytes into teh array array
char q[] = "Do you wanna build a C99 program?";

// This will be fun if our code has the word 'or' in later...
#define or "go debugging with gdb?"

// sizeof is not the same as strlen. You need to know how to use these correctly, including why you probably want strlen+1

static unsigned int i = sizeof(or) != strlen(or);

// Reading backwards, ptr is a pointer to a character. (It holds the address of the first byte of that string constant)
char* ptr = "lathe"; 

// Print something out
size_t come = fprintf(stdout,"%s door", ptr+2);

// Challenge: Why is the value of away equal to 1?
int away = ! (int) * "";


// Some system programming - ask for some virtual memory

int* shared = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
munmap(shared,sizeof(int*));

// Now clone our process and run other programs
if(!fork()) { execlp("man","man","-3","ftell", (char*)0); perror("failed"); }
if(!fork()) { execlp("make","make", "snowman", (char*)0); execlp("make","make", (char*)0)); }

// Let's get out of it?
exit(0);
```

## So you want to master System Programming? And get a better grade than B?
```C
int main(int argc, char** argv) {
	puts("Great! We have plenty of useful resources for you, but it's up to you to");
	puts(" be an active learner and learn how to solve problems and debug code.");
	puts("Bring your near-completed answers the problems below");
	puts(" to the first lab to show that you've been working on this.");
	printf("A few \"don't knows\" or \"unsure\" is fine for lab 1.\n"); 
	puts("Warning: you and your peers will work hard in this class.");
	puts("This is not CS225; you will be pushed much harder to");
	puts(" work things out on your own.");
	fprintf(stdout,"This homework is a stepping stone to all future assignments.\n");
	char p[] = "So, you will want to clear up any confusions or misconceptions.\n";
	write(1, p, strlen(p) );
	char buffer[1024];
	sprintf(buffer,"For grading purposes, this homework 0 will be graded as part of your lab %d work.\n", 1);
	write(1, buffer, strlen(buffer));
	printf("Press Return to continue\n");
	read(0, buffer, sizeof(buffer));
	return 0;
}
```
## Watch the videos and write up your answers to the following questions

**Important!**

The virtual machine-in-your-browser and the videos you need for HW0 are here:

http://cs-education.github.io/sys/

The coursebook:

http://cs341.cs.illinois.edu/coursebook/index.html

Questions? Comments? Use Ed: (you'll need to accept the sign up link I sent you)
https://edstem.org/

The in-browser virtual machine runs entirely in Javascript and is fastest in Chrome. Note the VM and any code you write is reset when you reload the page, *so copy your code to a separate document.* The post-video challenges (e.g. Haiku poem) are not part of homework 0 but you learn the most by doing (rather than just passively watching) - so we suggest you have some fun with each end-of-video challenge.

HW0 questions are below. Copy your answers into a text document (which the course staff will grade later) because you'll need to submit them later in the course. More information will be in the first lab.

## Chapter 1

In which our intrepid hero battles standard out, standard error, file descriptors and writing to files.

### Hello, World! (system call style)
1. Write a program that uses `write()` to print out "Hi! My name is `<Your Name>`".

```C
#include <unistd.h>

int main() {
	write(1, "Hi! My name is Zhilan Wang", 26);
	return 0;
}
```

### Hello, Standard Error Stream!
2. Write a function to print out a triangle of height `n` to standard error.
   - Your function should have the signature `void write_triangle(int n)` and should use `write()`.
   - The triangle should look like this, for n = 3:
	```C
	*
	**
	***
	```

```C
#include <unistd.h>
void write_triangle(int n) {
	if (n <= 0) {
		return;
	}
	int i;
	int j;
	for (i = 1; i < n+1; i++) {
		for (j = 0; j < i; j++) {
			write(1, "*", 1);
		}
		write(1, "\n", 1);
	} 
}
int main() {
	write_triangle(5);		//when n = 5
	return 0;
}
```
### Writing to files
3. Take your program from "Hello, World!" modify it write to a file called `hello_world.txt`.
   - Make sure to to use correct flags and a correct mode for `open()` (`man 2 open` is your friend).
```C

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
	// int count;
	mode_t mode = S_IRUSR | S_IWUSR;
	int fildes = open("hello_world.txt", O_CREAT | O_TRUNC | O_RDWR, mode);
	write(fildes,"Hello!",6);
	close(fildes);
	return 0;
}

```
### Not everything is a system call
4. Take your program from "Writing to files" and replace `write()` with `printf()`.
   - Make sure to print to the file instead of standard out!

```C
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int main() {
	// int count;
	mode_t mode = S_IRUSR | S_IWUSR;
	close(1);
	int fildes = open("hello_world.txt", O_CREAT | O_TRUNC | O_RDWR, mode);
	// write(fildes,"Hello!",6);
	printf("%s", "Hello!");
	close(fildes);
	return 0;
}
```

5. What are some differences between `write()` and `printf()`?

write() is too basic since it only designed to write a sequence of bytes. However, we can write data in different format by using printf(), and it can also print out various data type. For example, string, char, and int. (Paraphrased from Reddit and Stack Overflow).

## Chapter 2

Sizing up C types and their limits, `int` and `char` arrays, and incrementing pointers

### Not all bytes are 8 bits?
1. How many bits are there in a byte? 
8 bits.

2. How many bytes are there in a `char`? 
1 byte.

3. How many bytes the following are on your machine? 
   - `int`, `double`, `float`, `long`, and `long long`
   ```C
   int -> 4
   double -> 8
   float -> 4
   long -> 4 
   longlong -> 8
   ```

### Follow the int pointer
4. On a machine with 8 byte integers:
```C
#include <stdio.h>
int main(){
    int data[8];
	printf("data is at %p \n", data);
	printf("data is at %p \n", data+2);
	return 0;
} 
```
If the address of data is `0x7fbd9d40`, then what is the address of `data+2`?

0x7fbd9d50

5. What is `data[3]` equivalent to in C?
   - Hint: what does C convert `data[3]` to before dereferencing the address?

it means data+3

### `sizeof` character arrays, incrementing pointers
  
Remember, the type of a string constant `"abc"` is an array.

6. Why does this segfault?
```C
char *ptr = "hello";
*ptr = 'J';
```
Different parts of the processes memory are valid for reading or reading & writing. As a string, "hello" can only be read, in other words, it is immutable. So the hardware is sophisticated enough to know which parts of memory are read, which parts of memory are read and write, and which parts of memory are invalid. Therefore, we will have a segmentation fault.

7. What does `sizeof("Hello\0World")` return? 

12

8. What does `strlen("Hello\0World")` return? 

5

9. Give an example of X such that `sizeof(X)` is 3. 

sizeof("hi") is 3.

10. Give an example of Y such that `sizeof(Y)` might be 4 or 8 depending on the machine.

sizeof(1) is 4 or 8 depending on the machine.

## Chapter 3

Program arguments, environment variables, and working with character arrays (strings)

### Program arguments, `argc`, `argv`
1. What are two ways to find the length of `argv`?

(1) argc contains the count of arguments in argv, which is the length of it.

(2) using the following code
```C
# include <stdio.h>
int main(int argc, char* argv[]) {
	int count = 0;
	while(argv[count]) {
		count++;
	}
	return 0;
}
```

2. What does `argv[0]` represent?

argv[0] is the filename of the program.(e.g. ./program)

### Environment Variables
3. Where are the pointers to environment variables stored (on the stack, the heap, somewhere else)?

Somewhere else. It stored in the process.

### String searching (strings are just char arrays)
4. On a machine where pointers are 8 bytes, and with the following code:
```C
char *ptr = "Hello";
char array[] = "Hello";
```
What are the values of `sizeof(ptr)` and `sizeof(array)`? Why?

sizeof(ptr) is 8. Since the size of a pointer on this particular machine are 8 bytes.
sizeof(array) is 6. Because the array contains five bytes of chars and a zero at the end.

### Lifetime of automatic variables

5. What data structure manages the lifetime of automatic variables?

Stack.

## Chapter 4 
Heap and stack memory, and working with structs

### Memory allocation using `malloc`, the heap, and time
1. If I want to use data after the lifetime of the function it was created in ends, where should I put it? How do I put it there?

malloc, calloc, realloc

2. What are the differences between heap and stack memory?

Heap is the dynamic memory for programmer to allocate, whereas the stack stores local variables. The allocation and deallocation for stack memory is automatically done, but heap memory will not be deallocated until it is freed. (From CS225 course website)

3. Are there other kinds of memory in a process?

Text segment, initialized data segment, uninitialized data segment.

4. Fill in the blank: "In a good C program, for every malloc, there is a ___".

free.

### Heap allocation gotchas
5. What is one reason `malloc` can fail?

The system is low on memory, in other words, there is no more space.

6. What are some differences between `time()` and `ctime()`?

time() returns time_t, and it will returns the time since Jan 1, 1970 in seconds.
The ctime() function returns the string representing the localtime based on the argument timer. (From GeeksforGeeks) Moreover, ctime() will return a string and it allows greater control over to the precise formatting. ctime() will use static storage. 

7. What is wrong with this code snippet?
```C
free(ptr);
free(ptr);
```
Double free.

8. What is wrong with this code snippet?
```C
free(ptr);
printf("%s\n", ptr);
```
This will result a dangling pointer which means that the pointer do not point to a valid object.

9. How can one avoid the previous two mistakes? 

Set ptr = NULL, so that the pointer will no longer pointing to invalid memory.

### `struct`, `typedef`s, and a linked list
10. Create a `struct` that represents a `Person`. Then make a `typedef`, so that `struct Person` can be replaced with a single word. A person should contain the following information: their name (a string), their age (an integer), and a list of their friends (stored as a pointer to an array of pointers to `Person`s).
```C
#include <stdio.h>
#include <stdlib.h>
struct Person {
	char name;
	int age;
	struct Person* friends;
}

typedef struct Person person_t;

int main() {
	return 0;
}
```

11. Now, make two persons on the heap, "Agent Smith" and "Sonny Moore", who are 128 and 256 years old respectively and are friends with each other.

```C
#include <stdio.h>
#include <stdlib.h>
struct Person {
	char* name;
	int age;
	struct Person** friends;
}

typedef struct Person person_t;

int main() {
	person_t* one = (person_t*) malloc(sizeof(person_t));
	person_t* two = (person_t*) malloc(sizeof(person_t));
	one->name = "Agent Smith";
	two->name = "Sonny Moore";
	one->age = 128;
	two->age = 256;
	one->friends = two;
	two->friends = one;
	free(one);
	free(two);
	return 0;
}
```

### Duplicating strings, memory allocation and deallocation of structures
Create functions to create and destroy a Person (Person's and their names should live on the heap).

12. `create()` should take a name and age. The name should be copied onto the heap. Use malloc to reserve sufficient memory for everyone having up to ten friends. Be sure initialize all fields (why?).
```C
person_t* create(char* aname, int aage) {
	printf("Creating link %s -> %s", aname, aage);
	person_t* result = (person_t*) malloc(sizeof(person_t));
	if (aname == NULL) {
		result->name = "default_name";
	}
	if (aage == NULL) {
		result->name = 0;
	}
	result->name = strdup(aname);
	result->age = aage;
	result->friends = (person_t*) malloc(sizeof(person_t) * 10); //ten friends
	return result;
}
```

13. `destroy()` should free up not only the memory of the person struct, but also free all of its attributes that are stored on the heap. Destroying one person should not destroy any others.
```C
void destroy(person_t* p) {
	free(p->name);
	free(p->age);
	memset(p->friends, 0, sizeof(person_t));
	free(p);
}
```

## Chapter 5 

Text input and output and parsing using `getchar`, `gets`, and `getline`.

### Reading characters, trouble with gets
1. What functions can be used for getting characters from `stdin` and writing them to `stdout`?

getchar() and putchar().

2. Name one issue with `gets()`.

gets() does not have any array bound testing, therefore it might cause buffer overflow.

### Introducing `sscanf` and friends

3. Write code that parses the string "Hello 5 World" and initializes 3 variables to "Hello", 5, and "World".

```C
#include <stdio.h>

int main() {
	char* data = "Hello 5 World";
	char buffer1[10];
	char buffer2[20];
	int score = -42;
	sscanf(data, "%s %d %s", buffer1, &score, buffer2); 
	return 0;
}
```

### `getline` is useful
4. What does one need to define before including `getline()`?
```C
#define _GNU_SOURCE
```

5. Write a C program to print out the content of a file line-by-line using `getline()`.

```C
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>k

int main(int argc, char **argv){
  FILE *f = fopen(argv, "r");
  char* line = NULL;
  ssize_t rd;
  size_t len = 0;
  while ((rd = getline(&line, &len, f)) != -1) {
               printf("%s", line);
  }
  fclose(f);
  if (line) {
	free(line);
  }
  return 0;
}
```

## C Development

These are general tips for compiling and developing using a compiler and git. Some web searches will be useful here

1. What compiler flag is used to generate a debug build?
-g

2. You modify the Makefile to generate debug builds and type `make` again. Explain why this is insufficient to generate a new build.

First need make clean to clear previous target.

3. Are tabs or spaces used to indent the commands after the rule in a Makefile?

Tabs used to indent the commands after the rule in a Makefile.

4. What does `git commit` do? What's a `sha` in the context of git?

"git commit" will save the changes to the local repository so that we could keep track of the progress and changes. 

"sha" stands for Secure Hash Algorithm, which is meaningful hashes for commit objects. Git use it to identify different objects and allows us to know where changes were made. (From Vjeko.com)

5. What does `git log` show you?

It displays all the commits in reverse chronological order. It also includes the SHA, author, date and message of the commit.

6. What does `git status` tell you and how would the contents of `.gitignore` change its output?

"git status" will tell you which files are stages, not stages, not being tracked. However, we could intentionally ignore and untracked files by setting the path of that file as .gitignore. Filed that already tracked Git will have no effect. (https://www.atlassian.com/git/tutorials/inspecting-a-repository)

7. What does `git push` do? Why is it not just sufficient to commit with `git commit -m 'fixed all bugs' `?

git push will upload or update the content to the repository after local commits. It is not enough to just commit with `git commit -m 'fixed all bugs' ` since we only commit the files to local instead of remote.

8. What does a non-fast-forward error `git push` reject mean? What is the most common way of dealing with this?

It happens when the commit is lost or another user is pushing to the same branch. The most common way to fix this problem is to fetch and merge the remote branch.

## Optional (Just for fun)
- Convert your a song lyrics into System Programming and C code and share on Ed.
- Find, in your opinion, the best and worst C code on the web and post the link to Ed.
- Write a short C program with a deliberate subtle C bug and post it on Ed to see if others can spot your bug.
- Do you have any cool/disastrous system programming bugs you've heard about? Feel free to share with your peers and the course staff on Ed.